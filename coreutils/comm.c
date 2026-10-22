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
स्थिर शून्य writeline(अक्षर *ओळ, पूर्णांक class)
{
	पूर्णांक flags = option_mask32;
	जर (class == 0) {
		जर (flags & COMM_OPT_1)
			परत;
	} अन्यथा जर (class == 1) {
		जर (flags & COMM_OPT_2)
			परत;
		जर (!(flags & COMM_OPT_1))
			अक्षर_दो('\t');
	} अन्यथा /*if (class == 2)*/ {
		जर (flags & COMM_OPT_3)
			परत;
		जर (!(flags & COMM_OPT_1))
			अक्षर_दो('\t');
		जर (!(flags & COMM_OPT_2))
			अक्षर_दो('\t');
	}
	माला_दो(ओळ);
}

पूर्णांक comm_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक comm_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *thisline[2];
	खाता *stream[2];
	पूर्णांक i;
	पूर्णांक order;

	getopt32(argv, "^" "123" "\0" "=2");
	argv += optind;

	साठी (i = 0; i < 2; ++i) {
		stream[i] = xfopen_stdin(argv[i]);
	}

	order = 0;
	thisline[1] = thisline[0] = रिक्त;
	पर्यंत (1) {
		जर (order <= 0) {
			मुक्त(thisline[0]);
			thisline[0] = xmalloc_fgetline(stream[0]);
		}
		जर (order >= 0) {
			मुक्त(thisline[1]);
			thisline[1] = xmalloc_fgetline(stream[1]);
		}

		i = !thisline[0] + (!thisline[1] << 1);
		जर (i)
			थांबा;
		order = म_भेद(thisline[0], thisline[1]);

		जर (order >= 0)
			writeline(thisline[1], order ? 1 : 2);
		अन्यथा
			writeline(thisline[0], 0);
	}

	/* EOF at least on one of the streams */
	i &= 1;
	जर (thisline[i]) {
		/* stream[i] is not at EOF yet */
		/* we did not print thisline[i] yet */
		अक्षर *p = thisline[i];
		writeline(p, i);
		पर्यंत (1) {
			मुक्त(p);
			p = xmalloc_fgetline(stream[i]);
			जर (!p)
				थांबा;
			writeline(p, i);
		}
	}

	जर (ENABLE_FEATURE_CLEAN_UP) {
		ख_बंद(stream[0]);
		ख_बंद(stream[1]);
	}

	परत निकास_सफल;
}
