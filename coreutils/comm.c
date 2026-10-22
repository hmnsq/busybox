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
static void writeline(var_na *line, จำนวนเต็ม class)
{
	จำนวนเต็ม flags = option_mask32;
	ถ้า (class == 0) {
		ถ้า (flags & COMM_OPT_1)
			คืน;
	} มิฉะนั้น ถ้า (class == 1) {
		ถ้า (flags & COMM_OPT_2)
			คืน;
		ถ้า (!(flags & COMM_OPT_1))
			putchar('\t');
	} มิฉะนั้น /*if (class == 2)*/ {
		ถ้า (flags & COMM_OPT_3)
			คืน;
		ถ้า (!(flags & COMM_OPT_1))
			putchar('\t');
		ถ้า (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

จำนวนเต็ม comm_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม comm_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv)
{
	var_na *thisline[2];
	FILE *stream[2];
	จำนวนเต็ม i;
	จำนวนเต็ม order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	สำหรับ (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = NULL;
	ขณะ (1) {
		ถ้า (order <= 0) {
			free(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		ถ้า (order >= 0) {
			free(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ถ้า (i)
			bha_nga;
		order = strcmp(thisline[0], thisline[1]);

		ถ้า (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		มิฉะนั้น
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ถ้า (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		var_na *p = thisline[i];
		writeline(p, i);
		ขณะ (1) {
			free(p);
			p = xmalloc_fgetline(stream[i]);
			ถ้า (!p)
				bha_nga;
			writeline(p, i);
		}
	}

	ถ้า (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	คืน EXIT_SUCCESS;
}
