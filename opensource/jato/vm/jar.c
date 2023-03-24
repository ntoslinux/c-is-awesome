#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/string.h"
#include "lib/list.h"
#include "lib/zip.h"

#include "vm/jar.h"

struct parse_buffer {
	const char		*data;
	unsigned int		i;
};

/*
 * The following is an implementation of the Jar file format as specified in:
 *
 * http://download.oracle.com/javase/1.3/docs/guide/jar/jar.html
 */

static struct jar_header *jar_header_alloc(void)
{
	return malloc(sizeof(struct jar_header));
}

static void jar_header_free(struct jar_header *jh)
{
	free(jh->name);
	free(jh->value);
	free(jh);
}

static bool parse_alphanum(struct parse_buffer *b, char *alphanum_result)
{
	char c = b->data[b->i];

	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
	    || (c >= '0' && c <= '9')) {
		*alphanum_result = c;
		++b->i;
		return true;
	}

	return false;
}

static bool parse_headerchar(struct parse_buffer *b, char *headerchar_result)
{
	char ch;

	if (parse_alphanum(b, headerchar_result))
		return true;

	ch = b->data[b->i];

	if (ch == '-' || ch == '_') {
		*headerchar_result = ch;
		++b->i;
		return true;
	}

	return false;
}

static bool parse_newline(struct parse_buffer *b)
{
	if (b->data[b->i] == '\r') {
		++b->i;

		if (b->data[b->i] == '\n')
			++b->i;

		return true;
	}

	if (b->data[b->i] == '\n') {
		++b->i;

		return true;
	}

	return false;
}

static bool parse_otherchar(struct parse_buffer *b, char *otherchar_result)
{
	char c = b->data[b->i];

	if (c == '\0' || c == '\r' || c == '\n')
		return false;

	*otherchar_result = c;
	++b->i;

	return true;
}

static bool parse_continuation(struct parse_buffer *b,
			       char **continuation_result)
{
	struct string *continuation;
	char otherchar;

	if (b->data[b->i] != ' ')
		return false;

	++b->i;

	continuation = alloc_str();

	while (parse_otherchar(b, &otherchar))
		str_append(continuation, "%c", otherchar);

	if (!parse_newline(b))
		goto out_free_continuation;

	*continuation_result = strdup(continuation->value);

	free_str(continuation);

	return true;

out_free_continuation:
	free_str(continuation);

	return false;
}

static bool parse_value(struct parse_buffer *b, char **value_result)
{
	struct string *value;
	char *continuation;
	char otherchar;

	if (b->data[b->i] != ' ')
		return false;

	++b->i;

	value = alloc_str();

	while (parse_otherchar(b, &otherchar))
		str_append(value, "%c", otherchar);

	if (!parse_newline(b))
		goto out_free_value;

	while (parse_continuation(b, &continuation))
		str_append(value, "%s", continuation);

	*value_result = strdup(value->value);

	free_str(value);

	return true;

out_free_value:
	free_str(value);

	return false;
}

static bool parse_name(struct parse_buffer *b, char **name_result)
{
	struct string *name;
	char headerchar;
	char alphanum;

	if (!parse_alphanum(b, &alphanum))
		return false;

	name = alloc_str();

	str_append(name, "%c", alphanum);

	while (parse_headerchar(b, &headerchar))
		str_append(name, "%c", headerchar);

	*name_result = strdup(name->value);

	free_str(name);

	return true;
}

static bool parse_header(struct parse_buffer *b,
			 struct jar_header **header_result)
{
	struct jar_header *header;
	char *value;
	char *name;

	if (!parse_name(b, &name))
		return false;

	if (b->data[b->i] != ':')
		goto out_free_name;

	++b->i;

	if (!parse_value(b, &value))
		goto out_free_name;

	header = jar_header_alloc();

	header->name	= name;
	header->value	= value;

	*header_result = header;

	return true;

out_free_name:
	free(name);

	return false;
}

/* Manifest Specification */

static struct jar_manifest *jar_manifest_alloc(void)
{
	struct jar_manifest *jm = malloc(sizeof *jm);

	jm->main_section = NULL;
	INIT_LIST_HEAD(&jm->individual_sections);

	return jm;
}

static void jar_manifest_free(struct jar_manifest *jm)
{
	struct jar_individual_section *section, *tmp_section;

	free(jm->main_section);

	list_for_each_entry_safe(section, tmp_section,
				 &jm->individual_sections, node) {
		free(section);
	}

	free(jm);
}

static struct jar_main_section *jar_main_section_alloc(void)
{
	struct jar_main_section *jms = malloc(sizeof *jms);

	INIT_LIST_HEAD(&jms->main_attributes);

	return jms;
}

static void jar_main_section_free(struct jar_main_section *jms)
{
	free(jms);
}

static struct jar_individual_section *jar_individual_section_alloc(void)
{
	struct jar_individual_section *jis = malloc(sizeof *jis);

	jis->name_header = NULL;
	INIT_LIST_HEAD(&jis->perentry_attributes);

	return jis;
}

static void jar_individual_section_free(struct jar_individual_section *jis)
{
	struct jar_header *header, *tmp_header;

	free(jis->name_header);

	list_for_each_entry_safe(header, tmp_header,
				 &jis->perentry_attributes, node) {
		free(header);
	}

	free(jis);
}

static struct jar_individual_section *
parse_individual_section(struct parse_buffer *b)
{
	struct jar_individual_section *individual_section;
	struct jar_header *perentry_attribute;

	individual_section = jar_individual_section_alloc();

	if (!parse_header(b, &individual_section->name_header))
		goto out_free_individual_section;

	if (strcmp(individual_section->name_header->name, "Name"))
		goto out_free_individual_section;

	while (parse_header(b, &perentry_attribute)) {
		list_add_tail(&perentry_attribute->node,
			      &individual_section->perentry_attributes);
	}

	return individual_section;

out_free_individual_section:
	jar_individual_section_free(individual_section);
	return NULL;
}

static bool parse_digit(struct parse_buffer *b, unsigned int *digit_result)
{
	char c = b->data[b->i];

	if (c >= '0' && c <= '9') {
		*digit_result = c - '0';
		++b->i;
		return true;
	}

	return false;
}

static bool parse_version_number(struct parse_buffer *b,
				 unsigned int *version_number_result)
{
	unsigned int version_number = 0;
	unsigned int digit;

	if (!parse_digit(b, &version_number))
		return false;

	while (parse_digit(b, &digit))
		version_number = 10 * version_number + digit;

	*version_number_result = version_number;

	return true;
}

static bool parse_version_info(struct parse_buffer *b,
			       unsigned int *major_version_result,
			       unsigned int *minor_version_result)
{
	struct jar_header *header;
	struct parse_buffer pb;

	if (!parse_header(b, &header))
		return false;

	if (strcmp(header->name, "Manifest-Version"))
		goto out_free_header;

	pb.data	= header->value;
	pb.i	= 0;

	if (!parse_version_number(&pb, major_version_result))
		goto out_free_header;

	if (pb.data[pb.i] != '.')
		goto out_free_header;

	++pb.i;

	if (!parse_version_number(&pb, minor_version_result))
		goto out_free_header;

	/* Make sure this was the end of the value */
	if (pb.data[pb.i] != '\0')
		goto out_free_header;

	jar_header_free(header);

	return true;

out_free_header:
	jar_header_free(header);

	return false;
}

static bool parse_main_section(struct parse_buffer *b,
			       struct jar_main_section **main_section_result)
{
	struct jar_main_section *main_section = jar_main_section_alloc();
	struct jar_header *attribute;

	if (!parse_version_info(b,
				&main_section->major_version,
				&main_section->minor_version)) {
		goto out_free_main_section;
	}
#if 0
	/* Note: Although the grammar specifies a newline here, it is actually
	 * part of the version-info we parsed above (version-info is actually
	 * a header/attribute, which already includes the newline). */
	if (!parse_newline(b))
		goto out_free_main_section;
#endif

	while (parse_header(b, &attribute))
		list_add_tail(&attribute->node, &main_section->main_attributes);

	*main_section_result = main_section;

	return true;

out_free_main_section:
	jar_main_section_free(main_section);

	return false;
}

static bool parse_manifest_file(struct parse_buffer *b,
				struct jar_manifest **manifest_result)
{
	struct jar_manifest *manifest = jar_manifest_alloc();
	struct jar_individual_section *section;

	if (!parse_main_section(b, &manifest->main_section))
		goto out_free_manifest;

	if (!parse_newline(b))
		goto out_free_manifest;

	while ((section = parse_individual_section(b)) != NULL) {
		list_add_tail(&section->node, &manifest->individual_sections);
	}

	*manifest_result = manifest;

	return true;

out_free_manifest:
	jar_manifest_free(manifest);

	return false;
}

static struct jar_manifest *read_manifest(struct zip *zip)
{
	struct zip_entry *zip_entry;
	struct parse_buffer pb;
	struct jar_manifest *manifest;

	zip_entry = zip_entry_find(zip, "META-INF/MANIFEST.MF");
	if (!zip_entry)
		return NULL;

	pb.data	= zip_entry_data(zip, zip_entry);
	pb.i	= 0;

	if (!parse_manifest_file(&pb, &manifest)) {
		NOT_IMPLEMENTED;
		printf("parse error, byte offset %d. oops\n", pb.i);
		return NULL;
	}

	return manifest;
}

struct vm_jar *vm_jar_open(const char *filename)
{
	struct jar_manifest *manifest;
	struct vm_jar *jar = NULL;
	struct zip *zip;

	zip = zip_open(filename);
	if (!zip)
		goto error_out;

	jar = malloc(sizeof *jar);
	if (!jar)
		goto error_out_close;

	manifest = read_manifest(zip);
	if (!manifest) {
		free(jar);
		goto error_out_close;
	}

	jar->manifest = manifest;
error_out_close:
	zip_close(zip);
error_out:
	return jar;
}

const char *vm_jar_get_main_class(const struct vm_jar *jar)
{
	struct jar_main_section *section;
	struct jar_header *header;

	if (!jar)
		return NULL;

	if (!jar->manifest)
		return NULL;

	if (!jar->manifest->main_section)
		return NULL;

	section = jar->manifest->main_section;

	list_for_each_entry(header, &section->main_attributes, node) {
		if (!strcmp(header->name, "Main-Class"))
			return header->value;
	}

	return NULL;
}
