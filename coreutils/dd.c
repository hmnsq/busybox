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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

/* This is a NOEXEC applet. Be very careful! */


क्रमागत {
	ifd = STDIN_FILENO,
	ofd = STDOUT_FILENO,
};

काष्ठा globals {
	off_t out_full, out_part, in_full, in_part;
#अगर ENABLE_FEATURE_DD_THIRD_STATUS_पंक्ति
	अचिन्हित दीर्घ दीर्घ योग_bytes;
	अचिन्हित दीर्घ दीर्घ begin_time_us;
#पूर्ण_अगर
	पूर्णांक flags;
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { \
	setup_common_bufsiz(); \
	/* we have to zero it out because of NOEXEC */ \
	स_रखो(&G, 0, माप(G)); \
} जबतक (0)

क्रमागत {
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
	FLAG_गिनती_BYTES   = (1 << 6) * ENABLE_FEATURE_DD_IBS_OBS,
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
	FLAG_गिनती         = 1 << 13,
	FLAG_STATUS_NONE   = 1 << 14,
	FLAG_STATUS_NOXFER = 1 << 15,
};

अटल व्योम dd_output_status(पूर्णांक UNUSED_PARAM cur_signal)
{
#अगर ENABLE_FEATURE_DD_THIRD_STATUS_पंक्ति
	द्विगुन seconds;
	अचिन्हित दीर्घ दीर्घ bytes_sec;
	अचिन्हित दीर्घ दीर्घ now_us = monotonic_us(); /* before fprintf */
#पूर्ण_अगर

	/* Deliberately using %u, not %d */
	ख_लिखो(मानक_त्रुटि, "%"OFF_FMT"u+%"OFF_FMT"u records in\n"
			"%"OFF_FMT"u+%"OFF_FMT"u records out\n",
			G.in_full, G.in_part,
			G.out_full, G.out_part);

#अगर ENABLE_FEATURE_DD_THIRD_STATUS_पंक्ति
# अगर ENABLE_FEATURE_DD_STATUS
	अगर (G.flags & FLAG_STATUS_NOXFER) /* status=noxfer active? */
		वापस;
	//TODO: should status=none make dd stop reacting to USR1 entirely?
	//So far we react to it (we print the stats),
	//status=none only suppresses final, non-USR1 generated status message.
# endif
	ख_लिखो(मानक_त्रुटि, /*G.total_bytes < 1024
				? "%llu bytes copied, " : */ "%llu bytes (%sB) copied, "
			, G.योग_bytes,
			/* show fractional digit, use suffixes */
			make_human_readable_str(G.योग_bytes, 1, 0)
	);
	/* Corner cases:
	 * ./busybox dd </dev/null >/dev/null
	 * ./busybox dd bs=1M count=2000 </dev/zero >/dev/null
	 * (echo DONE) | ./busybox dd >/dev/null
	 * (sleep 1; echo DONE) | ./busybox dd >/dev/null
	 */
	seconds = (now_us - G.begin_time_us) / 1000000.0;
	bytes_sec = G.योग_bytes / seconds;
	ख_लिखो(मानक_त्रुटि, "%f seconds, %sB/s\n",
			seconds,
			/* show fractional digit, use suffixes */
			make_human_readable_str(bytes_sec, 1, 0)
	);
#पूर्ण_अगर
}

#अगर ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
अटल पूर्णांक clear_O_DIRECT(पूर्णांक fd)
{
	अगर (त्रुटि_सं == EINVAL) {
		पूर्णांक fl = fcntl(fd, F_GETFL);
		अगर (fl & O_DIRECT) {
			fcntl(fd, F_SETFL, fl & ~O_DIRECT);
			वापस 1;
		}
	}
	वापस 0;
}
# endif
#पूर्ण_अगर

अटल ssize_t dd_read(व्योम *ibuf, माप_प्रकार ibs)
{
	ssize_t n;

#अगर ENABLE_FEATURE_DD_IBS_OBS
 read_again:
	अगर (G.flags & FLAG_FULLBLOCK)
		n = full_read(ifd, ibuf, ibs);
	अन्यथा
#पूर्ण_अगर
		n = safe_read(ifd, ibuf, ibs);
#अगर ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	अगर (n < 0 && (G.flags & FLAG_IDIRECT) && clear_O_DIRECT(ifd))
		जाओ read_again;
# endif
#पूर्ण_अगर
	वापस n;
}

अटल bool write_and_stats(स्थिर व्योम *buf, माप_प्रकार len, माप_प्रकार obs,
	स्थिर अक्षर *filename)
{
	ssize_t n;

 IF_FEATURE_DD_IBS_OBS(write_again:)
	n = full_write(ofd, buf, len);
#अगर ENABLE_FEATURE_DD_IBS_OBS
# ifdef O_DIRECT
	अगर (n < 0 && (G.flags & FLAG_ODIRECT) && clear_O_DIRECT(ofd))
		जाओ write_again;
# endif
#पूर्ण_अगर

#अगर ENABLE_FEATURE_DD_THIRD_STATUS_पंक्ति
	अगर (n > 0)
		G.योग_bytes += n;
#पूर्ण_अगर
	अगर ((माप_प्रकार)n == obs) {
		G.out_full++;
		वापस 0;
	}
	अगर ((माप_प्रकार)n == len) {
		G.out_part++;
		वापस 0;
	}
	/* n is < len (and possibly is -1).
	 * Even if n >= 0, errno is usually set correctly.
	 * For example, if writing to block device and getting ENOSPC,
	 * full_write() first sees a short write, then tries to write
	 * the remainder and gets errno set to ENOSPC.
	 * It returns n > 0 (the amount which it did write).
	 */
	bb_perror_संदेश("error writing '%s'", filename);
	वापस 1;
}

#अगर ENABLE_LFS
# define XATOU_SFX xatoull_sfx
#अन्यथा
# define XATOU_SFX xatoul_sfx
#पूर्ण_अगर

#अगर ENABLE_FEATURE_DD_IBS_OBS
अटल पूर्णांक parse_comma_flags(अक्षर *val, स्थिर अक्षर *शब्दगण, स्थिर अक्षर *error_in)
{
	पूर्णांक flags = 0;
	जबतक (1) {
		पूर्णांक n;
		अक्षर *arg;
		/* find ',', replace them with NUL so we can use val for
		 * index_in_strings() without copying.
		 * We rely on val being non-null, else strchr would fault.
		 */
		arg = म_अक्षर(val, ',');
		अगर (arg)
			*arg = '\0';
		n = सूचकांक_in_strings(शब्दगण, val);
		अगर (n < 0)
			bb_error_संदेश_and_die(bb_संदेश_invalid_arg_to, val, error_in);
		flags |= (1 << n);
		अगर (!arg) /* no ',' left, so this was the last specifier */
			अवरोध;
		*arg = ','; /* to preserve ps listing */
		val = arg + 1; /* skip this keyword and ',' */
	}
	वापस flags;
}
#पूर्ण_अगर

अटल व्योम *alloc_buf(माप_प्रकार आकार)
{
	/* Important for "{i,o}flag=direct" - buffers must be page aligned */
	अगर (आकार >= bb_getpagesize())
		वापस xmmap_anon(आकार);
	वापस xmalloc(आकार);
}

पूर्णांक dd_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक dd_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अटल स्थिर अक्षर keywords[] ALIGN1 =
		"bs\0""count\0""seek\0""skip\0""if\0""of\0"IF_FEATURE_DD_STATUS("status\0")
#अगर ENABLE_FEATURE_DD_IBS_OBS
		"ibs\0""obs\0""conv\0""iflag\0""oflag\0"
#पूर्ण_अगर
		;
#अगर ENABLE_FEATURE_DD_IBS_OBS
	अटल स्थिर अक्षर conv_शब्दगण[] ALIGN1 =
		"notrunc\0""sync\0""noerror\0""fsync\0""swab\0";
	अटल स्थिर अक्षर iflag_शब्दगण[] ALIGN1 =
		"skip_bytes\0""count_bytes\0""fullblock\0""direct\0";
	अटल स्थिर अक्षर oflag_शब्दगण[] ALIGN1 =
		"seek_bytes\0append\0""direct\0";
#पूर्ण_अगर
#अगर ENABLE_FEATURE_DD_STATUS
	अटल स्थिर अक्षर status_शब्दगण[] ALIGN1 =
		"none\0""noxfer\0";
#पूर्ण_अगर
	क्रमागत {
		OP_bs = 0,
		OP_गिनती,
		OP_seek,
		OP_skip,
		OP_if,
		OP_of,
		IF_FEATURE_DD_STATUS(OP_status,)
#अगर ENABLE_FEATURE_DD_IBS_OBS
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
		OP_iflag_गिनती_bytes,
		OP_iflag_fullblock,
		OP_iflag_direct,
		OP_oflag_seek_bytes,
		OP_oflag_direct,
#पूर्ण_अगर
	};
	exitcode_t exitcode = निकास_त्रुटि;
	पूर्णांक i;
	माप_प्रकार ibs = 512;
	अक्षर *ibuf;
#अगर ENABLE_FEATURE_DD_IBS_OBS
	माप_प्रकार obs = 512;
	अक्षर *obuf;
#अन्यथा
# define obs  ibs
# define obuf ibuf
#पूर्ण_अगर
	/* These are all zeroed at once! */
	काष्ठा {
		IF_FEATURE_DD_IBS_OBS(माप_प्रकार ocount;)
		ssize_t पूर्व_read_आकार; /* for detecting swab failure */
		off_t गिनती;
		off_t seek, skip;
		स्थिर अक्षर *infile, *outfile;
	} Z;
#घोषणा ocount  (Z.ocount )
#घोषणा पूर्व_read_आकार (Z.पूर्व_read_आकार)
#घोषणा गिनती   (Z.गिनती  )
#घोषणा seek    (Z.seek   )
#घोषणा skip    (Z.skip   )
#घोषणा infile  (Z.infile )
#घोषणा outfile (Z.outfile)

	स_रखो(&Z, 0, माप(Z));
	INIT_G();
	//fflush_all(); - is this needed because of NOEXEC?

	क्रम (i = 1; argv[i]; i++) {
		पूर्णांक what;
		अक्षर *val;
		अक्षर *arg = argv[i];

#अगर ENABLE_DESKTOP
		/* "dd --". NB: coreutils 6.9 will complain if they see
		 * more than one of them. We wouldn't. */
		अगर (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
			जारी;
#पूर्ण_अगर
		val = म_अक्षर(arg, '=');
		अगर (val == शून्य)
			bb_show_usage();
		*val = '\0';
		what = सूचकांक_in_strings(keywords, arg);
		अगर (what < 0)
			bb_show_usage();
		/* *val = '='; - to preserve ps listing? */
		val++;
#अगर ENABLE_FEATURE_DD_IBS_OBS
		अगर (what == OP_ibs) {
			/* Must fit into positive ssize_t */
			ibs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		अगर (what == OP_obs) {
			obs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			/*continue;*/
		}
		अगर (what == OP_conv) {
			G.flags |= parse_comma_flags(val, conv_शब्दगण, "conv");
			/*continue;*/
		}
		अगर (what == OP_iflag) {
			G.flags |= parse_comma_flags(val, iflag_शब्दगण, "iflag") << FLAG_IFLAG_SHIFT;
			/*continue;*/
		}
		अगर (what == OP_oflag) {
			G.flags |= parse_comma_flags(val, oflag_शब्दगण, "oflag") << FLAG_OFLAG_SHIFT;
			/*continue;*/
		}
#पूर्ण_अगर
		अगर (what == OP_bs) {
			ibs = xatoul_range_sfx(val, 1, ((माप_प्रकार)-1L)/2, cwbkMG_suffixes);
			obs = ibs;
			/*continue;*/
		}
		/* These can be large: */
		अगर (what == OP_गिनती) {
			G.flags |= FLAG_गिनती;
			गिनती = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		अगर (what == OP_seek) {
			seek = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		अगर (what == OP_skip) {
			skip = XATOU_SFX(val, cwbkMG_suffixes);
			/*continue;*/
		}
		अगर (what == OP_if) {
			infile = val;
			/*continue;*/
		}
		अगर (what == OP_of) {
			outfile = val;
			/*continue;*/
		}
#अगर ENABLE_FEATURE_DD_STATUS
		अगर (what == OP_status) {
			पूर्णांक n;
			n = सूचकांक_in_strings(status_शब्दगण, val);
			अगर (n < 0)
				bb_error_संदेश_and_die(bb_संदेश_invalid_arg_to, val, "status");
			G.flags |= FLAG_STATUS_NONE << n;
			/*continue;*/
		}
#पूर्ण_अगर
	} /* end of "for (argv[i])" */

	ibuf = alloc_buf(ibs);
	obuf = ibuf;
#अगर ENABLE_FEATURE_DD_IBS_OBS
	अगर (ibs != obs) {
		G.flags |= FLAG_TWOBUFS;
		obuf = alloc_buf(obs);
	}
#पूर्ण_अगर

#अगर ENABLE_FEATURE_DD_SIGNAL_HANDLING
	signal_SA_RESTART_empty_mask(SIGUSR1, dd_output_status);
#पूर्ण_अगर
#अगर ENABLE_FEATURE_DD_THIRD_STATUS_पंक्ति
	G.begin_time_us = monotonic_us();
#पूर्ण_अगर

	अगर (infile) {
		पूर्णांक iflag = O_RDONLY;
#अगर ENABLE_FEATURE_DD_IBS_OBS
		अगर (G.flags & FLAG_IDIRECT) {
# ifdef O_DIRECT
			iflag |= O_DIRECT;
# अन्यथा
			bb_error_संदेश_and_die("O_DIRECT not supported on this platform");
# endif
		}
#पूर्ण_अगर
		xmove_fd(xopen(infile, iflag), ifd);
	} अन्यथा {
		infile = bb_संदेश_standard_निवेश;
	}
	अगर (outfile) {
		पूर्णांक oflag = O_WRONLY | O_CREAT;

		अगर (!seek && !(G.flags & FLAG_NOTRUNC))
			oflag |= O_TRUNC;
		अगर (G.flags & FLAG_APPEND)
			oflag |= O_APPEND;
#अगर ENABLE_FEATURE_DD_IBS_OBS
		अगर (G.flags & FLAG_ODIRECT) {
# ifdef O_DIRECT
			oflag |= O_DIRECT;
# अन्यथा
			bb_error_संदेश_and_die("O_DIRECT not supported on this platform");
# endif
		}
#पूर्ण_अगर
		xmove_fd(xopen(outfile, oflag), ofd);

		अगर (seek && !(G.flags & FLAG_NOTRUNC)) {
			माप_प्रकार blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
			अगर (ftruncate(ofd, seek * blocksz) < 0) {
				काष्ठा stat st;

				अगर (ख_स्थिति(ofd, &st) < 0
				 || S_ISREG(st.st_mode)
				 || S_ISDIR(st.st_mode)
				) {
					जाओ die_outfile;
				}
			}
		}
	} अन्यथा {
		outfile = bb_संदेश_standard_output;
	}
	अगर (skip) {
		माप_प्रकार blocksz = (G.flags & FLAG_SKIP_BYTES) ? 1 : ibs;
		अगर (lseek(ifd, skip * blocksz, प्रस्तुत_से) < 0) {
			करो {
				ssize_t n = dd_read(ibuf, blocksz);
				अगर (n < 0)
					जाओ die_infile;
				अगर (n == 0)
					अवरोध;
			} जबतक (--skip != 0);
		}
	}
	अगर (seek) {
		माप_प्रकार blocksz = (G.flags & FLAG_SEEK_BYTES) ? 1 : obs;
		अगर (lseek(ofd, seek * blocksz, प्रस्तुत_से) < 0)
			जाओ die_outfile;
	}

	जबतक (1) {
		ssize_t n = ibs;

		अगर (G.flags & FLAG_गिनती) {
			अगर (गिनती == 0)
				अवरोध;
			अगर ((G.flags & FLAG_गिनती_BYTES) && गिनती < ibs)
				n = गिनती;
		}

		n = dd_read(ibuf, n);
		अगर (n == 0)
			अवरोध;
		अगर (n < 0) {
			/* "Bad block" */
			अगर (!(G.flags & FLAG_NOERROR))
				जाओ die_infile;
			bb_simple_perror_संदेश(infile);
			/* GNU dd with conv=noerror skips over bad blocks */
			xlseek(ifd, ibs, प्रस्तुत_से);
			/* conv=noerror,sync writes NULs,
			 * conv=noerror just ignores input bad blocks */
			n = 0;
		}
		अगर (G.flags & FLAG_SWAB) {
			uint16_t *p16;
			ssize_t n2;

			/* Our code allows only last read to be odd-sized */
			अगर (पूर्व_read_आकार & 1)
				bb_error_संदेश_and_die("can't swab %lu byte buffer",
						(अचिन्हित दीर्घ)पूर्व_read_आकार);
			पूर्व_read_आकार = n;

			/* If n is odd, last byte is not swapped:
			 *  echo -n "qwe" | dd conv=swab
			 * prints "wqe".
			 */
			p16 = (व्योम*) ibuf;
			n2 = (n >> 1);
			जबतक (--n2 >= 0) {
				*p16 = bswap_16(*p16);
				p16++;
			}
		}
		गिनती -= (G.flags & FLAG_गिनती_BYTES) ? n : 1;
		अगर ((माप_प्रकार)n == ibs)
			G.in_full++;
		अन्यथा {
			G.in_part++;
			अगर (G.flags & FLAG_SYNC) {
				स_रखो(ibuf + n, 0, ibs - n);
				n = ibs;
			}
		}
#अगर ENABLE_FEATURE_DD_IBS_OBS
		अगर (G.flags & FLAG_TWOBUFS) {
			अक्षर *tmp = ibuf;
			जबतक (n) {
				माप_प्रकार d = obs - ocount;
				अगर (d > (माप_प्रकार)n)
					d = n;
				स_नकल(obuf + ocount, tmp, d);
				n -= d;
				tmp += d;
				ocount += d;
				अगर (ocount == obs) {
					अगर (write_and_stats(obuf, obs, obs, outfile))
						जाओ out_status;
					ocount = 0;
				}
			}
		} अन्यथा
#पूर्ण_अगर
		{
			अगर (write_and_stats(ibuf, n, obs, outfile))
				जाओ out_status;
		}
	}

	अगर (G.flags & FLAG_FSYNC) {
		अगर (fsync(ofd) < 0)
			जाओ die_outfile;
	}

#अगर ENABLE_FEATURE_DD_IBS_OBS
	अगर (ocount != 0) {
		अगर (write_and_stats(obuf, ocount, obs, outfile))
			जाओ out_status;
	}
#पूर्ण_अगर
	अगर (बंद(ifd) < 0) {
 die_infile:
		bb_simple_perror_संदेश_and_die(infile);
	}

	अगर (बंद(ofd) < 0) {
 die_outfile:
		bb_simple_perror_संदेश_and_die(outfile);
	}

	exitcode = निकास_सफल;
 out_status:
	अगर (!ENABLE_FEATURE_DD_STATUS || !(G.flags & FLAG_STATUS_NONE))
		dd_output_status(0);

#अगर 0 /* can't just free(), they can be mmap()ed */
	अगर (ENABLE_FEATURE_CLEAN_UP) {
		मुक्त(obuf);
		अगर (G.flags & FLAG_TWOBUFS)
			मुक्त(ibuf);
	}
#पूर्ण_अगर

	वापस exitcode;
}
