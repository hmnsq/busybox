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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ <regex.h>
#അന്തര്ഭാവ <fnmatch.h>

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

#എങ്കിൽ ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#അല്ലെങ്കിൽ
# define BASH_TEST2 0
#endif

എണ്ണൽ token {
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

#എങ്കിൽ BASH_TEST2
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
#define is_int_op(a)      (((unsigned അക്ഷരം)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((unsigned അക്ഷരം)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((unsigned അക്ഷരം)((a) - FILNT)) <= 2)
#define is_file_access(a) (((unsigned അക്ഷരം)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((unsigned അക്ഷരം)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((unsigned അക്ഷരം)((a) - FILSUID)) <= 2)

#എങ്കിൽ TEST_DEBUG
പൂർണ്ണസംഖ്യ depth;
#define nest_സന്ദേശം(...) ചെയ്യുക { \
	depth++; \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ആകുമ്പോൾ (0)
#define unnest_സന്ദേശം(...) ചെയ്യുക { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
	depth--; \
} ആകുമ്പോൾ (0)
#define dbg_സന്ദേശം(...) ചെയ്യുക { \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__); \
} ആകുമ്പോൾ (0)
#define unnest_സന്ദേശം_and_return(വ്യഞ്ജകം, ...) ചെയ്യുക { \
	സംഖ്യ_t __res = (വ്യഞ്ജകം); \
	fprintf(stderr, "%*s", depth*2, ""); \
	fprintf(stderr, __VA_ARGS__, res); \
	depth--; \
	തിരികെ __res; \
} ആകുമ്പോൾ (0)
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം TOKSTR[] ALIGN_PTR = {
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
#എങ്കിൽ BASH_TEST2
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
#അല്ലെങ്കിൽ
#define nest_സന്ദേശം(...)   ((ശൂന്യം)0)
#define unnest_സന്ദേശം(...) ((ശൂന്യം)0)
#define dbg_സന്ദേശം(...)    ((ശൂന്യം)0)
#define unnest_സന്ദേശം_and_return(വ്യഞ്ജകം, ...) തിരികെ വ്യഞ്ജകം
#endif

എണ്ണൽ {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

ഘടന operator_t {
	unsigned അക്ഷരം op_അക്കം, op_type;
};

സ്ഥിതം സ്ഥിരം ഘടന operator_t ops_table[] ALIGN2 = {
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
#എങ്കിൽ BASH_TEST2
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
#എങ്കിൽ BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
സ്ഥിതം സ്ഥിരം അക്ഷരം ops_texts[] ALIGN1 =
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
#എങ്കിൽ BASH_TEST2
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
#എങ്കിൽ BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#എങ്കിൽ ENABLE_FEATURE_TEST_64
തരനിർവചനം int64_t സംഖ്യ_t;
#അല്ലെങ്കിൽ
തരനിർവചനം പൂർണ്ണസംഖ്യ സംഖ്യ_t;
#endif


/* We try to minimize both static and stack usage. */
ഘടന test_statics {
	അക്ഷരം **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	സ്ഥിരം ഘടന operator_t *last_operator;
	gid_t *group_array;
	പൂർണ്ണസംഖ്യ ngroups;
#എങ്കിൽ BASH_TEST2
	bool bash_test2;
#endif
	jmp_buf leaving;
};

/* See test_ptr_hack.c */
ബാഹ്യ ഘടന test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#define S (*test_ptr_to_statics)
#define args            (S.args         )
#define last_operator   (S.last_operator)
#define group_array     (S.group_array  )
#define ngroups         (S.ngroups      )
#define bash_test2      (S.bash_test2   )
#define leaving         (S.leaving      )

#define INIT_S() ചെയ്യുക { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, വലുപ്പം(S)); \
} ആകുമ്പോൾ (0)
#define DEINIT_S() ചെയ്യുക { \
	സ്വതന്ത്രം(group_array); \
	സ്വതന്ത്രം(test_ptr_to_statics); \
} ആകുമ്പോൾ (0)

സ്ഥിതം സംഖ്യ_t primary(എണ്ണൽ token n);

സ്ഥിതം ശൂന്യം syntax(സ്ഥിരം അക്ഷരം *op, സ്ഥിരം അക്ഷരം *സന്ദേശം) NORETURN;
സ്ഥിതം ശൂന്യം syntax(സ്ഥിരം അക്ഷരം *op, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	എങ്കിൽ (op && *op) {
		bb_error_സന്ദേശം("%s: %s", op, സന്ദേശം);
	} അല്ലെങ്കിൽ {
		bb_error_സന്ദേശം("%s: %s"+4, സന്ദേശം);
	}
	longjmp(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
സ്ഥിതം സംഖ്യ_t getn(സ്ഥിരം അക്ഷരം *s)
{
	അക്ഷരം *p;
#എങ്കിൽ ENABLE_FEATURE_TEST_64
	long long r;
#അല്ലെങ്കിൽ
	long r;
#endif

	errno = 0;
#എങ്കിൽ ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#അല്ലെങ്കിൽ
	r = strtol(s, &p, 10);
#endif

	എങ്കിൽ (errno != 0)
		syntax(s, "out of range");

	എങ്കിൽ (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	തിരികെ r;
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


സ്ഥിതം എണ്ണൽ token check_operator(സ്ഥിരം അക്ഷരം *s)
{
	സ്ഥിതം സ്ഥിരം ഘടന operator_t no_op = {
		.op_അക്കം = -1,
		.op_type = -1
	};
	പൂർണ്ണസംഖ്യ n;

	last_operator = &no_op;
	എങ്കിൽ (s == ശൂന്യ)
		തിരികെ EOI;
	n = index_in_strings(ops_texts, s);
	എങ്കിൽ (n < 0)
		തിരികെ OPERAND;

#എങ്കിൽ BASH_TEST2
	എങ്കിൽ (ops_table[n].op_അക്കം == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		തിരികെ OPERAND;
	}
	എങ്കിൽ (ops_table[n].op_അക്കം == BAND || ops_table[n].op_അക്കം == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		എങ്കിൽ (bash_test2 == (s[0] == '-'))
			തിരികെ OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	തിരികെ ops_table[n].op_അക്കം;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ binop(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *opnd1, *opnd2;
	സ്ഥിരം ഘടന operator_t *op;
	സംഖ്യ_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	എങ്കിൽ (opnd2 == ശൂന്യ)
		syntax(args[-1], "argument expected");

	എങ്കിൽ (is_int_op(op->op_അക്കം)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		എങ്കിൽ (op->op_അക്കം == INTEQ)
			തിരികെ val1 == val2;
		എങ്കിൽ (op->op_അക്കം == INTNE)
			തിരികെ val1 != val2;
		എങ്കിൽ (op->op_അക്കം == INTGE)
			തിരികെ val1 >= val2;
		എങ്കിൽ (op->op_അക്കം == INTGT)
			തിരികെ val1 >  val2;
		എങ്കിൽ (op->op_അക്കം == INTLE)
			തിരികെ val1 <= val2;
		/*if (op->op_num == INTLT)*/
		തിരികെ val1 <  val2;
	}
#എങ്കിൽ BASH_TEST2
	എങ്കിൽ (bash_test2) {
		എങ്കിൽ (op->op_അക്കം == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			തിരികെ val1 == 0;
		}
		എങ്കിൽ (op->op_അക്കം == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			തിരികെ val1 != 0;
		}
		എങ്കിൽ (op->op_അക്കം == REGEX) {
			regex_t re_buffer;
			memset(&re_buffer, 0, വലുപ്പം(re_buffer));
			എങ്കിൽ (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				longjmp(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, ശൂന്യ, 0);
			regfree(&re_buffer);
			തിരികെ val1 == 0;
		}
	}
#endif
	എങ്കിൽ (is_str_op(op->op_അക്കം)) {
		val1 = strcmp(opnd1, opnd2);
		എങ്കിൽ (op->op_അക്കം == STREQ)
			തിരികെ val1 == 0;
		എങ്കിൽ (op->op_അക്കം == STRNE)
			തിരികെ val1 != 0;
		എങ്കിൽ (op->op_അക്കം == STRLT)
			തിരികെ val1 < 0;
		/*if (op->op_num == STRGT)*/
		തിരികെ val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		ഘടന stat b1, b2;

		എങ്കിൽ (stat(opnd1, &b1) || stat(opnd2, &b2))
			തിരികെ 0; /* false, since at least one stat failed */
		എങ്കിൽ (op->op_അക്കം == FILNT)
			തിരികെ b1.st_mtime > b2.st_mtime;
		എങ്കിൽ (op->op_അക്കം == FILOT)
			തിരികെ b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		തിരികെ b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

സ്ഥിതം ശൂന്യം initialize_group_array(ശൂന്യം)
{
	group_array = bb_getgroups(&ngroups, ശൂന്യ);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
സ്ഥിതം പൂർണ്ണസംഖ്യ is_a_group_member(gid_t gid)
{
	പൂർണ്ണസംഖ്യ i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	എങ്കിൽ (gid == getgid() || gid == getegid())
		തിരികെ 1;

	എങ്കിൽ (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	വേണ്ടി (i = 0; i < ngroups; i++)
		എങ്കിൽ (gid == group_array[i])
			തിരികെ 1;

	തിരികെ 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
സ്ഥിതം പൂർണ്ണസംഖ്യ test_eaccess(ഘടന stat *st, പൂർണ്ണസംഖ്യ mode)
{
	unsigned പൂർണ്ണസംഖ്യ euid = geteuid();

	എങ്കിൽ (euid == 0) {
		/* Root can read or write any file. */
		എങ്കിൽ (mode != X_OK)
			തിരികെ 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		എങ്കിൽ (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			തിരികെ 0;
	}

	എങ്കിൽ (st->st_uid == euid)  /* owner */
		mode <<= 6;
	അല്ലെങ്കിൽ എങ്കിൽ (is_a_group_member(st->st_gid))
		mode <<= 3;

	എങ്കിൽ (st->st_mode & mode)
		തിരികെ 0;

	തിരികെ -1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ filstat(അക്ഷരം *nm, എണ്ണൽ token mode)
{
	ഘടന stat s;
	unsigned i = i; /* gcc 3.x thinks it can be used uninitialized */

	എങ്കിൽ (mode == FILSYM) {
#ifdef S_IFLNK
		എങ്കിൽ (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			പോകുക filetype;
		}
#endif
		തിരികെ 0;
	}

	എങ്കിൽ (stat(nm, &s) != 0)
		തിരികെ 0;
	എങ്കിൽ (mode == FILEXIST)
		തിരികെ 1;
	എങ്കിൽ (is_file_access(mode)) {
		എങ്കിൽ (mode == FILRD)
			i = R_OK;
		എങ്കിൽ (mode == FILWR)
			i = W_OK;
		എങ്കിൽ (mode == FILEX)
			i = X_OK;
		തിരികെ test_eaccess(&s, i) == 0;
	}
	എങ്കിൽ (is_file_type(mode)) {
		എങ്കിൽ (mode == FILREG)
			i = S_IFREG;
		എങ്കിൽ (mode == FILDIR)
			i = S_IFDIR;
		എങ്കിൽ (mode == FILCDEV)
			i = S_IFCHR;
		എങ്കിൽ (mode == FILBDEV)
			i = S_IFBLK;
		എങ്കിൽ (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#അല്ലെങ്കിൽ
			തിരികെ 0;
#endif
		}
		എങ്കിൽ (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#അല്ലെങ്കിൽ
			തിരികെ 0;
#endif
		}
 filetype:
		തിരികെ ((s.st_mode & S_IFMT) == i);
	}
	എങ്കിൽ (is_file_bit(mode)) {
		എങ്കിൽ (mode == FILSUID)
			i = S_ISUID;
		എങ്കിൽ (mode == FILSGID)
			i = S_ISGID;
		എങ്കിൽ (mode == FILSTCK)
			i = S_ISVTX;
		തിരികെ ((s.st_mode & i) != 0);
	}
	എങ്കിൽ (mode == FILGZ)
		തിരികെ s.st_size > 0L;
	എങ്കിൽ (mode == FILUID)
		തിരികെ s.st_uid == geteuid();
	എങ്കിൽ (mode == FILGID)
		തിരികെ s.st_gid == getegid();
	തിരികെ 1; /* NOTREACHED */
}


സ്ഥിതം സംഖ്യ_t nexpr(എണ്ണൽ token n)
{
	സംഖ്യ_t res;

	nest_സന്ദേശം(">nexpr(%s)\n", TOKSTR[n]);
	എങ്കിൽ (n == UNOT) {
		n = check_operator(*++args);
		എങ്കിൽ (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_സന്ദേശം("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			തിരികെ 1;
		}
		res = !nexpr(n);
		unnest_സന്ദേശം("<nexpr:%lld\n", res);
		തിരികെ res;
	}
	res = primary(n);
	unnest_സന്ദേശം("<nexpr:%lld\n", res);
	തിരികെ res;
}


സ്ഥിതം സംഖ്യ_t aexpr(എണ്ണൽ token n)
{
	സംഖ്യ_t res;

	nest_സന്ദേശം(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_സന്ദേശം("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	എങ്കിൽ (check_operator(*++args) == BAND) {
		dbg_സന്ദേശം("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_സന്ദേശം("<aexpr:%lld\n", res);
		തിരികെ res;
	}
	args--;
	unnest_സന്ദേശം("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	തിരികെ res;
}


സ്ഥിതം സംഖ്യ_t oexpr(എണ്ണൽ token n)
{
	സംഖ്യ_t res;

	nest_സന്ദേശം(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_സന്ദേശം("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	എങ്കിൽ (check_operator(*++args) == BOR) {
		dbg_സന്ദേശം("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_സന്ദേശം("<oexpr:%lld\n", res);
		തിരികെ res;
	}
	args--;
	unnest_സന്ദേശം("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	തിരികെ res;
}


സ്ഥിതം സംഖ്യ_t primary(എണ്ണൽ token n)
{
#എങ്കിൽ TEST_DEBUG
	സംഖ്യ_t res = res; /* for compiler */
#അല്ലെങ്കിൽ
	സംഖ്യ_t res;
#endif
	സ്ഥിരം ഘടന operator_t *args0_op;

	nest_സന്ദേശം(">primary(%s)\n", TOKSTR[n]);
	എങ്കിൽ (n == EOI) {
		syntax(ശൂന്യ, "argument expected");
	}
	എങ്കിൽ (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		എങ്കിൽ (check_operator(*++args) != RPAREN)
			syntax(ശൂന്യ, "closing paren expected");
		unnest_സന്ദേശം("<primary:%lld\n", res);
		തിരികെ res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	എങ്കിൽ (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		എങ്കിൽ (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			എങ്കിൽ (last_operator->op_type == BINOP)
				unnest_സന്ദേശം_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	എങ്കിൽ (args0_op->op_type == UNOP) {
		/* unary expression */
		എങ്കിൽ (args[1] == ശൂന്യ)
//			syntax(args0_op->op_text, "argument expected");
			പോകുക check_emptiness;
		args++;
		എങ്കിൽ (n == STREZ)
			unnest_സന്ദേശം_and_return(args[0][0] == '\0', "<primary:%lld\n");
		എങ്കിൽ (n == STRNZ)
			unnest_സന്ദേശം_and_return(args[0][0] != '\0', "<primary:%lld\n");
		എങ്കിൽ (n == FILTT)
			unnest_സന്ദേശം_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_സന്ദേശം_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	എങ്കിൽ (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_സന്ദേശം_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_സന്ദേശം_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


പൂർണ്ണസംഖ്യ test_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ res;
	സ്ഥിരം അക്ഷരം *arg0;
#എങ്കിൽ BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	എങ്കിൽ ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		എങ്കിൽ (!arg0[1]) { /* "[" ? */
			എങ്കിൽ (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_സന്ദേശം("missing ]");
				തിരികെ 2;
			}
		} അല്ലെങ്കിൽ { /* assuming "[[" */
			എങ്കിൽ (strcmp(argv[argc], "]]") != 0) {
				bb_simple_error_സന്ദേശം("missing ]]");
				തിരികെ 2;
			}
#എങ്കിൽ BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = ശൂന്യ;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#എങ്കിൽ BASH_TEST2
	bash_test2 = bt2;
#endif

	res = setjmp(leaving);
	എങ്കിൽ (res)
		പോകുക ret;

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
	എങ്കിൽ (1) {
		പൂർണ്ണസംഖ്യ negate = 0;
 again:
		എങ്കിൽ (!argv[0]) {
			/* "test" */
			res = 1;
			പോകുക ret_special;
		}
		എങ്കിൽ (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			പോകുക ret_special;
		}
		എങ്കിൽ (argv[2]) {
			എങ്കിൽ (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				എങ്കിൽ (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					പോകുക ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				പോകുക check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			എങ്കിൽ (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				എങ്കിൽ (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = ശൂന്യ;
					argv++;
				}
			}
		}
 check_negate:
		എങ്കിൽ (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			പോകുക again;
		}
	}

	res = !oexpr(check_operator(*args));

	എങ്കിൽ (*args != ശൂന്യ && *++args != ശൂന്യ) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_സന്ദേശം("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	തിരികെ res;
}
