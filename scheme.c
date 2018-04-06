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
	struct object *obj = new_object(OBJTYPE_FIXNUM);
	if (!obj)
		return NULL;

	obj->fixnum_value = strtol(buf, NULL, 10);
	return obj;
}

struct object *eval(struct object *exp)
{
	return exp;
}

void print(struct object *obj)
{
	if (!obj) return;

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

	return make_fixnum(line_buf);
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
	
