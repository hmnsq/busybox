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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "common_bufsiz.h"
#ଅନ୍ତର୍ଭାଵ "xregex.h"

#ଯଦି ENABLE_ପଦସମଷ୍ଟି_MATH_SUPPORT_64
ପ୍ରକାରବ୍ୟାଖ୍ୟା int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#ଅନ୍ୟଥା
ପ୍ରକାରବ୍ୟାଖ୍ୟା long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ଗଣନା {
	INTEGER,
	STRING
};

/* A value is.... */
ସଂରଚନା valinfo {
	smallint type;                  /* Which kind. */
	ସଂଘ {                         /* The value itself. */
		arith_t i;
		ଅକ୍ଷର *s;
	} u;
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା valinfo ମୂଲ୍ୟ;

/* The arguments given to the program, minus the program name.  */
ସଂରଚନା globals {
	ଅକ୍ଷର **args;
} FIX_ALIASING;
#define G (*(ସଂରଚନା globals*)bb_common_bufsiz1)
#define INIT_G() କର { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ଯେତେବେଳେ (0)

/* forward declarations */
ସ୍ଥିର ମୂଲ୍ୟ *eval(ଶୂନ୍ୟ);


/* Return a VALUE for I.  */

ସ୍ଥିର ମୂଲ୍ୟ *int_ମୂଲ୍ୟ(arith_t i)
{
	ମୂଲ୍ୟ *v;

	v = xzalloc(ଆକାର(ମୂଲ୍ୟ));
	ଯଦି (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	ଫେରନ୍ତୁ v;
}

/* Return a VALUE for S.  */

ସ୍ଥିର ମୂଲ୍ୟ *str_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *s)
{
	ମୂଲ୍ୟ *v;

	v = xzalloc(ଆକାର(ମୂଲ୍ୟ));
	ଯଦି (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	ଫେରନ୍ତୁ v;
}

/* Free VALUE V, including structure components.  */

ସ୍ଥିର ଶୂନ୍ୟ freev(ମୂଲ୍ୟ *v)
{
	ଯଦି (v->type == STRING)
		ମୁକ୍ତ(v->u.s);
	ମୁକ୍ତ(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା null(ମୂଲ୍ୟ *v)
{
	ଯଦି (v->type == INTEGER)
		ଫେରନ୍ତୁ v->u.i == 0;
	/* STRING: */
	ଫେରନ୍ତୁ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ସ୍ଥିର ଶୂନ୍ୟ tostring(ମୂଲ୍ୟ *v)
{
	ଯଦି (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ସ୍ଥିର bool toarith(ମୂଲ୍ୟ *v)
{
	ଯଦି (v->type == STRING) {
		arith_t i;
		ଅକ୍ଷର *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		ଯଦି ((v->u.s == e) || *e)
			ଫେରନ୍ତୁ 0;
		ମୁକ୍ତ(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	ଫେରନ୍ତୁ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextarg(ଅଚଳ ଅକ୍ଷର *str)
{
	ଯଦି (*G.args == ଖାଲି || strcmp(*G.args, str) != 0)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)str[0] + (unsigned ଅକ୍ଷର)str[1];
}

/* The comparison operator handling functions.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp_common(ମୂଲ୍ୟ *l, ମୂଲ୍ୟ *r, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	ଯଦି (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	ଯଦି (op == '<')
		ଫେରନ୍ତୁ ll < rr;
	ଯଦି (op == ('<' + '='))
		ଫେରନ୍ତୁ ll <= rr;
	ଯଦି (op == '=' || (op == '=' + '='))
		ଫେରନ୍ତୁ ll == rr;
	ଯଦି (op == '!' + '=')
		ଫେରନ୍ତୁ ll != rr;
	ଯଦି (op == '>')
		ଫେରନ୍ତୁ ll > rr;
	/* >= */
	ଫେରନ୍ତୁ ll >= rr;
}

/* The arithmetic operator handling functions.  */

ସ୍ଥିର arith_t arithmetic_common(ମୂଲ୍ୟ *l, ମୂଲ୍ୟ *r, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op)
{
	arith_t li, ri;

	ଯଦି (!toarith(l) || !toarith(r))
		bb_simple_error_ସନ୍ଦେଶ_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	ଯଦି (op == '+')
		ଫେରନ୍ତୁ li + ri;
	ଯଦି (op == '-')
		ଫେରନ୍ତୁ li - ri;
	ଯଦି (op == '*')
		ଫେରନ୍ତୁ li * ri;
	ଯଦି (ri == 0)
		bb_simple_error_ସନ୍ଦେଶ_and_die("division by zero");
	ଯଦି (op == '/')
		ଫେରନ୍ତୁ li / ri;
	ଫେରନ୍ତୁ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ସ୍ଥିର ମୂଲ୍ୟ *docolon(ମୂଲ୍ୟ *sv, ମୂଲ୍ୟ *pv)
{
	ଗଣନା { NMATCH = 2 };
	ମୂଲ୍ୟ *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	ଯଦି (pv->u.s[0] == '^') {
		bb_error_ସନ୍ଦେଶ(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, ଆକାର(re_buffer));
	memset(re_regs, 0, ଆକାର(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	ଯଦି (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		ଯଦି (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_ମୂଲ୍ୟ(sv->u.s + re_regs[1].rm_so);
		} ଅନ୍ୟଥା {
			v = int_ମୂଲ୍ୟ(re_regs[0].rm_eo);
		}
	} ଅନ୍ୟଥା {
		/* Match failed -- return the right kind of null.  */
		ଯଦି (re_buffer.re_nsub > 0)
			v = str_ମୂଲ୍ୟ("");
		ଅନ୍ୟଥା
			v = int_ମୂଲ୍ୟ(0);
	}
	regfree(&re_buffer);
	ଫେରନ୍ତୁ v;
}

/* Handle bare operands and ( expr ) syntax.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval7(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *v;

	ଯଦି (!*G.args)
		bb_simple_error_ସନ୍ଦେଶ_and_die("syntax error");

	ଯଦି (nextarg("(")) {
		G.args++;
		v = eval();
		ଯଦି (!nextarg(")"))
			bb_simple_error_ସନ୍ଦେଶ_and_die("syntax error");
		G.args++;
		ଫେରନ୍ତୁ v;
	}

	ଯଦି (nextarg(")"))
		bb_simple_error_ସନ୍ଦେଶ_and_die("syntax error");

	ଫେରନ୍ତୁ str_ମୂଲ୍ୟ(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval6(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	ମୂଲ୍ୟ *r, *i1, *i2;
	ମୂଲ୍ୟ *l = l; /* silence gcc */
	ମୂଲ୍ୟ *v = v; /* silence gcc */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	ଯଦି (ଚାବି == 0) /* not a keyword */
		ଫେରନ୍ତୁ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	ଯଦି (ଚାବି == 1) { /* quote */
		ଯଦି (!*G.args)
			bb_simple_error_ସନ୍ଦେଶ_and_die("syntax error");
		ଫେରନ୍ତୁ str_ମୂଲ୍ୟ(*G.args++);
	}
	ଯଦି (ଚାବି == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_ମୂଲ୍ୟ(strlen(r->u.s));
		freev(r);
	} ଅନ୍ୟଥା
		l = eval6();

	ଯଦି (ଚାବି == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	ଯଦି (ଚାବି == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_ମୂଲ୍ୟ(strcspn(l->u.s, r->u.s) + 1);
		ଯଦି (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	ଯଦି (ଚାବି == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		ଯଦି (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_ମୂଲ୍ୟ("");
		ଅନ୍ୟଥା {
			v = xmalloc(ଆକାର(ମୂଲ୍ୟ));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	ଫେରନ୍ତୁ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval5(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r, *v;

	l = eval6();
	ଯେତେବେଳେ (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	ଫେରନ୍ତୁ l;
}

/* Handle *, /, % operators.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval4(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
	arith_t val;

	l = eval5();
	ଯେତେବେଳେ (1) {
		op = nextarg("*");
		ଯଦି (!op) { op = nextarg("/");
		 ଯଦି (!op) { op = nextarg("%");
		  ଯଦି (!op) ଫେରନ୍ତୁ l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ମୂଲ୍ୟ(val);
	}
}

/* Handle +, - operators.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval3(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
	arith_t val;

	l = eval4();
	ଯେତେବେଳେ (1) {
		op = nextarg("+");
		ଯଦି (!op) {
			op = nextarg("-");
			ଯଦି (!op) ଫେରନ୍ତୁ l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ମୂଲ୍ୟ(val);
	}
}

/* Handle comparisons.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval2(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
	arith_t val;

	l = eval3();
	ଯେତେବେଳେ (1) {
		op = nextarg("<");
		ଯଦି (!op) { op = nextarg("<=");
		 ଯଦି (!op) { op = nextarg("=");
		  ଯଦି (!op) { op = nextarg("==");
		   ଯଦି (!op) { op = nextarg("!=");
		    ଯଦି (!op) { op = nextarg(">=");
		     ଯଦି (!op) { op = nextarg(">");
		      ଯଦି (!op) ଫେରନ୍ତୁ l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_ମୂଲ୍ୟ(val);
	}
}

/* Handle &.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval1(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r;

	l = eval2();
	ଯେତେବେଳେ (nextarg("&")) {
		G.args++;
		r = eval2();
		ଯଦି (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_ମୂଲ୍ୟ(0);
		} ଅନ୍ୟଥା
			freev(r);
	}
	ଫେରନ୍ତୁ l;
}

/* Handle |.  */

ସ୍ଥିର ମୂଲ୍ୟ *eval(ଶୂନ୍ୟ)
{
	ମୂଲ୍ୟ *l, *r;

	l = eval1();
	ଯେତେବେଳେ (nextarg("|")) {
		G.args++;
		r = eval1();
		ଯଦି (null(l)) {
			freev(l);
			l = r;
		} ଅନ୍ୟଥା
			freev(r);
	}
	ଫେରନ୍ତୁ l;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ମୂଲ୍ୟ *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	ଯଦି (*G.args == ଖାଲି) {
		bb_simple_error_ସନ୍ଦେଶ_and_die("too few arguments");
	}
	v = eval();
	ଯଦି (*G.args)
		bb_simple_error_ସନ୍ଦେଶ_and_die("syntax error");
	ଯଦି (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	ଅନ୍ୟଥା
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
