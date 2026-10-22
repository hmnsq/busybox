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

#ضمن "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

تعداد {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
ساكن فراغ convert(حرف *fn, صحيح conv_type)
{
	FILE *اندريون, *ٻاهريون;
	صحيح ch;
	حرف *عارضي_fn = عارضي_fn; /* for compiler */
	حرف *resolved_fn = resolved_fn;

	اندريون = stdin;
	ٻاهريون = stdout;
	إذا (fn != عدم) {
		بنية stat st;
		صحيح fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		إذا (resolved_fn == عدم)
			bb_simple_perror_نياپو_and_die(fn);
		اندريون = xfopen_for_read(resolved_fn);
		xfstat(fileno(اندريون), &st, resolved_fn);

		عارضي_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(عارضي_fn);
		إذا (fchmod(fd, st.st_گهڻائي) == -1)
			bb_simple_perror_نياپو_and_die(عارضي_fn);
		fchown(fd, st.st_uid, st.st_gid);

		ٻاهريون = xfdopen_for_write(fd);
	}

	طالما ((ch = fgetc(اندريون)) != EOF) {
		إذا (ch == '\r')
			استمر;
		إذا (ch == '\n')
			إذا (conv_type == CT_UNIX2DOS)
				fputc('\r', ٻاهريون);
		fputc(ch, ٻاهريون);
	}

	إذا (fn != عدم) {
		إذا (fclose(اندريون) < 0 || fclose(ٻاهريون) < 0) {
			unlink(عارضي_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(عارضي_fn, resolved_fn);
		حرر(عارضي_fn);
		حرر(resolved_fn);
	}
}

صحيح dos2unix_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح dos2unix_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	صحيح o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	إذا (ENABLE_DOS2UNIX
	 && (!ENABLE_UNIX2DOS || applet_name[0] == 'd')
	) {
		conv_type = CT_DOS2UNIX;
	} وإلا {
		conv_type = CT_UNIX2DOS;
	}

	/* -u convert to unix, -d convert to dos */
	o = getopt32(argv, "^" "du" "\0" "u--d:d--u"); /* mutually exclusive */

	/* Do the conversion requested by an argument else do the default
	 * conversion depending on our name.  */
	إذا (o)
		conv_type = o;

	argv += optind;
	افعل {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} طالما (*argv && *++argv);

	أرجع 0;
}
