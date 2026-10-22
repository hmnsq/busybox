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
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करोहि { setup_common_bufsiz(); } याव (0)

स्थिर शून्य tail_xprint_header(ध्रुव वर्ण *fmt, ध्रुव वर्ण *filename)
{
	सचे (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

स्थिर ssize_t tail_read(पूर्णाङ्क fd, वर्ण *buf, size_t गणनम्)
{
	ssize_t r;

	r = full_read(fd, buf, गणनम्);
	सचे (r < 0) {
		bb_simple_perror_सन्देशः(bb_सन्देशः_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	पटिनिवत्त r;
}

#define header_fmt_str "\n==> %s <==\n"

स्थिर unsigned eat_अङ्कः(ध्रुव वर्ण *p)
{
	सचे (*p == '-')
		p++;
	अञ्ञथा सचे (*p == '+') {
		p++;
		G.from_शिखरम् = 1;
	}
	पटिनिवत्त xatou_sfx(p, bkm_suffixes);
}

पूर्णाङ्क tail_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tail_main(पूर्णाङ्क argc, वर्ण **argv)
{
	unsigned गणनम् = 10;
	unsigned sleep_period = 1;
	ध्रुव वर्ण *str_c, *str_n;

	वर्ण *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	पूर्णाङ्क i, opt;

	पूर्णाङ्क *fds;
	ध्रुव वर्ण *fmt;
	पूर्णाङ्क prev_fd;

	INIT_G();

#सचे ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	सचे (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		गणनम् = eat_अङ्कः(argv[1]);
		argv++;
		argc--;
	}
#endif

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_TAIL("^")
			"fc:n:"IF_FEATURE_FANCY_TAIL("qs:+vF")
			IF_FEATURE_FANCY_TAIL("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_TAIL(,&sleep_period)
	);
#define FOLLOW (opt & 0x1)
#define गणनम्_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	सचे (opt & 0x2) गणनम् = eat_अङ्कः(str_c); // -c
	सचे (opt & 0x4) गणनम् = eat_अङ्कः(str_n); // -n
#सचे ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	सचे (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	सचे (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#अञ्ञथा
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(परिमाण(fds[0]) * (argc + 1));
	सचे (!argv[0]) {
		संरचना stat statbuf;

		सचे (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (वर्ण *) bb_सन्देशः_standard_निवेशः;
	}
	nfiles = i = 0;
	करोहि {
		पूर्णाङ्क fd = open_or_warn_stdin(argv[i]);
		सचे (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			अनुवर्त;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} याव (++i < argc);

	सचे (!nfiles)
		bb_simple_error_सन्देशः_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	सचे (!G.from_शिखरम् && गणनम्_BYTES) {
		सचे (tailbufsize < गणनम् + BUFSIZ) {
			tailbufsize = गणनम् + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = रिक्त;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	करोहि {
		वर्ण *buf;
		पूर्णाङ्क taillen;
		पूर्णाङ्क newlines_seen;
		unsigned seen;
		पूर्णाङ्क nread;
		पूर्णाङ्क fd = fds[i];

		सचे (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			अनुवर्त; /* may happen with -F */

		सचे (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		सचे (!G.from_शिखरम्) {
			off_t current = lseek(fd, 0, SEEK_END);
			सचे (current > 0) {
				unsigned off;
				सचे (गणनम्_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					सचे (गणनम् == 0)
						अनुवर्त; /* showing zero bytes is easy :) */
					current -= गणनम्;
					सचे (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, गणनम्);
					अनुवर्त;
				}
#सचे 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (गणनम् | 0xf); /* for small counts, be more paranoid */
				सचे (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				सचे (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		सचे (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		याव ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			सचे (G.from_शिखरम्) {
				पूर्णाङ्क nwrite = nread;
				सचे (seen < गणनम्) {
					/* We need to skip a few more bytes/lines */
					सचे (गणनम्_BYTES) {
						nwrite -= (गणनम् - seen);
						seen += nread;
					} अञ्ञथा {
						वर्ण *s = buf;
						करोहि {
							--nwrite;
							सचे (*s++ == '\n' && ++seen == गणनम्) {
								भङ्ग;
							}
						} याव (nwrite);
					}
				}
				सचे (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} अञ्ञथा सचे (गणनम्) {
				सचे (गणनम्_BYTES) {
					taillen += nread;
					सचे (taillen > (पूर्णाङ्क)गणनम्) {
						memmove(tailbuf, tailbuf + taillen - गणनम्, गणनम्);
						taillen = गणनम्;
					}
				} अञ्ञथा {
					पूर्णाङ्क k = nread;
					पूर्णाङ्क newlines_in_buf = 0;

					करोहि { /* count '\n' in last read */
						k--;
						सचे (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} याव (k);

					सचे (newlines_seen + newlines_in_buf < (पूर्णाङ्क)गणनम्) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} अञ्ञथा {
						पूर्णाङ्क extra = (buf[nread-1] != '\n');
						वर्ण *s;

						k = newlines_seen + newlines_in_buf + extra - गणनम्;
						s = tailbuf;
						याव (k) {
							सचे (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = गणनम् - extra;
					}
					सचे (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		सचे (!G.from_शिखरम्) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} याव (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = रिक्त;

	सचे (FOLLOW) याव (1) {
		sleep(sleep_period);

		i = 0;
		करोहि {
			पूर्णाङ्क nread;
			ध्रुव वर्ण *filename = argv[i];
			पूर्णाङ्क fd = fds[i];
			पूर्णाङ्क new_fd = -1;
			संरचना stat sbuf;

			सचे (FOLLOW_RETRY) {
				संरचना stat fsbuf;

				सचे (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					सचे (new_fd >= 0) {
						bb_error_सन्देशः("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						सचे (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} अञ्ञथा सचे (fd >= 0) {
						bb_perror_सन्देशः("%s has been renamed or deleted", filename);
					}
				}
			}
			सचे (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				अनुवर्त;
			सचे (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			अत्थाय (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				सचे (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					सचे (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				सचे (nread <= 0) {
					सचे (new_fd < 0)
						भङ्ग;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					अनुवर्त;
				}
				सचे (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = रिक्त;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} याव (++i < nfiles);
	} /* while (1) */

	सचे (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(fds);
		मुक्त(tailbuf);
	}
	पटिनिवत्त G.exitcode;
}
