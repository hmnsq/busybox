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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


ಎಣಿಕೆ {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

ರಚನೆ globals {
	off_t ಹೊರಹೋಗು_ತುಂಬಿದ, ಹೊರಹೋಗು_part, ಒಳ_ತುಂಬಿದ, ಒಳ_part;
#ಆದರೆ ENABLE_FEATURE_DD_ಮೂರನೇ_ಸ್ಥಿತಿ_ಸಾಲು
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಒಟ್ಟು_ಬೈಟುಗಳು;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ begin_ಸಮಯ_us;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ flags;
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	memset(&G, 0, ಗಾತ್ರ(G)); \
} ಆಗ (0)

ಎಣಿಕೆ {
	/* Must be in the same order as OP_conv_XXX! */
	/* (see "flags |= (1 << what)" below) */
	FLAG_NOTRUNC = (1 << 0) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SYNC    = (1 << 1) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_NOERROR = (1 << 2) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FSYNC   = (1 << 3) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_SWAB    = (1 << 4) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of conv flags */
	/* start of input flags */
	FLAG_IFLAG_ಸರಿಸು   = 5,
	FLAG_SKIP_ಬೈಟುಗಳು    = (1 << 5) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ಎಣಿಕೆ_ಬೈಟುಗಳು   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_FULLBLOCK     = (1 << 7) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_IDIRECT       = (1 << 8) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of input flags */
	/* start of output flags */
	FLAG_OFLAG_ಸರಿಸು   = 9,
	FLAG_SEEK_ಬೈಟುಗಳು    = (1 << 9) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_APPEND        = (1 << 10) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ODIRECT       = (1 << 11) * ENABLE_FEATURE_DD_IBS_OBS,
	/* end of output flags */
	FLAG_TWOBUFS       = (1 << 12) * ENABLE_FEATURE_DD_IBS_OBS,
	FLAG_ಎಣಿಕೆ         = 1 << 13,
	FLAG_ಸ್ಥಿತಿ_NONE   = 1 << 14,
	FLAG_ಸ್ಥಿತಿ_NOXFER = 1 << 15,
};

ಸ್ಥಿರ ಶೂನ್ಯ dd_output_ಸ್ಥಿತಿ(ಪೂರ್ಣಾಂಕ ಬಳಸದ_PARAM cur_signal)
{
#ಆದರೆ ENABLE_FEATURE_DD_ಮೂರನೇ_ಸ್ಥಿತಿ_ಸಾಲು
	ದ್ವಿಗುಣ seconds;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಬೈಟುಗಳು_sec;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ now_us = monotonic_us(); /* before fprintf */
#ಆದರೆ_ಅಂತ್ಯ

	/* Deliberately using %u, not %d */
	fprintf(stderr, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.ಒಳ_ತುಂಬಿದ, G.ಒಳ_part,
			G.ಹೊರಹೋಗು_ತುಂಬಿದ, G.ಹೊರಹೋಗು_part);

#ಆದರೆ ENABLE_FEATURE_DD_ಮೂರನೇ_ಸ್ಥಿತಿ_ಸಾಲು
# ಆದರೆ ENABLE_FEATURE_DD_ಸ್ಥಿತಿ
	ಆದರೆ (G.flags & FLAG_ಸ್ಥಿತಿ_NOXFER) /* status=noxfer active? */
		ಹಿಂತಿರುಗಿ;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	fprintf(stderr, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.ಒಟ್ಟು_ಬೈಟುಗಳು,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.ಒಟ್ಟು_ಬೈಟುಗಳು, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_ಸಮಯ_us) / 1000000.0;
	ಬೈಟುಗಳು_sec = G.ಒಟ್ಟು_ಬೈಟುಗಳು / seconds;
	fprintf(stderr, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(ಬೈಟುಗಳು_sec, 1, 0)
	);
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ನೇರ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ clear_O_ನೇರ(ಪೂರ್ಣಾಂಕ fd)
{
	ಆದರೆ (errno == EINVAL) {
		ಪೂರ್ಣಾಂಕ fl = fcntl(fd, F_GETFL);
		ಆದರೆ (fl & O_ನೇರ) {
			fcntl(fd, F_SETFL, fl & ~O_ನೇರ);
			ಹಿಂತಿರುಗಿ 1;
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}
# endif
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ssize_t dd_read(ಶೂನ್ಯ *ibuf, ಗಾತ್ರ_t ibs)
{
	ssize_t n;

#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	ಆದರೆ (G.flags & FLAG_FULLBLOCK)
		n = ತುಂಬಿದ_read(ifd, ibuf, ibs);
	ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		n = ಸುರಕ್ಷಿತ_read(ifd, ibuf, ibs);
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ನೇರ
	ಆದರೆ (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_ನೇರ(ifd))
		ಹೋಗು read_again;
# endif
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ bool write_and_stats(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, ಗಾತ್ರ_t len, ಗಾತ್ರ_t obs,
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = ತುಂಬಿದ_write(ofd, buf, len);
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_ನೇರ
	ಆದರೆ (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_ನೇರ(ofd))
		ಹೋಗು write_again;
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_DD_ಮೂರನೇ_ಸ್ಥಿತಿ_ಸಾಲು
	ಆದರೆ (n > 0)
		G.ಒಟ್ಟು_ಬೈಟುಗಳು += n;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ ((ಗಾತ್ರ_t)n == obs) {
		G.ಹೊರಹೋಗು_ತುಂಬಿದ++;
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ ((ಗಾತ್ರ_t)n == len) {
		G.ಹೊರಹೋಗು_part++;
		ಹಿಂತಿರುಗಿ 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_ಸಂದೇಶ("error writing '%s'", filename);
	ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#ಇಲ್ಲದಿದ್ದರೆ
# define XATOU_SFX xatoul_sfx
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_comma_flags(ಅಕ್ಷರ *val, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪದಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *error_ಒಳ)
{
	ಪೂರ್ಣಾಂಕ flags = 0;
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ n;
		ಅಕ್ಷರ *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = strchr(val, ',');
		ಆದರೆ (arg)
			*arg = '\0';
		n = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(ಪದಗಳು, val);
		ಆದರೆ (n < 0)
			bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_invalid_arg_ಗಮ್ಯ, val, error_ಒಳ);
		flags |= (1 << n);
		ಆದರೆ (!arg) /* no ',' left, so this was the last specifier */
			ನಿಲ್ಲಿಸು;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	ಹಿಂತಿರುಗಿ flags;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ *alloc_buf(ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	ಆದರೆ (ಗಾತ್ರ_2 >= bb_getpagesize())
		ಹಿಂತಿರುಗಿ xmmap_anon(ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ xmalloc(ಗಾತ್ರ_2);
}

ಪೂರ್ಣಾಂಕ dd_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ dd_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_ಸ್ಥಿತಿ("status\0")
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#ಆದರೆ_ಅಂತ್ಯ
		;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ conv_ಪದಗಳು[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ iflag_ಪದಗಳು[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ oflag_ಪದಗಳು[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_DD_ಸ್ಥಿತಿ
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ಸ್ಥಿತಿ_ಪದಗಳು[] ALIGN1 =
		"none\0""noxfer\0";
#ಆದರೆ_ಅಂತ್ಯ
	ಎಣಿಕೆ {
		OP_bs = 0,
		OP_ಎಣಿಕೆ,
		OP_seek,
		OP_skip,
		OP_if,
		OP_ದ,
		IF_FEATURE_DD_ಸ್ಥಿತಿ(OP_ಸ್ಥಿತಿ,)
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_skip_ಬೈಟುಗಳು,
		OP_iflag_ಎಣಿಕೆ_ಬೈಟುಗಳು,
		OP_iflag_fullblock,
		OP_iflag_ನೇರ,
		OP_oflag_seek_ಬೈಟುಗಳು,
		OP_oflag_ನೇರ,
#ಆದರೆ_ಅಂತ್ಯ
	};
	exitcode_t exitcode = EXIT_FAILURE;
	ಪೂರ್ಣಾಂಕ i;
	ಗಾತ್ರ_t ibs = 512;
	ಅಕ್ಷರ *ibuf;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
	ಗಾತ್ರ_t obs = 512;
	ಅಕ್ಷರ *obuf;
#ಇಲ್ಲದಿದ್ದರೆ
# define obs  ibs
# define obuf ibuf
#ಆದರೆ_ಅಂತ್ಯ
	/* These are all zeroed at once! */
	ರಚನೆ {
		IF_FEATURE_DD_IBS_OBS(ಗಾತ್ರ_t ocount;)
		ssize_t prev_read_ಗಾತ್ರ; /* for detecting swab failure */
		off_t ಎಣಿಕೆ_2;
		off_t seek, skip;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *infile, *outfile;
	} Z;
#ವ್ಯಾಖ್ಯಾನಿಸು ocount  (Z.ocount )
#ವ್ಯಾಖ್ಯಾನಿಸು prev_read_ಗಾತ್ರ (Z.prev_read_ಗಾತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಣಿಕೆ_2   (Z.ಎಣಿಕೆ_2  )
#ವ್ಯಾಖ್ಯಾನಿಸು seek    (Z.seek   )
#ವ್ಯಾಖ್ಯಾನಿಸು skip    (Z.skip   )
#ವ್ಯಾಖ್ಯಾನಿಸು infile  (Z.infile )
#ವ್ಯಾಖ್ಯಾನಿಸು outfile (Z.outfile)

	memset(&Z, 0, ಗಾತ್ರ(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	ಗಾಗಿ (i = 1; argv[i]; i++) {
		ಪೂರ್ಣಾಂಕ what;
		ಅಕ್ಷರ *val;
		ಅಕ್ಷರ *arg = argv[i];

#ಆದರೆ ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		ಆದರೆ (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
		val = strchr(arg, '=');
		ಆದರೆ (val == ಖಾಲಿ)
			bb_show_usage();
		*val = '\0';
		what = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(keywords, arg);
		ಆದರೆ (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
		ಆದರೆ (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((ಗಾತ್ರ_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ಆದರೆ (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((ಗಾತ್ರ_t)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		ಆದರೆ (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_ಪದಗಳು, "conv");
			/*continue;*/
		}
		ಆದರೆ (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_ಪದಗಳು, "iflag") << FLAG_IFLAG_ಸರಿಸು;
			/*continue;*/
		}
		ಆದರೆ (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_ಪದಗಳು, "oflag") << FLAG_OFLAG_ಸರಿಸು;
			/*continue;*/
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((ಗಾತ್ರ_t)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		ಆದರೆ (what == OP_ಎಣಿಕೆ) {
			G.flags |= FLAG_ಎಣಿಕೆ;
			ಎಣಿಕೆ_2 = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ಆದರೆ (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ಆದರೆ (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		ಆದರೆ (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		ಆದರೆ (what == OP_ದ) {
			outfile = val;
			/*continue;*/
		}
#ಆದರೆ ENABLE_FEATURE_DD_ಸ್ಥಿತಿ
		ಆದರೆ (what == OP_ಸ್ಥಿತಿ) {
			ಪೂರ್ಣಾಂಕ n;
			n = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(ಸ್ಥಿತಿ_ಪದಗಳು, val);
			ಆದರೆ (n < 0)
				bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_invalid_arg_ಗಮ್ಯ, val, "status");
			G.flags |= FLAG_ಸ್ಥಿತಿ_NONE << n;
			/*continue;*/
		}
#ಆದರೆ_ಅಂತ್ಯ
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
	ಆದರೆ (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_ಖಾಲಿಗಳು_ಮುಖವಾಡ(SIGUSR1, dd_output_ಸ್ಥಿತಿ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_DD_ಮೂರನೇ_ಸ್ಥಿತಿ_ಸಾಲು
	G.begin_ಸಮಯ_us = monotonic_us();
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (infile) {
		ಪೂರ್ಣಾಂಕ iflag = O_RDONLY;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
		ಆದರೆ (G.flags & FLAG_IDIRECT) {
# ifdef O_ನೇರ
			iflag |= O_ನೇರ;
# ಇಲ್ಲದಿದ್ದರೆ
			bb_error_ಸಂದೇಶ_and_die("O_DIRECT not supported on this platform");
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ
		xmove_fd(xopen(infile, iflag), ifd);
	} ಇಲ್ಲದಿದ್ದರೆ {
		infile = bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು;
	}
	ಆದರೆ (outfile) {
		ಪೂರ್ಣಾಂಕ oflag = O_WRONLY | O_CREAT;

		ಆದರೆ (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		ಆದರೆ (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
		ಆದರೆ (G.flags & FLAG_ODIRECT) {
# ifdef O_ನೇರ
			oflag |= O_ನೇರ;
# ಇಲ್ಲದಿದ್ದರೆ
			bb_error_ಸಂದೇಶ_and_die("O_DIRECT not supported on this platform");
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ
		xmove_fd(xopen(outfile, oflag), ofd);

		ಆದರೆ (seek && !(G.flags & FLAG_NOTRUNC)) {
			ಗಾತ್ರ_t blocksz = (G.flags & FLAG_SEEK_ಬೈಟುಗಳು) ? 1 : obs;
			ಆದರೆ (ftruncate(ofd, seek * blocksz) < 0) {
				ರಚನೆ stat st;

				ಆದರೆ (fstat(ofd, &st) < 0
				 || S_ISREG(st.st_ಬಹುಲಕ)
				 || S_ISDIR(st.st_ಬಹುಲಕ)
				) {
					ಹೋಗು die_outfile;
				}
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		outfile = bb_ಸಂದೇಶ_standard_output;
	}
	ಆದರೆ (skip) {
		ಗಾತ್ರ_t blocksz = (G.flags & FLAG_SKIP_ಬೈಟುಗಳು) ? 1 : ibs;
		ಆದರೆ (lseek(ifd, skip * blocksz, SEEK_CUR) < 0) {
			ಮಾಡು {
				ssize_t n = dd_read(ibuf, blocksz);
				ಆದರೆ (n < 0)
					ಹೋಗು die_infile;
				ಆದರೆ (n == 0)
					ನಿಲ್ಲಿಸು;
			} ಆಗ (--skip != 0);
		}
	}
	ಆದರೆ (seek) {
		ಗಾತ್ರ_t blocksz = (G.flags & FLAG_SEEK_ಬೈಟುಗಳು) ? 1 : obs;
		ಆದರೆ (lseek(ofd, seek * blocksz, SEEK_CUR) < 0)
			ಹೋಗು die_outfile;
	}

	ಆಗ (1) {
		ssize_t n = ibs;

		ಆದರೆ (G.flags & FLAG_ಎಣಿಕೆ) {
			ಆದರೆ (ಎಣಿಕೆ_2 == 0)
				ನಿಲ್ಲಿಸು;
			ಆದರೆ ((G.flags & FLAG_ಎಣಿಕೆ_ಬೈಟುಗಳು) && ಎಣಿಕೆ_2 < ibs)
				n = ಎಣಿಕೆ_2;
		}

		n = dd_read(ibuf, n);
		ಆದರೆ (n == 0)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (n < 0) {
			/* "Bad block" */
			ಆದರೆ (!(G.flags & FLAG_NOERROR))
				ಹೋಗು die_infile;
			bb_simple_perror_ಸಂದೇಶ(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, SEEK_CUR);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		ಆದರೆ (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			ಆದರೆ (prev_read_ಗಾತ್ರ & 1)
				bb_error_ಸಂದೇಶ_and_die("can't swab %lu byte buffer",
						(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)prev_read_ಗಾತ್ರ);
			prev_read_ಗಾತ್ರ = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (ಶೂನ್ಯ*) ibuf;
			n2 = (n >> 1);
			ಆಗ (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		ಎಣಿಕೆ_2 -= (G.flags & FLAG_ಎಣಿಕೆ_ಬೈಟುಗಳು) ? n : 1;
		ಆದರೆ ((ಗಾತ್ರ_t)n == ibs)
			G.ಒಳ_ತುಂಬಿದ++;
		ಇಲ್ಲದಿದ್ದರೆ {
			G.ಒಳ_part++;
			ಆದರೆ (G.flags & FLAG_SYNC) {
				memset(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
		ಆದರೆ (G.flags & FLAG_TWOBUFS) {
			ಅಕ್ಷರ *tmp = ibuf;
			ಆಗ (n) {
				ಗಾತ್ರ_t d = obs - ocount;
				ಆದರೆ (d > (ಗಾತ್ರ_t)n)
					d = n;
				ಸ್ಮರಣೆ_ನಕಲು(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				ಆದರೆ (ocount == obs) {
					ಆದರೆ (write_and_stats(obuf, obs, obs, outfile))
						ಹೋಗು ಹೊರಹೋಗು_ಸ್ಥಿತಿ;
					ocount = 0;
				}
			}
		} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		{
			ಆದರೆ (write_and_stats(ibuf, n, obs, outfile))
				ಹೋಗು ಹೊರಹೋಗು_ಸ್ಥಿತಿ;
		}
	}

	ಆದರೆ (G.flags & FLAG_FSYNC) {
		ಆದರೆ (fsync(ofd) < 0)
			ಹೋಗು die_outfile;
	}

#ಆದರೆ ENABLE_FEATURE_DD_IBS_OBS
	ಆದರೆ (ocount != 0) {
		ಆದರೆ (write_and_stats(obuf, ocount, obs, outfile))
			ಹೋಗು ಹೊರಹೋಗು_ಸ್ಥಿತಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (close(ifd) < 0) {
 die_infile:
		bb_simple_perror_ಸಂದೇಶ_and_die(infile);
	}

	ಆದರೆ (close(ofd) < 0) {
 die_outfile:
		bb_simple_perror_ಸಂದೇಶ_and_die(outfile);
	}

	exitcode = EXIT_SUCCESS;
 ಹೊರಹೋಗು_ಸ್ಥಿತಿ:
	ಆದರೆ (!ENABLE_FEATURE_DD_ಸ್ಥಿತಿ || !(G.flags & FLAG_ಸ್ಥಿತಿ_NONE))
		dd_output_ಸ್ಥಿತಿ(0);

#ಆದರೆ 0 /* can't just free(), they can be mmap()ed */
	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ) {
		ಬಿಡುಗಡೆ(obuf);
		ಆದರೆ (G.flags & FLAG_TWOBUFS)
			ಬಿಡುಗಡೆ(ibuf);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ exitcode;
}
