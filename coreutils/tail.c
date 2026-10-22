/* vi: set sw=4 ts=4: */
/*
 * Mini tail implementation for busybox
 *
 * Copyright (C) 2001 by Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Pretty much rewritten to fix numerous bugs and reduce realloc() calls.
 * Bugs fixed (although I may have forgotten one or two... it was pretty bad)
 * 1) mixing printf/write without fflush()ing stdout
 * 2) no check that any open files are present
 * 3) optstring had -q taking an arg
 * 4) no error checking on write in some cases, and a warning even then
 * 5) q and s interaction bug
 * 6) no check for lseek error
 * 7) lseek attempted when count==0 even if arg was +0 (from top)
 */
//config:config TAIL
//config:	bool "tail (7.2 kb)"
//config:	default y
//config:	help
//config:	tail is used to print the last specified number of lines
//config:	from files.
//config:
//config:config FEATURE_FANCY_TAIL
//config:	bool "Enable -q, -s, -v, and -F options"
//config:	default y
//config:	depends on TAIL
//config:	help
//config:	These options are provided by GNU tail, but
//config:	are not specified in the SUSv3 standard:
//config:		-q      Never output headers giving file names
//config:		-s SEC  Wait SEC seconds between reads with -f
//config:		-v      Always output headers giving file names
//config:		-F      Same as -f, but keep retrying

//applet:IF_TAIL(APPLET(tail, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TAIL) += tail.o

/* BB_AUDIT SUSv3 compliant (need fancy for -c) */
/* BB_AUDIT GNU compatible -c, -q, and -v options in 'fancy' configuration. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/tail.html */

//usage:#define tail_trivial_usage
//usage:       "[OPTIONS] [FILE]..."
//usage:#define tail_full_usage "\n\n"
//usage:       "Print last 10 lines of FILEs (or stdin) to.\n"
//usage:       "With more than one FILE, precede each with a filename header.\n"
//usage:     "\n	-c [+]N[bkm]	Print last N bytes"
//usage:     "\n	-n N[bkm]	Print last N lines"
//usage:     "\n	-n +N[bkm]	Start on Nth line and print the rest"
//usage:     "\n			(b:*512 k:*1024 m:*1024^2)"
//usage:	IF_FEATURE_FANCY_TAIL(
//usage:     "\n	-q		Never print headers"
//usage:     "\n	-v		Always print headers"
//usage:	)
//usage:     "\n	-f		Print data as file grows"
//usage:	IF_FEATURE_FANCY_TAIL(
//usage:     "\n	-F		Same as -f, but keep retrying"
//usage:     "\n	-s SECONDS	Wait SECONDS between reads with -f"
//usage:	)
//usage:
//usage:#define tail_example_usage
//usage:       "$ tail -n 1 /etc/resolv.conf\n"
//usage:       "nameserver 10.0.0.1\n"

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

संरचना globals {
	bool from_शिखरम्;
	bool exitcode;
} FIX_ALIASING;
#परिभाषय G (*(संरचना globals*)bb_common_bufsiz1)
#परिभाषय INIT_G() कुरु { सज्जता_common_bufsiz(); } यावत् (0)

स्थिर शून्य पुच्छ_xprint_शीर्षकम्(ध्रुव वर्ण *fmt, ध्रुव वर्ण *filename)
{
	यदि (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

स्थिर ssize_t पुच्छ_read(पूर्णाङ्क fd, वर्ण *buf, आकार_t गणनम्)
{
	ssize_t r;

	r = पूर्णम्_read(fd, buf, गणनम्);
	यदि (r < 0) {
		bb_simple_perror_सन्देशः(bb_सन्देशः_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	प्रत्यागम r;
}

#परिभाषय शीर्षकम्_fmt_str "\n==> %s <==\n"

स्थिर अचिह्नित eat_अङ्कः(ध्रुव वर्ण *p)
{
	यदि (*p == '-')
		p++;
	अन्यथा यदि (*p == '+') {
		p++;
		G.from_शिखरम् = 1;
	}
	प्रत्यागम xatou_sfx(p, bkm_suffixes);
}

पूर्णाङ्क पुच्छ_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क पुच्छ_main(पूर्णाङ्क argc, वर्ण **argv)
{
	अचिह्नित गणनम् = 10;
	अचिह्नित sleep_period = 1;
	ध्रुव वर्ण *str_c, *str_n;

	वर्ण *tailbuf;
	आकार_t tailbufsize;
	अचिह्नित शीर्षकम्_देहली = 1;
	अचिह्नित nfiles;
	पूर्णाङ्क i, opt;

	पूर्णाङ्क *fds;
	ध्रुव वर्ण *fmt;
	पूर्णाङ्क पूर्व_fd;

	INIT_G();

#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_पुच्छ
	/* Allow legacy syntax of an initial numeric option without -n. */
	यदि (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && अङ्कः_वा(argv[1][1])
	) {
		गणनम् = eat_अङ्कः(argv[1]);
		argv++;
		argc--;
	}
#यदि_अन्तः

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_पुच्छ("^")
			"fc:n:"IF_FEATURE_FANCY_पुच्छ("qs:+vF")
			IF_FEATURE_FANCY_पुच्छ("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_पुच्छ(,&sleep_period)
	);
#परिभाषय FOLLOW (opt & 0x1)
#परिभाषय गणनम्_अष्टकानि_2 (opt & 0x2)
	//if (opt & 0x1) // -f
	यदि (opt & 0x2) गणनम् = eat_अङ्कः(str_c); // -c
	यदि (opt & 0x4) गणनम् = eat_अङ्कः(str_n); // -n
#यदि ENABLE_FEATURE_FANCY_पुच्छ
	/* q: make it impossible for nfiles to be > header_threshold */
	यदि (opt & 0x8) शीर्षकम्_देहली = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	यदि (opt & 0x20) शीर्षकम्_देहली = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#अन्यथा
# define FOLLOW_RETRY 0
#यदि_अन्तः
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(परिमाण(fds[0]) * (argc + 1));
	यदि (!argv[0]) {
		संरचना stat statbuf;

		यदि (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_बहुलकम्)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (वर्ण *) bb_सन्देशः_standard_निवेशः;
	}
	nfiles = i = 0;
	कुरु {
		पूर्णाङ्क fd = open_or_warn_stdin(argv[i]);
		यदि (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			अनुवर्त;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} यावत् (++i < argc);

	यदि (!nfiles)
		bb_simple_error_सन्देशः_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	यदि (!G.from_शिखरम् && गणनम्_अष्टकानि_2) {
		यदि (tailbufsize < गणनम् + BUFSIZ) {
			tailbufsize = गणनम् + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = रिक्त;

	/* tail the files */

	fmt = शीर्षकम्_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	कुरु {
		वर्ण *buf;
		पूर्णाङ्क taillen;
		पूर्णाङ्क newlines_आलोकितम्;
		अचिह्नित आलोकितम्;
		पूर्णाङ्क nread;
		पूर्णाङ्क fd = fds[i];

		यदि (ENABLE_FEATURE_FANCY_पुच्छ && fd < 0)
			अनुवर्त; /* may happen with -F */

		यदि (nfiles > शीर्षकम्_देहली) {
			पुच्छ_xprint_शीर्षकम्(fmt, argv[i]);
			fmt = शीर्षकम्_fmt_str;
		}

		यदि (!G.from_शिखरम्) {
			off_t current = lseek(fd, 0, SEEK_अन्तः);
			यदि (current > 0) {
				अचिह्नित off;
				यदि (गणनम्_अष्टकानि_2) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					यदि (गणनम् == 0)
						अनुवर्त; /* showing zero bytes is easy :) */
					current -= गणनम्;
					यदि (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_समुच्चयः);
					bb_copyfd_आकार(fd, STDOUT_FILENO, गणनम्);
					अनुवर्त;
				}
#यदि 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (गणनम् | 0xf); /* for small counts, be more paranoid */
				यदि (off > (अधिकतम_पूर्णाङ्कः / (64*1024)))
					off = (अधिकतम_पूर्णाङ्कः / (64*1024));
				current -= off * (64*1024);
				यदि (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_समुच्चयः);
#यदि_अन्तः
			}
		}

		यदि (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		आलोकितम् = 1;
		newlines_आलोकितम् = 0;
		यावत् ((nread = पुच्छ_read(fd, buf, tailbufsize - taillen)) > 0) {
			यदि (G.from_शिखरम्) {
				पूर्णाङ्क nwrite = nread;
				यदि (आलोकितम् < गणनम्) {
					/* We need to skip a few more bytes/lines */
					यदि (गणनम्_अष्टकानि_2) {
						nwrite -= (गणनम् - आलोकितम्);
						आलोकितम् += nread;
					} अन्यथा {
						वर्ण *s = buf;
						कुरु {
							--nwrite;
							यदि (*s++ == '\n' && ++आलोकितम् == गणनम्) {
								भङ्ग;
							}
						} यावत् (nwrite);
					}
				}
				यदि (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} अन्यथा यदि (गणनम्) {
				यदि (गणनम्_अष्टकानि_2) {
					taillen += nread;
					यदि (taillen > (पूर्णाङ्क)गणनम्) {
						memmove(tailbuf, tailbuf + taillen - गणनम्, गणनम्);
						taillen = गणनम्;
					}
				} अन्यथा {
					पूर्णाङ्क k = nread;
					पूर्णाङ्क newlines_अन्तर्_buf = 0;

					कुरु { /* count '\n' in last read */
						k--;
						यदि (buf[k] == '\n') {
							newlines_अन्तर्_buf++;
						}
					} यावत् (k);

					यदि (newlines_आलोकितम् + newlines_अन्तर्_buf < (पूर्णाङ्क)गणनम्) {
						newlines_आलोकितम् += newlines_अन्तर्_buf;
						taillen += nread;
					} अन्यथा {
						पूर्णाङ्क extra = (buf[nread-1] != '\n');
						वर्ण *s;

						k = newlines_आलोकितम् + newlines_अन्तर्_buf + extra - गणनम्;
						s = tailbuf;
						यावत् (k) {
							यदि (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_आलोकितम् = गणनम् - extra;
					}
					यदि (tailbufsize < (आकार_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		यदि (!G.from_शिखरम्) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} यावत् (++i < nfiles);
	पूर्व_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = रिक्त;

	यदि (FOLLOW) यावत् (1) {
		sleep(sleep_period);

		i = 0;
		कुरु {
			पूर्णाङ्क nread;
			ध्रुव वर्ण *filename = argv[i];
			पूर्णाङ्क fd = fds[i];
			पूर्णाङ्क new_fd = -1;
			संरचना stat sbuf;

			यदि (FOLLOW_RETRY) {
				संरचना stat fsbuf;

				यदि (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = उद्घाटय(filename, O_RDONLY);
					यदि (new_fd >= 0) {
						bb_error_सन्देशः("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						यदि (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} अन्यथा यदि (fd >= 0) {
						bb_perror_सन्देशः("%s has been renamed or deleted", filename);
					}
				}
			}
			यदि (ENABLE_FEATURE_FANCY_पुच्छ && fd < 0)
				अनुवर्त;
			यदि (nfiles > शीर्षकम्_देहली) {
				fmt = शीर्षकम्_fmt_str;
			}
			प्रति (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				यदि (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_आकार > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					यदि (sbuf.st_आकार < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_समुच्चयः);
					}
				}

				nread = पुच्छ_read(fd, tailbuf, BUFSIZ);
				यदि (nread <= 0) {
					यदि (new_fd < 0)
						भङ्ग;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					अनुवर्त;
				}
				यदि (fmt && (fd != पूर्व_fd)) {
					पुच्छ_xprint_शीर्षकम्(fmt, filename);
					fmt = रिक्त;
					पूर्व_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} यावत् (++i < nfiles);
	} /* while (1) */

	यदि (ENABLE_FEATURE_CLEAN_ऊर्ध्वम्) {
		मुक्त(fds);
		मुक्त(tailbuf);
	}
	प्रत्यागम G.exitcode;
}
