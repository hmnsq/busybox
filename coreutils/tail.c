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

struct globals {
	bool from_top;
	bool exitcode;
} FIX_ALIASING;
#define G (*(struct globals*)bb_common_bufsiz1)
#define INIT_G() do { setup_common_bufsiz(); } 当 (0)

static void tail_xprint_header(const char *fmt, const char *filename)
{
	如果 (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

static ssize_t tail_read(整数 fd, char *buf, size_t count)
{
	ssize_t r;

	r = full_read(fd, buf, count);
	如果 (r < 0) {
		bb_simple_perror_msg(bb_msg_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	返回 r;
}

#define header_fmt_str "\n==> %s <==\n"

static unsigned eat_num(const char *p)
{
	如果 (*p == '-')
		p++;
	否则 如果 (*p == '+') {
		p++;
		G.from_top = 1;
	}
	返回 xatou_sfx(p, bkm_suffixes);
}

整数 tail_main(整数 argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
整数 tail_main(整数 argc, char **argv)
{
	unsigned count = 10;
	unsigned sleep_period = 1;
	const char *str_c, *str_n;

	char *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	整数 i, opt;

	整数 *fds;
	const char *fmt;
	整数 prev_fd;

	INIT_G();

#如果 ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	如果 (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
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
	如果 (opt & 0x2) count = eat_num(str_c); // -c
	如果 (opt & 0x4) count = eat_num(str_n); // -n
#如果 ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	如果 (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	如果 (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#否则
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(sizeof(fds[0]) * (argc + 1));
	如果 (!argv[0]) {
		struct stat statbuf;

		如果 (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (char *) bb_msg_standard_input;
	}
	nfiles = i = 0;
	do {
		整数 fd = open_or_warn_stdin(argv[i]);
		如果 (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			continue;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} 当 (++i < argc);

	如果 (!nfiles)
		bb_simple_error_msg_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	如果 (!G.from_top && COUNT_BYTES) {
		如果 (tailbufsize < count + BUFSIZ) {
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
		char *buf;
		整数 taillen;
		整数 newlines_seen;
		unsigned seen;
		整数 nread;
		整数 fd = fds[i];

		如果 (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			continue; /* may happen with -F */

		如果 (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		如果 (!G.from_top) {
			off_t current = lseek(fd, 0, SEEK_END);
			如果 (current > 0) {
				unsigned off;
				如果 (COUNT_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					如果 (count == 0)
						continue; /* showing zero bytes is easy :) */
					current -= count;
					如果 (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, count);
					continue;
				}
#如果 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (count | 0xf); /* for small counts, be more paranoid */
				如果 (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				如果 (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		如果 (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		当 ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			如果 (G.from_top) {
				整数 nwrite = nread;
				如果 (seen < count) {
					/* We need to skip a few more bytes/lines */
					如果 (COUNT_BYTES) {
						nwrite -= (count - seen);
						seen += nread;
					} 否则 {
						char *s = buf;
						do {
							--nwrite;
							如果 (*s++ == '\n' && ++seen == count) {
								break;
							}
						} 当 (nwrite);
					}
				}
				如果 (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} 否则 如果 (count) {
				如果 (COUNT_BYTES) {
					taillen += nread;
					如果 (taillen > (整数)count) {
						memmove(tailbuf, tailbuf + taillen - count, count);
						taillen = count;
					}
				} 否则 {
					整数 k = nread;
					整数 newlines_in_buf = 0;

					do { /* count '\n' in last read */
						k--;
						如果 (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} 当 (k);

					如果 (newlines_seen + newlines_in_buf < (整数)count) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} 否则 {
						整数 extra = (buf[nread-1] != '\n');
						char *s;

						k = newlines_seen + newlines_in_buf + extra - count;
						s = tailbuf;
						当 (k) {
							如果 (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = count - extra;
					}
					如果 (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		如果 (!G.from_top) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} 当 (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = NULL;

	如果 (FOLLOW) 当 (1) {
		sleep(sleep_period);

		i = 0;
		do {
			整数 nread;
			const char *filename = argv[i];
			整数 fd = fds[i];
			整数 new_fd = -1;
			struct stat sbuf;

			如果 (FOLLOW_RETRY) {
				struct stat fsbuf;

				如果 (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					如果 (new_fd >= 0) {
						bb_error_msg("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						如果 (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} 否则 如果 (fd >= 0) {
						bb_perror_msg("%s has been renamed or deleted", filename);
					}
				}
			}
			如果 (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				continue;
			如果 (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			为 (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				如果 (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					如果 (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				如果 (nread <= 0) {
					如果 (new_fd < 0)
						break;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					continue;
				}
				如果 (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = NULL;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} 当 (++i < nfiles);
	} /* while (1) */

	如果 (ENABLE_FEATURE_CLEAN_UP) {
		free(fds);
		free(tailbuf);
	}
	返回 G.exitcode;
}
