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

#تعریف_کرو TEST_DEBUG 0

#اگر ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#ورنہ
# define BASH_TEST2 0
#اگر_ختم

شمار token {
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
#اگر_ختم

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
	عملیہ
};
#تعریف_کرو is_int_op(a)      (((غیرعلامتی حرف)((a) - INTEQ)) <= 5)
#تعریف_کرو is_str_op(a)      (((غیرعلامتی حرف)((a) - STREZ)) <= 5)
#تعریف_کرو is_file_op(a)     (((غیرعلامتی حرف)((a) - FILNT)) <= 2)
#تعریف_کرو is_file_access(a) (((غیرعلامتی حرف)((a) - FILRD)) <= 2)
#تعریف_کرو is_file_type(a)   (((غیرعلامتی حرف)((a) - FILREG)) <= 5)
#تعریف_کرو is_file_ثنائی_ہندسہ(a)    (((غیرعلامتی حرف)((a) - FILSUID)) <= 2)

#اگر TEST_DEBUG
صحیح گہرائی;
#تعریف_کرو nest_پیغام(...) کرو { \
	گہرائی++; \
	fprintf(stderr, "%*s", گہرائی*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} جب (0)
#تعریف_کرو unnest_پیغام(...) کرو { \
	fprintf(stderr, "%*s", گہرائی*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	گہرائی--; \
} جب (0)
#تعریف_کرو dbg_پیغام(...) کرو { \
	fprintf(stderr, "%*s", گہرائی*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} جب (0)
#تعریف_کرو unnest_پیغام_and_return(اظہاریہ, ...) کرو { \
	نمبر_t __res = (اظہاریہ); \
	fprintf(stderr, "%*s", گہرائی*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	گہرائی--; \
	واپس __res; \
} جب (0)
ساکن ثابت حرف *ثابت TOKSTR[] ALIGN_PTR = {
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
#اگر_ختم
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
#ورنہ
#تعریف_کرو nest_پیغام(...)   ((خالی)0)
#تعریف_کرو unnest_پیغام(...) ((خالی)0)
#تعریف_کرو dbg_پیغام(...)    ((خالی)0)
#تعریف_کرو unnest_پیغام_and_return(اظہاریہ, ...) واپس اظہاریہ
#اگر_ختم

شمار {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ساخت operator_t {
	غیرعلامتی حرف op_عدد, op_type;
};

ساکن ثابت ساخت operator_t ops_جدول[] ALIGN2 = {
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
#اگر_ختم
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
#اگر_ختم
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ساکن ثابت حرف ops_texts[] ALIGN1 =
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
#اگر_ختم
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
#اگر_ختم
	"("   "\0"
	")"   "\0"
;


#اگر ENABLE_FEATURE_TEST_64
قسمتعریف int64_t نمبر_t;
#ورنہ
قسمتعریف صحیح نمبر_t;
#اگر_ختم


/* We try to minimize both static and stack usage. */
ساخت test_statics {
	حرف **دلائل;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ثابت ساخت operator_t *آخری_operator;
	gid_t *گروہ_array;
	صحیح ngroups;
#اگر BASH_TEST2
	bool bash_test2;
#اگر_ختم
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
خارجی ساخت test_statics *BB_GLOBAL_CONST test_ptr_مقصد_statics;

#تعریف_کرو S (*test_ptr_مقصد_statics)
#تعریف_کرو دلائل            (S.دلائل         )
#تعریف_کرو آخری_operator   (S.آخری_operator)
#تعریف_کرو گروہ_array     (S.گروہ_array  )
#تعریف_کرو ngroups         (S.ngroups      )
#تعریف_کرو bash_test2      (S.bash_test2   )
#تعریف_کرو leaving         (S.leaving      )

#تعریف_کرو INIT_S() کرو { \
	XZALLOC_CONST_PTR(&test_ptr_مقصد_statics, حجم(S)); \
} جب (0)
#تعریف_کرو DEINIT_S() کرو { \
	آزاد(گروہ_array); \
	آزاد(test_ptr_مقصد_statics); \
} جب (0)

ساکن نمبر_t primary(شمار token n);

ساکن خالی syntax(ثابت حرف *op, ثابت حرف *پیغام) NORETURN;
ساکن خالی syntax(ثابت حرف *op, ثابت حرف *پیغام)
{
	اگر (op && *op) {
		bb_error_پیغام("%s: %s", op, پیغام);
	} ورنہ {
		bb_error_پیغام("%s: %s"+4, پیغام);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ساکن نمبر_t getn(ثابت حرف *s)
{
	حرف *p;
#اگر ENABLE_FEATURE_TEST_64
	طویل طویل r;
#ورنہ
	طویل r;
#اگر_ختم

	errno = 0;
#اگر ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#ورنہ
	r = strtol(s, &p, 10);
#اگر_ختم

	اگر (errno != 0)
		syntax(s, "out of range");

	اگر (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	واپس r;
}

/*غیر_مستعمل
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


ساکن شمار token check_operator(ثابت حرف *s)
{
	ساکن ثابت ساخت operator_t no_op = {
		.op_عدد = -1,
		.op_type = -1
	};
	صحیح n;

	آخری_operator = &no_op;
	اگر (s == تہی)
		واپس EOI;
	n = اشاریہ_اندر_لڑیاں(ops_texts, s);
	اگر (n < 0)
		واپس عملیہ;

#اگر BASH_TEST2
	اگر (ops_جدول[n].op_عدد == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		واپس عملیہ;
	}
	اگر (ops_جدول[n].op_عدد == BAND || ops_جدول[n].op_عدد == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		اگر (bash_test2 == (s[0] == '-'))
			واپس عملیہ;
	}
#اگر_ختم

	آخری_operator = &ops_جدول[n];
	واپس ops_جدول[n].op_عدد;
}


ساکن صحیح binop(خالی)
{
	ثابت حرف *opnd1, *opnd2;
	ثابت ساخت operator_t *op;
	نمبر_t val1, val2;

	opnd1 = *دلائل;
	check_operator(*++دلائل);
	op = آخری_operator;

	opnd2 = *++دلائل;
	اگر (opnd2 == تہی)
		syntax(دلائل[-1], "argument expected");

	اگر (is_int_op(op->op_عدد)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		اگر (op->op_عدد == INTEQ)
			واپس val1 == val2;
		اگر (op->op_عدد == INTNE)
			واپس val1 != val2;
		اگر (op->op_عدد == INTGE)
			واپس val1 >= val2;
		اگر (op->op_عدد == INTGT)
			واپس val1 >  val2;
		اگر (op->op_عدد == INTLE)
			واپس val1 <= val2;
		/*if (op->op_num == INTLT)*/
		واپس val1 <  val2;
	}
#اگر BASH_TEST2
	اگر (bash_test2) {
		اگر (op->op_عدد == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			واپس val1 == 0;
		}
		اگر (op->op_عدد == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			واپس val1 != 0;
		}
		اگر (op->op_عدد == REGEX) {
			regex_t re_بفر;
			memset(&re_بفر, 0, حجم(re_بفر));
			اگر (regcomp(&re_بفر, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_بفر, opnd1, 0, تہی, 0);
			regfree(&re_بفر);
			واپس val1 == 0;
		}
	}
#اگر_ختم
	اگر (is_str_op(op->op_عدد)) {
		val1 = strcmp(opnd1, opnd2);
		اگر (op->op_عدد == STREQ)
			واپس val1 == 0;
		اگر (op->op_عدد == STRNE)
			واپس val1 != 0;
		اگر (op->op_عدد == STRLT)
			واپس val1 < 0;
		/*if (op->op_num == STRGT)*/
		واپس val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ساخت stat b1, b2;

		اگر (stat(opnd1, &b1) || stat(opnd2, &b2))
			واپس 0; /* false, since at least one stat failed */
		اگر (op->op_عدد == FILNT)
			واپس b1.st_mtime > b2.st_mtime;
		اگر (op->op_عدد == FILOT)
			واپس b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		واپس b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ساکن خالی initialize_گروہ_array(خالی)
{
	گروہ_array = bb_getgroups(&ngroups, تہی);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ساکن صحیح is_a_گروہ_member(gid_t gid)
{
	صحیح i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	اگر (gid == getgid() || gid == getegid())
		واپس 1;

	اگر (ngroups == 0)
		initialize_گروہ_array();

	/* Search through the list looking for GID. */
	برائے (i = 0; i < ngroups; i++)
		اگر (gid == گروہ_array[i])
			واپس 1;

	واپس 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ساکن صحیح test_eaccess(ساخت stat *st, صحیح منوال)
{
	غیرعلامتی صحیح euid = geteuid();

	اگر (euid == 0) {
		/* Root can read or write any file. */
		اگر (منوال != X_OK)
			واپس 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		اگر (st->st_منوال & (S_IXUSR | S_IXGRP | S_IXOTH))
			واپس 0;
	}

	اگر (st->st_uid == euid)  /* owner */
		منوال <<= 6;
	ورنہ اگر (is_a_گروہ_member(st->st_gid))
		منوال <<= 3;

	اگر (st->st_منوال & منوال)
		واپس 0;

	واپس -1;
}


ساکن صحیح filstat(حرف *nm, شمار token منوال)
{
	ساخت stat s;
	غیرعلامتی i = i; /* gcc 3.x thinks it can be used uninitialized */

	اگر (منوال == FILSYM) {
#اگر_تعریف_شدہ S_IFLNK
		اگر (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			جاؤ filetype;
		}
#اگر_ختم
		واپس 0;
	}

	اگر (stat(nm, &s) != 0)
		واپس 0;
	اگر (منوال == FILEXIST)
		واپس 1;
	اگر (is_file_access(منوال)) {
		اگر (منوال == FILRD)
			i = R_OK;
		اگر (منوال == FILWR)
			i = W_OK;
		اگر (منوال == FILEX)
			i = X_OK;
		واپس test_eaccess(&s, i) == 0;
	}
	اگر (is_file_type(منوال)) {
		اگر (منوال == FILREG)
			i = S_IFREG;
		اگر (منوال == FILDIR)
			i = S_IFDIR;
		اگر (منوال == FILCDEV)
			i = S_IFCHR;
		اگر (منوال == FILBDEV)
			i = S_IFBLK;
		اگر (منوال == FILFIFO) {
#اگر_تعریف_شدہ S_IFIFO
			i = S_IFIFO;
#ورنہ
			واپس 0;
#اگر_ختم
		}
		اگر (منوال == FILSOCK) {
#اگر_تعریف_شدہ S_IFSOCK
			i = S_IFSOCK;
#ورنہ
			واپس 0;
#اگر_ختم
		}
 filetype:
		واپس ((s.st_منوال & S_IFMT) == i);
	}
	اگر (is_file_ثنائی_ہندسہ(منوال)) {
		اگر (منوال == FILSUID)
			i = S_ISUID;
		اگر (منوال == FILSGID)
			i = S_ISGID;
		اگر (منوال == FILSTCK)
			i = S_ISVTX;
		واپس ((s.st_منوال & i) != 0);
	}
	اگر (منوال == FILGZ)
		واپس s.st_سائز > 0L;
	اگر (منوال == FILUID)
		واپس s.st_uid == geteuid();
	اگر (منوال == FILGID)
		واپس s.st_gid == getegid();
	واپس 1; /* NOTREACHED */
}


ساکن نمبر_t nexpr(شمار token n)
{
	نمبر_t res;

	nest_پیغام(">nexpr(%s)\n", TOKSTR[n]);
	اگر (n == UNOT) {
		n = check_operator(*++دلائل);
		اگر (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			دلائل--;
			unnest_پیغام("<nexpr:1 (!EOI), args:%s(%p)\n", دلائل[0], &دلائل[0]);
			واپس 1;
		}
		res = !nexpr(n);
		unnest_پیغام("<nexpr:%lld\n", res);
		واپس res;
	}
	res = primary(n);
	unnest_پیغام("<nexpr:%lld\n", res);
	واپس res;
}


ساکن نمبر_t aexpr(شمار token n)
{
	نمبر_t res;

	nest_پیغام(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_پیغام("aexpr: nexpr:%lld, next args:%s(%p)\n", res, دلائل[1], &دلائل[1]);
	اگر (check_operator(*++دلائل) == BAND) {
		dbg_پیغام("aexpr: arg is AND, next args:%s(%p)\n", دلائل[1], &دلائل[1]);
		res = aexpr(check_operator(*++دلائل)) && res;
		unnest_پیغام("<aexpr:%lld\n", res);
		واپس res;
	}
	دلائل--;
	unnest_پیغام("<aexpr:%lld, args:%s(%p)\n", res, دلائل[0], &دلائل[0]);
	واپس res;
}


ساکن نمبر_t oexpr(شمار token n)
{
	نمبر_t res;

	nest_پیغام(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_پیغام("oexpr: aexpr:%lld, next args:%s(%p)\n", res, دلائل[1], &دلائل[1]);
	اگر (check_operator(*++دلائل) == BOR) {
		dbg_پیغام("oexpr: next arg is OR, next args:%s(%p)\n", دلائل[1], &دلائل[1]);
		res = oexpr(check_operator(*++دلائل)) || res;
		unnest_پیغام("<oexpr:%lld\n", res);
		واپس res;
	}
	دلائل--;
	unnest_پیغام("<oexpr:%lld, args:%s(%p)\n", res, دلائل[0], &دلائل[0]);
	واپس res;
}


ساکن نمبر_t primary(شمار token n)
{
#اگر TEST_DEBUG
	نمبر_t res = res; /* for compiler */
#ورنہ
	نمبر_t res;
#اگر_ختم
	ثابت ساخت operator_t *args0_op;

	nest_پیغام(">primary(%s)\n", TOKSTR[n]);
	اگر (n == EOI) {
		syntax(تہی, "argument expected");
	}
	اگر (n == LPAREN) {
		res = oexpr(check_operator(*++دلائل));
		اگر (check_operator(*++دلائل) != RPAREN)
			syntax(تہی, "closing paren expected");
		unnest_پیغام("<primary:%lld\n", res);
		واپس res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = آخری_operator;
	/* last_operator = operator at args[1] */
	اگر (check_operator(دلائل[1]) != EOI) { /* if args[1] != NULL */
		اگر (دلائل[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			اگر (آخری_operator->op_type == BINOP)
				unnest_پیغام_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	اگر (args0_op->op_type == UNOP) {
		/* unary expression */
		اگر (دلائل[1] == تہی)
//			syntax(args0_op->op_text, "argument expected");
			جاؤ check_emptiness;
		دلائل++;
		اگر (n == STREZ)
			unnest_پیغام_and_return(دلائل[0][0] == '\0', "<primary:%lld\n");
		اگر (n == STRNZ)
			unnest_پیغام_and_return(دلائل[0][0] != '\0', "<primary:%lld\n");
		اگر (n == FILTT)
			unnest_پیغام_and_return(isatty(getn(*دلائل)), "<primary: isatty(%s)%lld\n", *دلائل);
		unnest_پیغام_and_return(filstat(*دلائل, n), "<primary: filstat(%s):%lld\n", *دلائل);
	}

	/*check_operator(args[1]); - already done */
	اگر (آخری_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_پیغام_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_پیغام_and_return(دلائل[0][0] != '\0', "<primary:%lld\n");
}


صحیح test_main(صحیح argc, حرف **argv)
{
	صحیح res;
	ثابت حرف *arg0;
#اگر BASH_TEST2
	bool bt2 = 0;
#اگر_ختم

	arg0 = bb_basename(argv[0]);
	اگر ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		اگر (!arg0[1]) { /* "[" ? */
			اگر (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_پیغام("missing ]");
				واپس 2;
			}
		} ورنہ { /* assuming "[[" */
			اگر (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_پیغام("missing ]]");
				واپس 2;
			}
#اگر BASH_TEST2
			bt2 = 1;
#اگر_ختم
		}
		argv[argc] = تہی;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#اگر BASH_TEST2
	bash_test2 = bt2;
#اگر_ختم

	res = setjmp(leaving);
	اگر (res)
		جاؤ ret;

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
	دلائل = argv;

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
			جاؤ ret_special;
		}
		اگر (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			جاؤ ret_special;
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
				اگر (آخری_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					دلائل = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					جاؤ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				جاؤ check_negate;
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
					argv[3] = تہی;
					argv++;
				}
			}
		}
 check_negate:
		اگر (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			جاؤ again;
		}
	}

	res = !oexpr(check_operator(*دلائل));

	اگر (*دلائل != تہی && *++دلائل != تہی) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_پیغام("%s: unknown operand", *دلائل);
		res = 2;
	}
 ret:
	DEINIT_S();
	واپس res;
}
