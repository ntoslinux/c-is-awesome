/*
 * Copyright (c) 2009  Tomasz Grabiec
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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "jit/exception.h"

#include "vm/call.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/object.h"
#include "vm/stack-trace.h"

static void
call_method_a(struct vm_method *method, void *target, unsigned long *args,
	      union jvalue *result)
{
	struct vm_object *exception;

	/*
	 * XXX: We cannot call JIT code with exception signalled
	 * because it will be caught by the nearest exception
	 * test. We must save pending exception and restore it upon
	 * return unless called method signalled exception itself.
	 */
	exception = exception_occurred();
	clear_exception();

	native_call(method, target, args, result);

	if (!exception_occurred() && exception)
		signal_exception(exception);
}

void vm_call_method_v(struct vm_method *method, va_list args,
		      union jvalue *result)
{
	unsigned long args_array[method->args_count];

	assert(!vm_class_is_interface(method->class));

	for (int i = 0; i < method->args_count; i++)
		args_array[i] = va_arg(args, unsigned long);

	void *target = vm_method_call_ptr(method);
	call_method_a(method, target, args_array, result);
}

void vm_call_method_this_a(struct vm_method *method,
			   struct vm_object *this,
			   unsigned long *args,
			   union jvalue *result)
{
	void *target;

	assert(args[0] == (unsigned long) this);

	if (vm_class_is_interface(method->class)) {
		struct vm_method *vmm
			= vm_class_get_method_recursive(this->class, method->name, method->type);
		target = vm_method_call_ptr(vmm);
	} else {
		target = this->class->vtable.native_ptr[method->virtual_index];
	}

	call_method_a(method, target, args, result);
}

void vm_call_method_a(struct vm_method *method, unsigned long *args,
		      union jvalue *result)
{
	assert(!vm_class_is_interface(method->class));
	call_method_a(method, vm_method_call_ptr(method), args, result);
}

void vm_call_method_this_v(struct vm_method *method,
			   struct vm_object *this,
			   va_list args,
			   union jvalue *result)
{
	unsigned long args_array[method->args_count];

	args_array[0] = (unsigned long) this;

	for (int i = 1; i < method->args_count; i++)
		args_array[i] = va_arg(args, unsigned long);

	vm_call_method_this_a(method, this, args_array, result);
}
