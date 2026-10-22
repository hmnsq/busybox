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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"

கட்டமைப்பு globals {
	bool from_உச்சி;
	bool exitcode;
} FIX_ALIASING;
#define G (*(கட்டமைப்பு globals*)bb_common_bufsiz1)
#define INIT_G() செய் { setup_common_bufsiz(); } போது (0)

நிலையான வெறுமை tail_xprint_header(மாறிலி எழுத்து *fmt, மாறிலி எழுத்து *filename)
{
	எனில் (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

நிலையான ssize_t tail_read(முழு fd, எழுத்து *buf, size_t மொத்தம்)
{
	ssize_t r;

	r = full_read(fd, buf, மொத்தம்);
	எனில் (r < 0) {
		bb_simple_perror_செய்தி(bb_செய்தி_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	திரும்பு r;
}

#define header_fmt_str "\n==> %s <==\n"

நிலையான unsigned eat_இலக்கம்(மாறிலி எழுத்து *p)
{
	எனில் (*p == '-')
		p++;
	இல்லையெனில் எனில் (*p == '+') {
		p++;
		G.from_உச்சி = 1;
	}
	திரும்பு xatou_sfx(p, bkm_suffixes);
}

முழு tail_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு tail_main(முழு argc, எழுத்து **argv)
{
	unsigned மொத்தம் = 10;
	unsigned sleep_period = 1;
	மாறிலி எழுத்து *str_c, *str_n;

	எழுத்து *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	முழு i, opt;

	முழு *fds;
	மாறிலி எழுத்து *fmt;
	முழு prev_fd;

	INIT_G();

#எனில் ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	எனில் (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		மொத்தம் = eat_இலக்கம்(argv[1]);
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
#define மொத்தம்_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	எனில் (opt & 0x2) மொத்தம் = eat_இலக்கம்(str_c); // -c
	எனில் (opt & 0x4) மொத்தம் = eat_இலக்கம்(str_n); // -n
#எனில் ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	எனில் (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	எனில் (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#இல்லையெனில்
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(அளவு(fds[0]) * (argc + 1));
	எனில் (!argv[0]) {
		கட்டமைப்பு stat statbuf;

		எனில் (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (எழுத்து *) bb_செய்தி_standard_உள்ளீடு;
	}
	nfiles = i = 0;
	செய் {
		முழு fd = open_or_warn_stdin(argv[i]);
		எனில் (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			தொடர்;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} போது (++i < argc);

	எனில் (!nfiles)
		bb_simple_error_செய்தி_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	எனில் (!G.from_உச்சி && மொத்தம்_BYTES) {
		எனில் (tailbufsize < மொத்தம் + BUFSIZ) {
			tailbufsize = மொத்தம் + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = வெற்று;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	செய் {
		எழுத்து *buf;
		முழு taillen;
		முழு newlines_seen;
		unsigned seen;
		முழு nread;
		முழு fd = fds[i];

		எனில் (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			தொடர்; /* may happen with -F */

		எனில் (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		எனில் (!G.from_உச்சி) {
			off_t current = lseek(fd, 0, SEEK_END);
			எனில் (current > 0) {
				unsigned off;
				எனில் (மொத்தம்_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					எனில் (மொத்தம் == 0)
						தொடர்; /* showing zero bytes is easy :) */
					current -= மொத்தம்;
					எனில் (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, மொத்தம்);
					தொடர்;
				}
#எனில் 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (மொத்தம் | 0xf); /* for small counts, be more paranoid */
				எனில் (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				எனில் (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		எனில் (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		போது ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			எனில் (G.from_உச்சி) {
				முழு nwrite = nread;
				எனில் (seen < மொத்தம்) {
					/* We need to skip a few more bytes/lines */
					எனில் (மொத்தம்_BYTES) {
						nwrite -= (மொத்தம் - seen);
						seen += nread;
					} இல்லையெனில் {
						எழுத்து *s = buf;
						செய் {
							--nwrite;
							எனில் (*s++ == '\n' && ++seen == மொத்தம்) {
								நிறுத்து;
							}
						} போது (nwrite);
					}
				}
				எனில் (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} இல்லையெனில் எனில் (மொத்தம்) {
				எனில் (மொத்தம்_BYTES) {
					taillen += nread;
					எனில் (taillen > (முழு)மொத்தம்) {
						memmove(tailbuf, tailbuf + taillen - மொத்தம், மொத்தம்);
						taillen = மொத்தம்;
					}
				} இல்லையெனில் {
					முழு k = nread;
					முழு newlines_in_buf = 0;

					செய் { /* count '\n' in last read */
						k--;
						எனில் (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} போது (k);

					எனில் (newlines_seen + newlines_in_buf < (முழு)மொத்தம்) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} இல்லையெனில் {
						முழு extra = (buf[nread-1] != '\n');
						எழுத்து *s;

						k = newlines_seen + newlines_in_buf + extra - மொத்தம்;
						s = tailbuf;
						போது (k) {
							எனில் (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = மொத்தம் - extra;
					}
					எனில் (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		எனில் (!G.from_உச்சி) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} போது (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = வெற்று;

	எனில் (FOLLOW) போது (1) {
		sleep(sleep_period);

		i = 0;
		செய் {
			முழு nread;
			மாறிலி எழுத்து *filename = argv[i];
			முழு fd = fds[i];
			முழு new_fd = -1;
			கட்டமைப்பு stat sbuf;

			எனில் (FOLLOW_RETRY) {
				கட்டமைப்பு stat fsbuf;

				எனில் (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					எனில் (new_fd >= 0) {
						bb_error_செய்தி("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						எனில் (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} இல்லையெனில் எனில் (fd >= 0) {
						bb_perror_செய்தி("%s has been renamed or deleted", filename);
					}
				}
			}
			எனில் (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				தொடர்;
			எனில் (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			க்கு (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				எனில் (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					எனில் (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				எனில் (nread <= 0) {
					எனில் (new_fd < 0)
						நிறுத்து;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					தொடர்;
				}
				எனில் (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = வெற்று;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} போது (++i < nfiles);
	} /* while (1) */

	எனில் (ENABLE_FEATURE_CLEAN_UP) {
		விடுவி(fds);
		விடுவி(tailbuf);
	}
	திரும்பு G.exitcode;
}
