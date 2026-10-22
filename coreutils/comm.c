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

#અન્તર્ભાવ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
સ્થિર શૂન્ય writeline(અક્ષર *લીટી, પૂર્ણાંક class)
{
	પૂર્ણાંક flags = option_mask32;
	જો (class == 0) {
		જો (flags & COMM_OPT_1)
			પાછા;
	} નહીંતર જો (class == 1) {
		જો (flags & COMM_OPT_2)
			પાછા;
		જો (!(flags & COMM_OPT_1))
			putchar('\t');
	} નહીંતર /*if (class == 2)*/ {
		જો (flags & COMM_OPT_3)
			પાછા;
		જો (!(flags & COMM_OPT_1))
			putchar('\t');
		જો (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(લીટી);
}

પૂર્ણાંક comm_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક comm_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર *thisline[2];
	FILE *stream[2];
	પૂર્ણાંક i;
	પૂર્ણાંક order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	માટે (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = ખાલી;
	જ્યારે (1) {
		જો (order <= 0) {
			મુક્ત(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		જો (order >= 0) {
			મુક્ત(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		જો (i)
			તોડો;
		order = strcmp(thisline[0], thisline[1]);

		જો (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		નહીંતર
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	જો (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		અક્ષર *p = thisline[i];
		writeline(p, i);
		જ્યારે (1) {
			મુક્ત(p);
			p = xmalloc_fgetline(stream[i]);
			જો (!p)
				તોડો;
			writeline(p, i);
		}
	}

	જો (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	પાછા EXIT_SUCCESS;
}
