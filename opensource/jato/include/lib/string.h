#ifndef JATO__LIB_STRING_H
#define JATO__LIB_STRING_H

#include <stdarg.h>

struct string {
	char *value;
	unsigned long length;
	unsigned long capacity;
};

void init_string_intern(void);

struct string *string_from_cstr(char *s);
struct string *string_from_cstr_dup(const char *s);
struct string *string_intern_cstr(const char *s);
struct string *alloc_str(void);
void free_str(struct string *);

int str_resize(struct string *, unsigned long);
int str_printf(struct string *, const char *, ...);
int str_append(struct string *, const char *, ...);
int str_vappend(struct string *, const char *, va_list);

#endif /* JATO__LIB_STRING_H */
