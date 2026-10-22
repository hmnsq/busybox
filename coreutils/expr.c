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

#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ "common_bufsiz.h"
#અન્તર્ભાવ "xregex.h"

#જો ENABLE_પદાવલિ_MATH_SUPPORT_64
પ્રકારવ્યાખ્યા int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#નહીંતર
પ્રકારવ્યાખ્યા long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
ગણના {
	INTEGER,
	STRING
};

/* A value is.... */
સંરચના valinfo {
	smallint type;                  /* Which kind. */
	સંઘ {                         /* The value itself. */
		arith_t i;
		અક્ષર *s;
	} u;
};
પ્રકારવ્યાખ્યા સંરચના valinfo કિંમત;

/* The arguments given to the program, minus the program name.  */
સંરચના globals {
	અક્ષર **args;
} FIX_ALIASING;
#define G (*(સંરચના globals*)bb_common_bufsiz1)
#define INIT_G() કરો { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} જ્યારે (0)

/* forward declarations */
સ્થિર કિંમત *eval(શૂન્ય);


/* Return a VALUE for I.  */

સ્થિર કિંમત *int_કિંમત(arith_t i)
{
	કિંમત *v;

	v = xzalloc(કદ(કિંમત));
	જો (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	પાછા v;
}

/* Return a VALUE for S.  */

સ્થિર કિંમત *str_કિંમત(અચલ અક્ષર *s)
{
	કિંમત *v;

	v = xzalloc(કદ(કિંમત));
	જો (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	પાછા v;
}

/* Free VALUE V, including structure components.  */

સ્થિર શૂન્ય freev(કિંમત *v)
{
	જો (v->type == STRING)
		મુક્ત(v->u.s);
	મુક્ત(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

સ્થિર પૂર્ણાંક null(કિંમત *v)
{
	જો (v->type == INTEGER)
		પાછા v->u.i == 0;
	/* STRING: */
	પાછા v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

સ્થિર શૂન્ય tostring(કિંમત *v)
{
	જો (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

સ્થિર bool toarith(કિંમત *v)
{
	જો (v->type == STRING) {
		arith_t i;
		અક્ષર *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		જો ((v->u.s == e) || *e)
			પાછા 0;
		મુક્ત(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	પાછા 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

સ્થિર પૂર્ણાંક nextarg(અચલ અક્ષર *str)
{
	જો (*G.args == ખાલી || strcmp(*G.args, str) != 0)
		પાછા 0;
	પાછા (unsigned અક્ષર)str[0] + (unsigned અક્ષર)str[1];
}

/* The comparison operator handling functions.  */

સ્થિર પૂર્ણાંક cmp_common(કિંમત *l, કિંમત *r, પૂર્ણાંક op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	જો (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	જો (op == '<')
		પાછા ll < rr;
	જો (op == ('<' + '='))
		પાછા ll <= rr;
	જો (op == '=' || (op == '=' + '='))
		પાછા ll == rr;
	જો (op == '!' + '=')
		પાછા ll != rr;
	જો (op == '>')
		પાછા ll > rr;
	/* >= */
	પાછા ll >= rr;
}

/* The arithmetic operator handling functions.  */

સ્થિર arith_t arithmetic_common(કિંમત *l, કિંમત *r, પૂર્ણાંક op)
{
	arith_t li, ri;

	જો (!toarith(l) || !toarith(r))
		bb_simple_error_સંદેશ_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	જો (op == '+')
		પાછા li + ri;
	જો (op == '-')
		પાછા li - ri;
	જો (op == '*')
		પાછા li * ri;
	જો (ri == 0)
		bb_simple_error_સંદેશ_and_die("division by zero");
	જો (op == '/')
		પાછા li / ri;
	પાછા li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

સ્થિર કિંમત *docolon(કિંમત *sv, કિંમત *pv)
{
	ગણના { NMATCH = 2 };
	કિંમત *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	જો (pv->u.s[0] == '^') {
		bb_error_સંદેશ(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, કદ(re_buffer));
	memset(re_regs, 0, કદ(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	જો (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		જો (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_કિંમત(sv->u.s + re_regs[1].rm_so);
		} નહીંતર {
			v = int_કિંમત(re_regs[0].rm_eo);
		}
	} નહીંતર {
		/* Match failed -- return the right kind of null.  */
		જો (re_buffer.re_nsub > 0)
			v = str_કિંમત("");
		નહીંતર
			v = int_કિંમત(0);
	}
	regfree(&re_buffer);
	પાછા v;
}

/* Handle bare operands and ( expr ) syntax.  */

સ્થિર કિંમત *eval7(શૂન્ય)
{
	કિંમત *v;

	જો (!*G.args)
		bb_simple_error_સંદેશ_and_die("syntax error");

	જો (nextarg("(")) {
		G.args++;
		v = eval();
		જો (!nextarg(")"))
			bb_simple_error_સંદેશ_and_die("syntax error");
		G.args++;
		પાછા v;
	}

	જો (nextarg(")"))
		bb_simple_error_સંદેશ_and_die("syntax error");

	પાછા str_કિંમત(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

સ્થિર કિંમત *eval6(શૂન્ય)
{
	સ્થિર અચલ અક્ષર keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	કિંમત *r, *i1, *i2;
	કિંમત *l = l; /* silence gcc */
	કિંમત *v = v; /* silence gcc */
	પૂર્ણાંક ચાવી = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	જો (ચાવી == 0) /* not a keyword */
		પાછા eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	જો (ચાવી == 1) { /* quote */
		જો (!*G.args)
			bb_simple_error_સંદેશ_and_die("syntax error");
		પાછા str_કિંમત(*G.args++);
	}
	જો (ચાવી == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_કિંમત(strlen(r->u.s));
		freev(r);
	} નહીંતર
		l = eval6();

	જો (ચાવી == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	જો (ચાવી == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_કિંમત(strcspn(l->u.s, r->u.s) + 1);
		જો (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	જો (ચાવી == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		જો (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_કિંમત("");
		નહીંતર {
			v = xmalloc(કદ(કિંમત));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	પાછા v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

સ્થિર કિંમત *eval5(શૂન્ય)
{
	કિંમત *l, *r, *v;

	l = eval6();
	જ્યારે (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	પાછા l;
}

/* Handle *, /, % operators.  */

સ્થિર કિંમત *eval4(શૂન્ય)
{
	કિંમત *l, *r;
	પૂર્ણાંક op;
	arith_t val;

	l = eval5();
	જ્યારે (1) {
		op = nextarg("*");
		જો (!op) { op = nextarg("/");
		 જો (!op) { op = nextarg("%");
		  જો (!op) પાછા l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_કિંમત(val);
	}
}

/* Handle +, - operators.  */

સ્થિર કિંમત *eval3(શૂન્ય)
{
	કિંમત *l, *r;
	પૂર્ણાંક op;
	arith_t val;

	l = eval4();
	જ્યારે (1) {
		op = nextarg("+");
		જો (!op) {
			op = nextarg("-");
			જો (!op) પાછા l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_કિંમત(val);
	}
}

/* Handle comparisons.  */

સ્થિર કિંમત *eval2(શૂન્ય)
{
	કિંમત *l, *r;
	પૂર્ણાંક op;
	arith_t val;

	l = eval3();
	જ્યારે (1) {
		op = nextarg("<");
		જો (!op) { op = nextarg("<=");
		 જો (!op) { op = nextarg("=");
		  જો (!op) { op = nextarg("==");
		   જો (!op) { op = nextarg("!=");
		    જો (!op) { op = nextarg(">=");
		     જો (!op) { op = nextarg(">");
		      જો (!op) પાછા l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_કિંમત(val);
	}
}

/* Handle &.  */

સ્થિર કિંમત *eval1(શૂન્ય)
{
	કિંમત *l, *r;

	l = eval2();
	જ્યારે (nextarg("&")) {
		G.args++;
		r = eval2();
		જો (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_કિંમત(0);
		} નહીંતર
			freev(r);
	}
	પાછા l;
}

/* Handle |.  */

સ્થિર કિંમત *eval(શૂન્ય)
{
	કિંમત *l, *r;

	l = eval1();
	જ્યારે (nextarg("|")) {
		G.args++;
		r = eval1();
		જો (null(l)) {
			freev(l);
			l = r;
		} નહીંતર
			freev(r);
	}
	પાછા l;
}

પૂર્ણાંક પદાવલિ_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક પદાવલિ_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	કિંમત *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	જો (*G.args == ખાલી) {
		bb_simple_error_સંદેશ_and_die("too few arguments");
	}
	v = eval();
	જો (*G.args)
		bb_simple_error_સંદેશ_and_die("syntax error");
	જો (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	નહીંતર
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
