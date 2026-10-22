/* vi: set sw=4 ts=4: */
/*
 * dos2unix for BusyBox
 *
 * dos2unix '\n' converter 0.5.0
 * based on Unix2Dos 0.9.0 by Peter Hanecak (made 19.2.1997)
 * Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.sk>.
 * All rights reserved.
 *
 * dos2unix filters reading input from stdin and writing output to stdout.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config DOS2UNIX
//config:	bool "dos2unix (5.5 kb)"
//config:	default y
//config:	help
//config:	dos2unix is used to convert a text file from DOS format to
//config:	UNIX format, and vice versa.
//config:
//config:config UNIX2DOS
//config:	bool "unix2dos (5.5 kb)"
//config:	default y
//config:	help
//config:	unix2dos is used to convert a text file from UNIX format to
//config:	DOS format, and vice versa.

//applet:IF_DOS2UNIX(APPLET_NOEXEC(dos2unix, dos2unix, BB_DIR_USR_BIN, BB_SUID_DROP, dos2unix))
//applet:IF_UNIX2DOS(APPLET_NOEXEC(unix2dos, dos2unix, BB_DIR_USR_BIN, BB_SUID_DROP, unix2dos))

//kbuild:lib-$(CONFIG_DOS2UNIX) += dos2unix.o
//kbuild:lib-$(CONFIG_UNIX2DOS) += dos2unix.o

//usage:#define dos2unix_trivial_usage
//usage:       "[-ud] [FILE]"
//usage:#define dos2unix_full_usage "\n\n"
//usage:       "Convert FILE in-place from DOS to Unix format.\n"
//usage:       "When no file is given, use stdin/stdout.\n"
//usage:     "\n	-u	dos2unix"
//usage:     "\n	-d	unix2dos"
//usage:
//usage:#define unix2dos_trivial_usage
//usage:       "[-ud] [FILE]"
//usage:#define unix2dos_full_usage "\n\n"
//usage:       "Convert FILE in-place from Unix to DOS format.\n"
//usage:       "When no file is given, use stdin/stdout.\n"
//usage:     "\n	-u	dos2unix"
//usage:     "\n	-d	unix2dos"

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

गणना {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
स्थिर शून्य convert(वर्ण *fn, पूर्णाङ्क conv_type)
{
	FILE *अन्तर्, *बहिर्गमनम्;
	पूर्णाङ्क ch;
	वर्ण *अस्थायी_fn = अस्थायी_fn; /* for compiler */
	वर्ण *resolved_fn = resolved_fn;

	अन्तर् = stdin;
	बहिर्गमनम् = stdout;
	यदि (fn != रिक्त) {
		संरचना stat st;
		पूर्णाङ्क fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		यदि (resolved_fn == रिक्त)
			bb_simple_perror_सन्देशः_and_die(fn);
		अन्तर् = xfopen_for_read(resolved_fn);
		xfstat(fileno(अन्तर्), &st, resolved_fn);

		अस्थायी_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(अस्थायी_fn);
		यदि (fchmod(fd, st.st_बहुलकम्) == -1)
			bb_simple_perror_सन्देशः_and_die(अस्थायी_fn);
		fchown(fd, st.st_uid, st.st_gid);

		बहिर्गमनम् = xfdopen_for_write(fd);
	}

	यावत् ((ch = fgetc(अन्तर्)) != EOF) {
		यदि (ch == '\r')
			अनुवर्त;
		यदि (ch == '\n')
			यदि (conv_type == CT_UNIX2DOS)
				fputc('\r', बहिर्गमनम्);
		fputc(ch, बहिर्गमनम्);
	}

	यदि (fn != रिक्त) {
		यदि (fclose(अन्तर्) < 0 || fclose(बहिर्गमनम्) < 0) {
			unlink(अस्थायी_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(अस्थायी_fn, resolved_fn);
		मुक्त(अस्थायी_fn);
		मुक्त(resolved_fn);
	}
}

पूर्णाङ्क dos2unix_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क dos2unix_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	पूर्णाङ्क o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	यदि (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} अन्यथा {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	यदि (o)
		conv_type = o;

	argv += optind;
	कुरु {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} यावत् (*argv && *++argv);

	प्रत्यागम 0;
}
