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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMM_OPT_1 (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMM_OPT_2 (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ਸਥਿਰ ਖਾਲੀ writeline(ਅੱਖਰ *ਸਤਰ, ਪੂਰਨ class)
{
	ਪੂਰਨ flags = option_mask32;
	ਜੇ (class == 0) {
		ਜੇ (flags & COMM_OPT_1)
			ਵਾਪਸ;
	} ਨਹੀਂਤਾਂ ਜੇ (class == 1) {
		ਜੇ (flags & COMM_OPT_2)
			ਵਾਪਸ;
		ਜੇ (!(flags & COMM_OPT_1))
			ਅੱਖਰ_ਲਿਖੋ('\t');
	} ਨਹੀਂਤਾਂ /*if (class == 2)*/ {
		ਜੇ (flags & COMM_OPT_3)
			ਵਾਪਸ;
		ਜੇ (!(flags & COMM_OPT_1))
			ਅੱਖਰ_ਲਿਖੋ('\t');
		ਜੇ (!(flags & COMM_OPT_2))
			ਅੱਖਰ_ਲਿਖੋ('\t');
	}
	puts(ਸਤਰ);
}

ਪੂਰਨ comm_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ comm_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *thisline[2];
	FILE *ਧਾਰਾ[2];
	ਪੂਰਨ i;
	ਪੂਰਨ ਕ੍ਰਮ;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	ਲਈ (i = 0; i < 2; ++i) {
		ਧਾਰਾ[i] = xfopen_stdin(argv[i]);
	}

	ਕ੍ਰਮ = 0;
	thisline[1] = thisline[0] = ਨਿਰਾ;
	ਜਦੋਂ (1) {
		ਜੇ (ਕ੍ਰਮ <= 0) {
			ਮੁਕਤ(thisline[0]);
			thisline[0] = xmalloc_fgetline(ਧਾਰਾ[0]);
		}
		ਜੇ (ਕ੍ਰਮ >= 0) {
			ਮੁਕਤ(thisline[1]);
			thisline[1] = xmalloc_fgetline(ਧਾਰਾ[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		ਜੇ (i)
			ਰੋਕੋ;
		ਕ੍ਰਮ = strcmp(thisline[0], thisline[1]);

		ਜੇ (ਕ੍ਰਮ >= 0)
			writeline(thisline[1], ਕ੍ਰਮ ? 1 : 2);
		ਨਹੀਂਤਾਂ
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	ਜੇ (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		ਅੱਖਰ *p = thisline[i];
		writeline(p, i);
		ਜਦੋਂ (1) {
			ਮੁਕਤ(p);
			p = xmalloc_fgetline(ਧਾਰਾ[i]);
			ਜੇ (!p)
				ਰੋਕੋ;
			writeline(p, i);
		}
	}

	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ) {
		fclose(ਧਾਰਾ[0]);
		fclose(ਧਾਰਾ[1]);
	}

	ਵਾਪਸ EXIT_SUCCESS;
}
