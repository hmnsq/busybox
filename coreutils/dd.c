/* vi: set sw=4 ts=4: */
/*
 * Mini dd implementation for busybox
 *
 * Copyright (C) 2000,2001  Matt Kraai
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config DD
//config:	bool "dd (8.3 kb)"
//config:	default y
//config:	help
//config:	dd copies a file (from standard input to standard output,
//config:	by default) using specific input and output blocksizes,
//config:	while optionally performing conversions on it.
//config:
//config:config FEATURE_DD_SIGNAL_HANDLING
//config:	bool "Enable signal handling for status reporting"
//config:	default y
//config:	depends on DD
//config:	help
//config:	Sending a SIGUSR1 signal to a running 'dd' process makes it
//config:	print to standard error the number of records read and written
//config:	so far, then to resume copying.
//config:
//config:	$ dd if=/dev/zero of=/dev/null &
//config:	$ pid=$!; kill -USR1 $pid; sleep 1; kill $pid
//config:	10899206+0 records in
//config:	10899206+0 records out
//config:
//config:config FEATURE_DD_THIRD_STATUS_LINE
//config:	bool "Enable the third status line upon signal"
//config:	default y
//config:	depends on DD && FEATURE_DD_SIGNAL_HANDLING
//config:	help
//config:	Displays a coreutils-like third status line with transferred bytes,
//config:	elapsed time and speed.
//config:
//config:config FEATURE_DD_IBS_OBS
//config:	bool "Enable ibs, obs, iflag, oflag and conv options"
//config:	default y
//config:	depends on DD
//config:	help
//config:	Enable support for writing a certain number of bytes in and out,
//config:	at a time, and performing conversions on the data stream.
//config:
//config:config FEATURE_DD_STATUS
//config:	bool "Enable status display options"
//config:	default y
//config:	depends on DD
//config:	help
//config:	Enable support for status=noxfer/none option.

//applet:IF_DD(APPLET_NOEXEC(dd, dd, BB_DIR_BIN, BB_SUID_DROP, dd))

//kbuild:lib-$(CONFIG_DD) += dd.o

//usage:#define dd_trivial_usage
//usage:       "[if=FILE] [of=FILE] [" IF_FEATURE_DD_IBS_OBS("ibs=N obs=N/") "bs=N] [count=N] [skip=N] [seek=N]"
//usage:	IF_FEATURE_DD_IBS_OBS("\n"
//usage:       "	[conv=notrunc|noerror|sync|fsync]\n"
//usage:       "	[iflag=skip_bytes|count_bytes|fullblock|direct] [oflag=seek_bytes|append|direct]"
//usage:	)
//usage:#define dd_full_usage "\n\n"
//usage:       "Copy a file with converting and formatting\n"
//usage:     "\n	if=FILE		Read from FILE instead of stdin"
//usage:     "\n	of=FILE		Write to FILE instead of stdout"
//usage:     "\n	bs=N		Read and write N bytes at a time"
//usage:	IF_FEATURE_DD_IBS_OBS(
//usage:     "\n	ibs=N		Read N bytes at a time"
//usage:	)
//usage:	IF_FEATURE_DD_IBS_OBS(
//usage:     "\n	obs=N		Write N bytes at a time"
//usage:	)
//usage:     "\n	count=N		Copy only N input blocks"
//usage:     "\n	skip=N		Skip N input blocks"
//usage:     "\n	seek=N		Skip N output blocks"
//usage:	IF_FEATURE_DD_IBS_OBS(
//usage:     "\n	conv=notrunc	Don't truncate output file"
//usage:     "\n	conv=noerror	Continue after read errors"
//usage:     "\n	conv=sync	Pad blocks with zeros"
//usage:     "\n	conv=fsync	Physically write data out before finishing"
//usage:     "\n	conv=swab	Swap every pair of bytes"
//usage:     "\n	iflag=skip_bytes	skip=N is in bytes"
//usage:     "\n	iflag=count_bytes	count=N is in bytes"
//usage:     "\n	oflag=seek_bytes	seek=N is in bytes"
//usage:     "\n	iflag=direct	O_DIRECT input"
//usage:     "\n	oflag=direct	O_DIRECT output"
//usage:     "\n	iflag=fullblock	Read full blocks"
//usage:     "\n	oflag=append	Open output in append mode"
//usage:	)
//usage:	IF_FEATURE_DD_STATUS(
//usage:     "\n	status=noxfer	Suppress rate output"
//usage:     "\n	status=none	Suppress all output"
//usage:	)
//usage:     "\n"
//usage:     "\nN may be suffixed by c (1), w (2), b (512), kB (1000), k (1024), MB, M, GB, G"
//usage:
//usage:#define dd_example_usage
//usage:       "$ dd if=/dev/zero of=/dev/ram1 bs=1M count=4\n"
//usage:       "4+0 records in\n"
//usage:       "4+0 records out\n"

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


गणना {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

संरचना globals {
	off_t out_full, out_part, in_full, in_part;
#यदि ENABLE_FEATURE_DD_THIRD_STATUS_पङ्क्ति
	अचिन्हित दीर्घ दीर्घ total_bytes;
	अचिन्हित दीर्घ दीर्घ begin_time_us;
#endif
	पूर्णाङ्क flags;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() गर { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	स_रखो(&G, 0, परिमाण(G)); \
} जबसम्म (0)

गणना {
	/* Must be in the same order as OP_conv_XXX! */
	/* (see "flags |= (1 << what)" below) */
	FLAG_NOTRUNC = (1 << 0) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SYNC    = (1 << 1) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_NOERROR = (1 << 2) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FSYNC   = (1 << 3) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SWAB    = (1 << 4) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of conv flags */
	/* start of input flags */
	FLAG_IFLAG_SHIFT   = 5,
	FLAG_SKIP_BYTES    = (1 << 5) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_गन्ती_BYTES   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FULLBLOCK     = (1 << 7) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_IDIRECT       = (1 << 8) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of input flags */
	/* start of output flags */
	FLAG_OFLAG_SHIFT   = 9,
	FLAG_SEEK_BYTES    = (1 << 9) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_APPEND        = (1 << 10) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ODIRECT       = (1 << 11) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of output flags */
	FLAG_TWOBUFS       = (1 << 12) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_गन्ती         = 1 << 13,
	FLAG_STATUS_NONE   = 1 << 14,
	FLAG_STATUS_NOXFER = 1 << 15,
};

स्थिर शून्य dd_output_status(पूर्णाङ्क UNUSED_PARAM cur_signal)
{
#यदि ENABLE_FEATURE_DD_THIRD_STATUS_पङ्क्ति
	द्विगुण seconds;
	अचिन्हित दीर्घ दीर्घ bytes_sec;
	अचिन्हित दीर्घ दीर्घ now_us = monotonic_us(); /* before fprintf */
#endif

	/* Deliberately using %u, not %d */
	ख_लिखो(मानक_त्रुटि, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.in_full, G.in_part,
			G.out_full, G.out_part);

#यदि ENABLE_FEATURE_DD_THIRD_STATUS_पङ्क्ति
# यदि ENABLE_FEATURE_DD_STATUS
	यदि (G.flags & FLAG_STATUS_NOXFER) /* status=noxfer active? */
		फर्काऊ;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	ख_लिखो(मानक_त्रुटि, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.total_bytes,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.total_bytes, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_time_us) / 1000000.0;
	bytes_sec = G.total_bytes / seconds;
	ख_लिखो(मानक_त्रुटि, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(bytes_sec, 1, 0)
	);
#endif
}

#यदि ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
स्थिर पूर्णाङ्क clear_O_DIRECT(पूर्णाङ्क fd)
{
	यदि (त्रुटि_सं == EINVAL) {
		पूर्णाङ्क fl = fcntl(fd, F_GETFL);
		यदि (fl & O_DIRECT) {
			fcntl(fd, F_SETFL, fl & ~O_DIRECT);
			फर्काऊ 1;
		}
	}
	फर्काऊ 0;
}
# endif
#endif

स्थिर ssize_t dd_read(शून्य *ibuf, माप_प्रकार ibs)
{
	ssize_t n;

#यदि ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	यदि (G.flags & FLAG_FULLBLOCK)
		n = full_read(ifd, ibuf, ibs);
	अन्यथा
#endif
		n = safe_read(ifd, ibuf, ibs);
#यदि ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	यदि (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_DIRECT(ifd))
		गच्छ read_again;
# endif
#endif
	फर्काऊ n;
}

स्थिर bool write_and_stats(अचर शून्य *buf, माप_प्रकार len, माप_प्रकार obs,
	अचर वर्ण *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = full_write(ofd, buf, len);
#यदि ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	यदि (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_DIRECT(ofd))
		गच्छ write_again;
# endif
#endif

#यदि ENABLE_FEATURE_DD_THIRD_STATUS_पङ्क्ति
	यदि (n > 0)
		G.total_bytes += n;
#endif
	यदि ((माप_प्रकार)n == obs) {
		G.out_full++;
		फर्काऊ 0;
	}
	यदि ((माप_प्रकार)n == len) {
		G.out_part++;
		फर्काऊ 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_सन्देश("error writing '%s'", filename);
	फर्काऊ 1;
}

#यदि ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#अन्यथा
# define XATOU_SFX xatoul_sfx
#endif

#यदि ENABLE_FEATURE_DD_IBS_OBS
स्थिर पूर्णाङ्क parse_comma_flags(वर्ण *val, अचर वर्ण *शब्दहरू, अचर वर्ण *error_in)
{
	पूर्णाङ्क flags = 0;
	जबसम्म (1) {
		पूर्णाङ्क n;
		वर्ण *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = म_अक्षर(val, ',');
		यदि (arg)
			*arg = '\0';
		n = index_in_strings(शब्दहरू, val);
		यदि (n < 0)
			bb_error_सन्देश_and_die(bb_सन्देश_invalid_arg_to, val, error_in);
		flags |= (1 << n);
		यदि (!arg) /* no ',' left, so this was the last specifier */
			रोक;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	फर्काऊ flags;
}
#endif

स्थिर शून्य *alloc_buf(माप_प्रकार size)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	यदि (size >= bb_getpagesize())
		फर्काऊ xmmap_anon(size);
	फर्काऊ xmalloc(size);
}

पूर्णाङ्क dd_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क dd_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	स्थिर अचर वर्ण keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_STATUS("status\0")
#यदि ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#endif
		;
#यदि ENABLE_FEATURE_DD_IBS_OBS
	स्थिर अचर वर्ण conv_शब्दहरू[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	स्थिर अचर वर्ण iflag_शब्दहरू[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	स्थिर अचर वर्ण oflag_शब्दहरू[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#endif
#यदि ENABLE_FEATURE_DD_STATUS
	स्थिर अचर वर्ण status_शब्दहरू[] ALIGN1 =
		"none\0""noxfer\0";
#endif
	गणना {
		OP_bs = 0,
		OP_गन्ती,
		OP_seek,
		OP_skip,
		OP_if,
		OP_of,
		IF_FEATURE_DD_STATUS(OP_status,)
#यदि ENABLE_FEATURE_DD_IBS_OBS
		OP_ibs,
		OP_obs,
		OP_conv,
		OP_iflag,
		OP_oflag,
		/* Must be in the same order as FLAG_XXX! */
		OP_conv_notrunc = 0,
		OP_conv_sync,
		OP_conv_noerror,
		OP_conv_fsync,
		OP_conv_swab,
	/* Unimplemented conv=XXX: */
	//nocreat       do not create the output file
	//excl          fail if the output file already exists
	//fdatasync     physically write output file data before finishing
	//lcase         change upper case to lower case
	//ucase         change lower case to upper case
	//block         pad newline-terminated records with spaces to cbs-size
	//unblock       replace trailing spaces in cbs-size records with newline
	//ascii         from EBCDIC to ASCII
	//ebcdic        from ASCII to EBCDIC
	//ibm           from ASCII to alternate EBCDIC
	/* Partially implemented: */
	//swab          swap every pair of input bytes: will abort on non-even reads
		OP_iflag_skip_bytes,
		OP_iflag_गन्ती_bytes,
		OP_iflag_fullblock,
		OP_iflag_direct,
		OP_oflag_seek_bytes,
		OP_oflag_direct,
#endif
	};
	exitcode_t exitcode = निकास_त्रुटि;
	पूर्णाङ्क i;
	माप_प्रकार ibs = 512;
	वर्ण *ibuf;
#यदि ENABLE_FEATURE_DD_IBS_OBS
	माप_प्रकार obs = 512;
	वर्ण *obuf;
#अन्यथा
# define obs  ibs
# define obuf ibuf
#endif
	/* These are all zeroed at once! */
	संरचना {
		IF_FEATURE_DD_IBS_OBS(माप_प्रकार ocount;)
		ssize_t prev_read_size; /* for detecting swab failure */
		off_t गन्ती;
		off_t seek, skip;
		अचर वर्ण *infile, *outfile;
	} Z;
#define ocount  (Z.ocount )
#define prev_read_size (Z.prev_read_size)
#define गन्ती   (Z.गन्ती  )
#define seek    (Z.seek   )
#define skip    (Z.skip   )
#define infile  (Z.infile )
#define outfile (Z.outfile)

	स_रखो(&Z, 0, परिमाण(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	लागि (i = 1; argv[i]; i++) {
		पूर्णाङ्क what;
		वर्ण *val;
		वर्ण *arg = argv[i];

#यदि ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		यदि (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			जारी;
#endif
		val = म_अक्षर(arg, '=');
		यदि (val == रिक्त)
			bb_show_usage();
		*val = '\0';
		what = index_in_strings(keywords, arg);
		यदि (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#यदि ENABLE_FEATURE_DD_IBS_OBS
		यदि (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		यदि (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		यदि (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_शब्दहरू, "conv");
			/*continue;*/
		}
		यदि (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_शब्दहरू, "iflag") << FLAG_IFLAG_SHIFT;
			/*continue;*/
		}
		यदि (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_शब्दहरू, "oflag") << FLAG_OFLAG_SHIFT;
			/*continue;*/
		}
#endif
		यदि (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		यदि (what == OP_गन्ती) {
			G.flags |= FLAG_गन्ती;
			गन्ती = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		यदि (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		यदि (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		यदि (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		यदि (what == OP_of) {
			outfile = val;
			/*continue;*/
		}
#यदि ENABLE_FEATURE_DD_STATUS
		यदि (what == OP_status) {
			पूर्णाङ्क n;
			n = index_in_strings(status_शब्दहरू, val);
			यदि (n < 0)
				bb_error_सन्देश_and_die(bb_सन्देश_invalid_arg_to, val, "status");
			G.flags |= FLAG_STATUS_NONE << n;
			/*continue;*/
		}
#endif
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#यदि ENABLE_FEATURE_DD_IBS_OBS
	यदि (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#endif

#यदि ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_empty_mask(SIGUSR1, dd_output_status);
#endif
#यदि ENABLE_FEATURE_DD_THIRD_STATUS_पङ्क्ति
	G.begin_time_us = monotonic_us();
#endif

	यदि (infile) {
		पूर्णाङ्क iflag = O_RDONLY;
#यदि ENABLE_FEATURE_DD_IBS_OBS
		यदि (G.flags & FLAG_IDIRECT) {
# ifdef O_DIRECT
			iflag |= O_DIRECT;
# अन्यथा
			bb_error_सन्देश_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(infile, iflag), ifd);
	} अन्यथा {
		infile = bb_सन्देश_standard_आगत;
	}
	यदि (outfile) {
		पूर्णाङ्क oflag = O_WRONLY | O_CREAT;

		यदि (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		यदि (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#यदि ENABLE_FEATURE_DD_IBS_OBS
		यदि (G.flags & FLAG_ODIRECT) {
# ifdef O_DIRECT
			oflag |= O_DIRECT;
# अन्यथा
			bb_error_सन्देश_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(outfile, oflag), ofd);

		यदि (seek && !(G.flags & FLAG_NOTRUNC)) {
			माप_प्रकार blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
			यदि (ftruncate(ofd, seek * blocksz) < 0) {
				संरचना stat st;

				यदि (ख_स्थिति(ofd, &st) < 0
				 || S_ISREG(st.st_mode)
				 || S_ISDIR(st.st_mode)
				) {
					गच्छ die_outfile;
				}
			}
		}
	} अन्यथा {
		outfile = bb_सन्देश_standard_output;
	}
	यदि (skip) {
		माप_प्रकार blocksz = (G.flags & FLAG_SKIP_BYTES) ? 1 : ibs;
		यदि (lseek(ifd, skip * blocksz, प्रस्तुत_से) < 0) {
			गर {
				ssize_t n = dd_read(ibuf, blocksz);
				यदि (n < 0)
					गच्छ die_infile;
				यदि (n == 0)
					रोक;
			} जबसम्म (--skip != 0);
		}
	}
	यदि (seek) {
		माप_प्रकार blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
		यदि (lseek(ofd, seek * blocksz, प्रस्तुत_से) < 0)
			गच्छ die_outfile;
	}

	जबसम्म (1) {
		ssize_t n = ibs;

		यदि (G.flags & FLAG_गन्ती) {
			यदि (गन्ती == 0)
				रोक;
			यदि ((G.flags & FLAG_गन्ती_BYTES) && गन्ती < ibs)
				n = गन्ती;
		}

		n = dd_read(ibuf, n);
		यदि (n == 0)
			रोक;
		यदि (n < 0) {
			/* "Bad block" */
			यदि (!(G.flags & FLAG_NOERROR))
				गच्छ die_infile;
			bb_simple_perror_सन्देश(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, प्रस्तुत_से);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		यदि (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			यदि (prev_read_size & 1)
				bb_error_सन्देश_and_die("can't swab %lu byte buffer",
						(अचिन्हित दीर्घ)prev_read_size);
			prev_read_size = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (शून्य*) ibuf;
			n2 = (n >> 1);
			जबसम्म (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		गन्ती -= (G.flags & FLAG_गन्ती_BYTES) ? n : 1;
		यदि ((माप_प्रकार)n == ibs)
			G.in_full++;
		अन्यथा {
			G.in_part++;
			यदि (G.flags & FLAG_SYNC) {
				स_रखो(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#यदि ENABLE_FEATURE_DD_IBS_OBS
		यदि (G.flags & FLAG_TWOBUFS) {
			वर्ण *tmp = ibuf;
			जबसम्म (n) {
				माप_प्रकार d = obs - ocount;
				यदि (d > (माप_प्रकार)n)
					d = n;
				स_नकल(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				यदि (ocount == obs) {
					यदि (write_and_stats(obuf, obs, obs, outfile))
						गच्छ out_status;
					ocount = 0;
				}
			}
		} अन्यथा
#endif
		{
			यदि (write_and_stats(ibuf, n, obs, outfile))
				गच्छ out_status;
		}
	}

	यदि (G.flags & FLAG_FSYNC) {
		यदि (fsync(ofd) < 0)
			गच्छ die_outfile;
	}

#यदि ENABLE_FEATURE_DD_IBS_OBS
	यदि (ocount != 0) {
		यदि (write_and_stats(obuf, ocount, obs, outfile))
			गच्छ out_status;
	}
#endif
	यदि (बंद(ifd) < 0) {
 die_infile:
		bb_simple_perror_सन्देश_and_die(infile);
	}

	यदि (बंद(ofd) < 0) {
 die_outfile:
		bb_simple_perror_सन्देश_and_die(outfile);
	}

	exitcode = निकास_सफल;
 out_status:
	यदि (!ENABLE_FEATURE_DD_STATUS || !(G.flags & FLAG_STATUS_NONE))
		dd_output_status(0);

#यदि 0 /* can't just free(), they can be mmap()ed */
	यदि (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(obuf);
		यदि (G.flags & FLAG_TWOBUFS)
			मुक्त(ibuf);
	}
#endif

	फर्काऊ exitcode;
}
