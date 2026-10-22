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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

ਬਣਤਰ globals {
	bool from_ਸਿਖਰ;
	bool exitcode;
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { ਤਿਆਰੀ_common_bufsiz(); } ਜਦੋਂ (0)

ਸਥਿਰ ਖਾਲੀ ਪੂਛ_xprint_ਸਿਰਲੇਖ(ਅਚਲ ਅੱਖਰ *fmt, ਅਚਲ ਅੱਖਰ *filename)
{
	ਜੇ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

ਸਥਿਰ ssize_t ਪੂਛ_read(ਪੂਰਨ fd, ਅੱਖਰ *buf, ਮਾਪ_t ਗਿਣਤੀ_2)
{
	ssize_t r;

	r = ਭਰਿਆ_read(fd, buf, ਗਿਣਤੀ_2);
	ਜੇ (r < 0) {
		bb_simple_perror_ਸੁਨੇਹਾ(bb_ਸੁਨੇਹਾ_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	ਵਾਪਸ r;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਰਲੇਖ_fmt_str "\n==> %s <==\n"

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ eat_ਅੰਕ(ਅਚਲ ਅੱਖਰ *p)
{
	ਜੇ (*p == '-')
		p++;
	ਨਹੀਂਤਾਂ ਜੇ (*p == '+') {
		p++;
		G.from_ਸਿਖਰ = 1;
	}
	ਵਾਪਸ xatou_sfx(p, bkm_suffixes);
}

ਪੂਰਨ ਪੂਛ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ ਪੂਛ_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = 10;
	ਬਿਨਾਨਿਸ਼ਾਨੀ sleep_period = 1;
	ਅਚਲ ਅੱਖਰ *str_c, *str_n;

	ਅੱਖਰ *tailbuf;
	ਮਾਪ_t tailbufsize;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਲੇਖ_ਹੱਦ = 1;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nfiles;
	ਪੂਰਨ i, opt;

	ਪੂਰਨ *fds;
	ਅਚਲ ਅੱਖਰ *fmt;
	ਪੂਰਨ prev_fd;

	INIT_G();

#ਜੇ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ਪੂਛ
	/* Allow legacy syntax of an initial numeric option without -n. */
	ਜੇ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && ਅੰਕ_ਹੈ(argv[1][1])
	) {
		ਗਿਣਤੀ_2 = eat_ਅੰਕ(argv[1]);
		argv++;
		argc--;
	}
#ਜੇ_ਅੰਤ

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_ਪੂਛ("^")
			"fc:n:"IF_FEATURE_FANCY_ਪੂਛ("qs:+vF")
			IF_FEATURE_FANCY_ਪੂਛ("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_ਪੂਛ(,&sleep_period)
	);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOLLOW (opt & 0x1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗਿਣਤੀ_ਬਾਈਟਾਂ_2 (opt & 0x2)
	//if (opt & 0x1) // -f
	ਜੇ (opt & 0x2) ਗਿਣਤੀ_2 = eat_ਅੰਕ(str_c); // -c
	ਜੇ (opt & 0x4) ਗਿਣਤੀ_2 = eat_ਅੰਕ(str_n); // -n
#ਜੇ ENABLE_FEATURE_FANCY_ਪੂਛ
	/* q: make it impossible for nfiles to be > header_threshold */
	ਜੇ (opt & 0x8) ਸਿਰਲੇਖ_ਹੱਦ = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ਜੇ (opt & 0x20) ਸਿਰਲੇਖ_ਹੱਦ = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#ਨਹੀਂਤਾਂ
# define FOLLOW_RETRY 0
#ਜੇ_ਅੰਤ
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(ਆਕਾਰ(fds[0]) * (argc + 1));
	ਜੇ (!argv[0]) {
		ਬਣਤਰ stat statbuf;

		ਜੇ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_ਬਹੁਲਕ)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (ਅੱਖਰ *) bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
	}
	nfiles = i = 0;
	ਕਰੋ {
		ਪੂਰਨ fd = open_or_warn_stdin(argv[i]);
		ਜੇ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			ਜਾਰੀ;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ਜਦੋਂ (++i < argc);

	ਜੇ (!nfiles)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ਜੇ (!G.from_ਸਿਖਰ && ਗਿਣਤੀ_ਬਾਈਟਾਂ_2) {
		ਜੇ (tailbufsize < ਗਿਣਤੀ_2 + BUFSIZ) {
			tailbufsize = ਗਿਣਤੀ_2 + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ਨਿਰਾ;

	/* tail the files */

	fmt = ਸਿਰਲੇਖ_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	ਕਰੋ {
		ਅੱਖਰ *buf;
		ਪੂਰਨ taillen;
		ਪੂਰਨ newlines_ਵੇਖਿਆ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਵੇਖਿਆ;
		ਪੂਰਨ nread;
		ਪੂਰਨ fd = fds[i];

		ਜੇ (ENABLE_FEATURE_FANCY_ਪੂਛ && fd < 0)
			ਜਾਰੀ; /* may happen with -F */

		ਜੇ (nfiles > ਸਿਰਲੇਖ_ਹੱਦ) {
			ਪੂਛ_xprint_ਸਿਰਲੇਖ(fmt, argv[i]);
			fmt = ਸਿਰਲੇਖ_fmt_str;
		}

		ਜੇ (!G.from_ਸਿਖਰ) {
			off_t current = lseek(fd, 0, SEEK_ਸਿਰਾ);
			ਜੇ (current > 0) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ off;
				ਜੇ (ਗਿਣਤੀ_ਬਾਈਟਾਂ_2) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ਜੇ (ਗਿਣਤੀ_2 == 0)
						ਜਾਰੀ; /* showing zero bytes is easy :) */
					current -= ਗਿਣਤੀ_2;
					ਜੇ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_ਸੈੱਟ);
					bb_copyfd_ਮਾਪ(fd, STDOUT_FILENO, ਗਿਣਤੀ_2);
					ਜਾਰੀ;
				}
#ਜੇ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (ਗਿਣਤੀ_2 | 0xf); /* for small counts, be more paranoid */
				ਜੇ (off > (ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / (64*1024)))
					off = (ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / (64*1024));
				current -= off * (64*1024);
				ਜੇ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_ਸੈੱਟ);
#ਜੇ_ਅੰਤ
			}
		}

		ਜੇ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		ਵੇਖਿਆ = 1;
		newlines_ਵੇਖਿਆ = 0;
		ਜਦੋਂ ((nread = ਪੂਛ_read(fd, buf, tailbufsize - taillen)) > 0) {
			ਜੇ (G.from_ਸਿਖਰ) {
				ਪੂਰਨ nwrite = nread;
				ਜੇ (ਵੇਖਿਆ < ਗਿਣਤੀ_2) {
					/* We need to skip a few more bytes/lines */
					ਜੇ (ਗਿਣਤੀ_ਬਾਈਟਾਂ_2) {
						nwrite -= (ਗਿਣਤੀ_2 - ਵੇਖਿਆ);
						ਵੇਖਿਆ += nread;
					} ਨਹੀਂਤਾਂ {
						ਅੱਖਰ *s = buf;
						ਕਰੋ {
							--nwrite;
							ਜੇ (*s++ == '\n' && ++ਵੇਖਿਆ == ਗਿਣਤੀ_2) {
								ਰੋਕੋ;
							}
						} ਜਦੋਂ (nwrite);
					}
				}
				ਜੇ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} ਨਹੀਂਤਾਂ ਜੇ (ਗਿਣਤੀ_2) {
				ਜੇ (ਗਿਣਤੀ_ਬਾਈਟਾਂ_2) {
					taillen += nread;
					ਜੇ (taillen > (ਪੂਰਨ)ਗਿਣਤੀ_2) {
						memmove(tailbuf, tailbuf + taillen - ਗਿਣਤੀ_2, ਗਿਣਤੀ_2);
						taillen = ਗਿਣਤੀ_2;
					}
				} ਨਹੀਂਤਾਂ {
					ਪੂਰਨ k = nread;
					ਪੂਰਨ newlines_ਵਿੱਚ_buf = 0;

					ਕਰੋ { /* count '\n' in last read */
						k--;
						ਜੇ (buf[k] == '\n') {
							newlines_ਵਿੱਚ_buf++;
						}
					} ਜਦੋਂ (k);

					ਜੇ (newlines_ਵੇਖਿਆ + newlines_ਵਿੱਚ_buf < (ਪੂਰਨ)ਗਿਣਤੀ_2) {
						newlines_ਵੇਖਿਆ += newlines_ਵਿੱਚ_buf;
						taillen += nread;
					} ਨਹੀਂਤਾਂ {
						ਪੂਰਨ extra = (buf[nread-1] != '\n');
						ਅੱਖਰ *s;

						k = newlines_ਵੇਖਿਆ + newlines_ਵਿੱਚ_buf + extra - ਗਿਣਤੀ_2;
						s = tailbuf;
						ਜਦੋਂ (k) {
							ਜੇ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_ਵੇਖਿਆ = ਗਿਣਤੀ_2 - extra;
					}
					ਜੇ (tailbufsize < (ਮਾਪ_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ਜੇ (!G.from_ਸਿਖਰ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ਜਦੋਂ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ਨਿਰਾ;

	ਜੇ (FOLLOW) ਜਦੋਂ (1) {
		sleep(sleep_period);

		i = 0;
		ਕਰੋ {
			ਪੂਰਨ nread;
			ਅਚਲ ਅੱਖਰ *filename = argv[i];
			ਪੂਰਨ fd = fds[i];
			ਪੂਰਨ new_fd = -1;
			ਬਣਤਰ stat sbuf;

			ਜੇ (FOLLOW_RETRY) {
				ਬਣਤਰ stat fsbuf;

				ਜੇ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ਜੇ (new_fd >= 0) {
						bb_error_ਸੁਨੇਹਾ("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ਜੇ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} ਨਹੀਂਤਾਂ ਜੇ (fd >= 0) {
						bb_perror_ਸੁਨੇਹਾ("%s has been renamed or deleted", filename);
					}
				}
			}
			ਜੇ (ENABLE_FEATURE_FANCY_ਪੂਛ && fd < 0)
				ਜਾਰੀ;
			ਜੇ (nfiles > ਸਿਰਲੇਖ_ਹੱਦ) {
				fmt = ਸਿਰਲੇਖ_fmt_str;
			}
			ਲਈ (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ਜੇ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_ਮਾਪ > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ਜੇ (sbuf.st_ਮਾਪ < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_ਸੈੱਟ);
					}
				}

				nread = ਪੂਛ_read(fd, tailbuf, BUFSIZ);
				ਜੇ (nread <= 0) {
					ਜੇ (new_fd < 0)
						ਰੋਕੋ;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					ਜਾਰੀ;
				}
				ਜੇ (fmt && (fd != prev_fd)) {
					ਪੂਛ_xprint_ਸਿਰਲੇਖ(fmt, filename);
					fmt = ਨਿਰਾ;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ਜਦੋਂ (++i < nfiles);
	} /* while (1) */

	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ) {
		ਮੁਕਤ(fds);
		ਮੁਕਤ(tailbuf);
	}
	ਵਾਪਸ G.exitcode;
}
