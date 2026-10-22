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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


গণনা {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

কাঠামো globals {
	off_t out_full, out_part, in_full, in_part;
#যদি ENABLE_FEATURE_DD_THIRD_STATUS_সারি
	unsigned long long যোগ_bytes;
	unsigned long long begin_time_us;
#endif
	পূর্ণসংখ্যা flags;
} FIX_ALIASING;
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, আকার(G)); \
} যখন (0)

গণনা {
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
	FLAG_গুনতি_BYTES   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
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
	FLAG_গুনতি         = 1 << 13,
	FLAG_STATUS_NONE   = 1 << 14,
	FLAG_STATUS_NOXFER = 1 << 15,
};

স্থির শূন্য dd_output_status(পূর্ণসংখ্যা UNUSED_PARAM cur_signal)
{
#যদি ENABLE_FEATURE_DD_THIRD_STATUS_সারি
	দ্বিগুণ seconds;
	unsigned long long bytes_sec;
	unsigned long long now_us = monotonic_us(); /* before fprintf */
#endif

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.in_full, G.in_part,
			G.out_full, G.out_part);

#যদি ENABLE_FEATURE_DD_THIRD_STATUS_সারি
# যদি ENABLE_FEATURE_DD_STATUS
	যদি (G.flags & FLAG_STATUS_NOXFER) /* status=noxfer active? */
		ফেরত;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.যোগ_bytes,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.যোগ_bytes, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_time_us) / 1000000.0;
	bytes_sec = G.যোগ_bytes / seconds;
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(bytes_sec, 1, 0)
	);
#endif
}

#যদি ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
স্থির পূর্ণসংখ্যা clear_O_DIRECT(পূর্ণসংখ্যা fd)
{
	যদি (errno == EINVAL) {
		পূর্ণসংখ্যা fl = fcntl(fd, F_GETFL);
		যদি (fl & O_DIRECT) {
			fcntl(fd, F_SETFL, fl & ~O_DIRECT);
			ফেরত 1;
		}
	}
	ফেরত 0;
}
# endif
#endif

স্থির ssize_t dd_read(শূন্য *ibuf, আকার_t ibs)
{
	ssize_t n;

#যদি ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	যদি (G.flags & FLAG_FULLBLOCK)
		n = full_read(ifd, ibuf, ibs);
	অন্যথা
#endif
		n = safe_read(ifd, ibuf, ibs);
#যদি ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	যদি (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_DIRECT(ifd))
		যাও read_again;
# endif
#endif
	ফেরত n;
}

স্থির bool write_and_stats(ধ্রুবক শূন্য *buf, আকার_t len, আকার_t obs,
	ধ্রুবক অক্ষর *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = full_write(ofd, buf, len);
#যদি ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	যদি (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_DIRECT(ofd))
		যাও write_again;
# endif
#endif

#যদি ENABLE_FEATURE_DD_THIRD_STATUS_সারি
	যদি (n > 0)
		G.যোগ_bytes += n;
#endif
	যদি ((আকার_t)n == obs) {
		G.out_full++;
		ফেরত 0;
	}
	যদি ((আকার_t)n == len) {
		G.out_part++;
		ফেরত 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_বার্তা("error writing '%s'", filename);
	ফেরত 1;
}

#যদি ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#অন্যথা
# define XATOU_SFX xatoul_sfx
#endif

#যদি ENABLE_FEATURE_DD_IBS_OBS
স্থির পূর্ণসংখ্যা parse_comma_flags(অক্ষর *val, ধ্রুবক অক্ষর *শব্দগুলি, ধ্রুবক অক্ষর *error_in)
{
	পূর্ণসংখ্যা flags = 0;
	যখন (1) {
		পূর্ণসংখ্যা n;
		অক্ষর *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		যদি (arg)
			*arg = '\0';
		n = সূচক_in_strings(শব্দগুলি, val);
		যদি (n < 0)
			bb_error_বার্তা_and_die(bb_বার্তা_invalid_arg_to, val, error_in);
		flags |= (1 << n);
		যদি (!arg) /* no ',' left, so this was the last specifier */
			বিরতি;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	ফেরত flags;
}
#endif

স্থির শূন্য *alloc_buf(আকার_t আকার_2)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	যদি (আকার_2 >= bb_getpagesize())
		ফেরত xmmap_anon(আকার_2);
	ফেরত xmalloc(আকার_2);
}

পূর্ণসংখ্যা dd_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা dd_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	স্থির ধ্রুবক অক্ষর keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_STATUS("status\0")
#যদি ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#endif
		;
#যদি ENABLE_FEATURE_DD_IBS_OBS
	স্থির ধ্রুবক অক্ষর conv_শব্দগুলি[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	স্থির ধ্রুবক অক্ষর iflag_শব্দগুলি[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	স্থির ধ্রুবক অক্ষর oflag_শব্দগুলি[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#endif
#যদি ENABLE_FEATURE_DD_STATUS
	স্থির ধ্রুবক অক্ষর status_শব্দগুলি[] ALIGN1 =
		"none\0""noxfer\0";
#endif
	গণনা {
		OP_bs = 0,
		OP_গুনতি,
		OP_seek,
		OP_skip,
		OP_if,
		OP_of,
		IF_FEATURE_DD_STATUS(OP_status,)
#যদি ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_গুনতি_bytes,
		OP_iflag_fullblock,
		OP_iflag_direct,
		OP_oflag_seek_bytes,
		OP_oflag_direct,
#endif
	};
	exitcode_t exitcode = EXIT_FAILURE;
	পূর্ণসংখ্যা i;
	আকার_t ibs = 512;
	অক্ষর *ibuf;
#যদি ENABLE_FEATURE_DD_IBS_OBS
	আকার_t obs = 512;
	অক্ষর *obuf;
#অন্যথা
# define obs  ibs
# define obuf ibuf
#endif
	/* These are all zeroed at once! */
	কাঠামো {
		IF_FEATURE_DD_IBS_OBS(আকার_t ocount;)
		ssize_t পূর্ব_read_আকার; /* for detecting swab failure */
		off_t গুনতি;
		off_t seek, skip;
		ধ্রুবক অক্ষর *infile, *outfile;
	} Z;
#define ocount  (Z.ocount )
#define পূর্ব_read_আকার (Z.পূর্ব_read_আকার)
#define গুনতি   (Z.গুনতি  )
#define seek    (Z.seek   )
#define skip    (Z.skip   )
#define infile  (Z.infile )
#define outfile (Z.outfile)

	memset(&Z, 0, আকার(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	জন্য (i = 1; argv[i]; i++) {
		পূর্ণসংখ্যা what;
		অক্ষর *val;
		অক্ষর *arg = argv[i];

#যদি ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		যদি (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			চালু;
#endif
		val = strchr(arg, '=');
		যদি (val == ফাঁকা)
			bb_show_usage();
		*val = '\0';
		what = সূচক_in_strings(keywords, arg);
		যদি (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#যদি ENABLE_FEATURE_DD_IBS_OBS
		যদি (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((আকার_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		যদি (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((আকার_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		যদি (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_শব্দগুলি, "conv");
			/*continue;*/
		}
		যদি (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_শব্দগুলি, "iflag") << FLAG_IFLAG_SHIFT;
			/*continue;*/
		}
		যদি (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_শব্দগুলি, "oflag") << FLAG_OFLAG_SHIFT;
			/*continue;*/
		}
#endif
		যদি (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((আকার_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		যদি (what == OP_গুনতি) {
			G.flags |= FLAG_গুনতি;
			গুনতি = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		যদি (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		যদি (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		যদি (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		যদি (what == OP_of) {
			outfile = val;
			/*continue;*/
		}
#যদি ENABLE_FEATURE_DD_STATUS
		যদি (what == OP_status) {
			পূর্ণসংখ্যা n;
			n = সূচক_in_strings(status_শব্দগুলি, val);
			যদি (n < 0)
				bb_error_বার্তা_and_die(bb_বার্তা_invalid_arg_to, val, "status");
			G.flags |= FLAG_STATUS_NONE << n;
			/*continue;*/
		}
#endif
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#যদি ENABLE_FEATURE_DD_IBS_OBS
	যদি (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#endif

#যদি ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_empty_mask(SIGUSR1, dd_output_status);
#endif
#যদি ENABLE_FEATURE_DD_THIRD_STATUS_সারি
	G.begin_time_us = monotonic_us();
#endif

	যদি (infile) {
		পূর্ণসংখ্যা iflag = O_RDONLY;
#যদি ENABLE_FEATURE_DD_IBS_OBS
		যদি (G.flags & FLAG_IDIRECT) {
# ifdef O_DIRECT
			iflag |= O_DIRECT;
# অন্যথা
			bb_error_বার্তা_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(infile, iflag), ifd);
	} অন্যথা {
		infile = bb_বার্তা_standard_প্রবেশ;
	}
	যদি (outfile) {
		পূর্ণসংখ্যা oflag = O_WRONLY | O_CREAT;

		যদি (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		যদি (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#যদি ENABLE_FEATURE_DD_IBS_OBS
		যদি (G.flags & FLAG_ODIRECT) {
# ifdef O_DIRECT
			oflag |= O_DIRECT;
# অন্যথা
			bb_error_বার্তা_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(outfile, oflag), ofd);

		যদি (seek && !(G.flags & FLAG_NOTRUNC)) {
			আকার_t blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
			যদি (ftruncate(ofd, seek * blocksz) < 0) {
				কাঠামো stat st;

				যদি (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_mode)
				 || S_ISDIR(st.st_mode)
				) {
					যাও die_outfile;
				}
			}
		}
	} অন্যথা {
		outfile = bb_বার্তা_standard_output;
	}
	যদি (skip) {
		আকার_t blocksz = (G.flags & FLAG_SKIP_BYTES) ? 1 : ibs;
		যদি (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			করো {
				ssize_t n = dd_read(ibuf, blocksz);
				যদি (n < 0)
					যাও die_infile;
				যদি (n == 0)
					বিরতি;
			} যখন (--skip != 0);
		}
	}
	যদি (seek) {
		আকার_t blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
		যদি (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			যাও die_outfile;
	}

	যখন (1) {
		ssize_t n = ibs;

		যদি (G.flags & FLAG_গুনতি) {
			যদি (গুনতি == 0)
				বিরতি;
			যদি ((G.flags & FLAG_গুনতি_BYTES) && গুনতি < ibs)
				n = গুনতি;
		}

		n = dd_read(ibuf, n);
		যদি (n == 0)
			বিরতি;
		যদি (n < 0) {
			/* "Bad block" */
			যদি (!(G.flags & FLAG_NOERROR))
				যাও die_infile;
			bb_simple_perror_বার্তা(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		যদি (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			যদি (পূর্ব_read_আকার & 1)
				bb_error_বার্তা_and_die("can't swab %lu byte buffer",
						(unsigned long)পূর্ব_read_আকার);
			পূর্ব_read_আকার = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (শূন্য*) ibuf;
			n2 = (n >> 1);
			যখন (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		গুনতি -= (G.flags & FLAG_গুনতি_BYTES) ? n : 1;
		যদি ((আকার_t)n == ibs)
			G.in_full++;
		অন্যথা {
			G.in_part++;
			যদি (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#যদি ENABLE_FEATURE_DD_IBS_OBS
		যদি (G.flags & FLAG_TWOBUFS) {
			অক্ষর *tmp = ibuf;
			যখন (n) {
				আকার_t d = obs - ocount;
				যদি (d > (আকার_t)n)
					d = n;
				memcpy(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				যদি (ocount == obs) {
					যদি (write_and_stats(obuf, obs, obs, outfile))
						যাও out_status;
					ocount = 0;
				}
			}
		} অন্যথা
#endif
		{
			যদি (write_and_stats(ibuf, n, obs, outfile))
				যাও out_status;
		}
	}

	যদি (G.flags & FLAG_FSYNC) {
		যদি (fsync(ofd) < 0)
			যাও die_outfile;
	}

#যদি ENABLE_FEATURE_DD_IBS_OBS
	যদি (ocount != 0) {
		যদি (write_and_stats(obuf, ocount, obs, outfile))
			যাও out_status;
	}
#endif
	যদি (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_বার্তা_and_die(infile);
	}

	যদি (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_বার্তা_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 out_status:
	যদি (!ENABLE_FEATURE_DD_STATUS || !(G.flags & FLAG_STATUS_NONE))
		dd_output_status(0);

#যদি 0 /* can't just free(), they can be mmap()ed */
	যদি (ENABLE_FEATURE_CLEAN_UP) {
		মুক্ত(obuf);
		যদি (G.flags & FLAG_TWOBUFS)
			মুক্ত(ibuf);
	}
#endif

	ফেরত exitcode;
}
