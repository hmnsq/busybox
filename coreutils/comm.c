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

#تعریف_کرو COMM_OPT_1 (1 << 0)
#تعریف_کرو COMM_OPT_2 (1 << 1)
#تعریف_کرو COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ساکن خالی writeline(حرف *سطر, صحیح class)
{
	صحیح flags = option_mask32;
	اگر (class == 0) {
		اگر (flags & COMM_OPT_1)
			واپس;
	} ورنہ اگر (class == 1) {
		اگر (flags & COMM_OPT_2)
			واپس;
		اگر (!(flags & COMM_OPT_1))
			حرف_لکھو('\t');
	} ورنہ /*if (class == 2)*/ {
		اگر (flags & COMM_OPT_3)
			واپس;
		اگر (!(flags & COMM_OPT_1))
			حرف_لکھو('\t');
		اگر (!(flags & COMM_OPT_2))
			حرف_لکھو('\t');
	}
	puts(سطر);
}

صحیح comm_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح comm_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	حرف *thisline[2];
	FILE *دھارا[2];
	صحیح i;
	صحیح ترتیب;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	برائے (i = 0; i < 2; ++i) {
		دھارا[i] = xfopen_stdin(argv[i]);
	}

	ترتیب = 0;
	thisline[1] = thisline[0] = تہی;
	جب (1) {
		اگر (ترتیب <= 0) {
			آزاد(thisline[0]);
			thisline[0] = xmalloc_fgetline(دھارا[0]);
		}
		اگر (ترتیب >= 0) {
			آزاد(thisline[1]);
			thisline[1] = xmalloc_fgetline(دھارا[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		اگر (i)
			توڑو;
		ترتیب = strcmp(thisline[0], thisline[1]);

		اگر (ترتیب >= 0)
			writeline(thisline[1], ترتیب ? 1 : 2);
		ورنہ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	اگر (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		حرف *p = thisline[i];
		writeline(p, i);
		جب (1) {
			آزاد(p);
			p = xmalloc_fgetline(دھارا[i]);
			اگر (!p)
				توڑو;
			writeline(p, i);
		}
	}

	اگر (ENABLE_FEATURE_CLEAN_اوپر) {
		fclose(دھارا[0]);
		fclose(دھارا[1]);
	}

	واپس EXIT_SUCCESS;
}
