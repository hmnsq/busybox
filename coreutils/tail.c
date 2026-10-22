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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

ഘടന globals {
	bool from_മുകൾ;
	bool exitcode;
} FIX_ALIASING;
#define G (*(ഘടന globals*)bb_common_bufsiz1)
#define INIT_G() ചെയ്യുക { setup_common_bufsiz(); } ആകുമ്പോൾ (0)

സ്ഥിതം ശൂന്യം tail_xprint_header(സ്ഥിരം അക്ഷരം *fmt, സ്ഥിരം അക്ഷരം *filename)
{
	എങ്കിൽ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

സ്ഥിതം ssize_t tail_read(പൂർണ്ണസംഖ്യ fd, അക്ഷരം *buf, size_t എണ്ണം)
{
	ssize_t r;

	r = full_read(fd, buf, എണ്ണം);
	എങ്കിൽ (r < 0) {
		bb_simple_perror_സന്ദേശം(bb_സന്ദേശം_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	തിരികെ r;
}

#define header_fmt_str "\n==> %s <==\n"

സ്ഥിതം unsigned eat_അക്കം(സ്ഥിരം അക്ഷരം *p)
{
	എങ്കിൽ (*p == '-')
		p++;
	അല്ലെങ്കിൽ എങ്കിൽ (*p == '+') {
		p++;
		G.from_മുകൾ = 1;
	}
	തിരികെ xatou_sfx(p, bkm_suffixes);
}

പൂർണ്ണസംഖ്യ tail_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ tail_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	unsigned എണ്ണം = 10;
	unsigned sleep_period = 1;
	സ്ഥിരം അക്ഷരം *str_c, *str_n;

	അക്ഷരം *tailbuf;
	size_t tailbufsize;
	unsigned header_threshold = 1;
	unsigned nfiles;
	പൂർണ്ണസംഖ്യ i, opt;

	പൂർണ്ണസംഖ്യ *fds;
	സ്ഥിരം അക്ഷരം *fmt;
	പൂർണ്ണസംഖ്യ prev_fd;

	INIT_G();

#എങ്കിൽ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	എങ്കിൽ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && isdigit(argv[1][1])
	) {
		എണ്ണം = eat_അക്കം(argv[1]);
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
#define എണ്ണം_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	എങ്കിൽ (opt & 0x2) എണ്ണം = eat_അക്കം(str_c); // -c
	എങ്കിൽ (opt & 0x4) എണ്ണം = eat_അക്കം(str_n); // -n
#എങ്കിൽ ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	എങ്കിൽ (opt & 0x8) header_threshold = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	എങ്കിൽ (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#അല്ലെങ്കിൽ
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(വലുപ്പം(fds[0]) * (argc + 1));
	എങ്കിൽ (!argv[0]) {
		ഘടന stat statbuf;

		എങ്കിൽ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (അക്ഷരം *) bb_സന്ദേശം_standard_നിവേശം;
	}
	nfiles = i = 0;
	ചെയ്യുക {
		പൂർണ്ണസംഖ്യ fd = open_or_warn_stdin(argv[i]);
		എങ്കിൽ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			തുടരുക;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ആകുമ്പോൾ (++i < argc);

	എങ്കിൽ (!nfiles)
		bb_simple_error_സന്ദേശം_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	എങ്കിൽ (!G.from_മുകൾ && എണ്ണം_BYTES) {
		എങ്കിൽ (tailbufsize < എണ്ണം + BUFSIZ) {
			tailbufsize = എണ്ണം + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ശൂന്യ;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	ചെയ്യുക {
		അക്ഷരം *buf;
		പൂർണ്ണസംഖ്യ taillen;
		പൂർണ്ണസംഖ്യ newlines_seen;
		unsigned seen;
		പൂർണ്ണസംഖ്യ nread;
		പൂർണ്ണസംഖ്യ fd = fds[i];

		എങ്കിൽ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			തുടരുക; /* may happen with -F */

		എങ്കിൽ (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		എങ്കിൽ (!G.from_മുകൾ) {
			off_t current = lseek(fd, 0, SEEK_END);
			എങ്കിൽ (current > 0) {
				unsigned off;
				എങ്കിൽ (എണ്ണം_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					എങ്കിൽ (എണ്ണം == 0)
						തുടരുക; /* showing zero bytes is easy :) */
					current -= എണ്ണം;
					എങ്കിൽ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_SET);
					bb_copyfd_size(fd, STDOUT_FILENO, എണ്ണം);
					തുടരുക;
				}
#എങ്കിൽ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (എണ്ണം | 0xf); /* for small counts, be more paranoid */
				എങ്കിൽ (off > (INT_MAX / (64*1024)))
					off = (INT_MAX / (64*1024));
				current -= off * (64*1024);
				എങ്കിൽ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_SET);
#endif
			}
		}

		എങ്കിൽ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		ആകുമ്പോൾ ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			എങ്കിൽ (G.from_മുകൾ) {
				പൂർണ്ണസംഖ്യ nwrite = nread;
				എങ്കിൽ (seen < എണ്ണം) {
					/* We need to skip a few more bytes/lines */
					എങ്കിൽ (എണ്ണം_BYTES) {
						nwrite -= (എണ്ണം - seen);
						seen += nread;
					} അല്ലെങ്കിൽ {
						അക്ഷരം *s = buf;
						ചെയ്യുക {
							--nwrite;
							എങ്കിൽ (*s++ == '\n' && ++seen == എണ്ണം) {
								നിർത്തുക;
							}
						} ആകുമ്പോൾ (nwrite);
					}
				}
				എങ്കിൽ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} അല്ലെങ്കിൽ എങ്കിൽ (എണ്ണം) {
				എങ്കിൽ (എണ്ണം_BYTES) {
					taillen += nread;
					എങ്കിൽ (taillen > (പൂർണ്ണസംഖ്യ)എണ്ണം) {
						memmove(tailbuf, tailbuf + taillen - എണ്ണം, എണ്ണം);
						taillen = എണ്ണം;
					}
				} അല്ലെങ്കിൽ {
					പൂർണ്ണസംഖ്യ k = nread;
					പൂർണ്ണസംഖ്യ newlines_in_buf = 0;

					ചെയ്യുക { /* count '\n' in last read */
						k--;
						എങ്കിൽ (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} ആകുമ്പോൾ (k);

					എങ്കിൽ (newlines_seen + newlines_in_buf < (പൂർണ്ണസംഖ്യ)എണ്ണം) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} അല്ലെങ്കിൽ {
						പൂർണ്ണസംഖ്യ extra = (buf[nread-1] != '\n');
						അക്ഷരം *s;

						k = newlines_seen + newlines_in_buf + extra - എണ്ണം;
						s = tailbuf;
						ആകുമ്പോൾ (k) {
							എങ്കിൽ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_seen = എണ്ണം - extra;
					}
					എങ്കിൽ (tailbufsize < (size_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		എങ്കിൽ (!G.from_മുകൾ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ആകുമ്പോൾ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ശൂന്യ;

	എങ്കിൽ (FOLLOW) ആകുമ്പോൾ (1) {
		sleep(sleep_period);

		i = 0;
		ചെയ്യുക {
			പൂർണ്ണസംഖ്യ nread;
			സ്ഥിരം അക്ഷരം *filename = argv[i];
			പൂർണ്ണസംഖ്യ fd = fds[i];
			പൂർണ്ണസംഖ്യ new_fd = -1;
			ഘടന stat sbuf;

			എങ്കിൽ (FOLLOW_RETRY) {
				ഘടന stat fsbuf;

				എങ്കിൽ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					എങ്കിൽ (new_fd >= 0) {
						bb_error_സന്ദേശം("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						എങ്കിൽ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} അല്ലെങ്കിൽ എങ്കിൽ (fd >= 0) {
						bb_perror_സന്ദേശം("%s has been renamed or deleted", filename);
					}
				}
			}
			എങ്കിൽ (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				തുടരുക;
			എങ്കിൽ (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			വേണ്ടി (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				എങ്കിൽ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					എങ്കിൽ (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_SET);
					}
				}

				nread = tail_read(fd, tailbuf, BUFSIZ);
				എങ്കിൽ (nread <= 0) {
					എങ്കിൽ (new_fd < 0)
						നിർത്തുക;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					തുടരുക;
				}
				എങ്കിൽ (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = ശൂന്യ;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ആകുമ്പോൾ (++i < nfiles);
	} /* while (1) */

	എങ്കിൽ (ENABLE_FEATURE_CLEAN_UP) {
		സ്വതന്ത്രം(fds);
		സ്വതന്ത്രം(tailbuf);
	}
	തിരികെ G.exitcode;
}
