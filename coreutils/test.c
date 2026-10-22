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

#якщо ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#інакше
# define BASH_TEST2 0
#endif

enum token {
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

#якщо BASH_TEST2
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
#define is_int_op(a)      (((unsigned char)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned char)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned char)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned char)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned char)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned char)((a) - FILSUID)) <= 2)

#якщо TEST_DEBUG
ціле depth;
#define nest_msg(...) do { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} поки (0)
#define unnest_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} поки (0)
#define dbg_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} поки (0)
#define unnest_msg_and_return(expr, ...) do { \
	number_t __res = (expr); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	повернути __res; \
} поки (0)
static const char *const TOKSTR[] ALIGN_PTR = {
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
#якщо BASH_TEST2
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
#інакше
#define nest_msg(...)   ((void)0)
#define unnest_msg(...) ((void)0)
#define dbg_msg(...)    ((void)0)
#define unnest_msg_and_return(expr, ...) повернути expr
#endif

enum {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

struct operator_t {
	unsigned char op_num, op_type;
};

static const struct operator_t ops_table[] ALIGN2 = {
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
#якщо BASH_TEST2
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
#якщо BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
static const char ops_texts[] ALIGN1 =
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
#якщо BASH_TEST2
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
#якщо BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#якщо ENABLE_FEATURE_TEST_64
typedef int64_t number_t;
#інакше
typedef ціле number_t;
#endif


/* We try to minimize both static and stack usage. */
struct test_statics {
	char **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	const struct operator_t *last_operator;
	gid_t *group_array;
	ціле ngroups;
#якщо BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
extern struct test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() do { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, sizeof(S)); \
} поки (0)
#define DEINIT_S() do { \
	free(group_array); \
	free(test_ptr_to_statics); \
} поки (0)

static number_t primary(enum token n);

static void syntax(const char *op, const char *msg) NORETURN;
static void syntax(const char *op, const char *msg)
{
	якщо (op && *op) {
		bb_error_msg("%s: %s", op, msg);
	} інакше {
		bb_error_msg("%s: %s"+4, msg);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
static number_t getn(const char *s)
{
	char *p;
#якщо ENABLE_FEATURE_TEST_64
	long long r;
#інакше
	long r;
#endif

	errno = 0;
#якщо ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#інакше
	r = strtol(s, &p, 10);
#endif

	якщо (errno != 0)
		syntax(s, "out of range");

	якщо (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	повернути r;
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


static enum token check_operator(const char *s)
{
	static const struct operator_t no_op = {
		.op_num = -1,
		.op_type = -1
	};
	ціле n;

	last_operator = &no_op;
	якщо (s == NULL)
		повернути EOI;
	n = index_in_strings(ops_texts, s);
	якщо (n < 0)
		повернути OPERAND;

#якщо BASH_TEST2
	якщо (ops_table[n].op_num == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		повернути OPERAND;
	}
	якщо (ops_table[n].op_num == BAND || ops_table[n].op_num == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		якщо (bash_test2 == (s[0] == '-'))
			повернути OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	повернути ops_table[n].op_num;
}


static ціле binop(void)
{
	const char *opnd1, *opnd2;
	const struct operator_t *op;
	number_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	якщо (opnd2 == NULL)
		syntax(args[-1], "argument expected");

	якщо (is_int_op(op->op_num)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		якщо (op->op_num == INTEQ)
			повернути val1 == val2;
		якщо (op->op_num == INTNE)
			повернути val1 != val2;
		якщо (op->op_num == INTGE)
			повернути val1 >= val2;
		якщо (op->op_num == INTGT)
			повернути val1 >  val2;
		якщо (op->op_num == INTLE)
			повернути val1 <= val2;
		/*if (op->op_num == INTLT)*/
		повернути val1 <  val2;
	}
#якщо BASH_TEST2
	якщо (bash_test2) {
		якщо (op->op_num == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			повернути val1 == 0;
		}
		якщо (op->op_num == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			повернути val1 != 0;
		}
		якщо (op->op_num == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, sizeof(re_buffer));
			якщо (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, NULL, 0);
			regfree(&re_buffer);
			повернути val1 == 0;
		}
	}
#endif
	якщо (is_str_op(op->op_num)) {
		val1 = strcmp(opnd1, opnd2);
		якщо (op->op_num == STREQ)
			повернути val1 == 0;
		якщо (op->op_num == STRNE)
			повернути val1 != 0;
		якщо (op->op_num == STRLT)
			повернути val1 < 0;
		/*if (op->op_num == STRGT)*/
		повернути val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		struct stat b1, b2;

		якщо (stat(opnd1, &b1) || stat(opnd2, &b2))
			повернути 0; /* false, since at least one stat failed */
		якщо (op->op_num == FILNT)
			повернути b1.st_mtime > b2.st_mtime;
		якщо (op->op_num == FILOT)
			повернути b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		повернути b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
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
static ціле is_a_group_member(gid_t gid)
{
	ціле i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	якщо (gid == getgid() || gid == getegid())
		повернути 1;

	якщо (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	для (i = 0; i < ngroups; i++)
		якщо (gid == group_array[i])
			повернути 1;

	повернути 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
static ціле test_eaccess(struct stat *st, ціле mode)
{
	unsigned ціле euid = geteuid();

	якщо (euid == 0) {
		/* Root can read or write any file. */
		якщо (mode != X_OK)
			повернути 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		якщо (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			повернути 0;
	}

	якщо (st->st_uid == euid)  /* owner */
		mode <<= 6;
	інакше якщо (is_a_group_member(st->st_gid))
		mode <<= 3;

	якщо (st->st_mode & mode)
		повернути 0;

	повернути -1;
}


static ціле filstat(char *nm, enum token mode)
{
	struct stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	якщо (mode == FILSYM) {
#ifdef S_IFLNK
		якщо (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			goto filetype;
		}
#endif
		повернути 0;
	}

	якщо (stat(nm, &s) != 0)
		повернути 0;
	якщо (mode == FILEXIST)
		повернути 1;
	якщо (is_file_access(mode)) {
		якщо (mode == FILRD)
			i = R_OK;
		якщо (mode == FILWR)
			i = W_OK;
		якщо (mode == FILEX)
			i = X_OK;
		повернути test_eaccess(&s, i) == 0;
	}
	якщо (is_file_type(mode)) {
		якщо (mode == FILREG)
			i = S_IFREG;
		якщо (mode == FILDIR)
			i = S_IFDIR;
		якщо (mode == FILCDEV)
			i = S_IFCHR;
		якщо (mode == FILBDEV)
			i = S_IFBLK;
		якщо (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#інакше
			повернути 0;
#endif
		}
		якщо (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#інакше
			повернути 0;
#endif
		}
 filetype:
		повернути ((s.st_mode & S_IFMT) == i);
	}
	якщо (is_file_bit(mode)) {
		якщо (mode == FILSUID)
			i = S_ISUID;
		якщо (mode == FILSGID)
			i = S_ISGID;
		якщо (mode == FILSTCK)
			i = S_ISVTX;
		повернути ((s.st_mode & i) != 0);
	}
	якщо (mode == FILGZ)
		повернути s.st_size > 0L;
	якщо (mode == FILUID)
		повернути s.st_uid == geteuid();
	якщо (mode == FILGID)
		повернути s.st_gid == getegid();
	повернути 1; /* NOTREACHED */
}


static number_t nexpr(enum token n)
{
	number_t res;

	nest_msg(">nexpr(%s)\n", TOKSTR[n]);
	якщо (n == UNOT) {
		n = check_operator(*++args);
		якщо (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_msg("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			повернути 1;
		}
		res = !nexpr(n);
		unnest_msg("<nexpr:%lld\n", res);
		повернути res;
	}
	res = primary(n);
	unnest_msg("<nexpr:%lld\n", res);
	повернути res;
}


static number_t aexpr(enum token n)
{
	number_t res;

	nest_msg(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_msg("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	якщо (check_operator(*++args) == BAND) {
		dbg_msg("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_msg("<aexpr:%lld\n", res);
		повернути res;
	}
	args--;
	unnest_msg("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	повернути res;
}


static number_t oexpr(enum token n)
{
	number_t res;

	nest_msg(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_msg("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	якщо (check_operator(*++args) == BOR) {
		dbg_msg("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_msg("<oexpr:%lld\n", res);
		повернути res;
	}
	args--;
	unnest_msg("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	повернути res;
}


static number_t primary(enum token n)
{
#якщо TEST_DEBUG
	number_t res = res; /* for compiler */
#інакше
	number_t res;
#endif
	const struct operator_t *args0_op;

	nest_msg(">primary(%s)\n", TOKSTR[n]);
	якщо (n == EOI) {
		syntax(NULL, "argument expected");
	}
	якщо (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		якщо (check_operator(*++args) != RPAREN)
			syntax(NULL, "closing paren expected");
		unnest_msg("<primary:%lld\n", res);
		повернути res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	якщо (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		якщо (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			якщо (last_operator->op_type == BINOP)
				unnest_msg_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	якщо (args0_op->op_type == UNOP) {
		/* unary expression */
		якщо (args[1] == NULL)
//			syntax(args0_op->op_text, "argument expected");
			goto check_emptiness;
		args++;
		якщо (n == STREZ)
			unnest_msg_and_return(args[0][0] == '\0', "<primary:%lld\n");
		якщо (n == STRNZ)
			unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
		якщо (n == FILTT)
			unnest_msg_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_msg_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	якщо (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_msg_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


ціле test_main(ціле argc, char **argv)
{
	ціле res;
	const char *arg0;
#якщо BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	якщо ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		якщо (!arg0[1]) { /* "[" ? */
			якщо (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_msg("missing ]");
				повернути 2;
			}
		} інакше { /* assuming "[[" */
			якщо (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_msg("missing ]]");
				повернути 2;
			}
#якщо BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = NULL;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#якщо BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	якщо (res)
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
	якщо (1) {
		ціле negate = 0;
 again:
		якщо (!argv[0]) {
			/* "test" */
			res = 1;
			goto ret_special;
		}
		якщо (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			goto ret_special;
		}
		якщо (argv[2]) {
			якщо (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				якщо (last_operator->op_type == BINOP) {
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
			якщо (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				якщо (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = NULL;
					argv++;
				}
			}
		}
 check_negate:
		якщо (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			goto again;
		}
	}

	res = !oexpr(check_operator(*args));

	якщо (*args != NULL && *++args != NULL) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_msg("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	повернути res;
}
