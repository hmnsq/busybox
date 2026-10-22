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

#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ "common_bufsiz.h"

સંરચના globals {
	bool from_ટોચ;
	bool exitcode;
} FIX_ALIASING;
#define G (*(સંરચના globals*)bb_common_bufsiz1)
#define INIT_G() કરો { setup_common_bufsiz(); } જ્યારે (0)

સ્થિર શૂન્ય tail_xprint_header(અચલ અક્ષર *fmt, અચલ અક્ષર *filename)
{
	જો (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

સ્થિર ssize_t tail_read(પૂર્ણાંક fd, અક્ષર *buf, size_t ગણતરી)
{
	ssize_t r;

	r = full_read(fd, buf, ગણતરી);
	જો (r < 0) {
		bb_simple_perror_સંદેશ(bb_સંદેશ_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	પાછા r;
}

#define header_fmt_str "\n==> %s <==\n"

સ્થિર unsigned eat_અંક(અચલ અક્ષર *p)
{
	જો (*p == '-')
		p++;
	નહીંતર જો (*p == '+') {
		p++;
		G.from_ટોચ = 1;
	}
	પાછા xatou_sfx(p, bkm_suffixes);
}

પૂર્ણાંક tail_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક tail_main(પૂર્ણાંક argc, અક્ષર **argv)
{
	unsigned ગણતરી = 10;
	unsigned sleep_period = 1;
	અચલ અક્ષર *str_c, *str_n;

	અક્ષર *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	પૂર્ણાંક i, opt;

	પૂર્ણાંક *fds;
	અચલ અક્ષર *fmt;
	પૂર્ણાંક prev_fd;

	INIT_G();

#જો ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	જો (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		ગણતરી = eat_અંક(argv[1]);
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
#define ગણતરી_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	જો (opt & 0x2) ગણતરી = eat_અંક(str_c); // -c
	જો (opt & 0x4) ગણતરી = eat_અંક(str_n); // -n
#જો ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	જો (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	જો (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#નહીંતર
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(કદ(fds[0]) * (argc + 1));
	જો (!argv[0]) {
		સંરચના stat statbuf;

		જો (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (અક્ષર *) bb_સંદેશ_standard_આગત;
	}
	nfiles = i = 0;
	કરો {
		પૂર્ણાંક fd = open_or_warn_stdin(argv[i]);
		જો (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			ચાલુ;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} જ્યારે (++i < argc);

	જો (!nfiles)
		bb_simple_error_સંદેશ_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	જો (!G.from_ટોચ && ગણતરી_BYTES) {
		જો (tailbufsize < ગણતરી + BUFSIZ) {
			tailbufsize = ગણતરી + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ખાલી;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	કરો {
		અક્ષર *buf;
		પૂર્ણાંક taillen;
		પૂર્ણાંક newlines_seen;
		unsigned seen;
		પૂર્ણાંક nread;
		પૂર્ણાંક fd = fds[i];

		જો (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			ચાલુ; /* may happen with -F */

		જો (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		જો (!G.from_ટોચ) {
			off_t current = lseek(fd, 0, SEEK_END);
			જો (current > 0) {
				unsigned off;
				જો (ગણતરી_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					જો (ગણતરી == 0)
						ચાલુ; /* showing zero bytes is easy :) */
					current -= ગણતરી;
					જો (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, ગણતરી);
					ચાલુ;
				}
#જો 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (ગણતરી | 0xf); /* for small counts, be more paranoid */
				જો (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				જો (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		જો (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		જ્યારે ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			જો (G.from_ટોચ) {
				પૂર્ણાંક nwrite = nread;
				જો (seen < ગણતરી) {
					/* We need to skip a few more bytes/lines */
					જો (ગણતરી_BYTES) {
						nwrite -= (ગણતરી - seen);
						seen += nread;
					} નહીંતર {
						અક્ષર *s = buf;
						કરો {
							--nwrite;
							જો (*s++ == '\n' && ++seen == ગણતરી) {
								તોડો;
							}
						} જ્યારે (nwrite);
					}
				}
				જો (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} નહીંતર જો (ગણતરી) {
				જો (ગણતરી_BYTES) {
					taillen += nread;
					જો (taillen > (પૂર્ણાંક)ગણતરી) {
						memmove(tailbuf, tailbuf + taillen - ગણતરી, ગણતરી);
						taillen = ગણતરી;
					}
				} નહીંતર {
					પૂર્ણાંક k = nread;
					પૂર્ણાંક newlines_in_buf = 0;

					કરો { /* count '\n' in last read */
						k--;
						જો (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} જ્યારે (k);

					જો (newlines_seen + newlines_in_buf < (પૂર્ણાંક)ગણતરી) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} નહીંતર {
						પૂર્ણાંક extra = (buf[nread-1] != '\n');
						અક્ષર *s;

						k = newlines_seen + newlines_in_buf + extra - ગણતરી;
						s = tailbuf;
						જ્યારે (k) {
							જો (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = ગણતરી - extra;
					}
					જો (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		જો (!G.from_ટોચ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} જ્યારે (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ખાલી;

	જો (FOLLOW) જ્યારે (1) {
		sleep(sleep_period);

		i = 0;
		કરો {
			પૂર્ણાંક nread;
			અચલ અક્ષર *filename = argv[i];
			પૂર્ણાંક fd = fds[i];
			પૂર્ણાંક new_fd = -1;
			સંરચના stat sbuf;

			જો (FOLLOW_RETRY) {
				સંરચના stat fsbuf;

				જો (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					જો (new_fd >= 0) {
						bb_error_સંદેશ("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						જો (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} નહીંતર જો (fd >= 0) {
						bb_perror_સંદેશ("%s has been renamed or deleted", filename);
					}
				}
			}
			જો (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				ચાલુ;
			જો (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			માટે (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				જો (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					જો (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				જો (nread <= 0) {
					જો (new_fd < 0)
						તોડો;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					ચાલુ;
				}
				જો (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = ખાલી;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} જ્યારે (++i < nfiles);
	} /* while (1) */

	જો (ENABLE_FEATURE_CLEAN_UP) {
		મુક્ત(fds);
		મુક્ત(tailbuf);
	}
	પાછા G.exitcode;
}
