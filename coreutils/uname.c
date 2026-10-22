/* vi: set sw=4 ts=4: */
/*
 * uname -- print system information
 * Copyright (C) 1989-1999 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Option		Example
 * -s, --sysname	SunOS
 * -n, --nodename	rocky8
 * -r, --release	4.0
 * -v, --version
 * -m, --machine	sun
 * -a, --all		SunOS rocky8 4.0  sun
 *
 * The default behavior is equivalent to '-s'.
 *
 * David MacKenzie <djm@gnu.ai.mit.edu>
 *
 * GNU coreutils 6.10:
 * Option:                      struct   Example(s):
 *                              utsname
 *                              field:
 * -s, --kernel-name            sysname  Linux
 * -n, --nodename               nodename localhost.localdomain
 * -r, --kernel-release         release  2.6.29
 * -v, --kernel-version         version  #1 SMP Sun Jan 11 20:52:37 EST 2009
 * -m, --machine                machine  x86_64   i686
 * -p, --processor              (none)   x86_64   i686
 * -i, --hardware-platform      (none)   x86_64   i386
 *      NB: vanilla coreutils reports "unknown" -p and -i,
 *      x86_64 and i686/i386 shown above are Fedora's inventions.
 * -o, --operating-system       (none)   GNU/Linux
 * -a, --all: all of the above, in the order shown.
 *      If -p or -i is not known, don't show them
 */
/* Busyboxed by Erik Andersen
 *
 * Before 2003: Glenn McGrath and Manuel Novoa III
 *  Further size reductions.
 * Mar 16, 2003: Manuel Novoa III (mjn3@codepoet.org)
 *  Now does proper error checking on i/o.  Plus some further space savings.
 * Jan 2009:
 *  Fix handling of -a to not print "unknown", add -o and -i support.
 */
//config:config UNAME
//config:	bool "uname (4.2 kb)"
//config:	default y
//config:	help
//config:	uname is used to print system information.
//config:
//config:config UNAME_OSNAME
//config:	string "Operating system name"
//config:	default "GNU/Linux"
//config:	depends on UNAME
//config:	help
//config:	Sets the operating system name reported by uname -o.  The
//config:	default is "GNU/Linux".
//config:
//can't use "ARCH" for this applet, all hell breaks loose in build system :)
//config:config BB_ARCH
//config:	bool "arch (1.4 kb)"
//config:	default y
//config:	help
//config:	Same as uname -m.

//                  APPLET_NOFORK:name   main   location    suid_type     help
//applet:IF_UNAME(APPLET_NOFORK(  uname, uname, BB_DIR_BIN, BB_SUID_DROP, uname))
//applet:IF_BB_ARCH(APPLET_NOFORK(arch,  uname, BB_DIR_BIN, BB_SUID_DROP, arch))

//kbuild:lib-$(CONFIG_UNAME)   += uname.o
//kbuild:lib-$(CONFIG_BB_ARCH) += uname.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/uname.html */

//usage:#define uname_trivial_usage
//usage:       "[-amnrspvio]"
//usage:#define uname_full_usage "\n\n"
//usage:       "Print system information\n"
//usage:     "\n	-a	Print all"
//usage:     "\n	-m	Machine (hardware) type"
//usage:     "\n	-n	Hostname"
//usage:     "\n	-r	Kernel release"
//usage:     "\n	-s	Kernel name (default)"
//usage:     "\n	-p	Processor type"
//usage:     "\n	-v	Kernel version"
//usage:     "\n	-i	Hardware platform"
//usage:     "\n	-o	OS name"
//usage:
//usage:#define uname_example_usage
//usage:       "$ uname -a\n"
//usage:       "Linux debian 2.4.23 #2 Tue Dec 23 17:09:10 MST 2003 i686 GNU/Linux\n"

//usage:#define arch_trivial_usage
//usage:       ""
//usage:#define arch_full_usage "\n\n"
//usage:       "Print system architecture"

#అన్తర్భావ "libbb.h"
/* After libbb.h, since it needs sys/types.h on some systems */
#అన్తర్భావ <sys/utsname.h>

రకనిర్వచనం నిర్మాణం {
	నిర్మాణం utsname name;
	అక్షరం processor[పరిమాణం(((నిర్మాణం utsname*)ఖాళీ)->machine)];
	అక్షరం platform[పరిమాణం(((నిర్మాణం utsname*)ఖాళీ)->machine)];
	అక్షరం os[పరిమాణం(CONFIG_UNAME_OSNAME)];
} uname_info_t;

#ఒకవేళ ENABLE_UNAME
#నిర్వచించు options "snrvmpioa"
స్థిర స్థిరరాశి గుర్తులేని పొట్టి utsname_స్థానభ్రంశం[] = {
	offsetof(uname_info_t, name.sysname), /* -s */
	offsetof(uname_info_t, name.nodename), /* -n */
	offsetof(uname_info_t, name.release), /* -r */
	offsetof(uname_info_t, name.సంస్కరణ), /* -v */
	offsetof(uname_info_t, name.machine), /* -m */
	offsetof(uname_info_t, processor), /* -p */
	offsetof(uname_info_t, platform), /* -i */
	offsetof(uname_info_t, os), /* -o */
};
#అయితే_ముగింపు

పూర్ణాంకం uname_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం uname_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv వాడని_PARAM)
{
	uname_info_t uname_info;
	IF_UNAME(స్థిరరాశి అక్షరం *unknown_str = "unknown";)
#ఒకవేళ ENABLE_UNAME
	గుర్తులేని toprint = (1 << 4); /* "arch" = "uname -m" */

	ఒకవేళ (!ENABLE_BB_ARCH || applet_name[0] == 'u') {
# ఒకవేళ ENABLE_LONG_OPTS
		స్థిర స్థిరరాశి అక్షరం uname_longopts[] ALIGN1 =
			/* name, has_arg, val */
			"all\0"               No_వాదం       "a"
			"kernel-name\0"       No_వాదం       "s"
			"nodename\0"          No_వాదం       "n"
			"kernel-release\0"    No_వాదం       "r"
			"release\0"           No_వాదం       "r"
			"kernel-version\0"    No_వాదం       "v"
			"machine\0"           No_వాదం       "m"
			"processor\0"         No_వాదం       "p"
			"hardware-platform\0" No_వాదం       "i"
			"operating-system\0"  No_వాదం       "o"
		;
# endif
		toprint = getopt32long(argv, options, uname_longopts);
		ఒకవేళ (argv[optind]) { /* coreutils-6.9 compat */
			bb_show_usage();
		}
		ఒకవేళ (toprint & (1 << 8)) { /* -a => all opts on */
			toprint = (1 << 8) - 1;
			unknown_str = ""; /* -a does not print unknown fields */
		}
		ఒకవేళ (toprint == 0) { /* no opts => -s (sysname) */
			toprint = 1;
		}
	} /* if "uname" */
#అయితే_ముగింపు

	uname(&uname_info.name); /* never fails */

#ఒకవేళ defined(__sparc__) && defined(__linux__)
	{
		అక్షరం *fake_sparc = getenv("FAKE_SPARC");
		ఒకవేళ (fake_sparc && (fake_sparc[0] | 0x20) == 'y') {
			strcpy(uname_info.name.machine, "sparc");
		}
	}
#అయితే_ముగింపు
	ఒకవేళ (ENABLE_BB_ARCH && (!ENABLE_UNAME || applet_name[0] == 'a')) {
		puts(uname_info.name.machine);
	} లేకపోతే {
#ఒకవేళ ENABLE_UNAME
		/* "uname" */
		స్థిరరాశి అక్షరం *fmt;
		స్థిరరాశి గుర్తులేని పొట్టి *delta;

		strcpy(uname_info.processor, unknown_str);
		strcpy(uname_info.platform, unknown_str);
		strcpy(uname_info.os, CONFIG_UNAME_OSNAME);
# ఒకవేళ ENABLE_FEDORA_COMPAT
		/* Fedora does something like this */
		strcpy(uname_info.processor, uname_info.name.machine);
		strcpy(uname_info.platform, uname_info.name.machine);
		ఒకవేళ (uname_info.platform[0] == 'i'
		 && uname_info.platform[1]
		 && uname_info.platform[2] == '8'
		 && uname_info.platform[3] == '6'
		) {
			uname_info.platform[1] = '3';
		}
# endif
		delta = utsname_స్థానభ్రంశం;
		fmt = " %s" + 1;
		చేయి {
			ఒకవేళ (toprint & 1) {
				స్థిరరాశి అక్షరం *p = (అక్షరం *)(&uname_info) + *delta;
				ఒకవేళ (p[0]) {
					రాయి(fmt, p);
					fmt = " %s";
				}
			}
			++delta;
		} అయితే (toprint >>= 1);
		bb_putchar('\n');
#అయితే_ముగింపు
	}

	fflush_stdout_and_exit_SUCCESS(); /* coreutils-6.9 compat */
}
