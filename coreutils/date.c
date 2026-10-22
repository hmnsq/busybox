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

#شامل "libbb.h"
#شامل "common_bufsiz.h"
#اگر ENABLE_FEATURE_DATE_NANO
# include <sys/syscall.h>
#اگر_ختم

شمار {
	OPT_RFC2822   = (1 << 0), /* R */
	OPT_سیٹ       = (1 << 1), /* s */
	OPT_UTC       = (1 << 2), /* u */
	OPT_DATE      = (1 << 3), /* d */
	OPT_REFERENCE = (1 << 4), /* r */
	OPT_ISO8601   = (1 << 5) * ENABLE_FEATURE_DATE_ISOFMT, /* I */
	OPT_STR2DT    = (1 << 6) * ENABLE_FEATURE_DATE_ISOFMT, /* D */
};

#اگر ENABLE_LONG_OPTS
ساکن ثابت حرف date_longopts[] ALIGN1 =
		"rfc-822\0"   No_دلیل       "R"
		"rfc-2822\0"  No_دلیل       "R"
		"set\0"       Required_دلیل "s"
		"utc\0"       No_دلیل       "u"
	/*	"universal\0" No_argument       "u" */
		"date\0"      Required_دلیل "d"
		"reference\0" Required_دلیل "r"
		;
#اگر_ختم

/* We are a NOEXEC applet.
 * Obstacles to NOFORK:
 * - we change env
 * - xasprintf result not freed
 * - after xasprintf we use other xfuncs
 */

صحیح date_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح date_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	ساخت timespec ts;
	ساخت tm tm_وقت;
	حرف buf_fmt_dt2str[64];
	غیرعلامتی opt;
	صحیح isofmt = -1;
	حرف *date_str;
	حرف *fmt_dt2str;
	حرف *fmt_str2dt;
	حرف *filename;
	حرف *isofmt_arg = تہی;

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

	اگر (opt & OPT_UTC)
		putenv((حرف*)"TZ=UTC0");

	اگر (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_ISO8601)) {
		isofmt = 0; /* default is date */
		اگر (isofmt_arg) {
			ساکن ثابت حرف isoformats[] ALIGN1 =
				"date\0""hours\0""minutes\0""seconds\0ns\0";
			isofmt = اشاریہ_اندر_substrings(isoformats, isofmt_arg);
			اگر (isofmt < 0)
				bb_show_usage();
		}
	}

	fmt_dt2str = تہی;
	اگر (argv[0] && argv[0][0] == '+') {
		fmt_dt2str = &argv[0][1]; /* skip over the '+' */
		argv++;
	}
	اگر (!(opt & (OPT_سیٹ | OPT_DATE))) { /* neither -s TIME nor -d TIME? */
		opt |= OPT_سیٹ;
		date_str = argv[0]; /* can be NULL */
		اگر (date_str) {
#اگر ENABLE_FEATURE_DATE_COMPAT
			صحیح len = strspn(date_str, "0123456789");
			اگر (date_str[len] == '\0'
			 || (date_str[len] == '.'
			    && ہندسہ_ہے(date_str[len+1])
			    && ہندسہ_ہے(date_str[len+2])
			    && date_str[len+3] == '\0'
			    )
			) {
				/* Dreaded MMDDhhmm[[CC]YY][.ss] format!
				 * It does not match -d or -s format.
				 * Some users actually do use it.
				 */
				len -= 8;
				اگر (len < 0 || len > 4 || (len & 1))
					bb_error_پیغام_and_die(bb_پیغام_invalid_date, date_str);
				اگر (len != 0) { /* move YY or CCYY to front */
					حرف buf[4];
					حافظہ_نقل(buf, date_str + 8, len);
					memmove(date_str + len, date_str, 8);
					حافظہ_نقل(date_str, buf, len);
				}
			}
#اگر_ختم
			argv++;
		}
	}
	اگر (*argv)
		bb_show_usage();

	/* Now we have parsed all the information except the date format
	 * which depends on whether the clock is being set or read */

	اگر (opt & OPT_REFERENCE) {
		ساخت stat statbuf;
		xstat(filename, &statbuf);
		ts.tv_sec = statbuf.st_mtime;
#اگر ENABLE_FEATURE_DATE_NANO
		ts.tv_nsec = statbuf.st_mtim.tv_nsec;
		/* Some toolchains use .st_mtimensec instead of st_mtim.tv_nsec.
		 * If you need #define _SVID_SOURCE 1 to enable st_mtim.tv_nsec,
		 * drop a mail to project mailing list please
		 */
#اگر_ختم
	} ورنہ {
#اگر ENABLE_FEATURE_DATE_NANO
		clock_gettime(CLOCK_REALTIME, &ts);
#ورنہ
		وقت(&ts.tv_sec);
#اگر_ختم
	}
#اگر !ENABLE_FEATURE_DATE_NANO
	ts.tv_nsec = 0;
#اگر_ختم
	localtime_r(&ts.tv_sec, &tm_وقت);

	/* If date string is given, update tm_time, and maybe set date */
	اگر (date_str != تہی) {
		صحیح check_dst = 1;
		/* Zero out fields - take her back to midnight! */
		tm_وقت.tm_sec = 0;
		tm_وقت.tm_min = 0;
		tm_وقت.tm_hour = 0;

		/* Process any date input to UNIX time since 1 Jan 1970 */
		اگر (ENABLE_FEATURE_DATE_ISOFMT && (opt & OPT_STR2DT)) {
			اگر (strptime(date_str, fmt_str2dt, &tm_وقت) == تہی)
				bb_error_پیغام_and_die(bb_پیغام_invalid_date, date_str);
		} ورنہ {
			check_dst = parse_datestr(date_str, &tm_وقت);
		}

		/* Correct any day of week and day of year etc. fields */
		/* Be sure to recheck dst (but not if date is UTC) */
		اگر (check_dst)
			tm_وقت.tm_isdst = -1;
		ts.tv_sec = validate_tm_وقت(date_str, &tm_وقت);
		ts.tv_nsec = 0;

		/* if setting time, set it */
		اگر ((opt & OPT_سیٹ) && clock_settime(CLOCK_REALTIME, &ts) < 0) {
			bb_simple_perror_پیغام("can't set date");
		}
	}

	/* Display output */

	/* Deal with format string */
	اگر (fmt_dt2str == تہی) {
		صحیح i;
		fmt_dt2str = buf_fmt_dt2str;
		اگر (ENABLE_FEATURE_DATE_ISOFMT && isofmt >= 0) {
			/* -I[SPEC]: 0:date 1:hours 2:minutes 3:seconds 4:ns*/
			strcpy(fmt_dt2str, "%Y-%m-%dT%H:%M:%S");
			i = 8 + 3 * isofmt;
			اگر (isofmt != 0) {
				صحیح n;
				اگر (isofmt == 4) {
					i -= 3;
					i += sprintf(&fmt_dt2str[i], ",%09u", (غیرعلامتی)ts.tv_nsec);
				}
				/* %z prints "+hhmm" timezone, but coreutils-8.30 prints "+hh:mm"! */
				/* ...therefore this atrocity: */
				n = strftime(&fmt_dt2str[i], 8, "%z", &tm_وقت);
				i += n;
				اگر (n == 5 && (fmt_dt2str[i-5] == '+' || fmt_dt2str[i-5] == '-')) {
					/* "mm" -> ":mm" */
					fmt_dt2str[i    ] = fmt_dt2str[i - 1];
					fmt_dt2str[i - 1] = fmt_dt2str[i - 2];
					fmt_dt2str[i - 2] = ':';
					i++;
				}
			}
			fmt_dt2str[i] = '\0';
		} ورنہ اگر (opt & OPT_RFC2822) {
			/* -R. undo busybox.c setlocale */
			اگر (ENABLE_LOCALE_SUPPORT)
				setlocale(LC_وقت, "C");
			fmt_dt2str = (حرف*)"%a, %d %b %Y %H:%M:%S %z";
		} ورنہ { /* default case */
			fmt_dt2str = (حرف*)"%a %b %e %H:%M:%S %Z %Y";
		}
	}
#اگر ENABLE_FEATURE_DATE_NANO
	ورنہ {
		/* User-specified fmt_dt2str */
		/* Search for and process "%N" */
		حرف *p = fmt_dt2str;
		جب ((p = strchr(p, '%')) != تہی) {
			صحیح n, m;
			غیرعلامتی pres, scale;

			p++;
			اگر (*p == '%') {
				p++;
				جاری;
			}
			n = strspn(p, "0123456789");
			اگر (p[n] != 'N') {
				p += n;
				جاری;
			}
			/* We have "%[nnn]N" */
			p[-1] = '\0';
			p[n] = '\0';
			scale = 1;
			pres = 9;
			اگر (n) {
				pres = xatoi_مثبت(p);
				اگر (pres == 0)
					pres = 9;
				m = 9 - pres;
				جب (--m >= 0)
					scale *= 10;
			}

			m = p - fmt_dt2str;
			p += n + 1;
			fmt_dt2str = xasprintf("%s%0*u%s", fmt_dt2str, pres, (غیرعلامتی)ts.tv_nsec / scale, p);
			p = fmt_dt2str + m;
		}
	}
#اگر_ختم

#تعریف_کرو date_buf bb_common_bufsiz1
	تیاری_common_bufsiz();
	اگر (*fmt_dt2str == '\0') {
		/* With no format string, just print a blank line */
		date_buf[0] = '\0';
	} ورنہ {
		/* Generate output string */
		strftime(date_buf, COMMON_BUFSIZE, fmt_dt2str, &tm_وقت);
	}
	puts(date_buf);

	واپس EXIT_SUCCESS;
}
