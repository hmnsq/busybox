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

#define TEST_DEBUG 0

#إذا ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#وإلا
# define BASH_TEST2 0
#endif

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
#define is_int_op(a)      (((unsigned حرف)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned حرف)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned حرف)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned حرف)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned حرف)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned حرف)((a) - FILSUID)) <= 2)

#إذا TEST_DEBUG
صحيح depth;
#define nest_ڤسنن(...) افعل { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} طالما (0)
#define unnest_ڤسنن(...) افعل { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} طالما (0)
#define dbg_ڤسنن(...) افعل { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} طالما (0)
#define unnest_ڤسنن_and_return(اوڠکاڤن, ...) افعل { \
	نومبور_t __res = (اوڠکاڤن); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
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
#وإلا
#define nest_ڤسنن(...)   ((فراغ)0)
#define unnest_ڤسنن(...) ((فراغ)0)
#define dbg_ڤسنن(...)    ((فراغ)0)
#define unnest_ڤسنن_and_return(اوڠکاڤن, ...) أرجع اوڠکاڤن
#endif

تعداد {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

بنية operator_t {
	unsigned حرف op_اڠک, op_type;
};

ساكن ثابت بنية operator_t ops_table[] ALIGN2 = {
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
#إذا BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
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
#إذا BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#إذا ENABLE_FEATURE_TEST_64
تعريفنوع int64_t نومبور_t;
#وإلا
تعريفنوع صحيح نومبور_t;
#endif


/* We try to minimize both static and stack usage. */
بنية test_statics {
	حرف **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ثابت بنية operator_t *last_operator;
	gid_t *group_array;
	صحيح ngroups;
#إذا BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
خارجي بنية test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() افعل { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, حجم(S)); \
} طالما (0)
#define DEINIT_S() افعل { \
	حرر(group_array); \
	حرر(test_ptr_to_statics); \
} طالما (0)

ساكن نومبور_t primary(تعداد token n);

ساكن فراغ syntax(ثابت حرف *op, ثابت حرف *ڤسنن) NORETURN;
ساكن فراغ syntax(ثابت حرف *op, ثابت حرف *ڤسنن)
{
	إذا (op && *op) {
		bb_error_ڤسنن("%s: %s", op, ڤسنن);
	} وإلا {
		bb_error_ڤسنن("%s: %s"+4, ڤسنن);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
ساكن نومبور_t getn(ثابت حرف *s)
{
	حرف *p;
#إذا ENABLE_FEATURE_TEST_64
	long long r;
#وإلا
	long r;
#endif

	errno = 0;
#إذا ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#وإلا
	r = strtol(s, &p, 10);
#endif

	إذا (errno != 0)
		syntax(s, "out of range");

	إذا (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	أرجع r;
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


ساكن تعداد token check_operator(ثابت حرف *s)
{
	ساكن ثابت بنية operator_t no_op = {
		.op_اڠک = -1,
		.op_type = -1
	};
	صحيح n;

	last_operator = &no_op;
	إذا (s == عدم)
		أرجع EOI;
	n = index_in_strings(ops_texts, s);
	إذا (n < 0)
		أرجع OPERAND;

#إذا BASH_TEST2
	إذا (ops_table[n].op_اڠک == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		أرجع OPERAND;
	}
	إذا (ops_table[n].op_اڠک == BAND || ops_table[n].op_اڠک == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		إذا (bash_test2 == (s[0] == '-'))
			أرجع OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	أرجع ops_table[n].op_اڠک;
}


ساكن صحيح binop(فراغ)
{
	ثابت حرف *opnd1, *opnd2;
	ثابت بنية operator_t *op;
	نومبور_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	إذا (opnd2 == عدم)
		syntax(args[-1], "argument expected");

	إذا (is_int_op(op->op_اڠک)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		إذا (op->op_اڠک == INTEQ)
			أرجع val1 == val2;
		إذا (op->op_اڠک == INTNE)
			أرجع val1 != val2;
		إذا (op->op_اڠک == INTGE)
			أرجع val1 >= val2;
		إذا (op->op_اڠک == INTGT)
			أرجع val1 >  val2;
		إذا (op->op_اڠک == INTLE)
			أرجع val1 <= val2;
		/*if (op->op_num == INTLT)*/
		أرجع val1 <  val2;
	}
#إذا BASH_TEST2
	إذا (bash_test2) {
		إذا (op->op_اڠک == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			أرجع val1 == 0;
		}
		إذا (op->op_اڠک == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			أرجع val1 != 0;
		}
		إذا (op->op_اڠک == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, حجم(re_buffer));
			إذا (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, عدم, 0);
			regfree(&re_buffer);
			أرجع val1 == 0;
		}
	}
#endif
	إذا (is_str_op(op->op_اڠک)) {
		val1 = strcmp(opnd1, opnd2);
		إذا (op->op_اڠک == STREQ)
			أرجع val1 == 0;
		إذا (op->op_اڠک == STRNE)
			أرجع val1 != 0;
		إذا (op->op_اڠک == STRLT)
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
		إذا (op->op_اڠک == FILNT)
			أرجع b1.st_mtime > b2.st_mtime;
		إذا (op->op_اڠک == FILOT)
			أرجع b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		أرجع b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

ساكن فراغ initialize_group_array(فراغ)
{
	group_array = bb_getgroups(&ngroups, عدم);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
ساكن صحيح is_a_group_member(gid_t gid)
{
	صحيح i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	إذا (gid == getgid() || gid == getegid())
		أرجع 1;

	إذا (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	لكل (i = 0; i < ngroups; i++)
		إذا (gid == group_array[i])
			أرجع 1;

	أرجع 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
ساكن صحيح test_eaccess(بنية stat *st, صحيح mode)
{
	unsigned صحيح euid = geteuid();

	إذا (euid == 0) {
		/* Root can read or write any file. */
		إذا (mode != X_OK)
			أرجع 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		إذا (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			أرجع 0;
	}

	إذا (st->st_uid == euid)  /* owner */
		mode <<= 6;
	وإلا إذا (is_a_group_member(st->st_gid))
		mode <<= 3;

	إذا (st->st_mode & mode)
		أرجع 0;

	أرجع -1;
}


ساكن صحيح filstat(حرف *nm, تعداد token mode)
{
	بنية stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	إذا (mode == FILSYM) {
#ifdef S_IFLNK
		إذا (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			اذهب filetype;
		}
#endif
		أرجع 0;
	}

	إذا (stat(nm, &s) != 0)
		أرجع 0;
	إذا (mode == FILEXIST)
		أرجع 1;
	إذا (is_file_access(mode)) {
		إذا (mode == FILRD)
			i = R_OK;
		إذا (mode == FILWR)
			i = W_OK;
		إذا (mode == FILEX)
			i = X_OK;
		أرجع test_eaccess(&s, i) == 0;
	}
	إذا (is_file_type(mode)) {
		إذا (mode == FILREG)
			i = S_IFREG;
		إذا (mode == FILDIR)
			i = S_IFDIR;
		إذا (mode == FILCDEV)
			i = S_IFCHR;
		إذا (mode == FILBDEV)
			i = S_IFBLK;
		إذا (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#وإلا
			أرجع 0;
#endif
		}
		إذا (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#وإلا
			أرجع 0;
#endif
		}
 filetype:
		أرجع ((s.st_mode & S_IFMT) == i);
	}
	إذا (is_file_bit(mode)) {
		إذا (mode == FILSUID)
			i = S_ISUID;
		إذا (mode == FILSGID)
			i = S_ISGID;
		إذا (mode == FILSTCK)
			i = S_ISVTX;
		أرجع ((s.st_mode & i) != 0);
	}
	إذا (mode == FILGZ)
		أرجع s.st_size > 0L;
	إذا (mode == FILUID)
		أرجع s.st_uid == geteuid();
	إذا (mode == FILGID)
		أرجع s.st_gid == getegid();
	أرجع 1; /* NOTREACHED */
}


ساكن نومبور_t nexpr(تعداد token n)
{
	نومبور_t res;

	nest_ڤسنن(">nexpr(%s)\n", TOKSTR[n]);
	إذا (n == UNOT) {
		n = check_operator(*++args);
		إذا (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_ڤسنن("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			أرجع 1;
		}
		res = !nexpr(n);
		unnest_ڤسنن("<nexpr:%lld\n", res);
		أرجع res;
	}
	res = primary(n);
	unnest_ڤسنن("<nexpr:%lld\n", res);
	أرجع res;
}


ساكن نومبور_t aexpr(تعداد token n)
{
	نومبور_t res;

	nest_ڤسنن(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_ڤسنن("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	إذا (check_operator(*++args) == BAND) {
		dbg_ڤسنن("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_ڤسنن("<aexpr:%lld\n", res);
		أرجع res;
	}
	args--;
	unnest_ڤسنن("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	أرجع res;
}


ساكن نومبور_t oexpr(تعداد token n)
{
	نومبور_t res;

	nest_ڤسنن(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_ڤسنن("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	إذا (check_operator(*++args) == BOR) {
		dbg_ڤسنن("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_ڤسنن("<oexpr:%lld\n", res);
		أرجع res;
	}
	args--;
	unnest_ڤسنن("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	أرجع res;
}


ساكن نومبور_t primary(تعداد token n)
{
#إذا TEST_DEBUG
	نومبور_t res = res; /* for compiler */
#وإلا
	نومبور_t res;
#endif
	ثابت بنية operator_t *args0_op;

	nest_ڤسنن(">primary(%s)\n", TOKSTR[n]);
	إذا (n == EOI) {
		syntax(عدم, "argument expected");
	}
	إذا (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		إذا (check_operator(*++args) != RPAREN)
			syntax(عدم, "closing paren expected");
		unnest_ڤسنن("<primary:%lld\n", res);
		أرجع res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	إذا (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		إذا (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			إذا (last_operator->op_type == BINOP)
				unnest_ڤسنن_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	إذا (args0_op->op_type == UNOP) {
		/* unary expression */
		إذا (args[1] == عدم)
//			syntax(args0_op->op_text, "argument expected");
			اذهب check_emptiness;
		args++;
		إذا (n == STREZ)
			unnest_ڤسنن_and_return(args[0][0] == '\0', "<primary:%lld\n");
		إذا (n == STRNZ)
			unnest_ڤسنن_and_return(args[0][0] != '\0', "<primary:%lld\n");
		إذا (n == FILTT)
			unnest_ڤسنن_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_ڤسنن_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	إذا (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_ڤسنن_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_ڤسنن_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


صحيح test_main(صحيح argc, حرف **argv)
{
	صحيح res;
	ثابت حرف *arg0;
#إذا BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	إذا ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		إذا (!arg0[1]) { /* "[" ? */
			إذا (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_ڤسنن("missing ]");
				أرجع 2;
			}
		} وإلا { /* assuming "[[" */
			إذا (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_ڤسنن("missing ]]");
				أرجع 2;
			}
#إذا BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = عدم;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#إذا BASH_TEST2
	bash_test2 = bt2;
#endif

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
	args = argv;

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
				إذا (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
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

	res = !oexpr(check_operator(*args));

	إذا (*args != عدم && *++args != عدم) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_ڤسنن("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	أرجع res;
}
