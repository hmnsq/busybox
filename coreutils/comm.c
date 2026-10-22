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

#ضمن "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ساكن فراغ writeline(حرف *line, صحيح class)
{
	صحيح flags = option_mask32;
	ܐܢ (class == 0) {
		ܐܢ (flags & COMM_OPT_1)
			ܦܢܐ;
	} ܐܚܪܢܐ ܐܢ (class == 1) {
		ܐܢ (flags & COMM_OPT_2)
			ܦܢܐ;
		ܐܢ (!(flags & COMM_OPT_1))
			putchar('\t');
	} ܐܚܪܢܐ /*if (class == 2)*/ {
		ܐܢ (flags & COMM_OPT_3)
			ܦܢܐ;
		ܐܢ (!(flags & COMM_OPT_1))
			putchar('\t');
		ܐܢ (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(line);
}

صحيح comm_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح comm_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	حرف *thisline[2];
	FILE *stream[2];
	صحيح i;
	صحيح order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	لكل (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = عدم;
	ܟܡܐ (1) {
		ܐܢ (order <= 0) {
			حرر(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		ܐܢ (order >= 0) {
			حرر(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ܐܢ (i)
			اكسر;
		order = strcmp(thisline[0], thisline[1]);

		ܐܢ (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		ܐܚܪܢܐ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ܐܢ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		حرف *p = thisline[i];
		writeline(p, i);
		ܟܡܐ (1) {
			حرر(p);
			p = xmalloc_fgetline(stream[i]);
			ܐܢ (!p)
				اكسر;
			writeline(p, i);
		}
	}

	ܐܢ (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	ܦܢܐ EXIT_SUCCESS;
}
