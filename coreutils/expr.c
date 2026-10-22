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

#ប្រសិនបើ ENABLE_EXPR_MATH_SUPPORT_64
typedef int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#បើមិនដូច្នេះ
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
} ខណៈពេល (0)

/* forward declarations */
static VALUE *eval(void);


/* Return a VALUE for I.  */

static VALUE *int_value(arith_t i)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	ប្រសិនបើ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ត្រឡប់ v;
}

/* Return a VALUE for S.  */

static VALUE *str_value(const var_na *s)
{
	VALUE *v;

	v = xzalloc(parimaa_na(VALUE));
	ប្រសិនបើ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ត្រឡប់ v;
}

/* Free VALUE V, including structure components.  */

static void freev(VALUE *v)
{
	ប្រសិនបើ (v->type == STRING)
		free(v->u.s);
	free(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

static ចំនួនគត់ null(VALUE *v)
{
	ប្រសិនបើ (v->type == INTEGER)
		ត្រឡប់ v->u.i == 0;
	/* STRING: */
	ត្រឡប់ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

static void tostring(VALUE *v)
{
	ប្រសិនបើ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

static bool toarith(VALUE *v)
{
	ប្រសិនបើ (v->type == STRING) {
		arith_t i;
		var_na *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ប្រសិនបើ ((v->u.s == e) || *e)
			ត្រឡប់ 0;
		free(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ត្រឡប់ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

static ចំនួនគត់ nextarg(const var_na *str)
{
	ប្រសិនបើ (*G.args == NULL || strcmp(*G.args, str) != 0)
		ត្រឡប់ 0;
	ត្រឡប់ (unsigned var_na)str[0] + (unsigned var_na)str[1];
}

/* The comparison operator handling functions.  */

static ចំនួនគត់ cmp_common(VALUE *l, VALUE *r, ចំនួនគត់ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ប្រសិនបើ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ប្រសិនបើ (op == '<')
		ត្រឡប់ ll < rr;
	ប្រសិនបើ (op == ('<' + '='))
		ត្រឡប់ ll <= rr;
	ប្រសិនបើ (op == '=' || (op == '=' + '='))
		ត្រឡប់ ll == rr;
	ប្រសិនបើ (op == '!' + '=')
		ត្រឡប់ ll != rr;
	ប្រសិនបើ (op == '>')
		ត្រឡប់ ll > rr;
	/* >= */
	ត្រឡប់ ll >= rr;
}

/* The arithmetic operator handling functions.  */

static arith_t arithmetic_common(VALUE *l, VALUE *r, ចំនួនគត់ op)
{
	arith_t li, ri;

	ប្រសិនបើ (!toarith(l) || !toarith(r))
		bb_simple_error_msg_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ប្រសិនបើ (op == '+')
		ត្រឡប់ li + ri;
	ប្រសិនបើ (op == '-')
		ត្រឡប់ li - ri;
	ប្រសិនបើ (op == '*')
		ត្រឡប់ li * ri;
	ប្រសិនបើ (ri == 0)
		bb_simple_error_msg_and_die("division by zero");
	ប្រសិនបើ (op == '/')
		ត្រឡប់ li / ri;
	ត្រឡប់ li % ri;
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

	ប្រសិនបើ (pv->u.s[0] == '^') {
		bb_error_msg(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, parimaa_na(re_buffer));
	memset(re_regs, 0, parimaa_na(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ប្រសិនបើ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ប្រសិនបើ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_value(sv->u.s + re_regs[1].rm_so);
		} បើមិនដូច្នេះ {
			v = int_value(re_regs[0].rm_eo);
		}
	} បើមិនដូច្នេះ {
		/* Match failed -- return the right kind of null.  */
		ប្រសិនបើ (re_buffer.re_nsub > 0)
			v = str_value("");
		បើមិនដូច្នេះ
			v = int_value(0);
	}
	regfree(&re_buffer);
	ត្រឡប់ v;
}

/* Handle bare operands and ( expr ) syntax.  */

static VALUE *eval7(void)
{
	VALUE *v;

	ប្រសិនបើ (!*G.args)
		bb_simple_error_msg_and_die("syntax error");

	ប្រសិនបើ (nextarg("(")) {
		G.args++;
		v = eval();
		ប្រសិនបើ (!nextarg(")"))
			bb_simple_error_msg_and_die("syntax error");
		G.args++;
		ត្រឡប់ v;
	}

	ប្រសិនបើ (nextarg(")"))
		bb_simple_error_msg_and_die("syntax error");

	ត្រឡប់ str_value(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

static VALUE *eval6(void)
{
	static const var_na keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	VALUE *r, *i1, *i2;
	VALUE *l = l; /* silence gcc */
	VALUE *v = v; /* silence gcc */
	ចំនួនគត់ key = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	ប្រសិនបើ (key == 0) /* not a keyword */
		ត្រឡប់ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	ប្រសិនបើ (key == 1) { /* quote */
		ប្រសិនបើ (!*G.args)
			bb_simple_error_msg_and_die("syntax error");
		ត្រឡប់ str_value(*G.args++);
	}
	ប្រសិនបើ (key == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_value(strlen(r->u.s));
		freev(r);
	} បើមិនដូច្នេះ
		l = eval6();

	ប្រសិនបើ (key == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ប្រសិនបើ (key == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_value(strcspn(l->u.s, r->u.s) + 1);
		ប្រសិនបើ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ប្រសិនបើ (key == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ប្រសិនបើ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_value("");
		បើមិនដូច្នេះ {
			v = xmalloc(parimaa_na(VALUE));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ត្រឡប់ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

static VALUE *eval5(void)
{
	VALUE *l, *r, *v;

	l = eval6();
	ខណៈពេល (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ត្រឡប់ l;
}

/* Handle *, /, % operators.  */

static VALUE *eval4(void)
{
	VALUE *l, *r;
	ចំនួនគត់ op;
	arith_t val;

	l = eval5();
	ខណៈពេល (1) {
		op = nextarg("*");
		ប្រសិនបើ (!op) { op = nextarg("/");
		 ប្រសិនបើ (!op) { op = nextarg("%");
		  ប្រសិនបើ (!op) ត្រឡប់ l;
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
	ចំនួនគត់ op;
	arith_t val;

	l = eval4();
	ខណៈពេល (1) {
		op = nextarg("+");
		ប្រសិនបើ (!op) {
			op = nextarg("-");
			ប្រសិនបើ (!op) ត្រឡប់ l;
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
	ចំនួនគត់ op;
	arith_t val;

	l = eval3();
	ខណៈពេល (1) {
		op = nextarg("<");
		ប្រសិនបើ (!op) { op = nextarg("<=");
		 ប្រសិនបើ (!op) { op = nextarg("=");
		  ប្រសិនបើ (!op) { op = nextarg("==");
		   ប្រសិនបើ (!op) { op = nextarg("!=");
		    ប្រសិនបើ (!op) { op = nextarg(">=");
		     ប្រសិនបើ (!op) { op = nextarg(">");
		      ប្រសិនបើ (!op) ត្រឡប់ l;
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
	ខណៈពេល (nextarg("&")) {
		G.args++;
		r = eval2();
		ប្រសិនបើ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_value(0);
		} បើមិនដូច្នេះ
			freev(r);
	}
	ត្រឡប់ l;
}

/* Handle |.  */

static VALUE *eval(void)
{
	VALUE *l, *r;

	l = eval1();
	ខណៈពេល (nextarg("|")) {
		G.args++;
		r = eval1();
		ប្រសិនបើ (null(l)) {
			freev(l);
			l = r;
		} បើមិនដូច្នេះ
			freev(r);
	}
	ត្រឡប់ l;
}

ចំនួនគត់ expr_main(ចំនួនគត់ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ចំនួនគត់ expr_main(ចំនួនគត់ argc UNUSED_PARAM, var_na **argv)
{
	VALUE *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	ប្រសិនបើ (*G.args == NULL) {
		bb_simple_error_msg_and_die("too few arguments");
	}
	v = eval();
	ប្រសិនបើ (*G.args)
		bb_simple_error_msg_and_die("syntax error");
	ប្រសិនបើ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	បើមិនដូច្នេះ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
