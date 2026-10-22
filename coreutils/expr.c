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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"
#অন্তর্ভাব "xregex.h"

#যদি ENABLE_রাশিমালা_MATH_SUPPORT_64
প্রকারলেখ int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#অন্যথা
প্রকারলেখ long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
গণনা {
	INTEGER,
	STRING
};

/* A value is.... */
কাঠামো valinfo {
	smallint type;                  /* Which kind. */
	সংঘ {                         /* The value itself. */
		arith_t i;
		অক্ষর *s;
	} u;
};
প্রকারলেখ কাঠামো valinfo মান;

/* The arguments given to the program, minus the program name.  */
কাঠামো globals {
	অক্ষর **args;
} FIX_ALIASING;
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} যখন (0)

/* forward declarations */
স্থির মান *eval(শূন্য);


/* Return a VALUE for I.  */

স্থির মান *int_মান(arith_t i)
{
	মান *v;

	v = xzalloc(আকার(মান));
	যদি (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ফেরত v;
}

/* Return a VALUE for S.  */

স্থির মান *str_মান(ধ্রুবক অক্ষর *s)
{
	মান *v;

	v = xzalloc(আকার(মান));
	যদি (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ফেরত v;
}

/* Free VALUE V, including structure components.  */

স্থির শূন্য freev(মান *v)
{
	যদি (v->type == STRING)
		মুক্ত(v->u.s);
	মুক্ত(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

স্থির পূর্ণসংখ্যা null(মান *v)
{
	যদি (v->type == INTEGER)
		ফেরত v->u.i == 0;
	/* STRING: */
	ফেরত v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

স্থির শূন্য tostring(মান *v)
{
	যদি (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

স্থির bool toarith(মান *v)
{
	যদি (v->type == STRING) {
		arith_t i;
		অক্ষর *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		যদি ((v->u.s == e) || *e)
			ফেরত 0;
		মুক্ত(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ফেরত 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

স্থির পূর্ণসংখ্যা nextarg(ধ্রুবক অক্ষর *str)
{
	যদি (*G.args == ফাঁকা || strcmp(*G.args, str) != 0)
		ফেরত 0;
	ফেরত (unsigned অক্ষর)str[0] + (unsigned অক্ষর)str[1];
}

/* The comparison operator handling functions.  */

স্থির পূর্ণসংখ্যা cmp_common(মান *l, মান *r, পূর্ণসংখ্যা op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	যদি (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	যদি (op == '<')
		ফেরত ll < rr;
	যদি (op == ('<' + '='))
		ফেরত ll <= rr;
	যদি (op == '=' || (op == '=' + '='))
		ফেরত ll == rr;
	যদি (op == '!' + '=')
		ফেরত ll != rr;
	যদি (op == '>')
		ফেরত ll > rr;
	/* >= */
	ফেরত ll >= rr;
}

/* The arithmetic operator handling functions.  */

স্থির arith_t arithmetic_common(মান *l, মান *r, পূর্ণসংখ্যা op)
{
	arith_t li, ri;

	যদি (!toarith(l) || !toarith(r))
		bb_simple_error_বার্তা_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	যদি (op == '+')
		ফেরত li + ri;
	যদি (op == '-')
		ফেরত li - ri;
	যদি (op == '*')
		ফেরত li * ri;
	যদি (ri == 0)
		bb_simple_error_বার্তা_and_die("division by zero");
	যদি (op == '/')
		ফেরত li / ri;
	ফেরত li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

স্থির মান *docolon(মান *sv, মান *pv)
{
	গণনা { NMATCH = 2 };
	মান *v;
	regex_t re_বাফার;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	যদি (pv->u.s[0] == '^') {
		bb_error_বার্তা(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_বাফার, 0, আকার(re_বাফার));
	memset(re_regs, 0, আকার(re_regs));
	xregcomp(&re_বাফার, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	যদি (regexec(&re_বাফার, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		যদি (re_বাফার.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_মান(sv->u.s + re_regs[1].rm_so);
		} অন্যথা {
			v = int_মান(re_regs[0].rm_eo);
		}
	} অন্যথা {
		/* Match failed -- return the right kind of null.  */
		যদি (re_বাফার.re_nsub > 0)
			v = str_মান("");
		অন্যথা
			v = int_মান(0);
	}
	regfree(&re_বাফার);
	ফেরত v;
}

/* Handle bare operands and ( expr ) syntax.  */

স্থির মান *eval7(শূন্য)
{
	মান *v;

	যদি (!*G.args)
		bb_simple_error_বার্তা_and_die("syntax error");

	যদি (nextarg("(")) {
		G.args++;
		v = eval();
		যদি (!nextarg(")"))
			bb_simple_error_বার্তা_and_die("syntax error");
		G.args++;
		ফেরত v;
	}

	যদি (nextarg(")"))
		bb_simple_error_বার্তা_and_die("syntax error");

	ফেরত str_মান(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

স্থির মান *eval6(শূন্য)
{
	স্থির ধ্রুবক অক্ষর keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	মান *r, *i1, *i2;
	মান *l = l; /* silence gcc */
	মান *v = v; /* silence gcc */
	পূর্ণসংখ্যা চাবি = *G.args ? সূচক_in_strings(keywords, *G.args) + 1 : 0;

	যদি (চাবি == 0) /* not a keyword */
		ফেরত eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	যদি (চাবি == 1) { /* quote */
		যদি (!*G.args)
			bb_simple_error_বার্তা_and_die("syntax error");
		ফেরত str_মান(*G.args++);
	}
	যদি (চাবি == 2) { /*দৈর্ঘ্য*/
		r = eval6();
		tostring(r);
		v = int_মান(strlen(r->u.s));
		freev(r);
	} অন্যথা
		l = eval6();

	যদি (চাবি == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	যদি (চাবি == 4) { /*সূচক*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_মান(strcspn(l->u.s, r->u.s) + 1);
		যদি (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	যদি (চাবি == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		যদি (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_মান("");
		অন্যথা {
			v = xmalloc(আকার(মান));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ফেরত v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

স্থির মান *eval5(শূন্য)
{
	মান *l, *r, *v;

	l = eval6();
	যখন (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ফেরত l;
}

/* Handle *, /, % operators.  */

স্থির মান *eval4(শূন্য)
{
	মান *l, *r;
	পূর্ণসংখ্যা op;
	arith_t val;

	l = eval5();
	যখন (1) {
		op = nextarg("*");
		যদি (!op) { op = nextarg("/");
		 যদি (!op) { op = nextarg("%");
		  যদি (!op) ফেরত l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_মান(val);
	}
}

/* Handle +, - operators.  */

স্থির মান *eval3(শূন্য)
{
	মান *l, *r;
	পূর্ণসংখ্যা op;
	arith_t val;

	l = eval4();
	যখন (1) {
		op = nextarg("+");
		যদি (!op) {
			op = nextarg("-");
			যদি (!op) ফেরত l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_মান(val);
	}
}

/* Handle comparisons.  */

স্থির মান *eval2(শূন্য)
{
	মান *l, *r;
	পূর্ণসংখ্যা op;
	arith_t val;

	l = eval3();
	যখন (1) {
		op = nextarg("<");
		যদি (!op) { op = nextarg("<=");
		 যদি (!op) { op = nextarg("=");
		  যদি (!op) { op = nextarg("==");
		   যদি (!op) { op = nextarg("!=");
		    যদি (!op) { op = nextarg(">=");
		     যদি (!op) { op = nextarg(">");
		      যদি (!op) ফেরত l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_মান(val);
	}
}

/* Handle &.  */

স্থির মান *eval1(শূন্য)
{
	মান *l, *r;

	l = eval2();
	যখন (nextarg("&")) {
		G.args++;
		r = eval2();
		যদি (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_মান(0);
		} অন্যথা
			freev(r);
	}
	ফেরত l;
}

/* Handle |.  */

স্থির মান *eval(শূন্য)
{
	মান *l, *r;

	l = eval1();
	যখন (nextarg("|")) {
		G.args++;
		r = eval1();
		যদি (null(l)) {
			freev(l);
			l = r;
		} অন্যথা
			freev(r);
	}
	ফেরত l;
}

পূর্ণসংখ্যা রাশিমালা_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা রাশিমালা_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	মান *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	যদি (*G.args == ফাঁকা) {
		bb_simple_error_বার্তা_and_die("too few arguments");
	}
	v = eval();
	যদি (*G.args)
		bb_simple_error_বার্তা_and_die("syntax error");
	যদি (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	অন্যথা
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
