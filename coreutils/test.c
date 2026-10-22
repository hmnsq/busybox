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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <regex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fnmatch.h>

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TEST_DEBUG 0

#ਜੇ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#ਨਹੀਂਤਾਂ
# define BASH_TEST2 0
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ token {
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

#ਜੇ BASH_TEST2
	REGEX,
#ਜੇ_ਅੰਤ

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
	ਸੰਕਾਰਜ
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_int_op(a)      (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - INTEQ)) <= 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_str_op(a)      (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - STREZ)) <= 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_file_op(a)     (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - FILNT)) <= 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_file_access(a) (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - FILRD)) <= 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_file_type(a)   (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - FILREG)) <= 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_file_ਦੋਆਧਾਰੀ_ਅੰਕ(a)    (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((a) - FILSUID)) <= 2)

#ਜੇ TEST_DEBUG
ਪੂਰਨ ਡੂੰਘਾਈ;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nest_ਸੁਨੇਹਾ(...) ਕਰੋ { \
	ਡੂੰਘਾਈ++; \
	fprintf(stderr, "%*s", ਡੂੰਘਾਈ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unnest_ਸੁਨੇਹਾ(...) ਕਰੋ { \
	fprintf(stderr, "%*s", ਡੂੰਘਾਈ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	ਡੂੰਘਾਈ--; \
} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dbg_ਸੁਨੇਹਾ(...) ਕਰੋ { \
	fprintf(stderr, "%*s", ਡੂੰਘਾਈ*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unnest_ਸੁਨੇਹਾ_and_return(ਵਿਅੰਜਕ, ...) ਕਰੋ { \
	ਸੰਖਿਆ_t __res = (ਵਿਅੰਜਕ); \
	fprintf(stderr, "%*s", ਡੂੰਘਾਈ*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	ਡੂੰਘਾਈ--; \
	ਵਾਪਸ __res; \
} ਜਦੋਂ (0)
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ TOKSTR[] ALIGN_PTR = {
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
#ਜੇ BASH_TEST2
	"REGEX",
#ਜੇ_ਅੰਤ
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
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nest_ਸੁਨੇਹਾ(...)   ((ਖਾਲੀ)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unnest_ਸੁਨੇਹਾ(...) ((ਖਾਲੀ)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dbg_ਸੁਨੇਹਾ(...)    ((ਖਾਲੀ)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unnest_ਸੁਨੇਹਾ_and_return(ਵਿਅੰਜਕ, ...) ਵਾਪਸ ਵਿਅੰਜਕ
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ਬਣਤਰ operator_t {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ op_ਅੰਕ, op_type;
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ operator_t ops_ਸਾਰਣੀ[] ALIGN2 = {
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
#ਜੇ BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#ਜੇ_ਅੰਤ
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
#ਜੇ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#ਜੇ_ਅੰਤ
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ ops_texts[] ALIGN1 =
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
#ਜੇ BASH_TEST2
	"=~"  "\0"
#ਜੇ_ਅੰਤ
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
#ਜੇ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#ਜੇ_ਅੰਤ
	"("   "\0"
	")"   "\0"
;


#ਜੇ ENABLE_FEATURE_TEST_64
ਕਿਸਮਪਰਿਭਾਸ਼ਾ int64_t ਸੰਖਿਆ_t;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ ਸੰਖਿਆ_t;
#ਜੇ_ਅੰਤ


/* We try to minimize both static and stack usage. */
ਬਣਤਰ test_statics {
	ਅੱਖਰ **ਦਲੀਲਾਂ;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ਅਚਲ ਬਣਤਰ operator_t *ਆਖਰੀ_operator;
	gid_t *ਸਮੂਹ_array;
	ਪੂਰਨ ngroups;
#ਜੇ BASH_TEST2
	bool bash_test2;
#ਜੇ_ਅੰਤ
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
ਬਾਹਰੀ ਬਣਤਰ test_statics *BB_GLOBAL_CONST test_ptr_ਮੰਜ਼ਿਲ_statics;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ S (*test_ptr_ਮੰਜ਼ਿਲ_statics)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦਲੀਲਾਂ            (S.ਦਲੀਲਾਂ         )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਆਖਰੀ_operator   (S.ਆਖਰੀ_operator)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮੂਹ_array     (S.ਸਮੂਹ_array  )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ngroups         (S.ngroups      )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bash_test2      (S.bash_test2   )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ leaving         (S.leaving      )

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_S() ਕਰੋ { \
	XZALLOC_CONST_PTR(&test_ptr_ਮੰਜ਼ਿਲ_statics, ਆਕਾਰ(S)); \
} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEINIT_S() ਕਰੋ { \
	ਮੁਕਤ(ਸਮੂਹ_array); \
	ਮੁਕਤ(test_ptr_ਮੰਜ਼ਿਲ_statics); \
} ਜਦੋਂ (0)

ਸਥਿਰ ਸੰਖਿਆ_t primary(ਗਿਣਤੀ token n);

ਸਥਿਰ ਖਾਲੀ syntax(ਅਚਲ ਅੱਖਰ *op, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) NORETURN;
ਸਥਿਰ ਖਾਲੀ syntax(ਅਚਲ ਅੱਖਰ *op, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	ਜੇ (op && *op) {
		bb_error_ਸੁਨੇਹਾ("%s: %s", op, ਸੁਨੇਹਾ);
	} ਨਹੀਂਤਾਂ {
		bb_error_ਸੁਨੇਹਾ("%s: %s"+4, ਸੁਨੇਹਾ);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ਸਥਿਰ ਸੰਖਿਆ_t getn(ਅਚਲ ਅੱਖਰ *s)
{
	ਅੱਖਰ *p;
#ਜੇ ENABLE_FEATURE_TEST_64
	ਲੰਮਾ ਲੰਮਾ r;
#ਨਹੀਂਤਾਂ
	ਲੰਮਾ r;
#ਜੇ_ਅੰਤ

	errno = 0;
#ਜੇ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#ਨਹੀਂਤਾਂ
	r = strtol(s, &p, 10);
#ਜੇ_ਅੰਤ

	ਜੇ (errno != 0)
		syntax(s, "out of range");

	ਜੇ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ਵਾਪਸ r;
}

/*ਅਣਵਰਤਿਆ
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


ਸਥਿਰ ਗਿਣਤੀ token check_operator(ਅਚਲ ਅੱਖਰ *s)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ operator_t no_op = {
		.op_ਅੰਕ = -1,
		.op_type = -1
	};
	ਪੂਰਨ n;

	ਆਖਰੀ_operator = &no_op;
	ਜੇ (s == ਨਿਰਾ)
		ਵਾਪਸ EOI;
	n = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(ops_texts, s);
	ਜੇ (n < 0)
		ਵਾਪਸ ਸੰਕਾਰਜ;

#ਜੇ BASH_TEST2
	ਜੇ (ops_ਸਾਰਣੀ[n].op_ਅੰਕ == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ਵਾਪਸ ਸੰਕਾਰਜ;
	}
	ਜੇ (ops_ਸਾਰਣੀ[n].op_ਅੰਕ == BAND || ops_ਸਾਰਣੀ[n].op_ਅੰਕ == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		ਜੇ (bash_test2 == (s[0] == '-'))
			ਵਾਪਸ ਸੰਕਾਰਜ;
	}
#ਜੇ_ਅੰਤ

	ਆਖਰੀ_operator = &ops_ਸਾਰਣੀ[n];
	ਵਾਪਸ ops_ਸਾਰਣੀ[n].op_ਅੰਕ;
}


ਸਥਿਰ ਪੂਰਨ binop(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *opnd1, *opnd2;
	ਅਚਲ ਬਣਤਰ operator_t *op;
	ਸੰਖਿਆ_t val1, val2;

	opnd1 = *ਦਲੀਲਾਂ;
	check_operator(*++ਦਲੀਲਾਂ);
	op = ਆਖਰੀ_operator;

	opnd2 = *++ਦਲੀਲਾਂ;
	ਜੇ (opnd2 == ਨਿਰਾ)
		syntax(ਦਲੀਲਾਂ[-1], "argument expected");

	ਜੇ (is_int_op(op->op_ਅੰਕ)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		ਜੇ (op->op_ਅੰਕ == INTEQ)
			ਵਾਪਸ val1 == val2;
		ਜੇ (op->op_ਅੰਕ == INTNE)
			ਵਾਪਸ val1 != val2;
		ਜੇ (op->op_ਅੰਕ == INTGE)
			ਵਾਪਸ val1 >= val2;
		ਜੇ (op->op_ਅੰਕ == INTGT)
			ਵਾਪਸ val1 >  val2;
		ਜੇ (op->op_ਅੰਕ == INTLE)
			ਵਾਪਸ val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ਵਾਪਸ val1 <  val2;
	}
#ਜੇ BASH_TEST2
	ਜੇ (bash_test2) {
		ਜੇ (op->op_ਅੰਕ == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ਵਾਪਸ val1 == 0;
		}
		ਜੇ (op->op_ਅੰਕ == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ਵਾਪਸ val1 != 0;
		}
		ਜੇ (op->op_ਅੰਕ == REGEX) {
			regex_t re_ਬਫਰ;
			memset(&re_ਬਫਰ, 0, ਆਕਾਰ(re_ਬਫਰ));
			ਜੇ (regcomp(&re_ਬਫਰ, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_ਬਫਰ, opnd1, 0, ਨਿਰਾ, 0);
			regfree(&re_ਬਫਰ);
			ਵਾਪਸ val1 == 0;
		}
	}
#ਜੇ_ਅੰਤ
	ਜੇ (is_str_op(op->op_ਅੰਕ)) {
		val1 = strcmp(opnd1, opnd2);
		ਜੇ (op->op_ਅੰਕ == STREQ)
			ਵਾਪਸ val1 == 0;
		ਜੇ (op->op_ਅੰਕ == STRNE)
			ਵਾਪਸ val1 != 0;
		ਜੇ (op->op_ਅੰਕ == STRLT)
			ਵਾਪਸ val1 < 0;
		/*if (op->op_num == STRGT)*/
		ਵਾਪਸ val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ਬਣਤਰ stat b1, b2;

		ਜੇ (stat(opnd1, &b1) || stat(opnd2, &b2))
			ਵਾਪਸ 0; /* false, since at least one stat failed */
		ਜੇ (op->op_ਅੰਕ == FILNT)
			ਵਾਪਸ b1.st_mtime > b2.st_mtime;
		ਜੇ (op->op_ਅੰਕ == FILOT)
			ਵਾਪਸ b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ਵਾਪਸ b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ਸਥਿਰ ਖਾਲੀ initialize_ਸਮੂਹ_array(ਖਾਲੀ)
{
	ਸਮੂਹ_array = bb_getgroups(&ngroups, ਨਿਰਾ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ਸਥਿਰ ਪੂਰਨ is_a_ਸਮੂਹ_member(gid_t gid)
{
	ਪੂਰਨ i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	ਜੇ (gid == getgid() || gid == getegid())
		ਵਾਪਸ 1;

	ਜੇ (ngroups == 0)
		initialize_ਸਮੂਹ_array();

	/* Search through the list looking for GID. */
	ਲਈ (i = 0; i < ngroups; i++)
		ਜੇ (gid == ਸਮੂਹ_array[i])
			ਵਾਪਸ 1;

	ਵਾਪਸ 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ਸਥਿਰ ਪੂਰਨ test_eaccess(ਬਣਤਰ stat *st, ਪੂਰਨ ਬਹੁਲਕ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ euid = geteuid();

	ਜੇ (euid == 0) {
		/* Root can read or write any file. */
		ਜੇ (ਬਹੁਲਕ != X_OK)
			ਵਾਪਸ 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		ਜੇ (st->st_ਬਹੁਲਕ & (S_IXUSR | S_IXGRP | S_IXOTH))
			ਵਾਪਸ 0;
	}

	ਜੇ (st->st_uid == euid)  /* owner */
		ਬਹੁਲਕ <<= 6;
	ਨਹੀਂਤਾਂ ਜੇ (is_a_ਸਮੂਹ_member(st->st_gid))
		ਬਹੁਲਕ <<= 3;

	ਜੇ (st->st_ਬਹੁਲਕ & ਬਹੁਲਕ)
		ਵਾਪਸ 0;

	ਵਾਪਸ -1;
}


ਸਥਿਰ ਪੂਰਨ filstat(ਅੱਖਰ *nm, ਗਿਣਤੀ token ਬਹੁਲਕ)
{
	ਬਣਤਰ stat s;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i = i; /* gcc 3.x thinks it can be used uninitialized */

	ਜੇ (ਬਹੁਲਕ == FILSYM) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_IFLNK
		ਜੇ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			ਜਾਓ filetype;
		}
#ਜੇ_ਅੰਤ
		ਵਾਪਸ 0;
	}

	ਜੇ (stat(nm, &s) != 0)
		ਵਾਪਸ 0;
	ਜੇ (ਬਹੁਲਕ == FILEXIST)
		ਵਾਪਸ 1;
	ਜੇ (is_file_access(ਬਹੁਲਕ)) {
		ਜੇ (ਬਹੁਲਕ == FILRD)
			i = R_OK;
		ਜੇ (ਬਹੁਲਕ == FILWR)
			i = W_OK;
		ਜੇ (ਬਹੁਲਕ == FILEX)
			i = X_OK;
		ਵਾਪਸ test_eaccess(&s, i) == 0;
	}
	ਜੇ (is_file_type(ਬਹੁਲਕ)) {
		ਜੇ (ਬਹੁਲਕ == FILREG)
			i = S_IFREG;
		ਜੇ (ਬਹੁਲਕ == FILDIR)
			i = S_IFDIR;
		ਜੇ (ਬਹੁਲਕ == FILCDEV)
			i = S_IFCHR;
		ਜੇ (ਬਹੁਲਕ == FILBDEV)
			i = S_IFBLK;
		ਜੇ (ਬਹੁਲਕ == FILFIFO) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_IFIFO
			i = S_IFIFO;
#ਨਹੀਂਤਾਂ
			ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
		}
		ਜੇ (ਬਹੁਲਕ == FILSOCK) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ S_IFSOCK
			i = S_IFSOCK;
#ਨਹੀਂਤਾਂ
			ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
		}
 filetype:
		ਵਾਪਸ ((s.st_ਬਹੁਲਕ & S_IFMT) == i);
	}
	ਜੇ (is_file_ਦੋਆਧਾਰੀ_ਅੰਕ(ਬਹੁਲਕ)) {
		ਜੇ (ਬਹੁਲਕ == FILSUID)
			i = S_ISUID;
		ਜੇ (ਬਹੁਲਕ == FILSGID)
			i = S_ISGID;
		ਜੇ (ਬਹੁਲਕ == FILSTCK)
			i = S_ISVTX;
		ਵਾਪਸ ((s.st_ਬਹੁਲਕ & i) != 0);
	}
	ਜੇ (ਬਹੁਲਕ == FILGZ)
		ਵਾਪਸ s.st_ਮਾਪ > 0L;
	ਜੇ (ਬਹੁਲਕ == FILUID)
		ਵਾਪਸ s.st_uid == geteuid();
	ਜੇ (ਬਹੁਲਕ == FILGID)
		ਵਾਪਸ s.st_gid == getegid();
	ਵਾਪਸ 1; /* NOTREACHED */
}


ਸਥਿਰ ਸੰਖਿਆ_t nexpr(ਗਿਣਤੀ token n)
{
	ਸੰਖਿਆ_t res;

	nest_ਸੁਨੇਹਾ(">nexpr(%s)\n", TOKSTR[n]);
	ਜੇ (n == UNOT) {
		n = check_operator(*++ਦਲੀਲਾਂ);
		ਜੇ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			ਦਲੀਲਾਂ--;
			unnest_ਸੁਨੇਹਾ("<nexpr:1 (!EOI), args:%s(%p)\n", ਦਲੀਲਾਂ[0], &ਦਲੀਲਾਂ[0]);
			ਵਾਪਸ 1;
		}
		res = !nexpr(n);
		unnest_ਸੁਨੇਹਾ("<nexpr:%lld\n", res);
		ਵਾਪਸ res;
	}
	res = primary(n);
	unnest_ਸੁਨੇਹਾ("<nexpr:%lld\n", res);
	ਵਾਪਸ res;
}


ਸਥਿਰ ਸੰਖਿਆ_t aexpr(ਗਿਣਤੀ token n)
{
	ਸੰਖਿਆ_t res;

	nest_ਸੁਨੇਹਾ(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_ਸੁਨੇਹਾ("aexpr: nexpr:%lld, next args:%s(%p)\n", res, ਦਲੀਲਾਂ[1], &ਦਲੀਲਾਂ[1]);
	ਜੇ (check_operator(*++ਦਲੀਲਾਂ) == BAND) {
		dbg_ਸੁਨੇਹਾ("aexpr: arg is AND, next args:%s(%p)\n", ਦਲੀਲਾਂ[1], &ਦਲੀਲਾਂ[1]);
		res = aexpr(check_operator(*++ਦਲੀਲਾਂ)) && res;
		unnest_ਸੁਨੇਹਾ("<aexpr:%lld\n", res);
		ਵਾਪਸ res;
	}
	ਦਲੀਲਾਂ--;
	unnest_ਸੁਨੇਹਾ("<aexpr:%lld, args:%s(%p)\n", res, ਦਲੀਲਾਂ[0], &ਦਲੀਲਾਂ[0]);
	ਵਾਪਸ res;
}


ਸਥਿਰ ਸੰਖਿਆ_t oexpr(ਗਿਣਤੀ token n)
{
	ਸੰਖਿਆ_t res;

	nest_ਸੁਨੇਹਾ(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_ਸੁਨੇਹਾ("oexpr: aexpr:%lld, next args:%s(%p)\n", res, ਦਲੀਲਾਂ[1], &ਦਲੀਲਾਂ[1]);
	ਜੇ (check_operator(*++ਦਲੀਲਾਂ) == BOR) {
		dbg_ਸੁਨੇਹਾ("oexpr: next arg is OR, next args:%s(%p)\n", ਦਲੀਲਾਂ[1], &ਦਲੀਲਾਂ[1]);
		res = oexpr(check_operator(*++ਦਲੀਲਾਂ)) || res;
		unnest_ਸੁਨੇਹਾ("<oexpr:%lld\n", res);
		ਵਾਪਸ res;
	}
	ਦਲੀਲਾਂ--;
	unnest_ਸੁਨੇਹਾ("<oexpr:%lld, args:%s(%p)\n", res, ਦਲੀਲਾਂ[0], &ਦਲੀਲਾਂ[0]);
	ਵਾਪਸ res;
}


ਸਥਿਰ ਸੰਖਿਆ_t primary(ਗਿਣਤੀ token n)
{
#ਜੇ TEST_DEBUG
	ਸੰਖਿਆ_t res = res; /* for compiler */
#ਨਹੀਂਤਾਂ
	ਸੰਖਿਆ_t res;
#ਜੇ_ਅੰਤ
	ਅਚਲ ਬਣਤਰ operator_t *args0_op;

	nest_ਸੁਨੇਹਾ(">primary(%s)\n", TOKSTR[n]);
	ਜੇ (n == EOI) {
		syntax(ਨਿਰਾ, "argument expected");
	}
	ਜੇ (n == LPAREN) {
		res = oexpr(check_operator(*++ਦਲੀਲਾਂ));
		ਜੇ (check_operator(*++ਦਲੀਲਾਂ) != RPAREN)
			syntax(ਨਿਰਾ, "closing paren expected");
		unnest_ਸੁਨੇਹਾ("<primary:%lld\n", res);
		ਵਾਪਸ res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = ਆਖਰੀ_operator;
	/* last_operator = operator at args[1] */
	ਜੇ (check_operator(ਦਲੀਲਾਂ[1]) != EOI) { /* if args[1] != NULL */
		ਜੇ (ਦਲੀਲਾਂ[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			ਜੇ (ਆਖਰੀ_operator->op_type == BINOP)
				unnest_ਸੁਨੇਹਾ_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	ਜੇ (args0_op->op_type == UNOP) {
		/* unary expression */
		ਜੇ (ਦਲੀਲਾਂ[1] == ਨਿਰਾ)
//			syntax(args0_op->op_text, "argument expected");
			ਜਾਓ check_emptiness;
		ਦਲੀਲਾਂ++;
		ਜੇ (n == STREZ)
			unnest_ਸੁਨੇਹਾ_and_return(ਦਲੀਲਾਂ[0][0] == '\0', "<primary:%lld\n");
		ਜੇ (n == STRNZ)
			unnest_ਸੁਨੇਹਾ_and_return(ਦਲੀਲਾਂ[0][0] != '\0', "<primary:%lld\n");
		ਜੇ (n == FILTT)
			unnest_ਸੁਨੇਹਾ_and_return(isatty(getn(*ਦਲੀਲਾਂ)), "<primary: isatty(%s)%lld\n", *ਦਲੀਲਾਂ);
		unnest_ਸੁਨੇਹਾ_and_return(filstat(*ਦਲੀਲਾਂ, n), "<primary: filstat(%s):%lld\n", *ਦਲੀਲਾਂ);
	}

	/*check_operator(args[1]); - already done */
	ਜੇ (ਆਖਰੀ_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_ਸੁਨੇਹਾ_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_ਸੁਨੇਹਾ_and_return(ਦਲੀਲਾਂ[0][0] != '\0', "<primary:%lld\n");
}


ਪੂਰਨ test_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ res;
	ਅਚਲ ਅੱਖਰ *arg0;
#ਜੇ BASH_TEST2
	bool bt2 = 0;
#ਜੇ_ਅੰਤ

	arg0 = bb_basename(argv[0]);
	ਜੇ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		ਜੇ (!arg0[1]) { /* "[" ? */
			ਜੇ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_ਸੁਨੇਹਾ("missing ]");
				ਵਾਪਸ 2;
			}
		} ਨਹੀਂਤਾਂ { /* assuming "[[" */
			ਜੇ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_ਸੁਨੇਹਾ("missing ]]");
				ਵਾਪਸ 2;
			}
#ਜੇ BASH_TEST2
			bt2 = 1;
#ਜੇ_ਅੰਤ
		}
		argv[argc] = ਨਿਰਾ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#ਜੇ BASH_TEST2
	bash_test2 = bt2;
#ਜੇ_ਅੰਤ

	res = setjmp(leaving);
	ਜੇ (res)
		ਜਾਓ ret;

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
	ਦਲੀਲਾਂ = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	ਜੇ (1) {
		ਪੂਰਨ negate = 0;
 again:
		ਜੇ (!argv[0]) {
			/* "test" */
			res = 1;
			ਜਾਓ ret_special;
		}
		ਜੇ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			ਜਾਓ ret_special;
		}
		ਜੇ (argv[2]) {
			ਜੇ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				ਜੇ (ਆਖਰੀ_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					ਦਲੀਲਾਂ = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					ਜਾਓ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				ਜਾਓ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			ਜੇ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				ਜੇ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ਨਿਰਾ;
					argv++;
				}
			}
		}
 check_negate:
		ਜੇ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			ਜਾਓ again;
		}
	}

	res = !oexpr(check_operator(*ਦਲੀਲਾਂ));

	ਜੇ (*ਦਲੀਲਾਂ != ਨਿਰਾ && *++ਦਲੀਲਾਂ != ਨਿਰਾ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_ਸੁਨੇਹਾ("%s: unknown operand", *ਦਲੀਲਾਂ);
		res = 2;
	}
 ret:
	DEINIT_S();
	ਵਾਪਸ res;
}
