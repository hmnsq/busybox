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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


ਗਿਣਤੀ {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

ਬਣਤਰ globals {
	off_t ਬਾਹਰਲਾ_ਭਰਿਆ, ਬਾਹਰਲਾ_part, ਵਿੱਚ_ਭਰਿਆ, ਵਿੱਚ_part;
#ਜੇ ENABLE_FEATURE_DD_ਤੀਜਾ_ਸਥਿਤੀ_ਸਤਰ
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਕੁੱਲ_ਬਾਈਟਾਂ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ begin_ਸਮਾਂ_us;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ flags;
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, ਆਕਾਰ(G)); \
} ਜਦੋਂ (0)

ਗਿਣਤੀ {
	/* Must be in the same order as OP_conv_XXX! */
	/* (see "flags |= (1 << what)" below) */
	FLAG_NOTRUNC = (1 << 0) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SYNC    = (1 << 1) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_NOERROR = (1 << 2) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FSYNC   = (1 << 3) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SWAB    = (1 << 4) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of conv flags */
	/* start of input flags */
	FLAG_IFLAG_ਖਿਸਕਾਓ   = 5,
	FLAG_SKIP_ਬਾਈਟਾਂ    = (1 << 5) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ਗਿਣਤੀ_ਬਾਈਟਾਂ   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FULLBLOCK     = (1 << 7) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_IDIRECT       = (1 << 8) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of input flags */
	/* start of output flags */
	FLAG_OFLAG_ਖਿਸਕਾਓ   = 9,
	FLAG_SEEK_ਬਾਈਟਾਂ    = (1 << 9) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_APPEND        = (1 << 10) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ODIRECT       = (1 << 11) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of output flags */
	FLAG_TWOBUFS       = (1 << 12) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ਗਿਣਤੀ         = 1 << 13,
	FLAG_ਸਥਿਤੀ_NONE   = 1 << 14,
	FLAG_ਸਥਿਤੀ_NOXFER = 1 << 15,
};

ਸਥਿਰ ਖਾਲੀ dd_output_ਸਥਿਤੀ(ਪੂਰਨ ਅਣਵਰਤਿਆ_PARAM cur_signal)
{
#ਜੇ ENABLE_FEATURE_DD_ਤੀਜਾ_ਸਥਿਤੀ_ਸਤਰ
	ਦੁੱਗਣਾ seconds;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਬਾਈਟਾਂ_sec;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ now_us = monotonic_us(); /* before fprintf */
#ਜੇ_ਅੰਤ

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.ਵਿੱਚ_ਭਰਿਆ, G.ਵਿੱਚ_part,
			G.ਬਾਹਰਲਾ_ਭਰਿਆ, G.ਬਾਹਰਲਾ_part);

#ਜੇ ENABLE_FEATURE_DD_ਤੀਜਾ_ਸਥਿਤੀ_ਸਤਰ
# ਜੇ ENABLE_FEATURE_DD_ਸਥਿਤੀ
	ਜੇ (G.flags & FLAG_ਸਥਿਤੀ_NOXFER) /* status=noxfer active? */
		ਵਾਪਸ;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.ਕੁੱਲ_ਬਾਈਟਾਂ,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.ਕੁੱਲ_ਬਾਈਟਾਂ, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_ਸਮਾਂ_us) / 1000000.0;
	ਬਾਈਟਾਂ_sec = G.ਕੁੱਲ_ਬਾਈਟਾਂ / seconds;
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(ਬਾਈਟਾਂ_sec, 1, 0)
	);
#ਜੇ_ਅੰਤ
}

#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ਸਿੱਧਾ
ਸਥਿਰ ਪੂਰਨ clear_O_ਸਿੱਧਾ(ਪੂਰਨ fd)
{
	ਜੇ (errno == EINVAL) {
		ਪੂਰਨ fl = fcntl(fd, F_GETFL);
		ਜੇ (fl & O_ਸਿੱਧਾ) {
			fcntl(fd, F_SETFL, fl & ~O_ਸਿੱਧਾ);
			ਵਾਪਸ 1;
		}
	}
	ਵਾਪਸ 0;
}
# endif
#ਜੇ_ਅੰਤ

ਸਥਿਰ ssize_t dd_read(ਖਾਲੀ *ibuf, ਮਾਪ_t ibs)
{
	ssize_t n;

#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	ਜੇ (G.flags & FLAG_FULLBLOCK)
		n = ਭਰਿਆ_read(ifd, ibuf, ibs);
	ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		n = ਸੁਰੱਖਿਅਤ_read(ifd, ibuf, ibs);
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ਸਿੱਧਾ
	ਜੇ (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_ਸਿੱਧਾ(ifd))
		ਜਾਓ read_again;
# endif
#ਜੇ_ਅੰਤ
	ਵਾਪਸ n;
}

ਸਥਿਰ bool write_and_stats(ਅਚਲ ਖਾਲੀ *buf, ਮਾਪ_t len, ਮਾਪ_t obs,
	ਅਚਲ ਅੱਖਰ *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = ਭਰਿਆ_write(ofd, buf, len);
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ਸਿੱਧਾ
	ਜੇ (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_ਸਿੱਧਾ(ofd))
		ਜਾਓ write_again;
# endif
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_DD_ਤੀਜਾ_ਸਥਿਤੀ_ਸਤਰ
	ਜੇ (n > 0)
		G.ਕੁੱਲ_ਬਾਈਟਾਂ += n;
#ਜੇ_ਅੰਤ
	ਜੇ ((ਮਾਪ_t)n == obs) {
		G.ਬਾਹਰਲਾ_ਭਰਿਆ++;
		ਵਾਪਸ 0;
	}
	ਜੇ ((ਮਾਪ_t)n == len) {
		G.ਬਾਹਰਲਾ_part++;
		ਵਾਪਸ 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_ਸੁਨੇਹਾ("error writing '%s'", filename);
	ਵਾਪਸ 1;
}

#ਜੇ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ਨਹੀਂਤਾਂ
# define XATOU_SFX xatoul_sfx
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
ਸਥਿਰ ਪੂਰਨ parse_comma_flags(ਅੱਖਰ *val, ਅਚਲ ਅੱਖਰ *ਸ਼ਬਦ, ਅਚਲ ਅੱਖਰ *error_ਵਿੱਚ)
{
	ਪੂਰਨ flags = 0;
	ਜਦੋਂ (1) {
		ਪੂਰਨ n;
		ਅੱਖਰ *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		ਜੇ (arg)
			*arg = '\0';
		n = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(ਸ਼ਬਦ, val);
		ਜੇ (n < 0)
			bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_invalid_arg_ਮੰਜ਼ਿਲ, val, error_ਵਿੱਚ);
		flags |= (1 << n);
		ਜੇ (!arg) /* no ',' left, so this was the last specifier */
			ਰੋਕੋ;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	ਵਾਪਸ flags;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ *alloc_buf(ਮਾਪ_t ਮਾਪ)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	ਜੇ (ਮਾਪ >= bb_getpagesize())
		ਵਾਪਸ xmmap_anon(ਮਾਪ);
	ਵਾਪਸ xmalloc(ਮਾਪ);
}

ਪੂਰਨ dd_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ dd_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_ਸਥਿਤੀ("status\0")
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#ਜੇ_ਅੰਤ
		;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ conv_ਸ਼ਬਦ[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ iflag_ਸ਼ਬਦ[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ oflag_ਸ਼ਬਦ[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_DD_ਸਥਿਤੀ
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ ਸਥਿਤੀ_ਸ਼ਬਦ[] ALIGN1 =
		"none\0""noxfer\0";
#ਜੇ_ਅੰਤ
	ਗਿਣਤੀ {
		OP_bs = 0,
		OP_ਗਿਣਤੀ,
		OP_seek,
		OP_skip,
		OP_if,
		OP_ਦਾ,
		IF_FEATURE_DD_ਸਥਿਤੀ(OP_ਸਥਿਤੀ,)
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_skip_ਬਾਈਟਾਂ,
		OP_iflag_ਗਿਣਤੀ_ਬਾਈਟਾਂ,
		OP_iflag_fullblock,
		OP_iflag_ਸਿੱਧਾ,
		OP_oflag_seek_ਬਾਈਟਾਂ,
		OP_oflag_ਸਿੱਧਾ,
#ਜੇ_ਅੰਤ
	};
	exitcode_t exitcode = EXIT_FAILURE;
	ਪੂਰਨ i;
	ਮਾਪ_t ibs = 512;
	ਅੱਖਰ *ibuf;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
	ਮਾਪ_t obs = 512;
	ਅੱਖਰ *obuf;
#ਨਹੀਂਤਾਂ
# define obs  ibs
# define obuf ibuf
#ਜੇ_ਅੰਤ
	/* These are all zeroed at once! */
	ਬਣਤਰ {
		IF_FEATURE_DD_IBS_OBS(ਮਾਪ_t ocount;)
		ssize_t prev_read_ਮਾਪ; /* for detecting swab failure */
		off_t ਗਿਣਤੀ_2;
		off_t seek, skip;
		ਅਚਲ ਅੱਖਰ *infile, *outfile;
	} Z;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ocount  (Z.ocount )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ prev_read_ਮਾਪ (Z.prev_read_ਮਾਪ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗਿਣਤੀ_2   (Z.ਗਿਣਤੀ_2  )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ seek    (Z.seek   )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ skip    (Z.skip   )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ infile  (Z.infile )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ outfile (Z.outfile)

	memset(&Z, 0, ਆਕਾਰ(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	ਲਈ (i = 1; argv[i]; i++) {
		ਪੂਰਨ what;
		ਅੱਖਰ *val;
		ਅੱਖਰ *arg = argv[i];

#ਜੇ ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		ਜੇ (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			ਜਾਰੀ;
#ਜੇ_ਅੰਤ
		val = strchr(arg, '=');
		ਜੇ (val == ਨਿਰਾ)
			bb_show_usage();
		*val = '\0';
		what = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(keywords, arg);
		ਜੇ (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
		ਜੇ (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((ਮਾਪ_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ਜੇ (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((ਮਾਪ_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ਜੇ (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_ਸ਼ਬਦ, "conv");
			/*continue;*/
		}
		ਜੇ (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_ਸ਼ਬਦ, "iflag") << FLAG_IFLAG_ਖਿਸਕਾਓ;
			/*continue;*/
		}
		ਜੇ (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_ਸ਼ਬਦ, "oflag") << FLAG_OFLAG_ਖਿਸਕਾਓ;
			/*continue;*/
		}
#ਜੇ_ਅੰਤ
		ਜੇ (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((ਮਾਪ_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		ਜੇ (what == OP_ਗਿਣਤੀ) {
			G.flags |= FLAG_ਗਿਣਤੀ;
			ਗਿਣਤੀ_2 = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ਜੇ (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ਜੇ (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ਜੇ (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		ਜੇ (what == OP_ਦਾ) {
			outfile = val;
			/*continue;*/
		}
#ਜੇ ENABLE_FEATURE_DD_ਸਥਿਤੀ
		ਜੇ (what == OP_ਸਥਿਤੀ) {
			ਪੂਰਨ n;
			n = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(ਸਥਿਤੀ_ਸ਼ਬਦ, val);
			ਜੇ (n < 0)
				bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_invalid_arg_ਮੰਜ਼ਿਲ, val, "status");
			G.flags |= FLAG_ਸਥਿਤੀ_NONE << n;
			/*continue;*/
		}
#ਜੇ_ਅੰਤ
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
	ਜੇ (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_ਖਾਲੀ_ਥਾਵਾਂ_ਨਕਾਬ(SIGUSR1, dd_output_ਸਥਿਤੀ);
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_DD_ਤੀਜਾ_ਸਥਿਤੀ_ਸਤਰ
	G.begin_ਸਮਾਂ_us = monotonic_us();
#ਜੇ_ਅੰਤ

	ਜੇ (infile) {
		ਪੂਰਨ iflag = O_RDONLY;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
		ਜੇ (G.flags & FLAG_IDIRECT) {
# ifdef O_ਸਿੱਧਾ
			iflag |= O_ਸਿੱਧਾ;
# ਨਹੀਂਤਾਂ
			bb_error_ਸੁਨੇਹਾ_and_die("O_DIRECT not supported on this platform");
# endif
		}
#ਜੇ_ਅੰਤ
		xmove_fd(xopen(infile, iflag), ifd);
	} ਨਹੀਂਤਾਂ {
		infile = bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼;
	}
	ਜੇ (outfile) {
		ਪੂਰਨ oflag = O_WRONLY | O_CREAT;

		ਜੇ (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		ਜੇ (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
		ਜੇ (G.flags & FLAG_ODIRECT) {
# ifdef O_ਸਿੱਧਾ
			oflag |= O_ਸਿੱਧਾ;
# ਨਹੀਂਤਾਂ
			bb_error_ਸੁਨੇਹਾ_and_die("O_DIRECT not supported on this platform");
# endif
		}
#ਜੇ_ਅੰਤ
		xmove_fd(xopen(outfile, oflag), ofd);

		ਜੇ (seek && !(G.flags & FLAG_NOTRUNC)) {
			ਮਾਪ_t blocksz = (G.flags & FLAG_SEEK_ਬਾਈਟਾਂ) ? 1 : obs;
			ਜੇ (ftruncate(ofd, seek * blocksz) < 0) {
				ਬਣਤਰ stat st;

				ਜੇ (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_ਬਹੁਲਕ)
				 || S_ISDIR(st.st_ਬਹੁਲਕ)
				) {
					ਜਾਓ die_outfile;
				}
			}
		}
	} ਨਹੀਂਤਾਂ {
		outfile = bb_ਸੁਨੇਹਾ_standard_output;
	}
	ਜੇ (skip) {
		ਮਾਪ_t blocksz = (G.flags & FLAG_SKIP_ਬਾਈਟਾਂ) ? 1 : ibs;
		ਜੇ (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			ਕਰੋ {
				ssize_t n = dd_read(ibuf, blocksz);
				ਜੇ (n < 0)
					ਜਾਓ die_infile;
				ਜੇ (n == 0)
					ਰੋਕੋ;
			} ਜਦੋਂ (--skip != 0);
		}
	}
	ਜੇ (seek) {
		ਮਾਪ_t blocksz = (G.flags & FLAG_SEEK_ਬਾਈਟਾਂ) ? 1 : obs;
		ਜੇ (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			ਜਾਓ die_outfile;
	}

	ਜਦੋਂ (1) {
		ssize_t n = ibs;

		ਜੇ (G.flags & FLAG_ਗਿਣਤੀ) {
			ਜੇ (ਗਿਣਤੀ_2 == 0)
				ਰੋਕੋ;
			ਜੇ ((G.flags & FLAG_ਗਿਣਤੀ_ਬਾਈਟਾਂ) && ਗਿਣਤੀ_2 < ibs)
				n = ਗਿਣਤੀ_2;
		}

		n = dd_read(ibuf, n);
		ਜੇ (n == 0)
			ਰੋਕੋ;
		ਜੇ (n < 0) {
			/* "Bad block" */
			ਜੇ (!(G.flags & FLAG_NOERROR))
				ਜਾਓ die_infile;
			bb_simple_perror_ਸੁਨੇਹਾ(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		ਜੇ (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			ਜੇ (prev_read_ਮਾਪ & 1)
				bb_error_ਸੁਨੇਹਾ_and_die("can't swab %lu byte buffer",
						(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)prev_read_ਮਾਪ);
			prev_read_ਮਾਪ = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (ਖਾਲੀ*) ibuf;
			n2 = (n >> 1);
			ਜਦੋਂ (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		ਗਿਣਤੀ_2 -= (G.flags & FLAG_ਗਿਣਤੀ_ਬਾਈਟਾਂ) ? n : 1;
		ਜੇ ((ਮਾਪ_t)n == ibs)
			G.ਵਿੱਚ_ਭਰਿਆ++;
		ਨਹੀਂਤਾਂ {
			G.ਵਿੱਚ_part++;
			ਜੇ (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
		ਜੇ (G.flags & FLAG_TWOBUFS) {
			ਅੱਖਰ *tmp = ibuf;
			ਜਦੋਂ (n) {
				ਮਾਪ_t d = obs - ocount;
				ਜੇ (d > (ਮਾਪ_t)n)
					d = n;
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				ਜੇ (ocount == obs) {
					ਜੇ (write_and_stats(obuf, obs, obs, outfile))
						ਜਾਓ ਬਾਹਰਲਾ_ਸਥਿਤੀ;
					ocount = 0;
				}
			}
		} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		{
			ਜੇ (write_and_stats(ibuf, n, obs, outfile))
				ਜਾਓ ਬਾਹਰਲਾ_ਸਥਿਤੀ;
		}
	}

	ਜੇ (G.flags & FLAG_FSYNC) {
		ਜੇ (fsync(ofd) < 0)
			ਜਾਓ die_outfile;
	}

#ਜੇ ENABLE_FEATURE_DD_IBS_OBS
	ਜੇ (ocount != 0) {
		ਜੇ (write_and_stats(obuf, ocount, obs, outfile))
			ਜਾਓ ਬਾਹਰਲਾ_ਸਥਿਤੀ;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_ਸੁਨੇਹਾ_and_die(infile);
	}

	ਜੇ (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_ਸੁਨੇਹਾ_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 ਬਾਹਰਲਾ_ਸਥਿਤੀ:
	ਜੇ (!ENABLE_FEATURE_DD_ਸਥਿਤੀ || !(G.flags & FLAG_ਸਥਿਤੀ_NONE))
		dd_output_ਸਥਿਤੀ(0);

#ਜੇ 0 /* can't just free(), they can be mmap()ed */
	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ) {
		ਮੁਕਤ(obuf);
		ਜੇ (G.flags & FLAG_TWOBUFS)
			ਮੁਕਤ(ibuf);
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ exitcode;
}
