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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ಸಂರಚನಾ globals {
	bool from_ತುದಿ;
	bool exitcode;
} FIX_ALIASING;
#define G (*(ಸಂರಚನಾ globals*)bb_common_bufsiz1)
#define INIT_G() ಕುರು { setup_common_bufsiz(); } ಯಾವತ್ (0)

ಸ್ಥಿರ ಶೂನ್ಯ tail_xprint_header(ಧ್ರುವ ವರ್ಣ *fmt, ಧ್ರುವ ವರ್ಣ *filename)
{
	ಯದಿ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

ಸ್ಥಿರ ssize_t tail_read(ಪೂರ್ಣಾಙ್ಕ fd, ವರ್ಣ *buf, size_t ಎಣಿಕೆ)
{
	ssize_t r;

	r = full_read(fd, buf, ಎಣಿಕೆ);
	ಯದಿ (r < 0) {
		bb_simple_perror_ಸಂದೇಶ(bb_ಸಂದೇಶ_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	ಪ್ರತ್ಯಾಗಮ r;
}

#define header_fmt_str "\n==> %s <==\n"

ಸ್ಥಿರ unsigned eat_ಅಂಕೆ(ಧ್ರುವ ವರ್ಣ *p)
{
	ಯದಿ (*p == '-')
		p++;
	ಅನ್ಯಥಾ ಯದಿ (*p == '+') {
		p++;
		G.from_ತುದಿ = 1;
	}
	ಪ್ರತ್ಯಾಗಮ xatou_sfx(p, bkm_suffixes);
}

ಪೂರ್ಣಾಙ್ಕ tail_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ tail_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	unsigned ಎಣಿಕೆ = 10;
	unsigned sleep_period = 1;
	ಧ್ರುವ ವರ್ಣ *str_c, *str_n;

	ವರ್ಣ *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	ಪೂರ್ಣಾಙ್ಕ i, opt;

	ಪೂರ್ಣಾಙ್ಕ *fds;
	ಧ್ರುವ ವರ್ಣ *fmt;
	ಪೂರ್ಣಾಙ್ಕ prev_fd;

	INIT_G();

#ಯದಿ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	ಯದಿ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		ಎಣಿಕೆ = eat_ಅಂಕೆ(argv[1]);
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
#define ಎಣಿಕೆ_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	ಯದಿ (opt & 0x2) ಎಣಿಕೆ = eat_ಅಂಕೆ(str_c); // -c
	ಯದಿ (opt & 0x4) ಎಣಿಕೆ = eat_ಅಂಕೆ(str_n); // -n
#ಯದಿ ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	ಯದಿ (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ಯದಿ (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#ಅನ್ಯಥಾ
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(ಪರಿಮಾಣ(fds[0]) * (argc + 1));
	ಯದಿ (!argv[0]) {
		ಸಂರಚನಾ stat statbuf;

		ಯದಿ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (ವರ್ಣ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
	}
	nfiles = i = 0;
	ಕುರು {
		ಪೂರ್ಣಾಙ್ಕ fd = open_or_warn_stdin(argv[i]);
		ಯದಿ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			ಅನುವರ್ತ;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ಯಾವತ್ (++i < argc);

	ಯದಿ (!nfiles)
		bb_simple_error_ಸಂದೇಶ_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ಯದಿ (!G.from_ತುದಿ && ಎಣಿಕೆ_BYTES) {
		ಯದಿ (tailbufsize < ಎಣಿಕೆ + BUFSIZ) {
			tailbufsize = ಎಣಿಕೆ + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ರಿಕ್ತ;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	ಕುರು {
		ವರ್ಣ *buf;
		ಪೂರ್ಣಾಙ್ಕ taillen;
		ಪೂರ್ಣಾಙ್ಕ newlines_seen;
		unsigned seen;
		ಪೂರ್ಣಾಙ್ಕ nread;
		ಪೂರ್ಣಾಙ್ಕ fd = fds[i];

		ಯದಿ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			ಅನುವರ್ತ; /* may happen with -F */

		ಯದಿ (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		ಯದಿ (!G.from_ತುದಿ) {
			off_t current = lseek(fd, 0, SEEK_END);
			ಯದಿ (current > 0) {
				unsigned off;
				ಯದಿ (ಎಣಿಕೆ_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ಯದಿ (ಎಣಿಕೆ == 0)
						ಅನುವರ್ತ; /* showing zero bytes is easy :) */
					current -= ಎಣಿಕೆ;
					ಯದಿ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, ಎಣಿಕೆ);
					ಅನುವರ್ತ;
				}
#ಯದಿ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (ಎಣಿಕೆ | 0xf); /* for small counts, be more paranoid */
				ಯದಿ (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				ಯದಿ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		ಯದಿ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		ಯಾವತ್ ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			ಯದಿ (G.from_ತುದಿ) {
				ಪೂರ್ಣಾಙ್ಕ nwrite = nread;
				ಯದಿ (seen < ಎಣಿಕೆ) {
					/* We need to skip a few more bytes/lines */
					ಯದಿ (ಎಣಿಕೆ_BYTES) {
						nwrite -= (ಎಣಿಕೆ - seen);
						seen += nread;
					} ಅನ್ಯಥಾ {
						ವರ್ಣ *s = buf;
						ಕುರು {
							--nwrite;
							ಯದಿ (*s++ == '\n' && ++seen == ಎಣಿಕೆ) {
								ಭಙ್ಗ;
							}
						} ಯಾವತ್ (nwrite);
					}
				}
				ಯದಿ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} ಅನ್ಯಥಾ ಯದಿ (ಎಣಿಕೆ) {
				ಯದಿ (ಎಣಿಕೆ_BYTES) {
					taillen += nread;
					ಯದಿ (taillen > (ಪೂರ್ಣಾಙ್ಕ)ಎಣಿಕೆ) {
						memmove(tailbuf, tailbuf + taillen - ಎಣಿಕೆ, ಎಣಿಕೆ);
						taillen = ಎಣಿಕೆ;
					}
				} ಅನ್ಯಥಾ {
					ಪೂರ್ಣಾಙ್ಕ k = nread;
					ಪೂರ್ಣಾಙ್ಕ newlines_in_buf = 0;

					ಕುರು { /* count '\n' in last read */
						k--;
						ಯದಿ (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} ಯಾವತ್ (k);

					ಯದಿ (newlines_seen + newlines_in_buf < (ಪೂರ್ಣಾಙ್ಕ)ಎಣಿಕೆ) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} ಅನ್ಯಥಾ {
						ಪೂರ್ಣಾಙ್ಕ extra = (buf[nread-1] != '\n');
						ವರ್ಣ *s;

						k = newlines_seen + newlines_in_buf + extra - ಎಣಿಕೆ;
						s = tailbuf;
						ಯಾವತ್ (k) {
							ಯದಿ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = ಎಣಿಕೆ - extra;
					}
					ಯದಿ (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ಯದಿ (!G.from_ತುದಿ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ಯಾವತ್ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ರಿಕ್ತ;

	ಯದಿ (FOLLOW) ಯಾವತ್ (1) {
		sleep(sleep_period);

		i = 0;
		ಕುರು {
			ಪೂರ್ಣಾಙ್ಕ nread;
			ಧ್ರುವ ವರ್ಣ *filename = argv[i];
			ಪೂರ್ಣಾಙ್ಕ fd = fds[i];
			ಪೂರ್ಣಾಙ್ಕ new_fd = -1;
			ಸಂರಚನಾ stat sbuf;

			ಯದಿ (FOLLOW_RETRY) {
				ಸಂರಚನಾ stat fsbuf;

				ಯದಿ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ಯದಿ (new_fd >= 0) {
						bb_error_ಸಂದೇಶ("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ಯದಿ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} ಅನ್ಯಥಾ ಯದಿ (fd >= 0) {
						bb_perror_ಸಂದೇಶ("%s has been renamed or deleted", filename);
					}
				}
			}
			ಯದಿ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				ಅನುವರ್ತ;
			ಯದಿ (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			ಪ್ರತಿ (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ಯದಿ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ಯದಿ (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				ಯದಿ (nread <= 0) {
					ಯದಿ (new_fd < 0)
						ಭಙ್ಗ;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					ಅನುವರ್ತ;
				}
				ಯದಿ (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = ರಿಕ್ತ;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ಯಾವತ್ (++i < nfiles);
	} /* while (1) */

	ಯದಿ (ENABLE_FEATURE_CLEAN_UP) {
		ಮುಕ್ತ(fds);
		ಮುಕ್ತ(tailbuf);
	}
	ಪ್ರತ್ಯಾಗಮ G.exitcode;
}
