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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব <regex.h>
#অন্তর্ভাব <fnmatch.h>

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

#যদি ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#অন্যথা
# define BASH_TEST2 0
#endif

গণনা token {
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

#যদি BASH_TEST2
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
#define is_int_op(a)      (((unsigned অক্ষর)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned অক্ষর)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned অক্ষর)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned অক্ষর)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned অক্ষর)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned অক্ষর)((a) - FILSUID)) <= 2)

#যদি TEST_DEBUG
পূর্ণসংখ্যা depth;
#define nest_বার্তা(...) করো { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} যখন (0)
#define unnest_বার্তা(...) করো { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} যখন (0)
#define dbg_বার্তা(...) করো { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} যখন (0)
#define unnest_বার্তা_and_return(রাশিমালা, ...) করো { \
	সংখ্যা_t __res = (রাশিমালা); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	ফেরত __res; \
} যখন (0)
স্থির ধ্রুবক অক্ষর *ধ্রুবক TOKSTR[] ALIGN_PTR = {
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
#যদি BASH_TEST2
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
#অন্যথা
#define nest_বার্তা(...)   ((শূন্য)0)
#define unnest_বার্তা(...) ((শূন্য)0)
#define dbg_বার্তা(...)    ((শূন্য)0)
#define unnest_বার্তা_and_return(রাশিমালা, ...) ফেরত রাশিমালা
#endif

গণনা {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

কাঠামো operator_t {
	unsigned অক্ষর op_অঙ্ক, op_type;
};

স্থির ধ্রুবক কাঠামো operator_t ops_table[] ALIGN2 = {
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
#যদি BASH_TEST2
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
#যদি BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
স্থির ধ্রুবক অক্ষর ops_texts[] ALIGN1 =
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
#যদি BASH_TEST2
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
#যদি BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#যদি ENABLE_FEATURE_TEST_64
প্রকারলেখ int64_t সংখ্যা_t;
#অন্যথা
প্রকারলেখ পূর্ণসংখ্যা সংখ্যা_t;
#endif


/* We try to minimize both static and stack usage. */
কাঠামো test_statics {
	অক্ষর **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ধ্রুবক কাঠামো operator_t *last_operator;
	gid_t *group_array;
	পূর্ণসংখ্যা ngroups;
#যদি BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
বাহ্যিক কাঠামো test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() করো { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, আকার(S)); \
} যখন (0)
#define DEINIT_S() করো { \
	মুক্ত(group_array); \
	মুক্ত(test_ptr_to_statics); \
} যখন (0)

স্থির সংখ্যা_t primary(গণনা token n);

স্থির শূন্য syntax(ধ্রুবক অক্ষর *op, ধ্রুবক অক্ষর *বার্তা) NORETURN;
স্থির শূন্য syntax(ধ্রুবক অক্ষর *op, ধ্রুবক অক্ষর *বার্তা)
{
	যদি (op && *op) {
		bb_error_বার্তা("%s: %s", op, বার্তা);
	} অন্যথা {
		bb_error_বার্তা("%s: %s"+4, বার্তা);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
স্থির সংখ্যা_t getn(ধ্রুবক অক্ষর *s)
{
	অক্ষর *p;
#যদি ENABLE_FEATURE_TEST_64
	long long r;
#অন্যথা
	long r;
#endif

	errno = 0;
#যদি ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#অন্যথা
	r = strtol(s, &p, 10);
#endif

	যদি (errno != 0)
		syntax(s, "out of range");

	যদি (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	ফেরত r;
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


স্থির গণনা token check_operator(ধ্রুবক অক্ষর *s)
{
	স্থির ধ্রুবক কাঠামো operator_t no_op = {
		.op_অঙ্ক = -1,
		.op_type = -1
	};
	পূর্ণসংখ্যা n;

	last_operator = &no_op;
	যদি (s == ফাঁকা)
		ফেরত EOI;
	n = সূচক_in_strings(ops_texts, s);
	যদি (n < 0)
		ফেরত OPERAND;

#যদি BASH_TEST2
	যদি (ops_table[n].op_অঙ্ক == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		ফেরত OPERAND;
	}
	যদি (ops_table[n].op_অঙ্ক == BAND || ops_table[n].op_অঙ্ক == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		যদি (bash_test2 == (s[0] == '-'))
			ফেরত OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	ফেরত ops_table[n].op_অঙ্ক;
}


স্থির পূর্ণসংখ্যা binop(শূন্য)
{
	ধ্রুবক অক্ষর *opnd1, *opnd2;
	ধ্রুবক কাঠামো operator_t *op;
	সংখ্যা_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	যদি (opnd2 == ফাঁকা)
		syntax(args[-1], "argument expected");

	যদি (is_int_op(op->op_অঙ্ক)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		যদি (op->op_অঙ্ক == INTEQ)
			ফেরত val1 == val2;
		যদি (op->op_অঙ্ক == INTNE)
			ফেরত val1 != val2;
		যদি (op->op_অঙ্ক == INTGE)
			ফেরত val1 >= val2;
		যদি (op->op_অঙ্ক == INTGT)
			ফেরত val1 >  val2;
		যদি (op->op_অঙ্ক == INTLE)
			ফেরত val1 <= val2;
		/*if (op->op_num == INTLT)*/
		ফেরত val1 <  val2;
	}
#যদি BASH_TEST2
	যদি (bash_test2) {
		যদি (op->op_অঙ্ক == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ফেরত val1 == 0;
		}
		যদি (op->op_অঙ্ক == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			ফেরত val1 != 0;
		}
		যদি (op->op_অঙ্ক == REGEX) {
			regex_t re_বাফার;
			memset(&re_বাফার, 0, আকার(re_বাফার));
			যদি (regcomp(&re_বাফার, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_বাফার, opnd1, 0, ফাঁকা, 0);
			regfree(&re_বাফার);
			ফেরত val1 == 0;
		}
	}
#endif
	যদি (is_str_op(op->op_অঙ্ক)) {
		val1 = strcmp(opnd1, opnd2);
		যদি (op->op_অঙ্ক == STREQ)
			ফেরত val1 == 0;
		যদি (op->op_অঙ্ক == STRNE)
			ফেরত val1 != 0;
		যদি (op->op_অঙ্ক == STRLT)
			ফেরত val1 < 0;
		/*if (op->op_num == STRGT)*/
		ফেরত val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		কাঠামো stat b1, b2;

		যদি (stat(opnd1, &b1) || stat(opnd2, &b2))
			ফেরত 0; /* false, since at least one stat failed */
		যদি (op->op_অঙ্ক == FILNT)
			ফেরত b1.st_mtime > b2.st_mtime;
		যদি (op->op_অঙ্ক == FILOT)
			ফেরত b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		ফেরত b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

স্থির শূন্য initialize_group_array(শূন্য)
{
	group_array = bb_getgroups(&ngroups, ফাঁকা);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
স্থির পূর্ণসংখ্যা is_a_group_member(gid_t gid)
{
	পূর্ণসংখ্যা i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	যদি (gid == getgid() || gid == getegid())
		ফেরত 1;

	যদি (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	জন্য (i = 0; i < ngroups; i++)
		যদি (gid == group_array[i])
			ফেরত 1;

	ফেরত 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
স্থির পূর্ণসংখ্যা test_eaccess(কাঠামো stat *st, পূর্ণসংখ্যা mode)
{
	unsigned পূর্ণসংখ্যা euid = geteuid();

	যদি (euid == 0) {
		/* Root can read or write any file. */
		যদি (mode != X_OK)
			ফেরত 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		যদি (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			ফেরত 0;
	}

	যদি (st->st_uid == euid)  /* owner */
		mode <<= 6;
	অন্যথা যদি (is_a_group_member(st->st_gid))
		mode <<= 3;

	যদি (st->st_mode & mode)
		ফেরত 0;

	ফেরত -1;
}


স্থির পূর্ণসংখ্যা filstat(অক্ষর *nm, গণনা token mode)
{
	কাঠামো stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	যদি (mode == FILSYM) {
#ifdef S_IFLNK
		যদি (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			যাও filetype;
		}
#endif
		ফেরত 0;
	}

	যদি (stat(nm, &s) != 0)
		ফেরত 0;
	যদি (mode == FILEXIST)
		ফেরত 1;
	যদি (is_file_access(mode)) {
		যদি (mode == FILRD)
			i = R_OK;
		যদি (mode == FILWR)
			i = W_OK;
		যদি (mode == FILEX)
			i = X_OK;
		ফেরত test_eaccess(&s, i) == 0;
	}
	যদি (is_file_type(mode)) {
		যদি (mode == FILREG)
			i = S_IFREG;
		যদি (mode == FILDIR)
			i = S_IFDIR;
		যদি (mode == FILCDEV)
			i = S_IFCHR;
		যদি (mode == FILBDEV)
			i = S_IFBLK;
		যদি (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#অন্যথা
			ফেরত 0;
#endif
		}
		যদি (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#অন্যথা
			ফেরত 0;
#endif
		}
 filetype:
		ফেরত ((s.st_mode & S_IFMT) == i);
	}
	যদি (is_file_bit(mode)) {
		যদি (mode == FILSUID)
			i = S_ISUID;
		যদি (mode == FILSGID)
			i = S_ISGID;
		যদি (mode == FILSTCK)
			i = S_ISVTX;
		ফেরত ((s.st_mode & i) != 0);
	}
	যদি (mode == FILGZ)
		ফেরত s.st_আকার > 0L;
	যদি (mode == FILUID)
		ফেরত s.st_uid == geteuid();
	যদি (mode == FILGID)
		ফেরত s.st_gid == getegid();
	ফেরত 1; /* NOTREACHED */
}


স্থির সংখ্যা_t nexpr(গণনা token n)
{
	সংখ্যা_t res;

	nest_বার্তা(">nexpr(%s)\n", TOKSTR[n]);
	যদি (n == UNOT) {
		n = check_operator(*++args);
		যদি (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_বার্তা("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			ফেরত 1;
		}
		res = !nexpr(n);
		unnest_বার্তা("<nexpr:%lld\n", res);
		ফেরত res;
	}
	res = primary(n);
	unnest_বার্তা("<nexpr:%lld\n", res);
	ফেরত res;
}


স্থির সংখ্যা_t aexpr(গণনা token n)
{
	সংখ্যা_t res;

	nest_বার্তা(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_বার্তা("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	যদি (check_operator(*++args) == BAND) {
		dbg_বার্তা("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_বার্তা("<aexpr:%lld\n", res);
		ফেরত res;
	}
	args--;
	unnest_বার্তা("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ফেরত res;
}


স্থির সংখ্যা_t oexpr(গণনা token n)
{
	সংখ্যা_t res;

	nest_বার্তা(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_বার্তা("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	যদি (check_operator(*++args) == BOR) {
		dbg_বার্তা("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_বার্তা("<oexpr:%lld\n", res);
		ফেরত res;
	}
	args--;
	unnest_বার্তা("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	ফেরত res;
}


স্থির সংখ্যা_t primary(গণনা token n)
{
#যদি TEST_DEBUG
	সংখ্যা_t res = res; /* for compiler */
#অন্যথা
	সংখ্যা_t res;
#endif
	ধ্রুবক কাঠামো operator_t *args0_op;

	nest_বার্তা(">primary(%s)\n", TOKSTR[n]);
	যদি (n == EOI) {
		syntax(ফাঁকা, "argument expected");
	}
	যদি (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		যদি (check_operator(*++args) != RPAREN)
			syntax(ফাঁকা, "closing paren expected");
		unnest_বার্তা("<primary:%lld\n", res);
		ফেরত res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	যদি (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		যদি (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			যদি (last_operator->op_type == BINOP)
				unnest_বার্তা_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	যদি (args0_op->op_type == UNOP) {
		/* unary expression */
		যদি (args[1] == ফাঁকা)
//			syntax(args0_op->op_text, "argument expected");
			যাও check_emptiness;
		args++;
		যদি (n == STREZ)
			unnest_বার্তা_and_return(args[0][0] == '\0', "<primary:%lld\n");
		যদি (n == STRNZ)
			unnest_বার্তা_and_return(args[0][0] != '\0', "<primary:%lld\n");
		যদি (n == FILTT)
			unnest_বার্তা_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_বার্তা_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	যদি (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_বার্তা_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_বার্তা_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


পূর্ণসংখ্যা test_main(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা res;
	ধ্রুবক অক্ষর *arg0;
#যদি BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	যদি ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		যদি (!arg0[1]) { /* "[" ? */
			যদি (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_বার্তা("missing ]");
				ফেরত 2;
			}
		} অন্যথা { /* assuming "[[" */
			যদি (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_বার্তা("missing ]]");
				ফেরত 2;
			}
#যদি BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = ফাঁকা;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#যদি BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	যদি (res)
		যাও ret;

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
	যদি (1) {
		পূর্ণসংখ্যা negate = 0;
 again:
		যদি (!argv[0]) {
			/* "test" */
			res = 1;
			যাও ret_special;
		}
		যদি (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			যাও ret_special;
		}
		যদি (argv[2]) {
			যদি (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				যদি (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					যাও ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				যাও check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			যদি (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				যদি (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ফাঁকা;
					argv++;
				}
			}
		}
 check_negate:
		যদি (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			যাও again;
		}
	}

	res = !oexpr(check_operator(*args));

	যদি (*args != ফাঁকা && *++args != ফাঁকা) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_বার্তা("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	ফেরত res;
}
