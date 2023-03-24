/*
 * cafebabe - the class loader library in C
 * Copyright (C) 2010  Pekka Enberg
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

#include <stdio.h>
#include <stdlib.h>

#include "cafebabe/inner_classes_attribute.h"
#include "cafebabe/attribute_info.h"
#include "cafebabe/stream.h"
#include "cafebabe/class.h"

int
cafebabe_inner_classes_attribute_init(struct cafebabe_inner_classes_attribute *a, struct cafebabe_stream *s)
{
	if (cafebabe_stream_read_uint16(s, &a->number_of_classes))
		goto out;

	a->inner_classes = cafebabe_stream_malloc(s, sizeof(*a->inner_classes) * a->number_of_classes);
	if (!a->inner_classes)
		goto out;

	for (uint16_t i = 0; i < a->number_of_classes; ++i) {
		struct cafebabe_inner_class *e = &a->inner_classes[i];

		if (cafebabe_stream_read_uint16(s, &e->inner_class_info_index))
			goto out_free;

		if (cafebabe_stream_read_uint16(s, &e->outer_class_info_index))
			goto out_free;

		if (cafebabe_stream_read_uint16(s, &e->inner_name_index))
			goto out_free;

		if (cafebabe_stream_read_uint16(s, &e->inner_class_access_flags))
			goto out_free;
	}
	return 0;

out_free:
	free(a->inner_classes);
out:
	return 1;
}

void
cafebabe_inner_classes_attribute_deinit(struct cafebabe_inner_classes_attribute *a)
{
	free(a->inner_classes);
}

int cafebabe_read_inner_classes_attribute(const struct cafebabe_class *class,
					     const struct cafebabe_attribute_array *attributes,
					     struct cafebabe_inner_classes_attribute *inner_classes_attrib)
{
	const struct cafebabe_attribute_info *attribute;
	unsigned int inner_classes_index = 0;
	struct cafebabe_stream stream;

	if (cafebabe_attribute_array_get(attributes, "InnerClasses", class, &inner_classes_index))
		return 0;

	attribute = &class->attributes.array[inner_classes_index];

	cafebabe_stream_open_buffer(&stream, attribute->info, attribute->attribute_length);

	if (cafebabe_inner_classes_attribute_init(inner_classes_attrib, &stream))
		return -1;	/* XXX */

	cafebabe_stream_close_buffer(&stream);

	return 0;
}
