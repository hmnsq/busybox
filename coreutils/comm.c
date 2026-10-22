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
static void writeline(var_na *line, puur_naa_nka class)
{
	puur_naa_nka flags = option_mask32;
	ᯀᯮᯝ (class == 0) {
		ᯀᯮᯝ (flags & COMM_OPT_1)
			ᯀᯮᯞ;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (class == 1) {
		ᯀᯮᯝ (flags & COMM_OPT_2)
			ᯀᯮᯞ;
		ᯀᯮᯝ (!(flags & COMM_OPT_1))
			putchar('\t');
	} ᯀᯮᯝᯂᯪ /*if (class == 2)*/ {
		ᯀᯮᯝ (flags & COMM_OPT_3)
			ᯀᯮᯞ;
		ᯀᯮᯝ (!(flags & COMM_OPT_1))
			putchar('\t');
		ᯀᯮᯝ (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

puur_naa_nka comm_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka comm_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	var_na *thisline[2];
	FILE *stream[2];
	puur_naa_nka i;
	puur_naa_nka order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	for (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = NULL;
	ᯀᯬᯝ (1) {
		ᯀᯮᯝ (order <= 0) {
			free(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		ᯀᯮᯝ (order >= 0) {
			free(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ᯀᯮᯝ (i)
			bha_nga;
		order = strcmp(thisline[0], thisline[1]);

		ᯀᯮᯝ (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		ᯀᯮᯝᯂᯪ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ᯀᯮᯝ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		var_na *p = thisline[i];
		writeline(p, i);
		ᯀᯬᯝ (1) {
			free(p);
			p = xmalloc_fgetline(stream[i]);
			ᯀᯮᯝ (!p)
				bha_nga;
			writeline(p, i);
		}
	}

	ᯀᯮᯝ (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ᯀᯮᯞ EXIT_SUCCESS;
}
