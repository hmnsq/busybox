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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


గణన {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

నిర్మాణం globals {
	off_t బహిర్గతం_నిండిన, బహిర్గతం_part, లోపల_నిండిన, లోపల_part;
#ఒకవేళ ENABLE_FEATURE_DD_మూడవ_స్థితి_పంక్తి
	గుర్తులేని దీర్ఘ దీర్ఘ మొత్తం_బైట్లు;
	గుర్తులేని దీర్ఘ దీర్ఘ begin_సమయం_us;
#అయితే_ముగింపు
	పూర్ణాంకం flags;
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, పరిమాణం(G)); \
} అయితే (0)

గణన {
	/* Must be in the same order as OP_conv_XXX! */
	/* (see "flags |= (1 << what)" below) */
	FLAG_NOTRUNC = (1 << 0) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SYNC    = (1 << 1) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_NOERROR = (1 << 2) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FSYNC   = (1 << 3) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SWAB    = (1 << 4) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of conv flags */
	/* start of input flags */
	FLAG_IFLAG_జరుపు   = 5,
	FLAG_SKIP_బైట్లు    = (1 << 5) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_లెక్క_బైట్లు   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FULLBLOCK     = (1 << 7) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_IDIRECT       = (1 << 8) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of input flags */
	/* start of output flags */
	FLAG_OFLAG_జరుపు   = 9,
	FLAG_SEEK_బైట్లు    = (1 << 9) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_APPEND        = (1 << 10) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ODIRECT       = (1 << 11) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of output flags */
	FLAG_TWOBUFS       = (1 << 12) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_లెక్క         = 1 << 13,
	FLAG_స్థితి_NONE   = 1 << 14,
	FLAG_స్థితి_NOXFER = 1 << 15,
};

స్థిర శూన్య dd_output_స్థితి(పూర్ణాంకం వాడని_PARAM cur_signal)
{
#ఒకవేళ ENABLE_FEATURE_DD_మూడవ_స్థితి_పంక్తి
	ద్విగుణ seconds;
	గుర్తులేని దీర్ఘ దీర్ఘ బైట్లు_sec;
	గుర్తులేని దీర్ఘ దీర్ఘ now_us = monotonic_us(); /* before fprintf */
#అయితే_ముగింపు

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.లోపల_నిండిన, G.లోపల_part,
			G.బహిర్గతం_నిండిన, G.బహిర్గతం_part);

#ఒకవేళ ENABLE_FEATURE_DD_మూడవ_స్థితి_పంక్తి
# ఒకవేళ ENABLE_FEATURE_DD_స్థితి
	ఒకవేళ (G.flags & FLAG_స్థితి_NOXFER) /* status=noxfer active? */
		తిరిగి;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.మొత్తం_బైట్లు,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.మొత్తం_బైట్లు, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_సమయం_us) / 1000000.0;
	బైట్లు_sec = G.మొత్తం_బైట్లు / seconds;
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(బైట్లు_sec, 1, 0)
	);
#అయితే_ముగింపు
}

#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ప్రత్యక్ష
స్థిర పూర్ణాంకం clear_O_ప్రత్యక్ష(పూర్ణాంకం fd)
{
	ఒకవేళ (errno == EINVAL) {
		పూర్ణాంకం fl = fcntl(fd, F_GETFL);
		ఒకవేళ (fl & O_ప్రత్యక్ష) {
			fcntl(fd, F_SETFL, fl & ~O_ప్రత్యక్ష);
			తిరిగి 1;
		}
	}
	తిరిగి 0;
}
# endif
#అయితే_ముగింపు

స్థిర ssize_t dd_read(శూన్య *ibuf, కొలత_t ibs)
{
	ssize_t n;

#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	ఒకవేళ (G.flags & FLAG_FULLBLOCK)
		n = నిండిన_read(ifd, ibuf, ibs);
	లేకపోతే
#అయితే_ముగింపు
		n = సురక్షిత_read(ifd, ibuf, ibs);
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ప్రత్యక్ష
	ఒకవేళ (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_ప్రత్యక్ష(ifd))
		వెళ్ళు read_again;
# endif
#అయితే_ముగింపు
	తిరిగి n;
}

స్థిర bool write_and_stats(స్థిరరాశి శూన్య *buf, కొలత_t len, కొలత_t obs,
	స్థిరరాశి అక్షరం *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = నిండిన_write(ofd, buf, len);
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ప్రత్యక్ష
	ఒకవేళ (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_ప్రత్యక్ష(ofd))
		వెళ్ళు write_again;
# endif
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_DD_మూడవ_స్థితి_పంక్తి
	ఒకవేళ (n > 0)
		G.మొత్తం_బైట్లు += n;
#అయితే_ముగింపు
	ఒకవేళ ((కొలత_t)n == obs) {
		G.బహిర్గతం_నిండిన++;
		తిరిగి 0;
	}
	ఒకవేళ ((కొలత_t)n == len) {
		G.బహిర్గతం_part++;
		తిరిగి 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_సందేశం("error writing '%s'", filename);
	తిరిగి 1;
}

#ఒకవేళ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#లేకపోతే
# define XATOU_SFX xatoul_sfx
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
స్థిర పూర్ణాంకం parse_comma_flags(అక్షరం *val, స్థిరరాశి అక్షరం *పదాలు, స్థిరరాశి అక్షరం *error_లోపల)
{
	పూర్ణాంకం flags = 0;
	అయితే (1) {
		పూర్ణాంకం n;
		అక్షరం *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		ఒకవేళ (arg)
			*arg = '\0';
		n = సూచీ_లోపల_అక్షరశ్రేణులు(పదాలు, val);
		ఒకవేళ (n < 0)
			bb_error_సందేశం_and_die(bb_సందేశం_invalid_arg_గమ్యం, val, error_లోపల);
		flags |= (1 << n);
		ఒకవేళ (!arg) /* no ',' left, so this was the last specifier */
			ఆపు;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	తిరిగి flags;
}
#అయితే_ముగింపు

స్థిర శూన్య *alloc_buf(కొలత_t కొలత)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	ఒకవేళ (కొలత >= bb_getpagesize())
		తిరిగి xmmap_anon(కొలత);
	తిరిగి xmalloc(కొలత);
}

పూర్ణాంకం dd_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం dd_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	స్థిర స్థిరరాశి అక్షరం keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_స్థితి("status\0")
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#అయితే_ముగింపు
		;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
	స్థిర స్థిరరాశి అక్షరం conv_పదాలు[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	స్థిర స్థిరరాశి అక్షరం iflag_పదాలు[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	స్థిర స్థిరరాశి అక్షరం oflag_పదాలు[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_DD_స్థితి
	స్థిర స్థిరరాశి అక్షరం స్థితి_పదాలు[] ALIGN1 =
		"none\0""noxfer\0";
#అయితే_ముగింపు
	గణన {
		OP_bs = 0,
		OP_లెక్క,
		OP_seek,
		OP_skip,
		OP_if,
		OP_యొక్క,
		IF_FEATURE_DD_స్థితి(OP_స్థితి,)
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_skip_బైట్లు,
		OP_iflag_లెక్క_బైట్లు,
		OP_iflag_fullblock,
		OP_iflag_ప్రత్యక్ష,
		OP_oflag_seek_బైట్లు,
		OP_oflag_ప్రత్యక్ష,
#అయితే_ముగింపు
	};
	exitcode_t exitcode = EXIT_FAILURE;
	పూర్ణాంకం i;
	కొలత_t ibs = 512;
	అక్షరం *ibuf;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
	కొలత_t obs = 512;
	అక్షరం *obuf;
#లేకపోతే
# define obs  ibs
# define obuf ibuf
#అయితే_ముగింపు
	/* These are all zeroed at once! */
	నిర్మాణం {
		IF_FEATURE_DD_IBS_OBS(కొలత_t ocount;)
		ssize_t prev_read_కొలత; /* for detecting swab failure */
		off_t లెక్క;
		off_t seek, skip;
		స్థిరరాశి అక్షరం *infile, *outfile;
	} Z;
#నిర్వచించు ocount  (Z.ocount )
#నిర్వచించు prev_read_కొలత (Z.prev_read_కొలత)
#నిర్వచించు లెక్క   (Z.లెక్క  )
#నిర్వచించు seek    (Z.seek   )
#నిర్వచించు skip    (Z.skip   )
#నిర్వచించు infile  (Z.infile )
#నిర్వచించు outfile (Z.outfile)

	memset(&Z, 0, పరిమాణం(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	కొరకు (i = 1; argv[i]; i++) {
		పూర్ణాంకం what;
		అక్షరం *val;
		అక్షరం *arg = argv[i];

#ఒకవేళ ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		ఒకవేళ (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			కొనసాగు;
#అయితే_ముగింపు
		val = strchr(arg, '=');
		ఒకవేళ (val == ఖాళీ)
			bb_show_usage();
		*val = '\0';
		what = సూచీ_లోపల_అక్షరశ్రేణులు(keywords, arg);
		ఒకవేళ (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
		ఒకవేళ (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((కొలత_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ఒకవేళ (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((కొలత_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ఒకవేళ (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_పదాలు, "conv");
			/*continue;*/
		}
		ఒకవేళ (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_పదాలు, "iflag") << FLAG_IFLAG_జరుపు;
			/*continue;*/
		}
		ఒకవేళ (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_పదాలు, "oflag") << FLAG_OFLAG_జరుపు;
			/*continue;*/
		}
#అయితే_ముగింపు
		ఒకవేళ (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((కొలత_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		ఒకవేళ (what == OP_లెక్క) {
			G.flags |= FLAG_లెక్క;
			లెక్క = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ఒకవేళ (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ఒకవేళ (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ఒకవేళ (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		ఒకవేళ (what == OP_యొక్క) {
			outfile = val;
			/*continue;*/
		}
#ఒకవేళ ENABLE_FEATURE_DD_స్థితి
		ఒకవేళ (what == OP_స్థితి) {
			పూర్ణాంకం n;
			n = సూచీ_లోపల_అక్షరశ్రేణులు(స్థితి_పదాలు, val);
			ఒకవేళ (n < 0)
				bb_error_సందేశం_and_die(bb_సందేశం_invalid_arg_గమ్యం, val, "status");
			G.flags |= FLAG_స్థితి_NONE << n;
			/*continue;*/
		}
#అయితే_ముగింపు
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
	ఒకవేళ (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_ఖాళీలు_ముసుగు(SIGUSR1, dd_output_స్థితి);
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_DD_మూడవ_స్థితి_పంక్తి
	G.begin_సమయం_us = monotonic_us();
#అయితే_ముగింపు

	ఒకవేళ (infile) {
		పూర్ణాంకం iflag = O_RDONLY;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
		ఒకవేళ (G.flags & FLAG_IDIRECT) {
# ifdef O_ప్రత్యక్ష
			iflag |= O_ప్రత్యక్ష;
# లేకపోతే
			bb_error_సందేశం_and_die("O_DIRECT not supported on this platform");
# endif
		}
#అయితే_ముగింపు
		xmove_fd(xopen(infile, iflag), ifd);
	} లేకపోతే {
		infile = bb_సందేశం_standard_ఆదానం;
	}
	ఒకవేళ (outfile) {
		పూర్ణాంకం oflag = O_WRONLY | O_CREAT;

		ఒకవేళ (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		ఒకవేళ (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
		ఒకవేళ (G.flags & FLAG_ODIRECT) {
# ifdef O_ప్రత్యక్ష
			oflag |= O_ప్రత్యక్ష;
# లేకపోతే
			bb_error_సందేశం_and_die("O_DIRECT not supported on this platform");
# endif
		}
#అయితే_ముగింపు
		xmove_fd(xopen(outfile, oflag), ofd);

		ఒకవేళ (seek && !(G.flags & FLAG_NOTRUNC)) {
			కొలత_t blocksz = (G.flags & FLAG_SEEK_బైట్లు) ? 1 : obs;
			ఒకవేళ (ftruncate(ofd, seek * blocksz) < 0) {
				నిర్మాణం stat st;

				ఒకవేళ (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_బహుళకం)
				 || S_ISDIR(st.st_బహుళకం)
				) {
					వెళ్ళు die_outfile;
				}
			}
		}
	} లేకపోతే {
		outfile = bb_సందేశం_standard_output;
	}
	ఒకవేళ (skip) {
		కొలత_t blocksz = (G.flags & FLAG_SKIP_బైట్లు) ? 1 : ibs;
		ఒకవేళ (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			చేయి {
				ssize_t n = dd_read(ibuf, blocksz);
				ఒకవేళ (n < 0)
					వెళ్ళు die_infile;
				ఒకవేళ (n == 0)
					ఆపు;
			} అయితే (--skip != 0);
		}
	}
	ఒకవేళ (seek) {
		కొలత_t blocksz = (G.flags & FLAG_SEEK_బైట్లు) ? 1 : obs;
		ఒకవేళ (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			వెళ్ళు die_outfile;
	}

	అయితే (1) {
		ssize_t n = ibs;

		ఒకవేళ (G.flags & FLAG_లెక్క) {
			ఒకవేళ (లెక్క == 0)
				ఆపు;
			ఒకవేళ ((G.flags & FLAG_లెక్క_బైట్లు) && లెక్క < ibs)
				n = లెక్క;
		}

		n = dd_read(ibuf, n);
		ఒకవేళ (n == 0)
			ఆపు;
		ఒకవేళ (n < 0) {
			/* "Bad block" */
			ఒకవేళ (!(G.flags & FLAG_NOERROR))
				వెళ్ళు die_infile;
			bb_simple_perror_సందేశం(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		ఒకవేళ (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			ఒకవేళ (prev_read_కొలత & 1)
				bb_error_సందేశం_and_die("can't swab %lu byte buffer",
						(గుర్తులేని దీర్ఘ)prev_read_కొలత);
			prev_read_కొలత = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (శూన్య*) ibuf;
			n2 = (n >> 1);
			అయితే (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		లెక్క -= (G.flags & FLAG_లెక్క_బైట్లు) ? n : 1;
		ఒకవేళ ((కొలత_t)n == ibs)
			G.లోపల_నిండిన++;
		లేకపోతే {
			G.లోపల_part++;
			ఒకవేళ (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
		ఒకవేళ (G.flags & FLAG_TWOBUFS) {
			అక్షరం *tmp = ibuf;
			అయితే (n) {
				కొలత_t d = obs - ocount;
				ఒకవేళ (d > (కొలత_t)n)
					d = n;
				జ్ఞాపకం_నకలు(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				ఒకవేళ (ocount == obs) {
					ఒకవేళ (write_and_stats(obuf, obs, obs, outfile))
						వెళ్ళు బహిర్గతం_స్థితి;
					ocount = 0;
				}
			}
		} లేకపోతే
#అయితే_ముగింపు
		{
			ఒకవేళ (write_and_stats(ibuf, n, obs, outfile))
				వెళ్ళు బహిర్గతం_స్థితి;
		}
	}

	ఒకవేళ (G.flags & FLAG_FSYNC) {
		ఒకవేళ (fsync(ofd) < 0)
			వెళ్ళు die_outfile;
	}

#ఒకవేళ ENABLE_FEATURE_DD_IBS_OBS
	ఒకవేళ (ocount != 0) {
		ఒకవేళ (write_and_stats(obuf, ocount, obs, outfile))
			వెళ్ళు బహిర్గతం_స్థితి;
	}
#అయితే_ముగింపు
	ఒకవేళ (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_సందేశం_and_die(infile);
	}

	ఒకవేళ (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_సందేశం_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 బహిర్గతం_స్థితి:
	ఒకవేళ (!ENABLE_FEATURE_DD_స్థితి || !(G.flags & FLAG_స్థితి_NONE))
		dd_output_స్థితి(0);

#ఒకవేళ 0 /* can't just free(), they can be mmap()ed */
	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి) {
		విడుదల(obuf);
		ఒకవేళ (G.flags & FLAG_TWOBUFS)
			విడుదల(ibuf);
	}
#అయితే_ముగింపు

	తిరిగి exitcode;
}
