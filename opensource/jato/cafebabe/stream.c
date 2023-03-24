/*
 * cafebabe - the class loader library in C
 * Copyright (C) 2008  Vegard Nossum <vegardno@ifi.uio.no>
 *
 * This file is released under the GPL version 2 with the following
 * clarification and special exception:
 *
 *     Linking this library statically or dynamically with other modules is
 *     making a combined work based on this library. Thus, the terms and
 *     conditions of the GNU General Public License cover the whole
 *     combination.
 *
 *     As a special exception, the copyright holders of this library give you
 *     permission to link this library with independent modules to produce an
 *     executable, regardless of the license terms of these independent
 *     modules, and to copy and distribute the resulting executable under terms
 *     of your choice, provided that you also meet, for each linked independent
 *     module, the terms and conditions of the license of that module. An
 *     independent module is a module which is not derived from or based on
 *     this library. If you modify this library, you may extend this exception
 *     to your version of the library, but you are not obligated to do so. If
 *     you do not wish to do so, delete this exception statement from your
 *     version.
 *
 * Please refer to the file LICENSE for details.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cafebabe/error.h"
#include "cafebabe/stream.h"

int
cafebabe_stream_open(struct cafebabe_stream *s, const char *filename)
{
	s->filename = strdup(filename);
	if (!s->filename) {
		s->syscall_errno = errno;
		s->cafebabe_errno = CAFEBABE_ERROR_ERRNO;
		goto out;
	}

	s->fd = open(filename, O_RDONLY);
	if (s->fd == -1) {
		s->syscall_errno = errno;
		s->cafebabe_errno = CAFEBABE_ERROR_ERRNO;
		goto out_filename;
	}

	if (fstat(s->fd, &s->stat) == -1) {
		s->syscall_errno = errno;
		s->cafebabe_errno = CAFEBABE_ERROR_ERRNO;
		goto out_fd;
	}

	s->virtual = mmap(NULL, s->stat.st_size,
		PROT_READ, MAP_PRIVATE, s->fd, 0);
	if (s->virtual == MAP_FAILED) {
		s->syscall_errno = errno;
		s->cafebabe_errno = CAFEBABE_ERROR_ERRNO;
		goto out_fd;
	}

	s->virtual_i = 0;
	s->virtual_n = s->stat.st_size;

	return 0;

out_fd:
	do {
		if (close(s->fd == -1)) {
			if (errno == EINTR)
				continue;

			fprintf(stderr, "error: %s: %s\n",
				s->filename, strerror(errno));
			break;
		}
	} while(0);
out_filename:
	free(s->filename);
out:
	return 1;
}

void
cafebabe_stream_close(struct cafebabe_stream *s)
{
	if (munmap(s->virtual, s->stat.st_size) == -1) {
		/* This is the best we can do. It shouldn't happen. */
		fprintf(stderr, "error: %s\n", strerror(errno));
	}

	/* We try not to leak file descriptors. */
	do {
		if (close(s->fd) == -1) {
			if (errno == EINTR)
				continue;

			/* This is the best we can do. It shouldn't happen. */
			fprintf(stderr, "error: %s: %s\n",
				s->filename, strerror(errno));
			break;
		}
	} while(0);

	free(s->filename);
}

void
cafebabe_stream_open_buffer(struct cafebabe_stream *s,
	uint8_t *buf, unsigned int size)
{
	s->virtual = buf;
	s->virtual_i = 0;
	s->virtual_n = size;
}

void
cafebabe_stream_close_buffer(struct cafebabe_stream *s)
{
}

const char *
cafebabe_stream_error(struct cafebabe_stream *s)
{
	if (s->cafebabe_errno == CAFEBABE_ERROR_ERRNO)
		return strerror(s->syscall_errno);

	return cafebabe_strerror(s->cafebabe_errno);
}

int
cafebabe_stream_eof(struct cafebabe_stream *s)
{
	return s->virtual_i == s->virtual_n;
}

int
cafebabe_stream_read_uint8(struct cafebabe_stream *s, uint8_t *r)
{
	uint8_t *v = s->virtual;
	unsigned int i = s->virtual_i;

	if (i >= s->virtual_n) {
		s->cafebabe_errno = CAFEBABE_ERROR_UNEXPECTED_EOF;
		return 1;
	}

	*r = v[i];
	s->virtual_i += 1;
	return 0;
}

int
cafebabe_stream_read_uint16(struct cafebabe_stream *s, uint16_t *r)
{
	uint8_t *v = s->virtual;
	unsigned int i = s->virtual_i;

	if (i + 1 >= s->virtual_n) {
		s->cafebabe_errno = CAFEBABE_ERROR_UNEXPECTED_EOF;
		return 1;
	}

	*r = (v[i] << 8) | v[i + 1];
	s->virtual_i += 2;
	return 0;
}

int
cafebabe_stream_read_uint32(struct cafebabe_stream *s, uint32_t *r)
{
	uint8_t *v = s->virtual;
	unsigned int i = s->virtual_i;

	if (i + 3 >= s->virtual_n) {
		s->cafebabe_errno = CAFEBABE_ERROR_UNEXPECTED_EOF;
		return 1;
	}

	*r = (v[i] << 24) | (v[i + 1] << 16) | (v[i + 2] << 8) | v[i + 3];
	s->virtual_i += 4;
	return 0;
}

uint8_t *cafebabe_stream_pointer(struct cafebabe_stream *s)
{
	return s->virtual + s->virtual_i;
}

int
cafebabe_stream_skip(struct cafebabe_stream *s, unsigned int n)
{
	unsigned int i = s->virtual_i;

	if (i + n >= s->virtual_n) {
		s->cafebabe_errno = CAFEBABE_ERROR_UNEXPECTED_EOF;
		return 1;
	}

	s->virtual_i += n;
	return 0;
}

void *
cafebabe_stream_malloc(struct cafebabe_stream *s, size_t size)
{
	void *ptr = malloc(size);
	if (!ptr) {
		s->syscall_errno = ENOMEM;
		s->cafebabe_errno = CAFEBABE_ERROR_ERRNO;
	}

	return ptr;
}
