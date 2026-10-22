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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"
#അന്തര്ഭാവ "xregex.h"

#എങ്കിൽ ENABLE_വ്യഞ്ജകം_MATH_SUPPORT_64
തരനിർവചനം int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#അല്ലെങ്കിൽ
തരനിർവചനം long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
എണ്ണൽ {
	INTEGER,
	STRING
};

/* A value is.... */
ഘടന valinfo {
	smallint type;                  /* Which kind. */
	സംഘം {                         /* The value itself. */
		arith_t i;
		അക്ഷരം *s;
	} u;
};
തരനിർവചനം ഘടന valinfo മൂല്യം;

/* The arguments given to the program, minus the program name.  */
ഘടന globals {
	അക്ഷരം **args;
} FIX_ALIASING;
#define G (*(ഘടന globals*)bb_common_bufsiz1)
#define INIT_G() ചെയ്യുക { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} ആകുമ്പോൾ (0)

/* forward declarations */
സ്ഥിതം മൂല്യം *eval(ശൂന്യം);


/* Return a VALUE for I.  */

സ്ഥിതം മൂല്യം *int_മൂല്യം(arith_t i)
{
	മൂല്യം *v;

	v = xzalloc(വലുപ്പം(മൂല്യം));
	എങ്കിൽ (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	തിരികെ v;
}

/* Return a VALUE for S.  */

സ്ഥിതം മൂല്യം *str_മൂല്യം(സ്ഥിരം അക്ഷരം *s)
{
	മൂല്യം *v;

	v = xzalloc(വലുപ്പം(മൂല്യം));
	എങ്കിൽ (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	തിരികെ v;
}

/* Free VALUE V, including structure components.  */

സ്ഥിതം ശൂന്യം freev(മൂല്യം *v)
{
	എങ്കിൽ (v->type == STRING)
		സ്വതന്ത്രം(v->u.s);
	സ്വതന്ത്രം(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ null(മൂല്യം *v)
{
	എങ്കിൽ (v->type == INTEGER)
		തിരികെ v->u.i == 0;
	/* STRING: */
	തിരികെ v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

സ്ഥിതം ശൂന്യം tostring(മൂല്യം *v)
{
	എങ്കിൽ (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

സ്ഥിതം bool toarith(മൂല്യം *v)
{
	എങ്കിൽ (v->type == STRING) {
		arith_t i;
		അക്ഷരം *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		എങ്കിൽ ((v->u.s == e) || *e)
			തിരികെ 0;
		സ്വതന്ത്രം(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	തിരികെ 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ nextarg(സ്ഥിരം അക്ഷരം *str)
{
	എങ്കിൽ (*G.args == ശൂന്യ || strcmp(*G.args, str) != 0)
		തിരികെ 0;
	തിരികെ (unsigned അക്ഷരം)str[0] + (unsigned അക്ഷരം)str[1];
}

/* The comparison operator handling functions.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ cmp_common(മൂല്യം *l, മൂല്യം *r, പൂർണ്ണസംഖ്യ op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	എങ്കിൽ (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	എങ്കിൽ (op == '<')
		തിരികെ ll < rr;
	എങ്കിൽ (op == ('<' + '='))
		തിരികെ ll <= rr;
	എങ്കിൽ (op == '=' || (op == '=' + '='))
		തിരികെ ll == rr;
	എങ്കിൽ (op == '!' + '=')
		തിരികെ ll != rr;
	എങ്കിൽ (op == '>')
		തിരികെ ll > rr;
	/* >= */
	തിരികെ ll >= rr;
}

/* The arithmetic operator handling functions.  */

സ്ഥിതം arith_t arithmetic_common(മൂല്യം *l, മൂല്യം *r, പൂർണ്ണസംഖ്യ op)
{
	arith_t li, ri;

	എങ്കിൽ (!toarith(l) || !toarith(r))
		bb_simple_error_സന്ദേശം_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	എങ്കിൽ (op == '+')
		തിരികെ li + ri;
	എങ്കിൽ (op == '-')
		തിരികെ li - ri;
	എങ്കിൽ (op == '*')
		തിരികെ li * ri;
	എങ്കിൽ (ri == 0)
		bb_simple_error_സന്ദേശം_and_die("division by zero");
	എങ്കിൽ (op == '/')
		തിരികെ li / ri;
	തിരികെ li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

സ്ഥിതം മൂല്യം *docolon(മൂല്യം *sv, മൂല്യം *pv)
{
	എണ്ണൽ { NMATCH = 2 };
	മൂല്യം *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	എങ്കിൽ (pv->u.s[0] == '^') {
		bb_error_സന്ദേശം(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, വലുപ്പം(re_buffer));
	memset(re_regs, 0, വലുപ്പം(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	എങ്കിൽ (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		എങ്കിൽ (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_മൂല്യം(sv->u.s + re_regs[1].rm_so);
		} അല്ലെങ്കിൽ {
			v = int_മൂല്യം(re_regs[0].rm_eo);
		}
	} അല്ലെങ്കിൽ {
		/* Match failed -- return the right kind of null.  */
		എങ്കിൽ (re_buffer.re_nsub > 0)
			v = str_മൂല്യം("");
		അല്ലെങ്കിൽ
			v = int_മൂല്യം(0);
	}
	regfree(&re_buffer);
	തിരികെ v;
}

/* Handle bare operands and ( expr ) syntax.  */

സ്ഥിതം മൂല്യം *eval7(ശൂന്യം)
{
	മൂല്യം *v;

	എങ്കിൽ (!*G.args)
		bb_simple_error_സന്ദേശം_and_die("syntax error");

	എങ്കിൽ (nextarg("(")) {
		G.args++;
		v = eval();
		എങ്കിൽ (!nextarg(")"))
			bb_simple_error_സന്ദേശം_and_die("syntax error");
		G.args++;
		തിരികെ v;
	}

	എങ്കിൽ (nextarg(")"))
		bb_simple_error_സന്ദേശം_and_die("syntax error");

	തിരികെ str_മൂല്യം(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

സ്ഥിതം മൂല്യം *eval6(ശൂന്യം)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	മൂല്യം *r, *i1, *i2;
	മൂല്യം *l = l; /* silence gcc */
	മൂല്യം *v = v; /* silence gcc */
	പൂർണ്ണസംഖ്യ താക്കോൽ = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	എങ്കിൽ (താക്കോൽ == 0) /* not a keyword */
		തിരികെ eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	എങ്കിൽ (താക്കോൽ == 1) { /* quote */
		എങ്കിൽ (!*G.args)
			bb_simple_error_സന്ദേശം_and_die("syntax error");
		തിരികെ str_മൂല്യം(*G.args++);
	}
	എങ്കിൽ (താക്കോൽ == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_മൂല്യം(strlen(r->u.s));
		freev(r);
	} അല്ലെങ്കിൽ
		l = eval6();

	എങ്കിൽ (താക്കോൽ == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	എങ്കിൽ (താക്കോൽ == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_മൂല്യം(strcspn(l->u.s, r->u.s) + 1);
		എങ്കിൽ (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	എങ്കിൽ (താക്കോൽ == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		എങ്കിൽ (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_മൂല്യം("");
		അല്ലെങ്കിൽ {
			v = xmalloc(വലുപ്പം(മൂല്യം));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	തിരികെ v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

സ്ഥിതം മൂല്യം *eval5(ശൂന്യം)
{
	മൂല്യം *l, *r, *v;

	l = eval6();
	ആകുമ്പോൾ (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	തിരികെ l;
}

/* Handle *, /, % operators.  */

സ്ഥിതം മൂല്യം *eval4(ശൂന്യം)
{
	മൂല്യം *l, *r;
	പൂർണ്ണസംഖ്യ op;
	arith_t val;

	l = eval5();
	ആകുമ്പോൾ (1) {
		op = nextarg("*");
		എങ്കിൽ (!op) { op = nextarg("/");
		 എങ്കിൽ (!op) { op = nextarg("%");
		  എങ്കിൽ (!op) തിരികെ l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_മൂല്യം(val);
	}
}

/* Handle +, - operators.  */

സ്ഥിതം മൂല്യം *eval3(ശൂന്യം)
{
	മൂല്യം *l, *r;
	പൂർണ്ണസംഖ്യ op;
	arith_t val;

	l = eval4();
	ആകുമ്പോൾ (1) {
		op = nextarg("+");
		എങ്കിൽ (!op) {
			op = nextarg("-");
			എങ്കിൽ (!op) തിരികെ l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_മൂല്യം(val);
	}
}

/* Handle comparisons.  */

സ്ഥിതം മൂല്യം *eval2(ശൂന്യം)
{
	മൂല്യം *l, *r;
	പൂർണ്ണസംഖ്യ op;
	arith_t val;

	l = eval3();
	ആകുമ്പോൾ (1) {
		op = nextarg("<");
		എങ്കിൽ (!op) { op = nextarg("<=");
		 എങ്കിൽ (!op) { op = nextarg("=");
		  എങ്കിൽ (!op) { op = nextarg("==");
		   എങ്കിൽ (!op) { op = nextarg("!=");
		    എങ്കിൽ (!op) { op = nextarg(">=");
		     എങ്കിൽ (!op) { op = nextarg(">");
		      എങ്കിൽ (!op) തിരികെ l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_മൂല്യം(val);
	}
}

/* Handle &.  */

സ്ഥിതം മൂല്യം *eval1(ശൂന്യം)
{
	മൂല്യം *l, *r;

	l = eval2();
	ആകുമ്പോൾ (nextarg("&")) {
		G.args++;
		r = eval2();
		എങ്കിൽ (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_മൂല്യം(0);
		} അല്ലെങ്കിൽ
			freev(r);
	}
	തിരികെ l;
}

/* Handle |.  */

സ്ഥിതം മൂല്യം *eval(ശൂന്യം)
{
	മൂല്യം *l, *r;

	l = eval1();
	ആകുമ്പോൾ (nextarg("|")) {
		G.args++;
		r = eval1();
		എങ്കിൽ (null(l)) {
			freev(l);
			l = r;
		} അല്ലെങ്കിൽ
			freev(r);
	}
	തിരികെ l;
}

പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	മൂല്യം *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	എങ്കിൽ (*G.args == ശൂന്യ) {
		bb_simple_error_സന്ദേശം_and_die("too few arguments");
	}
	v = eval();
	എങ്കിൽ (*G.args)
		bb_simple_error_സന്ദേശം_and_die("syntax error");
	എങ്കിൽ (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	അല്ലെങ്കിൽ
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
