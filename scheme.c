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
	OBJTYPE_BOOLEAN,
	OBJTYPE_CHAR,
	OBJTYPE_STRING,
	OBJTYPE_EMPTYLIST,
	OBJTYPE_MAX,
};

struct object {
	enum obj_type type;
	union {
		long fixnum_value;
		int bool_value;
		char char_value;
		char *string_value;
		char emptylist_value[3];
	};
};


struct object *true_singleton;
struct object *false_singleton;
struct object *emptylist_singleton;

struct object *new_object(enum obj_type type)
{
	struct object *obj;

	if (type >= OBJTYPE_MAX)
		return NULL;

	obj = calloc(1, sizeof(*obj));
	obj->type = type;
	return obj;
}

struct object *make_fixnum(const char *buf)
{
	char *endptr;
	struct object *obj = new_object(OBJTYPE_FIXNUM);
	if (!obj)
		return NULL;

	obj->fixnum_value = strtol(buf, &endptr, 10);
	if (*endptr == '\0') /* entire string is valid */
		return obj;

	free(obj);
	return NULL;
}

struct object *make_char(const char *buf)
{
	struct object *obj = NULL;
	/* [0]='#', [1]='\' */
	if (isalpha(buf[2])) {
		obj = new_object(OBJTYPE_CHAR);
		if (!obj)
			return NULL;
		if (!strncmp(&buf[2], "newline", 8 /* newline + null */)) {
			obj->char_value = '\n';
		} else if (!strncmp(&buf[2], "space", 6 /* space + null */)) {
			obj->char_value = ' ';
		} else if (buf[3] == '\0') {
			obj->char_value = buf[2];
		} else {
			free(obj);
			obj = NULL;
		}
	} else if (buf[2] == '\n' || buf[2] == ' ') {
		obj = new_object(OBJTYPE_CHAR);
		if (!obj)
			return NULL;
		obj->char_value = buf[2];
	}
	return obj;
}

void print_char(const struct object *obj)
{
	if (obj->char_value == ' ')
		printf("#\\space");
	else if (obj->char_value == '\n')
		printf("#\\newline");
	else
		printf("#\\%c", (char)obj->char_value);
}

void read_string(FILE *in, char *line_buf, int *line_index)
{
	int max = 0;
	int ch;
	do {
		ch = fgetc(in);
		line_buf[*line_index] = (char)ch;
		(*line_index)++;

		if (ch == '\\') { /* escape-sequence */
			ch = fgetc(in);
			line_buf[*line_index] = (char)ch;
			(*line_index)++;
			continue;
		} else if (ch == '"')
			break;
	} while (max++ < MAX_LINE);
}

struct object *make_string(const char *buf)
{
	struct object *obj = NULL;

	obj = new_object(OBJTYPE_STRING);
	if (!obj)
		return NULL;

	obj->string_value = calloc(strlen(buf) + 1, sizeof(char)); /* +1 for null */
	strcpy(obj->string_value, buf);
	return obj;
}

void print_string(struct object *obj)
{
	char *ptr = obj->string_value;

	while (*ptr) {
		if (*ptr == '\n')
			printf("\\n");
		else
			printf("%c", *ptr);
		ptr++;
	}
}

void print_emptylist(struct object *obj)
{
	printf("%s", obj->emptylist_value);
}

struct object *get_emptylist(void)
{
	return emptylist_singleton;
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
		printf("%ld", obj->fixnum_value);
		break;
	case OBJTYPE_BOOLEAN:
		if (obj->bool_value == 1)
			printf("#t");
		else if (obj->bool_value == 0)
			printf("#f");
		else
			printf("Error at handling boolean type\n");
		break;
	case OBJTYPE_CHAR:
		print_char(obj);
		break;
	case OBJTYPE_STRING:
		print_string(obj);
		break;
	case OBJTYPE_EMPTYLIST:
		print_emptylist(obj);
		break;
	default:
		fprintf(stderr, "Cannot print the unknown type value\n");
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
	enum obj_type t = OBJTYPE_MAX;

	if (token[0] == '#' &&
	    (token[1] == 't' || token[1] == 'f')) {
		t = OBJTYPE_BOOLEAN;
	} else if (token[0] == '-' || token[0] == '+' || isdigit(token[0])) {
		t = OBJTYPE_FIXNUM;
	} else if (token[0] == '#' && token[1] == '\\') {
		t = OBJTYPE_CHAR;
	} else if (token[0] == '"' && token[strlen(token) - 1] == '"') {
		t =  OBJTYPE_STRING;
	} else if (token[0] == '(' && token[1] == ')') {
		t = OBJTYPE_EMPTYLIST;
	}
	return t;
}

struct object *get_boolean(const char *token)
{
	if (token[0] == '#' && token[1] == 't')
		return true_singleton;
	else if (token[0] == '#' && token[1] == 'f')
		return false_singleton;
	return NULL;
}

int isdelimeter(int ch)
{
	return isspace(ch) || (ch == '\n');
}

/*
 * read one expression from stdin
 * A expression starts with '#', number, ", (
 * and ends with space, newline, )
 * boolean: #t, #f
 * character: #\a
 * integer: 123, -123
 * string: "asdf"
 * empty list: ()
 */
struct object *read(FILE *in)
{
	int ch;
	char line_buf[MAX_LINE];
	int line_index = 0;
	int max = 0;
	enum obj_type type = OBJTYPE_MAX;
	int escape = 0;

	eat_space(in);

	do {
		ch = fgetc(in);

		if (ch == ';') {
			eat_line(in);
			break;
		} else if (ch == '\\') {
			/* Next character of '\' would be space or newline.
			 * do not check and just store
			 */
			line_buf[line_index++] = (char)ch;
			ch = fgetc(in);
			line_buf[line_index++] = (char)ch;
			continue;
		} else if (isdelimeter(ch)) {
			break;
		}

		line_buf[line_index++] = (char)ch;
	} while (max++ < MAX_LINE);
	line_buf[line_index] = 0;

	switch (get_type(line_buf)) {
	case OBJTYPE_FIXNUM:
		return make_fixnum(line_buf);
	case OBJTYPE_BOOLEAN:
		return get_boolean(line_buf);
	case OBJTYPE_CHAR:
		return make_char(line_buf);
	case OBJTYPE_STRING:
		return make_string(line_buf);
	case OBJTYPE_EMPTYLIST:
		return get_emptylist();
	case OBJTYPE_MAX:
		fprintf(stderr, "Unknown type\n");
	}

	return NULL;
}

void model_layer_init(void)
{
	true_singleton = new_object(OBJTYPE_BOOLEAN);
	true_singleton->bool_value = 1;
	false_singleton = new_object(OBJTYPE_BOOLEAN);
	false_singleton->bool_value = 0;
	emptylist_singleton = new_object(OBJTYPE_EMPTYLIST);
	strcpy(emptylist_singleton->emptylist_value, "()");
}

int main(void)
{
	printf("Start scheme repl\n");

	model_layer_init();
	
	/* REPL */
	while (1) {
		printf("> ");
		print(eval(read(stdin)));
		printf("\n");
	}

	return 0;
}
	
