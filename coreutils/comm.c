/* vi: set sw=4 ts=4: */
/*
 * Mini comm implementation for busybox
 *
 * Copyright (C) 2005 by Robert Sullivan <cogito.ergo.cogito@gmail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config COMM
//config:	bool "comm (4.4 kb)"
//config:	default y
//config:	help
//config:	comm is used to compare two files line by line and return
//config:	a three-column output.

//applet:IF_COMM(APPLET(comm, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_COMM) += comm.o

//usage:#define comm_trivial_usage
//usage:       "[-123] FILE1 FILE2"
//usage:#define comm_full_usage "\n\n"
//usage:       "Compare FILE1 with FILE2\n"
//usage:     "\n	-1	Suppress lines unique to FILE1"
//usage:     "\n	-2	Suppress lines unique to FILE2"
//usage:     "\n	-3	Suppress lines common to both files"

#അന്തര്ഭാവ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
സ്ഥിതം ശൂന്യം writeline(അക്ഷരം *വരി, പൂർണ്ണസംഖ്യ class)
{
	പൂർണ്ണസംഖ്യ flags = option_mask32;
	എങ്കിൽ (class == 0) {
		എങ്കിൽ (flags & COMM_OPT_1)
			തിരികെ;
	} അല്ലെങ്കിൽ എങ്കിൽ (class == 1) {
		എങ്കിൽ (flags & COMM_OPT_2)
			തിരികെ;
		എങ്കിൽ (!(flags & COMM_OPT_1))
			putchar('\t');
	} അല്ലെങ്കിൽ /*if (class == 2)*/ {
		എങ്കിൽ (flags & COMM_OPT_3)
			തിരികെ;
		എങ്കിൽ (!(flags & COMM_OPT_1))
			putchar('\t');
		എങ്കിൽ (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(വരി);
}

പൂർണ്ണസംഖ്യ comm_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ comm_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം *thisline[2];
	FILE *stream[2];
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	വേണ്ടി (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = ശൂന്യ;
	ആകുമ്പോൾ (1) {
		എങ്കിൽ (order <= 0) {
			സ്വതന്ത്രം(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		എങ്കിൽ (order >= 0) {
			സ്വതന്ത്രം(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		എങ്കിൽ (i)
			നിർത്തുക;
		order = strcmp(thisline[0], thisline[1]);

		എങ്കിൽ (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		അല്ലെങ്കിൽ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	എങ്കിൽ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		അക്ഷരം *p = thisline[i];
		writeline(p, i);
		ആകുമ്പോൾ (1) {
			സ്വതന്ത്രം(p);
			p = xmalloc_fgetline(stream[i]);
			എങ്കിൽ (!p)
				നിർത്തുക;
			writeline(p, i);
		}
	}

	എങ്കിൽ (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	തിരികെ EXIT_SUCCESS;
}
