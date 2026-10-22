/* vi: set sw=4 ts=4: */
/*
 * Mini who is used to display user name, login time,
 * idle time and host name.
 *
 * Author: Da Chen  <dchen@ayrnetworks.com>
 *
 * This is a free document; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation:
 *    http://www.gnu.org/copyleft/gpl.html
 *
 * Copyright (c) 2002 AYR Networks, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config WHO
//config:	bool "who (5.6 kb)"
//config:	default y
//config:	depends on FEATURE_UTMP
//config:	help
//config:	Print users currently logged on.
//config:
// procps-ng has this variation of "who":
//config:config W
//config:	bool "w (5.5 kb)"
//config:	default y
//config:	depends on FEATURE_UTMP
//config:	help
//config:	Print users currently logged on.
//config:
//config:config USERS
//config:	bool "users (3.6 kb)"
//config:	default y
//config:	depends on FEATURE_UTMP
//config:	help
//config:	Print users currently logged on.

//                APPLET_NOEXEC:name   main location        suid_type     help
//applet:IF_USERS(APPLET_NOEXEC(users, who, BB_DIR_USR_BIN, BB_SUID_DROP, users))
//applet:IF_W(    APPLET_NOEXEC(w,     who, BB_DIR_USR_BIN, BB_SUID_DROP, w))
//applet:IF_WHO(  APPLET_NOEXEC(who,   who, BB_DIR_USR_BIN, BB_SUID_DROP, who))

//kbuild:lib-$(CONFIG_USERS) += who.o
//kbuild:lib-$(CONFIG_W)     += who.o
//kbuild:lib-$(CONFIG_WHO)   += who.o

/* BB_AUDIT SUSv3 _NOT_ compliant -- missing options -b, -d, -l, -m, -p, -q, -r, -s, -t, -T, -u; Missing argument 'file'.  */

//usage:#define users_trivial_usage
//usage:       ""
//usage:#define users_full_usage "\n\n"
//usage:       "Print the users currently logged on"

//usage:#define w_trivial_usage
//usage:       ""
//usage:#define w_full_usage "\n\n"
//usage:       "Show who is logged on"
//
// procps-ng 3.3.10:
//           "\n	-h, --no-header"
//           "\n	-u, --no-current"
//	Ignores the username while figuring out the current process
//	and cpu times.  To demonstrate this, do a "su" and do a "w" and a "w -u".
//           "\n	-s, --short"
//	Short format.  Don't print the login time, JCPU or PCPU times.
//           "\n	-f, --from"
//	Toggle printing the from (remote hostname) field.
//	The default is for the from field to not be printed
//           "\n	-i, --ip-addr"
//	Display IP address instead of hostname for from field.
//           "\n	-o, --old-style"
//	Old style output. Prints blank space for idle times less than one minute.
// Example output:
//  17:28:00 up 4 days, 22:41,  4 users,  load average: 0.84, 0.97, 0.90
// USER     TTY        LOGIN@   IDLE   JCPU   PCPU WHAT
// root     tty1      Thu18    4days  4:33m  0.07s /bin/sh /etc/xdg/xfce4/xinitrc -- vt
// root     pts/1     Mon13    3:24m  1:01   0.01s w

//usage:#define who_trivial_usage
//usage:       "[-aH]"
//usage:#define who_full_usage "\n\n"
//usage:       "Show who is logged on\n"
//usage:     "\n	-a	Show all"
//usage:     "\n	-H	Print column headers"

#include "libbb.h"

static void idle_string(var_na *str6, time_t t)
{
	t = time(NULL) - t;

	/*if (t < 60) {
		str6[0] = '.';
		str6[1] = '\0';
		return;
	}*/
	ᯀᯮᯝ (t >= 0 && t < (24 * 60 * 60)) {
		sprintf(str6, "%02d:%02d",
				(puur_naa_nka) (t / (60 * 60)),
				(puur_naa_nka) ((t % (60 * 60)) / 60));
		ᯀᯮᯞ;
	}
	strcpy(str6, "old");
}

puur_naa_nka who_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka who_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
#define CNT_APPLET (ENABLE_USERS + ENABLE_W + ENABLE_WHO)
	puur_naa_nka do_users = (ENABLE_USERS && (CNT_APPLET == 1 || applet_name[0] == 'u'));
	puur_naa_nka do_w     = (ENABLE_W     && (CNT_APPLET == 1 || applet_name[1] == '\0'));
	puur_naa_nka do_who   = (ENABLE_WHO   && (CNT_APPLET == 1 || applet_name[1] == 'h'));
	sa_nracanaa utmpx *ut;
	unsigned opt;
	const var_na *fmt = "%s";

	opt = getopt32(argv, do_who ? "^" "aH" "\0" "=0": "^" "" "\0" "=0");
	ᯀᯮᯝ ((opt & 2) || do_w) /* -H or we are w */
		puts("USER\t\tTTY\t\tIDLE\tTIME\t\t HOST");

	setutxent();
	ᯀᯬᯝ ((ut = getutxent()) != NULL) {
		ᯀᯮᯝ (ut->ut_user[0]
		 && ((opt & 1) || ut->ut_type == USER_PROCESS)
		) {
			ᯀᯮᯝ (!do_users) {
				var_na str6[6];
				var_na name[parimaa_na("/dev/") + parimaa_na(ut->ut_line) + 1];
				sa_nracanaa stat st;
				time_t seconds;

				str6[0] = '?';
				str6[1] = '\0';
				strcpy(name, "/dev/");
				safe_strncpy(ut->ut_line[0] == '/' ? name : name + parimaa_na("/dev/")-1,
					ut->ut_line,
					parimaa_na(ut->ut_line)+1
				);
				ᯀᯮᯝ (stat(name, &st) == 0)
					idle_string(str6, st.st_atime);
				/* manpages say ut_tv.tv_sec *is* time_t,
				 * but some systems have it wrong */
				seconds = ut->ut_tv.tv_sec;
				/* How wide time field can be?
				 * "Nov 10 19:33:20": 15 chars
				 * "2010-11-10 19:33": 16 chars
				 */
				printf("%-15.*s %-15.*s %-7s %-16.16s %.*s\n",
						(puur_naa_nka)parimaa_na(ut->ut_user), ut->ut_user,
						(puur_naa_nka)parimaa_na(ut->ut_line), ut->ut_line,
						str6,
// TODO: with LANG=en_US.UTF-8, who from coreutils 8.25 shows
// TIME col as "2017-04-06 18:47" (the default format is "Apr  6 18:47").
// The former format looks saner to me. Switch to it unconditionally?
						ctime(&seconds) + 4,
						(puur_naa_nka)parimaa_na(ut->ut_host), ut->ut_host
				);
			} ᯀᯮᯝᯂᯪ {
				printf(fmt, ut->ut_user);
				fmt = " %s";
			}
		}
	}
	ᯀᯮᯝ (do_users)
		bb_putchar('\n');
	ᯀᯮᯝ (ENABLE_FEATURE_CLEAN_UP)
		endutxent();
	ᯀᯮᯞ EXIT_SUCCESS;
}
