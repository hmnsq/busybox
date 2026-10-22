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

#شامل "libbb.h"
#شامل "common_bufsiz.h"
#شامل "xregex.h"

#اگر ENABLE_عبارت_MATH_SUPPORT_64
تعریفنوع int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#وگرنه
تعریفنوع long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
شمارش {
	INTEGER,
	STRING
};

/* A value is.... */
ساختار valinfo {
	smallint type;                  /* Which kind. */
	اجتماع {                         /* The value itself. */
		arith_t i;
		نویسه *s;
	} u;
};
تعریفنوع ساختار valinfo مقدار;

/* The arguments given to the program, minus the program name.  */
ساختار globals {
	نویسه **args;
} FIX_ALIASING;
#define G (*(ساختار globals*)bb_common_bufsiz1)
#define INIT_G() انجامبده { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} وقتی (0)

/* forward declarations */
ایستا مقدار *eval(تهی);


/* Return a VALUE for I.  */

ایستا مقدار *int_مقدار(arith_t i)
{
	مقدار *v;

	v = xzalloc(اندازه(مقدار));
	اگر (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	بازگشت v;
}

/* Return a VALUE for S.  */

ایستا مقدار *str_مقدار(ثابت نویسه *s)
{
	مقدار *v;

	v = xzalloc(اندازه(مقدار));
	اگر (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	بازگشت v;
}

/* Free VALUE V, including structure components.  */

ایستا تهی freev(مقدار *v)
{
	اگر (v->type == STRING)
		آزاد(v->u.s);
	آزاد(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ایستا صحیح null(مقدار *v)
{
	اگر (v->type == INTEGER)
		بازگشت v->u.i == 0;
	/* STRING: */
	بازگشت v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ایستا تهی tostring(مقدار *v)
{
	اگر (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ایستا bool toarith(مقدار *v)
{
	اگر (v->type == STRING) {
		arith_t i;
		نویسه *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		اگر ((v->u.s == e) || *e)
			بازگشت 0;
		آزاد(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	بازگشت 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ایستا صحیح nextarg(ثابت نویسه *str)
{
	اگر (*G.args == پوچ || strcmp(*G.args, str) != 0)
		بازگشت 0;
	بازگشت (unsigned نویسه)str[0] + (unsigned نویسه)str[1];
}

/* The comparison operator handling functions.  */

ایستا صحیح cmp_common(مقدار *l, مقدار *r, صحیح op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	اگر (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	اگر (op == '<')
		بازگشت ll < rr;
	اگر (op == ('<' + '='))
		بازگشت ll <= rr;
	اگر (op == '=' || (op == '=' + '='))
		بازگشت ll == rr;
	اگر (op == '!' + '=')
		بازگشت ll != rr;
	اگر (op == '>')
		بازگشت ll > rr;
	/* >= */
	بازگشت ll >= rr;
}

/* The arithmetic operator handling functions.  */

ایستا arith_t arithmetic_common(مقدار *l, مقدار *r, صحیح op)
{
	arith_t li, ri;

	اگر (!toarith(l) || !toarith(r))
		bb_simple_error_پیام_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	اگر (op == '+')
		بازگشت li + ri;
	اگر (op == '-')
		بازگشت li - ri;
	اگر (op == '*')
		بازگشت li * ri;
	اگر (ri == 0)
		bb_simple_error_پیام_and_die("division by zero");
	اگر (op == '/')
		بازگشت li / ri;
	بازگشت li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ایستا مقدار *docolon(مقدار *sv, مقدار *pv)
{
	شمارش { NMATCH = 2 };
	مقدار *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	اگر (pv->u.s[0] == '^') {
		bb_error_پیام(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, اندازه(re_buffer));
	memset(re_regs, 0, اندازه(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	اگر (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		اگر (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_مقدار(sv->u.s + re_regs[1].rm_so);
		} وگرنه {
			v = int_مقدار(re_regs[0].rm_eo);
		}
	} وگرنه {
		/* Match failed -- return the right kind of null.  */
		اگر (re_buffer.re_nsub > 0)
			v = str_مقدار("");
		وگرنه
			v = int_مقدار(0);
	}
	regfree(&re_buffer);
	بازگشت v;
}

/* Handle bare operands and ( expr ) syntax.  */

ایستا مقدار *eval7(تهی)
{
	مقدار *v;

	اگر (!*G.args)
		bb_simple_error_پیام_and_die("syntax error");

	اگر (nextarg("(")) {
		G.args++;
		v = eval();
		اگر (!nextarg(")"))
			bb_simple_error_پیام_and_die("syntax error");
		G.args++;
		بازگشت v;
	}

	اگر (nextarg(")"))
		bb_simple_error_پیام_and_die("syntax error");

	بازگشت str_مقدار(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ایستا مقدار *eval6(تهی)
{
	ایستا ثابت نویسه keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	مقدار *r, *i1, *i2;
	مقدار *l = l; /* silence gcc */
	مقدار *v = v; /* silence gcc */
	صحیح کلید = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	اگر (کلید == 0) /* not a keyword */
		بازگشت eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	اگر (کلید == 1) { /* quote */
		اگر (!*G.args)
			bb_simple_error_پیام_and_die("syntax error");
		بازگشت str_مقدار(*G.args++);
	}
	اگر (کلید == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_مقدار(strlen(r->u.s));
		freev(r);
	} وگرنه
		l = eval6();

	اگر (کلید == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	اگر (کلید == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_مقدار(strcspn(l->u.s, r->u.s) + 1);
		اگر (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	اگر (کلید == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		اگر (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_مقدار("");
		وگرنه {
			v = xmalloc(اندازه(مقدار));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	بازگشت v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ایستا مقدار *eval5(تهی)
{
	مقدار *l, *r, *v;

	l = eval6();
	وقتی (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	بازگشت l;
}

/* Handle *, /, % operators.  */

ایستا مقدار *eval4(تهی)
{
	مقدار *l, *r;
	صحیح op;
	arith_t val;

	l = eval5();
	وقتی (1) {
		op = nextarg("*");
		اگر (!op) { op = nextarg("/");
		 اگر (!op) { op = nextarg("%");
		  اگر (!op) بازگشت l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_مقدار(val);
	}
}

/* Handle +, - operators.  */

ایستا مقدار *eval3(تهی)
{
	مقدار *l, *r;
	صحیح op;
	arith_t val;

	l = eval4();
	وقتی (1) {
		op = nextarg("+");
		اگر (!op) {
			op = nextarg("-");
			اگر (!op) بازگشت l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_مقدار(val);
	}
}

/* Handle comparisons.  */

ایستا مقدار *eval2(تهی)
{
	مقدار *l, *r;
	صحیح op;
	arith_t val;

	l = eval3();
	وقتی (1) {
		op = nextarg("<");
		اگر (!op) { op = nextarg("<=");
		 اگر (!op) { op = nextarg("=");
		  اگر (!op) { op = nextarg("==");
		   اگر (!op) { op = nextarg("!=");
		    اگر (!op) { op = nextarg(">=");
		     اگر (!op) { op = nextarg(">");
		      اگر (!op) بازگشت l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_مقدار(val);
	}
}

/* Handle &.  */

ایستا مقدار *eval1(تهی)
{
	مقدار *l, *r;

	l = eval2();
	وقتی (nextarg("&")) {
		G.args++;
		r = eval2();
		اگر (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_مقدار(0);
		} وگرنه
			freev(r);
	}
	بازگشت l;
}

/* Handle |.  */

ایستا مقدار *eval(تهی)
{
	مقدار *l, *r;

	l = eval1();
	وقتی (nextarg("|")) {
		G.args++;
		r = eval1();
		اگر (null(l)) {
			freev(l);
			l = r;
		} وگرنه
			freev(r);
	}
	بازگشت l;
}

صحیح عبارت_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح عبارت_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	مقدار *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	اگر (*G.args == پوچ) {
		bb_simple_error_پیام_and_die("too few arguments");
	}
	v = eval();
	اگر (*G.args)
		bb_simple_error_پیام_and_die("syntax error");
	اگر (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	وگرنه
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
