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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ <regex.h>
#ଅନ୍ତର୍ଭାଵ <fnmatch.h>

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

#ଯଦି ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#ଅନ୍ୟଥା
# define BASH_TEST2 0
#endif

ଗଣନା token {
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

#ଯଦି BASH_TEST2
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
#define is_int_op(a)      (((unsigned ଅକ୍ଷର)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned ଅକ୍ଷର)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned ଅକ୍ଷର)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned ଅକ୍ଷର)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned ଅକ୍ଷର)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned ଅକ୍ଷର)((a) - FILSUID)) <= 2)

#ଯଦି TEST_DEBUG
ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth;
#define nest_ସନ୍ଦେଶ(...) କର { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ଯେତେବେଳେ (0)
#define unnest_ସନ୍ଦେଶ(...) କର { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} ଯେତେବେଳେ (0)
#define dbg_ସନ୍ଦେଶ(...) କର { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ଯେତେବେଳେ (0)
#define unnest_ସନ୍ଦେଶ_and_return(ପଦସମଷ୍ଟି, ...) କର { \
	ସଂଖ୍ୟା_t __res = (ପଦସମଷ୍ଟି); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	ଫେରନ୍ତୁ __res; \
} ଯେତେବେଳେ (0)
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ TOKSTR[] ALIGN_PTR = {
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
#ଯଦି BASH_TEST2
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
#ଅନ୍ୟଥା
#define nest_ସନ୍ଦେଶ(...)   ((ଶୂନ୍ୟ)0)
#define unnest_ସନ୍ଦେଶ(...) ((ଶୂନ୍ୟ)0)
#define dbg_ସନ୍ଦେଶ(...)    ((ଶୂନ୍ୟ)0)
#define unnest_ସନ୍ଦେଶ_and_return(ପଦସମଷ୍ଟି, ...) ଫେରନ୍ତୁ ପଦସମଷ୍ଟି
#endif

ଗଣନା {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ସଂରଚନା operator_t {
	unsigned ଅକ୍ଷର op_ଅଙ୍କ, op_type;
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା operator_t ops_table[] ALIGN2 = {
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
#ଯଦି BASH_TEST2
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
#ଯଦି BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର ops_texts[] ALIGN1 =
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
#ଯଦି BASH_TEST2
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
#ଯଦି BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#ଯଦି ENABLE_FEATURE_TEST_64
ପ୍ରକାରବ୍ୟାଖ୍ୟା int64_t ସଂଖ୍ୟା_t;
#ଅନ୍ୟଥା
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା ସଂଖ୍ୟା_t;
#endif


/* We try to minimize both static and stack usage. */
ସଂରଚନା test_statics {
	ଅକ୍ଷର **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ଅଚଳ ସଂରଚନା operator_t *last_operator;
	gid_t *group_array;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ngroups;
#ଯଦି BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
ବାହ୍ୟ ସଂରଚନା test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() କର { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, ଆକାର(S)); \
} ଯେତେବେଳେ (0)
#define DEINIT_S() କର { \
	ମୁକ୍ତ(group_array); \
	ମୁକ୍ତ(test_ptr_to_statics); \
} ଯେତେବେଳେ (0)

ସ୍ଥିର ସଂଖ୍ୟା_t primary(ଗଣନା token n);

ସ୍ଥିର ଶୂନ୍ୟ syntax(ଅଚଳ ଅକ୍ଷର *op, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ syntax(ଅଚଳ ଅକ୍ଷର *op, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	ଯଦି (op && *op) {
		bb_error_ସନ୍ଦେଶ("%s: %s", op, ସନ୍ଦେଶ);
	} ଅନ୍ୟଥା {
		bb_error_ସନ୍ଦେଶ("%s: %s"+4, ସନ୍ଦେଶ);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ସ୍ଥିର ସଂଖ୍ୟା_t getn(ଅଚଳ ଅକ୍ଷର *s)
{
	ଅକ୍ଷର *p;
#ଯଦି ENABLE_FEATURE_TEST_64
	long long r;
#ଅନ୍ୟଥା
	long r;
#endif

	errno = 0;
#ଯଦି ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#ଅନ୍ୟଥା
	r = strtol(s, &p, 10);
#endif

	ଯଦି (errno != 0)
		syntax(s, "out of range");

	ଯଦି (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ଫେରନ୍ତୁ r;
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


ସ୍ଥିର ଗଣନା token check_operator(ଅଚଳ ଅକ୍ଷର *s)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା operator_t no_op = {
		.op_ଅଙ୍କ = -1,
		.op_type = -1
	};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

	last_operator = &no_op;
	ଯଦି (s == ଖାଲି)
		ଫେରନ୍ତୁ EOI;
	n = index_in_strings(ops_texts, s);
	ଯଦି (n < 0)
		ଫେରନ୍ତୁ OPERAND;

#ଯଦି BASH_TEST2
	ଯଦି (ops_table[n].op_ଅଙ୍କ == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ଫେରନ୍ତୁ OPERAND;
	}
	ଯଦି (ops_table[n].op_ଅଙ୍କ == BAND || ops_table[n].op_ଅଙ୍କ == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		ଯଦି (bash_test2 == (s[0] == '-'))
			ଫେରନ୍ତୁ OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	ଫେରନ୍ତୁ ops_table[n].op_ଅଙ୍କ;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା binop(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *opnd1, *opnd2;
	ଅଚଳ ସଂରଚନା operator_t *op;
	ସଂଖ୍ୟା_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	ଯଦି (opnd2 == ଖାଲି)
		syntax(args[-1], "argument expected");

	ଯଦି (is_int_op(op->op_ଅଙ୍କ)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		ଯଦି (op->op_ଅଙ୍କ == INTEQ)
			ଫେରନ୍ତୁ val1 == val2;
		ଯଦି (op->op_ଅଙ୍କ == INTNE)
			ଫେରନ୍ତୁ val1 != val2;
		ଯଦି (op->op_ଅଙ୍କ == INTGE)
			ଫେରନ୍ତୁ val1 >= val2;
		ଯଦି (op->op_ଅଙ୍କ == INTGT)
			ଫେରନ୍ତୁ val1 >  val2;
		ଯଦି (op->op_ଅଙ୍କ == INTLE)
			ଫେରନ୍ତୁ val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ଫେରନ୍ତୁ val1 <  val2;
	}
#ଯଦି BASH_TEST2
	ଯଦି (bash_test2) {
		ଯଦି (op->op_ଅଙ୍କ == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ଫେରନ୍ତୁ val1 == 0;
		}
		ଯଦି (op->op_ଅଙ୍କ == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ଫେରନ୍ତୁ val1 != 0;
		}
		ଯଦି (op->op_ଅଙ୍କ == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, ଆକାର(re_buffer));
			ଯଦି (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, ଖାଲି, 0);
			regfree(&re_buffer);
			ଫେରନ୍ତୁ val1 == 0;
		}
	}
#endif
	ଯଦି (is_str_op(op->op_ଅଙ୍କ)) {
		val1 = strcmp(opnd1, opnd2);
		ଯଦି (op->op_ଅଙ୍କ == STREQ)
			ଫେରନ୍ତୁ val1 == 0;
		ଯଦି (op->op_ଅଙ୍କ == STRNE)
			ଫେରନ୍ତୁ val1 != 0;
		ଯଦି (op->op_ଅଙ୍କ == STRLT)
			ଫେରନ୍ତୁ val1 < 0;
		/*if (op->op_num == STRGT)*/
		ଫେରନ୍ତୁ val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ସଂରଚନା stat b1, b2;

		ଯଦି (stat(opnd1, &b1) || stat(opnd2, &b2))
			ଫେରନ୍ତୁ 0; /* false, since at least one stat failed */
		ଯଦି (op->op_ଅଙ୍କ == FILNT)
			ଫେରନ୍ତୁ b1.st_mtime > b2.st_mtime;
		ଯଦି (op->op_ଅଙ୍କ == FILOT)
			ଫେରନ୍ତୁ b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ଫେରନ୍ତୁ b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ସ୍ଥିର ଶୂନ୍ୟ initialize_group_array(ଶୂନ୍ୟ)
{
	group_array = bb_getgroups(&ngroups, ଖାଲି);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_a_group_member(gid_t gid)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	ଯଦି (gid == getgid() || gid == getegid())
		ଫେରନ୍ତୁ 1;

	ଯଦି (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	ପାଇଁ (i = 0; i < ngroups; i++)
		ଯଦି (gid == group_array[i])
			ଫେରନ୍ତୁ 1;

	ଫେରନ୍ତୁ 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_eaccess(ସଂରଚନା stat *st, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା euid = geteuid();

	ଯଦି (euid == 0) {
		/* Root can read or write any file. */
		ଯଦି (mode != X_OK)
			ଫେରନ୍ତୁ 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		ଯଦି (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			ଫେରନ୍ତୁ 0;
	}

	ଯଦି (st->st_uid == euid)  /* owner */
		mode <<= 6;
	ଅନ୍ୟଥା ଯଦି (is_a_group_member(st->st_gid))
		mode <<= 3;

	ଯଦି (st->st_mode & mode)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ -1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filstat(ଅକ୍ଷର *nm, ଗଣନା token mode)
{
	ସଂରଚନା stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	ଯଦି (mode == FILSYM) {
#ifdef S_IFLNK
		ଯଦି (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			ଯାଅ filetype;
		}
#endif
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (stat(nm, &s) != 0)
		ଫେରନ୍ତୁ 0;
	ଯଦି (mode == FILEXIST)
		ଫେରନ୍ତୁ 1;
	ଯଦି (is_file_access(mode)) {
		ଯଦି (mode == FILRD)
			i = R_OK;
		ଯଦି (mode == FILWR)
			i = W_OK;
		ଯଦି (mode == FILEX)
			i = X_OK;
		ଫେରନ୍ତୁ test_eaccess(&s, i) == 0;
	}
	ଯଦି (is_file_type(mode)) {
		ଯଦି (mode == FILREG)
			i = S_IFREG;
		ଯଦି (mode == FILDIR)
			i = S_IFDIR;
		ଯଦି (mode == FILCDEV)
			i = S_IFCHR;
		ଯଦି (mode == FILBDEV)
			i = S_IFBLK;
		ଯଦି (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#ଅନ୍ୟଥା
			ଫେରନ୍ତୁ 0;
#endif
		}
		ଯଦି (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#ଅନ୍ୟଥା
			ଫେରନ୍ତୁ 0;
#endif
		}
 filetype:
		ଫେରନ୍ତୁ ((s.st_mode & S_IFMT) == i);
	}
	ଯଦି (is_file_bit(mode)) {
		ଯଦି (mode == FILSUID)
			i = S_ISUID;
		ଯଦି (mode == FILSGID)
			i = S_ISGID;
		ଯଦି (mode == FILSTCK)
			i = S_ISVTX;
		ଫେରନ୍ତୁ ((s.st_mode & i) != 0);
	}
	ଯଦି (mode == FILGZ)
		ଫେରନ୍ତୁ s.st_size > 0L;
	ଯଦି (mode == FILUID)
		ଫେରନ୍ତୁ s.st_uid == geteuid();
	ଯଦି (mode == FILGID)
		ଫେରନ୍ତୁ s.st_gid == getegid();
	ଫେରନ୍ତୁ 1; /* NOTREACHED */
}


ସ୍ଥିର ସଂଖ୍ୟା_t nexpr(ଗଣନା token n)
{
	ସଂଖ୍ୟା_t res;

	nest_ସନ୍ଦେଶ(">nexpr(%s)\n", TOKSTR[n]);
	ଯଦି (n == UNOT) {
		n = check_operator(*++args);
		ଯଦି (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_ସନ୍ଦେଶ("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			ଫେରନ୍ତୁ 1;
		}
		res = !nexpr(n);
		unnest_ସନ୍ଦେଶ("<nexpr:%lld\n", res);
		ଫେରନ୍ତୁ res;
	}
	res = primary(n);
	unnest_ସନ୍ଦେଶ("<nexpr:%lld\n", res);
	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ସଂଖ୍ୟା_t aexpr(ଗଣନା token n)
{
	ସଂଖ୍ୟା_t res;

	nest_ସନ୍ଦେଶ(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_ସନ୍ଦେଶ("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	ଯଦି (check_operator(*++args) == BAND) {
		dbg_ସନ୍ଦେଶ("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_ସନ୍ଦେଶ("<aexpr:%lld\n", res);
		ଫେରନ୍ତୁ res;
	}
	args--;
	unnest_ସନ୍ଦେଶ("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ସଂଖ୍ୟା_t oexpr(ଗଣନା token n)
{
	ସଂଖ୍ୟା_t res;

	nest_ସନ୍ଦେଶ(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_ସନ୍ଦେଶ("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	ଯଦି (check_operator(*++args) == BOR) {
		dbg_ସନ୍ଦେଶ("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_ସନ୍ଦେଶ("<oexpr:%lld\n", res);
		ଫେରନ୍ତୁ res;
	}
	args--;
	unnest_ସନ୍ଦେଶ("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ସଂଖ୍ୟା_t primary(ଗଣନା token n)
{
#ଯଦି TEST_DEBUG
	ସଂଖ୍ୟା_t res = res; /* for compiler */
#ଅନ୍ୟଥା
	ସଂଖ୍ୟା_t res;
#endif
	ଅଚଳ ସଂରଚନା operator_t *args0_op;

	nest_ସନ୍ଦେଶ(">primary(%s)\n", TOKSTR[n]);
	ଯଦି (n == EOI) {
		syntax(ଖାଲି, "argument expected");
	}
	ଯଦି (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		ଯଦି (check_operator(*++args) != RPAREN)
			syntax(ଖାଲି, "closing paren expected");
		unnest_ସନ୍ଦେଶ("<primary:%lld\n", res);
		ଫେରନ୍ତୁ res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	ଯଦି (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		ଯଦି (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			ଯଦି (last_operator->op_type == BINOP)
				unnest_ସନ୍ଦେଶ_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	ଯଦି (args0_op->op_type == UNOP) {
		/* unary expression */
		ଯଦି (args[1] == ଖାଲି)
//			syntax(args0_op->op_text, "argument expected");
			ଯାଅ check_emptiness;
		args++;
		ଯଦି (n == STREZ)
			unnest_ସନ୍ଦେଶ_and_return(args[0][0] == '\0', "<primary:%lld\n");
		ଯଦି (n == STRNZ)
			unnest_ସନ୍ଦେଶ_and_return(args[0][0] != '\0', "<primary:%lld\n");
		ଯଦି (n == FILTT)
			unnest_ସନ୍ଦେଶ_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_ସନ୍ଦେଶ_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	ଯଦି (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_ସନ୍ଦେଶ_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_ସନ୍ଦେଶ_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	ଅଚଳ ଅକ୍ଷର *arg0;
#ଯଦି BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	ଯଦି ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		ଯଦି (!arg0[1]) { /* "[" ? */
			ଯଦି (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_ସନ୍ଦେଶ("missing ]");
				ଫେରନ୍ତୁ 2;
			}
		} ଅନ୍ୟଥା { /* assuming "[[" */
			ଯଦି (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_ସନ୍ଦେଶ("missing ]]");
				ଫେରନ୍ତୁ 2;
			}
#ଯଦି BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = ଖାଲି;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#ଯଦି BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	ଯଦି (res)
		ଯାଅ ret;

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
	ଯଦି (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା negate = 0;
 again:
		ଯଦି (!argv[0]) {
			/* "test" */
			res = 1;
			ଯାଅ ret_special;
		}
		ଯଦି (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			ଯାଅ ret_special;
		}
		ଯଦି (argv[2]) {
			ଯଦି (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				ଯଦି (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					ଯାଅ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				ଯାଅ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			ଯଦି (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				ଯଦି (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ଖାଲି;
					argv++;
				}
			}
		}
 check_negate:
		ଯଦି (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			ଯାଅ again;
		}
	}

	res = !oexpr(check_operator(*args));

	ଯଦି (*args != ଖାଲି && *++args != ଖାଲି) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_ସନ୍ଦେଶ("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	ଫେରନ୍ତୁ res;
}
