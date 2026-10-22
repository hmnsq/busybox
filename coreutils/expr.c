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

#အကယ်၍ ENABLE_EXPR_MATH_SUPPORT_64
typedef int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#မဟုတ်ပါက
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
} နေစဉ် (0)

/* forward declarations */
static VALUE *eval(void);


/* Return a VALUE for I.  */

static VALUE *int_value(arith_t i)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	အကယ်၍ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ပြန်ပေး v;
}

/* Return a VALUE for S.  */

static VALUE *str_value(const var_na *s)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	အကယ်၍ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ပြန်ပေး v;
}

/* Free VALUE V, including structure components.  */

static void freev(VALUE *v)
{
	အကယ်၍ (v->type == STRING)
		free(v->u.s);
	free(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

static ကိန်းပြည့် null(VALUE *v)
{
	အကယ်၍ (v->type == INTEGER)
		ပြန်ပေး v->u.i == 0;
	/* STRING: */
	ပြန်ပေး v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

static void tostring(VALUE *v)
{
	အကယ်၍ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

static bool toarith(VALUE *v)
{
	အကယ်၍ (v->type == STRING) {
		arith_t i;
		var_na *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		အကယ်၍ ((v->u.s == e) || *e)
			ပြန်ပေး 0;
		free(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ပြန်ပေး 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

static ကိန်းပြည့် nextarg(const var_na *str)
{
	အကယ်၍ (*G.args == NULL || strcmp(*G.args, str) != 0)
		ပြန်ပေး 0;
	ပြန်ပေး (unsigned var_na)str[0] + (unsigned var_na)str[1];
}

/* The comparison operator handling functions.  */

static ကိန်းပြည့် cmp_common(VALUE *l, VALUE *r, ကိန်းပြည့် op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	အကယ်၍ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	အကယ်၍ (op == '<')
		ပြန်ပေး ll < rr;
	အကယ်၍ (op == ('<' + '='))
		ပြန်ပေး ll <= rr;
	အကယ်၍ (op == '=' || (op == '=' + '='))
		ပြန်ပေး ll == rr;
	အကယ်၍ (op == '!' + '=')
		ပြန်ပေး ll != rr;
	အကယ်၍ (op == '>')
		ပြန်ပေး ll > rr;
	/* >= */
	ပြန်ပေး ll >= rr;
}

/* The arithmetic operator handling functions.  */

static arith_t arithmetic_common(VALUE *l, VALUE *r, ကိန်းပြည့် op)
{
	arith_t li, ri;

	အကယ်၍ (!toarith(l) || !toarith(r))
		bb_simple_error_msg_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	အကယ်၍ (op == '+')
		ပြန်ပေး li + ri;
	အကယ်၍ (op == '-')
		ပြန်ပေး li - ri;
	အကယ်၍ (op == '*')
		ပြန်ပေး li * ri;
	အကယ်၍ (ri == 0)
		bb_simple_error_msg_and_die("division by zero");
	အကယ်၍ (op == '/')
		ပြန်ပေး li / ri;
	ပြန်ပေး li % ri;
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

	အကယ်၍ (pv->u.s[0] == '^') {
		bb_error_msg(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, parimaa_na(re_buffer));
	memset(re_regs, 0, parimaa_na(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	အကယ်၍ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		အကယ်၍ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_value(sv->u.s + re_regs[1].rm_so);
		} မဟုတ်ပါက {
			v = int_value(re_regs[0].rm_eo);
		}
	} မဟုတ်ပါက {
		/* Match failed -- return the right kind of null.  */
		အကယ်၍ (re_buffer.re_nsub > 0)
			v = str_value("");
		မဟုတ်ပါက
			v = int_value(0);
	}
	regfree(&re_buffer);
	ပြန်ပေး v;
}

/* Handle bare operands and ( expr ) syntax.  */

static VALUE *eval7(void)
{
	VALUE *v;

	အကယ်၍ (!*G.args)
		bb_simple_error_msg_and_die("syntax error");

	အကယ်၍ (nextarg("(")) {
		G.args++;
		v = eval();
		အကယ်၍ (!nextarg(")"))
			bb_simple_error_msg_and_die("syntax error");
		G.args++;
		ပြန်ပေး v;
	}

	အကယ်၍ (nextarg(")"))
		bb_simple_error_msg_and_die("syntax error");

	ပြန်ပေး str_value(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

static VALUE *eval6(void)
{
	static const var_na keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	VALUE *r, *i1, *i2;
	VALUE *l = l; /* silence gcc */
	VALUE *v = v; /* silence gcc */
	ကိန်းပြည့် key = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	အကယ်၍ (key == 0) /* not a keyword */
		ပြန်ပေး eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	အကယ်၍ (key == 1) { /* quote */
		အကယ်၍ (!*G.args)
			bb_simple_error_msg_and_die("syntax error");
		ပြန်ပေး str_value(*G.args++);
	}
	အကယ်၍ (key == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_value(strlen(r->u.s));
		freev(r);
	} မဟုတ်ပါက
		l = eval6();

	အကယ်၍ (key == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	အကယ်၍ (key == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_value(strcspn(l->u.s, r->u.s) + 1);
		အကယ်၍ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	အကယ်၍ (key == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		အကယ်၍ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_value("");
		မဟုတ်ပါက {
			v = xmalloc(parimaa_na(VALUE));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ပြန်ပေး v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

static VALUE *eval5(void)
{
	VALUE *l, *r, *v;

	l = eval6();
	နေစဉ် (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ပြန်ပေး l;
}

/* Handle *, /, % operators.  */

static VALUE *eval4(void)
{
	VALUE *l, *r;
	ကိန်းပြည့် op;
	arith_t val;

	l = eval5();
	နေစဉ် (1) {
		op = nextarg("*");
		အကယ်၍ (!op) { op = nextarg("/");
		 အကယ်၍ (!op) { op = nextarg("%");
		  အကယ်၍ (!op) ပြန်ပေး l;
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
	ကိန်းပြည့် op;
	arith_t val;

	l = eval4();
	နေစဉ် (1) {
		op = nextarg("+");
		အကယ်၍ (!op) {
			op = nextarg("-");
			အကယ်၍ (!op) ပြန်ပေး l;
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
	ကိန်းပြည့် op;
	arith_t val;

	l = eval3();
	နေစဉ် (1) {
		op = nextarg("<");
		အကယ်၍ (!op) { op = nextarg("<=");
		 အကယ်၍ (!op) { op = nextarg("=");
		  အကယ်၍ (!op) { op = nextarg("==");
		   အကယ်၍ (!op) { op = nextarg("!=");
		    အကယ်၍ (!op) { op = nextarg(">=");
		     အကယ်၍ (!op) { op = nextarg(">");
		      အကယ်၍ (!op) ပြန်ပေး l;
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
	နေစဉ် (nextarg("&")) {
		G.args++;
		r = eval2();
		အကယ်၍ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_value(0);
		} မဟုတ်ပါက
			freev(r);
	}
	ပြန်ပေး l;
}

/* Handle |.  */

static VALUE *eval(void)
{
	VALUE *l, *r;

	l = eval1();
	နေစဉ် (nextarg("|")) {
		G.args++;
		r = eval1();
		အကယ်၍ (null(l)) {
			freev(l);
			l = r;
		} မဟုတ်ပါက
			freev(r);
	}
	ပြန်ပေး l;
}

ကိန်းပြည့် expr_main(ကိန်းပြည့် argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ကိန်းပြည့် expr_main(ကိန်းပြည့် argc UNUSED_PARAM, var_na **argv)
{
	VALUE *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	အကယ်၍ (*G.args == NULL) {
		bb_simple_error_msg_and_die("too few arguments");
	}
	v = eval();
	အကယ်၍ (*G.args)
		bb_simple_error_msg_and_die("syntax error");
	အကယ်၍ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	မဟုတ်ပါက
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
