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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ <regex.h>
#ಅನ್ತರ್ಭಾವ <fnmatch.h>

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

#define TEST_DEBUG 0

#ಯದಿ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#ಅನ್ಯಥಾ
# define BASH_TEST2 0
#endif

ಗಣನಾ token {
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

#ಯದಿ BASH_TEST2
	REGEX,
#endif

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
	OPERAND
};
#define is_int_op(a)      (((unsigned ವರ್ಣ)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned ವರ್ಣ)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned ವರ್ಣ)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned ವರ್ಣ)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned ವರ್ಣ)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned ವರ್ಣ)((a) - FILSUID)) <= 2)

#ಯದಿ TEST_DEBUG
ಪೂರ್ಣಾಙ್ಕ depth;
#define nest_ಸಂದೇಶ(...) ಕುರು { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ಯಾವತ್ (0)
#define unnest_ಸಂದೇಶ(...) ಕುರು { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} ಯಾವತ್ (0)
#define dbg_ಸಂದೇಶ(...) ಕುರು { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ಯಾವತ್ (0)
#define unnest_ಸಂದೇಶ_and_return(ಅಭಿವ್ಯಕ್ತಿ, ...) ಕುರು { \
	ಸಂಖ್ಯೆ_t __res = (ಅಭಿವ್ಯಕ್ತಿ); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	ಪ್ರತ್ಯಾಗಮ __res; \
} ಯಾವತ್ (0)
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ TOKSTR[] ALIGN_PTR = {
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
#ಯದಿ BASH_TEST2
	"REGEX",
#endif
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
#ಅನ್ಯಥಾ
#define nest_ಸಂದೇಶ(...)   ((ಶೂನ್ಯ)0)
#define unnest_ಸಂದೇಶ(...) ((ಶೂನ್ಯ)0)
#define dbg_ಸಂದೇಶ(...)    ((ಶೂನ್ಯ)0)
#define unnest_ಸಂದೇಶ_and_return(ಅಭಿವ್ಯಕ್ತಿ, ...) ಪ್ರತ್ಯಾಗಮ ಅಭಿವ್ಯಕ್ತಿ
#endif

ಗಣನಾ {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ಸಂರಚನಾ operator_t {
	unsigned ವರ್ಣ op_ಅಂಕೆ, op_type;
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ operator_t ops_table[] ALIGN2 = {
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
#ಯದಿ BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#endif
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
#ಯದಿ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ ops_texts[] ALIGN1 =
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
#ಯದಿ BASH_TEST2
	"=~"  "\0"
#endif
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
#ಯದಿ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#ಯದಿ ENABLE_FEATURE_TEST_64
ಪ್ರಕಾರನಿರ್ದೇಶ int64_t ಸಂಖ್ಯೆ_t;
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ ಸಂಖ್ಯೆ_t;
#endif


/* We try to minimize both static and stack usage. */
ಸಂರಚನಾ test_statics {
	ವರ್ಣ **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ಧ್ರುವ ಸಂರಚನಾ operator_t *last_operator;
	gid_t *group_array;
	ಪೂರ್ಣಾಙ್ಕ ngroups;
#ಯದಿ BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
ಬಾಹ್ಯ ಸಂರಚನಾ test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() ಕುರು { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, ಪರಿಮಾಣ(S)); \
} ಯಾವತ್ (0)
#define DEINIT_S() ಕುರು { \
	ಮುಕ್ತ(group_array); \
	ಮುಕ್ತ(test_ptr_to_statics); \
} ಯಾವತ್ (0)

ಸ್ಥಿರ ಸಂಖ್ಯೆ_t primary(ಗಣನಾ token n);

ಸ್ಥಿರ ಶೂನ್ಯ syntax(ಧ್ರುವ ವರ್ಣ *op, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ syntax(ಧ್ರುವ ವರ್ಣ *op, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	ಯದಿ (op && *op) {
		bb_error_ಸಂದೇಶ("%s: %s", op, ಸಂದೇಶ);
	} ಅನ್ಯಥಾ {
		bb_error_ಸಂದೇಶ("%s: %s"+4, ಸಂದೇಶ);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ಸ್ಥಿರ ಸಂಖ್ಯೆ_t getn(ಧ್ರುವ ವರ್ಣ *s)
{
	ವರ್ಣ *p;
#ಯದಿ ENABLE_FEATURE_TEST_64
	long long r;
#ಅನ್ಯಥಾ
	long r;
#endif

	errno = 0;
#ಯದಿ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#ಅನ್ಯಥಾ
	r = strtol(s, &p, 10);
#endif

	ಯದಿ (errno != 0)
		syntax(s, "out of range");

	ಯದಿ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ಪ್ರತ್ಯಾಗಮ r;
}

/* UNUSED
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


ಸ್ಥಿರ ಗಣನಾ token check_operator(ಧ್ರುವ ವರ್ಣ *s)
{
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ operator_t no_op = {
		.op_ಅಂಕೆ = -1,
		.op_type = -1
	};
	ಪೂರ್ಣಾಙ್ಕ n;

	last_operator = &no_op;
	ಯದಿ (s == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ EOI;
	n = index_in_strings(ops_texts, s);
	ಯದಿ (n < 0)
		ಪ್ರತ್ಯಾಗಮ OPERAND;

#ಯದಿ BASH_TEST2
	ಯದಿ (ops_table[n].op_ಅಂಕೆ == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ಪ್ರತ್ಯಾಗಮ OPERAND;
	}
	ಯದಿ (ops_table[n].op_ಅಂಕೆ == BAND || ops_table[n].op_ಅಂಕೆ == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		ಯದಿ (bash_test2 == (s[0] == '-'))
			ಪ್ರತ್ಯಾಗಮ OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	ಪ್ರತ್ಯಾಗಮ ops_table[n].op_ಅಂಕೆ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ binop(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *opnd1, *opnd2;
	ಧ್ರುವ ಸಂರಚನಾ operator_t *op;
	ಸಂಖ್ಯೆ_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	ಯದಿ (opnd2 == ರಿಕ್ತ)
		syntax(args[-1], "argument expected");

	ಯದಿ (is_int_op(op->op_ಅಂಕೆ)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		ಯದಿ (op->op_ಅಂಕೆ == INTEQ)
			ಪ್ರತ್ಯಾಗಮ val1 == val2;
		ಯದಿ (op->op_ಅಂಕೆ == INTNE)
			ಪ್ರತ್ಯಾಗಮ val1 != val2;
		ಯದಿ (op->op_ಅಂಕೆ == INTGE)
			ಪ್ರತ್ಯಾಗಮ val1 >= val2;
		ಯದಿ (op->op_ಅಂಕೆ == INTGT)
			ಪ್ರತ್ಯಾಗಮ val1 >  val2;
		ಯದಿ (op->op_ಅಂಕೆ == INTLE)
			ಪ್ರತ್ಯಾಗಮ val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ಪ್ರತ್ಯಾಗಮ val1 <  val2;
	}
#ಯದಿ BASH_TEST2
	ಯದಿ (bash_test2) {
		ಯದಿ (op->op_ಅಂಕೆ == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ಪ್ರತ್ಯಾಗಮ val1 == 0;
		}
		ಯದಿ (op->op_ಅಂಕೆ == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ಪ್ರತ್ಯಾಗಮ val1 != 0;
		}
		ಯದಿ (op->op_ಅಂಕೆ == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, ಪರಿಮಾಣ(re_buffer));
			ಯದಿ (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, ರಿಕ್ತ, 0);
			regfree(&re_buffer);
			ಪ್ರತ್ಯಾಗಮ val1 == 0;
		}
	}
#endif
	ಯದಿ (is_str_op(op->op_ಅಂಕೆ)) {
		val1 = strcmp(opnd1, opnd2);
		ಯದಿ (op->op_ಅಂಕೆ == STREQ)
			ಪ್ರತ್ಯಾಗಮ val1 == 0;
		ಯದಿ (op->op_ಅಂಕೆ == STRNE)
			ಪ್ರತ್ಯಾಗಮ val1 != 0;
		ಯದಿ (op->op_ಅಂಕೆ == STRLT)
			ಪ್ರತ್ಯಾಗಮ val1 < 0;
		/*if (op->op_num == STRGT)*/
		ಪ್ರತ್ಯಾಗಮ val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ಸಂರಚನಾ stat b1, b2;

		ಯದಿ (stat(opnd1, &b1) || stat(opnd2, &b2))
			ಪ್ರತ್ಯಾಗಮ 0; /* false, since at least one stat failed */
		ಯದಿ (op->op_ಅಂಕೆ == FILNT)
			ಪ್ರತ್ಯಾಗಮ b1.st_mtime > b2.st_mtime;
		ಯದಿ (op->op_ಅಂಕೆ == FILOT)
			ಪ್ರತ್ಯಾಗಮ b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ಪ್ರತ್ಯಾಗಮ b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ಸ್ಥಿರ ಶೂನ್ಯ initialize_group_array(ಶೂನ್ಯ)
{
	group_array = bb_getgroups(&ngroups, ರಿಕ್ತ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_a_group_member(gid_t gid)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	ಯದಿ (gid == getgid() || gid == getegid())
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	ಪ್ರತಿ (i = 0; i < ngroups; i++)
		ಯದಿ (gid == group_array[i])
			ಪ್ರತ್ಯಾಗಮ 1;

	ಪ್ರತ್ಯಾಗಮ 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_eaccess(ಸಂರಚನಾ stat *st, ಪೂರ್ಣಾಙ್ಕ mode)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ euid = geteuid();

	ಯದಿ (euid == 0) {
		/* Root can read or write any file. */
		ಯದಿ (mode != X_OK)
			ಪ್ರತ್ಯಾಗಮ 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		ಯದಿ (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (st->st_uid == euid)  /* owner */
		mode <<= 6;
	ಅನ್ಯಥಾ ಯದಿ (is_a_group_member(st->st_gid))
		mode <<= 3;

	ಯದಿ (st->st_mode & mode)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ -1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filstat(ವರ್ಣ *nm, ಗಣನಾ token mode)
{
	ಸಂರಚನಾ stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	ಯದಿ (mode == FILSYM) {
#ifdef S_IFLNK
		ಯದಿ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			ಗಚ್ಛ filetype;
		}
#endif
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (stat(nm, &s) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (mode == FILEXIST)
		ಪ್ರತ್ಯಾಗಮ 1;
	ಯದಿ (is_file_access(mode)) {
		ಯದಿ (mode == FILRD)
			i = R_OK;
		ಯದಿ (mode == FILWR)
			i = W_OK;
		ಯದಿ (mode == FILEX)
			i = X_OK;
		ಪ್ರತ್ಯಾಗಮ test_eaccess(&s, i) == 0;
	}
	ಯದಿ (is_file_type(mode)) {
		ಯದಿ (mode == FILREG)
			i = S_IFREG;
		ಯದಿ (mode == FILDIR)
			i = S_IFDIR;
		ಯದಿ (mode == FILCDEV)
			i = S_IFCHR;
		ಯದಿ (mode == FILBDEV)
			i = S_IFBLK;
		ಯದಿ (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ 0;
#endif
		}
		ಯದಿ (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ 0;
#endif
		}
 filetype:
		ಪ್ರತ್ಯಾಗಮ ((s.st_mode & S_IFMT) == i);
	}
	ಯದಿ (is_file_bit(mode)) {
		ಯದಿ (mode == FILSUID)
			i = S_ISUID;
		ಯದಿ (mode == FILSGID)
			i = S_ISGID;
		ಯದಿ (mode == FILSTCK)
			i = S_ISVTX;
		ಪ್ರತ್ಯಾಗಮ ((s.st_mode & i) != 0);
	}
	ಯದಿ (mode == FILGZ)
		ಪ್ರತ್ಯಾಗಮ s.st_size > 0L;
	ಯದಿ (mode == FILUID)
		ಪ್ರತ್ಯಾಗಮ s.st_uid == geteuid();
	ಯದಿ (mode == FILGID)
		ಪ್ರತ್ಯಾಗಮ s.st_gid == getegid();
	ಪ್ರತ್ಯಾಗಮ 1; /* NOTREACHED */
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t nexpr(ಗಣನಾ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">nexpr(%s)\n", TOKSTR[n]);
	ಯದಿ (n == UNOT) {
		n = check_operator(*++args);
		ಯದಿ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_ಸಂದೇಶ("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			ಪ್ರತ್ಯಾಗಮ 1;
		}
		res = !nexpr(n);
		unnest_ಸಂದೇಶ("<nexpr:%lld\n", res);
		ಪ್ರತ್ಯಾಗಮ res;
	}
	res = primary(n);
	unnest_ಸಂದೇಶ("<nexpr:%lld\n", res);
	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t aexpr(ಗಣನಾ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_ಸಂದೇಶ("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	ಯದಿ (check_operator(*++args) == BAND) {
		dbg_ಸಂದೇಶ("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_ಸಂದೇಶ("<aexpr:%lld\n", res);
		ಪ್ರತ್ಯಾಗಮ res;
	}
	args--;
	unnest_ಸಂದೇಶ("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t oexpr(ಗಣನಾ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_ಸಂದೇಶ("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	ಯದಿ (check_operator(*++args) == BOR) {
		dbg_ಸಂದೇಶ("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_ಸಂದೇಶ("<oexpr:%lld\n", res);
		ಪ್ರತ್ಯಾಗಮ res;
	}
	args--;
	unnest_ಸಂದೇಶ("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t primary(ಗಣನಾ token n)
{
#ಯದಿ TEST_DEBUG
	ಸಂಖ್ಯೆ_t res = res; /* for compiler */
#ಅನ್ಯಥಾ
	ಸಂಖ್ಯೆ_t res;
#endif
	ಧ್ರುವ ಸಂರಚನಾ operator_t *args0_op;

	nest_ಸಂದೇಶ(">primary(%s)\n", TOKSTR[n]);
	ಯದಿ (n == EOI) {
		syntax(ರಿಕ್ತ, "argument expected");
	}
	ಯದಿ (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		ಯದಿ (check_operator(*++args) != RPAREN)
			syntax(ರಿಕ್ತ, "closing paren expected");
		unnest_ಸಂದೇಶ("<primary:%lld\n", res);
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	ಯದಿ (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		ಯದಿ (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			ಯದಿ (last_operator->op_type == BINOP)
				unnest_ಸಂದೇಶ_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	ಯದಿ (args0_op->op_type == UNOP) {
		/* unary expression */
		ಯದಿ (args[1] == ರಿಕ್ತ)
//			syntax(args0_op->op_text, "argument expected");
			ಗಚ್ಛ check_emptiness;
		args++;
		ಯದಿ (n == STREZ)
			unnest_ಸಂದೇಶ_and_return(args[0][0] == '\0', "<primary:%lld\n");
		ಯದಿ (n == STRNZ)
			unnest_ಸಂದೇಶ_and_return(args[0][0] != '\0', "<primary:%lld\n");
		ಯದಿ (n == FILTT)
			unnest_ಸಂದೇಶ_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_ಸಂದೇಶ_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	ಯದಿ (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_ಸಂದೇಶ_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_ಸಂದೇಶ_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


ಪೂರ್ಣಾಙ್ಕ test_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ res;
	ಧ್ರುವ ವರ್ಣ *arg0;
#ಯದಿ BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	ಯದಿ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		ಯದಿ (!arg0[1]) { /* "[" ? */
			ಯದಿ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_ಸಂದೇಶ("missing ]");
				ಪ್ರತ್ಯಾಗಮ 2;
			}
		} ಅನ್ಯಥಾ { /* assuming "[[" */
			ಯದಿ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_ಸಂದೇಶ("missing ]]");
				ಪ್ರತ್ಯಾಗಮ 2;
			}
#ಯದಿ BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = ರಿಕ್ತ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#ಯದಿ BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	ಯದಿ (res)
		ಗಚ್ಛ ret;

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
	args = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	ಯದಿ (1) {
		ಪೂರ್ಣಾಙ್ಕ negate = 0;
 again:
		ಯದಿ (!argv[0]) {
			/* "test" */
			res = 1;
			ಗಚ್ಛ ret_special;
		}
		ಯದಿ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			ಗಚ್ಛ ret_special;
		}
		ಯದಿ (argv[2]) {
			ಯದಿ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				ಯದಿ (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					ಗಚ್ಛ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				ಗಚ್ಛ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			ಯದಿ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				ಯದಿ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ರಿಕ್ತ;
					argv++;
				}
			}
		}
 check_negate:
		ಯದಿ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			ಗಚ್ಛ again;
		}
	}

	res = !oexpr(check_operator(*args));

	ಯದಿ (*args != ರಿಕ್ತ && *++args != ರಿಕ್ತ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_ಸಂದೇಶ("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	ಪ್ರತ್ಯಾಗಮ res;
}
