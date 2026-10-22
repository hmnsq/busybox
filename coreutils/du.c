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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

గణన {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_మొత్తం_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_మొత్తం        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

నిర్మాణం globals {
#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
	గుర్తులేని దీర్ఘ disp_unit;
#లేకపోతే
	గుర్తులేని disp_k;
#అయితే_ముగింపు
	పూర్ణాంకం max_print_లోతు;
	bool స్థితి_2;
	పూర్ణాంకం slink_లోతు;
	పూర్ణాంకం du_లోతు;
	dev_t dir_dev;
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { అమరిక_common_bufsiz(); } అయితే (0)


స్థిర శూన్య print(గుర్తులేని దీర్ఘ దీర్ఘ కొలత, స్థిరరాశి అక్షరం *filename)
{
	/* TODO - May not want to defer error checking here. */
#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
# ఒకవేళ ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	ఒకవేళ (G.disp_unit)
		కొలత += (G.disp_unit-1) / (గుర్తులేని)(512 * 2);
# endif
	రాయి("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(కొలత, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#లేకపోతే
	ఒకవేళ (G.disp_k) {
		ఒకవేళ (!(option_mask32 & OPT_b)) {
			కొలత++;
			కొలత >>= 1;
		} లేకపోతే {
			కొలత >>= 10;
		}
	}
	రాయి("%llu\t%s\n", కొలత, filename);
#అయితే_ముగింపు
}

/* tiny recursive du */
స్థిర గుర్తులేని దీర్ఘ దీర్ఘ du(స్థిరరాశి అక్షరం *filename)
{
	నిర్మాణం stat statbuf;
	గుర్తులేని దీర్ఘ దీర్ఘ కూడిక;

	ఒకవేళ (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_సందేశం(filename);
		G.స్థితి_2 = EXIT_FAILURE;
		తిరిగి 0;
	}

	ఒకవేళ (option_mask32 & OPT_x_one_FS) {
		ఒకవేళ (G.du_లోతు == 0) {
			G.dir_dev = statbuf.st_dev;
		} లేకపోతే ఒకవేళ (G.dir_dev != statbuf.st_dev) {
			తిరిగి 0;
		}
	}

	కూడిక = ((option_mask32 & OPT_b) ? statbuf.st_కొలత : statbuf.st_ఖండాలు);

	ఒకవేళ (S_ISLNK(statbuf.st_బహుళకం)) {
		ఒకవేళ (G.slink_లోతు > G.du_లోతు) { /* -H or -L */
			ఒకవేళ (stat(filename, &statbuf) != 0) {
				bb_simple_perror_సందేశం(filename);
				G.స్థితి_2 = EXIT_FAILURE;
				తిరిగి 0;
			}
			కూడిక = ((option_mask32 & OPT_b) ? statbuf.st_కొలత : statbuf.st_ఖండాలు);
			ఒకవేళ (G.slink_లోతు == 1) {
				/* Convert -H to -L */
				G.slink_లోతు = గరిష్ఠ_పూర్ణాంకం;
			}
		}
	}

	ఒకవేళ (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		ఒకవేళ (is_లోపల_ino_dev_hashtable(&statbuf)) {
			తిరిగి 0;
		}
		కూడు_గమ్యం_ino_dev_hashtable(&statbuf, ఖాళీ);
	}

	ఒకవేళ (S_ISDIR(statbuf.st_బహుళకం)) {
		DIR *dir;
		నిర్మాణం dirent *నమోదు_2;
		అక్షరం *newfile;

		dir = warn_opendir(filename);
		ఒకవేళ (!dir) {
			G.స్థితి_2 = EXIT_FAILURE;
			తిరిగి కూడిక;
		}

		అయితే ((నమోదు_2 = readdir(dir))) {
			newfile = concat_subpath_file(filename, నమోదు_2->d_name);
			ఒకవేళ (newfile == ఖాళీ)
				కొనసాగు;
			++G.du_లోతు;
			కూడిక += du(newfile);
			--G.du_లోతు;
			విడుదల(newfile);
		}
		closedir(dir);
	} లేకపోతే {
		ఒకవేళ (!(option_mask32 & OPT_a_files_too) && G.du_లోతు != 0)
			తిరిగి కూడిక;
	}
	ఒకవేళ (G.du_లోతు <= G.max_print_లోతు) {
		print(కూడిక, filename);
	}
	తిరిగి కూడిక;
}

పూర్ణాంకం du_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం du_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని దీర్ఘ దీర్ఘ మొత్తం;
	పూర్ణాంకం slink_లోతు_save;
	గుర్తులేని opt;

	INIT_G();

#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	ఒకవేళ (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#లేకపోతే
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#అయితే_ముగింపు
	G.max_print_లోతు = గరిష్ఠ_పూర్ణాంకం;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_లోతు
	);
	argv += optind;
	ఒకవేళ (opt & OPT_b) {
		G.disp_unit = 1;
	}
	ఒకవేళ (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	ఒకవేళ (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	ఒకవేళ (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#లేకపోతే
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_లోతు
	);
	argv += optind;
# ఒకవేళ !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	ఒకవేళ (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#అయితే_ముగింపు
	ఒకవేళ (opt & OPT_H_follow_links) {
		G.slink_లోతు = 1;
	}
	ఒకవేళ (opt & OPT_L_follow_links) {
		G.slink_లోతు = గరిష్ఠ_పూర్ణాంకం;
	}
	ఒకవేళ (opt & OPT_s_మొత్తం_norecurse) {
		G.max_print_లోతు = 0;
	}

	/* go through remaining args (if any) */
	ఒకవేళ (!*argv) {
		*--argv = (అక్షరం*)".";
		ఒకవేళ (G.slink_లోతు == 1) {
			G.slink_లోతు = 0;
		}
	}

	slink_లోతు_save = G.slink_లోతు;
	మొత్తం = 0;
	చేయి {
		మొత్తం += du(*argv);
		G.slink_లోతు = slink_లోతు_save;
	} అయితే (*++argv);

	ఒకవేళ (ENABLE_FEATURE_CLEAN_పైకి)
		reset_ino_dev_hashtable();
	ఒకవేళ (opt & OPT_c_మొత్తం)
		print(మొత్తం, "total");

	fflush_stdout_and_exit(G.స్థితి_2);
}
