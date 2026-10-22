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

#अन्तर्भाव "libbb.h"

#define COMM_OPT_1 (1 << 0)
#define COMM_OPT_2 (1 << 1)
#define COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
स्थिर शून्य writeline(वर्ण *ओळ, पूर्णाङ्क class)
{
	पूर्णाङ्क flags = option_mask32;
	यदि (class == 0) {
		यदि (flags & COMM_OPT_1)
			प्रत्यागम;
	} अन्यथा यदि (class == 1) {
		यदि (flags & COMM_OPT_2)
			प्रत्यागम;
		यदि (!(flags & COMM_OPT_1))
			putchar('\t');
	} अन्यथा /*if (class == 2)*/ {
		यदि (flags & COMM_OPT_3)
			प्रत्यागम;
		यदि (!(flags & COMM_OPT_1))
			putchar('\t');
		यदि (!(flags & COMM_OPT_2))
			putchar('\t');
	}
	puts(ओळ);
}

पूर्णाङ्क comm_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क comm_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण *thisline[2];
	FILE *stream[2];
	पूर्णाङ्क i;
	पूर्णाङ्क order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	प्रति (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = रिक्त;
	यावत् (1) {
		यदि (order <= 0) {
			मुक्त(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		यदि (order >= 0) {
			मुक्त(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		यदि (i)
			भङ्ग;
		order = strcmp(thisline[0], thisline[1]);

		यदि (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		अन्यथा
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	यदि (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		वर्ण *p = thisline[i];
		writeline(p, i);
		यावत् (1) {
			मुक्त(p);
			p = xmalloc_fgetline(stream[i]);
			यदि (!p)
				भङ्ग;
			writeline(p, i);
		}
	}

	यदि (ENABLE_FEATURE_CLEAN_UP) {
		fclose(stream[0]);
		fclose(stream[1]);
	}

	प्रत्यागम EXIT_SUCCESS;
}
