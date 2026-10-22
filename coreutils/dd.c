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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


تعداد {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

بنية globals {
	off_t out_full, out_part, in_full, in_part;
#إذا ENABLE_FEATURE_DD_THIRD_STATUS_باريس
	unsigned long long total_bytes;
	unsigned long long begin_time_us;
#endif
	صحيح flags;
} FIX_ALIASING;
#define G (*(بنية globals*)bb_common_bufsiz1)
#define INIT_G() افعل { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, حجم(G)); \
} طالما (0)

تعداد {
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
	FLAG_کيراءن_BYTES   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
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
	FLAG_کيراءن         = 1 << 13,
	FLAG_STATUS_NONE   = 1 << 14,
	FLAG_STATUS_NOXFER = 1 << 15,
};

ساكن فراغ dd_output_status(صحيح UNUSED_PARAM cur_signal)
{
#إذا ENABLE_FEATURE_DD_THIRD_STATUS_باريس
	مزدوج seconds;
	unsigned long long bytes_sec;
	unsigned long long now_us = monotonic_us(); /* before fprintf */
#endif

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.in_full, G.in_part,
			G.out_full, G.out_part);

#إذا ENABLE_FEATURE_DD_THIRD_STATUS_باريس
# إذا ENABLE_FEATURE_DD_STATUS
	إذا (G.flags & FLAG_STATUS_NOXFER) /* status=noxfer active? */
		أرجع;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
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
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(bytes_sec, 1, 0)
	);
#endif
}

#إذا ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
ساكن صحيح clear_O_DIRECT(صحيح fd)
{
	إذا (errno == EINVAL) {
		صحيح fl = fcntl(fd, F_GETFL);
		إذا (fl & O_DIRECT) {
			fcntl(fd, F_SETFL, fl & ~O_DIRECT);
			أرجع 1;
		}
	}
	أرجع 0;
}
# endif
#endif

ساكن ssize_t dd_read(فراغ *ibuf, size_t ibs)
{
	ssize_t n;

#إذا ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	إذا (G.flags & FLAG_FULLBLOCK)
		n = full_read(ifd, ibuf, ibs);
	وإلا
#endif
		n = safe_read(ifd, ibuf, ibs);
#إذا ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	إذا (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_DIRECT(ifd))
		اذهب read_again;
# endif
#endif
	أرجع n;
}

ساكن bool write_and_stats(ثابت فراغ *buf, size_t len, size_t obs,
	ثابت حرف *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = full_write(ofd, buf, len);
#إذا ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	إذا (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_DIRECT(ofd))
		اذهب write_again;
# endif
#endif

#إذا ENABLE_FEATURE_DD_THIRD_STATUS_باريس
	إذا (n > 0)
		G.total_bytes += n;
#endif
	إذا ((size_t)n == obs) {
		G.out_full++;
		أرجع 0;
	}
	إذا ((size_t)n == len) {
		G.out_part++;
		أرجع 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_ڤسنن("error writing '%s'", filename);
	أرجع 1;
}

#إذا ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#وإلا
# define XATOU_SFX xatoul_sfx
#endif

#إذا ENABLE_FEATURE_DD_IBS_OBS
ساكن صحيح parse_comma_flags(حرف *val, ثابت حرف *ڤرکاتاءن, ثابت حرف *error_in)
{
	صحيح flags = 0;
	طالما (1) {
		صحيح n;
		حرف *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		إذا (arg)
			*arg = '\0';
		n = index_in_strings(ڤرکاتاءن, val);
		إذا (n < 0)
			bb_error_ڤسنن_and_die(bb_ڤسنن_invalid_arg_to, val, error_in);
		flags |= (1 << n);
		إذا (!arg) /* no ',' left, so this was the last specifier */
			اكسر;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	أرجع flags;
}
#endif

ساكن فراغ *alloc_buf(size_t size)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	إذا (size >= bb_getpagesize())
		أرجع xmmap_anon(size);
	أرجع xmalloc(size);
}

صحيح dd_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح dd_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	ساكن ثابت حرف keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_STATUS("status\0")
#إذا ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#endif
		;
#إذا ENABLE_FEATURE_DD_IBS_OBS
	ساكن ثابت حرف conv_ڤرکاتاءن[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	ساكن ثابت حرف iflag_ڤرکاتاءن[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	ساكن ثابت حرف oflag_ڤرکاتاءن[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#endif
#إذا ENABLE_FEATURE_DD_STATUS
	ساكن ثابت حرف status_ڤرکاتاءن[] ALIGN1 =
		"none\0""noxfer\0";
#endif
	تعداد {
		OP_bs = 0,
		OP_کيراءن,
		OP_seek,
		OP_skip,
		OP_if,
		OP_of,
		IF_FEATURE_DD_STATUS(OP_status,)
#إذا ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_کيراءن_bytes,
		OP_iflag_fullblock,
		OP_iflag_direct,
		OP_oflag_seek_bytes,
		OP_oflag_direct,
#endif
	};
	exitcode_t exitcode = EXIT_FAILURE;
	صحيح i;
	size_t ibs = 512;
	حرف *ibuf;
#إذا ENABLE_FEATURE_DD_IBS_OBS
	size_t obs = 512;
	حرف *obuf;
#وإلا
# define obs  ibs
# define obuf ibuf
#endif
	/* These are all zeroed at once! */
	بنية {
		IF_FEATURE_DD_IBS_OBS(size_t ocount;)
		ssize_t prev_read_size; /* for detecting swab failure */
		off_t کيراءن;
		off_t seek, skip;
		ثابت حرف *infile, *outfile;
	} Z;
#define ocount  (Z.ocount )
#define prev_read_size (Z.prev_read_size)
#define کيراءن   (Z.کيراءن  )
#define seek    (Z.seek   )
#define skip    (Z.skip   )
#define infile  (Z.infile )
#define outfile (Z.outfile)

	memset(&Z, 0, حجم(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	لكل (i = 1; argv[i]; i++) {
		صحيح what;
		حرف *val;
		حرف *arg = argv[i];

#إذا ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		إذا (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			استمر;
#endif
		val = strchr(arg, '=');
		إذا (val == عدم)
			bb_show_usage();
		*val = '\0';
		what = index_in_strings(keywords, arg);
		إذا (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#إذا ENABLE_FEATURE_DD_IBS_OBS
		إذا (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((size_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		إذا (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((size_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		إذا (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_ڤرکاتاءن, "conv");
			/*continue;*/
		}
		إذا (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_ڤرکاتاءن, "iflag") << FLAG_IFLAG_SHIFT;
			/*continue;*/
		}
		إذا (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_ڤرکاتاءن, "oflag") << FLAG_OFLAG_SHIFT;
			/*continue;*/
		}
#endif
		إذا (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((size_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		إذا (what == OP_کيراءن) {
			G.flags |= FLAG_کيراءن;
			کيراءن = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		إذا (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		إذا (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		إذا (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		إذا (what == OP_of) {
			outfile = val;
			/*continue;*/
		}
#إذا ENABLE_FEATURE_DD_STATUS
		إذا (what == OP_status) {
			صحيح n;
			n = index_in_strings(status_ڤرکاتاءن, val);
			إذا (n < 0)
				bb_error_ڤسنن_and_die(bb_ڤسنن_invalid_arg_to, val, "status");
			G.flags |= FLAG_STATUS_NONE << n;
			/*continue;*/
		}
#endif
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#إذا ENABLE_FEATURE_DD_IBS_OBS
	إذا (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#endif

#إذا ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_empty_mask(SIGUSR1, dd_output_status);
#endif
#إذا ENABLE_FEATURE_DD_THIRD_STATUS_باريس
	G.begin_time_us = monotonic_us();
#endif

	إذا (infile) {
		صحيح iflag = O_RDONLY;
#إذا ENABLE_FEATURE_DD_IBS_OBS
		إذا (G.flags & FLAG_IDIRECT) {
# ifdef O_DIRECT
			iflag |= O_DIRECT;
# وإلا
			bb_error_ڤسنن_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(infile, iflag), ifd);
	} وإلا {
		infile = bb_ڤسنن_standard_ماسوقن;
	}
	إذا (outfile) {
		صحيح oflag = O_WRONLY | O_CREAT;

		إذا (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		إذا (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#إذا ENABLE_FEATURE_DD_IBS_OBS
		إذا (G.flags & FLAG_ODIRECT) {
# ifdef O_DIRECT
			oflag |= O_DIRECT;
# وإلا
			bb_error_ڤسنن_and_die("O_DIRECT not supported on this platform");
# endif
		}
#endif
		xmove_fd(xopen(outfile, oflag), ofd);

		إذا (seek && !(G.flags & FLAG_NOTRUNC)) {
			size_t blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
			إذا (ftruncate(ofd, seek * blocksz) < 0) {
				بنية stat st;

				إذا (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_mode)
				 || S_ISDIR(st.st_mode)
				) {
					اذهب die_outfile;
				}
			}
		}
	} وإلا {
		outfile = bb_ڤسنن_standard_output;
	}
	إذا (skip) {
		size_t blocksz = (G.flags & FLAG_SKIP_BYTES) ? 1 : ibs;
		إذا (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			افعل {
				ssize_t n = dd_read(ibuf, blocksz);
				إذا (n < 0)
					اذهب die_infile;
				إذا (n == 0)
					اكسر;
			} طالما (--skip != 0);
		}
	}
	إذا (seek) {
		size_t blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
		إذا (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			اذهب die_outfile;
	}

	طالما (1) {
		ssize_t n = ibs;

		إذا (G.flags & FLAG_کيراءن) {
			إذا (کيراءن == 0)
				اكسر;
			إذا ((G.flags & FLAG_کيراءن_BYTES) && کيراءن < ibs)
				n = کيراءن;
		}

		n = dd_read(ibuf, n);
		إذا (n == 0)
			اكسر;
		إذا (n < 0) {
			/* "Bad block" */
			إذا (!(G.flags & FLAG_NOERROR))
				اذهب die_infile;
			bb_simple_perror_ڤسنن(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		إذا (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			إذا (prev_read_size & 1)
				bb_error_ڤسنن_and_die("can't swab %lu byte buffer",
						(unsigned long)prev_read_size);
			prev_read_size = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (فراغ*) ibuf;
			n2 = (n >> 1);
			طالما (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		کيراءن -= (G.flags & FLAG_کيراءن_BYTES) ? n : 1;
		إذا ((size_t)n == ibs)
			G.in_full++;
		وإلا {
			G.in_part++;
			إذا (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#إذا ENABLE_FEATURE_DD_IBS_OBS
		إذا (G.flags & FLAG_TWOBUFS) {
			حرف *tmp = ibuf;
			طالما (n) {
				size_t d = obs - ocount;
				إذا (d > (size_t)n)
					d = n;
				memcpy(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				إذا (ocount == obs) {
					إذا (write_and_stats(obuf, obs, obs, outfile))
						اذهب out_status;
					ocount = 0;
				}
			}
		} وإلا
#endif
		{
			إذا (write_and_stats(ibuf, n, obs, outfile))
				اذهب out_status;
		}
	}

	إذا (G.flags & FLAG_FSYNC) {
		إذا (fsync(ofd) < 0)
			اذهب die_outfile;
	}

#إذا ENABLE_FEATURE_DD_IBS_OBS
	إذا (ocount != 0) {
		إذا (write_and_stats(obuf, ocount, obs, outfile))
			اذهب out_status;
	}
#endif
	إذا (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_ڤسنن_and_die(infile);
	}

	إذا (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_ڤسنن_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 out_status:
	إذا (!ENABLE_FEATURE_DD_STATUS || !(G.flags & FLAG_STATUS_NONE))
		dd_output_status(0);

#إذا 0 /* can't just free(), they can be mmap()ed */
	إذا (ENABLE_FEATURE_CLEAN_UP) {
		حرر(obuf);
		إذا (G.flags & FLAG_TWOBUFS)
			حرر(ibuf);
	}
#endif

	أرجع exitcode;
}
