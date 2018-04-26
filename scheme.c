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
#define MAX_TOKEN 64

enum obj_type {
	OBJTYPE_FIXNUM = 0x1,
	OBJTYPE_BOOLEAN,
	OBJTYPE_CHAR,
	OBJTYPE_STRING,
	OBJTYPE_EMPTYLIST,
	OBJTYPE_PAIR,
	OBJTYPE_SYMBOL,
	/* add new object-type here */
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
		char *symbol_value;
		struct _pair {
			struct object *car;
			struct object *cdr;
		} pair;
	};
	void (*print)(const struct object *obj);
};

/* global objects */
struct object *true_singleton;
struct object *false_singleton;
struct object *emptylist_singleton;

/* symbol list */
struct object *symbol_header;


/*
 * handlers of each type
 */
struct object *make_pair(FILE *in);
void print_pair(const struct object *obj);
struct object *make_char(FILE *in);
void print_char(const struct object *obj);
struct object *make_fixnum(FILE *in);
void print_fixnum(const struct object *obj);
struct object *make_string(FILE *in);
void print_string(const struct object *obj);
struct object *make_symbol(FILE *in);
void print_symbol(const struct object *obj);

void print_boolean(const struct object *obj);
void print_emptylist(const struct object *obj);
void print(struct object *obj);


int isdelimeter(int ch)
{
	return isspace(ch) || (ch == '\n') || (ch == ')') || (ch == ';');
}

int issymbol(int ch)
{
	return isalpha(ch) || (ch == '_');
}

struct object *new_object(enum obj_type type)
{
	struct object *obj;

	if (type >= OBJTYPE_MAX)
		return NULL;

	obj = calloc(1, sizeof(*obj));
	obj->type = type;

	switch (obj->type) {
	case OBJTYPE_FIXNUM:
		obj->print = print_fixnum;
		break;
	case OBJTYPE_BOOLEAN:
		obj->print = print_boolean;
		break;
	case OBJTYPE_CHAR:
		obj->print = print_char;
		break;
	case OBJTYPE_STRING:
		obj->print = print_string;
		break;
	case OBJTYPE_EMPTYLIST:
		obj->print = print_emptylist;
		break;
	case OBJTYPE_PAIR:
		obj->print = print_pair;
		break;
	case OBJTYPE_SYMBOL:
		obj->print = print_symbol;
		break;
	}

	return obj;
}

struct object *cons(struct object *car, struct object *cdr)
{
	struct object *obj = NULL;
	obj = new_object(OBJTYPE_PAIR);
	if (!obj)
		return NULL;

	obj->pair.car = car;
	obj->pair.cdr = cdr;
	return obj;
}

struct object *cdr(struct object *obj)
{
	return obj->pair.cdr;
}

struct object *car(struct object *obj)
{
	return obj->pair.car;
}

struct object *make_char(FILE *in)
{
	struct object *obj = new_object(OBJTYPE_CHAR);
	if (!obj)
		return NULL;
	int ch = fgetc(in);
	int next_ch;

	/* [0]='#', [1]='\' */
	if (isalpha(ch)) {
		next_ch = fgetc(in);
		if (isdelimeter(next_ch)) {
			ungetc(next_ch, in);
			obj->char_value = ch;
		} else if (ch == 'n' && next_ch == 'e') {
			char buf[6];
			int i;
			for (i = 0; i < 6; i++)
				buf[i] = fgetc(in);
			if (buf[0] == 'w' && buf[1] == 'l'
			    && buf[2] == 'i' && buf[3] == 'n'
			    && buf[4] == 'e' && isdelimeter(buf[5]))
				obj->char_value = '\n';
			else
				goto input_error;
		} else if (ch == 's' && next_ch == 'p') {
			char buf[4];
			int i;
			for (i = 0; i < 4; i++)
				buf[i] = fgetc(in);
			if (buf[0] == 'a' && buf[1] == 'c'
			    && buf[2] == 'e' && isdelimeter(buf[3]))
				obj->char_value = ' ';
			else
				goto input_error;
		} else
			goto input_error;
	} else if (ch == '\n' || ch == ' ') {
		obj->char_value = ch;
	} else {
		goto input_error;
	}

	return obj;
input_error:
	free(obj);
	return NULL;
}

struct object *make_fixnum(FILE *in)
{
	char *endptr;
	struct object *obj;
	int ch;
	int buf_index = 0;
	char *buf = calloc(MAX_TOKEN, sizeof(char));
	if (!buf)
		return NULL;

	do {
		ch = fgetc(in);
		if (isdelimeter(ch)) {
			ungetc(ch, in);
			break;
		}
		buf[buf_index++] = (char)ch;
	} while (buf_index < MAX_TOKEN);
	
	obj = new_object(OBJTYPE_FIXNUM);
	if (!obj) {
		free(buf);
		return NULL;
	}

	obj->fixnum_value = strtol(buf, &endptr, 10);
	if (*endptr == '\0') /* entire string is valid */
		return obj;

	free(obj);
	free(buf);
	return NULL;
}

struct object *make_string(FILE *in)
{
	struct object *obj = NULL;
	int ch;
	int buf_index = 0;
	char *buf = calloc(MAX_TOKEN, sizeof(char));
	if (!buf)
		return NULL;

	do {
		ch = fgetc(in);
		/* read everything until '"', ignore (, ; and anything */
		if (ch == '"')
			break;
		buf[buf_index++] = (char)ch;
	} while (buf_index < MAX_TOKEN);

	obj = new_object(OBJTYPE_STRING);
	if (!obj) {
		free(buf);
		return NULL;
	}

	obj->string_value = buf;
	strcpy(obj->string_value, buf);
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

void print_pair(const struct object *obj)
{
	struct object *car, *cdr;

	printf("(");

	do {
		car = obj->pair.car;
		print(car); /* recursive! */

		cdr = obj->pair.cdr;
		if (cdr->type == OBJTYPE_PAIR) {
			printf(" ");
			obj = cdr;
		} else if (cdr->type == OBJTYPE_EMPTYLIST) {
			break;
		} else {
			printf(" . ");
			print(cdr); /* recursive! */
			break;
		}
	} while (1);

	printf(")");
}

void print_string(const struct object *obj)
{
	char *ptr = obj->string_value;

	printf("\"");
	while (*ptr) {
		if (*ptr == '\n')
			printf("\\n");
		else
			printf("%c", *ptr);
		ptr++;
	}
	printf("\"");
}

void print_emptylist(const struct object *obj)
{
	printf("%s", obj->emptylist_value);
}

void print_boolean(const struct object *obj)
{
	if (obj->bool_value == 1)
		printf("#t");
	else if (obj->bool_value == 0)
		printf("#f");
	else
		printf("Error at handling boolean type\n");
}

void print_fixnum(const struct object *obj)
{
	printf("%ld", obj->fixnum_value);
}

struct object *get_emptylist(void)
{
	return emptylist_singleton;
}

struct object *find_symbol(struct object *obj, const char *buf)
{
	if (!obj)
		return NULL;

	if (obj->type != OBJTYPE_PAIR) {
		fprintf(stderr, "Symbol list is broken\n");
		return NULL;
	}

	if (!strcmp(obj->pair.car->symbol_value, buf))
		return obj;

	return find_symbol(cdr(obj), buf);
}

void print_all_symbol(struct object *obj)
{
	if (!obj) return;
	if (obj->type != OBJTYPE_PAIR) {
		fprintf(stderr, "Symbol list is broken\n");
		return;
	}
	print_all_symbol(cdr(obj));
}

struct object *make_symbol(FILE *in)
{
	struct object *obj;
	int ch;
	int buf_index = 0;
	char *buf = calloc(MAX_TOKEN, sizeof(char));
	if (!buf)
		return NULL;

	do {
		ch = fgetc(in);
		if (isdelimeter(ch) || !issymbol(ch)) {
			ungetc(ch, in);
			break;
		}
		buf[buf_index++] = (char)ch;
	} while (buf_index < MAX_TOKEN);

	obj = find_symbol(symbol_header, buf);
	if (obj) {
		free(buf);
		/* find_symbol() returns a pair object */
		return obj->pair.car;
	}

	obj = new_object(OBJTYPE_SYMBOL);
	if (!obj) {
		free(buf);
		return NULL;
	}

	obj->symbol_value = buf;
	symbol_header = cons(obj, symbol_header);
	return obj;
}

void print_symbol(const struct object *obj)
{
	printf("%s", obj->symbol_value);
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

	if (obj->type == OBJTYPE_PAIR
	    && car(obj)->type == OBJTYPE_SYMBOL
	    && !strcmp(car(obj)->symbol_value, "quote")) {
		/* If (quote (1 2 3)), (cadr (quote (1 2 3))) => (1 2 3) */
		print(car(cdr(obj)));
	} else if (obj->print)
		obj->print(obj);
	else
		fprintf(stderr, "Cannot find print call-back, broken object?\n");
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

struct object *get_boolean(const char *token)
{
	if (token[0] == '#' && token[1] == 't')
		return true_singleton;
	else if (token[0] == '#' && token[1] == 'f')
		return false_singleton;
	return NULL;
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

	ch = fgetc(in);

	if (ch == '(') {
		return make_pair(in);
	} else if (ch == '#') {
		/* char or boolean */
		ch = fgetc(in);
		if (ch == 't')
			return true_singleton;
		else if (ch == 'f')
			return false_singleton;
		else if (ch == '\\')
			return make_char(in);
	} else if (ch == '-' || ch == '+' || isdigit(ch)) {
		/* fixnum */
		ungetc(ch, in);
		return make_fixnum(in);
	} else if (ch == '"') {
		/* string */
		return make_string(in);
	} else if (issymbol(ch)) {
		ungetc(ch, in);
		return make_symbol(in);
	} else if (ch == '\'') {
		/* just ignore? */
		return read(in);
	} else
		fprintf(stdout, "Cannot identify input\n");
	/* error */
	return NULL;
}

struct object *make_pair(FILE *in)
{
	int ch;
	struct object *car;
	struct object *cdr;

	/* read the first */
	eat_space(in);

	ch = fgetc(in);
	if (ch == ')')
		return get_emptylist();
	ungetc(ch, in);

	car = read(in);

	eat_space(in);
	ch = fgetc(in);
	if (ch == '.') { /* eg) (1 . 2) or (1 . (2 . 3)) */
		ch = fgetc(in);
		if (!isspace(ch)) {
			fprintf(stderr, "dot must be followed by space\n");
			/* BUGBUG: free car */
			return NULL;
		}

		cdr = read(in);

		ch = fgetc(in);
		if (ch != ')') {
			fprintf(stderr, "make_pair: pair must be ends with )\n");
			goto parse_error;
		}
		return cons(car, cdr);
	} else {
		/* eg) (1 2) or (1 2 3 4)
		 * The car is object and cdr is another list */
		ungetc(ch, in);
		cdr = make_pair(in);
		return cons(car, cdr);
	}
parse_error:
	fprintf(stderr, "Cannot identify this pair\n");
	free(car);
	free(cdr);
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
	
