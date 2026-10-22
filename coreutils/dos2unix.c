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

#समावेश "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

क्रमागत {
	CT_UNIX2DOS = 1,
	CT_DOS2UNIX
};

/* if fn is NULL then input is stdin and output is stdout */
अटल व्योम convert(अक्षर *fn, पूर्णांक conv_type)
{
	खाता *in, *out;
	पूर्णांक ch;
	अक्षर *अस्थायी_fn = अस्थायी_fn; /* for compiler */
	अक्षर *resolved_fn = resolved_fn;

	in = मानक_निवेश;
	out = मानक_निकास;
	अगर (fn != शून्य) {
		काष्ठा stat st;
		पूर्णांक fd;

		resolved_fn = xmalloc_follow_symlinks(fn);
		अगर (resolved_fn == शून्य)
			bb_simple_perror_संदेश_and_die(fn);
		in = xfopen_for_read(resolved_fn);
		xfstat(fileno(in), &st, resolved_fn);

		अस्थायी_fn = xasprintf("%sXXXXXX", resolved_fn);
		fd = xmkstemp(अस्थायी_fn);
		अगर (fchmod(fd, st.st_mode) == -1)
			bb_simple_perror_संदेश_and_die(अस्थायी_fn);
		fchown(fd, st.st_uid, st.st_gid);

		out = xfdopen_for_write(fd);
	}

	जबतक ((ch = ख_अक्षर_लो(in)) != खातापूर्ण) {
		अगर (ch == '\r')
			जारी;
		अगर (ch == '\n')
			अगर (conv_type == CT_UNIX2DOS)
				ख_अक्षर_दो('\r', out);
		ख_अक्षर_दो(ch, out);
	}

	अगर (fn != शून्य) {
		अगर (ख_बंद(in) < 0 || ख_बंद(out) < 0) {
			unlink(अस्थायी_fn);
			bb_perror_nomsg_and_die();
		}
		xrename(अस्थायी_fn, resolved_fn);
		मुक्त(अस्थायी_fn);
		मुक्त(resolved_fn);
	}
}

पूर्णांक dos2unix_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक dos2unix_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	पूर्णांक o, conv_type;

	/* See if we are supposed to be doing dos2unix or unix2dos */
	अगर (ENABLE_DOS2UNIX
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
	अगर (o)
		conv_type = o;

	argv += optind;
	करो {
		/* might be convert(NULL) if there is no filename given */
		convert(*argv, conv_type);
	} जबतक (*argv && *++argv);

	वापस 0;
}
