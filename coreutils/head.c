/* vi: set sw=4 ts=4: */
/*
 * head implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config HEAD
//config:	bool "head (4 kb)"
//config:	default y
//config:	help
//config:	head is used to print the first specified number of lines
//config:	from files.
//config:
//config:config FEATURE_FANCY_HEAD
//config:	bool "Enable -c, -q, and -v"
//config:	default y
//config:	depends on HEAD

//applet:IF_HEAD(APPLET_NOEXEC(head, head, BB_DIR_USR_BIN, BB_SUID_DROP, head))

//kbuild:lib-$(CONFIG_HEAD) += head.o

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU compatible -c, -q, and -v options in 'fancy' configuration. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/head.html */

//usage:#define head_trivial_usage
//usage:       "[OPTIONS] [FILE]..."
//usage:#define head_full_usage "\n\n"
//usage:       "Print first 10 lines of FILEs (or stdin).\n"
//usage:       "With more than one FILE, precede each with a filename header.\n"
//usage:     "\n	-n N[bkm]	Print first N lines"
//usage:	IF_FEATURE_FANCY_HEAD(
//usage:     "\n	-n -N[bkm]	Print all except N last lines"
//usage:     "\n	-c [-]N[bkm]	Print first N bytes"
//usage:	)
//usage:     "\n			(b:*512 k:*1024 m:*1024^2)"
//usage:	IF_FEATURE_FANCY_HEAD(
//usage:     "\n	-q		Never print headers"
//usage:     "\n	-v		Always print headers"
//usage:	)
//usage:
//usage:#define head_example_usage
//usage:       "$ head -n 2 /etc/passwd\n"
//usage:       "root:x:0:0:root:/root:/bin/bash\n"
//usage:       "daemon:x:1:1:daemon:/usr/sbin:/bin/sh\n"

#അന്തര്ഭാവ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#എങ്കിൽ !ENABLE_FEATURE_FANCY_HEAD
# define print_first_N(fp,എണ്ണം,bytes) print_first_N(fp,എണ്ണം)
#endif
സ്ഥിതം ശൂന്യം
print_first_N(FILE *fp, unsigned long എണ്ണം, bool എണ്ണം_bytes)
{
#എങ്കിൽ !ENABLE_FEATURE_FANCY_HEAD
	സ്ഥിരം പൂർണ്ണസംഖ്യ എണ്ണം_bytes = 0;
#endif
	ആകുമ്പോൾ (എണ്ണം) {
		പൂർണ്ണസംഖ്യ c = getc(fp);
		എങ്കിൽ (c == EOF)
			നിർത്തുക;
		എങ്കിൽ (എണ്ണം_bytes || (c == '\n'))
			--എണ്ണം;
		putchar(c);
	}
}

#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
സ്ഥിതം ശൂന്യം
print_except_N_last_bytes(FILE *fp, unsigned എണ്ണം)
{
	unsigned അക്ഷരം *circle = xmalloc(++എണ്ണം);
	unsigned head = 0;
	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ c;
		c = getc(fp);
		എങ്കിൽ (c == EOF)
			പോകുക ret;
		circle[head++] = c;
		എങ്കിൽ (head == എണ്ണം)
			നിർത്തുക;
	}
	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ c;
		എങ്കിൽ (head == എണ്ണം)
			head = 0;
		putchar(circle[head]);
		c = getc(fp);
		എങ്കിൽ (c == EOF)
			പോകുക ret;
		circle[head] = c;
		head++;
	}
 ret:
	സ്വതന്ത്രം(circle);
}

സ്ഥിതം ശൂന്യം
print_except_N_last_വരികൾ(FILE *fp, unsigned എണ്ണം)
{
	അക്ഷരം **circle = xzalloc((++എണ്ണം) * വലുപ്പം(circle[0]));
	unsigned head = 0;
	വേണ്ടി (;;) {
		അക്ഷരം *c;
		c = xmalloc_fgets(fp);
		എങ്കിൽ (!c)
			പോകുക ret;
		circle[head++] = c;
		എങ്കിൽ (head == എണ്ണം)
			നിർത്തുക;
	}
	വേണ്ടി (;;) {
		അക്ഷരം *c;
		എങ്കിൽ (head == എണ്ണം)
			head = 0;
		fputs_stdout(circle[head]);
		c = xmalloc_fgets(fp);
		എങ്കിൽ (!c)
			പോകുക ret;
		സ്വതന്ത്രം(circle[head]);
		circle[head++] = c;
	}
 ret:
	head = 0;
	വേണ്ടി (;;) {
		സ്വതന്ത്രം(circle[head++]);
		എങ്കിൽ (head == എണ്ണം)
			നിർത്തുക;
	}
	സ്വതന്ത്രം(circle);
}
#അല്ലെങ്കിൽ
/* Must never be called */
ശൂന്യം print_except_N_last_bytes(FILE *fp, unsigned എണ്ണം);
ശൂന്യം print_except_N_last_വരികൾ(FILE *fp, unsigned എണ്ണം);
#endif

#എങ്കിൽ !ENABLE_FEATURE_FANCY_HEAD
# define eat_അക്കം(negative_N,p) eat_അക്കം(p)
#endif
സ്ഥിതം unsigned long
eat_അക്കം(bool *negative_N, സ്ഥിരം അക്ഷരം *p)
{
#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
	എങ്കിൽ (*p == '-') {
		*negative_N = 1;
		p++;
	}
#endif
	തിരികെ xatoul_sfx(p, bkm_suffixes);
}

സ്ഥിതം സ്ഥിരം അക്ഷരം head_opts[] ALIGN1 =
	"n:"
#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#define header_fmt_str "\n==> %s <==\n"

പൂർണ്ണസംഖ്യ head_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ head_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	unsigned long എണ്ണം = 10;
#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
	പൂർണ്ണസംഖ്യ header_threshhold = 1;
	bool എണ്ണം_bytes = 0;
	bool negative_N = 0;
#അല്ലെങ്കിൽ
# define header_threshhold 1
# define എണ്ണം_bytes       0
# define negative_N        0
#endif
	FILE *fp;
	സ്ഥിരം അക്ഷരം *fmt;
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ opt;
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;

#എങ്കിൽ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	എങ്കിൽ (argv[1] && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = argv[0] + 1;
		പോകുക GET_എണ്ണം;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	ആകുമ്പോൾ ((opt = getopt(argc, argv, head_opts)) > 0) {
		തിരഞ്ഞെടുപ്പ് (opt) {
#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
		അവസ്ഥ 'q':
			header_threshhold = INT_MAX;
			നിർത്തുക;
		അവസ്ഥ 'v':
			header_threshhold = -1;
			നിർത്തുക;
		അവസ്ഥ 'c':
			എണ്ണം_bytes = 1;
			/* fall through */
#endif
		അവസ്ഥ 'n':
			p = optarg;
#എങ്കിൽ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_എണ്ണം:
#endif
			എണ്ണം = eat_അക്കം(&negative_N, p);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	എങ്കിൽ (!*argv)
		*--argv = (അക്ഷരം*)"-";

	fmt = header_fmt_str + 1;
	എങ്കിൽ (argc <= header_threshhold) {
#എങ്കിൽ ENABLE_FEATURE_FANCY_HEAD
		header_threshhold = 0;
#അല്ലെങ്കിൽ
		fmt += 11; /* "" */
#endif
	}
	എങ്കിൽ (negative_N) {
		എങ്കിൽ (എണ്ണം >= INT_MAX / വലുപ്പം(അക്ഷരം*))
			bb_error_സന്ദേശം("count is too big: %lu", എണ്ണം);
	}

	ചെയ്യുക {
		fp = fopen_or_warn_stdin(*argv);
		എങ്കിൽ (fp) {
			എങ്കിൽ (fp == stdin) {
				*argv = (അക്ഷരം *) bb_സന്ദേശം_standard_നിവേശം;
			}
			എങ്കിൽ (header_threshhold) {
				printf(fmt, *argv);
			}
			എങ്കിൽ (negative_N) {
				എങ്കിൽ (എണ്ണം_bytes) {
					print_except_N_last_bytes(fp, എണ്ണം);
				} അല്ലെങ്കിൽ {
					print_except_N_last_വരികൾ(fp, എണ്ണം);
				}
			} അല്ലെങ്കിൽ {
				print_first_N(fp, എണ്ണം, എണ്ണം_bytes);
			}
			die_if_ferror_stdout();
			എങ്കിൽ (fclose_if_not_stdin(fp)) {
				bb_simple_perror_സന്ദേശം(*argv);
				retval = EXIT_FAILURE;
			}
		} അല്ലെങ്കിൽ {
			retval = EXIT_FAILURE;
		}
		fmt = header_fmt_str;
	} ആകുമ്പോൾ (*++argv);

	fflush_stdout_and_exit(retval);
}
