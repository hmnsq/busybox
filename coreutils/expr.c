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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "xregex.h"

#ਜੇ ENABLE_ਵਿਅੰਜਕ_MATH_SUPPORT_64
ਕਿਸਮਪਰਿਭਾਸ਼ਾ int64_t arith_t;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_REZ      "ll"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_REZ_TYPE (ਲੰਮਾ ਲੰਮਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRTOL(s, e, b) strtoll(s, e, b)
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ arith_t;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_REZ      "l"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_REZ_TYPE (ਲੰਮਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRTOL(s, e, b) strtol(s, e, b)
#ਜੇ_ਅੰਤ

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ਗਿਣਤੀ {
	INTEGER,
	STRING
};

/* A value is.... */
ਬਣਤਰ valinfo {
	smallint type;                  /* Which kind. */
	ਸੰਘ {                         /* The value itself. */
		arith_t i;
		ਅੱਖਰ *s;
	} u;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ valinfo ਮੁੱਲ;

/* The arguments given to the program, minus the program name.  */
ਬਣਤਰ globals {
	ਅੱਖਰ **ਦਲੀਲਾਂ;
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ਜਦੋਂ (0)

/* forward declarations */
ਸਥਿਰ ਮੁੱਲ *eval(ਖਾਲੀ);


/* Return a VALUE for I.  */

ਸਥਿਰ ਮੁੱਲ *int_ਮੁੱਲ(arith_t i)
{
	ਮੁੱਲ *v;

	v = xzalloc(ਆਕਾਰ(ਮੁੱਲ));
	ਜੇ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ਵਾਪਸ v;
}

/* Return a VALUE for S.  */

ਸਥਿਰ ਮੁੱਲ *str_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *s)
{
	ਮੁੱਲ *v;

	v = xzalloc(ਆਕਾਰ(ਮੁੱਲ));
	ਜੇ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ਵਾਪਸ v;
}

/* Free VALUE V, including structure components.  */

ਸਥਿਰ ਖਾਲੀ freev(ਮੁੱਲ *v)
{
	ਜੇ (v->type == STRING)
		ਮੁਕਤ(v->u.s);
	ਮੁਕਤ(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ਸਥਿਰ ਪੂਰਨ null(ਮੁੱਲ *v)
{
	ਜੇ (v->type == INTEGER)
		ਵਾਪਸ v->u.i == 0;
	/* STRING: */
	ਵਾਪਸ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ਸਥਿਰ ਖਾਲੀ tostring(ਮੁੱਲ *v)
{
	ਜੇ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ਸਥਿਰ bool toarith(ਮੁੱਲ *v)
{
	ਜੇ (v->type == STRING) {
		arith_t i;
		ਅੱਖਰ *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ਜੇ ((v->u.s == e) || *e)
			ਵਾਪਸ 0;
		ਮੁਕਤ(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ਵਾਪਸ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ਸਥਿਰ ਪੂਰਨ nextarg(ਅਚਲ ਅੱਖਰ *str)
{
	ਜੇ (*G.ਦਲੀਲਾਂ == ਨਿਰਾ || strcmp(*G.ਦਲੀਲਾਂ, str) != 0)
		ਵਾਪਸ 0;
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)str[0] + (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)str[1];
}

/* The comparison operator handling functions.  */

ਸਥਿਰ ਪੂਰਨ cmp_common(ਮੁੱਲ *l, ਮੁੱਲ *r, ਪੂਰਨ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ਜੇ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ਜੇ (op == '<')
		ਵਾਪਸ ll < rr;
	ਜੇ (op == ('<' + '='))
		ਵਾਪਸ ll <= rr;
	ਜੇ (op == '=' || (op == '=' + '='))
		ਵਾਪਸ ll == rr;
	ਜੇ (op == '!' + '=')
		ਵਾਪਸ ll != rr;
	ਜੇ (op == '>')
		ਵਾਪਸ ll > rr;
	/* >= */
	ਵਾਪਸ ll >= rr;
}

/* The arithmetic operator handling functions.  */

ਸਥਿਰ arith_t arithmetic_common(ਮੁੱਲ *l, ਮੁੱਲ *r, ਪੂਰਨ op)
{
	arith_t li, ri;

	ਜੇ (!toarith(l) || !toarith(r))
		bb_simple_error_ਸੁਨੇਹਾ_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ਜੇ (op == '+')
		ਵਾਪਸ li + ri;
	ਜੇ (op == '-')
		ਵਾਪਸ li - ri;
	ਜੇ (op == '*')
		ਵਾਪਸ li * ri;
	ਜੇ (ri == 0)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("division by zero");
	ਜੇ (op == '/')
		ਵਾਪਸ li / ri;
	ਵਾਪਸ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ਸਥਿਰ ਮੁੱਲ *docolon(ਮੁੱਲ *sv, ਮੁੱਲ *pv)
{
	ਗਿਣਤੀ { NMATCH = 2 };
	ਮੁੱਲ *v;
	regex_t re_ਬਫਰ;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ਜੇ (pv->u.s[0] == '^') {
		bb_error_ਸੁਨੇਹਾ(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_ਬਫਰ, 0, ਆਕਾਰ(re_ਬਫਰ));
	memset(re_regs, 0, ਆਕਾਰ(re_regs));
	xregcomp(&re_ਬਫਰ, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ਜੇ (regexec(&re_ਬਫਰ, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ਜੇ (re_ਬਫਰ.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_ਮੁੱਲ(sv->u.s + re_regs[1].rm_so);
		} ਨਹੀਂਤਾਂ {
			v = int_ਮੁੱਲ(re_regs[0].rm_eo);
		}
	} ਨਹੀਂਤਾਂ {
		/* Match failed -- return the right kind of null.  */
		ਜੇ (re_ਬਫਰ.re_nsub > 0)
			v = str_ਮੁੱਲ("");
		ਨਹੀਂਤਾਂ
			v = int_ਮੁੱਲ(0);
	}
	regfree(&re_ਬਫਰ);
	ਵਾਪਸ v;
}

/* Handle bare operands and ( expr ) syntax.  */

ਸਥਿਰ ਮੁੱਲ *eval7(ਖਾਲੀ)
{
	ਮੁੱਲ *v;

	ਜੇ (!*G.ਦਲੀਲਾਂ)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("syntax error");

	ਜੇ (nextarg("(")) {
		G.ਦਲੀਲਾਂ++;
		v = eval();
		ਜੇ (!nextarg(")"))
			bb_simple_error_ਸੁਨੇਹਾ_and_die("syntax error");
		G.ਦਲੀਲਾਂ++;
		ਵਾਪਸ v;
	}

	ਜੇ (nextarg(")"))
		bb_simple_error_ਸੁਨੇਹਾ_and_die("syntax error");

	ਵਾਪਸ str_ਮੁੱਲ(*G.ਦਲੀਲਾਂ++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ਸਥਿਰ ਮੁੱਲ *eval6(ਖਾਲੀ)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	ਮੁੱਲ *r, *i1, *i2;
	ਮੁੱਲ *l = l; /* silence gcc */
	ਮੁੱਲ *v = v; /* silence gcc */
	ਪੂਰਨ ਕੁੰਜੀ = *G.ਦਲੀਲਾਂ ? ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(keywords, *G.ਦਲੀਲਾਂ) + 1 : 0;

	ਜੇ (ਕੁੰਜੀ == 0) /* not a keyword */
		ਵਾਪਸ eval7();
	G.ਦਲੀਲਾਂ++; /* We have a valid token, so get the next argument.  */
	ਜੇ (ਕੁੰਜੀ == 1) { /* quote */
		ਜੇ (!*G.ਦਲੀਲਾਂ)
			bb_simple_error_ਸੁਨੇਹਾ_and_die("syntax error");
		ਵਾਪਸ str_ਮੁੱਲ(*G.ਦਲੀਲਾਂ++);
	}
	ਜੇ (ਕੁੰਜੀ == 2) { /*ਲੰਮਾਈ*/
		r = eval6();
		tostring(r);
		v = int_ਮੁੱਲ(ਲੜੀ_ਲੰਬਾਈ(r->u.s));
		freev(r);
	} ਨਹੀਂਤਾਂ
		l = eval6();

	ਜੇ (ਕੁੰਜੀ == 3) { /*ਮੇਲ*/
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ਜੇ (ਕੁੰਜੀ == 4) { /*ਸੂਚਕਾਂਕ*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_ਮੁੱਲ(strcspn(l->u.s, r->u.s) + 1);
		ਜੇ (v->u.i == (arith_t) ਲੜੀ_ਲੰਬਾਈ(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ਜੇ (ਕੁੰਜੀ == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ਜੇ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) ਲੜੀ_ਲੰਬਾਈ(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_ਮੁੱਲ("");
		ਨਹੀਂਤਾਂ {
			v = xmalloc(ਆਕਾਰ(ਮੁੱਲ));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ਵਾਪਸ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ਸਥਿਰ ਮੁੱਲ *eval5(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r, *v;

	l = eval6();
	ਜਦੋਂ (nextarg(":")) {
		G.ਦਲੀਲਾਂ++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ਵਾਪਸ l;
}

/* Handle *, /, % operators.  */

ਸਥਿਰ ਮੁੱਲ *eval4(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r;
	ਪੂਰਨ op;
	arith_t val;

	l = eval5();
	ਜਦੋਂ (1) {
		op = nextarg("*");
		ਜੇ (!op) { op = nextarg("/");
		 ਜੇ (!op) { op = nextarg("%");
		  ਜੇ (!op) ਵਾਪਸ l;
		}}
		G.ਦਲੀਲਾਂ++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ਮੁੱਲ(val);
	}
}

/* Handle +, - operators.  */

ਸਥਿਰ ਮੁੱਲ *eval3(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r;
	ਪੂਰਨ op;
	arith_t val;

	l = eval4();
	ਜਦੋਂ (1) {
		op = nextarg("+");
		ਜੇ (!op) {
			op = nextarg("-");
			ਜੇ (!op) ਵਾਪਸ l;
		}
		G.ਦਲੀਲਾਂ++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ਮੁੱਲ(val);
	}
}

/* Handle comparisons.  */

ਸਥਿਰ ਮੁੱਲ *eval2(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r;
	ਪੂਰਨ op;
	arith_t val;

	l = eval3();
	ਜਦੋਂ (1) {
		op = nextarg("<");
		ਜੇ (!op) { op = nextarg("<=");
		 ਜੇ (!op) { op = nextarg("=");
		  ਜੇ (!op) { op = nextarg("==");
		   ਜੇ (!op) { op = nextarg("!=");
		    ਜੇ (!op) { op = nextarg(">=");
		     ਜੇ (!op) { op = nextarg(">");
		      ਜੇ (!op) ਵਾਪਸ l;
		}}}}}}
		G.ਦਲੀਲਾਂ++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ਮੁੱਲ(val);
	}
}

/* Handle &.  */

ਸਥਿਰ ਮੁੱਲ *eval1(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r;

	l = eval2();
	ਜਦੋਂ (nextarg("&")) {
		G.ਦਲੀਲਾਂ++;
		r = eval2();
		ਜੇ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_ਮੁੱਲ(0);
		} ਨਹੀਂਤਾਂ
			freev(r);
	}
	ਵਾਪਸ l;
}

/* Handle |.  */

ਸਥਿਰ ਮੁੱਲ *eval(ਖਾਲੀ)
{
	ਮੁੱਲ *l, *r;

	l = eval1();
	ਜਦੋਂ (nextarg("|")) {
		G.ਦਲੀਲਾਂ++;
		r = eval1();
		ਜੇ (null(l)) {
			freev(l);
			l = r;
		} ਨਹੀਂਤਾਂ
			freev(r);
	}
	ਵਾਪਸ l;
}

ਪੂਰਨ ਵਿਅੰਜਕ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਵਿਅੰਜਕ_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਮੁੱਲ *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.ਦਲੀਲਾਂ = argv + 1;
	ਜੇ (*G.ਦਲੀਲਾਂ == ਨਿਰਾ) {
		bb_simple_error_ਸੁਨੇਹਾ_and_die("too few arguments");
	}
	v = eval();
	ਜੇ (*G.ਦਲੀਲਾਂ)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("syntax error");
	ਜੇ (v->type == INTEGER)
		ਲਿਖੋ("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ਨਹੀਂਤਾਂ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
