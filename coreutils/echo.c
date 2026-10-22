/* vi: set sw=4 ts=4: */
/*
 * echo implementation for busybox
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * Original copyright notice is retained at the end of this file.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Because of behavioral differences, implemented configurable SUSv3
 * or 'fancy' gnu-ish behaviors.  Also, reduced size and fixed bugs.
 * 1) In handling '\c' escape, the previous version only suppressed the
 *     trailing newline.  SUSv3 specifies _no_ output after '\c'.
 * 2) SUSv3 specifies that octal escapes are of the form \0{#{#{#}}}.
 *    The previous version did not allow 4-digit octals.
 */
//config:config ECHO
//config:	bool "echo (2 kb)"
//config:	default y
//config:	help
//config:	echo prints a specified string to stdout.
//config:
//config:# this entry also appears in shell/Config.in, next to the echo builtin
//config:config FEATURE_FANCY_ECHO
//config:	bool "Enable -n and -e options"
//config:	default y
//config:	depends on ECHO || ASH_ECHO || HUSH_ECHO

//applet:IF_ECHO(APPLET_NOFORK(echo, echo, BB_DIR_BIN, BB_SUID_DROP, echo))

//kbuild:lib-$(CONFIG_ECHO) += echo.o

//kbuild:lib-$(CONFIG_ASH_ECHO)  += echo.o
//kbuild:lib-$(CONFIG_HUSH_ECHO) += echo.o

/* BB_AUDIT SUSv3 compliant -- unless configured as fancy echo. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/echo.html */

//usage:#define echo_trivial_usage
//usage:	IF_FEATURE_FANCY_ECHO("[-neE] ") "[ARG]..."
//usage:#define echo_full_usage "\n\n"
//usage:       "Print ARGs to stdout"
//usage:	IF_FEATURE_FANCY_ECHO( "\n"
//usage:     "\n	-n	No trailing newline"
//usage:     "\n	-e	Interpret backslash escapes (\\t=tab etc)"
//usage:     "\n	-E	Don't interpret backslash escapes (default)"
//usage:	)
//usage:
//usage:#define echo_example_usage
//usage:       "$ echo \"Erik is cool\"\n"
//usage:       "Erik is cool\n"
//usage:	IF_FEATURE_FANCY_ECHO("$ echo -e \"Erik\\nis\\ncool\"\n"
//usage:       "Erik\n"
//usage:       "is\n"
//usage:       "cool\n"
//usage:       "$ echo \"Erik\\nis\\ncool\"\n"
//usage:       "Erik\\nis\\ncool\n")

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOFORK applet. Be very careful! */

/* NB: can be used by shell even if not enabled as applet */

/*
 * NB2: we don't use stdio, we need better error handing.
 * Examples include writing into non-opened stdout and error on write.
 *
 * With stdio, output gets shoveled into stdout buffer, and even
 * fflush cannot clear it out. It seems that even if libc receives
 * EBADF on write attempts, it feels determined to output data no matter what.
 * If echo is called by shell, it will try writing again later, and possibly
 * will clobber future output. Not good.
 *
 * Solaris has fpurge which discards buffered input. glibc has __fpurge.
 * But this function is not standard.
 */

ಪೂರ್ಣಾಂಕ echo_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ **pp;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg;
	ಅಕ್ಷರ *ಹೊರಹೋಗು;
	ಅಕ್ಷರ *ಬಫರ್;
	ಚಿಹ್ನೆರಹಿತ buflen;
	ಪೂರ್ಣಾಂಕ err;
#ಆದರೆ !ENABLE_FEATURE_FANCY_ECHO
	ಎಣಿಕೆ {
		eflag = 0,  /* 0 -- disable escape sequences */
		nflag = 1,  /* 1 -- print '\n' */
	};

	argv++;
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ nflag = 1;
	ಅಕ್ಷರ eflag = 0;

	ಆಗ ((arg = *++argv) != ಖಾಲಿ) {
		ಅಕ್ಷರ n, e;

		ಆದರೆ (arg[0] != '-')
			ನಿಲ್ಲಿಸು; /* not an option arg, echo it */

		/* If it appears that we are handling options, then make sure
		 * that all of the options specified are actually valid.
		 * Otherwise, the string should just be echoed.
		 */
		arg++;
		n = nflag;
		e = eflag;
		ಮಾಡು {
			ಆದರೆ (*arg == 'n')
				n = 0;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*arg == 'e')
				e = '\\';
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*arg != 'E') {
				/* "-ccc" arg with one of c's invalid, echo it */
				/* arg consisting from just "-" also handled here */
				ಹೋಗು just_echo;
			}
		} ಆಗ (*++arg);
		nflag = n;
		eflag = e;
	}
 just_echo:
#ಆದರೆ_ಅಂತ್ಯ

	buflen = 0;
	pp = argv;
	ಆಗ ((arg = *pp) != ಖಾಲಿ) {
		buflen += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(arg) + 1;
		pp++;
	}
	ಹೊರಹೋಗು = ಬಫರ್ = xmalloc(buflen + 1); /* +1 is needed for "no args" case */

	ಆಗ ((arg = *argv) != ಖಾಲಿ) {
		ಪೂರ್ಣಾಂಕ c;

		ಆದರೆ (!eflag) {
			/* optimization for very common case */
			ಹೊರಹೋಗು = stpcpy(ಹೊರಹೋಗು, arg);
		} ಇಲ್ಲದಿದ್ದರೆ
		ಆಗ ((c = *arg++) != '\0') {
			ಆದರೆ (c == eflag) {
				/* This is an "\x" sequence */

				ಆದರೆ (*arg == 'c') {
					/* "\c" means cancel newline and
					 * ignore all subsequent chars. */
					ಹೋಗು do_write;
				}
				/* Since SUSv3 mandates a first digit of 0, 4-digit octals
				* of the form \0### are accepted. */
				ಆದರೆ (*arg == '0') {
					ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(arg[1] - '0') < 8) {
						/* 2nd char is 0..7: skip leading '0' */
						arg++;
					}
				}
				/* bb_process_escape_sequence handles NUL correctly
				 * ("...\" case). */
				{
					/* optimization: don't force arg to be on-stack,
					 * use another variable for that. ~30 bytes win */
					ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = arg;
					c = bb_process_escape_sequence(&z);
					arg = z;
				}
			}
			*ಹೊರಹೋಗು++ = c;
		}

		ಆದರೆ (!*++argv)
			ನಿಲ್ಲಿಸು;
		*ಹೊರಹೋಗು++ = ' ';
	}

	ಆದರೆ (nflag) {
		*ಹೊರಹೋಗು++ = '\n';
	}

 do_write:
	/* Careful to error out on partial writes too (think ENOSPC!) */
	errno = 0;
	err = ತುಂಬಿದ_write(STDOUT_FILENO, ಬಫರ್, ಹೊರಹೋಗು - ಬಫರ್) != ಹೊರಹೋಗು - ಬಫರ್;
	ಆದರೆ (err) {
		bb_simple_perror_ಸಂದೇಶ(bb_ಸಂದೇಶ_write_error);
	}
	ಬಿಡುಗಡೆ(ಬಫರ್);
	ಹಿಂತಿರುಗಿ err;
}

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. <BSD Advertising Clause omitted per the July 22, 1999 licensing change
 *		ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change>
 *
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)echo.c	8.1 (Berkeley) 5/31/93
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಆವೃತ್ತಿ_WITH_WRITEV
/* We can't use stdio.
 * The reason for this is highly non-obvious.
 * echo_main is used from shell. Shell must correctly handle "echo foo"
 * if stdout is closed. With stdio, output gets shoveled into
 * stdout buffer, and even fflush cannot clear it out. It seems that
 * even if libc receives EBADF on write attempts, it feels determined
 * to output data no matter what. So it will try later,
 * and possibly will clobber future output. Not good.
 *
 * Using writev instead, with 'direct' conversion of argv vector.
 */

ಪೂರ್ಣಾಂಕ echo_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ರಚನೆ iovec io[argc];
	ರಚನೆ iovec *cur_io = io;
	ಅಕ್ಷರ *arg;
	ಅಕ್ಷರ *p;
#ಆದರೆ !ENABLE_FEATURE_FANCY_ECHO
	ಎಣಿಕೆ {
		eflag = '\\',
		nflag = 1,  /* 1 -- print '\n' */
	};
	arg = *++argv;
	ಆದರೆ (!arg)
		ಹೋಗು newline_ret;
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ nflag = 1;
	ಅಕ್ಷರ eflag = 0;

	ಆಗ (1) {
		arg = *++argv;
		ಆದರೆ (!arg)
			ಹೋಗು newline_ret;
		ಆದರೆ (*arg != '-')
			ನಿಲ್ಲಿಸು;

		/* If it appears that we are handling options, then make sure
		 * that all of the options specified are actually valid.
		 * Otherwise, the string should just be echoed.
		 */
		p = arg + 1;
		ಆದರೆ (!*p)	/* A single '-', so echo it. */
			ಹೋಗು just_echo;

		ಮಾಡು {
			ಆದರೆ (!strchr("neE", *p))
				ಹೋಗು just_echo;
		} ಆಗ (*++p);

		/* All of the options in this arg are valid, so handle them. */
		p = arg + 1;
		ಮಾಡು {
			ಆದರೆ (*p == 'n')
				nflag = 0;
			ಆದರೆ (*p == 'e')
				eflag = '\\';
		} ಆಗ (*++p);
	}
 just_echo:
#ಆದರೆ_ಅಂತ್ಯ

	ಆಗ (1) {
		/* arg is already == *argv and isn't NULL */
		ಪೂರ್ಣಾಂಕ c;

		cur_io->iov_ತಳ = p = arg;

		ಆದರೆ (!eflag) {
			/* optimization for very common case */
			p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(arg);
		} ಇಲ್ಲದಿದ್ದರೆ ಆಗ ((c = *arg++)) {
			ಆದರೆ (c == eflag) {
				/* This is an "\x" sequence */

				ಆದರೆ (*arg == 'c') {
					/* "\c" means cancel newline and
					 * ignore all subsequent chars. */
					cur_io->iov_len = p - (ಅಕ್ಷರ*)cur_io->iov_ತಳ;
					cur_io++;
					ಹೋಗು ret;
				}
				/* Since SUSv3 mandates a first digit of 0, 4-digit octals
				* of the form \0### are accepted. */
				ಆದರೆ (*arg == '0' && (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(arg[1] - '0') < 8) {
					arg++;
				}
//FIXME? we also accept non-0 starting sequences (see echo-prints-slash_41 test)
// echo -ne '-\41-' prints "-!-". bash 5.0.17 does not (prints "-\41-").
				/* bb_process_escape_sequence can handle nul correctly */
				c = bb_process_escape_sequence( (ಶೂನ್ಯ*) &arg);
			}
			*p++ = c;
		}

		arg = *++argv;
		ಆದರೆ (arg)
			*p++ = ' ';
		cur_io->iov_len = p - (ಅಕ್ಷರ*)cur_io->iov_ತಳ;
		cur_io++;
		ಆದರೆ (!arg)
			ನಿಲ್ಲಿಸು;
	}

 newline_ret:
	ಆದರೆ (nflag) {
		cur_io->iov_ತಳ = (ಅಕ್ಷರ*)"\n";
		cur_io->iov_len = 1;
		cur_io++;
	}
 ret:
	/* TODO: implement and use full_writev? */
	ಹಿಂತಿರುಗಿ writev(1, io, (cur_io - io)) >= 0;
}
#ಆದರೆ_ಅಂತ್ಯ
