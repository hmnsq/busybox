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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

నిర్మాణం globals {
	bool from_శిఖరం;
	bool exitcode;
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { అమరిక_common_bufsiz(); } అయితే (0)

స్థిర శూన్య తోక_xprint_శీర్షిక(స్థిరరాశి అక్షరం *fmt, స్థిరరాశి అక్షరం *filename)
{
	ఒకవేళ (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

స్థిర ssize_t తోక_read(పూర్ణాంకం fd, అక్షరం *buf, కొలత_t లెక్క)
{
	ssize_t r;

	r = నిండిన_read(fd, buf, లెక్క);
	ఒకవేళ (r < 0) {
		bb_simple_perror_సందేశం(bb_సందేశం_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	తిరిగి r;
}

#నిర్వచించు శీర్షిక_fmt_str "\n==> %s <==\n"

స్థిర గుర్తులేని eat_అంకె(స్థిరరాశి అక్షరం *p)
{
	ఒకవేళ (*p == '-')
		p++;
	లేకపోతే ఒకవేళ (*p == '+') {
		p++;
		G.from_శిఖరం = 1;
	}
	తిరిగి xatou_sfx(p, bkm_suffixes);
}

పూర్ణాంకం తోక_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం తోక_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	గుర్తులేని లెక్క = 10;
	గుర్తులేని sleep_period = 1;
	స్థిరరాశి అక్షరం *str_c, *str_n;

	అక్షరం *tailbuf;
	కొలత_t tailbufsize;
	గుర్తులేని శీర్షిక_పరిమితి = 1;
	గుర్తులేని nfiles;
	పూర్ణాంకం i, opt;

	పూర్ణాంకం *fds;
	స్థిరరాశి అక్షరం *fmt;
	పూర్ణాంకం prev_fd;

	INIT_G();

#ఒకవేళ ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_తోక
	/* Allow legacy syntax of an initial numeric option without -n. */
	ఒకవేళ (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && అంకెనా(argv[1][1])
	) {
		లెక్క = eat_అంకె(argv[1]);
		argv++;
		argc--;
	}
#అయితే_ముగింపు

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_తోక("^")
			"fc:n:"IF_FEATURE_FANCY_తోక("qs:+vF")
			IF_FEATURE_FANCY_తోక("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_తోక(,&sleep_period)
	);
#నిర్వచించు FOLLOW (opt & 0x1)
#నిర్వచించు లెక్క_బైట్లు_2 (opt & 0x2)
	//if (opt & 0x1) // -f
	ఒకవేళ (opt & 0x2) లెక్క = eat_అంకె(str_c); // -c
	ఒకవేళ (opt & 0x4) లెక్క = eat_అంకె(str_n); // -n
#ఒకవేళ ENABLE_FEATURE_FANCY_తోక
	/* q: make it impossible for nfiles to be > header_threshold */
	ఒకవేళ (opt & 0x8) శీర్షిక_పరిమితి = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	ఒకవేళ (opt & 0x20) శీర్షిక_పరిమితి = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#లేకపోతే
# define FOLLOW_RETRY 0
#అయితే_ముగింపు
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(పరిమాణం(fds[0]) * (argc + 1));
	ఒకవేళ (!argv[0]) {
		నిర్మాణం stat statbuf;

		ఒకవేళ (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_బహుళకం)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (అక్షరం *) bb_సందేశం_standard_ఆదానం;
	}
	nfiles = i = 0;
	చేయి {
		పూర్ణాంకం fd = open_or_warn_stdin(argv[i]);
		ఒకవేళ (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			కొనసాగు;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} అయితే (++i < argc);

	ఒకవేళ (!nfiles)
		bb_simple_error_సందేశం_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	ఒకవేళ (!G.from_శిఖరం && లెక్క_బైట్లు_2) {
		ఒకవేళ (tailbufsize < లెక్క + BUFSIZ) {
			tailbufsize = లెక్క + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = ఖాళీ;

	/* tail the files */

	fmt = శీర్షిక_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	చేయి {
		అక్షరం *buf;
		పూర్ణాంకం taillen;
		పూర్ణాంకం newlines_చూసిన;
		గుర్తులేని చూసిన;
		పూర్ణాంకం nread;
		పూర్ణాంకం fd = fds[i];

		ఒకవేళ (ENABLE_FEATURE_FANCY_తోక && fd < 0)
			కొనసాగు; /* may happen with -F */

		ఒకవేళ (nfiles > శీర్షిక_పరిమితి) {
			తోక_xprint_శీర్షిక(fmt, argv[i]);
			fmt = శీర్షిక_fmt_str;
		}

		ఒకవేళ (!G.from_శిఖరం) {
			off_t current = lseek(fd, 0, SEEK_చివర);
			ఒకవేళ (current > 0) {
				గుర్తులేని off;
				ఒకవేళ (లెక్క_బైట్లు_2) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					ఒకవేళ (లెక్క == 0)
						కొనసాగు; /* showing zero bytes is easy :) */
					current -= లెక్క;
					ఒకవేళ (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_సమితి);
					bb_copyfd_కొలత(fd, STDOUT_FILENO, లెక్క);
					కొనసాగు;
				}
#ఒకవేళ 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (లెక్క | 0xf); /* for small counts, be more paranoid */
				ఒకవేళ (off > (గరిష్ఠ_పూర్ణాంకం / (64*1024)))
					off = (గరిష్ఠ_పూర్ణాంకం / (64*1024));
				current -= off * (64*1024);
				ఒకవేళ (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_సమితి);
#అయితే_ముగింపు
			}
		}

		ఒకవేళ (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		చూసిన = 1;
		newlines_చూసిన = 0;
		అయితే ((nread = తోక_read(fd, buf, tailbufsize - taillen)) > 0) {
			ఒకవేళ (G.from_శిఖరం) {
				పూర్ణాంకం nwrite = nread;
				ఒకవేళ (చూసిన < లెక్క) {
					/* We need to skip a few more bytes/lines */
					ఒకవేళ (లెక్క_బైట్లు_2) {
						nwrite -= (లెక్క - చూసిన);
						చూసిన += nread;
					} లేకపోతే {
						అక్షరం *s = buf;
						చేయి {
							--nwrite;
							ఒకవేళ (*s++ == '\n' && ++చూసిన == లెక్క) {
								ఆపు;
							}
						} అయితే (nwrite);
					}
				}
				ఒకవేళ (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} లేకపోతే ఒకవేళ (లెక్క) {
				ఒకవేళ (లెక్క_బైట్లు_2) {
					taillen += nread;
					ఒకవేళ (taillen > (పూర్ణాంకం)లెక్క) {
						memmove(tailbuf, tailbuf + taillen - లెక్క, లెక్క);
						taillen = లెక్క;
					}
				} లేకపోతే {
					పూర్ణాంకం k = nread;
					పూర్ణాంకం newlines_లోపల_buf = 0;

					చేయి { /* count '\n' in last read */
						k--;
						ఒకవేళ (buf[k] == '\n') {
							newlines_లోపల_buf++;
						}
					} అయితే (k);

					ఒకవేళ (newlines_చూసిన + newlines_లోపల_buf < (పూర్ణాంకం)లెక్క) {
						newlines_చూసిన += newlines_లోపల_buf;
						taillen += nread;
					} లేకపోతే {
						పూర్ణాంకం extra = (buf[nread-1] != '\n');
						అక్షరం *s;

						k = newlines_చూసిన + newlines_లోపల_buf + extra - లెక్క;
						s = tailbuf;
						అయితే (k) {
							ఒకవేళ (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_చూసిన = లెక్క - extra;
					}
					ఒకవేళ (tailbufsize < (కొలత_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		ఒకవేళ (!G.from_శిఖరం) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} అయితే (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = ఖాళీ;

	ఒకవేళ (FOLLOW) అయితే (1) {
		sleep(sleep_period);

		i = 0;
		చేయి {
			పూర్ణాంకం nread;
			స్థిరరాశి అక్షరం *filename = argv[i];
			పూర్ణాంకం fd = fds[i];
			పూర్ణాంకం new_fd = -1;
			నిర్మాణం stat sbuf;

			ఒకవేళ (FOLLOW_RETRY) {
				నిర్మాణం stat fsbuf;

				ఒకవేళ (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					ఒకవేళ (new_fd >= 0) {
						bb_error_సందేశం("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						ఒకవేళ (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} లేకపోతే ఒకవేళ (fd >= 0) {
						bb_perror_సందేశం("%s has been renamed or deleted", filename);
					}
				}
			}
			ఒకవేళ (ENABLE_FEATURE_FANCY_తోక && fd < 0)
				కొనసాగు;
			ఒకవేళ (nfiles > శీర్షిక_పరిమితి) {
				fmt = శీర్షిక_fmt_str;
			}
			కొరకు (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				ఒకవేళ (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_కొలత > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					ఒకవేళ (sbuf.st_కొలత < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_సమితి);
					}
				}

				nread = తోక_read(fd, tailbuf, BUFSIZ);
				ఒకవేళ (nread <= 0) {
					ఒకవేళ (new_fd < 0)
						ఆపు;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					కొనసాగు;
				}
				ఒకవేళ (fmt && (fd != prev_fd)) {
					తోక_xprint_శీర్షిక(fmt, filename);
					fmt = ఖాళీ;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} అయితే (++i < nfiles);
	} /* while (1) */

	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి) {
		విడుదల(fds);
		విడుదల(tailbuf);
	}
	తిరిగి G.exitcode;
}
