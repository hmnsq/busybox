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

#ಯದಿ ENABLE_ಅಭಿವ್ಯಕ್ತಿ_MATH_SUPPORT_64
ಪ್ರಕಾರನಿರ್ದೇಶ int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ಗಣನಾ {
	INTEGER,
	STRING
};

/* A value is.... */
ಸಂರಚನಾ valinfo {
	smallint type;                  /* Which kind. */
	ಸಙ್ಘ {                         /* The value itself. */
		arith_t i;
		ವರ್ಣ *s;
	} u;
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ valinfo ಮೌಲ್ಯ;

/* The arguments given to the program, minus the program name.  */
ಸಂರಚನಾ globals {
	ವರ್ಣ **args;
} FIX_ALIASING;
#define G (*(ಸಂರಚನಾ globals*)bb_common_bufsiz1)
#define INIT_G() ಕುರು { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ಯಾವತ್ (0)

/* forward declarations */
ಸ್ಥಿರ ಮೌಲ್ಯ *eval(ಶೂನ್ಯ);


/* Return a VALUE for I.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *int_ಮೌಲ್ಯ(arith_t i)
{
	ಮೌಲ್ಯ *v;

	v = xzalloc(ಪರಿಮಾಣ(ಮೌಲ್ಯ));
	ಯದಿ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ಪ್ರತ್ಯಾಗಮ v;
}

/* Return a VALUE for S.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *str_ಮೌಲ್ಯ(ಧ್ರುವ ವರ್ಣ *s)
{
	ಮೌಲ್ಯ *v;

	v = xzalloc(ಪರಿಮಾಣ(ಮೌಲ್ಯ));
	ಯದಿ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ಪ್ರತ್ಯಾಗಮ v;
}

/* Free VALUE V, including structure components.  */

ಸ್ಥಿರ ಶೂನ್ಯ freev(ಮೌಲ್ಯ *v)
{
	ಯದಿ (v->type == STRING)
		ಮುಕ್ತ(v->u.s);
	ಮುಕ್ತ(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ null(ಮೌಲ್ಯ *v)
{
	ಯದಿ (v->type == INTEGER)
		ಪ್ರತ್ಯಾಗಮ v->u.i == 0;
	/* STRING: */
	ಪ್ರತ್ಯಾಗಮ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ಸ್ಥಿರ ಶೂನ್ಯ tostring(ಮೌಲ್ಯ *v)
{
	ಯದಿ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ಸ್ಥಿರ bool toarith(ಮೌಲ್ಯ *v)
{
	ಯದಿ (v->type == STRING) {
		arith_t i;
		ವರ್ಣ *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ಯದಿ ((v->u.s == e) || *e)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಮುಕ್ತ(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ಪ್ರತ್ಯಾಗಮ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nextarg(ಧ್ರುವ ವರ್ಣ *str)
{
	ಯದಿ (*G.args == ರಿಕ್ತ || strcmp(*G.args, str) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ (unsigned ವರ್ಣ)str[0] + (unsigned ವರ್ಣ)str[1];
}

/* The comparison operator handling functions.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmp_common(ಮೌಲ್ಯ *l, ಮೌಲ್ಯ *r, ಪೂರ್ಣಾಙ್ಕ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ಯದಿ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ಯದಿ (op == '<')
		ಪ್ರತ್ಯಾಗಮ ll < rr;
	ಯದಿ (op == ('<' + '='))
		ಪ್ರತ್ಯಾಗಮ ll <= rr;
	ಯದಿ (op == '=' || (op == '=' + '='))
		ಪ್ರತ್ಯಾಗಮ ll == rr;
	ಯದಿ (op == '!' + '=')
		ಪ್ರತ್ಯಾಗಮ ll != rr;
	ಯದಿ (op == '>')
		ಪ್ರತ್ಯಾಗಮ ll > rr;
	/* >= */
	ಪ್ರತ್ಯಾಗಮ ll >= rr;
}

/* The arithmetic operator handling functions.  */

ಸ್ಥಿರ arith_t arithmetic_common(ಮೌಲ್ಯ *l, ಮೌಲ್ಯ *r, ಪೂರ್ಣಾಙ್ಕ op)
{
	arith_t li, ri;

	ಯದಿ (!toarith(l) || !toarith(r))
		bb_simple_error_ಸಂದೇಶ_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ಯದಿ (op == '+')
		ಪ್ರತ್ಯಾಗಮ li + ri;
	ಯದಿ (op == '-')
		ಪ್ರತ್ಯಾಗಮ li - ri;
	ಯದಿ (op == '*')
		ಪ್ರತ್ಯಾಗಮ li * ri;
	ಯದಿ (ri == 0)
		bb_simple_error_ಸಂದೇಶ_and_die("division by zero");
	ಯದಿ (op == '/')
		ಪ್ರತ್ಯಾಗಮ li / ri;
	ಪ್ರತ್ಯಾಗಮ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ಸ್ಥಿರ ಮೌಲ್ಯ *docolon(ಮೌಲ್ಯ *sv, ಮೌಲ್ಯ *pv)
{
	ಗಣನಾ { NMATCH = 2 };
	ಮೌಲ್ಯ *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ಯದಿ (pv->u.s[0] == '^') {
		bb_error_ಸಂದೇಶ(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, ಪರಿಮಾಣ(re_buffer));
	memset(re_regs, 0, ಪರಿಮಾಣ(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ಯದಿ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ಯದಿ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_ಮೌಲ್ಯ(sv->u.s + re_regs[1].rm_so);
		} ಅನ್ಯಥಾ {
			v = int_ಮೌಲ್ಯ(re_regs[0].rm_eo);
		}
	} ಅನ್ಯಥಾ {
		/* Match failed -- return the right kind of null.  */
		ಯದಿ (re_buffer.re_nsub > 0)
			v = str_ಮೌಲ್ಯ("");
		ಅನ್ಯಥಾ
			v = int_ಮೌಲ್ಯ(0);
	}
	regfree(&re_buffer);
	ಪ್ರತ್ಯಾಗಮ v;
}

/* Handle bare operands and ( expr ) syntax.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval7(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *v;

	ಯದಿ (!*G.args)
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");

	ಯದಿ (nextarg("(")) {
		G.args++;
		v = eval();
		ಯದಿ (!nextarg(")"))
			bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
		G.args++;
		ಪ್ರತ್ಯಾಗಮ v;
	}

	ಯದಿ (nextarg(")"))
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");

	ಪ್ರತ್ಯಾಗಮ str_ಮೌಲ್ಯ(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval6(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	ಮೌಲ್ಯ *r, *i1, *i2;
	ಮೌಲ್ಯ *l = l; /* silence gcc */
	ಮೌಲ್ಯ *v = v; /* silence gcc */
	ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	ಯದಿ (ಕೀಲಿ == 0) /* not a keyword */
		ಪ್ರತ್ಯಾಗಮ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	ಯದಿ (ಕೀಲಿ == 1) { /* quote */
		ಯದಿ (!*G.args)
			bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
		ಪ್ರತ್ಯಾಗಮ str_ಮೌಲ್ಯ(*G.args++);
	}
	ಯದಿ (ಕೀಲಿ == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_ಮೌಲ್ಯ(strlen(r->u.s));
		freev(r);
	} ಅನ್ಯಥಾ
		l = eval6();

	ಯದಿ (ಕೀಲಿ == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ಯದಿ (ಕೀಲಿ == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_ಮೌಲ್ಯ(strcspn(l->u.s, r->u.s) + 1);
		ಯದಿ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ಯದಿ (ಕೀಲಿ == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ಯದಿ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_ಮೌಲ್ಯ("");
		ಅನ್ಯಥಾ {
			v = xmalloc(ಪರಿಮಾಣ(ಮೌಲ್ಯ));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ಪ್ರತ್ಯಾಗಮ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval5(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r, *v;

	l = eval6();
	ಯಾವತ್ (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ಪ್ರತ್ಯಾಗಮ l;
}

/* Handle *, /, % operators.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval4(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;
	ಪೂರ್ಣಾಙ್ಕ op;
	arith_t val;

	l = eval5();
	ಯಾವತ್ (1) {
		op = nextarg("*");
		ಯದಿ (!op) { op = nextarg("/");
		 ಯದಿ (!op) { op = nextarg("%");
		  ಯದಿ (!op) ಪ್ರತ್ಯಾಗಮ l;
		}}
		G.args++;
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
	ಪೂರ್ಣಾಙ್ಕ op;
	arith_t val;

	l = eval4();
	ಯಾವತ್ (1) {
		op = nextarg("+");
		ಯದಿ (!op) {
			op = nextarg("-");
			ಯದಿ (!op) ಪ್ರತ್ಯಾಗಮ l;
		}
		G.args++;
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
	ಪೂರ್ಣಾಙ್ಕ op;
	arith_t val;

	l = eval3();
	ಯಾವತ್ (1) {
		op = nextarg("<");
		ಯದಿ (!op) { op = nextarg("<=");
		 ಯದಿ (!op) { op = nextarg("=");
		  ಯದಿ (!op) { op = nextarg("==");
		   ಯದಿ (!op) { op = nextarg("!=");
		    ಯದಿ (!op) { op = nextarg(">=");
		     ಯದಿ (!op) { op = nextarg(">");
		      ಯದಿ (!op) ಪ್ರತ್ಯಾಗಮ l;
		}}}}}}
		G.args++;
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
	ಯಾವತ್ (nextarg("&")) {
		G.args++;
		r = eval2();
		ಯದಿ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_ಮೌಲ್ಯ(0);
		} ಅನ್ಯಥಾ
			freev(r);
	}
	ಪ್ರತ್ಯಾಗಮ l;
}

/* Handle |.  */

ಸ್ಥಿರ ಮೌಲ್ಯ *eval(ಶೂನ್ಯ)
{
	ಮೌಲ್ಯ *l, *r;

	l = eval1();
	ಯಾವತ್ (nextarg("|")) {
		G.args++;
		r = eval1();
		ಯದಿ (null(l)) {
			freev(l);
			l = r;
		} ಅನ್ಯಥಾ
			freev(r);
	}
	ಪ್ರತ್ಯಾಗಮ l;
}

ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಮೌಲ್ಯ *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	ಯದಿ (*G.args == ರಿಕ್ತ) {
		bb_simple_error_ಸಂದೇಶ_and_die("too few arguments");
	}
	v = eval();
	ಯದಿ (*G.args)
		bb_simple_error_ಸಂದೇಶ_and_die("syntax error");
	ಯದಿ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ಅನ್ಯಥಾ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
