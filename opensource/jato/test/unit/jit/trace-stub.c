#include <stdbool.h>

#include "arch/stack-frame.h"

struct vm_method;
struct compilation_unit;

bool opt_trace_invoke;
bool opt_trace_exceptions;
bool opt_trace_bytecode;
bool opt_trace_invoke_verbose;

void trace_invoke(struct compilation_unit *cu)
{
}

void trace_exception(struct compilation_unit *cu, struct jit_stack_frame *frame,
		     unsigned char *native_ptr)
{
}

void trace_exception_handler(unsigned char *addr)
{
}

void trace_exception_unwind(struct jit_stack_frame *frame)
{
}

void trace_exception_unwind_to_native(struct jit_stack_frame *frame)
{
}

void trace_bytecode(struct vm_method *method)
{
}

void trace_return_value(struct vm_method *vmm, unsigned long value)
{
}

void print_method(struct vm_method *vmm)
{
}
