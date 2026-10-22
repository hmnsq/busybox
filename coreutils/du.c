/* vi: set sw=4 ts=4: */
/*
 * Mini du implementation for busybox
 *
 * Copyright (C) 1999,2000,2001 by Lineo, inc. and John Beppu
 * Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
 * Copyright (C) 2002  Edward Betts <edward@debian.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Mostly rewritten for SUSv3 compliance and to fix bugs/defects.
 * 1) Added support for SUSv3 -a, -H, -L, gnu -c, and (busybox) -d options.
 *    The -d option allows setting of max depth (similar to gnu --max-depth).
 * 2) Fixed incorrect size calculations for links and directories, especially
 *    when errors occurred.  Calculates sizes should now match gnu du output.
 * 3) Added error checking of output.
 * 4) Fixed busybox bug #1284 involving long overflow with human_readable.
 */
//config:config DU
//config:	bool "du (6.5 kb)"
//config:	default y
//config:	help
//config:	du is used to report the amount of disk space used
//config:	for specified files.
//config:
//config:config FEATURE_DU_DEFAULT_BLOCKSIZE_1K
//config:	bool "Use default blocksize of 1024 bytes (else it's 512 bytes)"
//config:	default y
//config:	depends on DU

//applet:IF_DU(APPLET(du, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_DU) += du.o

/* BB_AUDIT SUSv3 compliant (unless default blocksize set to 1k) */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/du.html */

//usage:#define du_trivial_usage
//usage:       "[-aHLdclsx" IF_FEATURE_HUMAN_READABLE("hm") "k] [FILE]..."
//usage:#define du_full_usage "\n\n"
//usage:       "Summarize disk space used for FILEs (or directories)\n"
//usage:     "\n	-a	Show file sizes too"
//usage:     "\n	-b	Apparent size (including holes)"
//usage:     "\n	-L	Follow all symlinks"
//usage:     "\n	-H	Follow symlinks on command line"
//usage:     "\n	-d N	Limit output to directories (and files with -a) of depth < N"
//usage:     "\n	-c	Show grand total"
//usage:     "\n	-l	Count sizes many times if hard linked"
//usage:     "\n	-s	Display only a total for each argument"
//usage:     "\n	-x	Skip directories on different filesystems"
//usage:	IF_FEATURE_HUMAN_READABLE(
//usage:     "\n	-h	Sizes in human readable format (e.g., 1K 243M 2G)"
//usage:     "\n	-m	Sizes in megabytes"
//usage:	)
//usage:     "\n	-k	Sizes in kilobytes" IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(" (default)")
//usage:	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(
//usage:     "\n		Default unit is 512 bytes"
//usage:	)
//usage:
//usage:#define du_example_usage
//usage:       "$ du\n"
//usage:       "16      ./CVS\n"
//usage:       "12      ./kernel-patches/CVS\n"
//usage:       "80      ./kernel-patches\n"
//usage:       "12      ./tests/CVS\n"
//usage:       "36      ./tests\n"
//usage:       "12      ./scripts/CVS\n"
//usage:       "16      ./scripts\n"
//usage:       "12      ./docs/CVS\n"
//usage:       "104     ./docs\n"
//usage:       "2417    .\n"

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

تعداد {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_ټول_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_ټول        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

بنية globals {
#إذا ENABLE_FEATURE_HUMAN_READABLE
	بینښه اوږد disp_unit;
#وإلا
	بینښه disp_k;
#د_که_پای
	صحيح max_print_ژوروالی;
	bool حالت;
	صحيح slink_ژوروالی;
	صحيح du_ژوروالی;
	dev_t dir_dev;
} FIX_ALIASING;
#تعریف_کړه G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کړه INIT_G() افعل { تیاری_common_bufsiz(); } طالما (0)


ساكن فراغ print(بینښه اوږد اوږد اندازه, ثابت حرف *filename)
{
	/* TODO - May not want to defer error checking here. */
#إذا ENABLE_FEATURE_HUMAN_READABLE
# إذا ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	إذا (G.disp_unit)
		اندازه += (G.disp_unit-1) / (بینښه)(512 * 2);
# endif
	چاپ_کړه("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(اندازه, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#وإلا
	إذا (G.disp_k) {
		إذا (!(option_mask32 & OPT_b)) {
			اندازه++;
			اندازه >>= 1;
		} وإلا {
			اندازه >>= 10;
		}
	}
	چاپ_کړه("%llu\t%s\n", اندازه, filename);
#د_که_پای
}

/* tiny recursive du */
ساكن بینښه اوږد اوږد du(ثابت حرف *filename)
{
	بنية stat statbuf;
	بینښه اوږد اوږد مجموعه;

	إذا (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_پیغام(filename);
		G.حالت = EXIT_FAILURE;
		أرجع 0;
	}

	إذا (option_mask32 & OPT_x_one_FS) {
		إذا (G.du_ژوروالی == 0) {
			G.dir_dev = statbuf.st_dev;
		} وإلا إذا (G.dir_dev != statbuf.st_dev) {
			أرجع 0;
		}
	}

	مجموعه = ((option_mask32 & OPT_b) ? statbuf.st_اندازه : statbuf.st_بلاکونه);

	إذا (S_ISLNK(statbuf.st_ډېرپېښ)) {
		إذا (G.slink_ژوروالی > G.du_ژوروالی) { /* -H or -L */
			إذا (stat(filename, &statbuf) != 0) {
				bb_simple_perror_پیغام(filename);
				G.حالت = EXIT_FAILURE;
				أرجع 0;
			}
			مجموعه = ((option_mask32 & OPT_b) ? statbuf.st_اندازه : statbuf.st_بلاکونه);
			إذا (G.slink_ژوروالی == 1) {
				/* Convert -H to -L */
				G.slink_ژوروالی = لوی_عدد;
			}
		}
	}

	إذا (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		إذا (is_دننه_ino_dev_hashtable(&statbuf)) {
			أرجع 0;
		}
		جمع_کړه_منزل_ino_dev_hashtable(&statbuf, عدم);
	}

	إذا (S_ISDIR(statbuf.st_ډېرپېښ)) {
		DIR *dir;
		بنية dirent *ثبت;
		حرف *newfile;

		dir = warn_opendir(filename);
		إذا (!dir) {
			G.حالت = EXIT_FAILURE;
			أرجع مجموعه;
		}

		طالما ((ثبت = readdir(dir))) {
			newfile = concat_subpath_file(filename, ثبت->d_name);
			إذا (newfile == عدم)
				استمر;
			++G.du_ژوروالی;
			مجموعه += du(newfile);
			--G.du_ژوروالی;
			حرر(newfile);
		}
		closedir(dir);
	} وإلا {
		إذا (!(option_mask32 & OPT_a_files_too) && G.du_ژوروالی != 0)
			أرجع مجموعه;
	}
	إذا (G.du_ژوروالی <= G.max_print_ژوروالی) {
		print(مجموعه, filename);
	}
	أرجع مجموعه;
}

صحيح du_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح du_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	بینښه اوږد اوږد ټول;
	صحيح slink_ژوروالی_save;
	بینښه opt;

	INIT_G();

#إذا ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	إذا (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#وإلا
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#د_که_پای
	G.max_print_ژوروالی = لوی_عدد;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#إذا ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_ژوروالی
	);
	argv += optind;
	إذا (opt & OPT_b) {
		G.disp_unit = 1;
	}
	إذا (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	إذا (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	إذا (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#وإلا
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_ژوروالی
	);
	argv += optind;
# إذا !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	إذا (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#د_که_پای
	إذا (opt & OPT_H_follow_links) {
		G.slink_ژوروالی = 1;
	}
	إذا (opt & OPT_L_follow_links) {
		G.slink_ژوروالی = لوی_عدد;
	}
	إذا (opt & OPT_s_ټول_norecurse) {
		G.max_print_ژوروالی = 0;
	}

	/* go through remaining args (if any) */
	إذا (!*argv) {
		*--argv = (حرف*)".";
		إذا (G.slink_ژوروالی == 1) {
			G.slink_ژوروالی = 0;
		}
	}

	slink_ژوروالی_save = G.slink_ژوروالی;
	ټول = 0;
	افعل {
		ټول += du(*argv);
		G.slink_ژوروالی = slink_ژوروالی_save;
	} طالما (*++argv);

	إذا (ENABLE_FEATURE_CLEAN_پورته)
		reset_ino_dev_hashtable();
	إذا (opt & OPT_c_ټول)
		print(ټول, "total");

	fflush_stdout_and_exit(G.حالت);
}
