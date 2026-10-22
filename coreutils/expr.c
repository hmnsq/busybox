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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"
#ضمن "xregex.h"

#ܐܢ ENABLE_EXPR_MATH_SUPPORT_64
تعريفنوع int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#ܐܚܪܢܐ
تعريفنوع long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
تعداد {
	INTEGER,
	STRING
};

/* A value is.... */
بنية valinfo {
	smallint type;                  /* Which kind. */
	اتحاد {                         /* The value itself. */
		arith_t i;
		حرف *s;
	} u;
};
تعريفنوع بنية valinfo VALUE;

/* The arguments given to the program, minus the program name.  */
بنية globals {
	حرف **args;
} FIX_ALIASING;
#define G (*(بنية globals*)bb_common_bufsiz1)
#define INIT_G() ܥܒܕ { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ܟܡܐ (0)

/* forward declarations */
ساكن VALUE *eval(فراغ);


/* Return a VALUE for I.  */

ساكن VALUE *int_value(arith_t i)
{
	VALUE *v;

	v = xzalloc(حجم(VALUE));
	ܐܢ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ܦܢܐ v;
}

/* Return a VALUE for S.  */

ساكن VALUE *str_value(ثابت حرف *s)
{
	VALUE *v;

	v = xzalloc(حجم(VALUE));
	ܐܢ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ܦܢܐ v;
}

/* Free VALUE V, including structure components.  */

ساكن فراغ freev(VALUE *v)
{
	ܐܢ (v->type == STRING)
		حرر(v->u.s);
	حرر(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ساكن صحيح null(VALUE *v)
{
	ܐܢ (v->type == INTEGER)
		ܦܢܐ v->u.i == 0;
	/* STRING: */
	ܦܢܐ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ساكن فراغ tostring(VALUE *v)
{
	ܐܢ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ساكن bool toarith(VALUE *v)
{
	ܐܢ (v->type == STRING) {
		arith_t i;
		حرف *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ܐܢ ((v->u.s == e) || *e)
			ܦܢܐ 0;
		حرر(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ܦܢܐ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ساكن صحيح nextarg(ثابت حرف *str)
{
	ܐܢ (*G.args == عدم || strcmp(*G.args, str) != 0)
		ܦܢܐ 0;
	ܦܢܐ (unsigned حرف)str[0] + (unsigned حرف)str[1];
}

/* The comparison operator handling functions.  */

ساكن صحيح cmp_common(VALUE *l, VALUE *r, صحيح op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ܐܢ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ܐܢ (op == '<')
		ܦܢܐ ll < rr;
	ܐܢ (op == ('<' + '='))
		ܦܢܐ ll <= rr;
	ܐܢ (op == '=' || (op == '=' + '='))
		ܦܢܐ ll == rr;
	ܐܢ (op == '!' + '=')
		ܦܢܐ ll != rr;
	ܐܢ (op == '>')
		ܦܢܐ ll > rr;
	/* >= */
	ܦܢܐ ll >= rr;
}

/* The arithmetic operator handling functions.  */

ساكن arith_t arithmetic_common(VALUE *l, VALUE *r, صحيح op)
{
	arith_t li, ri;

	ܐܢ (!toarith(l) || !toarith(r))
		bb_simple_error_msg_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ܐܢ (op == '+')
		ܦܢܐ li + ri;
	ܐܢ (op == '-')
		ܦܢܐ li - ri;
	ܐܢ (op == '*')
		ܦܢܐ li * ri;
	ܐܢ (ri == 0)
		bb_simple_error_msg_and_die("division by zero");
	ܐܢ (op == '/')
		ܦܢܐ li / ri;
	ܦܢܐ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ساكن VALUE *docolon(VALUE *sv, VALUE *pv)
{
	تعداد { NMATCH = 2 };
	VALUE *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ܐܢ (pv->u.s[0] == '^') {
		bb_error_msg(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, حجم(re_buffer));
	memset(re_regs, 0, حجم(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ܐܢ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ܐܢ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_value(sv->u.s + re_regs[1].rm_so);
		} ܐܚܪܢܐ {
			v = int_value(re_regs[0].rm_eo);
		}
	} ܐܚܪܢܐ {
		/* Match failed -- return the right kind of null.  */
		ܐܢ (re_buffer.re_nsub > 0)
			v = str_value("");
		ܐܚܪܢܐ
			v = int_value(0);
	}
	regfree(&re_buffer);
	ܦܢܐ v;
}

/* Handle bare operands and ( expr ) syntax.  */

ساكن VALUE *eval7(فراغ)
{
	VALUE *v;

	ܐܢ (!*G.args)
		bb_simple_error_msg_and_die("syntax error");

	ܐܢ (nextarg("(")) {
		G.args++;
		v = eval();
		ܐܢ (!nextarg(")"))
			bb_simple_error_msg_and_die("syntax error");
		G.args++;
		ܦܢܐ v;
	}

	ܐܢ (nextarg(")"))
		bb_simple_error_msg_and_die("syntax error");

	ܦܢܐ str_value(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ساكن VALUE *eval6(فراغ)
{
	ساكن ثابت حرف keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	VALUE *r, *i1, *i2;
	VALUE *l = l; /* silence gcc */
	VALUE *v = v; /* silence gcc */
	صحيح key = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	ܐܢ (key == 0) /* not a keyword */
		ܦܢܐ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	ܐܢ (key == 1) { /* quote */
		ܐܢ (!*G.args)
			bb_simple_error_msg_and_die("syntax error");
		ܦܢܐ str_value(*G.args++);
	}
	ܐܢ (key == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_value(strlen(r->u.s));
		freev(r);
	} ܐܚܪܢܐ
		l = eval6();

	ܐܢ (key == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ܐܢ (key == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_value(strcspn(l->u.s, r->u.s) + 1);
		ܐܢ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ܐܢ (key == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ܐܢ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_value("");
		ܐܚܪܢܐ {
			v = xmalloc(حجم(VALUE));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ܦܢܐ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ساكن VALUE *eval5(فراغ)
{
	VALUE *l, *r, *v;

	l = eval6();
	ܟܡܐ (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ܦܢܐ l;
}

/* Handle *, /, % operators.  */

ساكن VALUE *eval4(فراغ)
{
	VALUE *l, *r;
	صحيح op;
	arith_t val;

	l = eval5();
	ܟܡܐ (1) {
		op = nextarg("*");
		ܐܢ (!op) { op = nextarg("/");
		 ܐܢ (!op) { op = nextarg("%");
		  ܐܢ (!op) ܦܢܐ l;
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

ساكن VALUE *eval3(فراغ)
{
	VALUE *l, *r;
	صحيح op;
	arith_t val;

	l = eval4();
	ܟܡܐ (1) {
		op = nextarg("+");
		ܐܢ (!op) {
			op = nextarg("-");
			ܐܢ (!op) ܦܢܐ l;
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

ساكن VALUE *eval2(فراغ)
{
	VALUE *l, *r;
	صحيح op;
	arith_t val;

	l = eval3();
	ܟܡܐ (1) {
		op = nextarg("<");
		ܐܢ (!op) { op = nextarg("<=");
		 ܐܢ (!op) { op = nextarg("=");
		  ܐܢ (!op) { op = nextarg("==");
		   ܐܢ (!op) { op = nextarg("!=");
		    ܐܢ (!op) { op = nextarg(">=");
		     ܐܢ (!op) { op = nextarg(">");
		      ܐܢ (!op) ܦܢܐ l;
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

ساكن VALUE *eval1(فراغ)
{
	VALUE *l, *r;

	l = eval2();
	ܟܡܐ (nextarg("&")) {
		G.args++;
		r = eval2();
		ܐܢ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_value(0);
		} ܐܚܪܢܐ
			freev(r);
	}
	ܦܢܐ l;
}

/* Handle |.  */

ساكن VALUE *eval(فراغ)
{
	VALUE *l, *r;

	l = eval1();
	ܟܡܐ (nextarg("|")) {
		G.args++;
		r = eval1();
		ܐܢ (null(l)) {
			freev(l);
			l = r;
		} ܐܚܪܢܐ
			freev(r);
	}
	ܦܢܐ l;
}

صحيح expr_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح expr_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	VALUE *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	ܐܢ (*G.args == عدم) {
		bb_simple_error_msg_and_die("too few arguments");
	}
	v = eval();
	ܐܢ (*G.args)
		bb_simple_error_msg_and_die("syntax error");
	ܐܢ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ܐܚܪܢܐ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
