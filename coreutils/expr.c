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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"
#समावेश "xregex.h"

#अगर ENABLE_व्यंजक_MATH_SUPPORT_64
प्रकार int64_t arith_t;

#घोषणा PF_REZ      "ll"
#घोषणा PF_REZ_TYPE (दीर्घ दीर्घ)
#घोषणा STRTOL(s, e, b) strtoll(s, e, b)
#अन्यथा
प्रकार दीर्घ arith_t;

#घोषणा PF_REZ      "l"
#घोषणा PF_REZ_TYPE (दीर्घ)
#घोषणा STRTOL(s, e, b) म_से_दीर्घ(s, e, b)
#पूर्ण_अगर

/* TODO: use bb_strtol[l]? It's easier to check for errors... */

/* The kinds of value we can have.  */
क्रमागत {
	INTEGER,
	STRING
};

/* A value is.... */
काष्ठा valinfo {
	smallint type;                  /* Which kind. */
	जोड़ {                         /* The value itself. */
		arith_t i;
		अक्षर *s;
	} u;
};
प्रकार काष्ठा valinfo मूल्य;

/* The arguments given to the program, minus the program name.  */
काष्ठा globals {
	अक्षर **args;
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { \
	setup_common_bufsiz(); \
	/* NB: noexec applet - globals not zeroed */ \
} जबतक (0)

/* forward declarations */
अटल मूल्य *eval(व्योम);


/* Return a VALUE for I.  */

अटल मूल्य *int_मूल्य(arith_t i)
{
	मूल्य *v;

	v = xzalloc(माप(मूल्य));
	अगर (INTEGER) /* otherwise xzalloc did it already */
		v->type = INTEGER;
	v->u.i = i;
	वापस v;
}

/* Return a VALUE for S.  */

अटल मूल्य *str_मूल्य(स्थिर अक्षर *s)
{
	मूल्य *v;

	v = xzalloc(माप(मूल्य));
	अगर (STRING) /* otherwise xzalloc did it already */
		v->type = STRING;
	v->u.s = xstrdup(s);
	वापस v;
}

/* Free VALUE V, including structure components.  */

अटल व्योम freev(मूल्य *v)
{
	अगर (v->type == STRING)
		मुक्त(v->u.s);
	मुक्त(v);
}

/* Return nonzero if V is a null-string or zero-number.  */

अटल पूर्णांक null(मूल्य *v)
{
	अगर (v->type == INTEGER)
		वापस v->u.i == 0;
	/* STRING: */
	वापस v->u.s[0] == '\0' || LONE_CHAR(v->u.s, '0');
}

/* Coerce V to a STRING value (can't fail).  */

अटल व्योम tostring(मूल्य *v)
{
	अगर (v->type == INTEGER) {
		v->u.s = xasprintf("%" PF_REZ "d", PF_REZ_TYPE v->u.i);
		v->type = STRING;
	}
}

/* Coerce V to an INTEGER value.  Return 1 on success, 0 on failure.  */

अटल bool toarith(मूल्य *v)
{
	अगर (v->type == STRING) {
		arith_t i;
		अक्षर *e;

		/* Don't interpret the empty string as an integer.  */
		/* Currently does not worry about overflow or int/long differences. */
		i = STRTOL(v->u.s, &e, 10);
		अगर ((v->u.s == e) || *e)
			वापस 0;
		मुक्त(v->u.s);
		v->u.i = i;
		v->type = INTEGER;
	}
	वापस 1;
}

/* Return str[0]+str[1] if the next token matches STR exactly.
   STR must not be NULL.  */

अटल पूर्णांक nextarg(स्थिर अक्षर *str)
{
	अगर (*G.args == शून्य || म_भेद(*G.args, str) != 0)
		वापस 0;
	वापस (अचिन्हित अक्षर)str[0] + (अचिन्हित अक्षर)str[1];
}

/* The comparison operator handling functions.  */

अटल पूर्णांक cmp_common(मूल्य *l, मूल्य *r, पूर्णांक op)
{
	arith_t ll, rr;

	ll = l->u.i;
	rr = r->u.i;
	अगर (l->type == STRING || r->type == STRING) {
		tostring(l);
		tostring(r);
		ll = म_भेद(l->u.s, r->u.s);
		rr = 0;
	}
	/* calculating ll - rr and checking the result is prone to overflows.
	 * We'll do it differently: */
	अगर (op == '<')
		वापस ll < rr;
	अगर (op == ('<' + '='))
		वापस ll <= rr;
	अगर (op == '=' || (op == '=' + '='))
		वापस ll == rr;
	अगर (op == '!' + '=')
		वापस ll != rr;
	अगर (op == '>')
		वापस ll > rr;
	/* >= */
	वापस ll >= rr;
}

/* The arithmetic operator handling functions.  */

अटल arith_t arithmetic_common(मूल्य *l, मूल्य *r, पूर्णांक op)
{
	arith_t li, ri;

	अगर (!toarith(l) || !toarith(r))
		bb_simple_error_संदेश_and_die("non-numeric argument");
	li = l->u.i;
	ri = r->u.i;
	अगर (op == '+')
		वापस li + ri;
	अगर (op == '-')
		वापस li - ri;
	अगर (op == '*')
		वापस li * ri;
	अगर (ri == 0)
		bb_simple_error_संदेश_and_die("division by zero");
	अगर (op == '/')
		वापस li / ri;
	वापस li % ri;
}

/* Do the : operator.
   SV is the VALUE for the lhs (the string),
   PV is the VALUE for the rhs (the pattern).  */

अटल मूल्य *docolon(मूल्य *sv, मूल्य *pv)
{
	क्रमागत { NMATCH = 2 };
	मूल्य *v;
	regex_t re_बफर;
	regmatch_t re_regs[NMATCH];

	tostring(sv);
	tostring(pv);

	अगर (pv->u.s[0] == '^') {
		bb_error_संदेश(
"warning: '%s': using '^' as the first character\n"
"of a basic regular expression is not portable; it is ignored", pv->u.s);
	}

	स_रखो(&re_बफर, 0, माप(re_बफर));
	स_रखो(re_regs, 0, माप(re_regs));
	xregcomp(&re_बफर, pv->u.s, 0);

	/* expr uses an anchored pattern match, so check that there was a
	 * match and that the match starts at offset 0. */
	अगर (regexec(&re_बफर, sv->u.s, NMATCH, re_regs, 0) != REG_NOMATCH
	 && re_regs[0].rm_so == 0
	) {
		/* Were \(...\) used? */
		अगर (re_बफर.re_nsub > 0 && re_regs[1].rm_so >= 0) {
			sv->u.s[re_regs[1].rm_eo] = '\0';
			v = str_मूल्य(sv->u.s + re_regs[1].rm_so);
		} अन्यथा {
			v = int_मूल्य(re_regs[0].rm_eo);
		}
	} अन्यथा {
		/* Match failed -- return the right kind of null.  */
		अगर (re_बफर.re_nsub > 0)
			v = str_मूल्य("");
		अन्यथा
			v = int_मूल्य(0);
	}
	regfree(&re_बफर);
	वापस v;
}

/* Handle bare operands and ( expr ) syntax.  */

अटल मूल्य *eval7(व्योम)
{
	मूल्य *v;

	अगर (!*G.args)
		bb_simple_error_संदेश_and_die("syntax error");

	अगर (nextarg("(")) {
		G.args++;
		v = eval();
		अगर (!nextarg(")"))
			bb_simple_error_संदेश_and_die("syntax error");
		G.args++;
		वापस v;
	}

	अगर (nextarg(")"))
		bb_simple_error_संदेश_and_die("syntax error");

	वापस str_मूल्य(*G.args++);
}

/* Handle match, substr, index, length, and quote keywords.  */

अटल मूल्य *eval6(व्योम)
{
	अटल स्थिर अक्षर keywords[] ALIGN1 =
		"quote\0""length\0""match\0""index\0""substr\0";

	मूल्य *r, *i1, *i2;
	मूल्य *l = l; /* silence gcc */
	मूल्य *v = v; /* silence gcc */
	पूर्णांक कुंजी = *G.args ? सूचकांक_in_strings(keywords, *G.args) + 1 : 0;

	अगर (कुंजी == 0) /* not a keyword */
		वापस eval7();
	G.args++; /* We have a valid token, so get the next argument.  */
	अगर (कुंजी == 1) { /* quote */
		अगर (!*G.args)
			bb_simple_error_संदेश_and_die("syntax error");
		वापस str_मूल्य(*G.args++);
	}
	अगर (कुंजी == 2) { /*लंबाई*/
		r = eval6();
		tostring(r);
		v = int_मूल्य(म_माप(r->u.s));
		freev(r);
	} अन्यथा
		l = eval6();

	अगर (कुंजी == 3) { /* match */
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
	}
	अगर (कुंजी == 4) { /*सूचकांक*/
		r = eval6();
		tostring(l);
		tostring(r);
		v = int_मूल्य(म_खोज(l->u.s, r->u.s) + 1);
		अगर (v->u.i == (arith_t) म_माप(l->u.s) + 1)
			v->u.i = 0;
		freev(l);
		freev(r);
	}
	अगर (कुंजी == 5) { /* substr */
		i1 = eval6();
		i2 = eval6();
		tostring(l);
		अगर (!toarith(i1) || !toarith(i2)
		 || i1->u.i > (arith_t) म_माप(l->u.s)
		 || i1->u.i <= 0 || i2->u.i <= 0)
			v = str_मूल्य("");
		अन्यथा {
			v = xmalloc(माप(मूल्य));
			v->type = STRING;
			v->u.s = xstrndup(l->u.s + i1->u.i - 1, i2->u.i);
		}
		freev(l);
		freev(i1);
		freev(i2);
	}
	वापस v;
}

/* Handle : operator (pattern matching).
   Calls docolon to do the real work.  */

अटल मूल्य *eval5(व्योम)
{
	मूल्य *l, *r, *v;

	l = eval6();
	जबतक (nextarg(":")) {
		G.args++;
		r = eval6();
		v = docolon(l, r);
		freev(l);
		freev(r);
		l = v;
	}
	वापस l;
}

/* Handle *, /, % operators.  */

अटल मूल्य *eval4(व्योम)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval5();
	जबतक (1) {
		op = nextarg("*");
		अगर (!op) { op = nextarg("/");
		 अगर (!op) { op = nextarg("%");
		  अगर (!op) वापस l;
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

अटल मूल्य *eval3(व्योम)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval4();
	जबतक (1) {
		op = nextarg("+");
		अगर (!op) {
			op = nextarg("-");
			अगर (!op) वापस l;
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

अटल मूल्य *eval2(व्योम)
{
	मूल्य *l, *r;
	पूर्णांक op;
	arith_t val;

	l = eval3();
	जबतक (1) {
		op = nextarg("<");
		अगर (!op) { op = nextarg("<=");
		 अगर (!op) { op = nextarg("=");
		  अगर (!op) { op = nextarg("==");
		   अगर (!op) { op = nextarg("!=");
		    अगर (!op) { op = nextarg(">=");
		     अगर (!op) { op = nextarg(">");
		      अगर (!op) वापस l;
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

अटल मूल्य *eval1(व्योम)
{
	मूल्य *l, *r;

	l = eval2();
	जबतक (nextarg("&")) {
		G.args++;
		r = eval2();
		अगर (null(l) || null(r)) {
			freev(l);
			freev(r);
			l = int_मूल्य(0);
		} अन्यथा
			freev(r);
	}
	वापस l;
}

/* Handle |.  */

अटल मूल्य *eval(व्योम)
{
	मूल्य *l, *r;

	l = eval1();
	जबतक (nextarg("|")) {
		G.args++;
		r = eval1();
		अगर (null(l)) {
			freev(l);
			l = r;
		} अन्यथा
			freev(r);
	}
	वापस l;
}

पूर्णांक व्यंजक_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक व्यंजक_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	मूल्य *v;

	INIT_G();

	xfunc_error_retval = 2; /* coreutils compat */
	G.args = argv + 1;
	अगर (*G.args == शून्य) {
		bb_simple_error_संदेश_and_die("too few arguments");
	}
	v = eval();
	अगर (*G.args)
		bb_simple_error_संदेश_and_die("syntax error");
	अगर (v->type == INTEGER)
		म_लिखो("%" PF_REZ "d\n", PF_REZ_TYPE v->u.i);
	अन्यथा
		माला_दो(v->u.s);
	fflush_stdout_and_exit(null(v));
}
