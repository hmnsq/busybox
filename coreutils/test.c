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

#include "libbb.h"
#include <regex.h>
#include <fnmatch.h>

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

#གལ་ཏེ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#དེ་མིན
# define BASH_TEST2 0
#endif

ga_nanaa token {
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

#གལ་ཏེ BASH_TEST2
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
#define is_int_op(a)      (((unsigned var_na)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned var_na)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned var_na)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned var_na)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned var_na)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned var_na)((a) - FILSUID)) <= 2)

#གལ་ཏེ TEST_DEBUG
ཧྲིལ depth;
#define nest_msg(...) do { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} སྐབས (0)
#define unnest_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} སྐབས (0)
#define dbg_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} སྐབས (0)
#define unnest_msg_and_return(expr, ...) do { \
	number_t __res = (expr); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	ཕྱིར __res; \
} སྐབས (0)
static const var_na *const TOKSTR[] ALIGN_PTR = {
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
#གལ་ཏེ BASH_TEST2
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
#དེ་མིན
#define nest_msg(...)   ((void)0)
#define unnest_msg(...) ((void)0)
#define dbg_msg(...)    ((void)0)
#define unnest_msg_and_return(expr, ...) ཕྱིར expr
#endif

ga_nanaa {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

sa_nracanaa operator_t {
	unsigned var_na op_num, op_type;
};

static const sa_nracanaa operator_t ops_table[] ALIGN2 = {
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
#གལ་ཏེ BASH_TEST2
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
#གལ་ཏེ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
static const var_na ops_texts[] ALIGN1 =
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
#གལ་ཏེ BASH_TEST2
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
#གལ་ཏེ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#གལ་ཏེ ENABLE_FEATURE_TEST_64
typedef int64_t number_t;
#དེ་མིན
typedef ཧྲིལ number_t;
#endif


/* We try to minimize both static and stack usage. */
sa_nracanaa test_statics {
	var_na **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	const sa_nracanaa operator_t *last_operator;
	gid_t *group_array;
	ཧྲིལ ngroups;
#གལ་ཏེ BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
extern sa_nracanaa test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() do { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, parimaa_na(S)); \
} སྐབས (0)
#define DEINIT_S() do { \
	free(group_array); \
	free(test_ptr_to_statics); \
} སྐབས (0)

static number_t primary(ga_nanaa token n);

static void syntax(const var_na *op, const var_na *msg) NORETURN;
static void syntax(const var_na *op, const var_na *msg)
{
	གལ་ཏེ (op && *op) {
		bb_error_msg("%s: %s", op, msg);
	} དེ་མིན {
		bb_error_msg("%s: %s"+4, msg);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
static number_t getn(const var_na *s)
{
	var_na *p;
#གལ་ཏེ ENABLE_FEATURE_TEST_64
	long long r;
#དེ་མིན
	long r;
#endif

	errno = 0;
#གལ་ཏེ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#དེ་མིན
	r = strtol(s, &p, 10);
#endif

	གལ་ཏེ (errno != 0)
		syntax(s, "out of range");

	གལ་ཏེ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ཕྱིར r;
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


static ga_nanaa token check_operator(const var_na *s)
{
	static const sa_nracanaa operator_t no_op = {
		.op_num = -1,
		.op_type = -1
	};
	ཧྲིལ n;

	last_operator = &no_op;
	གལ་ཏེ (s == NULL)
		ཕྱིར EOI;
	n = index_in_strings(ops_texts, s);
	གལ་ཏེ (n < 0)
		ཕྱིར OPERAND;

#གལ་ཏེ BASH_TEST2
	གལ་ཏེ (ops_table[n].op_num == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ཕྱིར OPERAND;
	}
	གལ་ཏེ (ops_table[n].op_num == BAND || ops_table[n].op_num == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		གལ་ཏེ (bash_test2 == (s[0] == '-'))
			ཕྱིར OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	ཕྱིར ops_table[n].op_num;
}


static ཧྲིལ binop(void)
{
	const var_na *opnd1, *opnd2;
	const sa_nracanaa operator_t *op;
	number_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	གལ་ཏེ (opnd2 == NULL)
		syntax(args[-1], "argument expected");

	གལ་ཏེ (is_int_op(op->op_num)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		གལ་ཏེ (op->op_num == INTEQ)
			ཕྱིར val1 == val2;
		གལ་ཏེ (op->op_num == INTNE)
			ཕྱིར val1 != val2;
		གལ་ཏེ (op->op_num == INTGE)
			ཕྱིར val1 >= val2;
		གལ་ཏེ (op->op_num == INTGT)
			ཕྱིར val1 >  val2;
		གལ་ཏེ (op->op_num == INTLE)
			ཕྱིར val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ཕྱིར val1 <  val2;
	}
#གལ་ཏེ BASH_TEST2
	གལ་ཏེ (bash_test2) {
		གལ་ཏེ (op->op_num == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ཕྱིར val1 == 0;
		}
		གལ་ཏེ (op->op_num == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ཕྱིར val1 != 0;
		}
		གལ་ཏེ (op->op_num == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, parimaa_na(re_buffer));
			གལ་ཏེ (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, NULL, 0);
			regfree(&re_buffer);
			ཕྱིར val1 == 0;
		}
	}
#endif
	གལ་ཏེ (is_str_op(op->op_num)) {
		val1 = strcmp(opnd1, opnd2);
		གལ་ཏེ (op->op_num == STREQ)
			ཕྱིར val1 == 0;
		གལ་ཏེ (op->op_num == STRNE)
			ཕྱིར val1 != 0;
		གལ་ཏེ (op->op_num == STRLT)
			ཕྱིར val1 < 0;
		/*if (op->op_num == STRGT)*/
		ཕྱིར val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		sa_nracanaa stat b1, b2;

		གལ་ཏེ (stat(opnd1, &b1) || stat(opnd2, &b2))
			ཕྱིར 0; /* false, since at least one stat failed */
		གལ་ཏེ (op->op_num == FILNT)
			ཕྱིར b1.st_mtime > b2.st_mtime;
		གལ་ཏེ (op->op_num == FILOT)
			ཕྱིར b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ཕྱིར b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

static void initialize_group_array(void)
{
	group_array = bb_getgroups(&ngroups, NULL);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
static ཧྲིལ is_a_group_member(gid_t gid)
{
	ཧྲིལ i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	གལ་ཏེ (gid == getgid() || gid == getegid())
		ཕྱིར 1;

	གལ་ཏེ (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	ཆེད་དུ (i = 0; i < ngroups; i++)
		གལ་ཏེ (gid == group_array[i])
			ཕྱིར 1;

	ཕྱིར 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
static ཧྲིལ test_eaccess(sa_nracanaa stat *st, ཧྲིལ mode)
{
	unsigned ཧྲིལ euid = geteuid();

	གལ་ཏེ (euid == 0) {
		/* Root can read or write any file. */
		གལ་ཏེ (mode != X_OK)
			ཕྱིར 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		གལ་ཏེ (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			ཕྱིར 0;
	}

	གལ་ཏེ (st->st_uid == euid)  /* owner */
		mode <<= 6;
	དེ་མིན གལ་ཏེ (is_a_group_member(st->st_gid))
		mode <<= 3;

	གལ་ཏེ (st->st_mode & mode)
		ཕྱིར 0;

	ཕྱིར -1;
}


static ཧྲིལ filstat(var_na *nm, ga_nanaa token mode)
{
	sa_nracanaa stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	གལ་ཏེ (mode == FILSYM) {
#ifdef S_IFLNK
		གལ་ཏེ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			goto filetype;
		}
#endif
		ཕྱིར 0;
	}

	གལ་ཏེ (stat(nm, &s) != 0)
		ཕྱིར 0;
	གལ་ཏེ (mode == FILEXIST)
		ཕྱིར 1;
	གལ་ཏེ (is_file_access(mode)) {
		གལ་ཏེ (mode == FILRD)
			i = R_OK;
		གལ་ཏེ (mode == FILWR)
			i = W_OK;
		གལ་ཏེ (mode == FILEX)
			i = X_OK;
		ཕྱིར test_eaccess(&s, i) == 0;
	}
	གལ་ཏེ (is_file_type(mode)) {
		གལ་ཏེ (mode == FILREG)
			i = S_IFREG;
		གལ་ཏེ (mode == FILDIR)
			i = S_IFDIR;
		གལ་ཏེ (mode == FILCDEV)
			i = S_IFCHR;
		གལ་ཏེ (mode == FILBDEV)
			i = S_IFBLK;
		གལ་ཏེ (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#དེ་མིན
			ཕྱིར 0;
#endif
		}
		གལ་ཏེ (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#དེ་མིན
			ཕྱིར 0;
#endif
		}
 filetype:
		ཕྱིར ((s.st_mode & S_IFMT) == i);
	}
	གལ་ཏེ (is_file_bit(mode)) {
		གལ་ཏེ (mode == FILSUID)
			i = S_ISUID;
		གལ་ཏེ (mode == FILSGID)
			i = S_ISGID;
		གལ་ཏེ (mode == FILSTCK)
			i = S_ISVTX;
		ཕྱིར ((s.st_mode & i) != 0);
	}
	གལ་ཏེ (mode == FILGZ)
		ཕྱིར s.st_size > 0L;
	གལ་ཏེ (mode == FILUID)
		ཕྱིར s.st_uid == geteuid();
	གལ་ཏེ (mode == FILGID)
		ཕྱིར s.st_gid == getegid();
	ཕྱིར 1; /* NOTREACHED */
}


static number_t nexpr(ga_nanaa token n)
{
	number_t res;

	nest_msg(">nexpr(%s)\n", TOKSTR[n]);
	གལ་ཏེ (n == UNOT) {
		n = check_operator(*++args);
		གལ་ཏེ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_msg("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			ཕྱིར 1;
		}
		res = !nexpr(n);
		unnest_msg("<nexpr:%lld\n", res);
		ཕྱིར res;
	}
	res = primary(n);
	unnest_msg("<nexpr:%lld\n", res);
	ཕྱིར res;
}


static number_t aexpr(ga_nanaa token n)
{
	number_t res;

	nest_msg(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_msg("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	གལ་ཏེ (check_operator(*++args) == BAND) {
		dbg_msg("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_msg("<aexpr:%lld\n", res);
		ཕྱིར res;
	}
	args--;
	unnest_msg("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ཕྱིར res;
}


static number_t oexpr(ga_nanaa token n)
{
	number_t res;

	nest_msg(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_msg("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	གལ་ཏེ (check_operator(*++args) == BOR) {
		dbg_msg("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_msg("<oexpr:%lld\n", res);
		ཕྱིར res;
	}
	args--;
	unnest_msg("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ཕྱིར res;
}


static number_t primary(ga_nanaa token n)
{
#གལ་ཏེ TEST_DEBUG
	number_t res = res; /* for compiler */
#དེ་མིན
	number_t res;
#endif
	const sa_nracanaa operator_t *args0_op;

	nest_msg(">primary(%s)\n", TOKSTR[n]);
	གལ་ཏེ (n == EOI) {
		syntax(NULL, "argument expected");
	}
	གལ་ཏེ (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		གལ་ཏེ (check_operator(*++args) != RPAREN)
			syntax(NULL, "closing paren expected");
		unnest_msg("<primary:%lld\n", res);
		ཕྱིར res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	གལ་ཏེ (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		གལ་ཏེ (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			གལ་ཏེ (last_operator->op_type == BINOP)
				unnest_msg_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	གལ་ཏེ (args0_op->op_type == UNOP) {
		/* unary expression */
		གལ་ཏེ (args[1] == NULL)
//			syntax(args0_op->op_text, "argument expected");
			goto check_emptiness;
		args++;
		གལ་ཏེ (n == STREZ)
			unnest_msg_and_return(args[0][0] == '\0', "<primary:%lld\n");
		གལ་ཏེ (n == STRNZ)
			unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
		གལ་ཏེ (n == FILTT)
			unnest_msg_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_msg_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	གལ་ཏེ (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_msg_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


ཧྲིལ test_main(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ res;
	const var_na *arg0;
#གལ་ཏེ BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	གལ་ཏེ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		གལ་ཏེ (!arg0[1]) { /* "[" ? */
			གལ་ཏེ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_msg("missing ]");
				ཕྱིར 2;
			}
		} དེ་མིན { /* assuming "[[" */
			གལ་ཏེ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_msg("missing ]]");
				ཕྱིར 2;
			}
#གལ་ཏེ BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = NULL;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#གལ་ཏེ BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	གལ་ཏེ (res)
		goto ret;

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
	གལ་ཏེ (1) {
		ཧྲིལ negate = 0;
 again:
		གལ་ཏེ (!argv[0]) {
			/* "test" */
			res = 1;
			goto ret_special;
		}
		གལ་ཏེ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			goto ret_special;
		}
		གལ་ཏེ (argv[2]) {
			གལ་ཏེ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				གལ་ཏེ (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					goto ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				goto check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			གལ་ཏེ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				གལ་ཏེ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = NULL;
					argv++;
				}
			}
		}
 check_negate:
		གལ་ཏེ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			goto again;
		}
	}

	res = !oexpr(check_operator(*args));

	གལ་ཏེ (*args != NULL && *++args != NULL) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_msg("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	ཕྱིར res;
}
