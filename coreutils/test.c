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

#ضمن "libbb.h"
#ضمن <regex.h>
#ضمن <fnmatch.h>

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

#تعریف_کن TEST_DEBUG 0

#إذا ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#وإلا
# define BASH_TEST2 0
#پایان_اگر

تعداد token {
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

#إذا BASH_TEST2
	REGEX,
#پایان_اگر

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
	عملوند
};
#تعریف_کن is_int_op(a)      (((بیعلامت حرف)((a) - INTEQ)) <= 5)
#تعریف_کن is_str_op(a)      (((بیعلامت حرف)((a) - STREZ)) <= 5)
#تعریف_کن is_file_op(a)     (((بیعلامت حرف)((a) - FILNT)) <= 2)
#تعریف_کن is_file_access(a) (((بیعلامت حرف)((a) - FILRD)) <= 2)
#تعریف_کن is_file_type(a)   (((بیعلامت حرف)((a) - FILREG)) <= 5)
#تعریف_کن is_file_بیت(a)    (((بیعلامت حرف)((a) - FILSUID)) <= 2)

#إذا TEST_DEBUG
صحيح عمق;
#تعریف_کن nest_پیام(...) افعل { \
	عمق++; \
	fprintf(stderr, "%*s", عمق*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} طالما (0)
#تعریف_کن unnest_پیام(...) افعل { \
	fprintf(stderr, "%*s", عمق*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	عمق--; \
} طالما (0)
#تعریف_کن dbg_پیام(...) افعل { \
	fprintf(stderr, "%*s", عمق*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} طالما (0)
#تعریف_کن unnest_پیام_and_return(عبارت, ...) افعل { \
	عدد_t __res = (عبارت); \
	fprintf(stderr, "%*s", عمق*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	عمق--; \
	أرجع __res; \
} طالما (0)
ساكن ثابت حرف *ثابت TOKSTR[] ALIGN_PTR = {
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
#إذا BASH_TEST2
	"REGEX",
#پایان_اگر
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
#وإلا
#تعریف_کن nest_پیام(...)   ((فراغ)0)
#تعریف_کن unnest_پیام(...) ((فراغ)0)
#تعریف_کن dbg_پیام(...)    ((فراغ)0)
#تعریف_کن unnest_پیام_and_return(عبارت, ...) أرجع عبارت
#پایان_اگر

تعداد {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

بنية operator_t {
	بیعلامت حرف op_رقم, op_type;
};

ساكن ثابت بنية operator_t ops_جدول[] ALIGN2 = {
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
#إذا BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#پایان_اگر
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
#إذا BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#پایان_اگر
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
ساكن ثابت حرف ops_texts[] ALIGN1 =
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
#إذا BASH_TEST2
	"=~"  "\0"
#پایان_اگر
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
#إذا BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#پایان_اگر
	"("   "\0"
	")"   "\0"
;


#إذا ENABLE_FEATURE_TEST_64
تعريفنوع int64_t عدد_t;
#وإلا
تعريفنوع صحيح عدد_t;
#پایان_اگر


/* We try to minimize both static and stack usage. */
بنية test_statics {
	حرف **آرگومانها;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ثابت بنية operator_t *آخرین_operator;
	gid_t *گروه_array;
	صحيح ngroups;
#إذا BASH_TEST2
	bool bash_test2;
#پایان_اگر
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
خارجي بنية test_statics *BB_GLOBAL_CONST test_ptr_مقصد_statics;

#تعریف_کن S (*test_ptr_مقصد_statics)
#تعریف_کن آرگومانها            (S.آرگومانها         )
#تعریف_کن آخرین_operator   (S.آخرین_operator)
#تعریف_کن گروه_array     (S.گروه_array  )
#تعریف_کن ngroups         (S.ngroups      )
#تعریف_کن bash_test2      (S.bash_test2   )
#تعریف_کن leaving         (S.leaving      )

#تعریف_کن INIT_S() افعل { \
	XZALLOC_CONST_PTR(&test_ptr_مقصد_statics, حجم(S)); \
} طالما (0)
#تعریف_کن DEINIT_S() افعل { \
	حرر(گروه_array); \
	حرر(test_ptr_مقصد_statics); \
} طالما (0)

ساكن عدد_t primary(تعداد token n);

ساكن فراغ syntax(ثابت حرف *op, ثابت حرف *پیام) NORETURN;
ساكن فراغ syntax(ثابت حرف *op, ثابت حرف *پیام)
{
	إذا (op && *op) {
		bb_error_پیام("%s: %s", op, پیام);
	} وإلا {
		bb_error_پیام("%s: %s"+4, پیام);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ساكن عدد_t getn(ثابت حرف *s)
{
	حرف *p;
#إذا ENABLE_FEATURE_TEST_64
	دراز دراز r;
#وإلا
	دراز r;
#پایان_اگر

	errno = 0;
#إذا ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#وإلا
	r = strtol(s, &p, 10);
#پایان_اگر

	إذا (errno != 0)
		syntax(s, "out of range");

	إذا (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	أرجع r;
}

/*استفادهنشده
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


ساكن تعداد token check_operator(ثابت حرف *s)
{
	ساكن ثابت بنية operator_t no_op = {
		.op_رقم = -1,
		.op_type = -1
	};
	صحيح n;

	آخرین_operator = &no_op;
	إذا (s == عدم)
		أرجع EOI;
	n = شاخص_درون_رشتهها(ops_texts, s);
	إذا (n < 0)
		أرجع عملوند;

#إذا BASH_TEST2
	إذا (ops_جدول[n].op_رقم == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		أرجع عملوند;
	}
	إذا (ops_جدول[n].op_رقم == BAND || ops_جدول[n].op_رقم == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		إذا (bash_test2 == (s[0] == '-'))
			أرجع عملوند;
	}
#پایان_اگر

	آخرین_operator = &ops_جدول[n];
	أرجع ops_جدول[n].op_رقم;
}


ساكن صحيح binop(فراغ)
{
	ثابت حرف *opnd1, *opnd2;
	ثابت بنية operator_t *op;
	عدد_t val1, val2;

	opnd1 = *آرگومانها;
	check_operator(*++آرگومانها);
	op = آخرین_operator;

	opnd2 = *++آرگومانها;
	إذا (opnd2 == عدم)
		syntax(آرگومانها[-1], "argument expected");

	إذا (is_int_op(op->op_رقم)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		إذا (op->op_رقم == INTEQ)
			أرجع val1 == val2;
		إذا (op->op_رقم == INTNE)
			أرجع val1 != val2;
		إذا (op->op_رقم == INTGE)
			أرجع val1 >= val2;
		إذا (op->op_رقم == INTGT)
			أرجع val1 >  val2;
		إذا (op->op_رقم == INTLE)
			أرجع val1 <= val2;
		/*if (op->op_num == INTLT)*/
		أرجع val1 <  val2;
	}
#إذا BASH_TEST2
	إذا (bash_test2) {
		إذا (op->op_رقم == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			أرجع val1 == 0;
		}
		إذا (op->op_رقم == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			أرجع val1 != 0;
		}
		إذا (op->op_رقم == REGEX) {
			regex_t re_میانگیر;
			memset(&re_میانگیر, 0, حجم(re_میانگیر));
			إذا (regcomp(&re_میانگیر, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_میانگیر, opnd1, 0, عدم, 0);
			regfree(&re_میانگیر);
			أرجع val1 == 0;
		}
	}
#پایان_اگر
	إذا (is_str_op(op->op_رقم)) {
		val1 = strcmp(opnd1, opnd2);
		إذا (op->op_رقم == STREQ)
			أرجع val1 == 0;
		إذا (op->op_رقم == STRNE)
			أرجع val1 != 0;
		إذا (op->op_رقم == STRLT)
			أرجع val1 < 0;
		/*if (op->op_num == STRGT)*/
		أرجع val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		بنية stat b1, b2;

		إذا (stat(opnd1, &b1) || stat(opnd2, &b2))
			أرجع 0; /* false, since at least one stat failed */
		إذا (op->op_رقم == FILNT)
			أرجع b1.st_mtime > b2.st_mtime;
		إذا (op->op_رقم == FILOT)
			أرجع b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		أرجع b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ساكن فراغ initialize_گروه_array(فراغ)
{
	گروه_array = bb_getgroups(&ngroups, عدم);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ساكن صحيح is_a_گروه_member(gid_t gid)
{
	صحيح i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	إذا (gid == getgid() || gid == getegid())
		أرجع 1;

	إذا (ngroups == 0)
		initialize_گروه_array();

	/* Search through the list looking for GID. */
	لكل (i = 0; i < ngroups; i++)
		إذا (gid == گروه_array[i])
			أرجع 1;

	أرجع 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ساكن صحيح test_eaccess(بنية stat *st, صحيح نما)
{
	بیعلامت صحيح euid = geteuid();

	إذا (euid == 0) {
		/* Root can read or write any file. */
		إذا (نما != X_OK)
			أرجع 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		إذا (st->st_نما & (S_IXUSR | S_IXGRP | S_IXOTH))
			أرجع 0;
	}

	إذا (st->st_uid == euid)  /* owner */
		نما <<= 6;
	وإلا إذا (is_a_گروه_member(st->st_gid))
		نما <<= 3;

	إذا (st->st_نما & نما)
		أرجع 0;

	أرجع -1;
}


ساكن صحيح filstat(حرف *nm, تعداد token نما)
{
	بنية stat s;
	بیعلامت i = i; /* gcc 3.x thinks it can be used uninitialized */

	إذا (نما == FILSYM) {
#اگر_تعریف_شده S_IFLNK
		إذا (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			اذهب filetype;
		}
#پایان_اگر
		أرجع 0;
	}

	إذا (stat(nm, &s) != 0)
		أرجع 0;
	إذا (نما == FILEXIST)
		أرجع 1;
	إذا (is_file_access(نما)) {
		إذا (نما == FILRD)
			i = R_OK;
		إذا (نما == FILWR)
			i = W_OK;
		إذا (نما == FILEX)
			i = X_OK;
		أرجع test_eaccess(&s, i) == 0;
	}
	إذا (is_file_type(نما)) {
		إذا (نما == FILREG)
			i = S_IFREG;
		إذا (نما == FILDIR)
			i = S_IFDIR;
		إذا (نما == FILCDEV)
			i = S_IFCHR;
		إذا (نما == FILBDEV)
			i = S_IFBLK;
		إذا (نما == FILFIFO) {
#اگر_تعریف_شده S_IFIFO
			i = S_IFIFO;
#وإلا
			أرجع 0;
#پایان_اگر
		}
		إذا (نما == FILSOCK) {
#اگر_تعریف_شده S_IFSOCK
			i = S_IFSOCK;
#وإلا
			أرجع 0;
#پایان_اگر
		}
 filetype:
		أرجع ((s.st_نما & S_IFMT) == i);
	}
	إذا (is_file_بیت(نما)) {
		إذا (نما == FILSUID)
			i = S_ISUID;
		إذا (نما == FILSGID)
			i = S_ISGID;
		إذا (نما == FILSTCK)
			i = S_ISVTX;
		أرجع ((s.st_نما & i) != 0);
	}
	إذا (نما == FILGZ)
		أرجع s.st_اندازه > 0L;
	إذا (نما == FILUID)
		أرجع s.st_uid == geteuid();
	إذا (نما == FILGID)
		أرجع s.st_gid == getegid();
	أرجع 1; /* NOTREACHED */
}


ساكن عدد_t nexpr(تعداد token n)
{
	عدد_t res;

	nest_پیام(">nexpr(%s)\n", TOKSTR[n]);
	إذا (n == UNOT) {
		n = check_operator(*++آرگومانها);
		إذا (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			آرگومانها--;
			unnest_پیام("<nexpr:1 (!EOI), args:%s(%p)\n", آرگومانها[0], &آرگومانها[0]);
			أرجع 1;
		}
		res = !nexpr(n);
		unnest_پیام("<nexpr:%lld\n", res);
		أرجع res;
	}
	res = primary(n);
	unnest_پیام("<nexpr:%lld\n", res);
	أرجع res;
}


ساكن عدد_t aexpr(تعداد token n)
{
	عدد_t res;

	nest_پیام(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_پیام("aexpr: nexpr:%lld, next args:%s(%p)\n", res, آرگومانها[1], &آرگومانها[1]);
	إذا (check_operator(*++آرگومانها) == BAND) {
		dbg_پیام("aexpr: arg is AND, next args:%s(%p)\n", آرگومانها[1], &آرگومانها[1]);
		res = aexpr(check_operator(*++آرگومانها)) && res;
		unnest_پیام("<aexpr:%lld\n", res);
		أرجع res;
	}
	آرگومانها--;
	unnest_پیام("<aexpr:%lld, args:%s(%p)\n", res, آرگومانها[0], &آرگومانها[0]);
	أرجع res;
}


ساكن عدد_t oexpr(تعداد token n)
{
	عدد_t res;

	nest_پیام(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_پیام("oexpr: aexpr:%lld, next args:%s(%p)\n", res, آرگومانها[1], &آرگومانها[1]);
	إذا (check_operator(*++آرگومانها) == BOR) {
		dbg_پیام("oexpr: next arg is OR, next args:%s(%p)\n", آرگومانها[1], &آرگومانها[1]);
		res = oexpr(check_operator(*++آرگومانها)) || res;
		unnest_پیام("<oexpr:%lld\n", res);
		أرجع res;
	}
	آرگومانها--;
	unnest_پیام("<oexpr:%lld, args:%s(%p)\n", res, آرگومانها[0], &آرگومانها[0]);
	أرجع res;
}


ساكن عدد_t primary(تعداد token n)
{
#إذا TEST_DEBUG
	عدد_t res = res; /* for compiler */
#وإلا
	عدد_t res;
#پایان_اگر
	ثابت بنية operator_t *args0_op;

	nest_پیام(">primary(%s)\n", TOKSTR[n]);
	إذا (n == EOI) {
		syntax(عدم, "argument expected");
	}
	إذا (n == LPAREN) {
		res = oexpr(check_operator(*++آرگومانها));
		إذا (check_operator(*++آرگومانها) != RPAREN)
			syntax(عدم, "closing paren expected");
		unnest_پیام("<primary:%lld\n", res);
		أرجع res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = آخرین_operator;
	/* last_operator = operator at args[1] */
	إذا (check_operator(آرگومانها[1]) != EOI) { /* if args[1] != NULL */
		إذا (آرگومانها[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			إذا (آخرین_operator->op_type == BINOP)
				unnest_پیام_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	إذا (args0_op->op_type == UNOP) {
		/* unary expression */
		إذا (آرگومانها[1] == عدم)
//			syntax(args0_op->op_text, "argument expected");
			اذهب check_emptiness;
		آرگومانها++;
		إذا (n == STREZ)
			unnest_پیام_and_return(آرگومانها[0][0] == '\0', "<primary:%lld\n");
		إذا (n == STRNZ)
			unnest_پیام_and_return(آرگومانها[0][0] != '\0', "<primary:%lld\n");
		إذا (n == FILTT)
			unnest_پیام_and_return(isatty(getn(*آرگومانها)), "<primary: isatty(%s)%lld\n", *آرگومانها);
		unnest_پیام_and_return(filstat(*آرگومانها, n), "<primary: filstat(%s):%lld\n", *آرگومانها);
	}

	/*check_operator(args[1]); - already done */
	إذا (آخرین_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_پیام_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_پیام_and_return(آرگومانها[0][0] != '\0', "<primary:%lld\n");
}


صحيح test_main(صحيح argc, حرف **argv)
{
	صحيح res;
	ثابت حرف *arg0;
#إذا BASH_TEST2
	bool bt2 = 0;
#پایان_اگر

	arg0 = bb_basename(argv[0]);
	إذا ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		إذا (!arg0[1]) { /* "[" ? */
			إذا (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_پیام("missing ]");
				أرجع 2;
			}
		} وإلا { /* assuming "[[" */
			إذا (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_پیام("missing ]]");
				أرجع 2;
			}
#إذا BASH_TEST2
			bt2 = 1;
#پایان_اگر
		}
		argv[argc] = عدم;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#إذا BASH_TEST2
	bash_test2 = bt2;
#پایان_اگر

	res = setjmp(leaving);
	إذا (res)
		اذهب ret;

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
	آرگومانها = argv;

	/* Implement special cases from POSIX.2, section 4.62.4.
	 * Testcase: "test '(' = '('"
	 * The general parser would misinterpret '(' as group start.
	 */
	إذا (1) {
		صحيح negate = 0;
 again:
		إذا (!argv[0]) {
			/* "test" */
			res = 1;
			اذهب ret_special;
		}
		إذا (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			اذهب ret_special;
		}
		إذا (argv[2]) {
			إذا (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				إذا (آخرین_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					آرگومانها = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					اذهب ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				اذهب check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			إذا (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				إذا (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = عدم;
					argv++;
				}
			}
		}
 check_negate:
		إذا (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			اذهب again;
		}
	}

	res = !oexpr(check_operator(*آرگومانها));

	إذا (*آرگومانها != عدم && *++آرگومانها != عدم) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_پیام("%s: unknown operand", *آرگومانها);
		res = 2;
	}
 ret:
	DEINIT_S();
	أرجع res;
}
