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

#അന്തര്ഭാവ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

എണ്ണൽ {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
സ്ഥിതം ശൂന്യം convert(അക്ഷരം *fn, പൂർണ്ണസംഖ്യ conv_type)
{
	FILE *in, *out;
	പൂർണ്ണസംഖ്യ ch;
	അക്ഷരം *temp_fn = temp_fn; /* for compiler */
	അക്ഷരം *resolved_fn = resolved_fn;

	in = stdin;
	out = stdout;
	എങ്കിൽ (fn != ശൂന്യ) {
		ഘടന stat st;
		പൂർണ്ണസംഖ്യ fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		എങ്കിൽ (resolved_fn == ശൂന്യ)
			bb_simple_perror_സന്ദേശം_and_die(fn);
		in = xfopen_for_read(resolved_fn);
		xfstat(fileno(in), &st, resolved_fn);

		temp_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(temp_fn);
		എങ്കിൽ (fchmod(fd, st.st_mode) == -1)
			bb_simple_perror_സന്ദേശം_and_die(temp_fn);
		fchown(fd, st.st_uid, st.st_gid);

		out = xfdopen_for_write(fd);
	}

	ആകുമ്പോൾ ((ch = fgetc(in)) != EOF) {
		എങ്കിൽ (ch == '\r')
			തുടരുക;
		എങ്കിൽ (ch == '\n')
			എങ്കിൽ (conv_type == CT_UNIX2DOS)
				fputc('\r', out);
		fputc(ch, out);
	}

	എങ്കിൽ (fn != ശൂന്യ) {
		എങ്കിൽ (fclose(in) < 0 || fclose(out) < 0) {
			unlink(temp_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(temp_fn, resolved_fn);
		സ്വതന്ത്രം(temp_fn);
		സ്വതന്ത്രം(resolved_fn);
	}
}

പൂർണ്ണസംഖ്യ dos2unix_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ dos2unix_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	എങ്കിൽ (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} അല്ലെങ്കിൽ {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	എങ്കിൽ (o)
		conv_type = o;

	argv += optind;
	ചെയ്യുക {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} ആകുമ്പോൾ (*argv && *++argv);

	തിരികെ 0;
}
