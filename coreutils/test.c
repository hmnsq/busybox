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

#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ <regex.h>
#અન્તર્ભાવ <fnmatch.h>

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

#જો ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#નહીંતર
# define BASH_TEST2 0
#endif

ગણના token {
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

#જો BASH_TEST2
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
#define is_int_op(a)      (((unsigned અક્ષર)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned અક્ષર)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned અક્ષર)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned અક્ષર)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned અક્ષર)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned અક્ષર)((a) - FILSUID)) <= 2)

#જો TEST_DEBUG
પૂર્ણાંક depth;
#define nest_સંદેશ(...) કરો { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} જ્યારે (0)
#define unnest_સંદેશ(...) કરો { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} જ્યારે (0)
#define dbg_સંદેશ(...) કરો { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} જ્યારે (0)
#define unnest_સંદેશ_and_return(પદાવલિ, ...) કરો { \
	સંખ્યા_t __res = (પદાવલિ); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	પાછા __res; \
} જ્યારે (0)
સ્થિર અચલ અક્ષર *અચલ TOKSTR[] ALIGN_PTR = {
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
#જો BASH_TEST2
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
#નહીંતર
#define nest_સંદેશ(...)   ((શૂન્ય)0)
#define unnest_સંદેશ(...) ((શૂન્ય)0)
#define dbg_સંદેશ(...)    ((શૂન્ય)0)
#define unnest_સંદેશ_and_return(પદાવલિ, ...) પાછા પદાવલિ
#endif

ગણના {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

સંરચના operator_t {
	unsigned અક્ષર op_અંક, op_type;
};

સ્થિર અચલ સંરચના operator_t ops_table[] ALIGN2 = {
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
#જો BASH_TEST2
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
#જો BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
સ્થિર અચલ અક્ષર ops_texts[] ALIGN1 =
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
#જો BASH_TEST2
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
#જો BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#જો ENABLE_FEATURE_TEST_64
પ્રકારવ્યાખ્યા int64_t સંખ્યા_t;
#નહીંતર
પ્રકારવ્યાખ્યા પૂર્ણાંક સંખ્યા_t;
#endif


/* We try to minimize both static and stack usage. */
સંરચના test_statics {
	અક્ષર **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	અચલ સંરચના operator_t *last_operator;
	gid_t *group_array;
	પૂર્ણાંક ngroups;
#જો BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
બાહ્ય સંરચના test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() કરો { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, કદ(S)); \
} જ્યારે (0)
#define DEINIT_S() કરો { \
	મુક્ત(group_array); \
	મુક્ત(test_ptr_to_statics); \
} જ્યારે (0)

સ્થિર સંખ્યા_t primary(ગણના token n);

સ્થિર શૂન્ય syntax(અચલ અક્ષર *op, અચલ અક્ષર *સંદેશ) NORETURN;
સ્થિર શૂન્ય syntax(અચલ અક્ષર *op, અચલ અક્ષર *સંદેશ)
{
	જો (op && *op) {
		bb_error_સંદેશ("%s: %s", op, સંદેશ);
	} નહીંતર {
		bb_error_સંદેશ("%s: %s"+4, સંદેશ);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
સ્થિર સંખ્યા_t getn(અચલ અક્ષર *s)
{
	અક્ષર *p;
#જો ENABLE_FEATURE_TEST_64
	long long r;
#નહીંતર
	long r;
#endif

	errno = 0;
#જો ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#નહીંતર
	r = strtol(s, &p, 10);
#endif

	જો (errno != 0)
		syntax(s, "out of range");

	જો (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	પાછા r;
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


સ્થિર ગણના token check_operator(અચલ અક્ષર *s)
{
	સ્થિર અચલ સંરચના operator_t no_op = {
		.op_અંક = -1,
		.op_type = -1
	};
	પૂર્ણાંક n;

	last_operator = &no_op;
	જો (s == ખાલી)
		પાછા EOI;
	n = index_in_strings(ops_texts, s);
	જો (n < 0)
		પાછા OPERAND;

#જો BASH_TEST2
	જો (ops_table[n].op_અંક == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		પાછા OPERAND;
	}
	જો (ops_table[n].op_અંક == BAND || ops_table[n].op_અંક == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		જો (bash_test2 == (s[0] == '-'))
			પાછા OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	પાછા ops_table[n].op_અંક;
}


સ્થિર પૂર્ણાંક binop(શૂન્ય)
{
	અચલ અક્ષર *opnd1, *opnd2;
	અચલ સંરચના operator_t *op;
	સંખ્યા_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	જો (opnd2 == ખાલી)
		syntax(args[-1], "argument expected");

	જો (is_int_op(op->op_અંક)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		જો (op->op_અંક == INTEQ)
			પાછા val1 == val2;
		જો (op->op_અંક == INTNE)
			પાછા val1 != val2;
		જો (op->op_અંક == INTGE)
			પાછા val1 >= val2;
		જો (op->op_અંક == INTGT)
			પાછા val1 >  val2;
		જો (op->op_અંક == INTLE)
			પાછા val1 <= val2;
		/*if (op->op_num == INTLT)*/
		પાછા val1 <  val2;
	}
#જો BASH_TEST2
	જો (bash_test2) {
		જો (op->op_અંક == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			પાછા val1 == 0;
		}
		જો (op->op_અંક == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			પાછા val1 != 0;
		}
		જો (op->op_અંક == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, કદ(re_buffer));
			જો (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, ખાલી, 0);
			regfree(&re_buffer);
			પાછા val1 == 0;
		}
	}
#endif
	જો (is_str_op(op->op_અંક)) {
		val1 = strcmp(opnd1, opnd2);
		જો (op->op_અંક == STREQ)
			પાછા val1 == 0;
		જો (op->op_અંક == STRNE)
			પાછા val1 != 0;
		જો (op->op_અંક == STRLT)
			પાછા val1 < 0;
		/*if (op->op_num == STRGT)*/
		પાછા val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		સંરચના stat b1, b2;

		જો (stat(opnd1, &b1) || stat(opnd2, &b2))
			પાછા 0; /* false, since at least one stat failed */
		જો (op->op_અંક == FILNT)
			પાછા b1.st_mtime > b2.st_mtime;
		જો (op->op_અંક == FILOT)
			પાછા b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		પાછા b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

સ્થિર શૂન્ય initialize_group_array(શૂન્ય)
{
	group_array = bb_getgroups(&ngroups, ખાલી);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
સ્થિર પૂર્ણાંક is_a_group_member(gid_t gid)
{
	પૂર્ણાંક i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	જો (gid == getgid() || gid == getegid())
		પાછા 1;

	જો (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	માટે (i = 0; i < ngroups; i++)
		જો (gid == group_array[i])
			પાછા 1;

	પાછા 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
સ્થિર પૂર્ણાંક test_eaccess(સંરચના stat *st, પૂર્ણાંક mode)
{
	unsigned પૂર્ણાંક euid = geteuid();

	જો (euid == 0) {
		/* Root can read or write any file. */
		જો (mode != X_OK)
			પાછા 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		જો (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			પાછા 0;
	}

	જો (st->st_uid == euid)  /* owner */
		mode <<= 6;
	નહીંતર જો (is_a_group_member(st->st_gid))
		mode <<= 3;

	જો (st->st_mode & mode)
		પાછા 0;

	પાછા -1;
}


સ્થિર પૂર્ણાંક filstat(અક્ષર *nm, ગણના token mode)
{
	સંરચના stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	જો (mode == FILSYM) {
#ifdef S_IFLNK
		જો (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			જાઓ filetype;
		}
#endif
		પાછા 0;
	}

	જો (stat(nm, &s) != 0)
		પાછા 0;
	જો (mode == FILEXIST)
		પાછા 1;
	જો (is_file_access(mode)) {
		જો (mode == FILRD)
			i = R_OK;
		જો (mode == FILWR)
			i = W_OK;
		જો (mode == FILEX)
			i = X_OK;
		પાછા test_eaccess(&s, i) == 0;
	}
	જો (is_file_type(mode)) {
		જો (mode == FILREG)
			i = S_IFREG;
		જો (mode == FILDIR)
			i = S_IFDIR;
		જો (mode == FILCDEV)
			i = S_IFCHR;
		જો (mode == FILBDEV)
			i = S_IFBLK;
		જો (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#નહીંતર
			પાછા 0;
#endif
		}
		જો (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#નહીંતર
			પાછા 0;
#endif
		}
 filetype:
		પાછા ((s.st_mode & S_IFMT) == i);
	}
	જો (is_file_bit(mode)) {
		જો (mode == FILSUID)
			i = S_ISUID;
		જો (mode == FILSGID)
			i = S_ISGID;
		જો (mode == FILSTCK)
			i = S_ISVTX;
		પાછા ((s.st_mode & i) != 0);
	}
	જો (mode == FILGZ)
		પાછા s.st_size > 0L;
	જો (mode == FILUID)
		પાછા s.st_uid == geteuid();
	જો (mode == FILGID)
		પાછા s.st_gid == getegid();
	પાછા 1; /* NOTREACHED */
}


સ્થિર સંખ્યા_t nexpr(ગણના token n)
{
	સંખ્યા_t res;

	nest_સંદેશ(">nexpr(%s)\n", TOKSTR[n]);
	જો (n == UNOT) {
		n = check_operator(*++args);
		જો (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_સંદેશ("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			પાછા 1;
		}
		res = !nexpr(n);
		unnest_સંદેશ("<nexpr:%lld\n", res);
		પાછા res;
	}
	res = primary(n);
	unnest_સંદેશ("<nexpr:%lld\n", res);
	પાછા res;
}


સ્થિર સંખ્યા_t aexpr(ગણના token n)
{
	સંખ્યા_t res;

	nest_સંદેશ(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_સંદેશ("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	જો (check_operator(*++args) == BAND) {
		dbg_સંદેશ("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_સંદેશ("<aexpr:%lld\n", res);
		પાછા res;
	}
	args--;
	unnest_સંદેશ("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	પાછા res;
}


સ્થિર સંખ્યા_t oexpr(ગણના token n)
{
	સંખ્યા_t res;

	nest_સંદેશ(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_સંદેશ("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	જો (check_operator(*++args) == BOR) {
		dbg_સંદેશ("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_સંદેશ("<oexpr:%lld\n", res);
		પાછા res;
	}
	args--;
	unnest_સંદેશ("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	પાછા res;
}


સ્થિર સંખ્યા_t primary(ગણના token n)
{
#જો TEST_DEBUG
	સંખ્યા_t res = res; /* for compiler */
#નહીંતર
	સંખ્યા_t res;
#endif
	અચલ સંરચના operator_t *args0_op;

	nest_સંદેશ(">primary(%s)\n", TOKSTR[n]);
	જો (n == EOI) {
		syntax(ખાલી, "argument expected");
	}
	જો (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		જો (check_operator(*++args) != RPAREN)
			syntax(ખાલી, "closing paren expected");
		unnest_સંદેશ("<primary:%lld\n", res);
		પાછા res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	જો (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		જો (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			જો (last_operator->op_type == BINOP)
				unnest_સંદેશ_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	જો (args0_op->op_type == UNOP) {
		/* unary expression */
		જો (args[1] == ખાલી)
//			syntax(args0_op->op_text, "argument expected");
			જાઓ check_emptiness;
		args++;
		જો (n == STREZ)
			unnest_સંદેશ_and_return(args[0][0] == '\0', "<primary:%lld\n");
		જો (n == STRNZ)
			unnest_સંદેશ_and_return(args[0][0] != '\0', "<primary:%lld\n");
		જો (n == FILTT)
			unnest_સંદેશ_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_સંદેશ_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	જો (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_સંદેશ_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_સંદેશ_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


પૂર્ણાંક test_main(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક res;
	અચલ અક્ષર *arg0;
#જો BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	જો ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		જો (!arg0[1]) { /* "[" ? */
			જો (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_સંદેશ("missing ]");
				પાછા 2;
			}
		} નહીંતર { /* assuming "[[" */
			જો (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_સંદેશ("missing ]]");
				પાછા 2;
			}
#જો BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = ખાલી;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#જો BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	જો (res)
		જાઓ ret;

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
	જો (1) {
		પૂર્ણાંક negate = 0;
 again:
		જો (!argv[0]) {
			/* "test" */
			res = 1;
			જાઓ ret_special;
		}
		જો (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			જાઓ ret_special;
		}
		જો (argv[2]) {
			જો (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				જો (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					જાઓ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				જાઓ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			જો (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				જો (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ખાલી;
					argv++;
				}
			}
		}
 check_negate:
		જો (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			જાઓ again;
		}
	}

	res = !oexpr(check_operator(*args));

	જો (*args != ખાલી && *++args != ખાલી) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_સંદેશ("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	પાછા res;
}
