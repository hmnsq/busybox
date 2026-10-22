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
static void writeline(char *line, 整数 class)
{
	整数 flags = option_mask32;
	如果 (class == 0) {
		如果 (flags & COMM_OPT_1)
			返回;
	} 否则 如果 (class == 1) {
		如果 (flags & COMM_OPT_2)
			返回;
		如果 (!(flags & COMM_OPT_1))
			putchar('\t');
	} 否则 /*if (class == 2)*/ {
		如果 (flags & COMM_OPT_3)
			返回;
		如果 (!(flags & COMM_OPT_1))
			putchar('\t');
		如果 (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

整数 comm_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 comm_main(整数 argc UNUSED_PARAM, char **argv)
{
	char *thisline[2];
	FILE *stream[2];
	整数 i;
	整数 order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	为 (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = NULL;
	当 (1) {
		如果 (order <= 0) {
			free(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		如果 (order >= 0) {
			free(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		如果 (i)
			break;
		order = strcmp(thisline[0], thisline[1]);

		如果 (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		否则
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	如果 (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		char *p = thisline[i];
		writeline(p, i);
		当 (1) {
			free(p);
			p = xmalloc_fgetline(stream[i]);
			如果 (!p)
				break;
			writeline(p, i);
		}
	}

	如果 (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	返回 EXIT_SUCCESS;
}
