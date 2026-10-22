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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

काष्ठा globals {
	bool from_शीर्ष;
	bool exitcode;
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { setup_common_bufsiz(); } जबतक (0)

अटल व्योम पुच्छ_xprint_header(स्थिर अक्षर *fmt, स्थिर अक्षर *filename)
{
	अगर (fdprintf(STDOUT_FILENO, fmt, filename) < 0)
		bb_perror_nomsg_and_die();
}

अटल ssize_t पुच्छ_read(पूर्णांक fd, अक्षर *buf, माप_प्रकार गिनती)
{
	ssize_t r;

	r = full_read(fd, buf, गिनती);
	अगर (r < 0) {
		bb_simple_perror_संदेश(bb_संदेश_read_error);
		G.exitcode = निकास_त्रुटि;
	}

	वापस r;
}

#घोषणा header_fmt_str "\n==> %s <==\n"

अटल अचिन्हित eat_अंक(स्थिर अक्षर *p)
{
	अगर (*p == '-')
		p++;
	अन्यथा अगर (*p == '+') {
		p++;
		G.from_शीर्ष = 1;
	}
	वापस xatou_sfx(p, bkm_suffixes);
}

पूर्णांक पुच्छ_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक पुच्छ_main(पूर्णांक argc, अक्षर **argv)
{
	अचिन्हित गिनती = 10;
	अचिन्हित sleep_period = 1;
	स्थिर अक्षर *str_c, *str_n;

	अक्षर *tailbuf;
	माप_प्रकार tailbufsize;
	अचिन्हित header_threshold = 1;
	अचिन्हित nfiles;
	पूर्णांक i, opt;

	पूर्णांक *fds;
	स्थिर अक्षर *fmt;
	पूर्णांक पूर्व_fd;

	INIT_G();

#अगर ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_पुच्छ
	/* Allow legacy syntax of an initial numeric option without -n. */
	अगर (argv[1] && (argv[1][0] == '+' || argv[1][0] == '-')
	 && है_अंक(argv[1][1])
	) {
		गिनती = eat_अंक(argv[1]);
		argv++;
		argc--;
	}
#पूर्ण_अगर

	/* -s NUM, -F imlies -f */
	opt = getopt32(argv, IF_FEATURE_FANCY_पुच्छ("^")
			"fc:n:"IF_FEATURE_FANCY_पुच्छ("qs:+vF")
			IF_FEATURE_FANCY_पुच्छ("\0" "Ff"),
			&str_c, &str_n IF_FEATURE_FANCY_पुच्छ(,&sleep_period)
	);
#घोषणा FOLLOW (opt & 0x1)
#घोषणा गिनती_BYTES (opt & 0x2)
	//if (opt & 0x1) // -f
	अगर (opt & 0x2) गिनती = eat_अंक(str_c); // -c
	अगर (opt & 0x4) गिनती = eat_अंक(str_n); // -n
#अगर ENABLE_FEATURE_FANCY_पुच्छ
	/* q: make it impossible for nfiles to be > header_threshold */
	अगर (opt & 0x8) header_threshold = अच_पूर्णांक_उच्च; // -q
	//if (opt & 0x10) // -s
	अगर (opt & 0x20) header_threshold = 0; // -v
# define FOLLOW_RETRY (opt & 0x40)
#अन्यथा
# define FOLLOW_RETRY 0
#पूर्ण_अगर
	argc -= optind;
	argv += optind;

	/* open all the files */
	fds = xmalloc(माप(fds[0]) * (argc + 1));
	अगर (!argv[0]) {
		काष्ठा stat statbuf;

		अगर (ख_स्थिति(STDIN_FILENO, &statbuf) == 0
		 && S_ISFIFO(statbuf.st_mode)
		) {
			opt &= ~1; /* clear FOLLOW */
		}
		argv[0] = (अक्षर *) bb_संदेश_standard_निवेश;
	}
	nfiles = i = 0;
	करो {
		पूर्णांक fd = open_or_warn_stdin(argv[i]);
		अगर (fd < 0 && !FOLLOW_RETRY) {
			G.exitcode = निकास_त्रुटि;
			जारी;
		}
		fds[nfiles] = fd;
		argv[nfiles++] = argv[i];
	} जबतक (++i < argc);

	अगर (!nfiles)
		bb_simple_error_संदेश_and_die("no files");

	/* prepare the buffer */
	tailbufsize = बफ_मान;
	अगर (!G.from_शीर्ष && गिनती_BYTES) {
		अगर (tailbufsize < गिनती + बफ_मान) {
			tailbufsize = गिनती + बफ_मान;
		}
	}
	/* tail -c1024m REGULAR_FILE doesn't really need 1G mem block.
	 * (In fact, it doesn't need ANY memory). So delay allocation.
	 */
	tailbuf = शून्य;

	/* tail the files */

	fmt = header_fmt_str + 1; /* skip leading newline in the header on the first output */
	i = 0;
	करो {
		अक्षर *buf;
		पूर्णांक taillen;
		पूर्णांक newlines_seen;
		अचिन्हित seen;
		पूर्णांक nread;
		पूर्णांक fd = fds[i];

		अगर (ENABLE_FEATURE_FANCY_पुच्छ && fd < 0)
			जारी; /* may happen with -F */

		अगर (nfiles > header_threshold) {
			पुच्छ_xprint_header(fmt, argv[i]);
			fmt = header_fmt_str;
		}

		अगर (!G.from_शीर्ष) {
			off_t current = lseek(fd, 0, अंत_से);
			अगर (current > 0) {
				अचिन्हित off;
				अगर (गिनती_BYTES) {
				/* Optimizing count-bytes case if the file is seekable.
				 * Beware of backing up too far.
				 * Also we exclude files with size 0 (because of /proc/xxx) */
					अगर (गिनती == 0)
						जारी; /* showing zero bytes is easy :) */
					current -= गिनती;
					अगर (current < 0)
						current = 0;
					xlseek(fd, current, शुरू_से);
					bb_copyfd_आकार(fd, STDOUT_FILENO, गिनती);
					जारी;
				}
#अगर 1 /* This is technically incorrect for *LONG* strings, but very useful */
				/* Optimizing count-lines case if the file is seekable.
				 * We assume the lines are <64k.
				 * (Users complain that tail takes too long
				 * on multi-gigabyte files) */
				off = (गिनती | 0xf); /* for small counts, be more paranoid */
				अगर (off > (पूर्णांक_उच्च / (64*1024)))
					off = (पूर्णांक_उच्च / (64*1024));
				current -= off * (64*1024);
				अगर (current < 0)
					current = 0;
				xlseek(fd, current, शुरू_से);
#पूर्ण_अगर
			}
		}

		अगर (!tailbuf)
			tailbuf = xmalloc(tailbufsize);

		buf = tailbuf;
		taillen = 0;
		/* "We saw 1st line/byte".
		 * Used only by +N code ("start from Nth", 1-based): */
		seen = 1;
		newlines_seen = 0;
		जबतक ((nread = पुच्छ_read(fd, buf, tailbufsize - taillen)) > 0) {
			अगर (G.from_शीर्ष) {
				पूर्णांक nwrite = nread;
				अगर (seen < गिनती) {
					/* We need to skip a few more bytes/lines */
					अगर (गिनती_BYTES) {
						nwrite -= (गिनती - seen);
						seen += nread;
					} अन्यथा {
						अक्षर *s = buf;
						करो {
							--nwrite;
							अगर (*s++ == '\n' && ++seen == गिनती) {
								अवरोध;
							}
						} जबतक (nwrite);
					}
				}
				अगर (nwrite > 0)
					xwrite(STDOUT_FILENO, buf + nread - nwrite, nwrite);
			} अन्यथा अगर (गिनती) {
				अगर (गिनती_BYTES) {
					taillen += nread;
					अगर (taillen > (पूर्णांक)गिनती) {
						स_हटाओ(tailbuf, tailbuf + taillen - गिनती, गिनती);
						taillen = गिनती;
					}
				} अन्यथा {
					पूर्णांक k = nread;
					पूर्णांक newlines_in_buf = 0;

					करो { /* count '\n' in last read */
						k--;
						अगर (buf[k] == '\n') {
							newlines_in_buf++;
						}
					} जबतक (k);

					अगर (newlines_seen + newlines_in_buf < (पूर्णांक)गिनती) {
						newlines_seen += newlines_in_buf;
						taillen += nread;
					} अन्यथा {
						पूर्णांक extra = (buf[nread-1] != '\n');
						अक्षर *s;

						k = newlines_seen + newlines_in_buf + extra - गिनती;
						s = tailbuf;
						जबतक (k) {
							अगर (*s == '\n') {
								k--;
							}
							s++;
						}
						taillen += nread - (s - tailbuf);
						स_हटाओ(tailbuf, s, taillen);
						newlines_seen = गिनती - extra;
					}
					अगर (tailbufsize < (माप_प्रकार)taillen + बफ_मान) {
						tailbufsize = taillen + बफ_मान;
						tailbuf = xrealloc(tailbuf, tailbufsize);
					}
				}
				buf = tailbuf + taillen;
			}
		} /* while (tail_read() > 0) */
		अगर (!G.from_शीर्ष) {
			xwrite(STDOUT_FILENO, tailbuf, taillen);
		}
	} जबतक (++i < nfiles);
	पूर्व_fd = fds[i-1];

	tailbuf = xrealloc(tailbuf, बफ_मान);

	fmt = शून्य;

	अगर (FOLLOW) जबतक (1) {
		sleep(sleep_period);

		i = 0;
		करो {
			पूर्णांक nread;
			स्थिर अक्षर *filename = argv[i];
			पूर्णांक fd = fds[i];
			पूर्णांक new_fd = -1;
			काष्ठा stat sbuf;

			अगर (FOLLOW_RETRY) {
				काष्ठा stat fsbuf;

				अगर (fd < 0
				 || ख_स्थिति(fd, &fsbuf) < 0
				 || stat(filename, &sbuf) < 0
				 || fsbuf.st_dev != sbuf.st_dev
				 || fsbuf.st_ino != sbuf.st_ino
				) {
					/* Looks like file has been created/renamed/deleted */
					new_fd = खोलो(filename, O_RDONLY);
					अगर (new_fd >= 0) {
						bb_error_संदेश("%s has %s; following end of new file",
							filename, (fd < 0) ? "appeared" : "been replaced"
						);
						अगर (fd < 0) {
							/* No previously open fd for this file,
							 * start using new_fd immediately. */
							fds[i] = fd = new_fd;
							new_fd = -1;
						}
					} अन्यथा अगर (fd >= 0) {
						bb_perror_संदेश("%s has been renamed or deleted", filename);
					}
				}
			}
			अगर (ENABLE_FEATURE_FANCY_पुच्छ && fd < 0)
				जारी;
			अगर (nfiles > header_threshold) {
				fmt = header_fmt_str;
			}
			क्रम (;;) {
				/* tail -f keeps following files even if they are truncated */
				/* /proc files report zero st_size, don't lseek them */
				अगर (ख_स्थिति(fd, &sbuf) == 0
				 /* && S_ISREG(sbuf.st_mode) TODO? */
				 && sbuf.st_आकार > 0
				) {
					off_t current = lseek(fd, 0, प्रस्तुत_से);
					अगर (sbuf.st_आकार < current) {
						//bb_perror_msg("%s: file truncated", filename); - says coreutils 8.32
						xlseek(fd, 0, शुरू_से);
					}
				}

				nread = पुच्छ_read(fd, tailbuf, बफ_मान);
				अगर (nread <= 0) {
					अगर (new_fd < 0)
						अवरोध;
					/* Switch to "tail -F"ing the new file */
					xmove_fd(new_fd, fd);
					new_fd = -1;
					जारी;
				}
				अगर (fmt && (fd != पूर्व_fd)) {
					पुच्छ_xprint_header(fmt, filename);
					fmt = शून्य;
					पूर्व_fd = fd;
				}
				xwrite(STDOUT_FILENO, tailbuf, nread);
			}
		} जबतक (++i < nfiles);
	} /* while (1) */

	अगर (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(fds);
		मुक्त(tailbuf);
	}
	वापस G.exitcode;
}
