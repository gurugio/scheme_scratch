/*
 * Scheme from Scratch
 * http://peter.michaux.ca/articles/scheme-from-scratch-introduction
 *
 */

#include <stdio.h>
#include <string.h>

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
	switch (obj->type) {
	case OBJTYPE_FIXNUM:
		printf("%ld\n", obj->fixnum_value);
		break;
	default:
		fprintf(stderr, "Unknown type\n");
		break;
	}
}

int main(void)
{

	struct object obj;
	obj.fixnum_value = 1L;
	obj.type = OBJTYPE_FIXNUM;

	/* REPL */
	print(eval(&obj));

	return 0;
}
	
