
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int
parse_command_line_option(const char *cmdline, const char *key, char **value)
{
	int status;
	char *ptr, *start_val;

	*value = NULL;
	ptr = strstr(cmdline, key);
	if (ptr == NULL) {
		status = -1;
		goto exit;
	}

	ptr += strlen(key) + 1;
	start_val = ptr;
	while (*ptr != 0 && !isspace(*ptr)) {
		ptr++;
	}

	if (ptr - start_val > 0) {
		*value = calloc(ptr - start_val + 1, sizeof(char));
		strncpy(*value, start_val, ptr - start_val);
	}
exit:
	return status;
}


int main()
{
	int status;
	char *p;

	status = parse_command_line_option("abc -d asdfasd", "-d", &p);
	if (status >= 0)
	return 0;
}



