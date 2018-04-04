/*
 * Scheme from Scratch
 * http://peter.michaux.ca/articles/scheme-from-scratch-introduction
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 128


enum obj_type {
	OBJTYPE_FIXNUM = 0x1,
};

struct object {
	enum obj_type type;
	union {
		long fixnum_value;
	};
};


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
	int negative = 0;
	char line_buf[MAX_LINE];
	int line_index = 0;
	int max = 0;
	
	eat_space(in);

	ch = fgetc(in);
	if (ch == '-')
		negative = 1;
	else
		ungetc(ch, in);

	do {
		ch = fgetc(in);

		if (ch == ';') {
			eat_line(in);
			break;
		} else if (ch == '\n')
			break;

		printf("get ch=%c\n", (char)ch);
		line_buf[line_index++] = (char)ch;
	} while (max++ < MAX_LINE);
	
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
	
