/*
 * Copyright (c) 2008  Pekka Enberg
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

#include "jit/vtable.h"
#include "jit/compilation-unit.h"
#include "vm/class.h"
#include <stdlib.h>

void vtable_init(struct vtable *vtable, unsigned int nr_methods)
{
	vtable->native_ptr = calloc(nr_methods, sizeof(void *));
}

void vtable_release(struct vtable *vtable)
{
	free(vtable->native_ptr);
}

void vtable_setup_method(struct vtable *vtable, unsigned long idx, void *native_ptr)
{
	vtable->native_ptr[idx] = native_ptr;
}

/**
 * This function replaces pointers in vtable so that they point
 * directly to compiled code instead of trampoline code.
 */
void fixup_vtable(struct compilation_unit *cu, struct vm_object *this,
		  void *target)
{
	struct vm_class *vmc = this->class;
	struct vm_method *vmm = cu->method;
	int index = vmm->virtual_index;

	/*
	 * A method can be invoked by invokevirtual and invokespecial. For
	 * example, a public method p() in class A is normally invoked with
	 * invokevirtual but if a class B that extends A calls that
	 * method by "super.p()" we use invokespecial instead.
	 *
	 * We must not fixup vtable entry in the class of this when this method
	 * was invoked by invokespecial.
	 */
        if (vmc->vtable.native_ptr[index] == vm_method_trampoline_ptr(vmm))
		vmc->vtable.native_ptr[index] = target;

	/* Fixup the vtable entry in declaring class */
	vmm->class->vtable.native_ptr[index] = target;
}
