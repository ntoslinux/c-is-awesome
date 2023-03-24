#ifndef __VM_NATIVES_H
#define __VM_NATIVES_H

#include <stdbool.h>

struct vm_native {
	const char *	class_name;
	const char *	method_name;
	void *		ptr;
};

#define DEFINE_NATIVE(_class_name, _method_name, _ptr) \
	{ .class_name = _class_name, .method_name = _method_name, .ptr = _ptr }

int vm_register_native(struct vm_native *native);
void vm_unregister_natives(void);
void *vm_lookup_native(const char *, const char *);

#endif
