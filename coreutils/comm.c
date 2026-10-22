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

#تعریف_کن COMM_OPT_1 (1 << 0)
#تعریف_کن COMM_OPT_2 (1 << 1)
#تعریف_کن COMM_OPT_3 (1 << 2)

/* writeline outputs the input given, appropriately aligned according to class */
ساكن فراغ writeline(حرف *سطر, صحيح class)
{
	صحيح flags = option_mask32;
	إذا (class == 0) {
		إذا (flags & COMM_OPT_1)
			أرجع;
	} وإلا إذا (class == 1) {
		إذا (flags & COMM_OPT_2)
			أرجع;
		إذا (!(flags & COMM_OPT_1))
			حرف_چاپ_کن('\t');
	} وإلا /*if (class == 2)*/ {
		إذا (flags & COMM_OPT_3)
			أرجع;
		إذا (!(flags & COMM_OPT_1))
			حرف_چاپ_کن('\t');
		إذا (!(flags & COMM_OPT_2))
			حرف_چاپ_کن('\t');
	}
	puts(سطر);
}

صحيح comm_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح comm_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	حرف *thisline[2];
	FILE *جریان[2];
	صحيح i;
	صحيح ترتیب;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	لكل (i = 0; i < 2; ++i) {
		جریان[i] = xfopen_stdin(argv[i]);
	}

	ترتیب = 0;
	thisline[1] = thisline[0] = عدم;
	طالما (1) {
		إذا (ترتیب <= 0) {
			حرر(thisline[0]);
			thisline[0] = xmalloc_fgetline(جریان[0]);
		}
		إذا (ترتیب >= 0) {
			حرر(thisline[1]);
			thisline[1] = xmalloc_fgetline(جریان[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		إذا (i)
			اكسر;
		ترتیب = strcmp(thisline[0], thisline[1]);

		إذا (ترتیب >= 0)
			writeline(thisline[1], ترتیب ? 1 : 2);
		وإلا
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	إذا (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		حرف *p = thisline[i];
		writeline(p, i);
		طالما (1) {
			حرر(p);
			p = xmalloc_fgetline(جریان[i]);
			إذا (!p)
				اكسر;
			writeline(p, i);
		}
	}

	إذا (ENABLE_FEATURE_CLEAN_به_بالا) {
		fclose(جریان[0]);
		fclose(جریان[1]);
	}

	أرجع EXIT_SUCCESS;
}
