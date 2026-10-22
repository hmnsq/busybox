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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


شمار {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

ساخت globals {
	off_t برآمد_بھرا, برآمد_part, اندر_بھرا, اندر_part;
#اگر ENABLE_FEATURE_DD_تیسرا_حالت_سطر
	غیرعلامتی طویل طویل کل_بائٹیں;
	غیرعلامتی طویل طویل begin_وقت_us;
#اگر_ختم
	صحیح flags;
} FIX_ALIASING;
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, حجم(G)); \
} جب (0)

شمار {
	/* Must be in the same order as OP_conv_XXX! */
	/* (see "flags |= (1 << what)" below) */
	FLAG_NOTRUNC = (1 << 0) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SYNC    = (1 << 1) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_NOERROR = (1 << 2) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FSYNC   = (1 << 3) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SWAB    = (1 << 4) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of conv flags */
	/* start of input flags */
	FLAG_IFLAG_سرکاؤ   = 5,
	FLAG_SKIP_بائٹیں    = (1 << 5) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_گنتی_بائٹیں   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FULLBLOCK     = (1 << 7) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_IDIRECT       = (1 << 8) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of input flags */
	/* start of output flags */
	FLAG_OFLAG_سرکاؤ   = 9,
	FLAG_SEEK_بائٹیں    = (1 << 9) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_APPEND        = (1 << 10) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ODIRECT       = (1 << 11) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of output flags */
	FLAG_TWOBUFS       = (1 << 12) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_گنتی         = 1 << 13,
	FLAG_حالت_NONE   = 1 << 14,
	FLAG_حالت_NOXFER = 1 << 15,
};

ساکن خالی dd_output_حالت(صحیح غیر_مستعمل_PARAM cur_signal)
{
#اگر ENABLE_FEATURE_DD_تیسرا_حالت_سطر
	دوہرا seconds;
	غیرعلامتی طویل طویل بائٹیں_sec;
	غیرعلامتی طویل طویل now_us = monotonic_us(); /* before fprintf */
#اگر_ختم

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.اندر_بھرا, G.اندر_part,
			G.برآمد_بھرا, G.برآمد_part);

#اگر ENABLE_FEATURE_DD_تیسرا_حالت_سطر
# اگر ENABLE_FEATURE_DD_حالت
	اگر (G.flags & FLAG_حالت_NOXFER) /* status=noxfer active? */
		واپس;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.کل_بائٹیں,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.کل_بائٹیں, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_وقت_us) / 1000000.0;
	بائٹیں_sec = G.کل_بائٹیں / seconds;
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(بائٹیں_sec, 1, 0)
	);
#اگر_ختم
}

#اگر ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_براہ_راست
ساکن صحیح clear_O_براہ_راست(صحیح fd)
{
	اگر (errno == EINVAL) {
		صحیح fl = fcntl(fd, F_GETFL);
		اگر (fl & O_براہ_راست) {
			fcntl(fd, F_SETFL, fl & ~O_براہ_راست);
			واپس 1;
		}
	}
	واپس 0;
}
# endif
#اگر_ختم

ساکن ssize_t dd_read(خالی *ibuf, سائز_t ibs)
{
	ssize_t n;

#اگر ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	اگر (G.flags & FLAG_FULLBLOCK)
		n = بھرا_read(ifd, ibuf, ibs);
	ورنہ
#اگر_ختم
		n = محفوظ_read(ifd, ibuf, ibs);
#اگر ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_براہ_راست
	اگر (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_براہ_راست(ifd))
		جاؤ read_again;
# endif
#اگر_ختم
	واپس n;
}

ساکن bool write_and_stats(ثابت خالی *buf, سائز_t len, سائز_t obs,
	ثابت حرف *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = بھرا_write(ofd, buf, len);
#اگر ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_براہ_راست
	اگر (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_براہ_راست(ofd))
		جاؤ write_again;
# endif
#اگر_ختم

#اگر ENABLE_FEATURE_DD_تیسرا_حالت_سطر
	اگر (n > 0)
		G.کل_بائٹیں += n;
#اگر_ختم
	اگر ((سائز_t)n == obs) {
		G.برآمد_بھرا++;
		واپس 0;
	}
	اگر ((سائز_t)n == len) {
		G.برآمد_part++;
		واپس 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_پیغام("error writing '%s'", filename);
	واپس 1;
}

#اگر ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ورنہ
# define XATOU_SFX xatoul_sfx
#اگر_ختم

#اگر ENABLE_FEATURE_DD_IBS_OBS
ساکن صحیح parse_comma_flags(حرف *val, ثابت حرف *الفاظ, ثابت حرف *error_اندر)
{
	صحیح flags = 0;
	جب (1) {
		صحیح n;
		حرف *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		اگر (arg)
			*arg = '\0';
		n = اشاریہ_اندر_لڑیاں(الفاظ, val);
		اگر (n < 0)
			bb_error_پیغام_and_die(bb_پیغام_invalid_arg_مقصد, val, error_اندر);
		flags |= (1 << n);
		اگر (!arg) /* no ',' left, so this was the last specifier */
			توڑو;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	واپس flags;
}
#اگر_ختم

ساکن خالی *alloc_buf(سائز_t سائز)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	اگر (سائز >= bb_getpagesize())
		واپس xmmap_anon(سائز);
	واپس xmalloc(سائز);
}

صحیح dd_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح dd_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	ساکن ثابت حرف keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_حالت("status\0")
#اگر ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#اگر_ختم
		;
#اگر ENABLE_FEATURE_DD_IBS_OBS
	ساکن ثابت حرف conv_الفاظ[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	ساکن ثابت حرف iflag_الفاظ[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	ساکن ثابت حرف oflag_الفاظ[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#اگر_ختم
#اگر ENABLE_FEATURE_DD_حالت
	ساکن ثابت حرف حالت_الفاظ[] ALIGN1 =
		"none\0""noxfer\0";
#اگر_ختم
	شمار {
		OP_bs = 0,
		OP_گنتی,
		OP_seek,
		OP_skip,
		OP_if,
		OP_کا,
		IF_FEATURE_DD_حالت(OP_حالت,)
#اگر ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_skip_بائٹیں,
		OP_iflag_گنتی_بائٹیں,
		OP_iflag_fullblock,
		OP_iflag_براہ_راست,
		OP_oflag_seek_بائٹیں,
		OP_oflag_براہ_راست,
#اگر_ختم
	};
	exitcode_t exitcode = EXIT_FAILURE;
	صحیح i;
	سائز_t ibs = 512;
	حرف *ibuf;
#اگر ENABLE_FEATURE_DD_IBS_OBS
	سائز_t obs = 512;
	حرف *obuf;
#ورنہ
# define obs  ibs
# define obuf ibuf
#اگر_ختم
	/* These are all zeroed at once! */
	ساخت {
		IF_FEATURE_DD_IBS_OBS(سائز_t ocount;)
		ssize_t پچھلا_read_سائز; /* for detecting swab failure */
		off_t گنتی;
		off_t seek, skip;
		ثابت حرف *infile, *outfile;
	} Z;
#تعریف_کرو ocount  (Z.ocount )
#تعریف_کرو پچھلا_read_سائز (Z.پچھلا_read_سائز)
#تعریف_کرو گنتی   (Z.گنتی  )
#تعریف_کرو seek    (Z.seek   )
#تعریف_کرو skip    (Z.skip   )
#تعریف_کرو infile  (Z.infile )
#تعریف_کرو outfile (Z.outfile)

	memset(&Z, 0, حجم(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	برائے (i = 1; argv[i]; i++) {
		صحیح what;
		حرف *val;
		حرف *arg = argv[i];

#اگر ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		اگر (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			جاری;
#اگر_ختم
		val = strchr(arg, '=');
		اگر (val == تہی)
			bb_show_usage();
		*val = '\0';
		what = اشاریہ_اندر_لڑیاں(keywords, arg);
		اگر (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#اگر ENABLE_FEATURE_DD_IBS_OBS
		اگر (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((سائز_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		اگر (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((سائز_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		اگر (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_الفاظ, "conv");
			/*continue;*/
		}
		اگر (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_الفاظ, "iflag") << FLAG_IFLAG_سرکاؤ;
			/*continue;*/
		}
		اگر (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_الفاظ, "oflag") << FLAG_OFLAG_سرکاؤ;
			/*continue;*/
		}
#اگر_ختم
		اگر (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((سائز_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		اگر (what == OP_گنتی) {
			G.flags |= FLAG_گنتی;
			گنتی = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		اگر (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		اگر (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		اگر (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		اگر (what == OP_کا) {
			outfile = val;
			/*continue;*/
		}
#اگر ENABLE_FEATURE_DD_حالت
		اگر (what == OP_حالت) {
			صحیح n;
			n = اشاریہ_اندر_لڑیاں(حالت_الفاظ, val);
			اگر (n < 0)
				bb_error_پیغام_and_die(bb_پیغام_invalid_arg_مقصد, val, "status");
			G.flags |= FLAG_حالت_NONE << n;
			/*continue;*/
		}
#اگر_ختم
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#اگر ENABLE_FEATURE_DD_IBS_OBS
	اگر (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#اگر_ختم

#اگر ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_خالی_خانے_نقاب(SIGUSR1, dd_output_حالت);
#اگر_ختم
#اگر ENABLE_FEATURE_DD_تیسرا_حالت_سطر
	G.begin_وقت_us = monotonic_us();
#اگر_ختم

	اگر (infile) {
		صحیح iflag = O_RDONLY;
#اگر ENABLE_FEATURE_DD_IBS_OBS
		اگر (G.flags & FLAG_IDIRECT) {
# ifdef O_براہ_راست
			iflag |= O_براہ_راست;
# ورنہ
			bb_error_پیغام_and_die("O_DIRECT not supported on this platform");
# endif
		}
#اگر_ختم
		xmove_fd(xopen(infile, iflag), ifd);
	} ورنہ {
		infile = bb_پیغام_standard_داخلہ;
	}
	اگر (outfile) {
		صحیح oflag = O_WRONLY | O_CREAT;

		اگر (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		اگر (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#اگر ENABLE_FEATURE_DD_IBS_OBS
		اگر (G.flags & FLAG_ODIRECT) {
# ifdef O_براہ_راست
			oflag |= O_براہ_راست;
# ورنہ
			bb_error_پیغام_and_die("O_DIRECT not supported on this platform");
# endif
		}
#اگر_ختم
		xmove_fd(xopen(outfile, oflag), ofd);

		اگر (seek && !(G.flags & FLAG_NOTRUNC)) {
			سائز_t blocksz = (G.flags & FLAG_SEEK_بائٹیں) ? 1 : obs;
			اگر (ftruncate(ofd, seek * blocksz) < 0) {
				ساخت stat st;

				اگر (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_منوال)
				 || S_ISDIR(st.st_منوال)
				) {
					جاؤ die_outfile;
				}
			}
		}
	} ورنہ {
		outfile = bb_پیغام_standard_output;
	}
	اگر (skip) {
		سائز_t blocksz = (G.flags & FLAG_SKIP_بائٹیں) ? 1 : ibs;
		اگر (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			کرو {
				ssize_t n = dd_read(ibuf, blocksz);
				اگر (n < 0)
					جاؤ die_infile;
				اگر (n == 0)
					توڑو;
			} جب (--skip != 0);
		}
	}
	اگر (seek) {
		سائز_t blocksz = (G.flags & FLAG_SEEK_بائٹیں) ? 1 : obs;
		اگر (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			جاؤ die_outfile;
	}

	جب (1) {
		ssize_t n = ibs;

		اگر (G.flags & FLAG_گنتی) {
			اگر (گنتی == 0)
				توڑو;
			اگر ((G.flags & FLAG_گنتی_بائٹیں) && گنتی < ibs)
				n = گنتی;
		}

		n = dd_read(ibuf, n);
		اگر (n == 0)
			توڑو;
		اگر (n < 0) {
			/* "Bad block" */
			اگر (!(G.flags & FLAG_NOERROR))
				جاؤ die_infile;
			bb_simple_perror_پیغام(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		اگر (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			اگر (پچھلا_read_سائز & 1)
				bb_error_پیغام_and_die("can't swab %lu byte buffer",
						(غیرعلامتی طویل)پچھلا_read_سائز);
			پچھلا_read_سائز = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (خالی*) ibuf;
			n2 = (n >> 1);
			جب (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		گنتی -= (G.flags & FLAG_گنتی_بائٹیں) ? n : 1;
		اگر ((سائز_t)n == ibs)
			G.اندر_بھرا++;
		ورنہ {
			G.اندر_part++;
			اگر (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#اگر ENABLE_FEATURE_DD_IBS_OBS
		اگر (G.flags & FLAG_TWOBUFS) {
			حرف *tmp = ibuf;
			جب (n) {
				سائز_t d = obs - ocount;
				اگر (d > (سائز_t)n)
					d = n;
				حافظہ_نقل(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				اگر (ocount == obs) {
					اگر (write_and_stats(obuf, obs, obs, outfile))
						جاؤ برآمد_حالت;
					ocount = 0;
				}
			}
		} ورنہ
#اگر_ختم
		{
			اگر (write_and_stats(ibuf, n, obs, outfile))
				جاؤ برآمد_حالت;
		}
	}

	اگر (G.flags & FLAG_FSYNC) {
		اگر (fsync(ofd) < 0)
			جاؤ die_outfile;
	}

#اگر ENABLE_FEATURE_DD_IBS_OBS
	اگر (ocount != 0) {
		اگر (write_and_stats(obuf, ocount, obs, outfile))
			جاؤ برآمد_حالت;
	}
#اگر_ختم
	اگر (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_پیغام_and_die(infile);
	}

	اگر (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_پیغام_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 برآمد_حالت:
	اگر (!ENABLE_FEATURE_DD_حالت || !(G.flags & FLAG_حالت_NONE))
		dd_output_حالت(0);

#اگر 0 /* can't just free(), they can be mmap()ed */
	اگر (ENABLE_FEATURE_CLEAN_اوپر) {
		آزاد(obuf);
		اگر (G.flags & FLAG_TWOBUFS)
			آزاد(ibuf);
	}
#اگر_ختم

	واپس exitcode;
}
