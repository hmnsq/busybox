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

#اگر ENABLE_اظہاریہ_MATH_SUPPORT_64
قسمتعریف int64_t arith_t;

#تعریف_کرو PF_REZ      "ll"
#تعریف_کرو PF_REZ_TYPE (طویل طویل)
#تعریف_کرو STRTOL(s, e, b) strtoll(s, e, b)
#ورنہ
قسمتعریف طویل arith_t;

#تعریف_کرو PF_REZ      "l"
#تعریف_کرو PF_REZ_TYPE (طویل)
#تعریف_کرو STRTOL(s, e, b) strtol(s, e, b)
#اگر_ختم

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
شمار {
	INTEGER,
	STRING
};

/* A value is.... */
ساخت valinfo {
	smallint type;                  /* Which kind. */
	اتحاد {                         /* The value itself. */
		arith_t i;
		حرف *s;
	} u;
};
قسمتعریف ساخت valinfo قدر;

/* The arguments given to the program, minus the program name.  */
ساخت globals {
	حرف **دلائل;
} FIX_ALIASING;
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} جب (0)

/* forward declarations */
ساکن قدر *eval(خالی);


/* Return a VALUE for I.  */

ساکن قدر *int_قدر(arith_t i)
{
	قدر *v;

	v = xzalloc(حجم(قدر));
	اگر (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	واپس v;
}

/* Return a VALUE for S.  */

ساکن قدر *str_قدر(ثابت حرف *s)
{
	قدر *v;

	v = xzalloc(حجم(قدر));
	اگر (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	واپس v;
}

/* Free VALUE V, including structure components.  */

ساکن خالی freev(قدر *v)
{
	اگر (v->type == STRING)
		آزاد(v->u.s);
	آزاد(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ساکن صحیح null(قدر *v)
{
	اگر (v->type == INTEGER)
		واپس v->u.i == 0;
	/* STRING: */
	واپس v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ساکن خالی tostring(قدر *v)
{
	اگر (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ساکن bool toarith(قدر *v)
{
	اگر (v->type == STRING) {
		arith_t i;
		حرف *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		اگر ((v->u.s == e) || *e)
			واپس 0;
		آزاد(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	واپس 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ساکن صحیح nextarg(ثابت حرف *str)
{
	اگر (*G.دلائل == تہی || strcmp(*G.دلائل, str) != 0)
		واپس 0;
	واپس (غیرعلامتی حرف)str[0] + (غیرعلامتی حرف)str[1];
}

/* The comparison operator handling functions.  */

ساکن صحیح cmp_common(قدر *l, قدر *r, صحیح op)
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
		واپس ll < rr;
	اگر (op == ('<' + '='))
		واپس ll <= rr;
	اگر (op == '=' || (op == '=' + '='))
		واپس ll == rr;
	اگر (op == '!' + '=')
		واپس ll != rr;
	اگر (op == '>')
		واپس ll > rr;
	/* >= */
	واپس ll >= rr;
}

/* The arithmetic operator handling functions.  */

ساکن arith_t arithmetic_common(قدر *l, قدر *r, صحیح op)
{
	arith_t li, ri;

	اگر (!toarith(l) || !toarith(r))
		bb_simple_error_پیغام_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	اگر (op == '+')
		واپس li + ri;
	اگر (op == '-')
		واپس li - ri;
	اگر (op == '*')
		واپس li * ri;
	اگر (ri == 0)
		bb_simple_error_پیغام_and_die("division by zero");
	اگر (op == '/')
		واپس li / ri;
	واپس li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ساکن قدر *docolon(قدر *sv, قدر *pv)
{
	شمار { NMATCH = 2 };
	قدر *v;
	regex_t re_بفر;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	اگر (pv->u.s[0] == '^') {
		bb_error_پیغام(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_بفر, 0, حجم(re_بفر));
	memset(re_regs, 0, حجم(re_regs));
	xregcomp(&re_بفر, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	اگر (regexec(&re_بفر, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		اگر (re_بفر.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_قدر(sv->u.s + re_regs[1].rm_so);
		} ورنہ {
			v = int_قدر(re_regs[0].rm_eo);
		}
	} ورنہ {
		/* Match failed -- return the right kind of null.  */
		اگر (re_بفر.re_nsub > 0)
			v = str_قدر("");
		ورنہ
			v = int_قدر(0);
	}
	regfree(&re_بفر);
	واپس v;
}

/* Handle bare operands and ( expr ) syntax.  */

ساکن قدر *eval7(خالی)
{
	قدر *v;

	اگر (!*G.دلائل)
		bb_simple_error_پیغام_and_die("syntax error");

	اگر (nextarg("(")) {
		G.دلائل++;
		v = eval();
		اگر (!nextarg(")"))
			bb_simple_error_پیغام_and_die("syntax error");
		G.دلائل++;
		واپس v;
	}

	اگر (nextarg(")"))
		bb_simple_error_پیغام_and_die("syntax error");

	واپس str_قدر(*G.دلائل++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ساکن قدر *eval6(خالی)
{
	ساکن ثابت حرف keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	قدر *r, *i1, *i2;
	قدر *l = l; /* silence gcc */
	قدر *v = v; /* silence gcc */
	صحیح کلید = *G.دلائل ? اشاریہ_اندر_لڑیاں(keywords, *G.دلائل) + 1 : 0;

	اگر (کلید == 0) /* not a keyword */
		واپس eval7();
	G.دلائل++; /* We have a valid token, so get the next argument.  */
	اگر (کلید == 1) { /* quote */
		اگر (!*G.دلائل)
			bb_simple_error_پیغام_and_die("syntax error");
		واپس str_قدر(*G.دلائل++);
	}
	اگر (کلید == 2) { /*لمبائی*/
		r = eval6();
		tostring(r);
		v = int_قدر(لڑی_لمبائی(r->u.s));
		freev(r);
	} ورنہ
		l = eval6();

	اگر (کلید == 3) { /*مطابقت*/
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	اگر (کلید == 4) { /*اشاریہ*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_قدر(strcspn(l->u.s, r->u.s) + 1);
		اگر (v->u.i == (arith_t) لڑی_لمبائی(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	اگر (کلید == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		اگر (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) لڑی_لمبائی(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_قدر("");
		ورنہ {
			v = xmalloc(حجم(قدر));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	واپس v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ساکن قدر *eval5(خالی)
{
	قدر *l, *r, *v;

	l = eval6();
	جب (nextarg(":")) {
		G.دلائل++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	واپس l;
}

/* Handle *, /, % operators.  */

ساکن قدر *eval4(خالی)
{
	قدر *l, *r;
	صحیح op;
	arith_t val;

	l = eval5();
	جب (1) {
		op = nextarg("*");
		اگر (!op) { op = nextarg("/");
		 اگر (!op) { op = nextarg("%");
		  اگر (!op) واپس l;
		}}
		G.دلائل++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قدر(val);
	}
}

/* Handle +, - operators.  */

ساکن قدر *eval3(خالی)
{
	قدر *l, *r;
	صحیح op;
	arith_t val;

	l = eval4();
	جب (1) {
		op = nextarg("+");
		اگر (!op) {
			op = nextarg("-");
			اگر (!op) واپس l;
		}
		G.دلائل++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قدر(val);
	}
}

/* Handle comparisons.  */

ساکن قدر *eval2(خالی)
{
	قدر *l, *r;
	صحیح op;
	arith_t val;

	l = eval3();
	جب (1) {
		op = nextarg("<");
		اگر (!op) { op = nextarg("<=");
		 اگر (!op) { op = nextarg("=");
		  اگر (!op) { op = nextarg("==");
		   اگر (!op) { op = nextarg("!=");
		    اگر (!op) { op = nextarg(">=");
		     اگر (!op) { op = nextarg(">");
		      اگر (!op) واپس l;
		}}}}}}
		G.دلائل++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قدر(val);
	}
}

/* Handle &.  */

ساکن قدر *eval1(خالی)
{
	قدر *l, *r;

	l = eval2();
	جب (nextarg("&")) {
		G.دلائل++;
		r = eval2();
		اگر (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_قدر(0);
		} ورنہ
			freev(r);
	}
	واپس l;
}

/* Handle |.  */

ساکن قدر *eval(خالی)
{
	قدر *l, *r;

	l = eval1();
	جب (nextarg("|")) {
		G.دلائل++;
		r = eval1();
		اگر (null(l)) {
			freev(l);
			l = r;
		} ورنہ
			freev(r);
	}
	واپس l;
}

صحیح اظہاریہ_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح اظہاریہ_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	قدر *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.دلائل = argv + 1;
	اگر (*G.دلائل == تہی) {
		bb_simple_error_پیغام_and_die("too few arguments");
	}
	v = eval();
	اگر (*G.دلائل)
		bb_simple_error_پیغام_and_die("syntax error");
	اگر (v->type == INTEGER)
		لکھو("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ورنہ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
