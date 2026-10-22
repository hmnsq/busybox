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

#شامل "libbb.h"
#شامل <regex.h>
#شامل <fnmatch.h>

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

#اگر ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#وگرنه
# define BASH_TEST2 0
#endif

شمارش token {
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

#اگر BASH_TEST2
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
#define is_int_op(a)      (((unsigned نویسه)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned نویسه)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned نویسه)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned نویسه)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned نویسه)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned نویسه)((a) - FILSUID)) <= 2)

#اگر TEST_DEBUG
صحیح depth;
#define nest_پیام(...) انجامبده { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} وقتی (0)
#define unnest_پیام(...) انجامبده { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} وقتی (0)
#define dbg_پیام(...) انجامبده { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} وقتی (0)
#define unnest_پیام_and_return(عبارت, ...) انجامبده { \
	عدد_t __res = (عبارت); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	بازگشت __res; \
} وقتی (0)
ایستا ثابت نویسه *ثابت TOKSTR[] ALIGN_PTR = {
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
#اگر BASH_TEST2
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
#وگرنه
#define nest_پیام(...)   ((تهی)0)
#define unnest_پیام(...) ((تهی)0)
#define dbg_پیام(...)    ((تهی)0)
#define unnest_پیام_and_return(عبارت, ...) بازگشت عبارت
#endif

شمارش {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ساختار operator_t {
	unsigned نویسه op_رقم, op_type;
};

ایستا ثابت ساختار operator_t ops_table[] ALIGN2 = {
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
#اگر BASH_TEST2
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
#اگر BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ایستا ثابت نویسه ops_texts[] ALIGN1 =
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
#اگر BASH_TEST2
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
#اگر BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#اگر ENABLE_FEATURE_TEST_64
تعریفنوع int64_t عدد_t;
#وگرنه
تعریفنوع صحیح عدد_t;
#endif


/* We try to minimize both static and stack usage. */
ساختار test_statics {
	نویسه **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ثابت ساختار operator_t *last_operator;
	gid_t *group_array;
	صحیح ngroups;
#اگر BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
بیرونی ساختار test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() انجامبده { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, اندازه(S)); \
} وقتی (0)
#define DEINIT_S() انجامبده { \
	آزاد(group_array); \
	آزاد(test_ptr_to_statics); \
} وقتی (0)

ایستا عدد_t primary(شمارش token n);

ایستا تهی syntax(ثابت نویسه *op, ثابت نویسه *پیام) NORETURN;
ایستا تهی syntax(ثابت نویسه *op, ثابت نویسه *پیام)
{
	اگر (op && *op) {
		bb_error_پیام("%s: %s", op, پیام);
	} وگرنه {
		bb_error_پیام("%s: %s"+4, پیام);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ایستا عدد_t getn(ثابت نویسه *s)
{
	نویسه *p;
#اگر ENABLE_FEATURE_TEST_64
	long long r;
#وگرنه
	long r;
#endif

	errno = 0;
#اگر ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#وگرنه
	r = strtol(s, &p, 10);
#endif

	اگر (errno != 0)
		syntax(s, "out of range");

	اگر (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	بازگشت r;
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


ایستا شمارش token check_operator(ثابت نویسه *s)
{
	ایستا ثابت ساختار operator_t no_op = {
		.op_رقم = -1,
		.op_type = -1
	};
	صحیح n;

	last_operator = &no_op;
	اگر (s == پوچ)
		بازگشت EOI;
	n = index_in_strings(ops_texts, s);
	اگر (n < 0)
		بازگشت OPERAND;

#اگر BASH_TEST2
	اگر (ops_table[n].op_رقم == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		بازگشت OPERAND;
	}
	اگر (ops_table[n].op_رقم == BAND || ops_table[n].op_رقم == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		اگر (bash_test2 == (s[0] == '-'))
			بازگشت OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	بازگشت ops_table[n].op_رقم;
}


ایستا صحیح binop(تهی)
{
	ثابت نویسه *opnd1, *opnd2;
	ثابت ساختار operator_t *op;
	عدد_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	اگر (opnd2 == پوچ)
		syntax(args[-1], "argument expected");

	اگر (is_int_op(op->op_رقم)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		اگر (op->op_رقم == INTEQ)
			بازگشت val1 == val2;
		اگر (op->op_رقم == INTNE)
			بازگشت val1 != val2;
		اگر (op->op_رقم == INTGE)
			بازگشت val1 >= val2;
		اگر (op->op_رقم == INTGT)
			بازگشت val1 >  val2;
		اگر (op->op_رقم == INTLE)
			بازگشت val1 <= val2;
		/*if (op->op_num == INTLT)*/
		بازگشت val1 <  val2;
	}
#اگر BASH_TEST2
	اگر (bash_test2) {
		اگر (op->op_رقم == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			بازگشت val1 == 0;
		}
		اگر (op->op_رقم == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			بازگشت val1 != 0;
		}
		اگر (op->op_رقم == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, اندازه(re_buffer));
			اگر (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, پوچ, 0);
			regfree(&re_buffer);
			بازگشت val1 == 0;
		}
	}
#endif
	اگر (is_str_op(op->op_رقم)) {
		val1 = strcmp(opnd1, opnd2);
		اگر (op->op_رقم == STREQ)
			بازگشت val1 == 0;
		اگر (op->op_رقم == STRNE)
			بازگشت val1 != 0;
		اگر (op->op_رقم == STRLT)
			بازگشت val1 < 0;
		/*if (op->op_num == STRGT)*/
		بازگشت val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ساختار stat b1, b2;

		اگر (stat(opnd1, &b1) || stat(opnd2, &b2))
			بازگشت 0; /* false, since at least one stat failed */
		اگر (op->op_رقم == FILNT)
			بازگشت b1.st_mtime > b2.st_mtime;
		اگر (op->op_رقم == FILOT)
			بازگشت b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		بازگشت b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ایستا تهی initialize_group_array(تهی)
{
	group_array = bb_getgroups(&ngroups, پوچ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ایستا صحیح is_a_group_member(gid_t gid)
{
	صحیح i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	اگر (gid == getgid() || gid == getegid())
		بازگشت 1;

	اگر (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	برای (i = 0; i < ngroups; i++)
		اگر (gid == group_array[i])
			بازگشت 1;

	بازگشت 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ایستا صحیح test_eaccess(ساختار stat *st, صحیح mode)
{
	unsigned صحیح euid = geteuid();

	اگر (euid == 0) {
		/* Root can read or write any file. */
		اگر (mode != X_OK)
			بازگشت 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		اگر (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			بازگشت 0;
	}

	اگر (st->st_uid == euid)  /* owner */
		mode <<= 6;
	وگرنه اگر (is_a_group_member(st->st_gid))
		mode <<= 3;

	اگر (st->st_mode & mode)
		بازگشت 0;

	بازگشت -1;
}


ایستا صحیح filstat(نویسه *nm, شمارش token mode)
{
	ساختار stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	اگر (mode == FILSYM) {
#ifdef S_IFLNK
		اگر (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			برو filetype;
		}
#endif
		بازگشت 0;
	}

	اگر (stat(nm, &s) != 0)
		بازگشت 0;
	اگر (mode == FILEXIST)
		بازگشت 1;
	اگر (is_file_access(mode)) {
		اگر (mode == FILRD)
			i = R_OK;
		اگر (mode == FILWR)
			i = W_OK;
		اگر (mode == FILEX)
			i = X_OK;
		بازگشت test_eaccess(&s, i) == 0;
	}
	اگر (is_file_type(mode)) {
		اگر (mode == FILREG)
			i = S_IFREG;
		اگر (mode == FILDIR)
			i = S_IFDIR;
		اگر (mode == FILCDEV)
			i = S_IFCHR;
		اگر (mode == FILBDEV)
			i = S_IFBLK;
		اگر (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#وگرنه
			بازگشت 0;
#endif
		}
		اگر (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#وگرنه
			بازگشت 0;
#endif
		}
 filetype:
		بازگشت ((s.st_mode & S_IFMT) == i);
	}
	اگر (is_file_bit(mode)) {
		اگر (mode == FILSUID)
			i = S_ISUID;
		اگر (mode == FILSGID)
			i = S_ISGID;
		اگر (mode == FILSTCK)
			i = S_ISVTX;
		بازگشت ((s.st_mode & i) != 0);
	}
	اگر (mode == FILGZ)
		بازگشت s.st_size > 0L;
	اگر (mode == FILUID)
		بازگشت s.st_uid == geteuid();
	اگر (mode == FILGID)
		بازگشت s.st_gid == getegid();
	بازگشت 1; /* NOTREACHED */
}


ایستا عدد_t nexpr(شمارش token n)
{
	عدد_t res;

	nest_پیام(">nexpr(%s)\n", TOKSTR[n]);
	اگر (n == UNOT) {
		n = check_operator(*++args);
		اگر (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_پیام("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			بازگشت 1;
		}
		res = !nexpr(n);
		unnest_پیام("<nexpr:%lld\n", res);
		بازگشت res;
	}
	res = primary(n);
	unnest_پیام("<nexpr:%lld\n", res);
	بازگشت res;
}


ایستا عدد_t aexpr(شمارش token n)
{
	عدد_t res;

	nest_پیام(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_پیام("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	اگر (check_operator(*++args) == BAND) {
		dbg_پیام("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_پیام("<aexpr:%lld\n", res);
		بازگشت res;
	}
	args--;
	unnest_پیام("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	بازگشت res;
}


ایستا عدد_t oexpr(شمارش token n)
{
	عدد_t res;

	nest_پیام(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_پیام("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	اگر (check_operator(*++args) == BOR) {
		dbg_پیام("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_پیام("<oexpr:%lld\n", res);
		بازگشت res;
	}
	args--;
	unnest_پیام("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	بازگشت res;
}


ایستا عدد_t primary(شمارش token n)
{
#اگر TEST_DEBUG
	عدد_t res = res; /* for compiler */
#وگرنه
	عدد_t res;
#endif
	ثابت ساختار operator_t *args0_op;

	nest_پیام(">primary(%s)\n", TOKSTR[n]);
	اگر (n == EOI) {
		syntax(پوچ, "argument expected");
	}
	اگر (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		اگر (check_operator(*++args) != RPAREN)
			syntax(پوچ, "closing paren expected");
		unnest_پیام("<primary:%lld\n", res);
		بازگشت res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	اگر (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		اگر (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			اگر (last_operator->op_type == BINOP)
				unnest_پیام_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	اگر (args0_op->op_type == UNOP) {
		/* unary expression */
		اگر (args[1] == پوچ)
//			syntax(args0_op->op_text, "argument expected");
			برو check_emptiness;
		args++;
		اگر (n == STREZ)
			unnest_پیام_and_return(args[0][0] == '\0', "<primary:%lld\n");
		اگر (n == STRNZ)
			unnest_پیام_and_return(args[0][0] != '\0', "<primary:%lld\n");
		اگر (n == FILTT)
			unnest_پیام_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_پیام_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	اگر (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_پیام_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_پیام_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


صحیح test_main(صحیح argc, نویسه **argv)
{
	صحیح res;
	ثابت نویسه *arg0;
#اگر BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	اگر ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		اگر (!arg0[1]) { /* "[" ? */
			اگر (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_پیام("missing ]");
				بازگشت 2;
			}
		} وگرنه { /* assuming "[[" */
			اگر (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_پیام("missing ]]");
				بازگشت 2;
			}
#اگر BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = پوچ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#اگر BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	اگر (res)
		برو ret;

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
	اگر (1) {
		صحیح negate = 0;
 again:
		اگر (!argv[0]) {
			/* "test" */
			res = 1;
			برو ret_special;
		}
		اگر (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			برو ret_special;
		}
		اگر (argv[2]) {
			اگر (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				اگر (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					برو ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				برو check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			اگر (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				اگر (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = پوچ;
					argv++;
				}
			}
		}
 check_negate:
		اگر (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			برو again;
		}
	}

	res = !oexpr(check_operator(*args));

	اگر (*args != پوچ && *++args != پوچ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_پیام("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	بازگشت res;
}
