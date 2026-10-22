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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"
#ਜੇ ENABLE_FEATURE_DATE_NANO
# include <sys/syscall.h>
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ {
	OPT_RFC2822   = (1 << 0), /* R */
	OPT_ਸੈੱਟ       = (1 << 1), /* s */
	OPT_UTC       = (1 << 2), /* u */
	OPT_DATE      = (1 << 3), /* d */
	OPT_REFERENCE = (1 << 4), /* r */
	OPT_ISO8601   = (1 << 5) * ENABLE_FEATURE_DATE_ISOFMT, /* I */
	OPT_STR2DT    = (1 << 6) * ENABLE_FEATURE_DATE_ISOFMT, /* D */
};

#ਜੇ ENABLE_LONG_OPTS
ਸਥਿਰ ਅਚਲ ਅੱਖਰ date_longopts[] ALIGN1 =
		"rfc-822\0"   No_ਦਲੀਲ       "R"
		"rfc-2822\0"  No_ਦਲੀਲ       "R"
		"set\0"       Required_ਦਲੀਲ "s"
		"utc\0"       No_ਦਲੀਲ       "u"
	/*	"universal\0" No_argument       "u" */
		"date\0"      Required_ਦਲੀਲ "d"
		"reference\0" Required_ਦਲੀਲ "r"
		;
#ਜੇ_ਅੰਤ

/* We are a NOEXEC applet.
 * Obstacles to NOFORK:
 * - we change env
 * - xasprintf result not freed
 * - after xasprintf we use other xfuncs
 */

ਪੂਰਨ date_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ date_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਣਤਰ timespec ts;
	ਬਣਤਰ tm tm_ਸਮਾਂ;
	ਅੱਖਰ buf_fmt_dt2str[64];
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	ਪੂਰਨ isofmt = -1;
	ਅੱਖਰ *date_str;
	ਅੱਖਰ *fmt_dt2str;
	ਅੱਖਰ *fmt_str2dt;
	ਅੱਖਰ *filename;
	ਅੱਖਰ *isofmt_arg = ਨਿਰਾ;

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

	ਜੇ (opt & OPT_UTC)
		putenv((ਅੱਖਰ*)"TZ=UTC0");

	ਜੇ (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_ISO8601)) {
		isofmt = 0; /* default is date */
		ਜੇ (isofmt_arg) {
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ isoformats[] ALIGN1 =
				"date\0""hours\0""minutes\0""seconds\0ns\0";
			isofmt = ਸੂਚਕਾਂਕ_ਵਿੱਚ_substrings(isoformats, isofmt_arg);
			ਜੇ (isofmt < 0)
				bb_show_usage();
		}
	}

	fmt_dt2str = ਨਿਰਾ;
	ਜੇ (argv[0] && argv[0][0] == '+') {
		fmt_dt2str = &argv[0][1]; /* skip over the '+' */
		argv++;
	}
	ਜੇ (!(opt & (OPT_ਸੈੱਟ | OPT_DATE))) { /* neither -s TIME nor -d TIME? */
		opt |= OPT_ਸੈੱਟ;
		date_str = argv[0]; /* can be NULL */
		ਜੇ (date_str) {
#ਜੇ ENABLE_FEATURE_DATE_COMPAT
			ਪੂਰਨ len = strspn(date_str, "0123456789");
			ਜੇ (date_str[len] == '\0'
			 || (date_str[len] == '.'
			    && ਅੰਕ_ਹੈ(date_str[len+1])
			    && ਅੰਕ_ਹੈ(date_str[len+2])
			    && date_str[len+3] == '\0'
			    )
			) {
				/* Dreaded MMDDhhmm[[CC]YY][.ss] format!
				 * It does not match -d or -s format.
				 * Some users actually do use it.
				 */
				len -= 8;
				ਜੇ (len < 0 || len > 4 || (len & 1))
					bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_invalid_date, date_str);
				ਜੇ (len != 0) { /* move YY or CCYY to front */
					ਅੱਖਰ buf[4];
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, date_str + 8, len);
					memmove(date_str + len, date_str, 8);
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(date_str, buf, len);
				}
			}
#ਜੇ_ਅੰਤ
			argv++;
		}
	}
	ਜੇ (*argv)
		bb_show_usage();

	/* Now we have parsed all the information except the date format
	 * which depends on whether the clock is being set or read */

	ਜੇ (opt & OPT_REFERENCE) {
		ਬਣਤਰ stat statbuf;
		xstat(filename, &statbuf);
		ts.tv_sec = statbuf.st_mtime;
#ਜੇ ENABLE_FEATURE_DATE_NANO
		ts.tv_nsec = statbuf.st_mtim.tv_nsec;
		/* Some toolchains use .st_mtimensec instead of st_mtim.tv_nsec.
		 * If you need #define _SVID_SOURCE 1 to enable st_mtim.tv_nsec,
		 * drop a mail to project mailing list please
		 */
#ਜੇ_ਅੰਤ
	} ਨਹੀਂਤਾਂ {
#ਜੇ ENABLE_FEATURE_DATE_NANO
		clock_gettime(CLOCK_REALTIME, &ts);
#ਨਹੀਂਤਾਂ
		ਸਮਾਂ(&ts.tv_sec);
#ਜੇ_ਅੰਤ
	}
#ਜੇ !ENABLE_FEATURE_DATE_NANO
	ts.tv_nsec = 0;
#ਜੇ_ਅੰਤ
	localtime_r(&ts.tv_sec, &tm_ਸਮਾਂ);

	/* If date string is given, update tm_time, and maybe set date */
	ਜੇ (date_str != ਨਿਰਾ) {
		ਪੂਰਨ check_dst = 1;
		/* Zero out fields - take her back to midnight! */
		tm_ਸਮਾਂ.tm_sec = 0;
		tm_ਸਮਾਂ.tm_min = 0;
		tm_ਸਮਾਂ.tm_hour = 0;

		/* Process any date input to UNIX time since 1 Jan 1970 */
		ਜੇ (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_STR2DT)) {
			ਜੇ (strptime(date_str, fmt_str2dt, &tm_ਸਮਾਂ) == ਨਿਰਾ)
				bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_invalid_date, date_str);
		} ਨਹੀਂਤਾਂ {
			check_dst = parse_datestr(date_str, &tm_ਸਮਾਂ);
		}

		/* Correct any day of week and day of year etc. fields */
		/* Be sure to recheck dst (but not if date is UTC) */
		ਜੇ (check_dst)
			tm_ਸਮਾਂ.tm_isdst = -1;
		ts.tv_sec = validate_tm_ਸਮਾਂ(date_str, &tm_ਸਮਾਂ);
		ts.tv_nsec = 0;

		/* if setting time, set it */
		ਜੇ ((opt & OPT_ਸੈੱਟ) && clock_settime(CLOCK_REALTIME, &ts) < 0) {
			bb_simple_perror_ਸੁਨੇਹਾ("can't set date");
		}
	}

	/* Display output */

	/* Deal with format string */
	ਜੇ (fmt_dt2str == ਨਿਰਾ) {
		ਪੂਰਨ i;
		fmt_dt2str = buf_fmt_dt2str;
		ਜੇ (ENABLE_FEATURE_DATE_ISOFMT && isofmt >= 0) {
			/* -I[SPEC]: 0:date 1:hours 2:minutes 3:seconds 4:ns*/
			strcpy(fmt_dt2str, "%Y-%m-%dT%H:%M:%S");
			i = 8 + 3 * isofmt;
			ਜੇ (isofmt != 0) {
				ਪੂਰਨ n;
				ਜੇ (isofmt == 4) {
					i -= 3;
					i += sprintf(&fmt_dt2str[i], ",%09u", (ਬਿਨਾਨਿਸ਼ਾਨੀ)ts.tv_nsec);
				}
				/* %z prints "+hhmm" timezone, but coreutils-8.30 prints "+hh:mm"! */
				/* ...therefore this atrocity: */
				n = strftime(&fmt_dt2str[i], 8, "%z", &tm_ਸਮਾਂ);
				i += n;
				ਜੇ (n == 5 && (fmt_dt2str[i-5] == '+' || fmt_dt2str[i-5] == '-')) {
					/* "mm" -> ":mm" */
					fmt_dt2str[i    ] = fmt_dt2str[i - 1];
					fmt_dt2str[i - 1] = fmt_dt2str[i - 2];
					fmt_dt2str[i - 2] = ':';
					i++;
				}
			}
			fmt_dt2str[i] = '\0';
		} ਨਹੀਂਤਾਂ ਜੇ (opt & OPT_RFC2822) {
			/* -R. undo busybox.c setlocale */
			ਜੇ (ENABLE_LOCALE_SUPPORT)
				setlocale(LC_ਸਮਾਂ, "C");
			fmt_dt2str = (ਅੱਖਰ*)"%a, %d %b %Y %H:%M:%S %z";
		} ਨਹੀਂਤਾਂ { /* default case */
			fmt_dt2str = (ਅੱਖਰ*)"%a %b %e %H:%M:%S %Z %Y";
		}
	}
#ਜੇ ENABLE_FEATURE_DATE_NANO
	ਨਹੀਂਤਾਂ {
		/* User-specified fmt_dt2str */
		/* Search for and process "%N" */
		ਅੱਖਰ *p = fmt_dt2str;
		ਜਦੋਂ ((p = strchr(p, '%')) != ਨਿਰਾ) {
			ਪੂਰਨ n, m;
			ਬਿਨਾਨਿਸ਼ਾਨੀ pres, scale;

			p++;
			ਜੇ (*p == '%') {
				p++;
				ਜਾਰੀ;
			}
			n = strspn(p, "0123456789");
			ਜੇ (p[n] != 'N') {
				p += n;
				ਜਾਰੀ;
			}
			/* We have "%[nnn]N" */
			p[-1] = '\0';
			p[n] = '\0';
			scale = 1;
			pres = 9;
			ਜੇ (n) {
				pres = xatoi_ਸਕਾਰਾਤਮਕ(p);
				ਜੇ (pres == 0)
					pres = 9;
				m = 9 - pres;
				ਜਦੋਂ (--m >= 0)
					scale *= 10;
			}

			m = p - fmt_dt2str;
			p += n + 1;
			fmt_dt2str = xasprintf("%s%0*u%s", fmt_dt2str, pres, (ਬਿਨਾਨਿਸ਼ਾਨੀ)ts.tv_nsec / scale, p);
			p = fmt_dt2str + m;
		}
	}
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ date_buf bb_common_bufsiz1
	ਤਿਆਰੀ_common_bufsiz();
	ਜੇ (*fmt_dt2str == '\0') {
		/* With no format string, just print a blank line */
		date_buf[0] = '\0';
	} ਨਹੀਂਤਾਂ {
		/* Generate output string */
		strftime(date_buf, COMMON_BUFSIZE, fmt_dt2str, &tm_ਸਮਾਂ);
	}
	puts(date_buf);

	ਵਾਪਸ EXIT_SUCCESS;
}
