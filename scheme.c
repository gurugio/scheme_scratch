/*
 * Scheme from Scratch
 * http://peter.michaux.ca/articles/scheme-from-scratch-introduction
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE 128


enum obj_type {
	OBJTYPE_FIXNUM = 0x1,
	OBJTYPE_MAX,
};

struct object {
	enum obj_type type;
	union {
		long fixnum_value;
	};
};


struct object *new_object(enum obj_type type)
{
	struct object *obj;

	if (type >= OBJTYPE_MAX)
		return NULL;

	obj = calloc(1, sizeof(*obj));
	obj->type = type;
	return obj;
}

struct object *make_fixnum(char *buf)
{
	char *endptr;
	struct object *obj = new_object(OBJTYPE_FIXNUM);
	if (!obj)
		return NULL;

	obj->fixnum_value = strtol(buf, &endptr, 10);
	if (*endptr == '\0') /* entire string is valid */
		return obj;
	else
		return NULL;
}

struct object *eval(struct object *exp)
{
	return exp;
}

void print(struct object *obj)
{
	if (!obj) {
		fprintf(stderr, "Failed to create object\n");
		return;
	}

	switch (obj->type) {
	case OBJTYPE_FIXNUM:
		printf("%ld\n", obj->fixnum_value);
		break;
	default:
		fprintf(stderr, "Unknown type\n");
		break;
	}
}

void eat_line(FILE *in)
{
	while (fgetc(in) != '\n')
		continue;
}

/* eat spaces and comment */
void eat_space(FILE *in)
{
	int ch;
	int max = 0;

	do {
		ch = fgetc(in);
		if (isspace(ch))
			continue;
		else if (ch == ';') {
			eat_line(in);
			break;
		}

		ungetc(ch, in);
		break;
	} while (max++ < MAX_LINE);
	return;
}

enum obj_type get_type(const char *token)
{
	if (token[0] == '#') {
		if (token[1] == 't') {
			/* return true singleton object */
		} else if (token[1] == 'f') {
			/* return false singleton object */
		} else {
			/* error */
		}
		return OBJTYPE_MAX;
	} else if (token[0] == '-' || token[0] == '+' || isdigit(token[0])) {
		/* fixnum */
		return OBJTYPE_FIXNUM;
	}
	/* unknown type yet */
	return OBJTYPE_MAX;
}

struct object *read(FILE *in)
{
	int ch;
	char line_buf[MAX_LINE];
	int line_index = 0;
	int max = 0;
	enum obj_type type = OBJTYPE_MAX;

	eat_space(in);

	do {
		ch = fgetc(in);

		if (ch == ';') {
			eat_line(in);
			break;
		} else if (ch == '\n') {
			break;
		}

		line_buf[line_index++] = (char)ch;
	} while (max++ < MAX_LINE);
	line_buf[line_index] = 0;

	while (isspace(line_buf[--line_index])) {
		line_buf[line_index] = 0;
	}

	switch (get_type(line_buf)) {
	case OBJTYPE_FIXNUM:
		return make_fixnum(line_buf);
	}

	return NULL;
}

int main(void)
{
	printf("Start scheme repl\n");
	
	/* REPL */
	while (1) {
		printf("> ");
		print(eval(read(stdin)));
		printf("\n");
	}

	return 0;
}
	
