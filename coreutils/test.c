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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ <regex.h>
#அந்தர்பாவ <fnmatch.h>

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

#எனில் ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#இல்லையெனில்
# define BASH_TEST2 0
#endif

எண்ணிக்கை token {
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

#எனில் BASH_TEST2
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
#define is_int_op(a)      (((unsigned எழுத்து)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned எழுத்து)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned எழுத்து)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned எழுத்து)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned எழுத்து)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned எழுத்து)((a) - FILSUID)) <= 2)

#எனில் TEST_DEBUG
முழு depth;
#define nest_செய்தி(...) செய் { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} போது (0)
#define unnest_செய்தி(...) செய் { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} போது (0)
#define dbg_செய்தி(...) செய் { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} போது (0)
#define unnest_செய்தி_and_return(கோவை, ...) செய் { \
	எண்_t __res = (கோவை); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	திரும்பு __res; \
} போது (0)
நிலையான மாறிலி எழுத்து *மாறிலி TOKSTR[] ALIGN_PTR = {
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
#எனில் BASH_TEST2
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
#இல்லையெனில்
#define nest_செய்தி(...)   ((வெறுமை)0)
#define unnest_செய்தி(...) ((வெறுமை)0)
#define dbg_செய்தி(...)    ((வெறுமை)0)
#define unnest_செய்தி_and_return(கோவை, ...) திரும்பு கோவை
#endif

எண்ணிக்கை {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

கட்டமைப்பு operator_t {
	unsigned எழுத்து op_இலக்கம், op_type;
};

நிலையான மாறிலி கட்டமைப்பு operator_t ops_table[] ALIGN2 = {
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
#எனில் BASH_TEST2
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
#எனில் BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
நிலையான மாறிலி எழுத்து ops_texts[] ALIGN1 =
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
#எனில் BASH_TEST2
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
#எனில் BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#எனில் ENABLE_FEATURE_TEST_64
வகைவரை int64_t எண்_t;
#இல்லையெனில்
வகைவரை முழு எண்_t;
#endif


/* We try to minimize both static and stack usage. */
கட்டமைப்பு test_statics {
	எழுத்து **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	மாறிலி கட்டமைப்பு operator_t *last_operator;
	gid_t *group_array;
	முழு ngroups;
#எனில் BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
வெளி கட்டமைப்பு test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() செய் { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, அளவு(S)); \
} போது (0)
#define DEINIT_S() செய் { \
	விடுவி(group_array); \
	விடுவி(test_ptr_to_statics); \
} போது (0)

நிலையான எண்_t primary(எண்ணிக்கை token n);

நிலையான வெறுமை syntax(மாறிலி எழுத்து *op, மாறிலி எழுத்து *செய்தி) NORETURN;
நிலையான வெறுமை syntax(மாறிலி எழுத்து *op, மாறிலி எழுத்து *செய்தி)
{
	எனில் (op && *op) {
		bb_error_செய்தி("%s: %s", op, செய்தி);
	} இல்லையெனில் {
		bb_error_செய்தி("%s: %s"+4, செய்தி);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
நிலையான எண்_t getn(மாறிலி எழுத்து *s)
{
	எழுத்து *p;
#எனில் ENABLE_FEATURE_TEST_64
	long long r;
#இல்லையெனில்
	long r;
#endif

	errno = 0;
#எனில் ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#இல்லையெனில்
	r = strtol(s, &p, 10);
#endif

	எனில் (errno != 0)
		syntax(s, "out of range");

	எனில் (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	திரும்பு r;
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


நிலையான எண்ணிக்கை token check_operator(மாறிலி எழுத்து *s)
{
	நிலையான மாறிலி கட்டமைப்பு operator_t no_op = {
		.op_இலக்கம் = -1,
		.op_type = -1
	};
	முழு n;

	last_operator = &no_op;
	எனில் (s == வெற்று)
		திரும்பு EOI;
	n = index_in_strings(ops_texts, s);
	எனில் (n < 0)
		திரும்பு OPERAND;

#எனில் BASH_TEST2
	எனில் (ops_table[n].op_இலக்கம் == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		திரும்பு OPERAND;
	}
	எனில் (ops_table[n].op_இலக்கம் == BAND || ops_table[n].op_இலக்கம் == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		எனில் (bash_test2 == (s[0] == '-'))
			திரும்பு OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	திரும்பு ops_table[n].op_இலக்கம்;
}


நிலையான முழு binop(வெறுமை)
{
	மாறிலி எழுத்து *opnd1, *opnd2;
	மாறிலி கட்டமைப்பு operator_t *op;
	எண்_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	எனில் (opnd2 == வெற்று)
		syntax(args[-1], "argument expected");

	எனில் (is_int_op(op->op_இலக்கம்)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		எனில் (op->op_இலக்கம் == INTEQ)
			திரும்பு val1 == val2;
		எனில் (op->op_இலக்கம் == INTNE)
			திரும்பு val1 != val2;
		எனில் (op->op_இலக்கம் == INTGE)
			திரும்பு val1 >= val2;
		எனில் (op->op_இலக்கம் == INTGT)
			திரும்பு val1 >  val2;
		எனில் (op->op_இலக்கம் == INTLE)
			திரும்பு val1 <= val2;
		/*if (op->op_num == INTLT)*/
		திரும்பு val1 <  val2;
	}
#எனில் BASH_TEST2
	எனில் (bash_test2) {
		எனில் (op->op_இலக்கம் == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			திரும்பு val1 == 0;
		}
		எனில் (op->op_இலக்கம் == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			திரும்பு val1 != 0;
		}
		எனில் (op->op_இலக்கம் == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, அளவு(re_buffer));
			எனில் (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, வெற்று, 0);
			regfree(&re_buffer);
			திரும்பு val1 == 0;
		}
	}
#endif
	எனில் (is_str_op(op->op_இலக்கம்)) {
		val1 = strcmp(opnd1, opnd2);
		எனில் (op->op_இலக்கம் == STREQ)
			திரும்பு val1 == 0;
		எனில் (op->op_இலக்கம் == STRNE)
			திரும்பு val1 != 0;
		எனில் (op->op_இலக்கம் == STRLT)
			திரும்பு val1 < 0;
		/*if (op->op_num == STRGT)*/
		திரும்பு val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		கட்டமைப்பு stat b1, b2;

		எனில் (stat(opnd1, &b1) || stat(opnd2, &b2))
			திரும்பு 0; /* false, since at least one stat failed */
		எனில் (op->op_இலக்கம் == FILNT)
			திரும்பு b1.st_mtime > b2.st_mtime;
		எனில் (op->op_இலக்கம் == FILOT)
			திரும்பு b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		திரும்பு b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

நிலையான வெறுமை initialize_group_array(வெறுமை)
{
	group_array = bb_getgroups(&ngroups, வெற்று);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
நிலையான முழு is_a_group_member(gid_t gid)
{
	முழு i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	எனில் (gid == getgid() || gid == getegid())
		திரும்பு 1;

	எனில் (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	க்கு (i = 0; i < ngroups; i++)
		எனில் (gid == group_array[i])
			திரும்பு 1;

	திரும்பு 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
நிலையான முழு test_eaccess(கட்டமைப்பு stat *st, முழு mode)
{
	unsigned முழு euid = geteuid();

	எனில் (euid == 0) {
		/* Root can read or write any file. */
		எனில் (mode != X_OK)
			திரும்பு 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		எனில் (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			திரும்பு 0;
	}

	எனில் (st->st_uid == euid)  /* owner */
		mode <<= 6;
	இல்லையெனில் எனில் (is_a_group_member(st->st_gid))
		mode <<= 3;

	எனில் (st->st_mode & mode)
		திரும்பு 0;

	திரும்பு -1;
}


நிலையான முழு filstat(எழுத்து *nm, எண்ணிக்கை token mode)
{
	கட்டமைப்பு stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	எனில் (mode == FILSYM) {
#ifdef S_IFLNK
		எனில் (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			செல் filetype;
		}
#endif
		திரும்பு 0;
	}

	எனில் (stat(nm, &s) != 0)
		திரும்பு 0;
	எனில் (mode == FILEXIST)
		திரும்பு 1;
	எனில் (is_file_access(mode)) {
		எனில் (mode == FILRD)
			i = R_OK;
		எனில் (mode == FILWR)
			i = W_OK;
		எனில் (mode == FILEX)
			i = X_OK;
		திரும்பு test_eaccess(&s, i) == 0;
	}
	எனில் (is_file_type(mode)) {
		எனில் (mode == FILREG)
			i = S_IFREG;
		எனில் (mode == FILDIR)
			i = S_IFDIR;
		எனில் (mode == FILCDEV)
			i = S_IFCHR;
		எனில் (mode == FILBDEV)
			i = S_IFBLK;
		எனில் (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#இல்லையெனில்
			திரும்பு 0;
#endif
		}
		எனில் (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#இல்லையெனில்
			திரும்பு 0;
#endif
		}
 filetype:
		திரும்பு ((s.st_mode & S_IFMT) == i);
	}
	எனில் (is_file_bit(mode)) {
		எனில் (mode == FILSUID)
			i = S_ISUID;
		எனில் (mode == FILSGID)
			i = S_ISGID;
		எனில் (mode == FILSTCK)
			i = S_ISVTX;
		திரும்பு ((s.st_mode & i) != 0);
	}
	எனில் (mode == FILGZ)
		திரும்பு s.st_size > 0L;
	எனில் (mode == FILUID)
		திரும்பு s.st_uid == geteuid();
	எனில் (mode == FILGID)
		திரும்பு s.st_gid == getegid();
	திரும்பு 1; /* NOTREACHED */
}


நிலையான எண்_t nexpr(எண்ணிக்கை token n)
{
	எண்_t res;

	nest_செய்தி(">nexpr(%s)\n", TOKSTR[n]);
	எனில் (n == UNOT) {
		n = check_operator(*++args);
		எனில் (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_செய்தி("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			திரும்பு 1;
		}
		res = !nexpr(n);
		unnest_செய்தி("<nexpr:%lld\n", res);
		திரும்பு res;
	}
	res = primary(n);
	unnest_செய்தி("<nexpr:%lld\n", res);
	திரும்பு res;
}


நிலையான எண்_t aexpr(எண்ணிக்கை token n)
{
	எண்_t res;

	nest_செய்தி(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_செய்தி("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	எனில் (check_operator(*++args) == BAND) {
		dbg_செய்தி("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_செய்தி("<aexpr:%lld\n", res);
		திரும்பு res;
	}
	args--;
	unnest_செய்தி("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	திரும்பு res;
}


நிலையான எண்_t oexpr(எண்ணிக்கை token n)
{
	எண்_t res;

	nest_செய்தி(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_செய்தி("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	எனில் (check_operator(*++args) == BOR) {
		dbg_செய்தி("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_செய்தி("<oexpr:%lld\n", res);
		திரும்பு res;
	}
	args--;
	unnest_செய்தி("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	திரும்பு res;
}


நிலையான எண்_t primary(எண்ணிக்கை token n)
{
#எனில் TEST_DEBUG
	எண்_t res = res; /* for compiler */
#இல்லையெனில்
	எண்_t res;
#endif
	மாறிலி கட்டமைப்பு operator_t *args0_op;

	nest_செய்தி(">primary(%s)\n", TOKSTR[n]);
	எனில் (n == EOI) {
		syntax(வெற்று, "argument expected");
	}
	எனில் (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		எனில் (check_operator(*++args) != RPAREN)
			syntax(வெற்று, "closing paren expected");
		unnest_செய்தி("<primary:%lld\n", res);
		திரும்பு res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	எனில் (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		எனில் (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			எனில் (last_operator->op_type == BINOP)
				unnest_செய்தி_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	எனில் (args0_op->op_type == UNOP) {
		/* unary expression */
		எனில் (args[1] == வெற்று)
//			syntax(args0_op->op_text, "argument expected");
			செல் check_emptiness;
		args++;
		எனில் (n == STREZ)
			unnest_செய்தி_and_return(args[0][0] == '\0', "<primary:%lld\n");
		எனில் (n == STRNZ)
			unnest_செய்தி_and_return(args[0][0] != '\0', "<primary:%lld\n");
		எனில் (n == FILTT)
			unnest_செய்தி_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_செய்தி_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	எனில் (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_செய்தி_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_செய்தி_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


முழு test_main(முழு argc, எழுத்து **argv)
{
	முழு res;
	மாறிலி எழுத்து *arg0;
#எனில் BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	எனில் ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		எனில் (!arg0[1]) { /* "[" ? */
			எனில் (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_செய்தி("missing ]");
				திரும்பு 2;
			}
		} இல்லையெனில் { /* assuming "[[" */
			எனில் (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_செய்தி("missing ]]");
				திரும்பு 2;
			}
#எனில் BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = வெற்று;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#எனில் BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	எனில் (res)
		செல் ret;

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
	எனில் (1) {
		முழு negate = 0;
 again:
		எனில் (!argv[0]) {
			/* "test" */
			res = 1;
			செல் ret_special;
		}
		எனில் (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			செல் ret_special;
		}
		எனில் (argv[2]) {
			எனில் (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				எனில் (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					செல் ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				செல் check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			எனில் (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				எனில் (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = வெற்று;
					argv++;
				}
			}
		}
 check_negate:
		எனில் (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			செல் again;
		}
	}

	res = !oexpr(check_operator(*args));

	எனில் (*args != வெற்று && *++args != வெற்று) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_செய்தி("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	திரும்பு res;
}
