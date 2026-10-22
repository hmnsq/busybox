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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"
#अन्तर्भाव "xregex.h"

#जर ENABLE_अभिव्यक्ती_MATH_SUPPORT_64
प्रकारव्याख्या int64_t arith_t;

#define PF_REZ      "ll"
#define PF_REZ_TYPE (दीर्घ दीर्घ)
#define STRTOL(s, e, b) strtoll(s, e, b)
#अन्यथा
प्रकारव्याख्या दीर्घ arith_t;

#define PF_REZ      "l"
#define PF_REZ_TYPE (दीर्घ)
#define STRTOL(s, e, b) म_से_दीर्घ(s, e, b)
#endif

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
गणना {
	INTEGER,
	STRING
};

/* A value is.... */
संरचना valinfo {
	smallint type;                  /* Which kind. */
	संघ {                         /* The value itself. */
		arith_t i;
		अक्षर *s;
	} u;
};
प्रकारव्याख्या संरचना valinfo मूल्य;

/* The arguments given to the program, minus the program name.  */
संरचना globals {
	अक्षर **args;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करा { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} पर्यंत (0)

/* forward declarations */
स्थिर मूल्य *eval(शून्य);


/* Return a VALUE for I.  */

स्थिर मूल्य *int_मूल्य(arith_t i)
{
	मूल्य *v;

	v = xzalloc(आकार(मूल्य));
	जर (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	परत v;
}

/* Return a VALUE for S.  */

स्थिर मूल्य *str_मूल्य(स्थिरांक अक्षर *s)
{
	मूल्य *v;

	v = xzalloc(आकार(मूल्य));
	जर (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	परत v;
}

/* Free VALUE V, including structure components.  */

स्थिर शून्य freev(मूल्य *v)
{
	जर (v->type == STRING)
		मुक्त(v->u.s);
	मुक्त(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

स्थिर पूर्णांक null(मूल्य *v)
{
	जर (v->type == INTEGER)
		परत v->u.i == 0;
	/* STRING: */
	परत v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

स्थिर शून्य tostring(मूल्य *v)
{
	जर (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

स्थिर bool toarith(मूल्य *v)
{
	जर (v->type == STRING) {
		arith_t i;
		अक्षर *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		जर ((v->u.s == e) || *e)
			परत 0;
		मुक्त(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	परत 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

स्थिर पूर्णांक nextarg(स्थिरांक अक्षर *str)
{
	जर (*G.args == रिक्त || म_भेद(*G.args, str) != 0)
		परत 0;
	परत (अचिन्हित अक्षर)str[0] + (अचिन्हित अक्षर)str[1];
}

/* The comparison operator handling functions.  */

स्थिर पूर्णांक cmp_common(मूल्य *l, मूल्य *r, पूर्णांक op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	जर (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = म_भेद(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	जर (op == '<')
		परत ll < rr;
	जर (op == ('<' + '='))
		परत ll <= rr;
	जर (op == '=' || (op == '=' + '='))
		परत ll == rr;
	जर (op == '!' + '=')
		परत ll != rr;
	जर (op == '>')
		परत ll > rr;
	/* >= */
	परत ll >= rr;
}

/* The arithmetic operator handling functions.  */

स्थिर arith_t arithmetic_common(मूल्य *l, मूल्य *r, पूर्णांक op)
{
	arith_t li, ri;

	जर (!toarith(l) || !toarith(r))
		bb_simple_error_संदेश_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	जर (op == '+')
		परत li + ri;
	जर (op == '-')
		परत li - ri;
	जर (op == '*')
		परत li * ri;
	जर (ri == 0)
		bb_simple_error_संदेश_and_die("division by zero");
	जर (op == '/')
		परत li / ri;
	परत li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

स्थिर मूल्य *docolon(मूल्य *sv, मूल्य *pv)
{
	गणना { NMATCH = 2 };
	मूल्य *v;
	regex_t re_buffer;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	जर (pv->u.s[0] == '^') {
		bb_error_संदेश(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	स_रखो(&re_buffer, 0, आकार(re_buffer));
	स_रखो(re_regs, 0, आकार(re_regs));
	xregcomp(&re_buffer, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	जर (regexec(&re_buffer, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		जर (re_buffer.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_मूल्य(sv->u.s + re_regs[1].rm_so);
		} अन्यथा {
			v = int_मूल्य(re_regs[0].rm_eo);
		}
	} अन्यथा {
		/* Match failed -- return the right kind of null.  */
		जर (re_buffer.re_nsub > 0)
			v = str_मूल्य("");
		अन्यथा
			v = int_मूल्य(0);
	}
	regfree(&re_buffer);
	परत v;
}

/* Handle bare operands and ( expr ) syntax.  */

स्थिर मूल्य *eval7(शून्य)
{
	मूल्य *v;

	जर (!*G.args)
		bb_simple_error_संदेश_and_die("syntax error");

	जर (nextarg("(")) {
		G.args++;
		v = eval();
		जर (!nextarg(")"))
			bb_simple_error_संदेश_and_die("syntax error");
		G.args++;
		परत v;
	}

	जर (nextarg(")"))
		bb_simple_error_संदेश_and_die("syntax error");

	परत str_मूल्य(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

स्थिर मूल्य *eval6(शून्य)
{
	स्थिर स्थिरांक अक्षर keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	मूल्य *r, *i1, *i2;
	मूल्य *l = l; /* silence gcc */
	मूल्य *v = v; /* silence gcc */
	पूर्णांक किल्ली = *G.args ? index_in_strings(keywords, *G.args) + 1 : 0;

	जर (किल्ली == 0) /* not a keyword */
		परत eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	जर (किल्ली == 1) { /* quote */
		जर (!*G.args)
			bb_simple_error_संदेश_and_die("syntax error");
		परत str_मूल्य(*G.args++);
	}
	जर (किल्ली == 2) { /* length */
		r = eval6();
		tostring(r);
		v = int_मूल्य(म_माप(r->u.s));
		freev(r);
	} अन्यथा
		l = eval6();

	जर (किल्ली == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	जर (किल्ली == 4) { /* index */
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_मूल्य(म_खोज(l->u.s, r->u.s) + 1);
		जर (v->u.i == (arith_t) म_माप(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	जर (किल्ली == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		जर (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) म_माप(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_मूल्य("");
		अन्यथा {
			v = xmalloc(आकार(मूल्य));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	परत v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

स्थिर मूल्य *eval5(शून्य)
{
	मूल्य *l, *r, *v;

	l = eval6();
	पर्यंत (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	परत l;
}

/* Handle *, /, % operators.  */

स्थिर मूल्य *eval4(शून्य)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval5();
	पर्यंत (1) {
		op = nextarg("*");
		जर (!op) { op = nextarg("/");
		 जर (!op) { op = nextarg("%");
		  जर (!op) परत l;
		}}
		G.args++;
		r = eval5();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_मूल्य(val);
	}
}

/* Handle +, - operators.  */

स्थिर मूल्य *eval3(शून्य)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval4();
	पर्यंत (1) {
		op = nextarg("+");
		जर (!op) {
			op = nextarg("-");
			जर (!op) परत l;
		}
		G.args++;
		r = eval4();
		val = arithmetic_common(l, r, op);
		freev(l);
		freev(r);
		l = int_मूल्य(val);
	}
}

/* Handle comparisons.  */

स्थिर मूल्य *eval2(शून्य)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval3();
	पर्यंत (1) {
		op = nextarg("<");
		जर (!op) { op = nextarg("<=");
		 जर (!op) { op = nextarg("=");
		  जर (!op) { op = nextarg("==");
		   जर (!op) { op = nextarg("!=");
		    जर (!op) { op = nextarg(">=");
		     जर (!op) { op = nextarg(">");
		      जर (!op) परत l;
		}}}}}}
		G.args++;
		r = eval3();
		toarith(l);
		toarith(r);
		val = cmp_common(l, r, op);
		freev(l);
		freev(r);
		l = int_मूल्य(val);
	}
}

/* Handle &.  */

स्थिर मूल्य *eval1(शून्य)
{
	मूल्य *l, *r;

	l = eval2();
	पर्यंत (nextarg("&")) {
		G.args++;
		r = eval2();
		जर (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_मूल्य(0);
		} अन्यथा
			freev(r);
	}
	परत l;
}

/* Handle |.  */

स्थिर मूल्य *eval(शून्य)
{
	मूल्य *l, *r;

	l = eval1();
	पर्यंत (nextarg("|")) {
		G.args++;
		r = eval1();
		जर (null(l)) {
			freev(l);
			l = r;
		} अन्यथा
			freev(r);
	}
	परत l;
}

पूर्णांक अभिव्यक्ती_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक अभिव्यक्ती_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	मूल्य *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	जर (*G.args == रिक्त) {
		bb_simple_error_संदेश_and_die("too few arguments");
	}
	v = eval();
	जर (*G.args)
		bb_simple_error_संदेश_and_die("syntax error");
	जर (v->type == INTEGER)
		म_लिखो("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	अन्यथा
		माला_दो(v->u.s);
	fflush_stdout_and_exit(null(v));
}
