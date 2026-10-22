/* vi: set sw=4 ts=4: */
/*
 * Mini expr implementation for busybox
 *
 * based on GNU expr Mike Parker.
 * Copyright (C) 86, 1991-1997, 1999 Free Software Foundation, Inc.
 *
 * Busybox modifications
 * Copyright (c) 2000  Edward Betts <edward@debian.org>.
 * Copyright (C) 2003-2005  Vladimir Oleynik <dzo@simtreas.ru>
 *  - reduced 464 bytes.
 *  - 64 math support
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* This program evaluates expressions.  Each token (operator, operand,
 * parenthesis) of the expression must be a separate argument.  The
 * parser used is a reasonably general one, though any incarnation of
 * it is language-specific.  It is especially nice for expressions.
 *
 * No parse tree is needed; a new node is evaluated immediately.
 * One function can handle multiple operators all of equal precedence,
 * provided they all associate ((x op x) op x).
 */
//config:config EXPR
//config:	bool "expr (6.8 kb)"
//config:	default y
//config:	help
//config:	expr is used to calculate numbers and print the result
//config:	to standard output.
//config:
//config:config EXPR_MATH_SUPPORT_64
//config:	bool "Extend Posix numbers support to 64 bit"
//config:	default y
//config:	depends on EXPR
//config:	help
//config:	Enable 64-bit math support in the expr applet. This will make
//config:	the applet slightly larger, but will allow computation with very
//config:	large numbers.

//applet:IF_EXPR(APPLET_NOEXEC(expr, expr, BB_DIR_USR_BIN, BB_SUID_DROP, expr))

//kbuild:lib-$(CONFIG_EXPR) += expr.o

//usage:#define expr_trivial_usage
//usage:       "EXPRESSION"
//usage:#define expr_full_usage "\n\n"
//usage:       "Print the value of EXPRESSION\n"
//usage:    "\n"
//usage:       "EXPRESSION may be:\n"
//usage:       "	ARG1 | ARG2	ARG1 if it is neither null nor 0, otherwise ARG2\n"
//usage:       "	ARG1 & ARG2	ARG1 if neither argument is null or 0, otherwise 0\n"
//usage:       "	ARG1 < ARG2	1 if ARG1 is less than ARG2, else 0. Similarly:\n"
//usage:       "	ARG1 <= ARG2\n"
//usage:       "	ARG1 = ARG2\n"
//usage:       "	ARG1 != ARG2\n"
//usage:       "	ARG1 >= ARG2\n"
//usage:       "	ARG1 > ARG2\n"
//usage:       "	ARG1 + ARG2	Sum of ARG1 and ARG2. Similarly:\n"
//usage:       "	ARG1 - ARG2\n"
//usage:       "	ARG1 * ARG2\n"
//usage:       "	ARG1 / ARG2\n"
//usage:       "	ARG1 % ARG2\n"
//usage:       "	STRING : REGEXP		Anchored pattern match of REGEXP in STRING\n"
//usage:       "	match STRING REGEXP	Same as STRING : REGEXP\n"
//usage:       "	substr STRING POS LEN	Substring of STRING, POS counts from 1\n"
//usage:       "	index STRING CHARS	Index in STRING where any CHARS is found, or 0\n"
//usage:       "	length STRING		Length of STRING\n"
//usage:       "	quote TOKEN		Interpret TOKEN as a string, even if\n"
//usage:       "				it is a keyword like 'match' or an\n"
//usage:       "				operator like '/'\n"
//usage:       "	(EXPRESSION)		Value of EXPRESSION\n"
//usage:       "\n"
//usage:       "Beware that many operators need to be escaped or quoted for shells.\n"
//usage:       "Comparisons are arithmetic if both ARGs are numbers, else\n"
//usage:       "lexicographical. Pattern matches return the string matched between\n"
//usage:       "\\( and \\) or null; if \\( and \\) are not used, they return the number\n"
//usage:       "of characters matched or 0."

#include "libbb.h"
#include "common_bufsiz.h"
#include "xregex.h"

#ຖ້າ ENABLE_EXPR_MATH_SUPPORT_64
typedef int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#ບໍ່ດັ່ງນັ້ນ
typedef long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ga_nanaa {
	INTEGER,
	STRING
};

/* A value is.... */
sa_nracanaa valinfo {
	smallint type;                  /* Which kind. */
	sa_ngha {                         /* The value itself. */
		arith_t i;
		var_na *s;
	} u;
};
typedef sa_nracanaa valinfo VALUE;

/* The arguments given to the program, minus the program name.  */
sa_nracanaa globals {
	var_na **args;
} FIX_ALIASING;
#define G (*(sa_nracanaa globals*)bb_common_bufsiz1)
#define INIT_G() do { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ໃນຂະນະທີ່ (0)

/* forward declarations */
static VALUE *eval(void);


/* Return a VALUE for I.  */

static VALUE *int_value(arith_t i)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	ຖ້າ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ສົ່ງຄືນ v;
}

/* Return a VALUE for S.  */

static VALUE *str_value(const var_na *s)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	ຖ້າ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ສົ່ງຄືນ v;
}

/* Free VALUE V, including structure components.  */

static void freev(VALUE *v)
{
	ຖ້າ (v->type == STRING)
		free(v->u.s);
	free(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

static ຈຳນວນເຕັມ null(VALUE *v)
{
	ຖ້າ (v->type == INTEGER)
		ສົ່ງຄືນ v->u.i == 0;
	/* STRING: */
	ສົ່ງຄືນ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

static void tostring(VALUE *v)
{
	ຖ້າ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

static bool toarith(VALUE *v)
{
	ຖ້າ (v->type == STRING) {
		arith_t i;
		var_na *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ຖ້າ ((v->u.s == e) || *e)
			ສົ່ງຄືນ 0;
		free(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ສົ່ງຄືນ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

static ຈຳນວນເຕັມ nextarg(const var_na *str)
{
	ຖ້າ (*G.args == NULL || strcmp(*G.args, str) != 0)
		ສົ່ງຄືນ 0;
	ສົ່ງຄືນ (unsigned var_na)str[0] + (unsigned var_na)str[1];
}

/* The comparison operator handling functions.  */

static ຈຳນວນເຕັມ cmp_common(VALUE *l, VALUE *r, ຈຳນວນເຕັມ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ຖ້າ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ຖ້າ (op == '<')
		ສົ່ງຄືນ ll < rr;
	ຖ້າ (op == ('<' + '='))
		ສົ່ງຄືນ ll <= rr;
	ຖ້າ (op == '=' || (op == '=' + '='))
		ສົ່ງຄືນ ll == rr;
	ຖ້າ (op == '!' + '=')
		ສົ່ງຄືນ ll != rr;
	ຖ້າ (op == '>')
		ສົ່ງຄືນ ll > rr;
	/* >= */
	ສົ່ງຄືນ ll >= rr;
}

/* The arithmetic operator handling functions.  */

static arith_t arithmetic_common(VALUE *l, VALUE *r, ຈຳນວນເຕັມ op)
{
	arith_t li, ri;

	ຖ້າ (!toarith(l) || !toarith(r))
		bb_simple_error_msg_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ຖ້າ (op == '+')
		ສົ່ງຄືນ li + ri;
	ຖ້າ (op == '-')
		ສົ່ງຄືນ li - ri;
	ຖ້າ (op == '*')
		ສົ່ງຄືນ li * ri;
	ຖ້າ (ri == 0)
		bb_simple_error_msg_and_die("division by zero");
	ຖ້າ (op == '/')
		ສົ່ງຄືນ li / ri;
	ສົ່ງຄືນ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

static VALUE *docolon(VALUE *sv, VALUE *pv)
{
	ga_nanaa { NMATCH = 2 };
	VALUE *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ຖ້າ (pv->u.s[0] == '^') {
		bb_error_msg(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, parimaa_na(re_buffer));
	memset(re_regs, 0, parimaa_na(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ຖ້າ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ຖ້າ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_value(sv->u.s + re_regs[1].rm_so);
		} ບໍ່ດັ່ງນັ້ນ {
			v = int_value(re_regs[0].rm_eo);
		}
	} ບໍ່ດັ່ງນັ້ນ {
		/* Match failed -- return the right kind of null.  */
		ຖ້າ (re_buffer.re_nsub > 0)
			v = str_value("");
		ບໍ່ດັ່ງນັ້ນ
			v = int_value(0);
	}
	regfree(&re_buffer);
	ສົ່ງຄືນ v;
}

/* Handle bare operands and ( expr ) syntax.  */

static VALUE *eval7(void)
{
	VALUE *v;

	ຖ້າ (!*G.args)
		bb_simple_error_msg_and_die("syntax error");

	ຖ້າ (nextarg("(")) {
		G.args++;
		v = eval();
		ຖ້າ (!nextarg(")"))
			bb_simple_error_msg_and_die("syntax error");
		G.args++;
		ສົ່ງຄືນ v;
	}

	ຖ້າ (nextarg(")"))
		bb_simple_error_msg_and_die("syntax error");

	ສົ່ງຄືນ str_value(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

static VALUE *eval6(void)
{
	static const var_na keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	VALUE *r, *i1, *i2;
	VALUE *l = l; /* silence gcc */
	VALUE *v = v; /* silence gcc */
	ຈຳນວນເຕັມ key = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	ຖ້າ (key == 0) /* not a keyword */
		ສົ່ງຄືນ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	ຖ້າ (key == 1) { /* quote */
		ຖ້າ (!*G.args)
			bb_simple_error_msg_and_die("syntax error");
		ສົ່ງຄືນ str_value(*G.args++);
	}
	ຖ້າ (key == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_value(strlen(r->u.s));
		freev(r);
	} ບໍ່ດັ່ງນັ້ນ
		l = eval6();

	ຖ້າ (key == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ຖ້າ (key == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_value(strcspn(l->u.s, r->u.s) + 1);
		ຖ້າ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ຖ້າ (key == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ຖ້າ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_value("");
		ບໍ່ດັ່ງນັ້ນ {
			v = xmalloc(parimaa_na(VALUE));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ສົ່ງຄືນ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

static VALUE *eval5(void)
{
	VALUE *l, *r, *v;

	l = eval6();
	ໃນຂະນະທີ່ (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ສົ່ງຄືນ l;
}

/* Handle *, /, % operators.  */

static VALUE *eval4(void)
{
	VALUE *l, *r;
	ຈຳນວນເຕັມ op;
	arith_t val;

	l = eval5();
	ໃນຂະນະທີ່ (1) {
		op = nextarg("*");
		ຖ້າ (!op) { op = nextarg("/");
		 ຖ້າ (!op) { op = nextarg("%");
		  ຖ້າ (!op) ສົ່ງຄືນ l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_value(val);
	}
}

/* Handle +, - operators.  */

static VALUE *eval3(void)
{
	VALUE *l, *r;
	ຈຳນວນເຕັມ op;
	arith_t val;

	l = eval4();
	ໃນຂະນະທີ່ (1) {
		op = nextarg("+");
		ຖ້າ (!op) {
			op = nextarg("-");
			ຖ້າ (!op) ສົ່ງຄືນ l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_value(val);
	}
}

/* Handle comparisons.  */

static VALUE *eval2(void)
{
	VALUE *l, *r;
	ຈຳນວນເຕັມ op;
	arith_t val;

	l = eval3();
	ໃນຂະນະທີ່ (1) {
		op = nextarg("<");
		ຖ້າ (!op) { op = nextarg("<=");
		 ຖ້າ (!op) { op = nextarg("=");
		  ຖ້າ (!op) { op = nextarg("==");
		   ຖ້າ (!op) { op = nextarg("!=");
		    ຖ້າ (!op) { op = nextarg(">=");
		     ຖ້າ (!op) { op = nextarg(">");
		      ຖ້າ (!op) ສົ່ງຄືນ l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_value(val);
	}
}

/* Handle &.  */

static VALUE *eval1(void)
{
	VALUE *l, *r;

	l = eval2();
	ໃນຂະນະທີ່ (nextarg("&")) {
		G.args++;
		r = eval2();
		ຖ້າ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_value(0);
		} ບໍ່ດັ່ງນັ້ນ
			freev(r);
	}
	ສົ່ງຄືນ l;
}

/* Handle |.  */

static VALUE *eval(void)
{
	VALUE *l, *r;

	l = eval1();
	ໃນຂະນະທີ່ (nextarg("|")) {
		G.args++;
		r = eval1();
		ຖ້າ (null(l)) {
			freev(l);
			l = r;
		} ບໍ່ດັ່ງນັ້ນ
			freev(r);
	}
	ສົ່ງຄືນ l;
}

ຈຳນວນເຕັມ expr_main(ຈຳນວນເຕັມ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ຈຳນວນເຕັມ expr_main(ຈຳນວນເຕັມ argc UNUSED_PARAM, var_na **argv)
{
	VALUE *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	ຖ້າ (*G.args == NULL) {
		bb_simple_error_msg_and_die("too few arguments");
	}
	v = eval();
	ຖ້າ (*G.args)
		bb_simple_error_msg_and_die("syntax error");
	ຖ້າ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ບໍ່ດັ່ງນັ້ນ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
