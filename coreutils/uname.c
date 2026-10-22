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

#ಅನ್ತರ್ಭಾವ "libbb.h"
/* After libbb.h, since it needs sys/types.h on some systems */
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ರಚನೆ utsname name;
	ಅಕ್ಷರ processor[ಗಾತ್ರ(((ರಚನೆ utsname*)ಖಾಲಿ)->machine)];
	ಅಕ್ಷರ platform[ಗಾತ್ರ(((ರಚನೆ utsname*)ಖಾಲಿ)->machine)];
	ಅಕ್ಷರ os[ಗಾತ್ರ(CONFIG_UNAME_OSNAME)];
} uname_info_t;

#ಆದರೆ ENABLE_UNAME
#ವ್ಯಾಖ್ಯಾನಿಸು options "snrvmpioa"
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ utsname_ಸ್ಥಾನಾಂತರ[] = {
	offsetof(uname_info_t, name.sysname), /* -s */
	offsetof(uname_info_t, name.nodename), /* -n */
	offsetof(uname_info_t, name.release), /* -r */
	offsetof(uname_info_t, name.ಆವೃತ್ತಿ), /* -v */
	offsetof(uname_info_t, name.machine), /* -m */
	offsetof(uname_info_t, processor), /* -p */
	offsetof(uname_info_t, platform), /* -i */
	offsetof(uname_info_t, os), /* -o */
};
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ uname_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ uname_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	uname_info_t uname_info;
	IF_UNAME(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *unknown_str = "unknown";)
#ಆದರೆ ENABLE_UNAME
	ಚಿಹ್ನೆರಹಿತ toprint = (1 << 4); /* "arch" = "uname -m" */

	ಆದರೆ (!ENABLE_BB_ARCH || applet_name[0] == 'u') {
# ಆದರೆ ENABLE_LONG_OPTS
		ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ uname_longopts[] ALIGN1 =
			/* name, has_arg, val */
			"all\0"               No_ವಾದ       "a"
			"kernel-name\0"       No_ವಾದ       "s"
			"nodename\0"          No_ವಾದ       "n"
			"kernel-release\0"    No_ವಾದ       "r"
			"release\0"           No_ವಾದ       "r"
			"kernel-version\0"    No_ವಾದ       "v"
			"machine\0"           No_ವಾದ       "m"
			"processor\0"         No_ವಾದ       "p"
			"hardware-platform\0" No_ವಾದ       "i"
			"operating-system\0"  No_ವಾದ       "o"
		;
# endif
		toprint = getopt32long(argv, options, uname_longopts);
		ಆದರೆ (argv[optind]) { /* coreutils-6.9 compat */
			bb_show_usage();
		}
		ಆದರೆ (toprint & (1 << 8)) { /* -a => all opts on */
			toprint = (1 << 8) - 1;
			unknown_str = ""; /* -a does not print unknown fields */
		}
		ಆದರೆ (toprint == 0) { /* no opts => -s (sysname) */
			toprint = 1;
		}
	} /* if "uname" */
#ಆದರೆ_ಅಂತ್ಯ

	uname(&uname_info.name); /* never fails */

#ಆದರೆ defined(__sparc__) && defined(__linux__)
	{
		ಅಕ್ಷರ *fake_sparc = getenv("FAKE_SPARC");
		ಆದರೆ (fake_sparc && (fake_sparc[0] | 0x20) == 'y') {
			strcpy(uname_info.name.machine, "sparc");
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (ENABLE_BB_ARCH && (!ENABLE_UNAME || applet_name[0] == 'a')) {
		puts(uname_info.name.machine);
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ ENABLE_UNAME
		/* "uname" */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt;
		ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *delta;

		strcpy(uname_info.processor, unknown_str);
		strcpy(uname_info.platform, unknown_str);
		strcpy(uname_info.os, CONFIG_UNAME_OSNAME);
# ಆದರೆ ENABLE_FEDORA_COMPAT
		/* Fedora does something like this */
		strcpy(uname_info.processor, uname_info.name.machine);
		strcpy(uname_info.platform, uname_info.name.machine);
		ಆದರೆ (uname_info.platform[0] == 'i'
		 && uname_info.platform[1]
		 && uname_info.platform[2] == '8'
		 && uname_info.platform[3] == '6'
		) {
			uname_info.platform[1] = '3';
		}
# endif
		delta = utsname_ಸ್ಥಾನಾಂತರ;
		fmt = " %s" + 1;
		ಮಾಡು {
			ಆದರೆ (toprint & 1) {
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = (ಅಕ್ಷರ *)(&uname_info) + *delta;
				ಆದರೆ (p[0]) {
					ಬರೆ(fmt, p);
					fmt = " %s";
				}
			}
			++delta;
		} ಆಗ (toprint >>= 1);
		bb_putchar('\n');
#ಆದರೆ_ಅಂತ್ಯ
	}

	fflush_stdout_and_exit_SUCCESS(); /* coreutils-6.9 compat */
}
