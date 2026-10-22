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

ರಚನೆ globals {
	bool from_ತುದಿ;
	bool exitcode;
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { ಸಿದ್ಧತೆ_common_bufsiz(); } ಆಗ (0)

ಸ್ಥಿರ ಶೂನ್ಯ ಬಾಲ_xprint_ಶೀರ್ಷಿಕೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಆದರೆ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

ಸ್ಥಿರ ssize_t ಬಾಲ_read(ಪೂರ್ಣಾಂಕ fd, ಅಕ್ಷರ *buf, ಗಾತ್ರ_t ಎಣಿಕೆ_2)
{
	ssize_t r;

	r = ತುಂಬಿದ_read(fd, buf, ಎಣಿಕೆ_2);
	ಆದರೆ (r < 0) {
		bb_simple_perror_ಸಂದೇಶ(bb_ಸಂದೇಶ_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	ಹಿಂತಿರುಗಿ r;
}

#ವ್ಯಾಖ್ಯಾನಿಸು ಶೀರ್ಷಿಕೆ_fmt_str "\n==> %s <==\n"

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ eat_ಅಂಕೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p)
{
	ಆದರೆ (*p == '-')
		p++;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*p == '+') {
		p++;
		G.from_ತುದಿ = 1;
	}
	ಹಿಂತಿರುಗಿ xatou_sfx(p, bkm_suffixes);
}

ಪೂರ್ಣಾಂಕ ಬಾಲ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ ಬಾಲ_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2 = 10;
	ಚಿಹ್ನೆರಹಿತ sleep_period = 1;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str_c, *str_n;

	ಅಕ್ಷರ *tailbuf;
	ಗಾತ್ರ_t tailbufsize;
	ಚಿಹ್ನೆರಹಿತ ಶೀರ್ಷಿಕೆ_ಮಿತಿ = 1;
	ಚಿಹ್ನೆರಹಿತ nfiles;
	ಪೂರ್ಣಾಂಕ i, opt;

	ಪೂರ್ಣಾಂಕ *fds;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt;
	ಪೂರ್ಣಾಂಕ prev_fd;

	INIT_G();

#ಆದರೆ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_ಬಾಲ
	/* Allow legacy syntax of an initial numeric option without -n. */
	ಆದರೆ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && ಅಂಕಿಯೇ(argv[1][1])
	) {
		ಎಣಿಕೆ_2 = eat_ಅಂಕೆ(argv[1]);
		argv++;
		argc--;
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_ಬಾಲ("^")
			"fc:n:"IF_FEATURE_FANCY_ಬಾಲ("qs:+vF")
			IF_FEATURE_FANCY_ಬಾಲ("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_ಬಾಲ(,&sleep_period)
	);
#ವ್ಯಾಖ್ಯಾನಿಸು FOLLOW (opt & 0x1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಣಿಕೆ_ಬೈಟುಗಳು_2 (opt & 0x2)
	//if (opt & 0x1) // -f
	ಆದರೆ (opt & 0x2) ಎಣಿಕೆ_2 = eat_ಅಂಕೆ(str_c); // -c
	ಆದರೆ (opt & 0x4) ಎಣಿಕೆ_2 = eat_ಅಂಕೆ(str_n); // -n
#ಆದರೆ ENABLE_FEATURE_FANCY_ಬಾಲ
	/* q: make it impossible for nfiles to be > header_threshold */
	ಆದರೆ (opt & 0x8) ಶೀರ್ಷಿಕೆ_ಮಿತಿ = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ಆದರೆ (opt & 0x20) ಶೀರ್ಷಿಕೆ_ಮಿತಿ = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#ಇಲ್ಲದಿದ್ದರೆ
# define FOLLOW_RETRY 0
#ಆದರೆ_ಅಂತ್ಯ
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(ಗಾತ್ರ(fds[0]) * (argc + 1));
	ಆದರೆ (!argv[0]) {
		ರಚನೆ stat statbuf;

		ಆದರೆ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_ಬಹುಲಕ)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (ಅಕ್ಷರ *) bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
	}
	nfiles = i = 0;
	ಮಾಡು {
		ಪೂರ್ಣಾಂಕ fd = open_or_warn_stdin(argv[i]);
		ಆದರೆ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} ಆಗ (++i < argc);

	ಆದರೆ (!nfiles)
		bb_simple_error_ಸಂದೇಶ_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ಆದರೆ (!G.from_ತುದಿ && ಎಣಿಕೆ_ಬೈಟುಗಳು_2) {
		ಆದರೆ (tailbufsize < ಎಣಿಕೆ_2 + BUFSIZ) {
			tailbufsize = ಎಣಿಕೆ_2 + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ಖಾಲಿ;

	/* tail the files */

	fmt = ಶೀರ್ಷಿಕೆ_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	ಮಾಡು {
		ಅಕ್ಷರ *buf;
		ಪೂರ್ಣಾಂಕ taillen;
		ಪೂರ್ಣಾಂಕ newlines_ನೋಡಿದ;
		ಚಿಹ್ನೆರಹಿತ ನೋಡಿದ;
		ಪೂರ್ಣಾಂಕ nread;
		ಪೂರ್ಣಾಂಕ fd = fds[i];

		ಆದರೆ (ENABLE_FEATURE_FANCY_ಬಾಲ && fd < 0)
			ಮುಂದುವರಿ; /* may happen with -F */

		ಆದರೆ (nfiles > ಶೀರ್ಷಿಕೆ_ಮಿತಿ) {
			ಬಾಲ_xprint_ಶೀರ್ಷಿಕೆ(fmt, argv[i]);
			fmt = ಶೀರ್ಷಿಕೆ_fmt_str;
		}

		ಆದರೆ (!G.from_ತುದಿ) {
			off_t current = lseek(fd, 0, SEEK_ಕೊನೆ);
			ಆದರೆ (current > 0) {
				ಚಿಹ್ನೆರಹಿತ off;
				ಆದರೆ (ಎಣಿಕೆ_ಬೈಟುಗಳು_2) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ಆದರೆ (ಎಣಿಕೆ_2 == 0)
						ಮುಂದುವರಿ; /* showing zero bytes is easy :) */
					current -= ಎಣಿಕೆ_2;
					ಆದರೆ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_ಗಣ);
					bb_copyfd_ಗಾತ್ರ(fd, STDOUT_FILENO, ಎಣಿಕೆ_2);
					ಮುಂದುವರಿ;
				}
#ಆದರೆ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (ಎಣಿಕೆ_2 | 0xf); /* for small counts, be more paranoid */
				ಆದರೆ (off > (ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / (64*1024)))
					off = (ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / (64*1024));
				current -= off * (64*1024);
				ಆದರೆ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_ಗಣ);
#ಆದರೆ_ಅಂತ್ಯ
			}
		}

		ಆದರೆ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		ನೋಡಿದ = 1;
		newlines_ನೋಡಿದ = 0;
		ಆಗ ((nread = ಬಾಲ_read(fd, buf, tailbufsize - taillen)) > 0) {
			ಆದರೆ (G.from_ತುದಿ) {
				ಪೂರ್ಣಾಂಕ nwrite = nread;
				ಆದರೆ (ನೋಡಿದ < ಎಣಿಕೆ_2) {
					/* We need to skip a few more bytes/lines */
					ಆದರೆ (ಎಣಿಕೆ_ಬೈಟುಗಳು_2) {
						nwrite -= (ಎಣಿಕೆ_2 - ನೋಡಿದ);
						ನೋಡಿದ += nread;
					} ಇಲ್ಲದಿದ್ದರೆ {
						ಅಕ್ಷರ *s = buf;
						ಮಾಡು {
							--nwrite;
							ಆದರೆ (*s++ == '\n' && ++ನೋಡಿದ == ಎಣಿಕೆ_2) {
								ನಿಲ್ಲಿಸು;
							}
						} ಆಗ (nwrite);
					}
				}
				ಆದರೆ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಣಿಕೆ_2) {
				ಆದರೆ (ಎಣಿಕೆ_ಬೈಟುಗಳು_2) {
					taillen += nread;
					ಆದರೆ (taillen > (ಪೂರ್ಣಾಂಕ)ಎಣಿಕೆ_2) {
						memmove(tailbuf, tailbuf + taillen - ಎಣಿಕೆ_2, ಎಣಿಕೆ_2);
						taillen = ಎಣಿಕೆ_2;
					}
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಪೂರ್ಣಾಂಕ k = nread;
					ಪೂರ್ಣಾಂಕ newlines_ಒಳ_buf = 0;

					ಮಾಡು { /* count '\n' in last read */
						k--;
						ಆದರೆ (buf[k] == '\n') {
							newlines_ಒಳ_buf++;
						}
					} ಆಗ (k);

					ಆದರೆ (newlines_ನೋಡಿದ + newlines_ಒಳ_buf < (ಪೂರ್ಣಾಂಕ)ಎಣಿಕೆ_2) {
						newlines_ನೋಡಿದ += newlines_ಒಳ_buf;
						taillen += nread;
					} ಇಲ್ಲದಿದ್ದರೆ {
						ಪೂರ್ಣಾಂಕ extra = (buf[nread-1] != '\n');
						ಅಕ್ಷರ *s;

						k = newlines_ನೋಡಿದ + newlines_ಒಳ_buf + extra - ಎಣಿಕೆ_2;
						s = tailbuf;
						ಆಗ (k) {
							ಆದರೆ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_ನೋಡಿದ = ಎಣಿಕೆ_2 - extra;
					}
					ಆದರೆ (tailbufsize < (ಗಾತ್ರ_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ಆದರೆ (!G.from_ತುದಿ) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} ಆಗ (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ಖಾಲಿ;

	ಆದರೆ (FOLLOW) ಆಗ (1) {
		sleep(sleep_period);

		i = 0;
		ಮಾಡು {
			ಪೂರ್ಣಾಂಕ nread;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = argv[i];
			ಪೂರ್ಣಾಂಕ fd = fds[i];
			ಪೂರ್ಣಾಂಕ new_fd = -1;
			ರಚನೆ stat sbuf;

			ಆದರೆ (FOLLOW_RETRY) {
				ರಚನೆ stat fsbuf;

				ಆದರೆ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ಆದರೆ (new_fd >= 0) {
						bb_error_ಸಂದೇಶ("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ಆದರೆ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fd >= 0) {
						bb_perror_ಸಂದೇಶ("%s has been renamed or deleted", filename);
					}
				}
			}
			ಆದರೆ (ENABLE_FEATURE_FANCY_ಬಾಲ && fd < 0)
				ಮುಂದುವರಿ;
			ಆದರೆ (nfiles > ಶೀರ್ಷಿಕೆ_ಮಿತಿ) {
				fmt = ಶೀರ್ಷಿಕೆ_fmt_str;
			}
			ಗಾಗಿ (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ಆದರೆ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_ಗಾತ್ರ > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ಆದರೆ (sbuf.st_ಗಾತ್ರ < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_ಗಣ);
					}
				}

				nread = ಬಾಲ_read(fd, tailbuf, BUFSIZ);
				ಆದರೆ (nread <= 0) {
					ಆದರೆ (new_fd < 0)
						ನಿಲ್ಲಿಸು;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					ಮುಂದುವರಿ;
				}
				ಆದರೆ (fmt && (fd != prev_fd)) {
					ಬಾಲ_xprint_ಶೀರ್ಷಿಕೆ(fmt, filename);
					fmt = ಖಾಲಿ;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} ಆಗ (++i < nfiles);
	} /* while (1) */

	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ) {
		ಬಿಡುಗಡೆ(fds);
		ಬಿಡುಗಡೆ(tailbuf);
	}
	ಹಿಂತಿರುಗಿ G.exitcode;
}
