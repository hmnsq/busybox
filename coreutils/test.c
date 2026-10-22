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

#जर ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#अन्यथा
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

#जर BASH_TEST2
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
#define is_int_op(a)      (((अचिन्हित अक्षर)((a) - INTEQ)) <= 5)
#define is_str_op(a)      (((अचिन्हित अक्षर)((a) - STREZ)) <= 5)
#define is_file_op(a)     (((अचिन्हित अक्षर)((a) - FILNT)) <= 2)
#define is_file_access(a) (((अचिन्हित अक्षर)((a) - FILRD)) <= 2)
#define is_file_type(a)   (((अचिन्हित अक्षर)((a) - FILREG)) <= 5)
#define is_file_bit(a)    (((अचिन्हित अक्षर)((a) - FILSUID)) <= 2)

#जर TEST_DEBUG
पूर्णांक depth;
#define nest_संदेश(...) करा { \
	depth++; \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
} पर्यंत (0)
#define unnest_संदेश(...) करा { \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
	depth--; \
} पर्यंत (0)
#define dbg_संदेश(...) करा { \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
} पर्यंत (0)
#define unnest_संदेश_and_return(अभिव्यक्ती, ...) करा { \
	संख्या_t __res = (अभिव्यक्ती); \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__, res); \
	depth--; \
	परत __res; \
} पर्यंत (0)
स्थिर स्थिरांक अक्षर *स्थिरांक TOKSTR[] ALIGN_PTR = {
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
#जर BASH_TEST2
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
#अन्यथा
#define nest_संदेश(...)   ((शून्य)0)
#define unnest_संदेश(...) ((शून्य)0)
#define dbg_संदेश(...)    ((शून्य)0)
#define unnest_संदेश_and_return(अभिव्यक्ती, ...) परत अभिव्यक्ती
#endif

गणना {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

संरचना operator_t {
	अचिन्हित अक्षर op_अंक, op_type;
};

स्थिर स्थिरांक संरचना operator_t ops_table[] ALIGN2 = {
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
#जर BASH_TEST2
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
#जर BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#endif
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
स्थिर स्थिरांक अक्षर ops_texts[] ALIGN1 =
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
#जर BASH_TEST2
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
#जर BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#endif
	"("   "\0"
	")"   "\0"
;


#जर ENABLE_FEATURE_TEST_64
प्रकारव्याख्या int64_t संख्या_t;
#अन्यथा
प्रकारव्याख्या पूर्णांक संख्या_t;
#endif


/* We try to minimize both static and stack usage. */
संरचना test_statics {
	अक्षर **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	स्थिरांक संरचना operator_t *last_operator;
	gid_t *group_array;
	पूर्णांक ngroups;
#जर BASH_TEST2
	bool bash_test2;
#endif
	लाँघ_बफ leaving;
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

#define INIT_S() करा { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, आकार(S)); \
} पर्यंत (0)
#define DEINIT_S() करा { \
	मुक्त(group_array); \
	मुक्त(test_ptr_to_statics); \
} पर्यंत (0)

स्थिर संख्या_t primary(गणना token n);

स्थिर शून्य syntax(स्थिरांक अक्षर *op, स्थिरांक अक्षर *संदेश) NORETURN;
स्थिर शून्य syntax(स्थिरांक अक्षर *op, स्थिरांक अक्षर *संदेश)
{
	जर (op && *op) {
		bb_error_संदेश("%s: %s", op, संदेश);
	} अन्यथा {
		bb_error_संदेश("%s: %s"+4, संदेश);
	}
	दीर्घ_लाँघ(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
स्थिर संख्या_t getn(स्थिरांक अक्षर *s)
{
	अक्षर *p;
#जर ENABLE_FEATURE_TEST_64
	दीर्घ दीर्घ r;
#अन्यथा
	दीर्घ r;
#endif

	त्रुटि_सं = 0;
#जर ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#अन्यथा
	r = म_से_दीर्घ(s, &p, 10);
#endif

	जर (त्रुटि_सं != 0)
		syntax(s, "out of range");

	जर (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	परत r;
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


स्थिर गणना token check_operator(स्थिरांक अक्षर *s)
{
	स्थिर स्थिरांक संरचना operator_t no_op = {
		.op_अंक = -1,
		.op_type = -1
	};
	पूर्णांक n;

	last_operator = &no_op;
	जर (s == रिक्त)
		परत EOI;
	n = index_in_strings(ops_texts, s);
	जर (n < 0)
		परत OPERAND;

#जर BASH_TEST2
	जर (ops_table[n].op_अंक == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		परत OPERAND;
	}
	जर (ops_table[n].op_अंक == BAND || ops_table[n].op_अंक == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		जर (bash_test2 == (s[0] == '-'))
			परत OPERAND;
	}
#endif

	last_operator = &ops_table[n];
	परत ops_table[n].op_अंक;
}


स्थिर पूर्णांक binop(शून्य)
{
	स्थिरांक अक्षर *opnd1, *opnd2;
	स्थिरांक संरचना operator_t *op;
	संख्या_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	जर (opnd2 == रिक्त)
		syntax(args[-1], "argument expected");

	जर (is_int_op(op->op_अंक)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		जर (op->op_अंक == INTEQ)
			परत val1 == val2;
		जर (op->op_अंक == INTNE)
			परत val1 != val2;
		जर (op->op_अंक == INTGE)
			परत val1 >= val2;
		जर (op->op_अंक == INTGT)
			परत val1 >  val2;
		जर (op->op_अंक == INTLE)
			परत val1 <= val2;
		/*if (op->op_num == INTLT)*/
		परत val1 <  val2;
	}
#जर BASH_TEST2
	जर (bash_test2) {
		जर (op->op_अंक == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			परत val1 == 0;
		}
		जर (op->op_अंक == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			परत val1 != 0;
		}
		जर (op->op_अंक == REGEX) {
			regex_t re_buffer;
			स_रखो(&re_buffer, 0, आकार(re_buffer));
			जर (regcomp(&re_buffer, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				दीर्घ_लाँघ(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_buffer, opnd1, 0, रिक्त, 0);
			regfree(&re_buffer);
			परत val1 == 0;
		}
	}
#endif
	जर (is_str_op(op->op_अंक)) {
		val1 = म_भेद(opnd1, opnd2);
		जर (op->op_अंक == STREQ)
			परत val1 == 0;
		जर (op->op_अंक == STRNE)
			परत val1 != 0;
		जर (op->op_अंक == STRLT)
			परत val1 < 0;
		/*if (op->op_num == STRGT)*/
		परत val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		संरचना stat b1, b2;

		जर (stat(opnd1, &b1) || stat(opnd2, &b2))
			परत 0; /* false, since at least one stat failed */
		जर (op->op_अंक == FILNT)
			परत b1.st_mtime > b2.st_mtime;
		जर (op->op_अंक == FILOT)
			परत b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		परत b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
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
स्थिर पूर्णांक is_a_group_member(gid_t gid)
{
	पूर्णांक i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	जर (gid == getgid() || gid == getegid())
		परत 1;

	जर (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	साठी (i = 0; i < ngroups; i++)
		जर (gid == group_array[i])
			परत 1;

	परत 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
स्थिर पूर्णांक test_eaccess(संरचना stat *st, पूर्णांक mode)
{
	अचिन्हित पूर्णांक euid = geteuid();

	जर (euid == 0) {
		/* Root can read or write any file. */
		जर (mode != X_OK)
			परत 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		जर (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			परत 0;
	}

	जर (st->st_uid == euid)  /* owner */
		mode <<= 6;
	अन्यथा जर (is_a_group_member(st->st_gid))
		mode <<= 3;

	जर (st->st_mode & mode)
		परत 0;

	परत -1;
}


स्थिर पूर्णांक filstat(अक्षर *nm, गणना token mode)
{
	संरचना stat s;
	अचिन्हित i = i; /* gcc 3.x thinks it can be used uninitialized */

	जर (mode == FILSYM) {
#ifdef S_IFLNK
		जर (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			जा filetype;
		}
#endif
		परत 0;
	}

	जर (stat(nm, &s) != 0)
		परत 0;
	जर (mode == FILEXIST)
		परत 1;
	जर (is_file_access(mode)) {
		जर (mode == FILRD)
			i = R_OK;
		जर (mode == FILWR)
			i = W_OK;
		जर (mode == FILEX)
			i = X_OK;
		परत test_eaccess(&s, i) == 0;
	}
	जर (is_file_type(mode)) {
		जर (mode == FILREG)
			i = S_IFREG;
		जर (mode == FILDIR)
			i = S_IFDIR;
		जर (mode == FILCDEV)
			i = S_IFCHR;
		जर (mode == FILBDEV)
			i = S_IFBLK;
		जर (mode == FILFIFO) {
#ifdef S_IFIFO
			i = S_IFIFO;
#अन्यथा
			परत 0;
#endif
		}
		जर (mode == FILSOCK) {
#ifdef S_IFSOCK
			i = S_IFSOCK;
#अन्यथा
			परत 0;
#endif
		}
 filetype:
		परत ((s.st_mode & S_IFMT) == i);
	}
	जर (is_file_bit(mode)) {
		जर (mode == FILSUID)
			i = S_ISUID;
		जर (mode == FILSGID)
			i = S_ISGID;
		जर (mode == FILSTCK)
			i = S_ISVTX;
		परत ((s.st_mode & i) != 0);
	}
	जर (mode == FILGZ)
		परत s.st_size > 0L;
	जर (mode == FILUID)
		परत s.st_uid == geteuid();
	जर (mode == FILGID)
		परत s.st_gid == getegid();
	परत 1; /* NOTREACHED */
}


स्थिर संख्या_t nexpr(गणना token n)
{
	संख्या_t res;

	nest_संदेश(">nexpr(%s)\n", TOKSTR[n]);
	जर (n == UNOT) {
		n = check_operator(*++args);
		जर (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_संदेश("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			परत 1;
		}
		res = !nexpr(n);
		unnest_संदेश("<nexpr:%lld\n", res);
		परत res;
	}
	res = primary(n);
	unnest_संदेश("<nexpr:%lld\n", res);
	परत res;
}


स्थिर संख्या_t aexpr(गणना token n)
{
	संख्या_t res;

	nest_संदेश(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_संदेश("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	जर (check_operator(*++args) == BAND) {
		dbg_संदेश("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_संदेश("<aexpr:%lld\n", res);
		परत res;
	}
	args--;
	unnest_संदेश("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	परत res;
}


स्थिर संख्या_t oexpr(गणना token n)
{
	संख्या_t res;

	nest_संदेश(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_संदेश("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	जर (check_operator(*++args) == BOR) {
		dbg_संदेश("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_संदेश("<oexpr:%lld\n", res);
		परत res;
	}
	args--;
	unnest_संदेश("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	परत res;
}


स्थिर संख्या_t primary(गणना token n)
{
#जर TEST_DEBUG
	संख्या_t res = res; /* for compiler */
#अन्यथा
	संख्या_t res;
#endif
	स्थिरांक संरचना operator_t *args0_op;

	nest_संदेश(">primary(%s)\n", TOKSTR[n]);
	जर (n == EOI) {
		syntax(रिक्त, "argument expected");
	}
	जर (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		जर (check_operator(*++args) != RPAREN)
			syntax(रिक्त, "closing paren expected");
		unnest_संदेश("<primary:%lld\n", res);
		परत res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	जर (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		जर (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			जर (last_operator->op_type == BINOP)
				unnest_संदेश_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	जर (args0_op->op_type == UNOP) {
		/* unary expression */
		जर (args[1] == रिक्त)
//			syntax(args0_op->op_text, "argument expected");
			जा check_emptiness;
		args++;
		जर (n == STREZ)
			unnest_संदेश_and_return(args[0][0] == '\0', "<primary:%lld\n");
		जर (n == STRNZ)
			unnest_संदेश_and_return(args[0][0] != '\0', "<primary:%lld\n");
		जर (n == FILTT)
			unnest_संदेश_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_संदेश_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	जर (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_संदेश_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_संदेश_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


पूर्णांक test_main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक res;
	स्थिरांक अक्षर *arg0;
#जर BASH_TEST2
	bool bt2 = 0;
#endif

	arg0 = bb_basename(argv[0]);
	जर ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		जर (!arg0[1]) { /* "[" ? */
			जर (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_संदेश("missing ]");
				परत 2;
			}
		} अन्यथा { /* assuming "[[" */
			जर (म_भेद(argv[argc], "]]") != 0) {
				bb_simple_error_संदेश("missing ]]");
				परत 2;
			}
#जर BASH_TEST2
			bt2 = 1;
#endif
		}
		argv[argc] = रिक्त;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#जर BASH_TEST2
	bash_test2 = bt2;
#endif

	res = बनाओ_लाँघ(leaving);
	जर (res)
		जा ret;

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
	जर (1) {
		पूर्णांक negate = 0;
 again:
		जर (!argv[0]) {
			/* "test" */
			res = 1;
			जा ret_special;
		}
		जर (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			जा ret_special;
		}
		जर (argv[2]) {
			जर (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				जर (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					जा ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				जा check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			जर (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				जर (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = रिक्त;
					argv++;
				}
			}
		}
 check_negate:
		जर (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			जा again;
		}
	}

	res = !oexpr(check_operator(*args));

	जर (*args != रिक्त && *++args != रिक्त) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_संदेश("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	परत res;
}
