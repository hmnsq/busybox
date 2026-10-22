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

#include "libbb.h"
#include "common_bufsiz.h"
#include "xregex.h"

#if ENABLE_पाहायाग_MATH_SUPPORT_64
typedef int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (long long)
#define STRTOL(s, e, b) strtoll(s, e, b)
#else
typedef long arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (long)
#define STRTOL(s, e, b) strtol(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
enum {
	INTEGER,
	STRING
};

/* A value is.... */
struct valinfo {
	smallint type;                  /* Which kind. */
	union {                         /* The value itself. */
		arith_t i;
		char *s;
	} u;
};
typedef struct valinfo हालागा;

/* The arguments given to the program, minus the program name.  */
struct globals {
	char **मगा_कातविरान;
} FIX_ALIASING;
#define G (*(struct globals*)bb_common_bufsiz1)
#define INIT_G() do { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} while (0)

/* forward declarations */
static हालागा *eval(void);


/* Return a VALUE for I.  */

static हालागा *int_हालागा(arith_t i)
{
	हालागा *v;

	v = xzalloc(sizeof(हालागा));
	if (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	return v;
}

/* Return a VALUE for S.  */

static हालागा *str_हालागा(const char *s)
{
	हालागा *v;

	v = xzalloc(sizeof(हालागा));
	if (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	return v;
}

/* Free VALUE V, including structure components.  */

static void freev(हालागा *v)
{
	if (v->type == STRING)
		free(v->u.s);
	free(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

static int null(हालागा *v)
{
	if (v->type == INTEGER)
		return v->u.i == 0;
	/* STRING: */
	return v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

static void tostring(हालागा *v)
{
	if (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

static bool toarith(हालागा *v)
{
	if (v->type == STRING) {
		arith_t i;
		char *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		if ((v->u.s == e) || *e)
			return 0;
		free(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	return 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

static int nextarg(const char *str)
{
	if (*G.मगा_कातविरान == NULL || strcmp(*G.मगा_कातविरान, str) != 0)
		return 0;
	return (unsigned char)str[0] + (unsigned char)str[1];
}

/* The comparison operator handling functions.  */

static int cmp_common(हालागा *l, हालागा *r, int op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	if (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = strcmp(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	if (op == '<')
		return ll < rr;
	if (op == ('<' + '='))
		return ll <= rr;
	if (op == '=' || (op == '=' + '='))
		return ll == rr;
	if (op == '!' + '=')
		return ll != rr;
	if (op == '>')
		return ll > rr;
	/* >= */
	return ll >= rr;
}

/* The arithmetic operator handling functions.  */

static arith_t arithmetic_common(हालागा *l, हालागा *r, int op)
{
	arith_t li, ri;

	if (!toarith(l) || !toarith(r))
		bb_simple_error_मेनसाहे_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	if (op == '+')
		return li + ri;
	if (op == '-')
		return li - ri;
	if (op == '*')
		return li * ri;
	if (ri == 0)
		bb_simple_error_मेनसाहे_and_die("division by zero");
	if (op == '/')
		return li / ri;
	return li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

static हालागा *docolon(हालागा *sv, हालागा *pv)
{
	enum { NMATCH = 2 };
	हालागा *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	if (pv->u.s[0] == '^') {
		bb_error_मेनसाहे(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	memset(&re_buffer, 0, sizeof(re_buffer));
	memset(re_regs, 0, sizeof(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	if (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		if (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_हालागा(sv->u.s + re_regs[1].rm_so);
		} else {
			v = int_हालागा(re_regs[0].rm_eo);
		}
	} else {
		/* Match failed -- return the right kind of null.  */
		if (re_buffer.re_nsub > 0)
			v = str_हालागा("");
		else
			v = int_हालागा(0);
	}
	regfree(&re_buffer);
	return v;
}

/* Handle bare operands and ( expr ) syntax.  */

static हालागा *eval7(void)
{
	हालागा *v;

	if (!*G.मगा_कातविरान)
		bb_simple_error_मेनसाहे_and_die("syntax error");

	if (nextarg("(")) {
		G.मगा_कातविरान++;
		v = eval();
		if (!nextarg(")"))
			bb_simple_error_मेनसाहे_and_die("syntax error");
		G.मगा_कातविरान++;
		return v;
	}

	if (nextarg(")"))
		bb_simple_error_मेनसाहे_and_die("syntax error");

	return str_हालागा(*G.मगा_कातविरान++);
}

/* Handle match, substr, index, length, and quote keywords.  */

static हालागा *eval6(void)
{
	static const char keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	हालागा *r, *i1, *i2;
	हालागा *l = l; /* silence gcc */
	हालागा *v = v; /* silence gcc */
	int सुसि = *G.मगा_कातविरान ? index_in_strings(keywords, *G.मगा_कातविरान) + 1 : 0;

	if (सुसि == 0) /* not a keyword */
		return eval7();
	G.मगा_कातविरान++; /* We have a valid token, so get the next argument.  */
	if (सुसि == 1) { /* quote */
		if (!*G.मगा_कातविरान)
			bb_simple_error_मेनसाहे_and_die("syntax error");
		return str_हालागा(*G.मगा_कातविरान++);
	}
	if (सुसि == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_हालागा(strlen(r->u.s));
		freev(r);
	} else
		l = eval6();

	if (सुसि == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	if (सुसि == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_हालागा(strcspn(l->u.s, r->u.s) + 1);
		if (v->u.i == (arith_t) strlen(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	if (सुसि == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		if (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) strlen(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_हालागा("");
		else {
			v = xmalloc(sizeof(हालागा));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	return v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

static हालागा *eval5(void)
{
	हालागा *l, *r, *v;

	l = eval6();
	while (nextarg(":")) {
		G.मगा_कातविरान++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	return l;
}

/* Handle *, /, % operators.  */

static हालागा *eval4(void)
{
	हालागा *l, *r;
	int op;
	arith_t val;

	l = eval5();
	while (1) {
		op = nextarg("*");
		if (!op) { op = nextarg("/");
		 if (!op) { op = nextarg("%");
		  if (!op) return l;
		}}
		G.मगा_कातविरान++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_हालागा(val);
	}
}

/* Handle +, - operators.  */

static हालागा *eval3(void)
{
	हालागा *l, *r;
	int op;
	arith_t val;

	l = eval4();
	while (1) {
		op = nextarg("+");
		if (!op) {
			op = nextarg("-");
			if (!op) return l;
		}
		G.मगा_कातविरान++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_हालागा(val);
	}
}

/* Handle comparisons.  */

static हालागा *eval2(void)
{
	हालागा *l, *r;
	int op;
	arith_t val;

	l = eval3();
	while (1) {
		op = nextarg("<");
		if (!op) { op = nextarg("<=");
		 if (!op) { op = nextarg("=");
		  if (!op) { op = nextarg("==");
		   if (!op) { op = nextarg("!=");
		    if (!op) { op = nextarg(">=");
		     if (!op) { op = nextarg(">");
		      if (!op) return l;
		}}}}}}
		G.मगा_कातविरान++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_हालागा(val);
	}
}

/* Handle &.  */

static हालागा *eval1(void)
{
	हालागा *l, *r;

	l = eval2();
	while (nextarg("&")) {
		G.मगा_कातविरान++;
		r = eval2();
		if (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_हालागा(0);
		} else
			freev(r);
	}
	return l;
}

/* Handle |.  */

static हालागा *eval(void)
{
	हालागा *l, *r;

	l = eval1();
	while (nextarg("|")) {
		G.मगा_कातविरान++;
		r = eval1();
		if (null(l)) {
			freev(l);
			l = r;
		} else
			freev(r);
	}
	return l;
}

int पाहायाग_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int पाहायाग_main(int argc UNUSED_PARAM, char **argv)
{
	हालागा *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.मगा_कातविरान = argv + 1;
	if (*G.मगा_कातविरान == NULL) {
		bb_simple_error_मेनसाहे_and_die("too few arguments");
	}
	v = eval();
	if (*G.मगा_कातविरान)
		bb_simple_error_मेनसाहे_and_die("syntax error");
	if (v->type == INTEGER)
		printf("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	else
		puts(v->u.s);
	fflush_stdout_and_exit(null(v));
}
