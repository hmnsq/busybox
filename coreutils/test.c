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

#समावेश "libbb.h"
#समावेश <regex.h>
#समावेश <fnmatch.h>

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

#घोषणा TEST_DEBUG 0

#अगर ENABLE_TEST2 \
 || (ENABLE_ASH_BASH_COMPAT && ENABLE_ASH_TEST) \
 || (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
# define BASH_TEST2 1
#अन्यथा
# define BASH_TEST2 0
#पूर्ण_अगर

क्रमागत token {
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

#अगर BASH_TEST2
	REGEX,
#पूर्ण_अगर

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
#घोषणा is_int_op(a)      (((अचिन्हित अक्षर)((a) - INTEQ)) <= 5)
#घोषणा is_str_op(a)      (((अचिन्हित अक्षर)((a) - STREZ)) <= 5)
#घोषणा is_file_op(a)     (((अचिन्हित अक्षर)((a) - FILNT)) <= 2)
#घोषणा is_file_access(a) (((अचिन्हित अक्षर)((a) - FILRD)) <= 2)
#घोषणा is_file_type(a)   (((अचिन्हित अक्षर)((a) - FILREG)) <= 5)
#घोषणा is_file_bit(a)    (((अचिन्हित अक्षर)((a) - FILSUID)) <= 2)

#अगर TEST_DEBUG
पूर्णांक depth;
#घोषणा nest_संदेश(...) करो { \
	depth++; \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
} जबतक (0)
#घोषणा unnest_संदेश(...) करो { \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
	depth--; \
} जबतक (0)
#घोषणा dbg_संदेश(...) करो { \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__); \
} जबतक (0)
#घोषणा unnest_संदेश_and_return(व्यंजक, ...) करो { \
	संख्या_t __res = (व्यंजक); \
	ख_लिखो(मानक_त्रुटि, "%*s", depth*2, ""); \
	ख_लिखो(मानक_त्रुटि, __VA_ARGS__, res); \
	depth--; \
	वापस __res; \
} जबतक (0)
अटल स्थिर अक्षर *स्थिर TOKSTR[] ALIGN_PTR = {
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
#अगर BASH_TEST2
	"REGEX",
#पूर्ण_अगर
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
#घोषणा nest_संदेश(...)   ((व्योम)0)
#घोषणा unnest_संदेश(...) ((व्योम)0)
#घोषणा dbg_संदेश(...)    ((व्योम)0)
#घोषणा unnest_संदेश_and_return(व्यंजक, ...) वापस व्यंजक
#पूर्ण_अगर

क्रमागत {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

काष्ठा operator_t {
	अचिन्हित अक्षर op_अंक, op_type;
};

अटल स्थिर काष्ठा operator_t ops_table[] ALIGN2 = {
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
#अगर BASH_TEST2
	{ /* "=~" */ REGEX   , BINOP  },
#पूर्ण_अगर
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
#अगर BASH_TEST2
	{ /* "&&" */ BAND    , BBINOP },
	{ /* "||" */ BOR     , BBINOP },
#पूर्ण_अगर
	{ /* "("  */ LPAREN  , PAREN  },
	{ /* ")"  */ RPAREN  , PAREN  },
};
/* Please keep these two tables in sync */
अटल स्थिर अक्षर ops_texts[] ALIGN1 =
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
#अगर BASH_TEST2
	"=~"  "\0"
#पूर्ण_अगर
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
#अगर BASH_TEST2
	"&&"  "\0"
	"||"  "\0"
#पूर्ण_अगर
	"("   "\0"
	")"   "\0"
;


#अगर ENABLE_FEATURE_TEST_64
प्रकार int64_t संख्या_t;
#अन्यथा
प्रकार पूर्णांक संख्या_t;
#पूर्ण_अगर


/* We try to minimize both static and stack usage. */
काष्ठा test_statics {
	अक्षर **args;
	/* set only by check_operator(), either to bogus struct
	 * or points to matching operator_t struct. Never NULL. */
	स्थिर काष्ठा operator_t *last_operator;
	gid_t *group_array;
	पूर्णांक ngroups;
#अगर BASH_TEST2
	bool bash_test2;
#पूर्ण_अगर
	लाँघ_बफ leaving;
};

/* See test_ptr_hack.c */
बाह्य काष्ठा test_statics *BB_GLOBAL_CONST test_ptr_to_statics;

#घोषणा S (*test_ptr_to_statics)
#घोषणा args            (S.args         )
#घोषणा last_operator   (S.last_operator)
#घोषणा group_array     (S.group_array  )
#घोषणा ngroups         (S.ngroups      )
#घोषणा bash_test2      (S.bash_test2   )
#घोषणा leaving         (S.leaving      )

#घोषणा INIT_S() करो { \
	XZALLOC_CONST_PTR(&test_ptr_to_statics, माप(S)); \
} जबतक (0)
#घोषणा DEINIT_S() करो { \
	मुक्त(group_array); \
	मुक्त(test_ptr_to_statics); \
} जबतक (0)

अटल संख्या_t primary(क्रमागत token n);

अटल व्योम syntax(स्थिर अक्षर *op, स्थिर अक्षर *संदेश) NORETURN;
अटल व्योम syntax(स्थिर अक्षर *op, स्थिर अक्षर *संदेश)
{
	अगर (op && *op) {
		bb_error_संदेश("%s: %s", op, संदेश);
	} अन्यथा {
		bb_error_संदेश("%s: %s"+4, संदेश);
	}
	दीर्घ_लाँघ(leaving, 2);
}

/* atoi with error detection */
//XXX: FIXME: duplicate of existing libbb function?
अटल संख्या_t getn(स्थिर अक्षर *s)
{
	अक्षर *p;
#अगर ENABLE_FEATURE_TEST_64
	दीर्घ दीर्घ r;
#अन्यथा
	दीर्घ r;
#पूर्ण_अगर

	त्रुटि_सं = 0;
#अगर ENABLE_FEATURE_TEST_64
	r = strtoll(s, &p, 10);
#अन्यथा
	r = म_से_दीर्घ(s, &p, 10);
#पूर्ण_अगर

	अगर (त्रुटि_सं != 0)
		syntax(s, "out of range");

	अगर (p == s || *(skip_whitespace(p)) != '\0')
		syntax(s, "bad number");

	वापस r;
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


अटल क्रमागत token check_operator(स्थिर अक्षर *s)
{
	अटल स्थिर काष्ठा operator_t no_op = {
		.op_अंक = -1,
		.op_type = -1
	};
	पूर्णांक n;

	last_operator = &no_op;
	अगर (s == शून्य)
		वापस EOI;
	n = सूचकांक_in_strings(ops_texts, s);
	अगर (n < 0)
		वापस OPERAND;

#अगर BASH_TEST2
	अगर (ops_table[n].op_अंक == REGEX && !bash_test2) {
		/* =~ is only for [[ ]] */
		वापस OPERAND;
	}
	अगर (ops_table[n].op_अंक == BAND || ops_table[n].op_अंक == BOR) {
		/* [ ]   accepts -a and -o but not && and || */
		/* [[ ]] accepts && and || but not -a and -o */
		अगर (bash_test2 == (s[0] == '-'))
			वापस OPERAND;
	}
#पूर्ण_अगर

	last_operator = &ops_table[n];
	वापस ops_table[n].op_अंक;
}


अटल पूर्णांक binop(व्योम)
{
	स्थिर अक्षर *opnd1, *opnd2;
	स्थिर काष्ठा operator_t *op;
	संख्या_t val1, val2;

	opnd1 = *args;
	check_operator(*++args);
	op = last_operator;

	opnd2 = *++args;
	अगर (opnd2 == शून्य)
		syntax(args[-1], "argument expected");

	अगर (is_int_op(op->op_अंक)) {
		val1 = getn(opnd1);
		val2 = getn(opnd2);
		अगर (op->op_अंक == INTEQ)
			वापस val1 == val2;
		अगर (op->op_अंक == INTNE)
			वापस val1 != val2;
		अगर (op->op_अंक == INTGE)
			वापस val1 >= val2;
		अगर (op->op_अंक == INTGT)
			वापस val1 >  val2;
		अगर (op->op_अंक == INTLE)
			वापस val1 <= val2;
		/*if (op->op_num == INTLT)*/
		वापस val1 <  val2;
	}
#अगर BASH_TEST2
	अगर (bash_test2) {
		अगर (op->op_अंक == STREQ) {
			val1 = fnmatch(opnd2, opnd1, 0);
			वापस val1 == 0;
		}
		अगर (op->op_अंक == STRNE) {
			val1 = fnmatch(opnd2, opnd1, 0);
			वापस val1 != 0;
		}
		अगर (op->op_अंक == REGEX) {
			regex_t re_बफर;
			स_रखो(&re_बफर, 0, माप(re_बफर));
			अगर (regcomp(&re_बफर, opnd2, REG_EXTENDED)) { // REG_NEWLINE?
				/* Bad regex */
				दीर्घ_लाँघ(leaving, 2); /* [[ a =~ * ]]; echo $? - prints 2 (silently, no error msg) */
			}
			val1 = regexec(&re_बफर, opnd1, 0, शून्य, 0);
			regfree(&re_बफर);
			वापस val1 == 0;
		}
	}
#पूर्ण_अगर
	अगर (is_str_op(op->op_अंक)) {
		val1 = म_भेद(opnd1, opnd2);
		अगर (op->op_अंक == STREQ)
			वापस val1 == 0;
		अगर (op->op_अंक == STRNE)
			वापस val1 != 0;
		अगर (op->op_अंक == STRLT)
			वापस val1 < 0;
		/*if (op->op_num == STRGT)*/
		वापस val1 > 0;
	}
	/* We are sure that these three are by now the only binops we didn't check
	 * yet, so we do not check if the class is correct:
	 */
/*	if (is_file_op(op->op_num)) */
	{
		काष्ठा stat b1, b2;

		अगर (stat(opnd1, &b1) || stat(opnd2, &b2))
			वापस 0; /* false, since at least one stat failed */
		अगर (op->op_अंक == FILNT)
			वापस b1.st_mtime > b2.st_mtime;
		अगर (op->op_अंक == FILOT)
			वापस b1.st_mtime < b2.st_mtime;
		/*if (op->op_num == FILEQ)*/
		वापस b1.st_dev == b2.st_dev && b1.st_ino == b2.st_ino;
	}
	/*return 1; - NOTREACHED */
}

अटल व्योम initialize_group_array(व्योम)
{
	group_array = bb_getgroups(&ngroups, शून्य);
}

/* Return non-zero if GID is one that we have in our groups list. */
//XXX: FIXME: duplicate of existing libbb function?
// see toplevel TODO file:
// possible code duplication ingroup() and is_a_group_member()
अटल पूर्णांक is_a_group_member(gid_t gid)
{
	पूर्णांक i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	अगर (gid == getgid() || gid == getegid())
		वापस 1;

	अगर (ngroups == 0)
		initialize_group_array();

	/* Search through the list looking for GID. */
	क्रम (i = 0; i < ngroups; i++)
		अगर (gid == group_array[i])
			वापस 1;

	वापस 0;
}


/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
अटल पूर्णांक test_eaccess(काष्ठा stat *st, पूर्णांक mode)
{
	अचिन्हित पूर्णांक euid = geteuid();

	अगर (euid == 0) {
		/* Root can read or write any file. */
		अगर (mode != X_OK)
			वापस 0;

		/* Root can execute any file that has any one of the execute
		 * bits set. */
		अगर (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			वापस 0;
	}

	अगर (st->st_uid == euid)  /* owner */
		mode <<= 6;
	अन्यथा अगर (is_a_group_member(st->st_gid))
		mode <<= 3;

	अगर (st->st_mode & mode)
		वापस 0;

	वापस -1;
}


अटल पूर्णांक filstat(अक्षर *nm, क्रमागत token mode)
{
	काष्ठा stat s;
	अचिन्हित i = i; /* gcc 3.x thinks it can be used uninitialized */

	अगर (mode == FILSYM) {
#अगर_घोषित S_IFLNK
		अगर (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			जाओ filetype;
		}
#पूर्ण_अगर
		वापस 0;
	}

	अगर (stat(nm, &s) != 0)
		वापस 0;
	अगर (mode == FILEXIST)
		वापस 1;
	अगर (is_file_access(mode)) {
		अगर (mode == FILRD)
			i = R_OK;
		अगर (mode == FILWR)
			i = W_OK;
		अगर (mode == FILEX)
			i = X_OK;
		वापस test_eaccess(&s, i) == 0;
	}
	अगर (is_file_type(mode)) {
		अगर (mode == FILREG)
			i = S_IFREG;
		अगर (mode == FILDIR)
			i = S_IFDIR;
		अगर (mode == FILCDEV)
			i = S_IFCHR;
		अगर (mode == FILBDEV)
			i = S_IFBLK;
		अगर (mode == FILFIFO) {
#अगर_घोषित S_IFIFO
			i = S_IFIFO;
#अन्यथा
			वापस 0;
#पूर्ण_अगर
		}
		अगर (mode == FILSOCK) {
#अगर_घोषित S_IFSOCK
			i = S_IFSOCK;
#अन्यथा
			वापस 0;
#पूर्ण_अगर
		}
 filetype:
		वापस ((s.st_mode & S_IFMT) == i);
	}
	अगर (is_file_bit(mode)) {
		अगर (mode == FILSUID)
			i = S_ISUID;
		अगर (mode == FILSGID)
			i = S_ISGID;
		अगर (mode == FILSTCK)
			i = S_ISVTX;
		वापस ((s.st_mode & i) != 0);
	}
	अगर (mode == FILGZ)
		वापस s.st_आकार > 0L;
	अगर (mode == FILUID)
		वापस s.st_uid == geteuid();
	अगर (mode == FILGID)
		वापस s.st_gid == getegid();
	वापस 1; /* NOTREACHED */
}


अटल संख्या_t nexpr(क्रमागत token n)
{
	संख्या_t res;

	nest_संदेश(">nexpr(%s)\n", TOKSTR[n]);
	अगर (n == UNOT) {
		n = check_operator(*++args);
		अगर (n == EOI) {
			/* special case: [ ! ], [ a -a ! ] are valid */
			/* IOW, "! ARG" may miss ARG */
			args--;
			unnest_संदेश("<nexpr:1 (!EOI), args:%s(%p)\n", args[0], &args[0]);
			वापस 1;
		}
		res = !nexpr(n);
		unnest_संदेश("<nexpr:%lld\n", res);
		वापस res;
	}
	res = primary(n);
	unnest_संदेश("<nexpr:%lld\n", res);
	वापस res;
}


अटल संख्या_t aexpr(क्रमागत token n)
{
	संख्या_t res;

	nest_संदेश(">aexpr(%s)\n", TOKSTR[n]);
	res = nexpr(n);
	dbg_संदेश("aexpr: nexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	अगर (check_operator(*++args) == BAND) {
		dbg_संदेश("aexpr: arg is AND, next args:%s(%p)\n", args[1], &args[1]);
		res = aexpr(check_operator(*++args)) && res;
		unnest_संदेश("<aexpr:%lld\n", res);
		वापस res;
	}
	args--;
	unnest_संदेश("<aexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	वापस res;
}


अटल संख्या_t oexpr(क्रमागत token n)
{
	संख्या_t res;

	nest_संदेश(">oexpr(%s)\n", TOKSTR[n]);
	res = aexpr(n);
	dbg_संदेश("oexpr: aexpr:%lld, next args:%s(%p)\n", res, args[1], &args[1]);
	अगर (check_operator(*++args) == BOR) {
		dbg_संदेश("oexpr: next arg is OR, next args:%s(%p)\n", args[1], &args[1]);
		res = oexpr(check_operator(*++args)) || res;
		unnest_संदेश("<oexpr:%lld\n", res);
		वापस res;
	}
	args--;
	unnest_संदेश("<oexpr:%lld, args:%s(%p)\n", res, args[0], &args[0]);
	वापस res;
}


अटल संख्या_t primary(क्रमागत token n)
{
#अगर TEST_DEBUG
	संख्या_t res = res; /* for compiler */
#अन्यथा
	संख्या_t res;
#पूर्ण_अगर
	स्थिर काष्ठा operator_t *args0_op;

	nest_संदेश(">primary(%s)\n", TOKSTR[n]);
	अगर (n == EOI) {
		syntax(शून्य, "argument expected");
	}
	अगर (n == LPAREN) {
		res = oexpr(check_operator(*++args));
		अगर (check_operator(*++args) != RPAREN)
			syntax(शून्य, "closing paren expected");
		unnest_संदेश("<primary:%lld\n", res);
		वापस res;
	}

	/* coreutils 6.9 checks "is args[1] binop and args[2] exist?" first,
	 * do the same */
	args0_op = last_operator;
	/* last_operator = operator at args[1] */
	अगर (check_operator(args[1]) != EOI) { /* if args[1] != NULL */
		अगर (args[2]) {
			// coreutils also does this:
			// if (args[3] && args[0] = "-l" && args[2] is BINOP)
			//	return binop(1 /* prepended by -l */);
			अगर (last_operator->op_type == BINOP)
				unnest_संदेश_and_return(binop(), "<primary: binop:%lld\n");
		}
	}
	/* check "is args[0] unop?" second */
	अगर (args0_op->op_type == UNOP) {
		/* unary expression */
		अगर (args[1] == शून्य)
//			syntax(args0_op->op_text, "argument expected");
			जाओ check_emptiness;
		args++;
		अगर (n == STREZ)
			unnest_संदेश_and_return(args[0][0] == '\0', "<primary:%lld\n");
		अगर (n == STRNZ)
			unnest_संदेश_and_return(args[0][0] != '\0', "<primary:%lld\n");
		अगर (n == FILTT)
			unnest_संदेश_and_return(isatty(getn(*args)), "<primary: isatty(%s)%lld\n", *args);
		unnest_संदेश_and_return(filstat(*args, n), "<primary: filstat(%s):%lld\n", *args);
	}

	/*check_operator(args[1]); - already done */
	अगर (last_operator->op_type == BINOP) {
		/* args[2] is known to be NULL, isn't it bound to fail? */
		unnest_संदेश_and_return(binop(), "<primary:%lld\n");
	}
 check_emptiness:
	unnest_संदेश_and_return(args[0][0] != '\0', "<primary:%lld\n");
}


पूर्णांक test_main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक res;
	स्थिर अक्षर *arg0;
#अगर BASH_TEST2
	bool bt2 = 0;
#पूर्ण_अगर

	arg0 = bb_basename(argv[0]);
	अगर ((ENABLE_TEST1 || ENABLE_TEST2 || ENABLE_ASH_TEST || ENABLE_HUSH_TEST)
	 && (arg0[0] == '[')
	) {
		--argc;
		अगर (!arg0[1]) { /* "[" ? */
			अगर (NOT_LONE_CHAR(argv[argc], ']')) {
				bb_simple_error_संदेश("missing ]");
				वापस 2;
			}
		} अन्यथा { /* assuming "[[" */
			अगर (म_भेद(argv[argc], "]]") != 0) {
				bb_simple_error_संदेश("missing ]]");
				वापस 2;
			}
#अगर BASH_TEST2
			bt2 = 1;
#पूर्ण_अगर
		}
		argv[argc] = शून्य;
	}
	/* argc is unused after this point */

	/* We must do DEINIT_S() prior to returning */
	INIT_S();

#अगर BASH_TEST2
	bash_test2 = bt2;
#पूर्ण_अगर

	res = बनाओ_लाँघ(leaving);
	अगर (res)
		जाओ ret;

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
	अगर (1) {
		पूर्णांक negate = 0;
 again:
		अगर (!argv[0]) {
			/* "test" */
			res = 1;
			जाओ ret_special;
		}
		अगर (!argv[1]) {
			/* "test [!] arg" */
			res = (argv[0][0] == '\0');
			जाओ ret_special;
		}
		अगर (argv[2]) {
			अगर (!argv[3]) {
				/*
				 * http://pubs.opengroup.org/onlinepubs/009695399/utilities/test.html
				 * """ 3 arguments:
				 * If $2 is a binary primary, perform the binary test of $1 and $3.
				 * """
				 */
				check_operator(argv[1]);
				अगर (last_operator->op_type == BINOP) {
					/* "test [!] arg1 <binary_op> arg2" */
					args = argv;
					res = (binop() == 0);
 ret_special:
					/* If there was leading "!" op... */
					res ^= negate;
					जाओ ret;
				}
				/* """If $1 is '(' and $3 is ')', perform the unary test of $2."""
				 * Looks like this works without additional coding.
				 */
				जाओ check_negate;
			}
			/* argv[3] exists (at least 4 args), is it exactly 4 args? */
			अगर (!argv[4]) {
				/*
				 * """ 4 arguments:
				 * If $1 is '!', negate the three-argument test of $2, $3, and $4.
				 * If $1 is '(' and $4 is ')', perform the two-argument test of $2 and $3.
				 * """
				 * Example why code below is necessary: test '(' ! -e ')'
				 */
				अगर (LONE_CHAR(argv[0], '(')
				 && LONE_CHAR(argv[3], ')')
				) {
					/* "test [!] ( x y )" */
					argv[3] = शून्य;
					argv++;
				}
			}
		}
 check_negate:
		अगर (LONE_CHAR(argv[0], '!')) {
			argv++;
			negate ^= 1;
			जाओ again;
		}
	}

	res = !oexpr(check_operator(*args));

	अगर (*args != शून्य && *++args != शून्य) {
		/* Examples:
		 * test 3 -lt 5 6
		 * test -t 1 2
		 */
		bb_error_संदेश("%s: unknown operand", *args);
		res = 2;
	}
 ret:
	DEINIT_S();
	वापस res;
}
