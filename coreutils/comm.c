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

#অন্তর্ভাৱ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
স্থির শূন্য writeline(ৱর্ণ *সাৰি, পূর্ণাঙ্ক class)
{
	পূর্ণাঙ্ক flags = option_mask32;
	যদি (class == 0) {
		যদি (flags & COMM_OPT_1)
			ঘূৰাই;
	} অন্যথা যদি (class == 1) {
		যদি (flags & COMM_OPT_2)
			ঘূৰাই;
		যদি (!(flags & COMM_OPT_1))
			putchar('\t');
	} অন্যথা /*if (class == 2)*/ {
		যদি (flags & COMM_OPT_3)
			ঘূৰাই;
		যদি (!(flags & COMM_OPT_1))
			putchar('\t');
		যদি (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(সাৰি);
}

পূর্ণাঙ্ক comm_main(পূর্ণাঙ্ক argc, ৱর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক comm_main(পূর্ণাঙ্ক argc UNUSED_PARAM, ৱর্ণ **argv)
{
	ৱর্ণ *thisline[2];
	FILE *stream[2];
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	বাবে (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = রিক্ত;
	যেতিয়ালৈ (1) {
		যদি (order <= 0) {
			মুক্ত(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		যদি (order >= 0) {
			মুক্ত(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		যদি (i)
			ভঙ্গ;
		order = strcmp(thisline[0], thisline[1]);

		যদি (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		অন্যথা
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	যদি (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		ৱর্ণ *p = thisline[i];
		writeline(p, i);
		যেতিয়ালৈ (1) {
			মুক্ত(p);
			p = xmalloc_fgetline(stream[i]);
			যদি (!p)
				ভঙ্গ;
			writeline(p, i);
		}
	}

	যদি (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ঘূৰাই EXIT_SUCCESS;
}
