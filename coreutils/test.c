/* vi: set sw=4 ts=4: */
/*
 * test implementation for busybox
 *
 * Copyright (c) by a whole pile of folks:
 *
 *     test(1); version 7-like  --  author Erik Baalbergen
 *     modified by Eric Gisin to be used as built-in.
 *     modified by Arnold Robbins to add SVR3 compatibility
 *     (-x -c -b -p -u -g -k) plus Korn's -L -nt -ot -ef and new -S (socket).
 *     modified by J.T. Conklin for NetBSD.
 *     modified by Herbert Xu to be used as built-in in ash.
 *     modified by Erik Andersen <andersen@codepoet.org> to be used
 *     in busybox.
 *     modified by Bernhard Reutner-Fischer to be useable (i.e. a bit less bloaty).
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * Original copyright notice states:
 *     "This program is in the Public Domain."
 */
//config:config TEST
//config:	bool "test (4.4 kb)"
//config:	default y
//config:	help
//config:	test is used to check file types and compare values,
//config:	returning an appropriate exit code. The bash shell
//config:	has test built in, ash can build it in optionally.
//config:
//config:config TEST1
//config:	bool "test as ["
//config:	default y
//config:	help
//config:	Provide test command in the "[ EXPR ]" form
//config:
//config:config TEST2
//config:	bool "test as [["
//config:	default y
//config:	help
//config:	Provide test command in the "[[ EXPR ]]" form
//config:
//config:config FEATURE_TEST_64
//config:	bool "Extend test to 64 bit"
//config:	default y
//config:	depends on TEST || TEST1 || TEST2 || ASH_TEST || HUSH_TEST
//config:	help
//config:	Enable 64-bit support in test.

//applet:IF_TEST(APPLET_NOFORK(test, test, BB_DIR_USR_BIN, BB_SUID_DROP, test))
//applet:IF_TEST1(APPLET_NOFORK([,   test, BB_DIR_USR_BIN, BB_SUID_DROP, test))
//applet:IF_TEST2(APPLET_NOFORK([[,  test, BB_DIR_USR_BIN, BB_SUID_DROP, test))

//kbuild:lib-$(CONFIG_TEST)  += test.o test_ptr_hack.o
//kbuild:lib-$(CONFIG_TEST1) += test.o test_ptr_hack.o
//kbuild:lib-$(CONFIG_TEST2) += test.o test_ptr_hack.o

//kbuild:lib-$(CONFIG_ASH_TEST)  += test.o test_ptr_hack.o
//kbuild:lib-$(CONFIG_HUSH_TEST) += test.o test_ptr_hack.o

/* "test --help" is special-cased to ignore --help */
//usage:#define test_trivial_usage NOUSAGE_STR
//usage:#define test_full_usage ""
//usage:
//usage:#define test_example_usage
//usage:       "$ test 1 -eq 2\n"
//usage:       "$ echo $?\n"
//usage:       "1\n"
//usage:       "$ test 1 -eq 1\n"
//usage:       "$ echo $?\n"
//usage:       "0\n"
//usage:       "$ [ -d /etc ]\n"
//usage:       "$ echo $?\n"
//usage:       "0\n"
//usage:       "$ [ -d /junk ]\n"
//usage:       "$ echo $?\n"
//usage:       "1\n"

#అన్తర్భావ "libbb.h"
#అన్తర్భావ <regex.h>
#అన్తర్భావ <fnmatch.h>

/* This is a NOFORK applet. Be very careful! */

/* test_main() is called from shells, and we need to be extra careful here.
 * This is true regardless of PREFER_APPLETS and SH_STANDALONE
 * state. */

/* test(1) accepts the following grammar:
	oexpr   ::= aexpr | aexpr "-o" oexpr ;
	aexpr   ::= nexpr | nexpr "-a" aexpr ;
	nexpr   ::= primary | "!" primary
	primary ::= unary-operator operand
		| operand binary-operator operand
		| operand
		| "(" oexpr ")"
		;
	unary-operator ::= "-r"|"-w"|"-x"|"-f"|"-d"|"-c"|"-b"|"-p"|
		"-u"|"-g"|"-k"|"-s"|"-t"|"-z"|"-n"|"-o"|"-O"|"-G"|"-L"|"-S";

	binary-operator ::= "="|"=="|"!="|"-eq"|"-ne"|"-ge"|"-gt"|"-le"|"-lt"|
			"-nt"|"-ot"|"-ef";
	operand ::= <any legal UNIX file name>
*/

/* TODO: handle [[ expr ]] bashism bash-compatibly.
 * [[ ]] is meant to be a "better [ ]", with less weird syntax
 * and without the risk of variables and quoted strings misinterpreted
 * as operators.
 * This will require support from shells - we need to know quote status
 * of each parameter (see below).
 *
 * Word splitting and pathname expansion should NOT be performed:
 *      # a="a b"; [[ $a = "a b" ]] && echo YES
 *      YES
 *      # [[ /bin/m* ]] && echo YES
 *      YES
 *
 * =~ should do regexp match
 * = and == should do pattern match against right side:
 *      # [[ *a* == bab ]] && echo YES
 *      # [[ bab == *a* ]] && echo YES
 *      YES
 * != does the negated == (i.e., also with pattern matching).
 * Pattern matching is quotation-sensitive:
 *      # [[ bab == "b"a* ]] && echo YES
 *      YES
 *      # [[ bab == b"a*" ]] && echo YES
 *
 * Conditional operators such as -f must be unquoted literals to be recognized:
 *      # [[ -e /bin ]] && echo YES
 *      YES
 *      # [[ '-e' /bin ]] && echo YES
 *      bash: conditional binary operator expected...
 *      # A='-e'; [[ $A /bin ]] && echo YES
 *      bash: conditional binary operator expected...
 *
 * || and && should work as -o and -a work in [ ]
 * -a and -o aren't recognized (&& and || are to be used instead)
 * ( and ) do not need to be quoted unlike in [ ]:
 *      # [[ ( abc ) && '' ]] && echo YES
 *      # [[ ( abc ) || '' ]] && echo YES
 *      YES
 *      # [[ ( abc ) -o '' ]] && echo YES
 *      bash: syntax error in conditional expression...
 *
 * Apart from the above, [[ expr ]] should work as [ expr ]
 */

#నిర్వచించు TEST_DEBUG 0

#ఒకవేళ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#లేకపోతే
# define BASH_TEST2 0
#అయితే_ముగింపు

గణన token {
	EOI,

	FILRD, /* file access */
	FILWR,
	FILEX,

	FILEXIST,

	FILREG, /* file type */
	FILDIR,
	FILCDEV,
	FILBDEV,
	FILFIFO,
	FILSOCK,

	FILSYM,
	FILGZ,
	FILTT,

	FILSUID, /* file bit */
	FILSGID,
	FILSTCK,

	FILNT, /* file ops */
	FILOT,
	FILEQ,

	FILUID,
	FILGID,

	STREZ, /* str ops */
	STRNZ,
	STREQ,
	STRNE,
	STRLT,
	STRGT,

#ఒకవేళ BASH_TEST2
	REGEX,
#అయితే_ముగింపు

	INTEQ, /* int ops */
	INTNE,
	INTGE,
	INTGT,
	INTLE,
	INTLT,

	UNOT,
	BAND,
	BOR,
	LPAREN,
	RPAREN,
	నిర్వాహ్యం
};
#నిర్వచించు is_int_op(a)      (((గుర్తులేని అక్షరం)((a) - INTEQ)) <= 5)
#నిర్వచించు is_str_op(a)      (((గుర్తులేని అక్షరం)((a) - STREZ)) <= 5)
#నిర్వచించు is_file_op(a)     (((గుర్తులేని అక్షరం)((a) - FILNT)) <= 2)
#నిర్వచించు is_file_access(a) (((గుర్తులేని అక్షరం)((a) - FILRD)) <= 2)
#నిర్వచించు is_file_type(a)   (((గుర్తులేని అక్షరం)((a) - FILREG)) <= 5)
#నిర్వచించు is_file_బిట్టు(a)    (((గుర్తులేని అక్షరం)((a) - FILSUID)) <= 2)

#ఒకవేళ TEST_DEBUG
పూర్ణాంకం లోతు;
#నిర్వచించు nest_సందేశం(...) చేయి { \
	లోతు++; \
	fprintf(stderr, "%*s", లోతు*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} అయితే (0)
#నిర్వచించు unnest_సందేశం(...) చేయి { \
	fprintf(stderr, "%*s", లోతు*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	లోతు--; \
} అయితే (0)
#నిర్వచించు dbg_సందేశం(...) చేయి { \
	fprintf(stderr, "%*s", లోతు*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} అయితే (0)
#నిర్వచించు unnest_సందేశం_and_return(సమాసం, ...) చేయి { \
	సంఖ్య_t __res = (సమాసం); \
	fprintf(stderr, "%*s", లోతు*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	లోతు--; \
	తిరిగి __res; \
} అయితే (0)
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి TOKSTR[] ALIGN_PTR = {
	"EOI",
	"FILRD",
	"FILWR",
	"FILEX",
	"FILEXIST",
	"FILREG",
	"FILDIR",
	"FILCDEV",
	"FILBDEV",
	"FILFIFO",
	"FILSOCK",
	"FILSYM",
	"FILGZ",
	"FILTT",
	"FILSUID",
	"FILSGID",
	"FILSTCK",
	"FILNT",
	"FILOT",
	"FILEQ",
	"FILUID",
	"FILGID",
	"STREZ",
	"STRNZ",
	"STREQ",
	"STRNE",
	"STRLT",
	"STRGT",
#ఒకవేళ BASH_TEST2
	"REGEX",
#అయితే_ముగింపు
	"INTEQ",
	"INTNE",
	"INTGE",
	"INTGT",
	"INTLE",
	"INTLT",
	"UNOT",
	"BAND",
	"BOR",
	"LPAREN",
	"RPAREN",
	"OPERAND"
};
#లేకపోతే
#నిర్వచించు nest_సందేశం(...)   ((శూన్య)0)
#నిర్వచించు unnest_సందేశం(...) ((శూన్య)0)
#నిర్వచించు dbg_సందేశం(...)    ((శూన్య)0)
#నిర్వచించు unnest_సందేశం_and_return(సమాసం, ...) తిరిగి సమాసం
#అయితే_ముగింపు

గణన {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

నిర్మాణం operator_t {
	గుర్తులేని అక్షరం op_అంకె, op_type;
};

స్థిర స్థిరరాశి నిర్మాణం operator_t ops_పట్టిక[] ALIGN2 = {
	{ /* "-r" */ FILRD   , UNOP   },
	{ /* "-w" */ FILWR   , UNOP   },
	{ /* "-x" */ FILEX   , UNOP   },
	{ /* "-e" */ FILEXIST, UNOP   },
	{ /* "-f" */ FILREG  , UNOP   },
	{ /* "-d" */ FILDIR  , UNOP   },
	{ /* "-c" */ FILCDEV , UNOP   },
	{ /* "-b" */ FILBDEV , UNOP   },
	{ /* "-p" */ FILFIFO , UNOP   },
	{ /* "-u" */ FILSUID , UNOP   },
	{ /* "-g" */ FILSGID , UNOP   },
	{ /* "-k" */ FILSTCK , UNOP   },
	{ /* "-s" */ FILGZ   , UNOP   },
	{ /* "-t" */ FILTT   , UNOP   },
	{ /* "-z" */ STREZ   , UNOP   },
	{ /* "-n" */ STRNZ   , UNOP   },
	{ /* "-h" */ FILSYM  , UNOP   },    /* for backwards compat */

	{ /* "-O" */ FILUID  , UNOP   },
	{ /* "-G" */ FILGID  , UNOP   },
	{ /* "-L" */ FILSYM  , UNOP   },
	{ /* "-S" */ FILSOCK , UNOP   },
	{ /* "="  */ STREQ   , BINOP  },
	/* "==" is bashism, http://pubs.opengroup.org/onlinepubs/9699919799/utilities/test.html
	 * lists only "=" as comparison operator.
	 */
	{ /* "==" */ STREQ   , BINOP  },
	{ /* "!=" */ STRNE   , BINOP  },
	{ /* "<"  */ STRLT   , BINOP  },
	{ /* ">"  */ STRGT   , BINOP  },
#ఒకవేళ BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#అయితే_ముగింపు
	{ /* "-eq"*/ INTEQ   , BINOP  },
	{ /* "-ne"*/ INTNE   , BINOP  },
	{ /* "-ge"*/ INTGE   , BINOP  },
	{ /* "-gt"*/ INTGT   , BINOP  },
	{ /* "-le"*/ INTLE   , BINOP  },
	{ /* "-lt"*/ INTLT   , BINOP  },
	{ /* "-nt"*/ FILNT   , BINOP  },
	{ /* "-ot"*/ FILOT   , BINOP  },
	{ /* "-ef"*/ FILEQ   , BINOP  },
	{ /* "!"  */ UNOT    , BUNOP  },
	{ /* "-a" */ BAND    , BBINOP },
	{ /* "-o" */ BOR     , BBINOP },
#ఒకవేళ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#అయితే_ముగింపు
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
స్థిర స్థిరరాశి అక్షరం ops_texts[] ALIGN1 =
	"-r"  "\0"
	"-w"  "\0"
	"-x"  "\0"
	"-e"  "\0"
	"-f"  "\0"
	"-d"  "\0"
	"-c"  "\0"
	"-b"  "\0"
	"-p"  "\0"
	"-u"  "\0"
	"-g"  "\0"
	"-k"  "\0"
	"-s"  "\0"
	"-t"  "\0"
	"-z"  "\0"
	"-n"  "\0"
	"-h"  "\0"

	"-O"  "\0"
	"-G"  "\0"
	"-L"  "\0"
	"-S"  "\0"
	"="   "\0"
	/* "==" is bashism */
	"=="  "\0"
	"!="  "\0"
	"<"   "\0"
	">"   "\0"
#ఒకవేళ BASH_TEST2
	"=~"  "\0"
#అయితే_ముగింపు
	"-eq" "\0"
	"-ne" "\0"
	"-ge" "\0"
	"-gt" "\0"
	"-le" "\0"
	"-lt" "\0"
	"-nt" "\0"
	"-ot" "\0"
	"-ef" "\0"
	"!"   "\0"
	"-a"  "\0"
	"-o"  "\0"
#ఒకవేళ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#అయితే_ముగింపు
	"("   "\0"
	")"   "\0"
;


#ఒకవేళ ENABLE_FEATURE_TEST_64
రకనిర్వచనం int64_t సంఖ్య_t;
#లేకపోతే
రకనిర్వచనం పూర్ణాంకం సంఖ్య_t;
#అయితే_ముగింపు


/* We try to minimize both static and stack usage. */
నిర్మాణం test_statics {
	అక్షరం **వాదనలు;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	స్థిరరాశి నిర్మాణం operator_t *చివరి_operator;
	gid_t *సమూహం_array;
	పూర్ణాంకం ngroups;
#ఒకవేళ BASH_TEST2
	bool bash_test2;
#అయితే_ముగింపు
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
బాహ్య నిర్మాణం test_statics *BB_GLOBAL_CONST test_ptr_గమ్యం_statics;

#నిర్వచించు S (*test_ptr_గమ్యం_statics)
#నిర్వచించు వాదనలు            (S.వాదనలు         )
#నిర్వచించు చివరి_operator   (S.చివరి_operator)
#నిర్వచించు సమూహం_array     (S.సమూహం_array  )
#నిర్వచించు ngroups         (S.ngroups      )
#నిర్వచించు bash_test2      (S.bash_test2   )
#నిర్వచించు leaving         (S.leaving      )

#నిర్వచించు INIT_S() చేయి { \
	XZALLOC_CONST_PTR(&test_ptr_గమ్యం_statics, పరిమాణం(S)); \
} అయితే (0)
#నిర్వచించు DEINIT_S() చేయి { \
	విడుదల(సమూహం_array); \
	విడుదల(test_ptr_గమ్యం_statics); \
} అయితే (0)

స్థిర సంఖ్య_t primary(గణన token n);

స్థిర శూన్య syntax(స్థిరరాశి అక్షరం *op, స్థిరరాశి అక్షరం *సందేశం) NORETURN;
స్థిర శూన్య syntax(స్థిరరాశి అక్షరం *op, స్థిరరాశి అక్షరం *సందేశం)
{
	ఒకవేళ (op && *op) {
		bb_error_సందేశం("%s: %s", op, సందేశం);
	} లేకపోతే {
		bb_error_సందేశం("%s: %s"+4, సందేశం);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
స్థిర సంఖ్య_t getn(స్థిరరాశి అక్షరం *s)
{
	అక్షరం *p;
#ఒకవేళ ENABLE_FEATURE_TEST_64
	దీర్ఘ దీర్ఘ r;
#లేకపోతే
	దీర్ఘ r;
#అయితే_ముగింపు

	errno = 0;
#ఒకవేళ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#లేకపోతే
	r = strtol(s, &p, 10);
#అయితే_ముగింపు

	ఒకవేళ (errno != 0)
		syntax(s, "out of range");

	ఒకవేళ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	తిరిగి r;
}

/*వాడని
static int newerf(const char *f1, const char *f2)
{
	struct stat b1, b2;

	return (stat(f1, &b1) == 0 &&
			stat(f2, &b2) == 0 && b1.st_mtime > b2.st_mtime);
}

static int olderf(const char *f1, const char *f2)
{
	struct stat b1, b2;

	return (stat(f1, &b1) == 0 &&
			stat(f2, &b2) == 0 && b1.st_mtime < b2.st_mtime);
}

static int equalf(const char *f1, const char *f2)
{
	struct stat b1, b2;

	return (stat(f1, &b1) == 0 &&
			stat(f2, &b2) == 0 &&
			b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino);
}
*/


స్థిర గణన token check_operator(స్థిరరాశి అక్షరం *s)
{
	స్థిర స్థిరరాశి నిర్మాణం operator_t no_op = {
		.op_అంకె = -1,
		.op_type = -1
	};
	పూర్ణాంకం n;

	చివరి_operator = &no_op;
	ఒకవేళ (s == ఖాళీ)
		తిరిగి EOI;
	n = సూచీ_లోపల_అక్షరశ్రేణులు(ops_texts, s);
	ఒకవేళ (n < 0)
		తిరిగి నిర్వాహ్యం;

#ఒకవేళ BASH_TEST2
	ఒకవేళ (ops_పట్టిక[n].op_అంకె == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		తిరిగి నిర్వాహ్యం;
	}
	ఒకవేళ (ops_పట్టిక[n].op_అంకె == BAND || ops_పట్టిక[n].op_అంకె == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		ఒకవేళ (bash_test2 == (s[0] == '-'))
			తిరిగి నిర్వాహ్యం;
	}
#అయితే_ముగింపు

	చివరి_operator = &ops_పట్టిక[n];
	తిరిగి ops_పట్టిక[n].op_అంకె;
}


స్థిర పూర్ణాంకం binop(శూన్య)
{
	స్థిరరాశి అక్షరం *opnd1, *opnd2;
	స్థిరరాశి నిర్మాణం operator_t *op;
	సంఖ్య_t val1, val2;

	opnd1 = *వాదనలు;
	check_operator(*++వాదనలు);
	op = చివరి_operator;

	opnd2 = *++వాదనలు;
	ఒకవేళ (opnd2 == ఖాళీ)
		syntax(వాదనలు[-1], "argument expected");

	ఒకవేళ (is_int_op(op->op_అంకె)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		ఒకవేళ (op->op_అంకె == INTEQ)
			తిరిగి val1 == val2;
		ఒకవేళ (op->op_అంకె == INTNE)
			తిరిగి val1 != val2;
		ఒకవేళ (op->op_అంకె == INTGE)
			తిరిగి val1 >= val2;
		ఒకవేళ (op->op_అంకె == INTGT)
			తిరిగి val1 >  val2;
		ఒకవేళ (op->op_అంకె == INTLE)
			తిరిగి val1 <= val2;
		/*if (op->op_num == INTLT)*/
		తిరిగి val1 <  val2;
	}
#ఒకవేళ BASH_TEST2
	ఒకవేళ (bash_test2) {
		ఒకవేళ (op->op_అంకె == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			తిరిగి val1 == 0;
		}
		ఒకవేళ (op->op_అంకె == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			తిరిగి val1 != 0;
		}
		ఒకవేళ (op->op_అంకె == REGEX) {
			regex_t re_బఫరు;
			memset(&re_బఫరు, 0, పరిమాణం(re_బఫరు));
			ఒకవేళ (regcomp(&re_బఫరు, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_బఫరు, opnd1, 0, ఖాళీ, 0);
			regfree(&re_బఫరు);
			తిరిగి val1 == 0;
		}
	}
#అయితే_ముగింపు
	ఒకవేళ (is_str_op(op->op_అంకె)) {
		val1 = strcmp(opnd1, opnd2);
		ఒకవేళ (op->op_అంకె == STREQ)
			తిరిగి val1 == 0;
		ఒకవేళ (op->op_అంకె == STRNE)
			తిరిగి val1 != 0;
		ఒకవేళ (op->op_అంకె == STRLT)
			తిరిగి val1 < 0;
		/*if (op->op_num == STRGT)*/
		తిరిగి val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		నిర్మాణం stat b1, b2;

		ఒకవేళ (stat(opnd1, &b1) || stat(opnd2, &b2))
			తిరిగి 0; /* false, since at least one stat failed */
		ఒకవేళ (op->op_అంకె == FILNT)
			తిరిగి b1.st_mtime > b2.st_mtime;
		ఒకవేళ (op->op_అంకె == FILOT)
			తిరిగి b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		తిరిగి b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

స్థిర శూన్య initialize_సమూహం_array(శూన్య)
{
	సమూహం_array = bb_getgroups(&ngroups, ఖాళీ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
స్థిర పూర్ణాంకం is_a_సమూహం_member(gid_t gid)
{
	పూర్ణాంకం i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	ఒకవేళ (gid == getgid() || gid == getegid())
		తిరిగి 1;

	ఒకవేళ (ngroups == 0)
		initialize_సమూహం_array();

	/* Search through the list looking for GID. */
	కొరకు (i = 0; i < ngroups; i++)
		ఒకవేళ (gid == సమూహం_array[i])
			తిరిగి 1;

	తిరిగి 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
స్థిర పూర్ణాంకం test_eaccess(నిర్మాణం stat *st, పూర్ణాంకం బహుళకం)
{
	గుర్తులేని పూర్ణాంకం euid = geteuid();

	ఒకవేళ (euid == 0) {
		/* Root can read or write any file. */
		ఒకవేళ (బహుళకం != X_OK)
			తిరిగి 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		ఒకవేళ (st->st_బహుళకం & (S_IXUSR | S_IXGRP | S_IXOTH))
			తిరిగి 0;
	}

	ఒకవేళ (st->st_uid == euid)  /* owner */
		బహుళకం <<= 6;
	లేకపోతే ఒకవేళ (is_a_సమూహం_member(st->st_gid))
		బహుళకం <<= 3;

	ఒకవేళ (st->st_బహుళకం & బహుళకం)
		తిరిగి 0;

	తిరిగి -1;
}


స్థిర పూర్ణాంకం filstat(అక్షరం *nm, గణన token బహుళకం)
{
	నిర్మాణం stat s;
	గుర్తులేని i = i; /* gcc 3.x thinks it can be used uninitialized */

	ఒకవేళ (బహుళకం == FILSYM) {
#నిర్వచితమైతే S_IFLNK
		ఒకవేళ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			వెళ్ళు filetype;
		}
#అయితే_ముగింపు
		తిరిగి 0;
	}

	ఒకవేళ (stat(nm, &s) != 0)
		తిరిగి 0;
	ఒకవేళ (బహుళకం == FILEXIST)
		తిరిగి 1;
	ఒకవేళ (is_file_access(బహుళకం)) {
		ఒకవేళ (బహుళకం == FILRD)
			i = R_OK;
		ఒకవేళ (బహుళకం == FILWR)
			i = W_OK;
		ఒకవేళ (బహుళకం == FILEX)
			i = X_OK;
		తిరిగి test_eaccess(&s, i) == 0;
	}
	ఒకవేళ (is_file_type(బహుళకం)) {
		ఒకవేళ (బహుళకం == FILREG)
			i = S_IFREG;
		ఒకవేళ (బహుళకం == FILDIR)
			i = S_IFDIR;
		ఒకవేళ (బహుళకం == FILCDEV)
			i = S_IFCHR;
		ఒకవేళ (బహుళకం == FILBDEV)
			i = S_IFBLK;
		ఒకవేళ (బహుళకం == FILFIFO) {
#నిర్వచితమైతే S_IFIFO
			i = S_IFIFO;
#లేకపోతే
			తిరిగి 0;
#అయితే_ముగింపు
		}
		ఒకవేళ (బహుళకం == FILSOCK) {
#నిర్వచితమైతే S_IFSOCK
			i = S_IFSOCK;
#లేకపోతే
			తిరిగి 0;
#అయితే_ముగింపు
		}
 filetype:
		తిరిగి ((s.st_బహుళకం & S_IFMT) == i);
	}
	ఒకవేళ (is_file_బిట్టు(బహుళకం)) {
		ఒకవేళ (బహుళకం == FILSUID)
			i = S_ISUID;
		ఒకవేళ (బహుళకం == FILSGID)
			i = S_ISGID;
		ఒకవేళ (బహుళకం == FILSTCK)
			i = S_ISVTX;
		తిరిగి ((s.st_బహుళకం & i) != 0);
	}
	ఒకవేళ (బహుళకం == FILGZ)
		తిరిగి s.st_కొలత > 0L;
	ఒకవేళ (బహుళకం == FILUID)
		తిరిగి s.st_uid == geteuid();
	ఒకవేళ (బహుళకం == FILGID)
		తిరిగి s.st_gid == getegid();
	తిరిగి 1; /* NOTREACHED */
}


స్థిర సంఖ్య_t nexpr(గణన token n)
{
	సంఖ్య_t res;

	nest_సందేశం(">nexpr(%s)\n", TOKSTR[n]);
	ఒకవేళ (n == UNOT) {
		n = check_operator(*++వాదనలు);
		ఒకవేళ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			వాదనలు--;
			unnest_సందేశం("<nexpr:1 (!EOI), args:%s(%p)\n", వాదనలు[0], &వాదనలు[0]);
			తిరిగి 1;
		}
		res = !nexpr(n);
		unnest_సందేశం("<nexpr:%lld\n", res);
		తిరిగి res;
	}
	res = primary(n);
	unnest_సందేశం("<nexpr:%lld\n", res);
	తిరిగి res;
}


స్థిర సంఖ్య_t aexpr(గణన token n)
{
	సంఖ్య_t res;

	nest_సందేశం(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_సందేశం("aexpr: nexpr:%lld, next args:%s(%p)\n", res, వాదనలు[1], &వాదనలు[1]);
	ఒకవేళ (check_operator(*++వాదనలు) == BAND) {
		dbg_సందేశం("aexpr: arg is AND, next args:%s(%p)\n", వాదనలు[1], &వాదనలు[1]);
		res = aexpr(check_operator(*++వాదనలు)) && res;
		unnest_సందేశం("<aexpr:%lld\n", res);
		తిరిగి res;
	}
	వాదనలు--;
	unnest_సందేశం("<aexpr:%lld, args:%s(%p)\n", res, వాదనలు[0], &వాదనలు[0]);
	తిరిగి res;
}


స్థిర సంఖ్య_t oexpr(గణన token n)
{
	సంఖ్య_t res;

	nest_సందేశం(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_సందేశం("oexpr: aexpr:%lld, next args:%s(%p)\n", res, వాదనలు[1], &వాదనలు[1]);
	ఒకవేళ (check_operator(*++వాదనలు) == BOR) {
		dbg_సందేశం("oexpr: next arg is OR, next args:%s(%p)\n", వాదనలు[1], &వాదనలు[1]);
		res = oexpr(check_operator(*++వాదనలు)) || res;
		unnest_సందేశం("<oexpr:%lld\n", res);
		తిరిగి res;
	}
	వాదనలు--;
	unnest_సందేశం("<oexpr:%lld, args:%s(%p)\n", res, వాదనలు[0], &వాదనలు[0]);
	తిరిగి res;
}


స్థిర సంఖ్య_t primary(గణన token n)
{
#ఒకవేళ TEST_DEBUG
	సంఖ్య_t res = res; /* for compiler */
#లేకపోతే
	సంఖ్య_t res;
#అయితే_ముగింపు
	స్థిరరాశి నిర్మాణం operator_t *args0_op;

	nest_సందేశం(">primary(%s)\n", TOKSTR[n]);
	ఒకవేళ (n == EOI) {
		syntax(ఖాళీ, "argument expected");
	}
	ఒకవేళ (n == LPAREN) {
		res = oexpr(check_operator(*++వాదనలు));
		ఒకవేళ (check_operator(*++వాదనలు) != RPAREN)
			syntax(ఖాళీ, "closing paren expected");
		unnest_సందేశం("<primary:%lld\n", res);
		తిరిగి res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = చివరి_operator;
	/* last_operator = operator at args[1] */
	ఒకవేళ (check_operator(వాదనలు[1]) != EOI) { /* if args[1] != NULL */
		ఒకవేళ (వాదనలు[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			ఒకవేళ (చివరి_operator->op_type == BINOP)
				unnest_సందేశం_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	ఒకవేళ (args0_op->op_type == UNOP) {
		/* unary expression */
		ఒకవేళ (వాదనలు[1] == ఖాళీ)
//			syntax(args0_op->op_text, "argument expected");
			వెళ్ళు check_emptiness;
		వాదనలు++;
		ఒకవేళ (n == STREZ)
			unnest_సందేశం_and_return(వాదనలు[0][0] == '\0', "<primary:%lld\n");
		ఒకవేళ (n == STRNZ)
			unnest_సందేశం_and_return(వాదనలు[0][0] != '\0', "<primary:%lld\n");
		ఒకవేళ (n == FILTT)
			unnest_సందేశం_and_return(isatty(getn(*వాదనలు)), "<primary: isatty(%s)%lld\n", *వాదనలు);
		unnest_సందేశం_and_return(filstat(*వాదనలు, n), "<primary: filstat(%s):%lld\n", *వాదనలు);
	}

	/*check_operator(args[1]); - already done */
	ఒకవేళ (చివరి_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_సందేశం_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_సందేశం_and_return(వాదనలు[0][0] != '\0', "<primary:%lld\n");
}


పూర్ణాంకం test_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం res;
	స్థిరరాశి అక్షరం *arg0;
#ఒకవేళ BASH_TEST2
	bool bt2 = 0;
#అయితే_ముగింపు

	arg0 = bb_basename(argv[0]);
	ఒకవేళ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		ఒకవేళ (!arg0[1]) { /* "[" ? */
			ఒకవేళ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_సందేశం("missing ]");
				తిరిగి 2;
			}
		} లేకపోతే { /* assuming "[[" */
			ఒకవేళ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_సందేశం("missing ]]");
				తిరిగి 2;
			}
#ఒకవేళ BASH_TEST2
			bt2 = 1;
#అయితే_ముగింపు
		}
		argv[argc] = ఖాళీ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#ఒకవేళ BASH_TEST2
	bash_test2 = bt2;
#అయితే_ముగింపు

	res = setjmp(leaving);
	ఒకవేళ (res)
		వెళ్ళు ret;

	/* resetting ngroups is probably unnecessary.  it will
	 * force a new call to getgroups(), which prevents using
	 * group data fetched during a previous call.  but the
	 * only way the group data could be stale is if there's
	 * been an intervening call to setgroups(), and this
	 * isn't likely in the case of a shell.  paranoia
	 * prevails...
	 */
	/*ngroups = 0; - done by INIT_S() */

	argv++;
	వాదనలు = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	ఒకవేళ (1) {
		పూర్ణాంకం negate = 0;
 again:
		ఒకవేళ (!argv[0]) {
			/* "test" */
			res = 1;
			వెళ్ళు ret_special;
		}
		ఒకవేళ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			వెళ్ళు ret_special;
		}
		ఒకవేళ (argv[2]) {
			ఒకవేళ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				ఒకవేళ (చివరి_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					వాదనలు = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					వెళ్ళు ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				వెళ్ళు check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			ఒకవేళ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				ఒకవేళ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ఖాళీ;
					argv++;
				}
			}
		}
 check_negate:
		ఒకవేళ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			వెళ్ళు again;
		}
	}

	res = !oexpr(check_operator(*వాదనలు));

	ఒకవేళ (*వాదనలు != ఖాళీ && *++వాదనలు != ఖాళీ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_సందేశం("%s: unknown operand", *వాదనలు);
		res = 2;
	}
 ret:
	DEINIT_S();
	తిరిగి res;
}
