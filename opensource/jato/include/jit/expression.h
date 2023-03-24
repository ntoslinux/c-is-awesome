#ifndef __EXPRESSION_H
#define __EXPRESSION_H

#include "vm/class.h"
#include "vm/field.h"
#include "vm/method.h"
#include "vm/system.h"
#include "vm/types.h"
#include "vm/vm.h"

#include "jit/tree-node.h"

#include "arch/instruction.h"

struct lookupswitch;
struct parse_context;

enum expression_type {
	EXPR_VALUE,
	EXPR_FLOAT_LOCAL,
	EXPR_FLOAT_TEMPORARY,
	EXPR_FLOAT_CLASS_FIELD,
	EXPR_FLOAT_INSTANCE_FIELD,
	EXPR_FVALUE,
	EXPR_LOCAL,
	EXPR_TEMPORARY,
	EXPR_ARRAY_DEREF,
	EXPR_BINOP,
	EXPR_UNARY_OP,
	EXPR_CONVERSION,
	EXPR_CONVERSION_FLOAT_TO_DOUBLE,
	EXPR_CONVERSION_DOUBLE_TO_FLOAT,
	EXPR_CONVERSION_FROM_FLOAT,
	EXPR_CONVERSION_TO_FLOAT,
	EXPR_CONVERSION_FROM_DOUBLE,
	EXPR_CONVERSION_TO_DOUBLE,
	EXPR_CLASS_FIELD,
	EXPR_INSTANCE_FIELD,
	EXPR_ARGS_LIST,
	EXPR_ARG,
	EXPR_ARG_THIS,
	EXPR_NO_ARGS,
	EXPR_NEW,
	EXPR_NEWARRAY,
	EXPR_ANEWARRAY,
	EXPR_MULTIANEWARRAY,
	EXPR_ARRAYLENGTH,
	EXPR_INSTANCEOF,
	EXPR_EXCEPTION_REF,
	EXPR_NULL_CHECK,
	EXPR_ARRAY_SIZE_CHECK,
	EXPR_MIMIC_STACK_SLOT,
	EXPR_LOOKUPSWITCH_BSEARCH,
	EXPR_TRUNCATION,
	EXPR_LAST,	/* Not a real type. Keep this last. */
};

enum binary_operator {
	OP_ADD = EXPR_LAST,
	OP_SUB,
	OP_MUL,
	OP_MUL_64,
	OP_DIV,
	OP_DIV_64,
	OP_REM,
	OP_REM_64,

	OP_SHL,
	OP_SHL_64,
	OP_SHR,
	OP_SHR_64,
	OP_USHR,
	OP_USHR_64,
	OP_AND,
	OP_OR,
	OP_XOR,

	OP_CMP,
	OP_CMPL,
	OP_CMPG,

	OP_EQ,
	OP_NE,
	OP_LT,
	OP_GE,
	OP_GT,
	OP_LE,

	OP_FADD,
	OP_FSUB,
	OP_FMUL,
	OP_FDIV,
	OP_FREM,

	OP_DADD,
	OP_DSUB,
	OP_DMUL,
	OP_DDIV,
	OP_DREM,

	BINOP_LAST,	/* Not a real operator. Keep this last. */
};

enum unary_operator {
	OP_NEG	= BINOP_LAST,
	OP_FNEG,
	OP_DNEG,
	OP_LAST,	/* Not a real operator. Keep this last. */
};

struct expression {
	unsigned long refcount;
	enum vm_type vm_type;
	unsigned long bytecode_offset;

	union {
		struct tree_node node;

		/*  EXPR_VALUE represents an integer or reference constant
		    expression (see JLS 15.28.). This expression type can be
		    used as an rvalue only.  */
		unsigned long long value;

		/*  EXPR_FVALUE represents a floating-point constant
		    expression (see JLS 15.28.). This expression type can be
		    used as an rvalue only.  */
		double fvalue;

		/*  EXPR_LOCAL represents a local variable or parameter
		    expression (see JLS 15.14. and 6.5.6.). This expression
		    type can be used as both lvalue and rvalue.  */
		unsigned long local_index;

		/*  EXPR_TEMPORARY represents a compiler-generated temporary
		    expression. This expression type can be used as both
		    lvalue and rvalue. */
		struct {
#ifdef CONFIG_32_BIT
			struct var_info *tmp_high;
#endif
			struct var_info *tmp_low;
		};

		/*  EXPR_ARRAY_DEREF represents an array access expression
		    (see JLS 15.13.). This expression type can be used as
		    both lvalue and rvalue.  */
		struct {
			struct tree_node *arrayref;
			struct tree_node *array_index;
		};

		/*  EXPR_BINOP represents an binary operation expression (see
		    JLS 15.17., 15.18., 15.19., 15.20., 15.21., 15.22.). This
		    expression type can be used as an rvalue only.  */
		struct {
			struct tree_node *binary_left;
			struct tree_node *binary_right;
		};

		/*  EXPR_UNARY_OP represents an unary operation expression
		    (see JLS 15.15.). This expression type can be used as an
		    rvalue only.  */
		struct {
			struct tree_node *unary_expression;
		};

		/*  EXPR_CONVERSION represents a type conversion (see JLS
		    5.1.).  This expression type can be used as an rvalue
		    only.  */
		/* EXPR_TRUNCATION represents conversion from J_INT to
		   J_CHAR, J_BYTE or J_SHORT. */
		struct {
			struct tree_node *from_expression;
			enum vm_type to_type;			/* EXPR_TRUNCATION only */
		};

		/*  EXPR_CLASS_FIELD represents class field access expression
		    (see JLS 15.11.). This expression type can be used as
		    either as lvalue or rvalue.  */
		struct {
			struct vm_field *class_field;
		};

		/*  EXPR_INSTANCE_FIELD represents instance field access
		    expression (see JLS 15.11.). This expression type can be
		    used as either as lvalue or rvalue.  */
		struct {
			struct tree_node *objectref_expression;
			struct vm_field *instance_field;
		};

		/*  EXPR_ARGS_LIST represents list of arguments passed to
		    method. This expression does not evaluate to a value and
		    is used for instruction selection only.  */
		struct {
			struct tree_node *args_left;
			struct tree_node *args_right;
		};

		/*  EXPR_ARG represents an argument passed to
		    method. This expression does not evaluate to a
		    value and is used for instruction selection
		    only. The same fields are used by EXPR_ARG_THIS
		    which represents the object reference on which the
		    invocation is done. It evaluates to J_REFERENCE
		    holding the object pointer.  */
		struct {
			struct tree_node *arg_expression;
			enum machine_reg arg_reg;
		};

		/*  EXPR_NO_ARGS is used for STMT_INVOKE expression type when
		    there are no arguments to pass.  */
		struct {
			/* Nothing. */
		};

		/*  EXPR_NEW represents creation of a new instance that is
		    unitialized .  */
		struct {
			struct vm_class *class;
		};

		/*  EXPR_NEWARRAY represents creation of a new array.  */
		struct {
			struct tree_node *array_size;
			unsigned long array_type;	/* T_INT, T_LONG, etc. */
		};

		/*  EXPR_ANEWARRAY represents creation of a new array of given
		   reference.  */
		struct {
		        struct tree_node *anewarray_size;
			struct vm_class *anewarray_ref_type;
		};

		/*  EXPR_MULTIANEWARRAY represents creation of a new multidimensional
		    array of given reference.  */
		struct {
			struct tree_node *multianewarray_dimensions;
			struct vm_class *multianewarray_ref_type;
		};

		/*  EXPR_ARRAYLENGTH represents length of an array.  */
		struct {
			struct tree_node *arraylength_ref;
		};

		/*  EXPR_INSTANCEOF is used to determine if object is of given type.  */
		struct {
			struct tree_node *instanceof_ref;
			struct vm_class *instanceof_class;
		};

		/* EXPR_NULL_CHECK is used to assure that NullPointerException
		   will be thrown if reference is null. */
		struct tree_node *null_check_ref;

		/* EXPR_ARRAY_SIZE_CHECK */
		struct tree_node *size_expr;

		/* EXPR_MIMIC_STACK_SLOT */
		struct {
			char entry;
			int slot_ndx;
		};

		/* EXPR_LOOKUPSWITCH_BSEARCH */
		struct  {
			struct tree_node *key;
			struct lookupswitch *lookupswitch_table;
		};
	};
};

static inline struct expression *to_expr(struct tree_node *node)
{
	return container_of(node, struct expression, node);
}

static inline enum expression_type expr_type(struct expression *expr)
{
	return (expr->node.op & EXPR_TYPE_MASK) >> EXPR_TYPE_SHIFT;
}

static inline void expr_set_type(struct expression *expr, int type)
{
	unsigned long op = expr->node.op & ~EXPR_TYPE_MASK;
	type <<= EXPR_TYPE_SHIFT;
	expr->node.op = op | type;
}

static inline enum binary_operator expr_bin_op(struct expression *expr)
{
	return (expr->node.op & OP_MASK) >> OP_SHIFT;
}

static inline enum unary_operator expr_unary_op(struct expression *expr)
{
	return (expr->node.op & OP_MASK) >> OP_SHIFT;
}

struct expression *alloc_expression(enum expression_type, enum vm_type);
void free_expression(struct expression *);

struct expression *expr_get(struct expression *);
void expr_put(struct expression *);

struct expression *value_expr(enum vm_type, unsigned long long);
struct expression *fvalue_expr(enum vm_type, double);
struct expression *local_expr(enum vm_type, unsigned long);
struct expression *temporary_expr(enum vm_type, struct compilation_unit *);
struct expression *mimic_stack_expr(enum vm_type, int, int);
struct expression *array_deref_expr(enum vm_type, struct expression *, struct expression *);
struct expression *binop_expr(enum vm_type, enum binary_operator, struct expression *, struct expression *);
struct expression *unary_op_expr(enum vm_type, enum unary_operator, struct expression *);
struct expression *conversion_expr(enum vm_type, struct expression *);
struct expression *conversion_double_to_float_expr(struct expression *);
struct expression *conversion_float_to_double_expr(struct expression *);
struct expression *conversion_from_float_expr(enum vm_type, struct expression *);
struct expression *conversion_to_float_expr(enum vm_type, struct expression *);
struct expression *conversion_from_double_expr(enum vm_type, struct expression *);
struct expression *conversion_to_double_expr(enum vm_type, struct expression *);

struct expression *class_field_expr(enum vm_type, struct vm_field *);
struct expression *instance_field_expr(enum vm_type, struct vm_field *, struct expression *);
struct expression *args_list_expr(struct expression *, struct expression *);
struct expression *arg_expr(struct expression *);
struct expression *arg_this_expr(struct expression *);
struct expression *no_args_expr(void);
struct expression *new_expr(struct vm_class *);
struct expression *newarray_expr(unsigned long, struct expression *);
struct expression *anewarray_expr(struct vm_class *, struct expression *);
struct expression *multianewarray_expr(struct vm_class *);
struct expression *arraylength_expr(struct expression *);
struct expression *instanceof_expr(struct expression *, struct vm_class *);
struct expression *exception_ref_expr(void);
struct expression *null_check_expr(struct expression *);
struct expression *array_size_check_expr(struct expression *);
struct expression *dup_expr(struct parse_context *, struct expression *);
struct expression *get_pure_expr(struct parse_context *, struct expression *);
struct expression *lookupswitch_bsearch_expr(struct expression *, struct lookupswitch *);
struct expression *truncation_expr(enum vm_type, struct expression *);
unsigned long nr_args(struct expression *);
int expr_nr_kids(struct expression *);
int expr_is_pure(struct expression *);

#endif
