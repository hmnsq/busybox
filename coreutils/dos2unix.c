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

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ಗಣನಾ {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
ಸ್ಥಿರ ಶೂನ್ಯ convert(ವರ್ಣ *fn, ಪೂರ್ಣಾಙ್ಕ conv_type)
{
	FILE *in, *out;
	ಪೂರ್ಣಾಙ್ಕ ch;
	ವರ್ಣ *temp_fn = temp_fn; /* for compiler */
	ವರ್ಣ *resolved_fn = resolved_fn;

	in = stdin;
	out = stdout;
	ಯದಿ (fn != ರಿಕ್ತ) {
		ಸಂರಚನಾ stat st;
		ಪೂರ್ಣಾಙ್ಕ fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		ಯದಿ (resolved_fn == ರಿಕ್ತ)
			bb_simple_perror_ಸಂದೇಶ_and_die(fn);
		in = xfopen_for_read(resolved_fn);
		xfstat(fileno(in), &st, resolved_fn);

		temp_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(temp_fn);
		ಯದಿ (fchmod(fd, st.st_mode) == -1)
			bb_simple_perror_ಸಂದೇಶ_and_die(temp_fn);
		fchown(fd, st.st_uid, st.st_gid);

		out = xfdopen_for_write(fd);
	}

	ಯಾವತ್ ((ch = fgetc(in)) != EOF) {
		ಯದಿ (ch == '\r')
			ಅನುವರ್ತ;
		ಯದಿ (ch == '\n')
			ಯದಿ (conv_type == CT_UNIX2DOS)
				fputc('\r', out);
		fputc(ch, out);
	}

	ಯದಿ (fn != ರಿಕ್ತ) {
		ಯದಿ (fclose(in) < 0 || fclose(out) < 0) {
			unlink(temp_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(temp_fn, resolved_fn);
		ಮುಕ್ತ(temp_fn);
		ಮುಕ್ತ(resolved_fn);
	}
}

ಪೂರ್ಣಾಙ್ಕ dos2unix_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ dos2unix_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	ಯದಿ (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} ಅನ್ಯಥಾ {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	ಯದಿ (o)
		conv_type = o;

	argv += optind;
	ಕುರು {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} ಯಾವತ್ (*argv && *++argv);

	ಪ್ರತ್ಯಾಗಮ 0;
}
