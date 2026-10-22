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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव <regex.h>
#अन्तर्भाव <fnmatch.h>

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

#जइ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#अण्णहा
# define BASH_TEST2 0
#endif

गणना token {
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

#जइ BASH_TEST2
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
#define is_int_op(a)      (((unsigned वर्ण)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned वर्ण)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned वर्ण)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned वर्ण)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned वर्ण)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned वर्ण)((a) - FILSUID)) <= 2)

#जइ TEST_DEBUG
पूर्णाङ्क depth;
#define nest_सन्देशः(...) करेहि { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} जाव (0)
#define unnest_सन्देशः(...) करेहि { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} जाव (0)
#define dbg_सन्देशः(...) करेहि { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} जाव (0)
#define unnest_सन्देशः_and_return(व्यञ्जकम्, ...) करेहि { \
	सङ्ख्या_t __res = (व्यञ्जकम्); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	प्रत्यागम __res; \
} जाव (0)
स्थिर ध्रुव वर्ण *ध्रुव TOKSTR[] ALIGN_PTR = {
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
#जइ BASH_TEST2
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
#अण्णहा
#define nest_सन्देशः(...)   ((शून्य)0)
#define unnest_सन्देशः(...) ((शून्य)0)
#define dbg_सन्देशः(...)    ((शून्य)0)
#define unnest_सन्देशः_and_return(व्यञ्जकम्, ...) प्रत्यागम व्यञ्जकम्
#endif

गणना {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

संरचना operator_t {
	unsigned वर्ण op_अङ्कः, op_type;
};

स्थिर ध्रुव संरचना operator_t ops_table[] ALIGN2 = {
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
#जइ BASH_TEST2
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
#जइ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
स्थिर ध्रुव वर्ण ops_texts[] ALIGN1 =
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
#जइ BASH_TEST2
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
#जइ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#जइ ENABLE_FEATURE_TEST_64
प्रकारनिर्देश int64_t सङ्ख्या_t;
#अण्णहा
प्रकारनिर्देश पूर्णाङ्क सङ्ख्या_t;
#endif


/* We try to minimize both static and stack usage. */
संरचना test_statics {
	वर्ण **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	ध्रुव संरचना operator_t *last_operator;
	gid_t *group_array;
	पूर्णाङ्क ngroups;
#जइ BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
बाह्य संरचना test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() करेहि { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, परिमाण(S)); \
} जाव (0)
#define DEINIT_S() करेहि { \
	मुक्त(group_array); \
	मुक्त(test_ptr_to_statics); \
} जाव (0)

स्थिर सङ्ख्या_t primary(गणना token n);

स्थिर शून्य syntax(ध्रुव वर्ण *op, ध्रुव वर्ण *सन्देशः) NORETURN;
स्थिर शून्य syntax(ध्रुव वर्ण *op, ध्रुव वर्ण *सन्देशः)
{
	जइ (op && *op) {
		bb_error_सन्देशः("%s: %s", op, सन्देशः);
	} अण्णहा {
		bb_error_सन्देशः("%s: %s"+4, सन्देशः);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
स्थिर सङ्ख्या_t getn(ध्रुव वर्ण *s)
{
	वर्ण *p;
#जइ ENABLE_FEATURE_TEST_64
	long long r;
#अण्णहा
	long r;
#endif

	errno = 0;
#जइ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#अण्णहा
	r = strtol(s, &p, 10);
#endif

	जइ (errno != 0)
		syntax(s, "out of range");

	जइ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	प्रत्यागम r;
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


स्थिर गणना token check_operator(ध्रुव वर्ण *s)
{
	स्थिर ध्रुव संरचना operator_t no_op = {
		.op_अङ्कः = -1,
		.op_type = -1
	};
	पूर्णाङ्क n;

	last_operator = &no_op;
	जइ (s == रिक्त)
		प्रत्यागम EOI;
	n = index_in_strings(ops_texts, s);
	जइ (n < 0)
		प्रत्यागम OPERAND;

#जइ BASH_TEST2
	जइ (ops_table[n].op_अङ्कः == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		प्रत्यागम OPERAND;
	}
	जइ (ops_table[n].op_अङ्कः == BAND || ops_table[n].op_अङ्कः == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		जइ (bash_test2 == (s[0] == '-'))
			प्रत्यागम OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	प्रत्यागम ops_table[n].op_अङ्कः;
}


स्थिर पूर्णाङ्क binop(शून्य)
{
	ध्रुव वर्ण *opnd1, *opnd2;
	ध्रुव संरचना operator_t *op;
	सङ्ख्या_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	जइ (opnd2 == रिक्त)
		syntax(args[-1], "argument expected");

	जइ (is_int_op(op->op_अङ्कः)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		जइ (op->op_अङ्कः == INTEQ)
			प्रत्यागम val1 == val2;
		जइ (op->op_अङ्कः == INTNE)
			प्रत्यागम val1 != val2;
		जइ (op->op_अङ्कः == INTGE)
			प्रत्यागम val1 >= val2;
		जइ (op->op_अङ्कः == INTGT)
			प्रत्यागम val1 >  val2;
		जइ (op->op_अङ्कः == INTLE)
			प्रत्यागम val1 <= val2;
		/*if (op->op_num == INTLT)*/
		प्रत्यागम val1 <  val2;
	}
#जइ BASH_TEST2
	जइ (bash_test2) {
		जइ (op->op_अङ्कः == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			प्रत्यागम val1 == 0;
		}
		जइ (op->op_अङ्कः == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			प्रत्यागम val1 != 0;
		}
		जइ (op->op_अङ्कः == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, परिमाण(re_buffer));
			जइ (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, रिक्त, 0);
			regfree(&re_buffer);
			प्रत्यागम val1 == 0;
		}
	}
#endif
	जइ (is_str_op(op->op_अङ्कः)) {
		val1 = strcmp(opnd1, opnd2);
		जइ (op->op_अङ्कः == STREQ)
			प्रत्यागम val1 == 0;
		जइ (op->op_अङ्कः == STRNE)
			प्रत्यागम val1 != 0;
		जइ (op->op_अङ्कः == STRLT)
			प्रत्यागम val1 < 0;
		/*if (op->op_num == STRGT)*/
		प्रत्यागम val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		संरचना stat b1, b2;

		जइ (stat(opnd1, &b1) || stat(opnd2, &b2))
			प्रत्यागम 0; /* false, since at least one stat failed */
		जइ (op->op_अङ्कः == FILNT)
			प्रत्यागम b1.st_mtime > b2.st_mtime;
		जइ (op->op_अङ्कः == FILOT)
			प्रत्यागम b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		प्रत्यागम b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

स्थिर शून्य initialize_group_array(शून्य)
{
	group_array = bb_getgroups(&ngroups, रिक्त);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
स्थिर पूर्णाङ्क is_a_group_member(gid_t gid)
{
	पूर्णाङ्क i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	जइ (gid == getgid() || gid == getegid())
		प्रत्यागम 1;

	जइ (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	प्रति (i = 0; i < ngroups; i++)
		जइ (gid == group_array[i])
			प्रत्यागम 1;

	प्रत्यागम 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
स्थिर पूर्णाङ्क test_eaccess(संरचना stat *st, पूर्णाङ्क mode)
{
	unsigned पूर्णाङ्क euid = geteuid();

	जइ (euid == 0) {
		/* Root can read or write any file. */
		जइ (mode != X_OK)
			प्रत्यागम 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		जइ (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			प्रत्यागम 0;
	}

	जइ (st->st_uid == euid)  /* owner */
		mode <<= 6;
	अण्णहा जइ (is_a_group_member(st->st_gid))
		mode <<= 3;

	जइ (st->st_mode & mode)
		प्रत्यागम 0;

	प्रत्यागम -1;
}


स्थिर पूर्णाङ्क filstat(वर्ण *nm, गणना token mode)
{
	संरचना stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	जइ (mode == FILSYM) {
#ifdef S_IFLNK
		जइ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			गच्छ filetype;
		}
#endif
		प्रत्यागम 0;
	}

	जइ (stat(nm, &s) != 0)
		प्रत्यागम 0;
	जइ (mode == FILEXIST)
		प्रत्यागम 1;
	जइ (is_file_access(mode)) {
		जइ (mode == FILRD)
			i = R_OK;
		जइ (mode == FILWR)
			i = W_OK;
		जइ (mode == FILEX)
			i = X_OK;
		प्रत्यागम test_eaccess(&s, i) == 0;
	}
	जइ (is_file_type(mode)) {
		जइ (mode == FILREG)
			i = S_IFREG;
		जइ (mode == FILDIR)
			i = S_IFDIR;
		जइ (mode == FILCDEV)
			i = S_IFCHR;
		जइ (mode == FILBDEV)
			i = S_IFBLK;
		जइ (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#अण्णहा
			प्रत्यागम 0;
#endif
		}
		जइ (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#अण्णहा
			प्रत्यागम 0;
#endif
		}
 filetype:
		प्रत्यागम ((s.st_mode & S_IFMT) == i);
	}
	जइ (is_file_bit(mode)) {
		जइ (mode == FILSUID)
			i = S_ISUID;
		जइ (mode == FILSGID)
			i = S_ISGID;
		जइ (mode == FILSTCK)
			i = S_ISVTX;
		प्रत्यागम ((s.st_mode & i) != 0);
	}
	जइ (mode == FILGZ)
		प्रत्यागम s.st_size > 0L;
	जइ (mode == FILUID)
		प्रत्यागम s.st_uid == geteuid();
	जइ (mode == FILGID)
		प्रत्यागम s.st_gid == getegid();
	प्रत्यागम 1; /* NOTREACHED */
}


स्थिर सङ्ख्या_t nexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">nexpr(%s)\n", TOKSTR[n]);
	जइ (n == UNOT) {
		n = check_operator(*++args);
		जइ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_सन्देशः("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			प्रत्यागम 1;
		}
		res = !nexpr(n);
		unnest_सन्देशः("<nexpr:%lld\n", res);
		प्रत्यागम res;
	}
	res = primary(n);
	unnest_सन्देशः("<nexpr:%lld\n", res);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t aexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_सन्देशः("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	जइ (check_operator(*++args) == BAND) {
		dbg_सन्देशः("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_सन्देशः("<aexpr:%lld\n", res);
		प्रत्यागम res;
	}
	args--;
	unnest_सन्देशः("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t oexpr(गणना token n)
{
	सङ्ख्या_t res;

	nest_सन्देशः(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_सन्देशः("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	जइ (check_operator(*++args) == BOR) {
		dbg_सन्देशः("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_सन्देशः("<oexpr:%lld\n", res);
		प्रत्यागम res;
	}
	args--;
	unnest_सन्देशः("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	प्रत्यागम res;
}


स्थिर सङ्ख्या_t primary(गणना token n)
{
#जइ TEST_DEBUG
	सङ्ख्या_t res = res; /* for compiler */
#अण्णहा
	सङ्ख्या_t res;
#endif
	ध्रुव संरचना operator_t *args0_op;

	nest_सन्देशः(">primary(%s)\n", TOKSTR[n]);
	जइ (n == EOI) {
		syntax(रिक्त, "argument expected");
	}
	जइ (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		जइ (check_operator(*++args) != RPAREN)
			syntax(रिक्त, "closing paren expected");
		unnest_सन्देशः("<primary:%lld\n", res);
		प्रत्यागम res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	जइ (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		जइ (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			जइ (last_operator->op_type == BINOP)
				unnest_सन्देशः_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	जइ (args0_op->op_type == UNOP) {
		/* unary expression */
		जइ (args[1] == रिक्त)
//			syntax(args0_op->op_text, "argument expected");
			गच्छ check_emptiness;
		args++;
		जइ (n == STREZ)
			unnest_सन्देशः_and_return(args[0][0] == '\0', "<primary:%lld\n");
		जइ (n == STRNZ)
			unnest_सन्देशः_and_return(args[0][0] != '\0', "<primary:%lld\n");
		जइ (n == FILTT)
			unnest_सन्देशः_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_सन्देशः_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	जइ (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_सन्देशः_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_सन्देशः_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


पूर्णाङ्क test_main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क res;
	ध्रुव वर्ण *arg0;
#जइ BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	जइ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		जइ (!arg0[1]) { /* "[" ? */
			जइ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_सन्देशः("missing ]");
				प्रत्यागम 2;
			}
		} अण्णहा { /* assuming "[[" */
			जइ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_सन्देशः("missing ]]");
				प्रत्यागम 2;
			}
#जइ BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = रिक्त;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#जइ BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	जइ (res)
		गच्छ ret;

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
	जइ (1) {
		पूर्णाङ्क negate = 0;
 again:
		जइ (!argv[0]) {
			/* "test" */
			res = 1;
			गच्छ ret_special;
		}
		जइ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			गच्छ ret_special;
		}
		जइ (argv[2]) {
			जइ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				जइ (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					गच्छ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				गच्छ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			जइ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				जइ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = रिक्त;
					argv++;
				}
			}
		}
 check_negate:
		जइ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			गच्छ again;
		}
	}

	res = !oexpr(check_operator(*args));

	जइ (*args != रिक्त && *++args != रिक्त) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_सन्देशः("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	प्रत्यागम res;
}
