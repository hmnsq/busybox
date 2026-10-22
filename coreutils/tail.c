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

#include "libbb.h"
#include "common_bufsiz.h"

sa_nracanaa globals {
	bool from_top;
	bool exitcode;
} FIX_ALIASING;
#define G (*(sa_nracanaa globals*)bb_common_bufsiz1)
#define INIT_G() do { setup_common_bufsiz(); } ខណៈពេល (0)

static void tail_xprint_header(const var_na *fmt, const var_na *filename)
{
	ប្រសិនបើ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

static ssize_t tail_read(ចំនួនគត់ fd, var_na *buf, size_t count)
{
	ssize_t r;

	r = full_read(fd, buf, count);
	ប្រសិនបើ (r < 0) {
		bb_simple_perror_msg(bb_msg_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	ត្រឡប់ r;
}

#define header_fmt_str "\n==> %s <==\n"

static unsigned eat_num(const var_na *p)
{
	ប្រសិនបើ (*p == '-')
		p++;
	បើមិនដូច្នេះ ប្រសិនបើ (*p == '+') {
		p++;
		G.from_top = 1;
	}
	ត្រឡប់ xatou_sfx(p, bkm_suffixes);
}

ចំនួនគត់ tail_main(ចំនួនគត់ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ចំនួនគត់ tail_main(ចំនួនគត់ argc, var_na **argv)
{
	unsigned count = 10;
	unsigned sleep_period = 1;
	const var_na *str_c, *str_n;

	var_na *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	ចំនួនគត់ i, opt;

	ចំនួនគត់ *fds;
	const var_na *fmt;
	ចំនួនគត់ prev_fd;

	INIT_G();

#ប្រសិនបើ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	ប្រសិនបើ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		count = eat_num(argv[1]);
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
#define COUNT_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	ប្រសិនបើ (opt & 0x2) count = eat_num(str_c); // -c
	ប្រសិនបើ (opt & 0x4) count = eat_num(str_n); // -n
#ប្រសិនបើ ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	ប្រសិនបើ (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ប្រសិនបើ (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#បើមិនដូច្នេះ
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(parimaa_na(fds[0]) * (argc + 1));
	ប្រសិនបើ (!argv[0]) {
		sa_nracanaa stat statbuf;

		ប្រសិនបើ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (var_na *) bb_msg_standard_input;
	}
	nfiles = i = 0;
	do {
		ចំនួនគត់ fd = open_or_warn_stdin(argv[i]);
		ប្រសិនបើ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			continue;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ខណៈពេល (++i < argc);

	ប្រសិនបើ (!nfiles)
		bb_simple_error_msg_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ប្រសិនបើ (!G.from_top && COUNT_BYTES) {
		ប្រសិនបើ (tailbufsize < count + BUFSIZ) {
			tailbufsize = count + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = NULL;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	do {
		var_na *buf;
		ចំនួនគត់ taillen;
		ចំនួនគត់ newlines_seen;
		unsigned seen;
		ចំនួនគត់ nread;
		ចំនួនគត់ fd = fds[i];

		ប្រសិនបើ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			continue; /* may happen with -F */

		ប្រសិនបើ (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		ប្រសិនបើ (!G.from_top) {
			off_t current = lseek(fd, 0, SEEK_END);
			ប្រសិនបើ (current > 0) {
				unsigned off;
				ប្រសិនបើ (COUNT_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ប្រសិនបើ (count == 0)
						continue; /* showing zero bytes is easy :) */
					current -= count;
					ប្រសិនបើ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, count);
					continue;
				}
#ប្រសិនបើ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (count | 0xf); /* for small counts, be more paranoid */
				ប្រសិនបើ (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				ប្រសិនបើ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		ប្រសិនបើ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		ខណៈពេល ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			ប្រសិនបើ (G.from_top) {
				ចំនួនគត់ nwrite = nread;
				ប្រសិនបើ (seen < count) {
					/* We need to skip a few more bytes/lines */
					ប្រសិនបើ (COUNT_BYTES) {
						nwrite -= (count - seen);
						seen += nread;
					} បើមិនដូច្នេះ {
						var_na *s = buf;
						do {
							--nwrite;
							ប្រសិនបើ (*s++ == '\n' && ++seen == count) {
								bha_nga;
							}
						} ខណៈពេល (nwrite);
					}
				}
				ប្រសិនបើ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} បើមិនដូច្នេះ ប្រសិនបើ (count) {
				ប្រសិនបើ (COUNT_BYTES) {
					taillen += nread;
					ប្រសិនបើ (taillen > (ចំនួនគត់)count) {
						memmove(tailbuf, tailbuf + taillen - count, count);
						taillen = count;
					}
				} បើមិនដូច្នេះ {
					ចំនួនគត់ k = nread;
					ចំនួនគត់ newlines_in_buf = 0;

					do { /* count '\n' in last read */
						k--;
						ប្រសិនបើ (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} ខណៈពេល (k);

					ប្រសិនបើ (newlines_seen + newlines_in_buf < (ចំនួនគត់)count) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} បើមិនដូច្នេះ {
						ចំនួនគត់ extra = (buf[nread-1] != '\n');
						var_na *s;

						k = newlines_seen + newlines_in_buf + extra - count;
						s = tailbuf;
						ខណៈពេល (k) {
							ប្រសិនបើ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = count - extra;
					}
					ប្រសិនបើ (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ប្រសិនបើ (!G.from_top) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ខណៈពេល (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = NULL;

	ប្រសិនបើ (FOLLOW) ខណៈពេល (1) {
		sleep(sleep_period);

		i = 0;
		do {
			ចំនួនគត់ nread;
			const var_na *filename = argv[i];
			ចំនួនគត់ fd = fds[i];
			ចំនួនគត់ new_fd = -1;
			sa_nracanaa stat sbuf;

			ប្រសិនបើ (FOLLOW_RETRY) {
				sa_nracanaa stat fsbuf;

				ប្រសិនបើ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ប្រសិនបើ (new_fd >= 0) {
						bb_error_msg("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ប្រសិនបើ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} បើមិនដូច្នេះ ប្រសិនបើ (fd >= 0) {
						bb_perror_msg("%s has been renamed or deleted", filename);
					}
				}
			}
			ប្រសិនបើ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				continue;
			ប្រសិនបើ (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			សម្រាប់ (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ប្រសិនបើ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ប្រសិនបើ (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				ប្រសិនបើ (nread <= 0) {
					ប្រសិនបើ (new_fd < 0)
						bha_nga;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					continue;
				}
				ប្រសិនបើ (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = NULL;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ខណៈពេល (++i < nfiles);
	} /* while (1) */

	ប្រសិនបើ (ENABLE_FEATURE_CLEAN_UP) {
		free(fds);
		free(tailbuf);
	}
	ត្រឡប់ G.exitcode;
}
