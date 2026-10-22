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

#அந்தர்பாவ "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
நிலையான வெறுமை writeline(எழுத்து *வரி, முழு class)
{
	முழு flags = option_mask32;
	எனில் (class == 0) {
		எனில் (flags & COMM_OPT_1)
			திரும்பு;
	} இல்லையெனில் எனில் (class == 1) {
		எனில் (flags & COMM_OPT_2)
			திரும்பு;
		எனில் (!(flags & COMM_OPT_1))
			putchar('\t');
	} இல்லையெனில் /*if (class == 2)*/ {
		எனில் (flags & COMM_OPT_3)
			திரும்பு;
		எனில் (!(flags & COMM_OPT_1))
			putchar('\t');
		எனில் (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(வரி);
}

முழு comm_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு comm_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து *thisline[2];
	FILE *stream[2];
	முழு i;
	முழு order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	க்கு (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = வெற்று;
	போது (1) {
		எனில் (order <= 0) {
			விடுவி(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		எனில் (order >= 0) {
			விடுவி(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		எனில் (i)
			நிறுத்து;
		order = strcmp(thisline[0], thisline[1]);

		எனில் (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		இல்லையெனில்
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	எனில் (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		எழுத்து *p = thisline[i];
		writeline(p, i);
		போது (1) {
			விடுவி(p);
			p = xmalloc_fgetline(stream[i]);
			எனில் (!p)
				நிறுத்து;
			writeline(p, i);
		}
	}

	எனில் (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	திரும்பு EXIT_SUCCESS;
}
