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

#nếu ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#không_thì
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

#nếu BASH_TEST2
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

#nếu TEST_DEBUG
số_nguyên depth;
#define nest_msg(...) do { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} trong_khi (0)
#define unnest_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} trong_khi (0)
#define dbg_msg(...) do { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} trong_khi (0)
#define unnest_msg_and_return(expr, ...) do { \
	number_t __res = (expr); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	trả_về __res; \
} trong_khi (0)
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
#nếu BASH_TEST2
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
#không_thì
#define nest_msg(...)   ((void)0)
#define unnest_msg(...) ((void)0)
#define dbg_msg(...)    ((void)0)
#define unnest_msg_and_return(expr, ...) trả_về expr
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
#nếu BASH_TEST2
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
#nếu BASH_TEST2
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
#nếu BASH_TEST2
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
#nếu BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#nếu ENABLE_FEATURE_TEST_64
typedef int64_t number_t;
#không_thì
typedef số_nguyên number_t;
#endif


/* We try to minimize both static and stack usage. */
struct test_statics {
	char **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	const struct operator_t *last_operator;
	gid_t *group_array;
	số_nguyên ngroups;
#nếu BASH_TEST2
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
} trong_khi (0)
#define DEINIT_S() do { \
	free(group_array); \
	free(test_ptr_to_statics); \
} trong_khi (0)

static number_t primary(enum token n);

static void syntax(const char *op, const char *msg) NORETURN;
static void syntax(const char *op, const char *msg)
{
	nếu (op && *op) {
		bb_error_msg("%s: %s", op, msg);
	} không_thì {
		bb_error_msg("%s: %s"+4, msg);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
static number_t getn(const char *s)
{
	char *p;
#nếu ENABLE_FEATURE_TEST_64
	long long r;
#không_thì
	long r;
#endif

	errno = 0;
#nếu ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#không_thì
	r = strtol(s, &p, 10);
#endif

	nếu (errno != 0)
		syntax(s, "out of range");

	nếu (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	trả_về r;
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
	số_nguyên n;

	last_operator = &no_op;
	nếu (s == NULL)
		trả_về EOI;
	n = index_in_strings(ops_texts, s);
	nếu (n < 0)
		trả_về OPERAND;

#nếu BASH_TEST2
	nếu (ops_table[n].op_num == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		trả_về OPERAND;
	}
	nếu (ops_table[n].op_num == BAND || ops_table[n].op_num == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		nếu (bash_test2 == (s[0] == '-'))
			trả_về OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	trả_về ops_table[n].op_num;
}


static số_nguyên binop(void)
{
	const char *opnd1, *opnd2;
	const struct operator_t *op;
	number_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	nếu (opnd2 == NULL)
		syntax(args[-1], "argument expected");

	nếu (is_int_op(op->op_num)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		nếu (op->op_num == INTEQ)
			trả_về val1 == val2;
		nếu (op->op_num == INTNE)
			trả_về val1 != val2;
		nếu (op->op_num == INTGE)
			trả_về val1 >= val2;
		nếu (op->op_num == INTGT)
			trả_về val1 >  val2;
		nếu (op->op_num == INTLE)
			trả_về val1 <= val2;
		/*if (op->op_num == INTLT)*/
		trả_về val1 <  val2;
	}
#nếu BASH_TEST2
	nếu (bash_test2) {
		nếu (op->op_num == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			trả_về val1 == 0;
		}
		nếu (op->op_num == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			trả_về val1 != 0;
		}
		nếu (op->op_num == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, sizeof(re_buffer));
			nếu (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, NULL, 0);
			regfree(&re_buffer);
			trả_về val1 == 0;
		}
	}
#endif
	nếu (is_str_op(op->op_num)) {
		val1 = strcmp(opnd1, opnd2);
		nếu (op->op_num == STREQ)
			trả_về val1 == 0;
		nếu (op->op_num == STRNE)
			trả_về val1 != 0;
		nếu (op->op_num == STRLT)
			trả_về val1 < 0;
		/*if (op->op_num == STRGT)*/
		trả_về val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		struct stat b1, b2;

		nếu (stat(opnd1, &b1) || stat(opnd2, &b2))
			trả_về 0; /* false, since at least one stat failed */
		nếu (op->op_num == FILNT)
			trả_về b1.st_mtime > b2.st_mtime;
		nếu (op->op_num == FILOT)
			trả_về b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		trả_về b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
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
static số_nguyên is_a_group_member(gid_t gid)
{
	số_nguyên i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	nếu (gid == getgid() || gid == getegid())
		trả_về 1;

	nếu (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	cho (i = 0; i < ngroups; i++)
		nếu (gid == group_array[i])
			trả_về 1;

	trả_về 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
static số_nguyên test_eaccess(struct stat *st, số_nguyên mode)
{
	unsigned số_nguyên euid = geteuid();

	nếu (euid == 0) {
		/* Root can read or write any file. */
		nếu (mode != X_OK)
			trả_về 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		nếu (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			trả_về 0;
	}

	nếu (st->st_uid == euid)  /* owner */
		mode <<= 6;
	không_thì nếu (is_a_group_member(st->st_gid))
		mode <<= 3;

	nếu (st->st_mode & mode)
		trả_về 0;

	trả_về -1;
}


static số_nguyên filstat(char *nm, enum token mode)
{
	struct stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	nếu (mode == FILSYM) {
#ifdef S_IFLNK
		nếu (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			goto filetype;
		}
#endif
		trả_về 0;
	}

	nếu (stat(nm, &s) != 0)
		trả_về 0;
	nếu (mode == FILEXIST)
		trả_về 1;
	nếu (is_file_access(mode)) {
		nếu (mode == FILRD)
			i = R_OK;
		nếu (mode == FILWR)
			i = W_OK;
		nếu (mode == FILEX)
			i = X_OK;
		trả_về test_eaccess(&s, i) == 0;
	}
	nếu (is_file_type(mode)) {
		nếu (mode == FILREG)
			i = S_IFREG;
		nếu (mode == FILDIR)
			i = S_IFDIR;
		nếu (mode == FILCDEV)
			i = S_IFCHR;
		nếu (mode == FILBDEV)
			i = S_IFBLK;
		nếu (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#không_thì
			trả_về 0;
#endif
		}
		nếu (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#không_thì
			trả_về 0;
#endif
		}
 filetype:
		trả_về ((s.st_mode & S_IFMT) == i);
	}
	nếu (is_file_bit(mode)) {
		nếu (mode == FILSUID)
			i = S_ISUID;
		nếu (mode == FILSGID)
			i = S_ISGID;
		nếu (mode == FILSTCK)
			i = S_ISVTX;
		trả_về ((s.st_mode & i) != 0);
	}
	nếu (mode == FILGZ)
		trả_về s.st_size > 0L;
	nếu (mode == FILUID)
		trả_về s.st_uid == geteuid();
	nếu (mode == FILGID)
		trả_về s.st_gid == getegid();
	trả_về 1; /* NOTREACHED */
}


static number_t nexpr(enum token n)
{
	number_t res;

	nest_msg(">nexpr(%s)\n", TOKSTR[n]);
	nếu (n == UNOT) {
		n = check_operator(*++args);
		nếu (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_msg("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			trả_về 1;
		}
		res = !nexpr(n);
		unnest_msg("<nexpr:%lld\n", res);
		trả_về res;
	}
	res = primary(n);
	unnest_msg("<nexpr:%lld\n", res);
	trả_về res;
}


static number_t aexpr(enum token n)
{
	number_t res;

	nest_msg(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_msg("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	nếu (check_operator(*++args) == BAND) {
		dbg_msg("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_msg("<aexpr:%lld\n", res);
		trả_về res;
	}
	args--;
	unnest_msg("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	trả_về res;
}


static number_t oexpr(enum token n)
{
	number_t res;

	nest_msg(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_msg("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	nếu (check_operator(*++args) == BOR) {
		dbg_msg("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_msg("<oexpr:%lld\n", res);
		trả_về res;
	}
	args--;
	unnest_msg("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	trả_về res;
}


static number_t primary(enum token n)
{
#nếu TEST_DEBUG
	number_t res = res; /* for compiler */
#không_thì
	number_t res;
#endif
	const struct operator_t *args0_op;

	nest_msg(">primary(%s)\n", TOKSTR[n]);
	nếu (n == EOI) {
		syntax(NULL, "argument expected");
	}
	nếu (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		nếu (check_operator(*++args) != RPAREN)
			syntax(NULL, "closing paren expected");
		unnest_msg("<primary:%lld\n", res);
		trả_về res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	nếu (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		nếu (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			nếu (last_operator->op_type == BINOP)
				unnest_msg_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	nếu (args0_op->op_type == UNOP) {
		/* unary expression */
		nếu (args[1] == NULL)
//			syntax(args0_op->op_text, "argument expected");
			goto check_emptiness;
		args++;
		nếu (n == STREZ)
			unnest_msg_and_return(args[0][0] == '\0', "<primary:%lld\n");
		nếu (n == STRNZ)
			unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
		nếu (n == FILTT)
			unnest_msg_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_msg_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	nếu (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_msg_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_msg_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


số_nguyên test_main(số_nguyên argc, char **argv)
{
	số_nguyên res;
	const char *arg0;
#nếu BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	nếu ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		nếu (!arg0[1]) { /* "[" ? */
			nếu (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_msg("missing ]");
				trả_về 2;
			}
		} không_thì { /* assuming "[[" */
			nếu (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_msg("missing ]]");
				trả_về 2;
			}
#nếu BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = NULL;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#nếu BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	nếu (res)
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
	nếu (1) {
		số_nguyên negate = 0;
 again:
		nếu (!argv[0]) {
			/* "test" */
			res = 1;
			goto ret_special;
		}
		nếu (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			goto ret_special;
		}
		nếu (argv[2]) {
			nếu (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				nếu (last_operator->op_type == BINOP) {
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
			nếu (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				nếu (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = NULL;
					argv++;
				}
			}
		}
 check_negate:
		nếu (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			goto again;
		}
	}

	res = !oexpr(check_operator(*args));

	nếu (*args != NULL && *++args != NULL) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_msg("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	trả_về res;
}
