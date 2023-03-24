/*
 * Copyright (c) 2010 Tomasz Grabiec
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

#include "lib/compile-lock.h"
#include "vm/thread.h"

#include <stdlib.h>

int compile_lock_init(struct compile_lock *cl, bool reentrant)
{
	cl->reentrant = reentrant;
	atomic_set(&cl->status, STATUS_INITIAL);
	atomic_set(&cl->nr_waiting, 0);
	return sem_init(&cl->wait_sem, 0, 0);
}


enum compile_lock_status compile_lock_enter(struct compile_lock *cl)
{
	/*
	 * Check if already compiled.  This is an optimisation, not
	 * necessary.
	 */
	enum compile_lock_status status = atomic_read(&cl->status);
	if (status > STATUS_COMPILING) {
		/* Status read should happen before read on protected data */
		smp_rmb();
		return status;
	}

	if (atomic_cmpxchg(&cl->status, 0, STATUS_COMPILING) == 0) {
		if (cl->reentrant)
			cl->compiling_ee = vm_get_exec_env();

		return STATUS_COMPILING;
	}

	atomic_inc(&cl->nr_waiting);

	smp_mb();

	/* Other thread is doing compilation... */
	while (atomic_read(&cl->status) == STATUS_COMPILING) {
		if (cl->reentrant && cl->compiling_ee == vm_get_exec_env()) {
			atomic_dec(&cl->nr_waiting);
			return STATUS_COMPILED_OK;
		}

		sem_wait(&cl->wait_sem);

		/* The read of .status should happen after wakeup. */
		smp_rmb();
	}

	atomic_dec(&cl->nr_waiting);
	return atomic_read(&cl->status);
}

void compile_lock_leave(struct compile_lock *cl,
			enum compile_lock_status status)
{
	/* All updates to protected data should happen before status update. */
	smp_wmb();

	atomic_set(&cl->status, status);

	/* the read on .nr_waiting must occure after .status is set */
	smp_mb();

	int nr_to_wake = atomic_read(&cl->nr_waiting);

	while (nr_to_wake--)
		sem_post(&cl->wait_sem);
}
