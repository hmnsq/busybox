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

#ವ್ಯಾಖ್ಯಾನಿಸು TEST_DEBUG 0

#ಆದರೆ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#ಇಲ್ಲದಿದ್ದರೆ
# define BASH_TEST2 0
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ token {
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

#ಆದರೆ BASH_TEST2
	REGEX,
#ಆದರೆ_ಅಂತ್ಯ

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
	ಕಾರ್ಯಾಂಶ
};
#ವ್ಯಾಖ್ಯಾನಿಸು is_int_op(a)      (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - INTEQ)) <= 5)
#ವ್ಯಾಖ್ಯಾನಿಸು is_str_op(a)      (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - STREZ)) <= 5)
#ವ್ಯಾಖ್ಯಾನಿಸು is_file_op(a)     (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - FILNT)) <= 2)
#ವ್ಯಾಖ್ಯಾನಿಸು is_file_access(a) (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - FILRD)) <= 2)
#ವ್ಯಾಖ್ಯಾನಿಸು is_file_type(a)   (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - FILREG)) <= 5)
#ವ್ಯಾಖ್ಯಾನಿಸು is_file_ಬಿಟ್(a)    (((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((a) - FILSUID)) <= 2)

#ಆದರೆ TEST_DEBUG
ಪೂರ್ಣಾಂಕ ಆಳ;
#ವ್ಯಾಖ್ಯಾನಿಸು nest_ಸಂದೇಶ(...) ಮಾಡು { \
	ಆಳ++; \
	fprintf(stderr, "%*s", ಆಳ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು unnest_ಸಂದೇಶ(...) ಮಾಡು { \
	fprintf(stderr, "%*s", ಆಳ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	ಆಳ--; \
} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು dbg_ಸಂದೇಶ(...) ಮಾಡು { \
	fprintf(stderr, "%*s", ಆಳ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು unnest_ಸಂದೇಶ_and_return(ಅಭಿವ್ಯಕ್ತಿ, ...) ಮಾಡು { \
	ಸಂಖ್ಯೆ_t __res = (ಅಭಿವ್ಯಕ್ತಿ); \
	fprintf(stderr, "%*s", ಆಳ*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	ಆಳ--; \
	ಹಿಂತಿರುಗಿ __res; \
} ಆಗ (0)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ TOKSTR[] ALIGN_PTR = {
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
#ಆದರೆ BASH_TEST2
	"REGEX",
#ಆದರೆ_ಅಂತ್ಯ
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
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು nest_ಸಂದೇಶ(...)   ((ಶೂನ್ಯ)0)
#ವ್ಯಾಖ್ಯಾನಿಸು unnest_ಸಂದೇಶ(...) ((ಶೂನ್ಯ)0)
#ವ್ಯಾಖ್ಯಾನಿಸು dbg_ಸಂದೇಶ(...)    ((ಶೂನ್ಯ)0)
#ವ್ಯಾಖ್ಯಾನಿಸು unnest_ಸಂದೇಶ_and_return(ಅಭಿವ್ಯಕ್ತಿ, ...) ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ರಚನೆ operator_t {
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ op_ಅಂಕೆ, op_type;
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ operator_t ops_ಕೋಷ್ಟಕ[] ALIGN2 = {
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
#ಆದರೆ BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#ಆದರೆ_ಅಂತ್ಯ
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
#ಆದರೆ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#ಆದರೆ_ಅಂತ್ಯ
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ops_texts[] ALIGN1 =
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
#ಆದರೆ BASH_TEST2
	"=~"  "\0"
#ಆದರೆ_ಅಂತ್ಯ
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
#ಆದರೆ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#ಆದರೆ_ಅಂತ್ಯ
	"("   "\0"
	")"   "\0"
;


#ಆದರೆ ENABLE_FEATURE_TEST_64
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ int64_t ಸಂಖ್ಯೆ_t;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ ಸಂಖ್ಯೆ_t;
#ಆದರೆ_ಅಂತ್ಯ


/* We try to minimize both static and stack usage. */
ರಚನೆ test_statics {
	ಅಕ್ಷರ **ವಾದಗಳು;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ಸ್ಥಿರಾಂಕ ರಚನೆ operator_t *ಕೊನೆಯ_operator;
	gid_t *ಗುಂಪು_array;
	ಪೂರ್ಣಾಂಕ ngroups;
#ಆದರೆ BASH_TEST2
	bool bash_test2;
#ಆದರೆ_ಅಂತ್ಯ
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
ಬಾಹ್ಯ ರಚನೆ test_statics *BB_GLOBAL_CONST test_ptr_ಗಮ್ಯ_statics;

#ವ್ಯಾಖ್ಯಾನಿಸು S (*test_ptr_ಗಮ್ಯ_statics)
#ವ್ಯಾಖ್ಯಾನಿಸು ವಾದಗಳು            (S.ವಾದಗಳು         )
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೊನೆಯ_operator   (S.ಕೊನೆಯ_operator)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗುಂಪು_array     (S.ಗುಂಪು_array  )
#ವ್ಯಾಖ್ಯಾನಿಸು ngroups         (S.ngroups      )
#ವ್ಯಾಖ್ಯಾನಿಸು bash_test2      (S.bash_test2   )
#ವ್ಯಾಖ್ಯಾನಿಸು leaving         (S.leaving      )

#ವ್ಯಾಖ್ಯಾನಿಸು INIT_S() ಮಾಡು { \
	XZALLOC_CONST_PTR(&test_ptr_ಗಮ್ಯ_statics, ಗಾತ್ರ(S)); \
} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು DEINIT_S() ಮಾಡು { \
	ಬಿಡುಗಡೆ(ಗುಂಪು_array); \
	ಬಿಡುಗಡೆ(test_ptr_ಗಮ್ಯ_statics); \
} ಆಗ (0)

ಸ್ಥಿರ ಸಂಖ್ಯೆ_t primary(ಎಣಿಕೆ token n);

ಸ್ಥಿರ ಶೂನ್ಯ syntax(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *op, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ syntax(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *op, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	ಆದರೆ (op && *op) {
		bb_error_ಸಂದೇಶ("%s: %s", op, ಸಂದೇಶ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		bb_error_ಸಂದೇಶ("%s: %s"+4, ಸಂದೇಶ);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ಸ್ಥಿರ ಸಂಖ್ಯೆ_t getn(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಅಕ್ಷರ *p;
#ಆದರೆ ENABLE_FEATURE_TEST_64
	ದೀರ್ಘ ದೀರ್ಘ r;
#ಇಲ್ಲದಿದ್ದರೆ
	ದೀರ್ಘ r;
#ಆದರೆ_ಅಂತ್ಯ

	errno = 0;
#ಆದರೆ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#ಇಲ್ಲದಿದ್ದರೆ
	r = strtol(s, &p, 10);
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (errno != 0)
		syntax(s, "out of range");

	ಆದರೆ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ಹಿಂತಿರುಗಿ r;
}

/*ಬಳಸದ
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


ಸ್ಥಿರ ಎಣಿಕೆ token check_operator(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ operator_t no_op = {
		.op_ಅಂಕೆ = -1,
		.op_type = -1
	};
	ಪೂರ್ಣಾಂಕ n;

	ಕೊನೆಯ_operator = &no_op;
	ಆದರೆ (s == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ EOI;
	n = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(ops_texts, s);
	ಆದರೆ (n < 0)
		ಹಿಂತಿರುಗಿ ಕಾರ್ಯಾಂಶ;

#ಆದರೆ BASH_TEST2
	ಆದರೆ (ops_ಕೋಷ್ಟಕ[n].op_ಅಂಕೆ == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ಹಿಂತಿರುಗಿ ಕಾರ್ಯಾಂಶ;
	}
	ಆದರೆ (ops_ಕೋಷ್ಟಕ[n].op_ಅಂಕೆ == BAND || ops_ಕೋಷ್ಟಕ[n].op_ಅಂಕೆ == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		ಆದರೆ (bash_test2 == (s[0] == '-'))
			ಹಿಂತಿರುಗಿ ಕಾರ್ಯಾಂಶ;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಕೊನೆಯ_operator = &ops_ಕೋಷ್ಟಕ[n];
	ಹಿಂತಿರುಗಿ ops_ಕೋಷ್ಟಕ[n].op_ಅಂಕೆ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ binop(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opnd1, *opnd2;
	ಸ್ಥಿರಾಂಕ ರಚನೆ operator_t *op;
	ಸಂಖ್ಯೆ_t val1, val2;

	opnd1 = *ವಾದಗಳು;
	check_operator(*++ವಾದಗಳು);
	op = ಕೊನೆಯ_operator;

	opnd2 = *++ವಾದಗಳು;
	ಆದರೆ (opnd2 == ಖಾಲಿ)
		syntax(ವಾದಗಳು[-1], "argument expected");

	ಆದರೆ (is_int_op(op->op_ಅಂಕೆ)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		ಆದರೆ (op->op_ಅಂಕೆ == INTEQ)
			ಹಿಂತಿರುಗಿ val1 == val2;
		ಆದರೆ (op->op_ಅಂಕೆ == INTNE)
			ಹಿಂತಿರುಗಿ val1 != val2;
		ಆದರೆ (op->op_ಅಂಕೆ == INTGE)
			ಹಿಂತಿರುಗಿ val1 >= val2;
		ಆದರೆ (op->op_ಅಂಕೆ == INTGT)
			ಹಿಂತಿರುಗಿ val1 >  val2;
		ಆದರೆ (op->op_ಅಂಕೆ == INTLE)
			ಹಿಂತಿರುಗಿ val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ಹಿಂತಿರುಗಿ val1 <  val2;
	}
#ಆದರೆ BASH_TEST2
	ಆದರೆ (bash_test2) {
		ಆದರೆ (op->op_ಅಂಕೆ == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ಹಿಂತಿರುಗಿ val1 == 0;
		}
		ಆದರೆ (op->op_ಅಂಕೆ == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ಹಿಂತಿರುಗಿ val1 != 0;
		}
		ಆದರೆ (op->op_ಅಂಕೆ == REGEX) {
			regex_t re_ಬಫರ್;
			memset(&re_ಬಫರ್, 0, ಗಾತ್ರ(re_ಬಫರ್));
			ಆದರೆ (regcomp(&re_ಬಫರ್, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_ಬಫರ್, opnd1, 0, ಖಾಲಿ, 0);
			regfree(&re_ಬಫರ್);
			ಹಿಂತಿರುಗಿ val1 == 0;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (is_str_op(op->op_ಅಂಕೆ)) {
		val1 = strcmp(opnd1, opnd2);
		ಆದರೆ (op->op_ಅಂಕೆ == STREQ)
			ಹಿಂತಿರುಗಿ val1 == 0;
		ಆದರೆ (op->op_ಅಂಕೆ == STRNE)
			ಹಿಂತಿರುಗಿ val1 != 0;
		ಆದರೆ (op->op_ಅಂಕೆ == STRLT)
			ಹಿಂತಿರುಗಿ val1 < 0;
		/*if (op->op_num == STRGT)*/
		ಹಿಂತಿರುಗಿ val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ರಚನೆ stat b1, b2;

		ಆದರೆ (stat(opnd1, &b1) || stat(opnd2, &b2))
			ಹಿಂತಿರುಗಿ 0; /* false, since at least one stat failed */
		ಆದರೆ (op->op_ಅಂಕೆ == FILNT)
			ಹಿಂತಿರುಗಿ b1.st_mtime > b2.st_mtime;
		ಆದರೆ (op->op_ಅಂಕೆ == FILOT)
			ಹಿಂತಿರುಗಿ b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ಹಿಂತಿರುಗಿ b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ಸ್ಥಿರ ಶೂನ್ಯ initialize_ಗುಂಪು_array(ಶೂನ್ಯ)
{
	ಗುಂಪು_array = bb_getgroups(&ngroups, ಖಾಲಿ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_a_ಗುಂಪು_member(gid_t gid)
{
	ಪೂರ್ಣಾಂಕ i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	ಆದರೆ (gid == getgid() || gid == getegid())
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (ngroups == 0)
		initialize_ಗುಂಪು_array();

	/* Search through the list looking for GID. */
	ಗಾಗಿ (i = 0; i < ngroups; i++)
		ಆದರೆ (gid == ಗುಂಪು_array[i])
			ಹಿಂತಿರುಗಿ 1;

	ಹಿಂತಿರುಗಿ 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_eaccess(ರಚನೆ stat *st, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ euid = geteuid();

	ಆದರೆ (euid == 0) {
		/* Root can read or write any file. */
		ಆದರೆ (ಬಹುಲಕ != X_OK)
			ಹಿಂತಿರುಗಿ 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		ಆದರೆ (st->st_ಬಹುಲಕ & (S_IXUSR | S_IXGRP | S_IXOTH))
			ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (st->st_uid == euid)  /* owner */
		ಬಹುಲಕ <<= 6;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (is_a_ಗುಂಪು_member(st->st_gid))
		ಬಹುಲಕ <<= 3;

	ಆದರೆ (st->st_ಬಹುಲಕ & ಬಹುಲಕ)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ -1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filstat(ಅಕ್ಷರ *nm, ಎಣಿಕೆ token ಬಹುಲಕ)
{
	ರಚನೆ stat s;
	ಚಿಹ್ನೆರಹಿತ i = i; /* gcc 3.x thinks it can be used uninitialized */

	ಆದರೆ (ಬಹುಲಕ == FILSYM) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_IFLNK
		ಆದರೆ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			ಹೋಗು filetype;
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (stat(nm, &s) != 0)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (ಬಹುಲಕ == FILEXIST)
		ಹಿಂತಿರುಗಿ 1;
	ಆದರೆ (is_file_access(ಬಹುಲಕ)) {
		ಆದರೆ (ಬಹುಲಕ == FILRD)
			i = R_OK;
		ಆದರೆ (ಬಹುಲಕ == FILWR)
			i = W_OK;
		ಆದರೆ (ಬಹುಲಕ == FILEX)
			i = X_OK;
		ಹಿಂತಿರುಗಿ test_eaccess(&s, i) == 0;
	}
	ಆದರೆ (is_file_type(ಬಹುಲಕ)) {
		ಆದರೆ (ಬಹುಲಕ == FILREG)
			i = S_IFREG;
		ಆದರೆ (ಬಹುಲಕ == FILDIR)
			i = S_IFDIR;
		ಆದರೆ (ಬಹುಲಕ == FILCDEV)
			i = S_IFCHR;
		ಆದರೆ (ಬಹುಲಕ == FILBDEV)
			i = S_IFBLK;
		ಆದರೆ (ಬಹುಲಕ == FILFIFO) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_IFIFO
			i = S_IFIFO;
#ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
		}
		ಆದರೆ (ಬಹುಲಕ == FILSOCK) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S_IFSOCK
			i = S_IFSOCK;
#ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
		}
 filetype:
		ಹಿಂತಿರುಗಿ ((s.st_ಬಹುಲಕ & S_IFMT) == i);
	}
	ಆದರೆ (is_file_ಬಿಟ್(ಬಹುಲಕ)) {
		ಆದರೆ (ಬಹುಲಕ == FILSUID)
			i = S_ISUID;
		ಆದರೆ (ಬಹುಲಕ == FILSGID)
			i = S_ISGID;
		ಆದರೆ (ಬಹುಲಕ == FILSTCK)
			i = S_ISVTX;
		ಹಿಂತಿರುಗಿ ((s.st_ಬಹುಲಕ & i) != 0);
	}
	ಆದರೆ (ಬಹುಲಕ == FILGZ)
		ಹಿಂತಿರುಗಿ s.st_ಗಾತ್ರ > 0L;
	ಆದರೆ (ಬಹುಲಕ == FILUID)
		ಹಿಂತಿರುಗಿ s.st_uid == geteuid();
	ಆದರೆ (ಬಹುಲಕ == FILGID)
		ಹಿಂತಿರುಗಿ s.st_gid == getegid();
	ಹಿಂತಿರುಗಿ 1; /* NOTREACHED */
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t nexpr(ಎಣಿಕೆ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">nexpr(%s)\n", TOKSTR[n]);
	ಆದರೆ (n == UNOT) {
		n = check_operator(*++ವಾದಗಳು);
		ಆದರೆ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			ವಾದಗಳು--;
			unnest_ಸಂದೇಶ("<nexpr:1 (!EOI), args:%s(%p)\n", ವಾದಗಳು[0], &ವಾದಗಳು[0]);
			ಹಿಂತಿರುಗಿ 1;
		}
		res = !nexpr(n);
		unnest_ಸಂದೇಶ("<nexpr:%lld\n", res);
		ಹಿಂತಿರುಗಿ res;
	}
	res = primary(n);
	unnest_ಸಂದೇಶ("<nexpr:%lld\n", res);
	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t aexpr(ಎಣಿಕೆ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_ಸಂದೇಶ("aexpr: nexpr:%lld, next args:%s(%p)\n", res, ವಾದಗಳು[1], &ವಾದಗಳು[1]);
	ಆದರೆ (check_operator(*++ವಾದಗಳು) == BAND) {
		dbg_ಸಂದೇಶ("aexpr: arg is AND, next args:%s(%p)\n", ವಾದಗಳು[1], &ವಾದಗಳು[1]);
		res = aexpr(check_operator(*++ವಾದಗಳು)) && res;
		unnest_ಸಂದೇಶ("<aexpr:%lld\n", res);
		ಹಿಂತಿರುಗಿ res;
	}
	ವಾದಗಳು--;
	unnest_ಸಂದೇಶ("<aexpr:%lld, args:%s(%p)\n", res, ವಾದಗಳು[0], &ವಾದಗಳು[0]);
	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t oexpr(ಎಣಿಕೆ token n)
{
	ಸಂಖ್ಯೆ_t res;

	nest_ಸಂದೇಶ(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_ಸಂದೇಶ("oexpr: aexpr:%lld, next args:%s(%p)\n", res, ವಾದಗಳು[1], &ವಾದಗಳು[1]);
	ಆದರೆ (check_operator(*++ವಾದಗಳು) == BOR) {
		dbg_ಸಂದೇಶ("oexpr: next arg is OR, next args:%s(%p)\n", ವಾದಗಳು[1], &ವಾದಗಳು[1]);
		res = oexpr(check_operator(*++ವಾದಗಳು)) || res;
		unnest_ಸಂದೇಶ("<oexpr:%lld\n", res);
		ಹಿಂತಿರುಗಿ res;
	}
	ವಾದಗಳು--;
	unnest_ಸಂದೇಶ("<oexpr:%lld, args:%s(%p)\n", res, ವಾದಗಳು[0], &ವಾದಗಳು[0]);
	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಸಂಖ್ಯೆ_t primary(ಎಣಿಕೆ token n)
{
#ಆದರೆ TEST_DEBUG
	ಸಂಖ್ಯೆ_t res = res; /* for compiler */
#ಇಲ್ಲದಿದ್ದರೆ
	ಸಂಖ್ಯೆ_t res;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿರಾಂಕ ರಚನೆ operator_t *args0_op;

	nest_ಸಂದೇಶ(">primary(%s)\n", TOKSTR[n]);
	ಆದರೆ (n == EOI) {
		syntax(ಖಾಲಿ, "argument expected");
	}
	ಆದರೆ (n == LPAREN) {
		res = oexpr(check_operator(*++ವಾದಗಳು));
		ಆದರೆ (check_operator(*++ವಾದಗಳು) != RPAREN)
			syntax(ಖಾಲಿ, "closing paren expected");
		unnest_ಸಂದೇಶ("<primary:%lld\n", res);
		ಹಿಂತಿರುಗಿ res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = ಕೊನೆಯ_operator;
	/* last_operator = operator at args[1] */
	ಆದರೆ (check_operator(ವಾದಗಳು[1]) != EOI) { /* if args[1] != NULL */
		ಆದರೆ (ವಾದಗಳು[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			ಆದರೆ (ಕೊನೆಯ_operator->op_type == BINOP)
				unnest_ಸಂದೇಶ_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	ಆದರೆ (args0_op->op_type == UNOP) {
		/* unary expression */
		ಆದರೆ (ವಾದಗಳು[1] == ಖಾಲಿ)
//			syntax(args0_op->op_text, "argument expected");
			ಹೋಗು check_emptiness;
		ವಾದಗಳು++;
		ಆದರೆ (n == STREZ)
			unnest_ಸಂದೇಶ_and_return(ವಾದಗಳು[0][0] == '\0', "<primary:%lld\n");
		ಆದರೆ (n == STRNZ)
			unnest_ಸಂದೇಶ_and_return(ವಾದಗಳು[0][0] != '\0', "<primary:%lld\n");
		ಆದರೆ (n == FILTT)
			unnest_ಸಂದೇಶ_and_return(isatty(getn(*ವಾದಗಳು)), "<primary: isatty(%s)%lld\n", *ವಾದಗಳು);
		unnest_ಸಂದೇಶ_and_return(filstat(*ವಾದಗಳು, n), "<primary: filstat(%s):%lld\n", *ವಾದಗಳು);
	}

	/*check_operator(args[1]); - already done */
	ಆದರೆ (ಕೊನೆಯ_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_ಸಂದೇಶ_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_ಸಂದೇಶ_and_return(ವಾದಗಳು[0][0] != '\0', "<primary:%lld\n");
}


ಪೂರ್ಣಾಂಕ test_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ res;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg0;
#ಆದರೆ BASH_TEST2
	bool bt2 = 0;
#ಆದರೆ_ಅಂತ್ಯ

	arg0 = bb_basename(argv[0]);
	ಆದರೆ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		ಆದರೆ (!arg0[1]) { /* "[" ? */
			ಆದರೆ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_ಸಂದೇಶ("missing ]");
				ಹಿಂತಿರುಗಿ 2;
			}
		} ಇಲ್ಲದಿದ್ದರೆ { /* assuming "[[" */
			ಆದರೆ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_ಸಂದೇಶ("missing ]]");
				ಹಿಂತಿರುಗಿ 2;
			}
#ಆದರೆ BASH_TEST2
			bt2 = 1;
#ಆದರೆ_ಅಂತ್ಯ
		}
		argv[argc] = ಖಾಲಿ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#ಆದರೆ BASH_TEST2
	bash_test2 = bt2;
#ಆದರೆ_ಅಂತ್ಯ

	res = setjmp(leaving);
	ಆದರೆ (res)
		ಹೋಗು ret;

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
	ವಾದಗಳು = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	ಆದರೆ (1) {
		ಪೂರ್ಣಾಂಕ negate = 0;
 again:
		ಆದರೆ (!argv[0]) {
			/* "test" */
			res = 1;
			ಹೋಗು ret_special;
		}
		ಆದರೆ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			ಹೋಗು ret_special;
		}
		ಆದರೆ (argv[2]) {
			ಆದರೆ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				ಆದರೆ (ಕೊನೆಯ_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					ವಾದಗಳು = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					ಹೋಗು ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				ಹೋಗು check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			ಆದರೆ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				ಆದರೆ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ಖಾಲಿ;
					argv++;
				}
			}
		}
 check_negate:
		ಆದರೆ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			ಹೋಗು again;
		}
	}

	res = !oexpr(check_operator(*ವಾದಗಳು));

	ಆದರೆ (*ವಾದಗಳು != ಖಾಲಿ && *++ವಾದಗಳು != ಖಾಲಿ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_ಸಂದೇಶ("%s: unknown operand", *ವಾದಗಳು);
		res = 2;
	}
 ret:
	DEINIT_S();
	ಹಿಂತಿರುಗಿ res;
}
