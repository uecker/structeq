/* algorithm for checking types for structural compatibility
 * 2018, Martin Uecker <martin.uecker@med.uni-goettingen.de>
 */
#include <stdbool.h>
#include <string.h>
#include <assert.h>

struct type {

	enum { INT, POINTER, STRUCT } kind;
	union {
		struct struct_info {

			const char* tag;
			int N;

			struct struct_member {

				const char* name;
				const struct type* type;

			}* members;

		} str_info;

		const struct type* ptr_type;
	} u;
};

struct pair;

static bool is_compat_struct(const struct struct_info* a, const struct struct_info* b, const struct pair* v);

bool is_compatible_r(const struct type* a, const struct type* b, const struct pair* v)
{
	if (a == b)
		return true;

	if (a->kind != b->kind)
		return false;

	switch (a->kind) {
	case INT:
		return true;
	case POINTER:
		return is_compatible_r(a->u.ptr_type, b->u.ptr_type, v);
	case STRUCT:
		return is_compat_struct(&a->u.str_info, &b->u.str_info, v);
	default:
		assert(0);
	}
}

bool is_compatible(const struct type* a, const struct type* b)
{
	return is_compatible_r(a, b, NULL);
}

struct pair {

	const struct struct_info* a;
	const struct struct_info* b;
	const struct pair* link;
};

static bool is_compat_struct(const struct struct_info* a, const struct struct_info* b, const struct pair* v)
{
	const struct pair v2 = { a, b, v };

	// pair seen before -> assume equivalence

	for (; NULL != v; v = v->link)
		if (   ((a == v->a) && (b == v->b))
		    || ((a == v->b) && (b == v->a)))
			return true;

	if (   (a->N != b->N)
	    || (0 != strcmp(a->tag, b->tag)))
		return false;

	for (int i = 0; i < a->N; i++)
		if (   (0 != strcmp(a->members[i].name,
				    b->members[i].name))
		    || !is_compatible_r(a->members[i].type,
					b->members[i].type, &v2))
			return false;

	return true;
}




#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static struct type make_struct(const char* tag, int N, struct struct_member members[N])
{
	return (struct type){

		.kind = STRUCT,
		.u.str_info = {

			.tag = tag,
			.N = N,
			.members = members
		},
	};
}

#define MAKE_STRUCT(t, m) make_struct(t, ARRAY_SIZE(m), m)

#define MAKE_POINTER(t) 			\
	(struct type){	.kind = POINTER,	\
			.u.ptr_type = (t) }



int main()
{
	struct type int_type = { .kind = INT };

	// Example: http://shape-of-code.coding-guidelines.com/2018/05/08/type-compatibility-name-vs-structural-equivalence/

	struct type str1 = MAKE_STRUCT("tag1", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str2 = MAKE_STRUCT("tag2", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));
	struct type str3 = MAKE_STRUCT("tag3", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str4 = MAKE_STRUCT("tag4", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str5 = MAKE_STRUCT("tag5", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));
	struct type str6 = MAKE_STRUCT("tag6", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));

	str1.u.str_info.members[1].type = &MAKE_POINTER(&str2);
	str2.u.str_info.members[1].type = &MAKE_POINTER(&str6);
	str2.u.str_info.members[2].type = &MAKE_POINTER(&str3);
	str3.u.str_info.members[1].type = &MAKE_POINTER(&str1);
	str4.u.str_info.members[1].type = &MAKE_POINTER(&str5);
	str5.u.str_info.members[1].type = &MAKE_POINTER(&str3);
	str5.u.str_info.members[2].type = &MAKE_POINTER(&str6);
	str6.u.str_info.members[1].type = &MAKE_POINTER(&str4);

	assert(is_compatible(&str1, &str1));

	struct type str1b = MAKE_STRUCT("tag1", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str2b = MAKE_STRUCT("tag2", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));
	struct type str3b = MAKE_STRUCT("tag3", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str4b = MAKE_STRUCT("tag4", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));
	struct type str5b = MAKE_STRUCT("tag5", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));
	struct type str6b = MAKE_STRUCT("tag6", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL },
	}));

	str1b.u.str_info.members[1].type = &MAKE_POINTER(&str2b);
	str2b.u.str_info.members[1].type = &MAKE_POINTER(&str6b);
	str2b.u.str_info.members[2].type = &MAKE_POINTER(&str3b);
	str3b.u.str_info.members[1].type = &MAKE_POINTER(&str1b);
	str4b.u.str_info.members[1].type = &MAKE_POINTER(&str5b);
	str5b.u.str_info.members[1].type = &MAKE_POINTER(&str3b);
	str5b.u.str_info.members[2].type = &MAKE_POINTER(&str6b);
	str6b.u.str_info.members[1].type = &MAKE_POINTER(&str4b);

	assert(is_compatible(&str1, &str1b));


	// Example: http://shape-of-code.coding-guidelines.com/2012/01/14/type-compatibility-the-hard-way/

	struct type str1c = MAKE_STRUCT("tag", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));
	struct type str2c = MAKE_STRUCT("tag", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));

	str1c.u.str_info.members[1].type = &MAKE_POINTER(&str1c);
	str1c.u.str_info.members[2].type = &MAKE_POINTER(&str2c);
	str2c.u.str_info.members[1].type = &MAKE_POINTER(&str2c);
	str2c.u.str_info.members[2].type = &MAKE_POINTER(&str1c);

	struct type str1d = MAKE_STRUCT("tag", ((struct struct_member[]){
		{ "a", &int_type }, { "b", NULL }, { "c", NULL },
	}));

	str1d.u.str_info.members[1].type = &MAKE_POINTER(&str1d);
	str1d.u.str_info.members[2].type = &MAKE_POINTER(&str1d);

	assert(is_compatible(&str1c, &str2c));
	assert(is_compatible(&str1c, &str1d));
	assert(is_compatible(&str2c, &str1d));
}

