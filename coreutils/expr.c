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

#إذا ENABLE_ئىپادە_MATH_SUPPORT_64
تعريفنوع int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#وإلا
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
تعريفنوع بنية valinfo قىممەت;

/* The arguments given to the program, minus the program name.  */
بنية globals {
	حرف **args;
} FIX_ALIASING;
#define G (*(بنية globals*)bb_common_bufsiz1)
#define INIT_G() افعل { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} طالما (0)

/* forward declarations */
ساكن قىممەت *eval(فراغ);


/* Return a VALUE for I.  */

ساكن قىممەت *int_قىممەت(arith_t i)
{
	قىممەت *v;

	v = xzalloc(حجم(قىممەت));
	إذا (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	أرجع v;
}

/* Return a VALUE for S.  */

ساكن قىممەت *str_قىممەت(ثابت حرف *s)
{
	قىممەت *v;

	v = xzalloc(حجم(قىممەت));
	إذا (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	أرجع v;
}

/* Free VALUE V, including structure components.  */

ساكن فراغ freev(قىممەت *v)
{
	إذا (v->type == STRING)
		حرر(v->u.s);
	حرر(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

ساكن صحيح null(قىممەت *v)
{
	إذا (v->type == INTEGER)
		أرجع v->u.i == 0;
	/* STRING: */
	أرجع v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

ساكن فراغ tostring(قىممەت *v)
{
	إذا (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

ساكن bool toarith(قىممەت *v)
{
	إذا (v->type == STRING) {
		arith_t i;
		حرف *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		إذا ((v->u.s == e) || *e)
			أرجع 0;
		حرر(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	أرجع 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

ساكن صحيح nextarg(ثابت حرف *str)
{
	إذا (*G.args == عدم || strcmp(*G.args, str) != 0)
		أرجع 0;
	أرجع (unsigned حرف)str[0] + (unsigned حرف)str[1];
}

/* The comparison operator handling functions.  */

ساكن صحيح cmp_common(قىممەت *l, قىممەت *r, صحيح op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	إذا (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	إذا (op == '<')
		أرجع ll < rr;
	إذا (op == ('<' + '='))
		أرجع ll <= rr;
	إذا (op == '=' || (op == '=' + '='))
		أرجع ll == rr;
	إذا (op == '!' + '=')
		أرجع ll != rr;
	إذا (op == '>')
		أرجع ll > rr;
	/* >= */
	أرجع ll >= rr;
}

/* The arithmetic operator handling functions.  */

ساكن arith_t arithmetic_common(قىممەت *l, قىممەت *r, صحيح op)
{
	arith_t li, ri;

	إذا (!toarith(l) || !toarith(r))
		bb_simple_error_ئۇچۇر_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	إذا (op == '+')
		أرجع li + ri;
	إذا (op == '-')
		أرجع li - ri;
	إذا (op == '*')
		أرجع li * ri;
	إذا (ri == 0)
		bb_simple_error_ئۇچۇر_and_die("division by zero");
	إذا (op == '/')
		أرجع li / ri;
	أرجع li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

ساكن قىممەت *docolon(قىممەت *sv, قىممەت *pv)
{
	تعداد { NMATCH = 2 };
	قىممەت *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	إذا (pv->u.s[0] == '^') {
		bb_error_ئۇچۇر(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, حجم(re_buffer));
	memset(re_regs, 0, حجم(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	إذا (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		إذا (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_قىممەت(sv->u.s + re_regs[1].rm_so);
		} وإلا {
			v = int_قىممەت(re_regs[0].rm_eo);
		}
	} وإلا {
		/* Match failed -- return the right kind of null.  */
		إذا (re_buffer.re_nsub > 0)
			v = str_قىممەت("");
		وإلا
			v = int_قىممەت(0);
	}
	regfree(&re_buffer);
	أرجع v;
}

/* Handle bare operands and ( expr ) syntax.  */

ساكن قىممەت *eval7(فراغ)
{
	قىممەت *v;

	إذا (!*G.args)
		bb_simple_error_ئۇچۇر_and_die("syntax error");

	إذا (nextarg("(")) {
		G.args++;
		v = eval();
		إذا (!nextarg(")"))
			bb_simple_error_ئۇچۇر_and_die("syntax error");
		G.args++;
		أرجع v;
	}

	إذا (nextarg(")"))
		bb_simple_error_ئۇچۇر_and_die("syntax error");

	أرجع str_قىممەت(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

ساكن قىممەت *eval6(فراغ)
{
	ساكن ثابت حرف keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	قىممەت *r, *i1, *i2;
	قىممەت *l = l; /* silence gcc */
	قىممەت *v = v; /* silence gcc */
	صحيح ئاچقۇچ = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	إذا (ئاچقۇچ == 0) /* not a keyword */
		أرجع eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	إذا (ئاچقۇچ == 1) { /* quote */
		إذا (!*G.args)
			bb_simple_error_ئۇچۇر_and_die("syntax error");
		أرجع str_قىممەت(*G.args++);
	}
	إذا (ئاچقۇچ == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_قىممەت(strlen(r->u.s));
		freev(r);
	} وإلا
		l = eval6();

	إذا (ئاچقۇچ == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	إذا (ئاچقۇچ == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_قىممەت(strcspn(l->u.s, r->u.s) + 1);
		إذا (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	إذا (ئاچقۇچ == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		إذا (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_قىممەت("");
		وإلا {
			v = xmalloc(حجم(قىممەت));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	أرجع v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

ساكن قىممەت *eval5(فراغ)
{
	قىممەت *l, *r, *v;

	l = eval6();
	طالما (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	أرجع l;
}

/* Handle *, /, % operators.  */

ساكن قىممەت *eval4(فراغ)
{
	قىممەت *l, *r;
	صحيح op;
	arith_t val;

	l = eval5();
	طالما (1) {
		op = nextarg("*");
		إذا (!op) { op = nextarg("/");
		 إذا (!op) { op = nextarg("%");
		  إذا (!op) أرجع l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قىممەت(val);
	}
}

/* Handle +, - operators.  */

ساكن قىممەت *eval3(فراغ)
{
	قىممەت *l, *r;
	صحيح op;
	arith_t val;

	l = eval4();
	طالما (1) {
		op = nextarg("+");
		إذا (!op) {
			op = nextarg("-");
			إذا (!op) أرجع l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قىممەت(val);
	}
}

/* Handle comparisons.  */

ساكن قىممەت *eval2(فراغ)
{
	قىممەت *l, *r;
	صحيح op;
	arith_t val;

	l = eval3();
	طالما (1) {
		op = nextarg("<");
		إذا (!op) { op = nextarg("<=");
		 إذا (!op) { op = nextarg("=");
		  إذا (!op) { op = nextarg("==");
		   إذا (!op) { op = nextarg("!=");
		    إذا (!op) { op = nextarg(">=");
		     إذا (!op) { op = nextarg(">");
		      إذا (!op) أرجع l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_قىممەت(val);
	}
}

/* Handle &.  */

ساكن قىممەت *eval1(فراغ)
{
	قىممەت *l, *r;

	l = eval2();
	طالما (nextarg("&")) {
		G.args++;
		r = eval2();
		إذا (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_قىممەت(0);
		} وإلا
			freev(r);
	}
	أرجع l;
}

/* Handle |.  */

ساكن قىممەت *eval(فراغ)
{
	قىممەت *l, *r;

	l = eval1();
	طالما (nextarg("|")) {
		G.args++;
		r = eval1();
		إذا (null(l)) {
			freev(l);
			l = r;
		} وإلا
			freev(r);
	}
	أرجع l;
}

صحيح ئىپادە_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح ئىپادە_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	قىممەت *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	إذا (*G.args == عدم) {
		bb_simple_error_ئۇچۇر_and_die("too few arguments");
	}
	v = eval();
	إذا (*G.args)
		bb_simple_error_ئۇچۇر_and_die("syntax error");
	إذا (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	وإلا
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
