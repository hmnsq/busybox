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

#شامل "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ایستا تهی writeline(نویسه *سطر, صحیح class)
{
	صحیح flags = option_mask32;
	اگر (class == 0) {
		اگر (flags & COMM_OPT_1)
			بازگشت;
	} وگرنه اگر (class == 1) {
		اگر (flags & COMM_OPT_2)
			بازگشت;
		اگر (!(flags & COMM_OPT_1))
			putchar('\t');
	} وگرنه /*if (class == 2)*/ {
		اگر (flags & COMM_OPT_3)
			بازگشت;
		اگر (!(flags & COMM_OPT_1))
			putchar('\t');
		اگر (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(سطر);
}

صحیح comm_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح comm_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	نویسه *thisline[2];
	FILE *stream[2];
	صحیح i;
	صحیح order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	برای (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = پوچ;
	وقتی (1) {
		اگر (order <= 0) {
			آزاد(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		اگر (order >= 0) {
			آزاد(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		اگر (i)
			بشکن;
		order = strcmp(thisline[0], thisline[1]);

		اگر (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		وگرنه
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	اگر (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		نویسه *p = thisline[i];
		writeline(p, i);
		وقتی (1) {
			آزاد(p);
			p = xmalloc_fgetline(stream[i]);
			اگر (!p)
				بشکن;
			writeline(p, i);
		}
	}

	اگر (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	بازگشت EXIT_SUCCESS;
}
