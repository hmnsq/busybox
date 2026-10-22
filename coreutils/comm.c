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
static void writeline(var_na *line, ကိန်းပြည့် class)
{
	ကိန်းပြည့် flags = option_mask32;
	အကယ်၍ (class == 0) {
		အကယ်၍ (flags & COMM_OPT_1)
			ပြန်ပေး;
	} မဟုတ်ပါက အကယ်၍ (class == 1) {
		အကယ်၍ (flags & COMM_OPT_2)
			ပြန်ပေး;
		အကယ်၍ (!(flags & COMM_OPT_1))
			putchar('\t');
	} မဟုတ်ပါက /*if (class == 2)*/ {
		အကယ်၍ (flags & COMM_OPT_3)
			ပြန်ပေး;
		အကယ်၍ (!(flags & COMM_OPT_1))
			putchar('\t');
		အကယ်၍ (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

ကိန်းပြည့် comm_main(ကိန်းပြည့် argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ကိန်းပြည့် comm_main(ကိန်းပြည့် argc UNUSED_PARAM, var_na **argv)
{
	var_na *thisline[2];
	FILE *stream[2];
	ကိန်းပြည့် i;
	ကိန်းပြည့် order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	အတွက် (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = NULL;
	နေစဉ် (1) {
		အကယ်၍ (order <= 0) {
			free(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		အကယ်၍ (order >= 0) {
			free(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		အကယ်၍ (i)
			bha_nga;
		order = strcmp(thisline[0], thisline[1]);

		အကယ်၍ (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		မဟုတ်ပါက
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	အကယ်၍ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		var_na *p = thisline[i];
		writeline(p, i);
		နေစဉ် (1) {
			free(p);
			p = xmalloc_fgetline(stream[i]);
			အကယ်၍ (!p)
				bha_nga;
			writeline(p, i);
		}
	}

	အကယ်၍ (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ပြန်ပေး EXIT_SUCCESS;
}
