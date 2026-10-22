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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ସ୍ଥିର ଶୂନ୍ୟ writeline(ଅକ୍ଷର *ଧାଡ଼ି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା class)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = option_mask32;
	ଯଦି (class == 0) {
		ଯଦି (flags & COMM_OPT_1)
			ଫେରନ୍ତୁ;
	} ଅନ୍ୟଥା ଯଦି (class == 1) {
		ଯଦି (flags & COMM_OPT_2)
			ଫେରନ୍ତୁ;
		ଯଦି (!(flags & COMM_OPT_1))
			putchar('\t');
	} ଅନ୍ୟଥା /*if (class == 2)*/ {
		ଯଦି (flags & COMM_OPT_3)
			ଫେରନ୍ତୁ;
		ଯଦି (!(flags & COMM_OPT_1))
			putchar('\t');
		ଯଦି (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(ଧାଡ଼ି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା comm_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା comm_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *thisline[2];
	FILE *stream[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	ପାଇଁ (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = ଖାଲି;
	ଯେତେବେଳେ (1) {
		ଯଦି (order <= 0) {
			ମୁକ୍ତ(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		ଯଦି (order >= 0) {
			ମୁକ୍ତ(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ଯଦି (i)
			ଭାଙ୍ଗ;
		order = strcmp(thisline[0], thisline[1]);

		ଯଦି (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		ଅନ୍ୟଥା
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ଯଦି (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		ଅକ୍ଷର *p = thisline[i];
		writeline(p, i);
		ଯେତେବେଳେ (1) {
			ମୁକ୍ତ(p);
			p = xmalloc_fgetline(stream[i]);
			ଯଦି (!p)
				ଭାଙ୍ଗ;
			writeline(p, i);
		}
	}

	ଯଦି (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
