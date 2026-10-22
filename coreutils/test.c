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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव <regex.h>
#अन्तर्भाव <fnmatch.h>

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

#परिभाषय TEST_DEBUG 0

#यदि ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#अन्यथा
# define BASH_TEST2 0
#यदि_अन्तः

गणना token {
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

#यदि BASH_TEST2
	REGEX,
#यदि_अन्तः

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
	सङ्कार्यम्
};
#परिभाषय is_int_op(a)      (((अचिह्नित वर्ण)((a) - INTEQ)) <= 5)
#परिभाषय is_str_op(a)      (((अचिह्नित वर्ण)((a) - STREZ)) <= 5)
#परिभाषय is_file_op(a)     (((अचिह्नित वर्ण)((a) - FILNT)) <= 2)
#परिभाषय is_file_access(a) (((अचिह्नित वर्ण)((a) - FILRD)) <= 2)
#परिभाषय is_file_type(a)   (((अचिह्नित वर्ण)((a) - FILREG)) <= 5)
#परिभाषय is_file_bit(a)    (((अचिह्नित वर्ण)((a) - FILSUID)) <= 2)

#यदि TEST_DEBUG
पूर्णाङ्क गभीरता;
#परिभाषय nest_सन्देशः(...) कुरु { \
	गभीरता++; \
	fprintf(stderr, "%*s", गभीरता*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} यावत् (0)
#परिभाषय unnest_सन्देशः(...) कुरु { \
	fprintf(stderr, "%*s", गभीरता*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	गभीरता--; \
} यावत् (0)
#परिभाषय dbg_सन्देशः(...) कुरु { \
	fprintf(stderr, "%*s", गभीरता*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} यावत् (0)
#परिभाषय unnest_सन्देशः_and_return(व्यञ्जकम्, ...) कुरु { \
	सङ्ख्या_t __res = (व्यञ्जकम्); \
	fprintf(stderr, "%*s", गभीरता*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	गभीरता--; \
	प्रत्यागम __res; \
} यावत् (0)
स्थिर ध्रुव वर्ण *ध्रुव TOKSTR[] ALIGN_PTR = {
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
#यदि BASH_TEST2
	"REGEX",
#यदि_अन्तः
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
#अन्यथा
#परिभाषय nest_सन्देशः(...)   ((शून्य)0)
#परिभाषय unnest_सन्देशः(...) ((शून्य)0)
#परिभाषय dbg_सन्देशः(...)    ((शून्य)0)
#परिभाषय unnest_सन्देशः_and_return(व्यञ्जकम्, ...) प्रत्यागम व्यञ्जकम्
#यदि_अन्तः

गणना {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

संरचना operator_t {
	अचिह्नित वर्ण op_अङ्कः, op_type;
};

स्थिर ध्रुव संरचना operator_t ops_सारणी[] ALIGN2 = {
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
#यदि BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#यदि_अन्तः
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
#यदि BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#यदि_अन्तः
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
स्थिर ध्रुव वर्ण ops_texts[] ALIGN1 =
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
#यदि BASH_TEST2
	"=~"  "\0"
#यदि_अन्तः
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
#यदि BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#यदि_अन्तः
	"("   "\0"
	")"   "\0"
;


#यदि ENABLE_FEATURE_TEST_64
प्रकारनिर्देश int64_t सङ्ख्या_t;
#अन्यथा
प्रकारनिर्देश पूर्णाङ्क सङ्ख्या_t;
#यदि_अन्तः


/* We try to minimize both static and stack usage. */
संरचना test_statics {
	वर्ण **तर्काः;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ध्रुव संरचना operator_t *चरमः_operator;
	gid_t *गणः_array;
	पूर्णाङ्क ngroups;
#यदि BASH_TEST2
	bool bash_test2;
#यदि_अन्तः
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
बाह्य संरचना test_statics *BB_GLOBAL_CONST test_ptr_गन्तव्यम्_statics;

#परिभाषय S (*test_ptr_गन्तव्यम्_statics)
#परिभाषय तर्काः            (S.तर्काः         )
#परिभाषय चरमः_operator   (S.चरमः_operator)
#परिभाषय गणः_array     (S.गणः_array  )
#परिभाषय ngroups         (S.ngroups      )
#परिभाषय bash_test2      (S.bash_test2   )
#परिभाषय leaving         (S.leaving      )

#परिभाषय INIT_S() कुरु { \
	XZALLOC_CONST_PTR(&test_ptr_गन्तव्यम्_statics, परिमाण(S)); \
} यावत् (0)
#परिभाषय DEINIT_S() कुरु { \
	मुक्त(गणः_array); \
	मुक्त(test_ptr_गन्तव्यम्_statics); \
} यावत् (0)

स्थिर सङ्ख्या_t primary(गणना token n);

स्थिर शून्य syntax(ध्रुव वर्ण *op, ध्रुव वर्ण *सन्देशः) NORETURN;
स्थिर शून्य syntax(ध्रुव वर्ण *op, ध्रुव वर्ण *सन्देशः)
{
	यदि (op && *op) {
		bb_error_सन्देशः("%s: %s", op, सन्देशः);
	} अन्यथा {
		bb_error_सन्देशः("%s: %s"+4, सन्देशः);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
स्थिर सङ्ख्या_t getn(ध्रुव वर्ण *s)
{
	वर्ण *p;
#यदि ENABLE_FEATURE_TEST_64
	दीर्घ दीर्घ r;
#अन्यथा
	दीर्घ r;
#यदि_अन्तः

	errno = 0;
#यदि ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#अन्यथा
	r = strtol(s, &p, 10);
#यदि_अन्तः

	यदि (errno != 0)
		syntax(s, "out of range");

	यदि (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	प्रत्यागम r;
}

/*अप्रयुक्तम्
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


स्थिर गणना token check_operator(ध्रुव वर्ण *s)
{
	स्थिर ध्रुव संरचना operator_t no_op = {
		.op_अङ्कः = -1,
		.op_type = -1
	};
	पूर्णाङ्क n;

	चरमः_operator = &no_op;
	यदि (s == रिक्त)
		प्रत्यागम EOI;
	n = सूचकांक_अन्तर्_वाक्यानि(ops_texts, s);
	यदि (n < 0)
		प्रत्यागम सङ्कार्यम्;

#यदि BASH_TEST2
	यदि (ops_सारणी[n].op_अङ्कः == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		प्रत्यागम सङ्कार्यम्;
	}
	यदि (ops_सारणी[n].op_अङ्कः == BAND || ops_सारणी[n].op_अङ्कः == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		यदि (bash_test2 == (s[0] == '-'))
			प्रत्यागम सङ्कार्यम्;
	}
#यदि_अन्तः

	चरमः_operator = &ops_सारणी[n];
	प्रत्यागम ops_सारणी[n].op_अङ्कः;
}


स्थिर पूर्णाङ्क binop(शून्य)
{
	ध्रुव वर्ण *opnd1, *opnd2;
	ध्रुव संरचना operator_t *op;
	सङ्ख्या_t val1, val2;

	opnd1 = *तर्काः;
	check_operator(*++तर्काः);
	op = चरमः_operator;

	opnd2 = *++तर्काः;
	यदि (opnd2 == रिक्त)
		syntax(तर्काः[-1], "argument expected");

	यदि (is_int_op(op->op_अङ्कः)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		यदि (op->op_अङ्कः == INTEQ)
			प्रत्यागम val1 == val2;
		यदि (op->op_अङ्कः == INTNE)
			प्रत्यागम val1 != val2;
		यदि (op->op_अङ्कः == INTGE)
			प्रत्यागम val1 >= val2;
		यदि (op->op_अङ्कः == INTGT)
			प्रत्यागम val1 >  val2;
		यदि (op->op_अङ्कः == INTLE)
			प्रत्यागम val1 <= val2;
		/*if (op->op_num == INTLT)*/
		प्रत्यागम val1 <  val2;
	}
#यदि BASH_TEST2
	यदि (bash_test2) {
		यदि (op->op_अङ्कः == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			प्रत्यागम val1 == 0;
		}
		यदि (op->op_अङ्कः == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			प्रत्यागम val1 != 0;
		}
		यदि (op->op_अङ्कः == REGEX) {
			regex_t re_बफर;
			स्मृतिं_पूरय(&re_बफर, 0, परिमाण(re_बफर));
			यदि (regcomp(&re_बफर, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_बफर, opnd1, 0, रिक्त, 0);
			regfree(&re_बफर);
			प्रत्यागम val1 == 0;
		}
	}
#यदि_अन्तः
	यदि (is_str_op(op->op_अङ्कः)) {
		val1 = वाक्यं_तुलय(opnd1, opnd2);
		यदि (op->op_अङ्कः == STREQ)
			प्रत्यागम val1 == 0;
		यदि (op->op_अङ्कः == STRNE)
			प्रत्यागम val1 != 0;
		यदि (op->op_अङ्कः == STRLT)
			प्रत्यागम val1 < 0;
		/*if (op->op_num == STRGT)*/
		प्रत्यागम val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		संरचना stat b1, b2;

		यदि (stat(opnd1, &b1) || stat(opnd2, &b2))
			प्रत्यागम 0; /* false, since at least one stat failed */
		यदि (op->op_अङ्कः == FILNT)
			प्रत्यागम b1.st_mtime > b2.st_mtime;
		यदि (op->op_अङ्कः == FILOT)
			प्रत्यागम b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		प्रत्यागम b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

स्थिर शून्य initialize_गणः_array(शून्य)
{
	गणः_array = bb_getgroups(&ngroups, रिक्त);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
स्थिर पूर्णाङ्क is_a_गणः_member(gid_t gid)
{
	पूर्णाङ्क i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	यदि (gid == getgid() || gid == getegid())
		प्रत्यागम 1;

	यदि (ngroups == 0)
		initialize_गणः_array();

	/* Search through the list looking for GID. */
	प्रति (i = 0; i < ngroups; i++)
		यदि (gid == गणः_array[i])
			प्रत्यागम 1;

	प्रत्यागम 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
स्थिर पूर्णाङ्क test_eaccess(संरचना stat *st, पूर्णाङ्क बहुलकम्)
{
	अचिह्नित पूर्णाङ्क euid = geteuid();

	यदि (euid == 0) {
		/* Root can read or write any file. */
		यदि (बहुलकम् != X_OK)
			प्रत्यागम 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		यदि (st->st_बहुलकम् & (S_IXUSR | S_IXGRP | S_IXOTH))
			प्रत्यागम 0;
	}

	यदि (st->st_uid == euid)  /* owner */
		बहुलकम् <<= 6;
	अन्यथा यदि (is_a_गणः_member(st->st_gid))
		बहुलकम् <<= 3;

	यदि (st->st_बहुलकम् & बहुलकम्)
		प्रत्यागम 0;

	प्रत्यागम -1;
}


स्थिर पूर्णाङ्क filstat(वर्ण *nm, गणना token बहुलकम्)
{
	संरचना stat s;
	अचिह्नित i = i; /* gcc 3.x thinks it can be used uninitialized */

	यदि (बहुलकम् == FILSYM) {
#यदि_परिभाषितम् S_IFLNK
		यदि (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			गच्छ filetype;
		}
#यदि_अन्तः
		प्रत्यागम 0;
	}

	यदि (stat(nm, &s) != 0)
		प्रत्यागम 0;
	यदि (बहुलकम् == FILEXIST)
		प्रत्यागम 1;
	यदि (is_file_access(बहुलकम्)) {
		यदि (बहुलकम् == FILRD)
			i = R_OK;
		यदि (बहुलकम् == FILWR)
			i = W_OK;
		यदि (बहुलकम् == FILEX)
			i = X_OK;
		प्रत्यागम test_eaccess(&s, i) == 0;
	}
	यदि (is_file_type(बहुलकम्)) {
		यदि (बहुलकम् == FILREG)
			i = S_IFREG;
		यदि (बहुलकम् == FILDIR)
			i = S_IFDIR;
		यदि (बहुलकम् == FILCDEV)
			i = S_IFCHR;
		यदि (बहुलकम् == FILBDEV)
			i = S_IFBLK;
		यदि (बहुलकम् == FILFIFO) {
#यदि_परिभाषितम् S_IFIFO
			i = S_IFIFO;
#अन्यथा
			प्रत्यागम 0;
#यदि_अन्तः
		}
		यदि (बहुलकम् == FILSOCK) {
#यदि_परिभाषितम् S_IFSOCK
			i = S_IFSOCK;
#अन्यथा
			प्रत्यागम 0;
#यदि_अन्तः
		}
 filetype:
		प्रत्यागम ((s.st_बहुलकम् & S_IFMT) == i);
	}
	यदि (is_file_bit(बहुलकम्)) {
		यदि (बहुलकम् == FILSUID)
			i = S_ISUID;
		यदि (बहुलकम् == FILSGID)
			i = S_ISGID;
		यदि (बहुलकम् == FILSTCK)
			i = S_ISVTX;
		प्रत्यागम ((s.st_बहुलकम् & i) != 0);
	}
	यदि (बहुलकम् == FILGZ)
		प्रत्यागम s.st_आकार > 0L;
	यदि (बहुलकम् == FILUID)
		प्रत्यागम s.st_uid == geteuid();
	यदि (बहुलकम् == FILGID)
		प्रत्यागम s.st_gid == getegid();
	प्रत्यागम 1; /* NOTREACHED */
}


स्थिर सङ्ख्या_t nexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">nexpr(%s)\n", TOKSTR[n]);
	यदि (n == UNOT) {
		n = check_operator(*++तर्काः);
		यदि (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			तर्काः--;
			unnest_सन्देशः("<nexpr:1 (!EOI), args:%s(%p)\n", तर्काः[0], &तर्काः[0]);
			प्रत्यागम 1;
		}
		res = !nexpr(n);
		unnest_सन्देशः("<nexpr:%lld\n", res);
		प्रत्यागम res;
	}
	res = primary(n);
	unnest_सन्देशः("<nexpr:%lld\n", res);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t aexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_सन्देशः("aexpr: nexpr:%lld, next args:%s(%p)\n", res, तर्काः[1], &तर्काः[1]);
	यदि (check_operator(*++तर्काः) == BAND) {
		dbg_सन्देशः("aexpr: arg is AND, next args:%s(%p)\n", तर्काः[1], &तर्काः[1]);
		res = aexpr(check_operator(*++तर्काः)) && res;
		unnest_सन्देशः("<aexpr:%lld\n", res);
		प्रत्यागम res;
	}
	तर्काः--;
	unnest_सन्देशः("<aexpr:%lld, args:%s(%p)\n", res, तर्काः[0], &तर्काः[0]);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t oexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_सन्देशः("oexpr: aexpr:%lld, next args:%s(%p)\n", res, तर्काः[1], &तर्काः[1]);
	यदि (check_operator(*++तर्काः) == BOR) {
		dbg_सन्देशः("oexpr: next arg is OR, next args:%s(%p)\n", तर्काः[1], &तर्काः[1]);
		res = oexpr(check_operator(*++तर्काः)) || res;
		unnest_सन्देशः("<oexpr:%lld\n", res);
		प्रत्यागम res;
	}
	तर्काः--;
	unnest_सन्देशः("<oexpr:%lld, args:%s(%p)\n", res, तर्काः[0], &तर्काः[0]);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t primary(गणना token n)
{
#यदि TEST_DEBUG
	सङ्ख्या_t res = res; /* for compiler */
#अन्यथा
	सङ्ख्या_t res;
#यदि_अन्तः
	ध्रुव संरचना operator_t *args0_op;

	nest_सन्देशः(">primary(%s)\n", TOKSTR[n]);
	यदि (n == EOI) {
		syntax(रिक्त, "argument expected");
	}
	यदि (n == LPAREN) {
		res = oexpr(check_operator(*++तर्काः));
		यदि (check_operator(*++तर्काः) != RPAREN)
			syntax(रिक्त, "closing paren expected");
		unnest_सन्देशः("<primary:%lld\n", res);
		प्रत्यागम res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = चरमः_operator;
	/* last_operator = operator at args[1] */
	यदि (check_operator(तर्काः[1]) != EOI) { /* if args[1] != NULL */
		यदि (तर्काः[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			यदि (चरमः_operator->op_type == BINOP)
				unnest_सन्देशः_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	यदि (args0_op->op_type == UNOP) {
		/* unary expression */
		यदि (तर्काः[1] == रिक्त)
//			syntax(args0_op->op_text, "argument expected");
			गच्छ check_emptiness;
		तर्काः++;
		यदि (n == STREZ)
			unnest_सन्देशः_and_return(तर्काः[0][0] == '\0', "<primary:%lld\n");
		यदि (n == STRNZ)
			unnest_सन्देशः_and_return(तर्काः[0][0] != '\0', "<primary:%lld\n");
		यदि (n == FILTT)
			unnest_सन्देशः_and_return(isatty(getn(*तर्काः)), "<primary: isatty(%s)%lld\n", *तर्काः);
		unnest_सन्देशः_and_return(filstat(*तर्काः, n), "<primary: filstat(%s):%lld\n", *तर्काः);
	}

	/*check_operator(args[1]); - already done */
	यदि (चरमः_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_सन्देशः_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_सन्देशः_and_return(तर्काः[0][0] != '\0', "<primary:%lld\n");
}


पूर्णाङ्क test_main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क res;
	ध्रुव वर्ण *arg0;
#यदि BASH_TEST2
	bool bt2 = 0;
#यदि_अन्तः

	arg0 = bb_basename(argv[0]);
	यदि ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		यदि (!arg0[1]) { /* "[" ? */
			यदि (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_सन्देशः("missing ]");
				प्रत्यागम 2;
			}
		} अन्यथा { /* assuming "[[" */
			यदि (वाक्यं_तुलय(argv[argc], "]]") != 0) {
				bb_simple_error_सन्देशः("missing ]]");
				प्रत्यागम 2;
			}
#यदि BASH_TEST2
			bt2 = 1;
#यदि_अन्तः
		}
		argv[argc] = रिक्त;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#यदि BASH_TEST2
	bash_test2 = bt2;
#यदि_अन्तः

	res = setjmp(leaving);
	यदि (res)
		गच्छ ret;

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
	तर्काः = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	यदि (1) {
		पूर्णाङ्क negate = 0;
 again:
		यदि (!argv[0]) {
			/* "test" */
			res = 1;
			गच्छ ret_special;
		}
		यदि (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			गच्छ ret_special;
		}
		यदि (argv[2]) {
			यदि (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				यदि (चरमः_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					तर्काः = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					गच्छ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				गच्छ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			यदि (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				यदि (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = रिक्त;
					argv++;
				}
			}
		}
 check_negate:
		यदि (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			गच्छ again;
		}
	}

	res = !oexpr(check_operator(*तर्काः));

	यदि (*तर्काः != रिक्त && *++तर्काः != रिक्त) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_सन्देशः("%s: unknown operand", *तर्काः);
		res = 2;
	}
 ret:
	DEINIT_S();
	प्रत्यागम res;
}
