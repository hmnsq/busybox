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

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ga_nanaa {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
static void convert(var_na *fn, ចំនួនគត់ conv_type)
{
	FILE *in, *out;
	ចំនួនគត់ ch;
	var_na *temp_fn = temp_fn; /* for compiler */
	var_na *resolved_fn = resolved_fn;

	in = stdin;
	out = stdout;
	ប្រសិនបើ (fn != NULL) {
		sa_nracanaa stat st;
		ចំនួនគត់ fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		ប្រសិនបើ (resolved_fn == NULL)
			bb_simple_perror_msg_and_die(fn);
		in = xfopen_for_read(resolved_fn);
		xfstat(fileno(in), &st, resolved_fn);

		temp_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(temp_fn);
		ប្រសិនបើ (fchmod(fd, st.st_mode) == -1)
			bb_simple_perror_msg_and_die(temp_fn);
		fchown(fd, st.st_uid, st.st_gid);

		out = xfdopen_for_write(fd);
	}

	ខណៈពេល ((ch = fgetc(in)) != EOF) {
		ប្រសិនបើ (ch == '\r')
			continue;
		ប្រសិនបើ (ch == '\n')
			ប្រសិនបើ (conv_type == CT_UNIX2DOS)
				fputc('\r', out);
		fputc(ch, out);
	}

	ប្រសិនបើ (fn != NULL) {
		ប្រសិនបើ (fclose(in) < 0 || fclose(out) < 0) {
			unlink(temp_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(temp_fn, resolved_fn);
		free(temp_fn);
		free(resolved_fn);
	}
}

ចំនួនគត់ dos2unix_main(ចំនួនគត់ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ចំនួនគត់ dos2unix_main(ចំនួនគត់ argc UNUSED_PARAM, var_na **argv)
{
	ចំនួនគត់ o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	ប្រសិនបើ (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} បើមិនដូច្នេះ {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	ប្រសិនបើ (o)
		conv_type = o;

	argv += optind;
	do {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} ខណៈពេល (*argv && *++argv);

	ត្រឡប់ 0;
}
