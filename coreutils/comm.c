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

#ಅನ್ತರ್ಭಾವ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ಸ್ಥಿರ ಶೂನ್ಯ writeline(ವರ್ಣ *ಸಾಲು, ಪೂರ್ಣಾಙ್ಕ class)
{
	ಪೂರ್ಣಾಙ್ಕ flags = option_mask32;
	ಯದಿ (class == 0) {
		ಯದಿ (flags & COMM_OPT_1)
			ಪ್ರತ್ಯಾಗಮ;
	} ಅನ್ಯಥಾ ಯದಿ (class == 1) {
		ಯದಿ (flags & COMM_OPT_2)
			ಪ್ರತ್ಯಾಗಮ;
		ಯದಿ (!(flags & COMM_OPT_1))
			putchar('\t');
	} ಅನ್ಯಥಾ /*if (class == 2)*/ {
		ಯದಿ (flags & COMM_OPT_3)
			ಪ್ರತ್ಯಾಗಮ;
		ಯದಿ (!(flags & COMM_OPT_1))
			putchar('\t');
		ಯದಿ (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(ಸಾಲು);
}

ಪೂರ್ಣಾಙ್ಕ comm_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ comm_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ *thisline[2];
	FILE *stream[2];
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	ಪ್ರತಿ (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = ರಿಕ್ತ;
	ಯಾವತ್ (1) {
		ಯದಿ (order <= 0) {
			ಮುಕ್ತ(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		ಯದಿ (order >= 0) {
			ಮುಕ್ತ(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ಯದಿ (i)
			ಭಙ್ಗ;
		order = strcmp(thisline[0], thisline[1]);

		ಯದಿ (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		ಅನ್ಯಥಾ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ಯದಿ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		ವರ್ಣ *p = thisline[i];
		writeline(p, i);
		ಯಾವತ್ (1) {
			ಮುಕ್ತ(p);
			p = xmalloc_fgetline(stream[i]);
			ಯದಿ (!p)
				ಭಙ್ಗ;
			writeline(p, i);
		}
	}

	ಯದಿ (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
