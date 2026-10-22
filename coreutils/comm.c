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

#అన్తర్భావ "libbb.h"

#నిర్వచించు COMM_OPT_1 (1 << 0)
#నిర్వచించు COMM_OPT_2 (1 << 1)
#నిర్వచించు COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
స్థిర శూన్య writeline(అక్షరం *పంక్తి, పూర్ణాంకం class)
{
	పూర్ణాంకం flags = option_mask32;
	ఒకవేళ (class == 0) {
		ఒకవేళ (flags & COMM_OPT_1)
			తిరిగి;
	} లేకపోతే ఒకవేళ (class == 1) {
		ఒకవేళ (flags & COMM_OPT_2)
			తిరిగి;
		ఒకవేళ (!(flags & COMM_OPT_1))
			అక్షరం_రాయి('\t');
	} లేకపోతే /*if (class == 2)*/ {
		ఒకవేళ (flags & COMM_OPT_3)
			తిరిగి;
		ఒకవేళ (!(flags & COMM_OPT_1))
			అక్షరం_రాయి('\t');
		ఒకవేళ (!(flags & COMM_OPT_2))
			అక్షరం_రాయి('\t');
	}
	puts(పంక్తి);
}

పూర్ణాంకం comm_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం comm_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం *thisline[2];
	FILE *ప్రవాహం[2];
	పూర్ణాంకం i;
	పూర్ణాంకం క్రమం;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	కొరకు (i = 0; i < 2; ++i) {
		ప్రవాహం[i] = xfopen_stdin(argv[i]);
	}

	క్రమం = 0;
	thisline[1] = thisline[0] = ఖాళీ;
	అయితే (1) {
		ఒకవేళ (క్రమం <= 0) {
			విడుదల(thisline[0]);
			thisline[0] = xmalloc_fgetline(ప్రవాహం[0]);
		}
		ఒకవేళ (క్రమం >= 0) {
			విడుదల(thisline[1]);
			thisline[1] = xmalloc_fgetline(ప్రవాహం[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ఒకవేళ (i)
			ఆపు;
		క్రమం = strcmp(thisline[0], thisline[1]);

		ఒకవేళ (క్రమం >= 0)
			writeline(thisline[1], క్రమం ? 1 : 2);
		లేకపోతే
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ఒకవేళ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		అక్షరం *p = thisline[i];
		writeline(p, i);
		అయితే (1) {
			విడుదల(p);
			p = xmalloc_fgetline(ప్రవాహం[i]);
			ఒకవేళ (!p)
				ఆపు;
			writeline(p, i);
		}
	}

	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి) {
		fclose(ప్రవాహం[0]);
		fclose(ప్రవాహం[1]);
	}

	తిరిగి EXIT_SUCCESS;
}
