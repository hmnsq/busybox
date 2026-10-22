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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

সংরচনা globals {
	bool from_শীর্ষ;
	bool exitcode;
} FIX_ALIASING;
#define G (*(সংরচনা globals*)bb_common_bufsiz1)
#define INIT_G() কুরু { setup_common_bufsiz(); } যাবত্ (0)

স্থির শূন্য tail_xprint_header(ধ্রুব বর্ণ *fmt, ধ্রুব বর্ণ *filename)
{
	যদি (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

স্থির ssize_t tail_read(পূর্ণাঙ্ক fd, বর্ণ *buf, size_t গুনতি)
{
	ssize_t r;

	r = full_read(fd, buf, গুনতি);
	যদি (r < 0) {
		bb_simple_perror_বার্তা(bb_বার্তা_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	প্রত্যাগম r;
}

#define header_fmt_str "\n==> %s <==\n"

স্থির unsigned eat_অঙ্ক(ধ্রুব বর্ণ *p)
{
	যদি (*p == '-')
		p++;
	অন্যথা যদি (*p == '+') {
		p++;
		G.from_শীর্ষ = 1;
	}
	প্রত্যাগম xatou_sfx(p, bkm_suffixes);
}

পূর্ণাঙ্ক tail_main(পূর্ণাঙ্ক argc, বর্ণ **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণাঙ্ক tail_main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	unsigned গুনতি = 10;
	unsigned sleep_period = 1;
	ধ্রুব বর্ণ *str_c, *str_n;

	বর্ণ *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	পূর্ণাঙ্ক i, opt;

	পূর্ণাঙ্ক *fds;
	ধ্রুব বর্ণ *fmt;
	পূর্ণাঙ্ক prev_fd;

	INIT_G();

#যদি ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	যদি (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		গুনতি = eat_অঙ্ক(argv[1]);
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
#define গুনতি_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	যদি (opt & 0x2) গুনতি = eat_অঙ্ক(str_c); // -c
	যদি (opt & 0x4) গুনতি = eat_অঙ্ক(str_n); // -n
#যদি ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	যদি (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	যদি (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#অন্যথা
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(পরিমাণ(fds[0]) * (argc + 1));
	যদি (!argv[0]) {
		সংরচনা stat statbuf;

		যদি (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (বর্ণ *) bb_বার্তা_standard_প্রবেশ;
	}
	nfiles = i = 0;
	কুরু {
		পূর্ণাঙ্ক fd = open_or_warn_stdin(argv[i]);
		যদি (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			অনুবর্ত;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} যাবত্ (++i < argc);

	যদি (!nfiles)
		bb_simple_error_বার্তা_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	যদি (!G.from_শীর্ষ && গুনতি_BYTES) {
		যদি (tailbufsize < গুনতি + BUFSIZ) {
			tailbufsize = গুনতি + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = রিক্ত;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	কুরু {
		বর্ণ *buf;
		পূর্ণাঙ্ক taillen;
		পূর্ণাঙ্ক newlines_seen;
		unsigned seen;
		পূর্ণাঙ্ক nread;
		পূর্ণাঙ্ক fd = fds[i];

		যদি (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			অনুবর্ত; /* may happen with -F */

		যদি (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		যদি (!G.from_শীর্ষ) {
			off_t current = lseek(fd, 0, SEEK_END);
			যদি (current > 0) {
				unsigned off;
				যদি (গুনতি_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					যদি (গুনতি == 0)
						অনুবর্ত; /* showing zero bytes is easy :) */
					current -= গুনতি;
					যদি (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, গুনতি);
					অনুবর্ত;
				}
#যদি 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (গুনতি | 0xf); /* for small counts, be more paranoid */
				যদি (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				যদি (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		যদি (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		যাবত্ ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			যদি (G.from_শীর্ষ) {
				পূর্ণাঙ্ক nwrite = nread;
				যদি (seen < গুনতি) {
					/* We need to skip a few more bytes/lines */
					যদি (গুনতি_BYTES) {
						nwrite -= (গুনতি - seen);
						seen += nread;
					} অন্যথা {
						বর্ণ *s = buf;
						কুরু {
							--nwrite;
							যদি (*s++ == '\n' && ++seen == গুনতি) {
								ভঙ্গ;
							}
						} যাবত্ (nwrite);
					}
				}
				যদি (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} অন্যথা যদি (গুনতি) {
				যদি (গুনতি_BYTES) {
					taillen += nread;
					যদি (taillen > (পূর্ণাঙ্ক)গুনতি) {
						memmove(tailbuf, tailbuf + taillen - গুনতি, গুনতি);
						taillen = গুনতি;
					}
				} অন্যথা {
					পূর্ণাঙ্ক k = nread;
					পূর্ণাঙ্ক newlines_in_buf = 0;

					কুরু { /* count '\n' in last read */
						k--;
						যদি (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} যাবত্ (k);

					যদি (newlines_seen + newlines_in_buf < (পূর্ণাঙ্ক)গুনতি) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} অন্যথা {
						পূর্ণাঙ্ক extra = (buf[nread-1] != '\n');
						বর্ণ *s;

						k = newlines_seen + newlines_in_buf + extra - গুনতি;
						s = tailbuf;
						যাবত্ (k) {
							যদি (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = গুনতি - extra;
					}
					যদি (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		যদি (!G.from_শীর্ষ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} যাবত্ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = রিক্ত;

	যদি (FOLLOW) যাবত্ (1) {
		sleep(sleep_period);

		i = 0;
		কুরু {
			পূর্ণাঙ্ক nread;
			ধ্রুব বর্ণ *filename = argv[i];
			পূর্ণাঙ্ক fd = fds[i];
			পূর্ণাঙ্ক new_fd = -1;
			সংরচনা stat sbuf;

			যদি (FOLLOW_RETRY) {
				সংরচনা stat fsbuf;

				যদি (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					যদি (new_fd >= 0) {
						bb_error_বার্তা("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						যদি (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} অন্যথা যদি (fd >= 0) {
						bb_perror_বার্তা("%s has been renamed or deleted", filename);
					}
				}
			}
			যদি (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				অনুবর্ত;
			যদি (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			প্রতি (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				যদি (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					যদি (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				যদি (nread <= 0) {
					যদি (new_fd < 0)
						ভঙ্গ;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					অনুবর্ত;
				}
				যদি (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = রিক্ত;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} যাবত্ (++i < nfiles);
	} /* while (1) */

	যদি (ENABLE_FEATURE_CLEAN_UP) {
		মুক্ত(fds);
		মুক্ত(tailbuf);
	}
	প্রত্যাগম G.exitcode;
}
