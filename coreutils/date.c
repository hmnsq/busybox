/* vi: set sw=4 ts=4: */
/*
 * Mini date implementation for busybox
 *
 * by Matthew Grant <grantma@anathoth.gen.nz>
 *
 * iso-format handling added by Robert Griebl <griebl@gmx.de>
 * bugfixes and cleanup by Bernhard Reutner-Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* This 'date' command supports only 2 time setting formats,
   all the GNU strftime stuff (its in libc, lets use it),
   setting time using UTC and displaying it, as well as
   an RFC 2822 compliant date output for shell scripting
   mail commands */

/* Input parsing code is always bulky - used heavy duty libc stuff as
   much as possible, missed out a lot of bounds checking */

//config:config DATE
//config:	bool "date (7.2 kb)"
//config:	default y
//config:	help
//config:	date is used to set the system date or display the
//config:	current time in the given format.
//config:
//config:config FEATURE_DATE_ISOFMT
//config:	bool "Enable ISO date format output (-I)"
//config:	default y
//config:	depends on DATE
//config:	help
//config:	Enable option (-I) to output an ISO-8601 compliant
//config:	date/time string.
//config:
//config:config FEATURE_DATE_NANO
//config:	bool "Support %[num]N nanosecond format specifier"
//config:	default n # stat's nanosecond field is a bit non-portable
//config:	depends on DATE
//config:	help
//config:	Support %[num]N format specifier. Adds ~250 bytes of code.
//config:
//config:config FEATURE_DATE_COMPAT
//config:	bool "Support weird 'date MMDDhhmm[[YY]YY][.ss]' format"
//config:	default y
//config:	depends on DATE
//config:	help
//config:	System time can be set by 'date -s DATE' and simply 'date DATE',
//config:	but formats of DATE string are different. 'date DATE' accepts
//config:	a rather weird MMDDhhmm[[YY]YY][.ss] format with completely
//config:	unnatural placement of year between minutes and seconds.
//config:	date -s (and other commands like touch -d) use more sensible
//config:	formats (for one, ISO format YYYY-MM-DD hh:mm:ss.ssssss).
//config:
//config:	With this option off, 'date DATE' and 'date -s DATE' support
//config:	the same format. With it on, 'date DATE' additionally supports
//config:	MMDDhhmm[[YY]YY][.ss] format.

//applet:IF_DATE(APPLET_NOEXEC(date, date, BB_DIR_BIN, BB_SUID_DROP, date))
/* bb_common_bufsiz1 usage here is safe wrt NOEXEC: not expecting it to be zeroed. */

//kbuild:lib-$(CONFIG_DATE) += date.o

/* GNU coreutils 6.9 man page:
 * date [OPTION]... [+FORMAT]
 * date [-u|--utc|--universal] [MMDDhhmm[[CC]YY][.ss]]
 * -d, --date=STRING
 *      display time described by STRING, not 'now'
 * -f, --file=DATEFILE
 *      like --date once for each line of DATEFILE
 * -r, --reference=FILE
 *      display the last modification time of FILE
 * -R, --rfc-2822
 *      output date and time in RFC 2822 format.
 *      Example: Mon, 07 Aug 2006 12:34:56 -0600
 * --rfc-3339=TIMESPEC
 *      output date and time in RFC 3339 format.
 *      TIMESPEC='date', 'seconds', or 'ns'
 *      Date and time components are separated by a single space:
 *      2006-08-07 12:34:56-06:00
 * -s, --set=STRING
 *      set time described by STRING
 * -u, --utc, --universal
 *      print or set Coordinated Universal Time
 *
 * Busybox:
 * long options are not supported
 * -f is not supported
 * -I seems to roughly match --rfc-3339, but -I has _optional_ param
 *    (thus "-I seconds" doesn't work, only "-Iseconds"),
 *    and does not support -Ins
 * -D FMT is a bbox extension for _input_ conversion of -d DATE
 */

//usage:#define date_trivial_usage
//usage:       "[OPTIONS] [+FMT] [[-s] TIME]"
//usage:#define date_full_usage "\n\n"
//usage:       "Display time (using +FMT), or set time\n"
//usage:     "\n	-u		Work in UTC (don't convert to local time)"
//usage:     "\n	[-s] TIME	Set time to TIME"
//usage:     "\n	-d TIME		Display TIME, not 'now'"
//usage:	IF_FEATURE_DATE_ISOFMT(
//usage:     "\n	-D FMT		FMT (strptime format) for -s/-d TIME conversion"
////////^^^^^^^^^^^^^^^^^^^^^^ busybox invention, not compat
//usage:	)
//usage:     "\n	-r FILE		Display last modification time of FILE"
//usage:     "\n	-R		Output RFC-2822 date"
//usage:	IF_FEATURE_DATE_ISOFMT(
//usage:     "\n	-I[SPEC]	Output ISO-8601 date"
//usage:     "\n			SPEC=date (default), hours, minutes, seconds or ns"
//usage:	)
//usage:     "\n"
//usage:     "\nRecognized TIME formats:"
//usage:     "\n	@seconds_since_1970"
//usage:     "\n	hh:mm[:ss]"
//usage:     "\n	[YYYY.]MM.DD-hh:mm[:ss]"
//usage:     "\n	YYYY-MM-DD hh:mm[:ss]"
//usage:     "\n	[[[[[YY]YY]MM]DD]hh]mm[.ss]"
//usage:	IF_FEATURE_DATE_COMPAT(
//usage:     "\n	'date TIME' form accepts MMDDhhmm[[YY]YY][.ss] instead"
//usage:	)
//usage:
//usage:#define date_example_usage
//usage:       "$ date\n"
//usage:       "Wed Apr 12 18:52:41 MDT 2000\n"

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"
#ఒకవేళ ENABLE_FEATURE_DATE_NANO
# include <sys/syscall.h>
#అయితే_ముగింపు

గణన {
	OPT_RFC2822   = (1 << 0), /* R */
	OPT_సమితి       = (1 << 1), /* s */
	OPT_UTC       = (1 << 2), /* u */
	OPT_DATE      = (1 << 3), /* d */
	OPT_REFERENCE = (1 << 4), /* r */
	OPT_ISO8601   = (1 << 5) * ENABLE_FEATURE_DATE_ISOFMT, /* I */
	OPT_STR2DT    = (1 << 6) * ENABLE_FEATURE_DATE_ISOFMT, /* D */
};

#ఒకవేళ ENABLE_LONG_OPTS
స్థిర స్థిరరాశి అక్షరం date_longopts[] ALIGN1 =
		"rfc-822\0"   No_వాదం       "R"
		"rfc-2822\0"  No_వాదం       "R"
		"set\0"       Required_వాదం "s"
		"utc\0"       No_వాదం       "u"
	/*	"universal\0" No_argument       "u" */
		"date\0"      Required_వాదం "d"
		"reference\0" Required_వాదం "r"
		;
#అయితే_ముగింపు

/* We are a NOEXEC applet.
 * Obstacles to NOFORK:
 * - we change env
 * - xasprintf result not freed
 * - after xasprintf we use other xfuncs
 */

పూర్ణాంకం date_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం date_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	నిర్మాణం timespec ts;
	నిర్మాణం tm tm_సమయం;
	అక్షరం buf_fmt_dt2str[64];
	గుర్తులేని opt;
	పూర్ణాంకం isofmt = -1;
	అక్షరం *date_str;
	అక్షరం *fmt_dt2str;
	అక్షరం *fmt_str2dt;
	అక్షరం *filename;
	అక్షరం *isofmt_arg = ఖాళీ;

	opt = getopt32long(argv, "^"
			"Rs:ud:r:"
			IF_FEATURE_DATE_ISOFMT("I::D:")
			"\0"
			"d--s:s--d"
			IF_FEATURE_DATE_ISOFMT(":R--I:I--R"),
			date_longopts,
			&date_str, &date_str, &filename
			IF_FEATURE_DATE_ISOFMT(, &isofmt_arg, &fmt_str2dt)
	);
	argv += optind;

	ఒకవేళ (opt & OPT_UTC)
		putenv((అక్షరం*)"TZ=UTC0");

	ఒకవేళ (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_ISO8601)) {
		isofmt = 0; /* default is date */
		ఒకవేళ (isofmt_arg) {
			స్థిర స్థిరరాశి అక్షరం isoformats[] ALIGN1 =
				"date\0""hours\0""minutes\0""seconds\0ns\0";
			isofmt = సూచీ_లోపల_substrings(isoformats, isofmt_arg);
			ఒకవేళ (isofmt < 0)
				bb_show_usage();
		}
	}

	fmt_dt2str = ఖాళీ;
	ఒకవేళ (argv[0] && argv[0][0] == '+') {
		fmt_dt2str = &argv[0][1]; /* skip over the '+' */
		argv++;
	}
	ఒకవేళ (!(opt & (OPT_సమితి | OPT_DATE))) { /* neither -s TIME nor -d TIME? */
		opt |= OPT_సమితి;
		date_str = argv[0]; /* can be NULL */
		ఒకవేళ (date_str) {
#ఒకవేళ ENABLE_FEATURE_DATE_COMPAT
			పూర్ణాంకం len = strspn(date_str, "0123456789");
			ఒకవేళ (date_str[len] == '\0'
			 || (date_str[len] == '.'
			    && అంకెనా(date_str[len+1])
			    && అంకెనా(date_str[len+2])
			    && date_str[len+3] == '\0'
			    )
			) {
				/* Dreaded MMDDhhmm[[CC]YY][.ss] format!
				 * It does not match -d or -s format.
				 * Some users actually do use it.
				 */
				len -= 8;
				ఒకవేళ (len < 0 || len > 4 || (len & 1))
					bb_error_సందేశం_and_die(bb_సందేశం_invalid_date, date_str);
				ఒకవేళ (len != 0) { /* move YY or CCYY to front */
					అక్షరం buf[4];
					జ్ఞాపకం_నకలు(buf, date_str + 8, len);
					memmove(date_str + len, date_str, 8);
					జ్ఞాపకం_నకలు(date_str, buf, len);
				}
			}
#అయితే_ముగింపు
			argv++;
		}
	}
	ఒకవేళ (*argv)
		bb_show_usage();

	/* Now we have parsed all the information except the date format
	 * which depends on whether the clock is being set or read */

	ఒకవేళ (opt & OPT_REFERENCE) {
		నిర్మాణం stat statbuf;
		xstat(filename, &statbuf);
		ts.tv_sec = statbuf.st_mtime;
#ఒకవేళ ENABLE_FEATURE_DATE_NANO
		ts.tv_nsec = statbuf.st_mtim.tv_nsec;
		/* Some toolchains use .st_mtimensec instead of st_mtim.tv_nsec.
		 * If you need #define _SVID_SOURCE 1 to enable st_mtim.tv_nsec,
		 * drop a mail to project mailing list please
		 */
#అయితే_ముగింపు
	} లేకపోతే {
#ఒకవేళ ENABLE_FEATURE_DATE_NANO
		clock_gettime(CLOCK_REALTIME, &ts);
#లేకపోతే
		సమయం(&ts.tv_sec);
#అయితే_ముగింపు
	}
#ఒకవేళ !ENABLE_FEATURE_DATE_NANO
	ts.tv_nsec = 0;
#అయితే_ముగింపు
	localtime_r(&ts.tv_sec, &tm_సమయం);

	/* If date string is given, update tm_time, and maybe set date */
	ఒకవేళ (date_str != ఖాళీ) {
		పూర్ణాంకం check_dst = 1;
		/* Zero out fields - take her back to midnight! */
		tm_సమయం.tm_sec = 0;
		tm_సమయం.tm_min = 0;
		tm_సమయం.tm_hour = 0;

		/* Process any date input to UNIX time since 1 Jan 1970 */
		ఒకవేళ (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_STR2DT)) {
			ఒకవేళ (strptime(date_str, fmt_str2dt, &tm_సమయం) == ఖాళీ)
				bb_error_సందేశం_and_die(bb_సందేశం_invalid_date, date_str);
		} లేకపోతే {
			check_dst = parse_datestr(date_str, &tm_సమయం);
		}

		/* Correct any day of week and day of year etc. fields */
		/* Be sure to recheck dst (but not if date is UTC) */
		ఒకవేళ (check_dst)
			tm_సమయం.tm_isdst = -1;
		ts.tv_sec = validate_tm_సమయం(date_str, &tm_సమయం);
		ts.tv_nsec = 0;

		/* if setting time, set it */
		ఒకవేళ ((opt & OPT_సమితి) && clock_settime(CLOCK_REALTIME, &ts) < 0) {
			bb_simple_perror_సందేశం("can't set date");
		}
	}

	/* Display output */

	/* Deal with format string */
	ఒకవేళ (fmt_dt2str == ఖాళీ) {
		పూర్ణాంకం i;
		fmt_dt2str = buf_fmt_dt2str;
		ఒకవేళ (ENABLE_FEATURE_DATE_ISOFMT && isofmt >= 0) {
			/* -I[SPEC]: 0:date 1:hours 2:minutes 3:seconds 4:ns*/
			strcpy(fmt_dt2str, "%Y-%m-%dT%H:%M:%S");
			i = 8 + 3 * isofmt;
			ఒకవేళ (isofmt != 0) {
				పూర్ణాంకం n;
				ఒకవేళ (isofmt == 4) {
					i -= 3;
					i += sprintf(&fmt_dt2str[i], ",%09u", (గుర్తులేని)ts.tv_nsec);
				}
				/* %z prints "+hhmm" timezone, but coreutils-8.30 prints "+hh:mm"! */
				/* ...therefore this atrocity: */
				n = strftime(&fmt_dt2str[i], 8, "%z", &tm_సమయం);
				i += n;
				ఒకవేళ (n == 5 && (fmt_dt2str[i-5] == '+' || fmt_dt2str[i-5] == '-')) {
					/* "mm" -> ":mm" */
					fmt_dt2str[i    ] = fmt_dt2str[i - 1];
					fmt_dt2str[i - 1] = fmt_dt2str[i - 2];
					fmt_dt2str[i - 2] = ':';
					i++;
				}
			}
			fmt_dt2str[i] = '\0';
		} లేకపోతే ఒకవేళ (opt & OPT_RFC2822) {
			/* -R. undo busybox.c setlocale */
			ఒకవేళ (ENABLE_LOCALE_SUPPORT)
				setlocale(LC_సమయం, "C");
			fmt_dt2str = (అక్షరం*)"%a, %d %b %Y %H:%M:%S %z";
		} లేకపోతే { /* default case */
			fmt_dt2str = (అక్షరం*)"%a %b %e %H:%M:%S %Z %Y";
		}
	}
#ఒకవేళ ENABLE_FEATURE_DATE_NANO
	లేకపోతే {
		/* User-specified fmt_dt2str */
		/* Search for and process "%N" */
		అక్షరం *p = fmt_dt2str;
		అయితే ((p = strchr(p, '%')) != ఖాళీ) {
			పూర్ణాంకం n, m;
			గుర్తులేని pres, scale;

			p++;
			ఒకవేళ (*p == '%') {
				p++;
				కొనసాగు;
			}
			n = strspn(p, "0123456789");
			ఒకవేళ (p[n] != 'N') {
				p += n;
				కొనసాగు;
			}
			/* We have "%[nnn]N" */
			p[-1] = '\0';
			p[n] = '\0';
			scale = 1;
			pres = 9;
			ఒకవేళ (n) {
				pres = xatoi_ధనాత్మక(p);
				ఒకవేళ (pres == 0)
					pres = 9;
				m = 9 - pres;
				అయితే (--m >= 0)
					scale *= 10;
			}

			m = p - fmt_dt2str;
			p += n + 1;
			fmt_dt2str = xasprintf("%s%0*u%s", fmt_dt2str, pres, (గుర్తులేని)ts.tv_nsec / scale, p);
			p = fmt_dt2str + m;
		}
	}
#అయితే_ముగింపు

#నిర్వచించు date_buf bb_common_bufsiz1
	అమరిక_common_bufsiz();
	ఒకవేళ (*fmt_dt2str == '\0') {
		/* With no format string, just print a blank line */
		date_buf[0] = '\0';
	} లేకపోతే {
		/* Generate output string */
		strftime(date_buf, COMMON_BUFSIZE, fmt_dt2str, &tm_సమయం);
	}
	puts(date_buf);

	తిరిగి EXIT_SUCCESS;
}
