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

#include "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
static void writeline(char *line, 정수 class)
{
	정수 flags = option_mask32;
	만약 (class == 0) {
		만약 (flags & COMM_OPT_1)
			반환;
	} 아니면 만약 (class == 1) {
		만약 (flags & COMM_OPT_2)
			반환;
		만약 (!(flags & COMM_OPT_1))
			putchar('\t');
	} 아니면 /*if (class == 2)*/ {
		만약 (flags & COMM_OPT_3)
			반환;
		만약 (!(flags & COMM_OPT_1))
			putchar('\t');
		만약 (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

정수 comm_main(정수 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
정수 comm_main(정수 argc UNUSED_PARAM, char **argv)
{
	char *thisline[2];
	FILE *stream[2];
	정수 i;
	정수 order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	위해 (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = NULL;
	동안 (1) {
		만약 (order <= 0) {
			free(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		만약 (order >= 0) {
			free(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		만약 (i)
			break;
		order = strcmp(thisline[0], thisline[1]);

		만약 (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		아니면
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	만약 (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		char *p = thisline[i];
		writeline(p, i);
		동안 (1) {
			free(p);
			p = xmalloc_fgetline(stream[i]);
			만약 (!p)
				break;
			writeline(p, i);
		}
	}

	만약 (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	반환 EXIT_SUCCESS;
}
