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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"
#அந்தர்பாவ "xregex.h"

#எனில் ENABLE_கோவை_MATH_SUPPORT_64
வகைவரை int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#இல்லையெனில்
வகைவரை long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
எண்ணிக்கை {
	INTEGER,
	STRING
};

/* A value is.... */
கட்டமைப்பு valinfo {
	smallint type;                  /* Which kind. */
	ஒன்றிப்பு {                         /* The value itself. */
		arith_t i;
		எழுத்து *s;
	} u;
};
வகைவரை கட்டமைப்பு valinfo மதிப்பு;

/* The arguments given to the program, minus the program name.  */
கட்டமைப்பு globals {
	எழுத்து **args;
} FIX_ALIASING;
#define G (*(கட்டமைப்பு globals*)bb_common_bufsiz1)
#define INIT_G() செய் { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} போது (0)

/* forward declarations */
நிலையான மதிப்பு *eval(வெறுமை);


/* Return a VALUE for I.  */

நிலையான மதிப்பு *int_மதிப்பு(arith_t i)
{
	மதிப்பு *v;

	v = xzalloc(அளவு(மதிப்பு));
	எனில் (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	திரும்பு v;
}

/* Return a VALUE for S.  */

நிலையான மதிப்பு *str_மதிப்பு(மாறிலி எழுத்து *s)
{
	மதிப்பு *v;

	v = xzalloc(அளவு(மதிப்பு));
	எனில் (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	திரும்பு v;
}

/* Free VALUE V, including structure components.  */

நிலையான வெறுமை freev(மதிப்பு *v)
{
	எனில் (v->type == STRING)
		விடுவி(v->u.s);
	விடுவி(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

நிலையான முழு null(மதிப்பு *v)
{
	எனில் (v->type == INTEGER)
		திரும்பு v->u.i == 0;
	/* STRING: */
	திரும்பு v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

நிலையான வெறுமை tostring(மதிப்பு *v)
{
	எனில் (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

நிலையான bool toarith(மதிப்பு *v)
{
	எனில் (v->type == STRING) {
		arith_t i;
		எழுத்து *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		எனில் ((v->u.s == e) || *e)
			திரும்பு 0;
		விடுவி(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	திரும்பு 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

நிலையான முழு nextarg(மாறிலி எழுத்து *str)
{
	எனில் (*G.args == வெற்று || strcmp(*G.args, str) != 0)
		திரும்பு 0;
	திரும்பு (unsigned எழுத்து)str[0] + (unsigned எழுத்து)str[1];
}

/* The comparison operator handling functions.  */

நிலையான முழு cmp_common(மதிப்பு *l, மதிப்பு *r, முழு op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	எனில் (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	எனில் (op == '<')
		திரும்பு ll < rr;
	எனில் (op == ('<' + '='))
		திரும்பு ll <= rr;
	எனில் (op == '=' || (op == '=' + '='))
		திரும்பு ll == rr;
	எனில் (op == '!' + '=')
		திரும்பு ll != rr;
	எனில் (op == '>')
		திரும்பு ll > rr;
	/* >= */
	திரும்பு ll >= rr;
}

/* The arithmetic operator handling functions.  */

நிலையான arith_t arithmetic_common(மதிப்பு *l, மதிப்பு *r, முழு op)
{
	arith_t li, ri;

	எனில் (!toarith(l) || !toarith(r))
		bb_simple_error_செய்தி_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	எனில் (op == '+')
		திரும்பு li + ri;
	எனில் (op == '-')
		திரும்பு li - ri;
	எனில் (op == '*')
		திரும்பு li * ri;
	எனில் (ri == 0)
		bb_simple_error_செய்தி_and_die("division by zero");
	எனில் (op == '/')
		திரும்பு li / ri;
	திரும்பு li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

நிலையான மதிப்பு *docolon(மதிப்பு *sv, மதிப்பு *pv)
{
	எண்ணிக்கை { NMATCH = 2 };
	மதிப்பு *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	எனில் (pv->u.s[0] == '^') {
		bb_error_செய்தி(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, அளவு(re_buffer));
	memset(re_regs, 0, அளவு(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	எனில் (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		எனில் (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_மதிப்பு(sv->u.s + re_regs[1].rm_so);
		} இல்லையெனில் {
			v = int_மதிப்பு(re_regs[0].rm_eo);
		}
	} இல்லையெனில் {
		/* Match failed -- return the right kind of null.  */
		எனில் (re_buffer.re_nsub > 0)
			v = str_மதிப்பு("");
		இல்லையெனில்
			v = int_மதிப்பு(0);
	}
	regfree(&re_buffer);
	திரும்பு v;
}

/* Handle bare operands and ( expr ) syntax.  */

நிலையான மதிப்பு *eval7(வெறுமை)
{
	மதிப்பு *v;

	எனில் (!*G.args)
		bb_simple_error_செய்தி_and_die("syntax error");

	எனில் (nextarg("(")) {
		G.args++;
		v = eval();
		எனில் (!nextarg(")"))
			bb_simple_error_செய்தி_and_die("syntax error");
		G.args++;
		திரும்பு v;
	}

	எனில் (nextarg(")"))
		bb_simple_error_செய்தி_and_die("syntax error");

	திரும்பு str_மதிப்பு(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

நிலையான மதிப்பு *eval6(வெறுமை)
{
	நிலையான மாறிலி எழுத்து keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	மதிப்பு *r, *i1, *i2;
	மதிப்பு *l = l; /* silence gcc */
	மதிப்பு *v = v; /* silence gcc */
	முழு சாவி = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	எனில் (சாவி == 0) /* not a keyword */
		திரும்பு eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	எனில் (சாவி == 1) { /* quote */
		எனில் (!*G.args)
			bb_simple_error_செய்தி_and_die("syntax error");
		திரும்பு str_மதிப்பு(*G.args++);
	}
	எனில் (சாவி == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_மதிப்பு(strlen(r->u.s));
		freev(r);
	} இல்லையெனில்
		l = eval6();

	எனில் (சாவி == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	எனில் (சாவி == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_மதிப்பு(strcspn(l->u.s, r->u.s) + 1);
		எனில் (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	எனில் (சாவி == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		எனில் (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_மதிப்பு("");
		இல்லையெனில் {
			v = xmalloc(அளவு(மதிப்பு));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	திரும்பு v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

நிலையான மதிப்பு *eval5(வெறுமை)
{
	மதிப்பு *l, *r, *v;

	l = eval6();
	போது (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	திரும்பு l;
}

/* Handle *, /, % operators.  */

நிலையான மதிப்பு *eval4(வெறுமை)
{
	மதிப்பு *l, *r;
	முழு op;
	arith_t val;

	l = eval5();
	போது (1) {
		op = nextarg("*");
		எனில் (!op) { op = nextarg("/");
		 எனில் (!op) { op = nextarg("%");
		  எனில் (!op) திரும்பு l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_மதிப்பு(val);
	}
}

/* Handle +, - operators.  */

நிலையான மதிப்பு *eval3(வெறுமை)
{
	மதிப்பு *l, *r;
	முழு op;
	arith_t val;

	l = eval4();
	போது (1) {
		op = nextarg("+");
		எனில் (!op) {
			op = nextarg("-");
			எனில் (!op) திரும்பு l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_மதிப்பு(val);
	}
}

/* Handle comparisons.  */

நிலையான மதிப்பு *eval2(வெறுமை)
{
	மதிப்பு *l, *r;
	முழு op;
	arith_t val;

	l = eval3();
	போது (1) {
		op = nextarg("<");
		எனில் (!op) { op = nextarg("<=");
		 எனில் (!op) { op = nextarg("=");
		  எனில் (!op) { op = nextarg("==");
		   எனில் (!op) { op = nextarg("!=");
		    எனில் (!op) { op = nextarg(">=");
		     எனில் (!op) { op = nextarg(">");
		      எனில் (!op) திரும்பு l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_மதிப்பு(val);
	}
}

/* Handle &.  */

நிலையான மதிப்பு *eval1(வெறுமை)
{
	மதிப்பு *l, *r;

	l = eval2();
	போது (nextarg("&")) {
		G.args++;
		r = eval2();
		எனில் (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_மதிப்பு(0);
		} இல்லையெனில்
			freev(r);
	}
	திரும்பு l;
}

/* Handle |.  */

நிலையான மதிப்பு *eval(வெறுமை)
{
	மதிப்பு *l, *r;

	l = eval1();
	போது (nextarg("|")) {
		G.args++;
		r = eval1();
		எனில் (null(l)) {
			freev(l);
			l = r;
		} இல்லையெனில்
			freev(r);
	}
	திரும்பு l;
}

முழு கோவை_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு கோவை_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	மதிப்பு *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	எனில் (*G.args == வெற்று) {
		bb_simple_error_செய்தி_and_die("too few arguments");
	}
	v = eval();
	எனில் (*G.args)
		bb_simple_error_செய்தி_and_die("syntax error");
	எனில் (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	இல்லையெனில்
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
