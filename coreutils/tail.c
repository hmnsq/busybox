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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "common_bufsiz.h"

ସଂରଚନା globals {
	bool from_ଶୀର୍ଷ;
	bool exitcode;
} FIX_ALIASING;
#define G (*(ସଂରଚନା globals*)bb_common_bufsiz1)
#define INIT_G() କର { setup_common_bufsiz(); } ଯେତେବେଳେ (0)

ସ୍ଥିର ଶୂନ୍ୟ tail_xprint_header(ଅଚଳ ଅକ୍ଷର *fmt, ଅଚଳ ଅକ୍ଷର *filename)
{
	ଯଦି (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

ସ୍ଥିର ssize_t tail_read(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅକ୍ଷର *buf, size_t ଗଣନା_2)
{
	ssize_t r;

	r = full_read(fd, buf, ଗଣନା_2);
	ଯଦି (r < 0) {
		bb_simple_perror_ସନ୍ଦେଶ(bb_ସନ୍ଦେଶ_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	ଫେରନ୍ତୁ r;
}

#define header_fmt_str "\n==> %s <==\n"

ସ୍ଥିର unsigned eat_ଅଙ୍କ(ଅଚଳ ଅକ୍ଷର *p)
{
	ଯଦି (*p == '-')
		p++;
	ଅନ୍ୟଥା ଯଦି (*p == '+') {
		p++;
		G.from_ଶୀର୍ଷ = 1;
	}
	ଫେରନ୍ତୁ xatou_sfx(p, bkm_suffixes);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tail_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା tail_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	unsigned ଗଣନା_2 = 10;
	unsigned sleep_period = 1;
	ଅଚଳ ଅକ୍ଷର *str_c, *str_n;

	ଅକ୍ଷର *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, opt;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *fds;
	ଅଚଳ ଅକ୍ଷର *fmt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev_fd;

	INIT_G();

#ଯଦି ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	ଯଦି (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		ଗଣନା_2 = eat_ଅଙ୍କ(argv[1]);
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
#define ଗଣନା_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	ଯଦି (opt & 0x2) ଗଣନା_2 = eat_ଅଙ୍କ(str_c); // -c
	ଯଦି (opt & 0x4) ଗଣନା_2 = eat_ଅଙ୍କ(str_n); // -n
#ଯଦି ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	ଯଦି (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ଯଦି (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#ଅନ୍ୟଥା
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(ଆକାର(fds[0]) * (argc + 1));
	ଯଦି (!argv[0]) {
		ସଂରଚନା stat statbuf;

		ଯଦି (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (ଅକ୍ଷର *) bb_ସନ୍ଦେଶ_standard_ନିବେଶ;
	}
	nfiles = i = 0;
	କର {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open_or_warn_stdin(argv[i]);
		ଯଦି (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			ଜାରି;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ଯେତେବେଳେ (++i < argc);

	ଯଦି (!nfiles)
		bb_simple_error_ସନ୍ଦେଶ_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ଯଦି (!G.from_ଶୀର୍ଷ && ଗଣନା_BYTES) {
		ଯଦି (tailbufsize < ଗଣନା_2 + BUFSIZ) {
			tailbufsize = ଗଣନା_2 + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ଖାଲି;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	କର {
		ଅକ୍ଷର *buf;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା taillen;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା newlines_seen;
		unsigned seen;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା nread;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = fds[i];

		ଯଦି (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			ଜାରି; /* may happen with -F */

		ଯଦି (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		ଯଦି (!G.from_ଶୀର୍ଷ) {
			off_t current = lseek(fd, 0, SEEK_END);
			ଯଦି (current > 0) {
				unsigned off;
				ଯଦି (ଗଣନା_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ଯଦି (ଗଣନା_2 == 0)
						ଜାରି; /* showing zero bytes is easy :) */
					current -= ଗଣନା_2;
					ଯଦି (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, ଗଣନା_2);
					ଜାରି;
				}
#ଯଦି 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (ଗଣନା_2 | 0xf); /* for small counts, be more paranoid */
				ଯଦି (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				ଯଦି (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		ଯଦି (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		ଯେତେବେଳେ ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			ଯଦି (G.from_ଶୀର୍ଷ) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା nwrite = nread;
				ଯଦି (seen < ଗଣନା_2) {
					/* We need to skip a few more bytes/lines */
					ଯଦି (ଗଣନା_BYTES) {
						nwrite -= (ଗଣନା_2 - seen);
						seen += nread;
					} ଅନ୍ୟଥା {
						ଅକ୍ଷର *s = buf;
						କର {
							--nwrite;
							ଯଦି (*s++ == '\n' && ++seen == ଗଣନା_2) {
								ଭାଙ୍ଗ;
							}
						} ଯେତେବେଳେ (nwrite);
					}
				}
				ଯଦି (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} ଅନ୍ୟଥା ଯଦି (ଗଣନା_2) {
				ଯଦି (ଗଣନା_BYTES) {
					taillen += nread;
					ଯଦି (taillen > (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଗଣନା_2) {
						memmove(tailbuf, tailbuf + taillen - ଗଣନା_2, ଗଣନା_2);
						taillen = ଗଣନା_2;
					}
				} ଅନ୍ୟଥା {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = nread;
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା newlines_in_buf = 0;

					କର { /* count '\n' in last read */
						k--;
						ଯଦି (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} ଯେତେବେଳେ (k);

					ଯଦି (newlines_seen + newlines_in_buf < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଗଣନା_2) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} ଅନ୍ୟଥା {
						ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = (buf[nread-1] != '\n');
						ଅକ୍ଷର *s;

						k = newlines_seen + newlines_in_buf + extra - ଗଣନା_2;
						s = tailbuf;
						ଯେତେବେଳେ (k) {
							ଯଦି (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = ଗଣନା_2 - extra;
					}
					ଯଦି (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ଯଦି (!G.from_ଶୀର୍ଷ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ଯେତେବେଳେ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ଖାଲି;

	ଯଦି (FOLLOW) ଯେତେବେଳେ (1) {
		sleep(sleep_period);

		i = 0;
		କର {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା nread;
			ଅଚଳ ଅକ୍ଷର *filename = argv[i];
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = fds[i];
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_fd = -1;
			ସଂରଚନା stat sbuf;

			ଯଦି (FOLLOW_RETRY) {
				ସଂରଚନା stat fsbuf;

				ଯଦି (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ଯଦି (new_fd >= 0) {
						bb_error_ସନ୍ଦେଶ("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ଯଦି (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} ଅନ୍ୟଥା ଯଦି (fd >= 0) {
						bb_perror_ସନ୍ଦେଶ("%s has been renamed or deleted", filename);
					}
				}
			}
			ଯଦି (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				ଜାରି;
			ଯଦି (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			ପାଇଁ (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ଯଦି (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ଯଦି (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				ଯଦି (nread <= 0) {
					ଯଦି (new_fd < 0)
						ଭାଙ୍ଗ;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					ଜାରି;
				}
				ଯଦି (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = ଖାଲି;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ଯେତେବେଳେ (++i < nfiles);
	} /* while (1) */

	ଯଦି (ENABLE_FEATURE_CLEAN_UP) {
		ମୁକ୍ତ(fds);
		ମୁକ୍ତ(tailbuf);
	}
	ଫେରନ୍ତୁ G.exitcode;
}
