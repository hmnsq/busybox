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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

संरचना globals {
	bool from_शीर्ष;
	bool exitcode;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() गर { setup_common_bufsiz(); } जबसम्म (0)

स्थिर शून्य tail_xprint_header(अचर वर्ण *fmt, अचर वर्ण *filename)
{
	यदि (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

स्थिर ssize_t tail_read(पूर्णाङ्क fd, वर्ण *buf, माप_प्रकार गन्ती)
{
	ssize_t r;

	r = full_read(fd, buf, गन्ती);
	यदि (r < 0) {
		bb_simple_perror_सन्देश(bb_सन्देश_read_error);
		G.exitcode = निकास_त्रुटि;
	}

	फर्काऊ r;
}

#define header_fmt_str "\n==> %s <==\n"

स्थिर अचिन्हित eat_अङ्क(अचर वर्ण *p)
{
	यदि (*p == '-')
		p++;
	अन्यथा यदि (*p == '+') {
		p++;
		G.from_शीर्ष = 1;
	}
	फर्काऊ xatou_sfx(p, bkm_suffixes);
}

पूर्णाङ्क tail_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tail_main(पूर्णाङ्क argc, वर्ण **argv)
{
	अचिन्हित गन्ती = 10;
	अचिन्हित sleep_period = 1;
	अचर वर्ण *str_c, *str_n;

	वर्ण *tailbuf;
	माप_प्रकार tailbufsize;
	अचिन्हित header_threshold = 1;
	अचिन्हित nfiles;
	पूर्णाङ्क i, opt;

	पूर्णाङ्क *fds;
	अचर वर्ण *fmt;
	पूर्णाङ्क prev_fd;

	INIT_G();

#यदि ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_TAIL
	/* Allow legacy syntax of an initial numeric option without -n. */
	यदि (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && है_अंक(argv[1][1])
	) {
		गन्ती = eat_अङ्क(argv[1]);
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
#define गन्ती_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	यदि (opt & 0x2) गन्ती = eat_अङ्क(str_c); // -c
	यदि (opt & 0x4) गन्ती = eat_अङ्क(str_n); // -n
#यदि ENABLE_FEATURE_FANCY_TAIL
	/* q: make it impossible for nfiles to be > header_threshold */
	यदि (opt & 0x8) header_threshold = अच_पूर्णांक_उच्च; // -q
	//if (opt & 0x10) // -s
	यदि (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#अन्यथा
# define FOLLOW_RETRY 0
#endif
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(परिमाण(fds[0]) * (argc + 1));
	यदि (!argv[0]) {
		संरचना stat statbuf;

		यदि (ख_स्थिति(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (वर्ण *) bb_सन्देश_standard_आगत;
	}
	nfiles = i = 0;
	गर {
		पूर्णाङ्क fd = open_or_warn_stdin(argv[i]);
		यदि (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = निकास_त्रुटि;
			जारी;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} जबसम्म (++i < argc);

	यदि (!nfiles)
		bb_simple_error_सन्देश_and_die("no files");

	/* prepare the buffer */
	tailbufsize = बफ_मान;
	यदि (!G.from_शीर्ष && गन्ती_BYTES) {
		यदि (tailbufsize < गन्ती + बफ_मान) {
			tailbufsize = गन्ती + बफ_मान;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = रिक्त;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	गर {
		वर्ण *buf;
		पूर्णाङ्क taillen;
		पूर्णाङ्क newlines_seen;
		अचिन्हित seen;
		पूर्णाङ्क nread;
		पूर्णाङ्क fd = fds[i];

		यदि (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
			जारी; /* may happen with -F */

		यदि (nfiles > header_threshold) {
			tail_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		यदि (!G.from_शीर्ष) {
			off_t current = lseek(fd, 0, अंत_से);
			यदि (current > 0) {
				अचिन्हित off;
				यदि (गन्ती_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					यदि (गन्ती == 0)
						जारी; /* showing zero bytes is easy :) */
					current -= गन्ती;
					यदि (current < 0)
						current = 0;
					xlseek(fd, current, शुरू_से);
					bb_copyfd_size(fd, STDOUT_FILENO, गन्ती);
					जारी;
				}
#यदि 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (गन्ती | 0xf); /* for small counts, be more paranoid */
				यदि (off > (पूर्णांक_उच्च / (64*1024)))
					off = (पूर्णांक_उच्च / (64*1024));
				current -= off * (64*1024);
				यदि (current < 0)
					current = 0;
				xlseek(fd, current, शुरू_से);
#endif
			}
		}

		यदि (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		जबसम्म ((nread = tail_read(fd, buf, tailbufsize - taillen)) > 0) {
			यदि (G.from_शीर्ष) {
				पूर्णाङ्क nwrite = nread;
				यदि (seen < गन्ती) {
					/* We need to skip a few more bytes/lines */
					यदि (गन्ती_BYTES) {
						nwrite -= (गन्ती - seen);
						seen += nread;
					} अन्यथा {
						वर्ण *s = buf;
						गर {
							--nwrite;
							यदि (*s++ == '\n' && ++seen == गन्ती) {
								रोक;
							}
						} जबसम्म (nwrite);
					}
				}
				यदि (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} अन्यथा यदि (गन्ती) {
				यदि (गन्ती_BYTES) {
					taillen += nread;
					यदि (taillen > (पूर्णाङ्क)गन्ती) {
						स_हटाओ(tailbuf, tailbuf + taillen - गन्ती, गन्ती);
						taillen = गन्ती;
					}
				} अन्यथा {
					पूर्णाङ्क k = nread;
					पूर्णाङ्क newlines_in_buf = 0;

					गर { /* count '\n' in last read */
						k--;
						यदि (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} जबसम्म (k);

					यदि (newlines_seen + newlines_in_buf < (पूर्णाङ्क)गन्ती) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} अन्यथा {
						पूर्णाङ्क extra = (buf[nread-1] != '\n');
						वर्ण *s;

						k = newlines_seen + newlines_in_buf + extra - गन्ती;
						s = tailbuf;
						जबसम्म (k) {
							यदि (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						स_हटाओ(tailbuf, s, taillen);
						newlines_seen = गन्ती - extra;
					}
					यदि (tailbufsize < (माप_प्रकार)taillen + बफ_मान) {
						tailbufsize = taillen + बफ_मान;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		यदि (!G.from_शीर्ष) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} जबसम्म (++i < nfiles);
	prev_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, बफ_मान);

	fmt = रिक्त;

	यदि (FOLLOW) जबसम्म (1) {
		sleep(sleep_period);

		i = 0;
		गर {
			पूर्णाङ्क nread;
			अचर वर्ण *filename = argv[i];
			पूर्णाङ्क fd = fds[i];
			पूर्णाङ्क new_fd = -1;
			संरचना stat sbuf;

			यदि (FOLLOW_RETRY) {
				संरचना stat fsbuf;

				यदि (fd < 0
				 || ख_स्थिति(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = खोलो(filename, O_RDONLY);
					यदि (new_fd >= 0) {
						bb_error_सन्देश("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						यदि (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} अन्यथा यदि (fd >= 0) {
						bb_perror_सन्देश("%s has been renamed or deleted", filename);
					}
				}
			}
			यदि (ENABLE_FEATURE_FANCY_TAIL && fd < 0)
				जारी;
			यदि (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			लागि (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				यदि (ख_स्थिति(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_size > 0
				) {
					off_t current = lseek(fd, 0, प्रस्तुत_से);
					यदि (sbuf.st_size < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, शुरू_से);
					}
				}

				nread = tail_read(fd, tailbuf, बफ_मान);
				यदि (nread <= 0) {
					यदि (new_fd < 0)
						रोक;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					जारी;
				}
				यदि (fmt && (fd != prev_fd)) {
					tail_xprint_header(fmt, filename);
					fmt = रिक्त;
					prev_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} जबसम्म (++i < nfiles);
	} /* while (1) */

	यदि (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(fds);
		मुक्त(tailbuf);
	}
	फर्काऊ G.exitcode;
}
