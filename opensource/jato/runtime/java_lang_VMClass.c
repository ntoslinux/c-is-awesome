/*
 * Copyright (c) 2009 Tomasz Grabiec
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

#include "runtime/java_lang_VMClass.h"

#include "jit/exception.h"

#include "vm/classloader.h"
#include "vm/annotation.h"
#include "vm/reflection.h"
#include "vm/preload.h"
#include "vm/errors.h"
#include "vm/object.h"
#include "vm/class.h"
#include "vm/call.h"
#include "vm/utf8.h"
#include "vm/vm.h"

#include <stddef.h>

struct vm_object *java_lang_VMClass_forName(struct vm_object *name, jboolean initialize, struct vm_object *loader)
{
	struct vm_class *class;
	char *class_name;

	if (!name) {
		signal_new_exception(vm_java_lang_NullPointerException, NULL);
		return NULL;
	}

	class_name = vm_string_classname_to_cstr(name);
	if (!class_name)
		return throw_oom_error();

	class = classloader_load(loader, class_name);
	if (!class)
		goto throw_cnf;

	if (initialize) {
		if (vm_class_ensure_init(class))
			goto throw_cnf;
	}

	return class->object;
 throw_cnf:
	signal_new_exception(vm_java_lang_ClassNotFoundException, class_name);
	free(class_name);
	return NULL;
}

struct vm_object *java_lang_VMClass_getClassLoader(struct vm_object *object)
{
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(object);
	if (!vmc)
		return NULL;

	return vmc->classloader;
}

struct vm_object *java_lang_VMClass_getComponentType(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return NULL;

	if (!vm_class_is_array_class(class)) {
		warn("%s", class->name);
		return NULL;
	}

	return vm_class_get_array_element_class(class)->object;
}

jobject java_lang_VMClass_getDeclaredAnnotations(jobject klass)
{
	struct vm_object *result;
	struct vm_class *vmc;
	unsigned int i;

	vmc = vm_object_to_vm_class(klass);
	if (!vmc)
		return rethrow_exception();

	result = vm_object_alloc_array(vm_array_of_java_lang_annotation_Annotation, vmc->nr_annotations);
	if (!result)
		return rethrow_exception();

	for (i = 0; i < vmc->nr_annotations; i++) {
		struct vm_annotation *vma = vmc->annotations[i];
		struct vm_object *annotation;

		annotation = vm_annotation_to_object(vma);
		if (!annotation)
			return rethrow_exception();

		array_set_field_object(result, i, annotation);
	}

	return result;
}

jobject java_lang_VMClass_getDeclaredClasses(jobject klass, jboolean public_only)
{
	struct vm_object *result;
	unsigned int count = 0;
	unsigned int index = 0;
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(klass);
	if (!vmc)
		return NULL;

	for (unsigned int i = 0; i < vmc->nr_inner_classes; i++) {
		uint16_t inner_ndx = vmc->inner_classes[i];
		struct vm_class *inner_vmc;

		inner_vmc = vm_class_resolve_class(vmc, inner_ndx);
		if (!inner_vmc)
			return rethrow_exception();

		if (!vm_class_is_public(inner_vmc) && public_only)
			continue;

		count++;
	}

	result = vm_object_alloc_array(vm_array_of_java_lang_Class, count);
	if (!result)
		return rethrow_exception();

	for (unsigned int i = 0; i < vmc->nr_inner_classes; i++) {
		uint16_t inner_ndx = vmc->inner_classes[i];
		struct vm_class *inner_vmc;

		inner_vmc = vm_class_resolve_class(vmc, inner_ndx);
		if (!inner_vmc)
			return rethrow_exception();

		if (!vm_class_is_public(inner_vmc) && public_only)
			continue;

		array_set_field_ptr(result, index++, inner_vmc->object);
	}

	return result;
}

jobject java_lang_VMClass_getDeclaredConstructors(jobject clazz, jboolean public_only)
{
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(clazz);
	if (!vmc)
		return NULL;

	if (vm_class_is_primitive_class(vmc) || vm_class_is_array_class(vmc))
		return vm_object_alloc_array(vm_array_of_java_lang_reflect_Constructor, 0);

	int count = 0;

	for (int i = 0; i < vmc->class->methods_count; i++) {
		struct vm_method *vmm = &vmc->methods[i];

		if (!vm_method_is_constructor(vmm) ||
		    (!vm_method_is_public(vmm) && public_only))
			continue;

		count++;
	}

	struct vm_object *array;

	array = vm_object_alloc_array(vm_array_of_java_lang_reflect_Constructor,
				      count);
	if (!array)
		return rethrow_exception();

	int index = 0;

	for (int i = 0; i < vmc->class->methods_count; i++) {
		struct vm_method *vmm = &vmc->methods[i];

		if (!vm_method_is_constructor(vmm) ||
		    (!vm_method_is_public(vmm) && public_only))
			continue;

		struct vm_object *ctor
			= vm_object_alloc(vm_java_lang_reflect_Constructor);

		if (!ctor)
			return rethrow_exception();

		if (vm_java_lang_reflect_VMConstructor != NULL) { /* Classpath 0.98 */
			struct vm_object *vm_ctor;

			vm_ctor = vm_object_alloc(vm_java_lang_reflect_VMConstructor);
			if (!vm_ctor)
				return rethrow_exception();

			field_set_object(vm_ctor, vm_java_lang_reflect_VMConstructor_clazz, clazz);
			field_set_int(vm_ctor, vm_java_lang_reflect_VMConstructor_slot, i);

			field_set_object(ctor, vm_java_lang_reflect_Constructor_cons, vm_ctor);
		} else {
			field_set_object(ctor, vm_java_lang_reflect_Constructor_clazz, clazz);
			field_set_int(ctor, vm_java_lang_reflect_Constructor_slot, i);
		}
		array_set_field_ptr(array, index++, ctor);
	}

	return array;
}

jobject java_lang_VMClass_getDeclaredFields(jobject clazz, jboolean public_only)
{
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(clazz);
	if (!vmc)
		return NULL;

	if (vm_class_is_primitive_class(vmc) || vm_class_is_array_class(vmc))
		return vm_object_alloc_array(vm_array_of_java_lang_reflect_Field, 0);

	int count;

	if (public_only) {
		count = 0;

		for (int i = 0; i < vmc->class->fields_count; i++) {
			struct vm_field *vmf = &vmc->fields[i];

			if (vm_field_is_public(vmf))
				count ++;
		}
	} else {
		count = vmc->class->fields_count;
	}

	struct vm_object *array
		= vm_object_alloc_array(vm_array_of_java_lang_reflect_Field,
					count);
	if (!array)
		return rethrow_exception();

	int index = 0;

	for (int i = 0; i < vmc->class->fields_count; i++) {
		struct vm_field *vmf = &vmc->fields[i];

		if (public_only && !vm_field_is_public(vmf))
			continue;

		array_set_field_ptr(array, index++, vm_field_to_java_lang_reflect_field(vmf, clazz, i));
	}

	return array;
}

jobject java_lang_VMClass_getDeclaredMethods(jobject clazz, jboolean public_only)
{
	struct vm_class *vmc;
	int count;

	vmc = vm_object_to_vm_class(clazz);
	if (!vmc)
		return NULL;

	if (vm_class_is_primitive_class(vmc) || vm_class_is_array_class(vmc))
		return vm_object_alloc_array(vm_array_of_java_lang_reflect_Method, 0);

	count = 0;
	for (int i = 0; i < vmc->class->methods_count; i++) {
		struct vm_method *vmm = &vmc->methods[i];

		if (public_only && !vm_method_is_public(vmm))
			continue;

		if (vm_method_is_special(vmm))
			continue;

		count ++;
	}

	struct vm_object *array
		= vm_object_alloc_array(vm_array_of_java_lang_reflect_Method, count);
	if (!array)
		return rethrow_exception();

	int index = 0;

	for (int i = 0; i < vmc->class->methods_count; i++) {
		struct vm_method *vmm = &vmc->methods[i];

		if (public_only && !vm_method_is_public(vmm))
			continue;

		if (vm_method_is_special(vmm))
			continue;

		array_set_field_ptr(array, index++, vm_method_to_java_lang_reflect_method(vmm, clazz, i));
	}

	return array;
}

jobject java_lang_VMClass_getDeclaringClass(jobject class)
{
	struct vm_class *declaring_class;
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(class);
	if (!vmc)
		return NULL;

	declaring_class	= vmc->declaring_class;
	if (!declaring_class)
		return NULL;

	return declaring_class->object;
}

jobject java_lang_VMClass_getEnclosingClass(jobject class)
{
	struct vm_class *enclosing_class;
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(class);
	if (!vmc)
		return NULL;

	enclosing_class	= vmc->enclosing_class;
	if (!enclosing_class)
		return NULL;

	return enclosing_class->object;
}

jobject java_lang_VMClass_getInterfaces(jobject clazz)
{
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(clazz);
	if (!vmc)
		return NULL;

	struct vm_object *array
		= vm_object_alloc_array(vm_array_of_java_lang_Class,
					vmc->nr_interfaces);
	if (!array)
		return rethrow_exception();

	for (unsigned int i = 0; i < vmc->nr_interfaces; i++) {
		vm_class_ensure_init(vmc->interfaces[i]);
		if (exception_occurred())
			return NULL;

		array_set_field_ptr(array, i, vmc->interfaces[i]->object);
	}

	return array;
}

jint java_lang_VMClass_getModifiers(struct vm_object *clazz, jboolean ignore_inner_classes_attrib)
{
	struct vm_class *class = vm_object_to_vm_class(clazz);
	if (!class)
		return 0;

	if (vm_class_is_member(class) && !ignore_inner_classes_attrib)
		return class->inner_class_access_flags;

	return class->access_flags;
}

struct vm_object *java_lang_VMClass_getName(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return NULL;

	struct vm_object *obj;
	char *dot_name;

	dot_name = slash_to_dots(class->name);
	obj =  vm_object_alloc_string_from_c(dot_name);
	free(dot_name);

	return obj;
}

jobject java_lang_VMClass_getSuperclass(jobject clazz)
{
	struct vm_class *vmc;

	vmc = vm_object_to_vm_class(clazz);
	if (!vmc)
		return NULL;

	if (vm_class_is_array_class(vmc))
		return vm_java_lang_Object->object;

	if (vm_class_is_interface(vmc) ||
	    vm_class_is_primitive_class(vmc) ||
	    vmc == vm_java_lang_Object)
		return NULL;

	vm_class_ensure_object(vmc->super);
	return vmc->super->object;
}

jboolean java_lang_VMClass_isAnonymousClass(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return false;

	return vm_class_is_anonymous(class);
}

jboolean java_lang_VMClass_isArray(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return false;

	return vm_class_is_array_class(class);
}

jboolean java_lang_VMClass_isAssignableFrom(struct vm_object *clazz_1, struct vm_object *clazz_2)
{
	struct vm_class *vmc_1 = vm_object_to_vm_class(clazz_1);
	struct vm_class *vmc_2 = vm_object_to_vm_class(clazz_2);

	if (!vmc_1 || !vmc_2)
		return false;

	return vm_class_is_assignable_from(vmc_1, vmc_2);
}

jboolean java_lang_VMClass_isInstance(struct vm_object *clazz, struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(clazz);

	if (!object || !class)
		return false;

	return vm_class_is_assignable_from(class, object->class);
}

jboolean java_lang_VMClass_isInterface(struct vm_object *clazz)
{
	struct vm_class *class = vm_object_to_vm_class(clazz);
	if (!class)
		return false;

	return vm_class_is_interface(class);
}

jboolean java_lang_VMClass_isLocalClass(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return false;

	return vm_class_is_local(class);
}

jboolean java_lang_VMClass_isMemberClass(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return false;

	return vm_class_is_member(class);
}

jboolean java_lang_VMClass_isPrimitive(struct vm_object *object)
{
	struct vm_class *class = vm_object_to_vm_class(object);

	if (!class)
		return false;

	return vm_class_is_primitive_class(class);
}
