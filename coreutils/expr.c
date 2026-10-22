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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"
#అన్తర్భావ "xregex.h"

#ఒకవేళ ENABLE_సమాసం_MATH_SUPPORT_64
రకనిర్వచనం int64_t arith_t;

#నిర్వచించు PF_REZ      "ll"
#నిర్వచించు PF_REZ_TYPE (దీర్ఘ దీర్ఘ)
#నిర్వచించు STRTOL(s, e, b) strtoll(s, e, b)
#లేకపోతే
రకనిర్వచనం దీర్ఘ arith_t;

#నిర్వచించు PF_REZ      "l"
#నిర్వచించు PF_REZ_TYPE (దీర్ఘ)
#నిర్వచించు STRTOL(s, e, b) strtol(s, e, b)
#అయితే_ముగింపు

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
గణన {
	INTEGER,
	STRING
};

/* A value is.... */
నిర్మాణం valinfo {
	smallint type;                  /* Which kind. */
	సంఘం {                         /* The value itself. */
		arith_t i;
		అక్షరం *s;
	} u;
};
రకనిర్వచనం నిర్మాణం valinfo విలువ;

/* The arguments given to the program, minus the program name.  */
నిర్మాణం globals {
	అక్షరం **వాదనలు;
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} అయితే (0)

/* forward declarations */
స్థిర విలువ *eval(శూన్య);


/* Return a VALUE for I.  */

స్థిర విలువ *int_విలువ(arith_t i)
{
	విలువ *v;

	v = xzalloc(పరిమాణం(విలువ));
	ఒకవేళ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	తిరిగి v;
}

/* Return a VALUE for S.  */

స్థిర విలువ *str_విలువ(స్థిరరాశి అక్షరం *s)
{
	విలువ *v;

	v = xzalloc(పరిమాణం(విలువ));
	ఒకవేళ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	తిరిగి v;
}

/* Free VALUE V, including structure components.  */

స్థిర శూన్య freev(విలువ *v)
{
	ఒకవేళ (v->type == STRING)
		విడుదల(v->u.s);
	విడుదల(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

స్థిర పూర్ణాంకం null(విలువ *v)
{
	ఒకవేళ (v->type == INTEGER)
		తిరిగి v->u.i == 0;
	/* STRING: */
	తిరిగి v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

స్థిర శూన్య tostring(విలువ *v)
{
	ఒకవేళ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

స్థిర bool toarith(విలువ *v)
{
	ఒకవేళ (v->type == STRING) {
		arith_t i;
		అక్షరం *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ఒకవేళ ((v->u.s == e) || *e)
			తిరిగి 0;
		విడుదల(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	తిరిగి 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

స్థిర పూర్ణాంకం nextarg(స్థిరరాశి అక్షరం *str)
{
	ఒకవేళ (*G.వాదనలు == ఖాళీ || strcmp(*G.వాదనలు, str) != 0)
		తిరిగి 0;
	తిరిగి (గుర్తులేని అక్షరం)str[0] + (గుర్తులేని అక్షరం)str[1];
}

/* The comparison operator handling functions.  */

స్థిర పూర్ణాంకం cmp_common(విలువ *l, విలువ *r, పూర్ణాంకం op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ఒకవేళ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ఒకవేళ (op == '<')
		తిరిగి ll < rr;
	ఒకవేళ (op == ('<' + '='))
		తిరిగి ll <= rr;
	ఒకవేళ (op == '=' || (op == '=' + '='))
		తిరిగి ll == rr;
	ఒకవేళ (op == '!' + '=')
		తిరిగి ll != rr;
	ఒకవేళ (op == '>')
		తిరిగి ll > rr;
	/* >= */
	తిరిగి ll >= rr;
}

/* The arithmetic operator handling functions.  */

స్థిర arith_t arithmetic_common(విలువ *l, విలువ *r, పూర్ణాంకం op)
{
	arith_t li, ri;

	ఒకవేళ (!toarith(l) || !toarith(r))
		bb_simple_error_సందేశం_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ఒకవేళ (op == '+')
		తిరిగి li + ri;
	ఒకవేళ (op == '-')
		తిరిగి li - ri;
	ఒకవేళ (op == '*')
		తిరిగి li * ri;
	ఒకవేళ (ri == 0)
		bb_simple_error_సందేశం_and_die("division by zero");
	ఒకవేళ (op == '/')
		తిరిగి li / ri;
	తిరిగి li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

స్థిర విలువ *docolon(విలువ *sv, విలువ *pv)
{
	గణన { NMATCH = 2 };
	విలువ *v;
	regex_t re_బఫరు;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ఒకవేళ (pv->u.s[0] == '^') {
		bb_error_సందేశం(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_బఫరు, 0, పరిమాణం(re_బఫరు));
	memset(re_regs, 0, పరిమాణం(re_regs));
	xregcomp(&re_బఫరు, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ఒకవేళ (regexec(&re_బఫరు, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ఒకవేళ (re_బఫరు.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_విలువ(sv->u.s + re_regs[1].rm_so);
		} లేకపోతే {
			v = int_విలువ(re_regs[0].rm_eo);
		}
	} లేకపోతే {
		/* Match failed -- return the right kind of null.  */
		ఒకవేళ (re_బఫరు.re_nsub > 0)
			v = str_విలువ("");
		లేకపోతే
			v = int_విలువ(0);
	}
	regfree(&re_బఫరు);
	తిరిగి v;
}

/* Handle bare operands and ( expr ) syntax.  */

స్థిర విలువ *eval7(శూన్య)
{
	విలువ *v;

	ఒకవేళ (!*G.వాదనలు)
		bb_simple_error_సందేశం_and_die("syntax error");

	ఒకవేళ (nextarg("(")) {
		G.వాదనలు++;
		v = eval();
		ఒకవేళ (!nextarg(")"))
			bb_simple_error_సందేశం_and_die("syntax error");
		G.వాదనలు++;
		తిరిగి v;
	}

	ఒకవేళ (nextarg(")"))
		bb_simple_error_సందేశం_and_die("syntax error");

	తిరిగి str_విలువ(*G.వాదనలు++);
}

/* Handle match, substr, index, length, and quote keywords.  */

స్థిర విలువ *eval6(శూన్య)
{
	స్థిర స్థిరరాశి అక్షరం keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	విలువ *r, *i1, *i2;
	విలువ *l = l; /* silence gcc */
	విలువ *v = v; /* silence gcc */
	పూర్ణాంకం కీలకం = *G.వాదనలు ? సూచీ_లోపల_అక్షరశ్రేణులు(keywords, *G.వాదనలు) + 1 : 0;

	ఒకవేళ (కీలకం == 0) /* not a keyword */
		తిరిగి eval7();
	G.వాదనలు++; /* We have a valid token, so get the next argument.  */
	ఒకవేళ (కీలకం == 1) { /* quote */
		ఒకవేళ (!*G.వాదనలు)
			bb_simple_error_సందేశం_and_die("syntax error");
		తిరిగి str_విలువ(*G.వాదనలు++);
	}
	ఒకవేళ (కీలకం == 2) { /*నిడివి*/
		r = eval6();
		tostring(r);
		v = int_విలువ(అక్షరశ్రేణి_పొడవు(r->u.s));
		freev(r);
	} లేకపోతే
		l = eval6();

	ఒకవేళ (కీలకం == 3) { /*సరిపోలిక*/
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ఒకవేళ (కీలకం == 4) { /*సూచీ*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_విలువ(strcspn(l->u.s, r->u.s) + 1);
		ఒకవేళ (v->u.i == (arith_t) అక్షరశ్రేణి_పొడవు(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ఒకవేళ (కీలకం == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ఒకవేళ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) అక్షరశ్రేణి_పొడవు(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_విలువ("");
		లేకపోతే {
			v = xmalloc(పరిమాణం(విలువ));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	తిరిగి v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

స్థిర విలువ *eval5(శూన్య)
{
	విలువ *l, *r, *v;

	l = eval6();
	అయితే (nextarg(":")) {
		G.వాదనలు++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	తిరిగి l;
}

/* Handle *, /, % operators.  */

స్థిర విలువ *eval4(శూన్య)
{
	విలువ *l, *r;
	పూర్ణాంకం op;
	arith_t val;

	l = eval5();
	అయితే (1) {
		op = nextarg("*");
		ఒకవేళ (!op) { op = nextarg("/");
		 ఒకవేళ (!op) { op = nextarg("%");
		  ఒకవేళ (!op) తిరిగి l;
		}}
		G.వాదనలు++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_విలువ(val);
	}
}

/* Handle +, - operators.  */

స్థిర విలువ *eval3(శూన్య)
{
	విలువ *l, *r;
	పూర్ణాంకం op;
	arith_t val;

	l = eval4();
	అయితే (1) {
		op = nextarg("+");
		ఒకవేళ (!op) {
			op = nextarg("-");
			ఒకవేళ (!op) తిరిగి l;
		}
		G.వాదనలు++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_విలువ(val);
	}
}

/* Handle comparisons.  */

స్థిర విలువ *eval2(శూన్య)
{
	విలువ *l, *r;
	పూర్ణాంకం op;
	arith_t val;

	l = eval3();
	అయితే (1) {
		op = nextarg("<");
		ఒకవేళ (!op) { op = nextarg("<=");
		 ఒకవేళ (!op) { op = nextarg("=");
		  ఒకవేళ (!op) { op = nextarg("==");
		   ఒకవేళ (!op) { op = nextarg("!=");
		    ఒకవేళ (!op) { op = nextarg(">=");
		     ఒకవేళ (!op) { op = nextarg(">");
		      ఒకవేళ (!op) తిరిగి l;
		}}}}}}
		G.వాదనలు++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_విలువ(val);
	}
}

/* Handle &.  */

స్థిర విలువ *eval1(శూన్య)
{
	విలువ *l, *r;

	l = eval2();
	అయితే (nextarg("&")) {
		G.వాదనలు++;
		r = eval2();
		ఒకవేళ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_విలువ(0);
		} లేకపోతే
			freev(r);
	}
	తిరిగి l;
}

/* Handle |.  */

స్థిర విలువ *eval(శూన్య)
{
	విలువ *l, *r;

	l = eval1();
	అయితే (nextarg("|")) {
		G.వాదనలు++;
		r = eval1();
		ఒకవేళ (null(l)) {
			freev(l);
			l = r;
		} లేకపోతే
			freev(r);
	}
	తిరిగి l;
}

పూర్ణాంకం సమాసం_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం సమాసం_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	విలువ *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.వాదనలు = argv + 1;
	ఒకవేళ (*G.వాదనలు == ఖాళీ) {
		bb_simple_error_సందేశం_and_die("too few arguments");
	}
	v = eval();
	ఒకవేళ (*G.వాదనలు)
		bb_simple_error_సందేశం_and_die("syntax error");
	ఒకవేళ (v->type == INTEGER)
		రాయి("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	లేకపోతే
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
