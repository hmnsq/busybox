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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

ਗਿਣਤੀ {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
ਸਥਿਰ ਖਾਲੀ convert(ਅੱਖਰ *fn, ਪੂਰਨ conv_type)
{
	FILE *ਵਿੱਚ, *ਬਾਹਰਲਾ;
	ਪੂਰਨ ch;
	ਅੱਖਰ *ਆਰਜ਼ੀ_fn = ਆਰਜ਼ੀ_fn; /* for compiler */
	ਅੱਖਰ *resolved_fn = resolved_fn;

	ਵਿੱਚ = stdin;
	ਬਾਹਰਲਾ = stdout;
	ਜੇ (fn != ਨਿਰਾ) {
		ਬਣਤਰ stat st;
		ਪੂਰਨ fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		ਜੇ (resolved_fn == ਨਿਰਾ)
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(fn);
		ਵਿੱਚ = xfopen_for_read(resolved_fn);
		xfstat(fileno(ਵਿੱਚ), &st, resolved_fn);

		ਆਰਜ਼ੀ_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(ਆਰਜ਼ੀ_fn);
		ਜੇ (fchmod(fd, st.st_ਬਹੁਲਕ) == -1)
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(ਆਰਜ਼ੀ_fn);
		fchown(fd, st.st_uid, st.st_gid);

		ਬਾਹਰਲਾ = xfdopen_for_write(fd);
	}

	ਜਦੋਂ ((ch = fgetc(ਵਿੱਚ)) != EOF) {
		ਜੇ (ch == '\r')
			ਜਾਰੀ;
		ਜੇ (ch == '\n')
			ਜੇ (conv_type == CT_UNIX2DOS)
				fputc('\r', ਬਾਹਰਲਾ);
		fputc(ch, ਬਾਹਰਲਾ);
	}

	ਜੇ (fn != ਨਿਰਾ) {
		ਜੇ (fclose(ਵਿੱਚ) < 0 || fclose(ਬਾਹਰਲਾ) < 0) {
			unlink(ਆਰਜ਼ੀ_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(ਆਰਜ਼ੀ_fn, resolved_fn);
		ਮੁਕਤ(ਆਰਜ਼ੀ_fn);
		ਮੁਕਤ(resolved_fn);
	}
}

ਪੂਰਨ dos2unix_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ dos2unix_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਪੂਰਨ o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	ਜੇ (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} ਨਹੀਂਤਾਂ {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	ਜੇ (o)
		conv_type = o;

	argv += optind;
	ਕਰੋ {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} ਜਦੋਂ (*argv && *++argv);

	ਵਾਪਸ 0;
}
