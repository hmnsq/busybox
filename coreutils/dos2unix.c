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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

గణన {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
స్థిర శూన్య convert(అక్షరం *fn, పూర్ణాంకం conv_type)
{
	FILE *లోపల, *బహిర్గతం;
	పూర్ణాంకం ch;
	అక్షరం *తాత్కాలిక_fn = తాత్కాలిక_fn; /* for compiler */
	అక్షరం *resolved_fn = resolved_fn;

	లోపల = stdin;
	బహిర్గతం = stdout;
	ఒకవేళ (fn != ఖాళీ) {
		నిర్మాణం stat st;
		పూర్ణాంకం fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		ఒకవేళ (resolved_fn == ఖాళీ)
			bb_simple_perror_సందేశం_and_die(fn);
		లోపల = xfopen_for_read(resolved_fn);
		xfstat(fileno(లోపల), &st, resolved_fn);

		తాత్కాలిక_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(తాత్కాలిక_fn);
		ఒకవేళ (fchmod(fd, st.st_బహుళకం) == -1)
			bb_simple_perror_సందేశం_and_die(తాత్కాలిక_fn);
		fchown(fd, st.st_uid, st.st_gid);

		బహిర్గతం = xfdopen_for_write(fd);
	}

	అయితే ((ch = fgetc(లోపల)) != EOF) {
		ఒకవేళ (ch == '\r')
			కొనసాగు;
		ఒకవేళ (ch == '\n')
			ఒకవేళ (conv_type == CT_UNIX2DOS)
				fputc('\r', బహిర్గతం);
		fputc(ch, బహిర్గతం);
	}

	ఒకవేళ (fn != ఖాళీ) {
		ఒకవేళ (fclose(లోపల) < 0 || fclose(బహిర్గతం) < 0) {
			unlink(తాత్కాలిక_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(తాత్కాలిక_fn, resolved_fn);
		విడుదల(తాత్కాలిక_fn);
		విడుదల(resolved_fn);
	}
}

పూర్ణాంకం dos2unix_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం dos2unix_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	పూర్ణాంకం o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	ఒకవేళ (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} లేకపోతే {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	ఒకవేళ (o)
		conv_type = o;

	argv += optind;
	చేయి {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} అయితే (*argv && *++argv);

	తిరిగి 0;
}
