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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"
#ಅನ್ತರ್ಭಾವ "xregex.h"

#ಆದರೆ ENABLE_ಅಭಿವ್ಯಕ್ತಿ_MATH_SUPPORT_64
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ int64_t arith_t;

#ವ್ಯಾಖ್ಯಾನಿಸು PF_REZ      "ll"
#ವ್ಯಾಖ್ಯಾನಿಸು PF_REZ_TYPE (ದೀರ್ಘ ದೀರ್ಘ)
#ವ್ಯಾಖ್ಯಾನಿಸು STRTOL(s, e, b) strtoll(s, e, b)
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ arith_t;

#ವ್ಯಾಖ್ಯಾನಿಸು PF_REZ      "l"
#ವ್ಯಾಖ್ಯಾನಿಸು PF_REZ_TYPE (ದೀರ್ಘ)
#ವ್ಯಾಖ್ಯಾನಿಸು STRTOL(s, e, b) strtol(s, e, b)
#ಆದರೆ_ಅಂತ್ಯ

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ಎಣಿಕೆ {
	INTEGER,
	STRING
};

/* A value is.... */
ರಚನೆ valinfo {
	smallint type;                  /* Which kind. */
	ಒಕ್ಕೂಟ {                         /* The value itself. */
		arith_t i;
		ಅಕ್ಷರ *s;
	} u;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ valinfo ಮೌಲ್ಯ;

/* The arguments given to the program, minus the program name.  */
ರಚನೆ globals {
	ಅಕ್ಷರ **ವಾದಗಳು;
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ಆಗ (0)

/* forward declarations */
ಸ್ಥಿರ ಮೌಲ್ಯ *eval(ಶೂನ್ಯ);


/* Return a VALUE for I.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *int_ಮೌಲ್ಯ(arith_t i)
{
	ಮೌಲ್ಯ *v;

	v = xzalloc(ಗಾತ್ರ(ಮೌಲ್ಯ));
	ಆದರೆ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ಹಿಂತಿರುಗಿ v;
}

/* Return a VALUE for S.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *str_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಮೌಲ್ಯ *v;

	v = xzalloc(ಗಾತ್ರ(ಮೌಲ್ಯ));
	ಆದರೆ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ಹಿಂತಿರುಗಿ v;
}

/* Free VALUE V, including structure components.  */

ಸ್ಥಿರ ಶೂನ್ಯ freev(ಮೌಲ್ಯ *v)
{
	ಆದರೆ (v->type == STRING)
		ಬಿಡುಗಡೆ(v->u.s);
	ಬಿಡುಗಡೆ(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ null(ಮೌಲ್ಯ *v)
{
	ಆದರೆ (v->type == INTEGER)
		ಹಿಂತಿರುಗಿ v->u.i == 0;
	/* STRING: */
	ಹಿಂತಿರುಗಿ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ಸ್ಥಿರ ಶೂನ್ಯ tostring(ಮೌಲ್ಯ *v)
{
	ಆದರೆ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ಸ್ಥಿರ bool toarith(ಮೌಲ್ಯ *v)
{
	ಆದರೆ (v->type == STRING) {
		arith_t i;
		ಅಕ್ಷರ *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ಆದರೆ ((v->u.s == e) || *e)
			ಹಿಂತಿರುಗಿ 0;
		ಬಿಡುಗಡೆ(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ಹಿಂತಿರುಗಿ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nextarg(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಆದರೆ (*G.ವಾದಗಳು == ಖಾಲಿ || strcmp(*G.ವಾದಗಳು, str) != 0)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)str[0] + (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)str[1];
}

/* The comparison operator handling functions.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmp_common(ಮೌಲ್ಯ *l, ಮೌಲ್ಯ *r, ಪೂರ್ಣಾಂಕ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ಆದರೆ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ಆದರೆ (op == '<')
		ಹಿಂತಿರುಗಿ ll < rr;
	ಆದರೆ (op == ('<' + '='))
		ಹಿಂತಿರುಗಿ ll <= rr;
	ಆದರೆ (op == '=' || (op == '=' + '='))
		ಹಿಂತಿರುಗಿ ll == rr;
	ಆದರೆ (op == '!' + '=')
		ಹಿಂತಿರುಗಿ ll != rr;
	ಆದರೆ (op == '>')
		ಹಿಂತಿರುಗಿ ll > rr;
	/* >= */
	ಹಿಂತಿರುಗಿ ll >= rr;
}

/* The arithmetic operator handling functions.  */

ಸ್ಥಿರ arith_t arithmetic_common(ಮೌಲ್ಯ *l, ಮೌಲ್ಯ *r, ಪೂರ್ಣಾಂಕ op)
{
	arith_t li, ri;

	ಆದರೆ (!toarith(l) || !toarith(r))
		bb_simple_error_ಸಂದೇಶ_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ಆದರೆ (op == '+')
		ಹಿಂತಿರುಗಿ li + ri;
	ಆದರೆ (op == '-')
		ಹಿಂತಿರುಗಿ li - ri;
	ಆದರೆ (op == '*')
		ಹಿಂತಿರುಗಿ li * ri;
	ಆದರೆ (ri == 0)
		bb_simple_error_ಸಂದೇಶ_and_die("division by zero");
	ಆದರೆ (op == '/')
		ಹಿಂತಿರುಗಿ li / ri;
	ಹಿಂತಿರುಗಿ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ಸ್ಥಿರ ಮೌಲ್ಯ *docolon(ಮೌಲ್ಯ *sv, ಮೌಲ್ಯ *pv)
{
	ಎಣಿಕೆ { NMATCH = 2 };
	ಮೌಲ್ಯ *v;
	regex_t re_ಬಫರ್;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ಆದರೆ (pv->u.s[0] == '^') {
		bb_error_ಸಂದೇಶ(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_ಬಫರ್, 0, ಗಾತ್ರ(re_ಬಫರ್));
	memset(re_regs, 0, ಗಾತ್ರ(re_regs));
	xregcomp(&re_ಬಫರ್, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ಆದರೆ (regexec(&re_ಬಫರ್, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ಆದರೆ (re_ಬಫರ್.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_ಮೌಲ್ಯ(sv->u.s + re_regs[1].rm_so);
		} ಇಲ್ಲದಿದ್ದರೆ {
			v = int_ಮೌಲ್ಯ(re_regs[0].rm_eo);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Match failed -- return the right kind of null.  */
		ಆದರೆ (re_ಬಫರ್.re_nsub > 0)
			v = str_ಮೌಲ್ಯ("");
		ಇಲ್ಲದಿದ್ದರೆ
			v = int_ಮೌಲ್ಯ(0);
	}
	regfree(&re_ಬಫರ್);
	ಹಿಂತಿರುಗಿ v;
}

/* Handle bare operands and ( expr ) syntax.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval7(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *v;

	ಆದರೆ (!*G.ವಾದಗಳು)
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");

	ಆದರೆ (nextarg("(")) {
		G.ವಾದಗಳು++;
		v = eval();
		ಆದರೆ (!nextarg(")"))
			bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
		G.ವಾದಗಳು++;
		ಹಿಂತಿರುಗಿ v;
	}

	ಆದರೆ (nextarg(")"))
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");

	ಹಿಂತಿರುಗಿ str_ಮೌಲ್ಯ(*G.ವಾದಗಳು++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval6(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	ಮೌಲ್ಯ *r, *i1, *i2;
	ಮೌಲ್ಯ *l = l; /* silence gcc */
	ಮೌಲ್ಯ *v = v; /* silence gcc */
	ಪೂರ್ಣಾಂಕ ಕೀಲಿ = *G.ವಾದಗಳು ? ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(keywords, *G.ವಾದಗಳು) + 1 : 0;

	ಆದರೆ (ಕೀಲಿ == 0) /* not a keyword */
		ಹಿಂತಿರುಗಿ eval7();
	G.ವಾದಗಳು++; /* We have a valid token, so get the next argument.  */
	ಆದರೆ (ಕೀಲಿ == 1) { /* quote */
		ಆದರೆ (!*G.ವಾದಗಳು)
			bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
		ಹಿಂತಿರುಗಿ str_ಮೌಲ್ಯ(*G.ವಾದಗಳು++);
	}
	ಆದರೆ (ಕೀಲಿ == 2) { /*ಉದ್ದಳತೆ*/
		r = eval6();
		tostring(r);
		v = int_ಮೌಲ್ಯ(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(r->u.s));
		freev(r);
	} ಇಲ್ಲದಿದ್ದರೆ
		l = eval6();

	ಆದರೆ (ಕೀಲಿ == 3) { /*ಹೊಂದಿಕೆ*/
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ಆದರೆ (ಕೀಲಿ == 4) { /*ಸೂಚ್ಯಂಕ*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_ಮೌಲ್ಯ(strcspn(l->u.s, r->u.s) + 1);
		ಆದರೆ (v->u.i == (arith_t) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ಆದರೆ (ಕೀಲಿ == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ಆದರೆ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_ಮೌಲ್ಯ("");
		ಇಲ್ಲದಿದ್ದರೆ {
			v = xmalloc(ಗಾತ್ರ(ಮೌಲ್ಯ));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ಹಿಂತಿರುಗಿ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval5(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r, *v;

	l = eval6();
	ಆಗ (nextarg(":")) {
		G.ವಾದಗಳು++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ಹಿಂತಿರುಗಿ l;
}

/* Handle *, /, % operators.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval4(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;
	ಪೂರ್ಣಾಂಕ op;
	arith_t val;

	l = eval5();
	ಆಗ (1) {
		op = nextarg("*");
		ಆದರೆ (!op) { op = nextarg("/");
		 ಆದರೆ (!op) { op = nextarg("%");
		  ಆದರೆ (!op) ಹಿಂತಿರುಗಿ l;
		}}
		G.ವಾದಗಳು++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ಮೌಲ್ಯ(val);
	}
}

/* Handle +, - operators.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval3(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;
	ಪೂರ್ಣಾಂಕ op;
	arith_t val;

	l = eval4();
	ಆಗ (1) {
		op = nextarg("+");
		ಆದರೆ (!op) {
			op = nextarg("-");
			ಆದರೆ (!op) ಹಿಂತಿರುಗಿ l;
		}
		G.ವಾದಗಳು++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ಮೌಲ್ಯ(val);
	}
}

/* Handle comparisons.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval2(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;
	ಪೂರ್ಣಾಂಕ op;
	arith_t val;

	l = eval3();
	ಆಗ (1) {
		op = nextarg("<");
		ಆದರೆ (!op) { op = nextarg("<=");
		 ಆದರೆ (!op) { op = nextarg("=");
		  ಆದರೆ (!op) { op = nextarg("==");
		   ಆದರೆ (!op) { op = nextarg("!=");
		    ಆದರೆ (!op) { op = nextarg(">=");
		     ಆದರೆ (!op) { op = nextarg(">");
		      ಆದರೆ (!op) ಹಿಂತಿರುಗಿ l;
		}}}}}}
		G.ವಾದಗಳು++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ಮೌಲ್ಯ(val);
	}
}

/* Handle &.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval1(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;

	l = eval2();
	ಆಗ (nextarg("&")) {
		G.ವಾದಗಳು++;
		r = eval2();
		ಆದರೆ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_ಮೌಲ್ಯ(0);
		} ಇಲ್ಲದಿದ್ದರೆ
			freev(r);
	}
	ಹಿಂತಿರುಗಿ l;
}

/* Handle |.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;

	l = eval1();
	ಆಗ (nextarg("|")) {
		G.ವಾದಗಳು++;
		r = eval1();
		ಆದರೆ (null(l)) {
			freev(l);
			l = r;
		} ಇಲ್ಲದಿದ್ದರೆ
			freev(r);
	}
	ಹಿಂತಿರುಗಿ l;
}

ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಮೌಲ್ಯ *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.ವಾದಗಳು = argv + 1;
	ಆದರೆ (*G.ವಾದಗಳು == ಖಾಲಿ) {
		bb_simple_error_ಸಂದೇಶ_and_die("too few arguments");
	}
	v = eval();
	ಆದರೆ (*G.ವಾದಗಳು)
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
	ಆದರೆ (v->type == INTEGER)
		ಬರೆ("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ಇಲ್ಲದಿದ್ದರೆ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
