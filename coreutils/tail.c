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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

بنية globals {
	bool from_بالاترین;
	bool exitcode;
} FIX_ALIASING;
#تعریف_کن G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کن INIT_G() افعل { راهاندازی_common_bufsiz(); } طالما (0)

ساكن فراغ دم_xprint_سرآیند(ثابت حرف *fmt, ثابت حرف *filename)
{
	إذا (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

ساكن ssize_t دم_read(صحيح fd, حرف *buf, اندازه_t شمار)
{
	ssize_t r;

	r = پر_read(fd, buf, شمار);
	إذا (r < 0) {
		bb_simple_perror_پیام(bb_پیام_read_error);
		G.exitcode = EXIT_FAILURE;
	}

	أرجع r;
}

#تعریف_کن سرآیند_fmt_str "\n==> %s <==\n"

ساكن بیعلامت eat_رقم(ثابت حرف *p)
{
	إذا (*p == '-')
		p++;
	وإلا إذا (*p == '+') {
		p++;
		G.from_بالاترین = 1;
	}
	أرجع xatou_sfx(p, bkm_suffixes);
}

صحيح دم_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح دم_main(صحيح argc, حرف **argv)
{
	بیعلامت شمار = 10;
	بیعلامت sleep_period = 1;
	ثابت حرف *str_c, *str_n;

	حرف *tailbuf;
	اندازه_t tailbufsize;
	بیعلامت سرآیند_آستانه = 1;
	بیعلامت nfiles;
	صحيح i, opt;

	صحيح *fds;
	ثابت حرف *fmt;
	صحيح prev_fd;

	INIT_G();

#إذا ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_دم
	/* Allow legacy syntax of an initial numeric option without -n. */
	إذا (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && آیا_رقم(argv[1][1])
	) {
		شمار = eat_رقم(argv[1]);
		argv++;
		argc--;
	}
#پایان_اگر

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_دم("^")
			"fc:n:"IF_FEATURE_FANCY_دم("qs:+vF")
			IF_FEATURE_FANCY_دم("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_دم(,&sleep_period)
	);
#تعریف_کن FOLLOW (opt & 0x1)
#تعریف_کن شمار_بایتها_2 (opt & 0x2)
	//if (opt & 0x1) // -f
	إذا (opt & 0x2) شمار = eat_رقم(str_c); // -c
	إذا (opt & 0x4) شمار = eat_رقم(str_n); // -n
#إذا ENABLE_FEATURE_FANCY_دم
	/* q: make it impossible for nfiles to be > header_threshold */
	إذا (opt & 0x8) سرآیند_آستانه = UINT_MAX; // -q
	//if (opt & 0x10) // -s
	إذا (opt & 0x20) سرآیند_آستانه = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#وإلا
# define FOLLOW_RETRY 0
#پایان_اگر
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(حجم(fds[0]) * (argc + 1));
	إذا (!argv[0]) {
		بنية stat statbuf;

		إذا (fstat(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_نما)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (حرف *) bb_پیام_standard_درون_داد;
	}
	nfiles = i = 0;
	افعل {
		صحيح fd = open_or_warn_stdin(argv[i]);
		إذا (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = EXIT_FAILURE;
			استمر;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} طالما (++i < argc);

	إذا (!nfiles)
		bb_simple_error_پیام_and_die("no files");

	/* prepare the buffer */
	tailbufsize = BUFSIZ;
	إذا (!G.from_بالاترین && شمار_بایتها_2) {
		إذا (tailbufsize < شمار + BUFSIZ) {
			tailbufsize = شمار + BUFSIZ;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = عدم;

	/* tail the files */

	fmt = سرآیند_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	افعل {
		حرف *buf;
		صحيح taillen;
		صحيح newlines_دیده;
		بیعلامت دیده;
		صحيح nread;
		صحيح fd = fds[i];

		إذا (ENABLE_FEATURE_FANCY_دم && fd < 0)
			استمر; /* may happen with -F */

		إذا (nfiles > سرآیند_آستانه) {
			دم_xprint_سرآیند(fmt, argv[i]);
			fmt = سرآیند_fmt_str;
		}

		إذا (!G.from_بالاترین) {
			off_t current = lseek(fd, 0, SEEK_انتها);
			إذا (current > 0) {
				بیعلامت off;
				إذا (شمار_بایتها_2) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					إذا (شمار == 0)
						استمر; /* showing zero bytes is easy :) */
					current -= شمار;
					إذا (current < 0)
						current = 0;
					xlseek(fd, current, SEEK_مجموعه);
					bb_copyfd_اندازه(fd, STDOUT_FILENO, شمار);
					استمر;
				}
#إذا 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (شمار | 0xf); /* for small counts, be more paranoid */
				إذا (off > (بیشترین_صحیح / (64*1024)))
					off = (بیشترین_صحیح / (64*1024));
				current -= off * (64*1024);
				إذا (current < 0)
					current = 0;
				xlseek(fd, current, SEEK_مجموعه);
#پایان_اگر
			}
		}

		إذا (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		دیده = 1;
		newlines_دیده = 0;
		طالما ((nread = دم_read(fd, buf, tailbufsize - taillen)) > 0) {
			إذا (G.from_بالاترین) {
				صحيح nwrite = nread;
				إذا (دیده < شمار) {
					/* We need to skip a few more bytes/lines */
					إذا (شمار_بایتها_2) {
						nwrite -= (شمار - دیده);
						دیده += nread;
					} وإلا {
						حرف *s = buf;
						افعل {
							--nwrite;
							إذا (*s++ == '\n' && ++دیده == شمار) {
								اكسر;
							}
						} طالما (nwrite);
					}
				}
				إذا (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} وإلا إذا (شمار) {
				إذا (شمار_بایتها_2) {
					taillen += nread;
					إذا (taillen > (صحيح)شمار) {
						memmove(tailbuf, tailbuf + taillen - شمار, شمار);
						taillen = شمار;
					}
				} وإلا {
					صحيح k = nread;
					صحيح newlines_درون_buf = 0;

					افعل { /* count '\n' in last read */
						k--;
						إذا (buf[k] == '\n') {
							newlines_درون_buf++;
						}
					} طالما (k);

					إذا (newlines_دیده + newlines_درون_buf < (صحيح)شمار) {
						newlines_دیده += newlines_درون_buf;
						taillen += nread;
					} وإلا {
						صحيح extra = (buf[nread-1] != '\n');
						حرف *s;

						k = newlines_دیده + newlines_درون_buf + extra - شمار;
						s = tailbuf;
						طالما (k) {
							إذا (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						memmove(tailbuf, s, taillen);
						newlines_دیده = شمار - extra;
					}
					إذا (tailbufsize < (اندازه_t)taillen + BUFSIZ) {
						tailbufsize = taillen + BUFSIZ;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		إذا (!G.from_بالاترین) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} طالما (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, BUFSIZ);

	fmt = عدم;

	إذا (FOLLOW) طالما (1) {
		sleep(sleep_period);

		i = 0;
		افعل {
			صحيح nread;
			ثابت حرف *filename = argv[i];
			صحيح fd = fds[i];
			صحيح new_fd = -1;
			بنية stat sbuf;

			إذا (FOLLOW_RETRY) {
				بنية stat fsbuf;

				إذا (fd < 0
				 || fstat(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = open(filename, O_RDONLY);
					إذا (new_fd >= 0) {
						bb_error_پیام("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						إذا (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} وإلا إذا (fd >= 0) {
						bb_perror_پیام("%s has been renamed or deleted", filename);
					}
				}
			}
			إذا (ENABLE_FEATURE_FANCY_دم && fd < 0)
				استمر;
			إذا (nfiles > سرآیند_آستانه) {
				fmt = سرآیند_fmt_str;
			}
			لكل (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				إذا (fstat(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_اندازه > 0
				) {
					off_t current = lseek(fd, 0, SEEK_CUR);
					إذا (sbuf.st_اندازه < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, SEEK_مجموعه);
					}
				}

				nread = دم_read(fd, tailbuf, BUFSIZ);
				إذا (nread <= 0) {
					إذا (new_fd < 0)
						اكسر;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					استمر;
				}
				إذا (fmt && (fd != prev_fd)) {
					دم_xprint_سرآیند(fmt, filename);
					fmt = عدم;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} طالما (++i < nfiles);
	} /* while (1) */

	إذا (ENABLE_FEATURE_CLEAN_به_بالا) {
		حرر(fds);
		حرر(tailbuf);
	}
	أرجع G.exitcode;
}
