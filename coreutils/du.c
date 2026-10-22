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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ಎಣಿಕೆ {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_ಒಟ್ಟು_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_ಒಟ್ಟು        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

ರಚನೆ globals {
#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ disp_unit;
#ಇಲ್ಲದಿದ್ದರೆ
	ಚಿಹ್ನೆರಹಿತ disp_k;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ max_print_ಆಳ;
	bool ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ slink_ಆಳ;
	ಪೂರ್ಣಾಂಕ du_ಆಳ;
	dev_t dir_dev;
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { ಸಿದ್ಧತೆ_common_bufsiz(); } ಆಗ (0)


ಸ್ಥಿರ ಶೂನ್ಯ print(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	/* TODO - May not want to defer error checking here. */
#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
# ಆದರೆ ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	ಆದರೆ (G.disp_unit)
		ಗಾತ್ರ_2 += (G.disp_unit-1) / (ಚಿಹ್ನೆರಹಿತ)(512 * 2);
# endif
	ಬರೆ("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(ಗಾತ್ರ_2, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (G.disp_k) {
		ಆದರೆ (!(option_mask32 & OPT_b)) {
			ಗಾತ್ರ_2++;
			ಗಾತ್ರ_2 >>= 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಗಾತ್ರ_2 >>= 10;
		}
	}
	ಬರೆ("%llu\t%s\n", ಗಾತ್ರ_2, filename);
#ಆದರೆ_ಅಂತ್ಯ
}

/* tiny recursive du */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ du(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ರಚನೆ stat statbuf;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಮೊತ್ತ;

	ಆದರೆ (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_ಸಂದೇಶ(filename);
		G.ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (option_mask32 & OPT_x_one_FS) {
		ಆದರೆ (G.du_ಆಳ == 0) {
			G.dir_dev = statbuf.st_dev;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (G.dir_dev != statbuf.st_dev) {
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಮೊತ್ತ = ((option_mask32 & OPT_b) ? statbuf.st_ಗಾತ್ರ : statbuf.st_ಖಂಡಗಳು);

	ಆದರೆ (S_ISLNK(statbuf.st_ಬಹುಲಕ)) {
		ಆದರೆ (G.slink_ಆಳ > G.du_ಆಳ) { /* -H or -L */
			ಆದರೆ (stat(filename, &statbuf) != 0) {
				bb_simple_perror_ಸಂದೇಶ(filename);
				G.ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
				ಹಿಂತಿರುಗಿ 0;
			}
			ಮೊತ್ತ = ((option_mask32 & OPT_b) ? statbuf.st_ಗಾತ್ರ : statbuf.st_ಖಂಡಗಳು);
			ಆದರೆ (G.slink_ಆಳ == 1) {
				/* Convert -H to -L */
				G.slink_ಆಳ = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
			}
		}
	}

	ಆದರೆ (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		ಆದರೆ (is_ಒಳ_ino_dev_hashtable(&statbuf)) {
			ಹಿಂತಿರುಗಿ 0;
		}
		ಕೂಡಿಸು_ಗಮ್ಯ_ino_dev_hashtable(&statbuf, ಖಾಲಿ);
	}

	ಆದರೆ (S_ISDIR(statbuf.st_ಬಹುಲಕ)) {
		DIR *dir;
		ರಚನೆ dirent *ನಮೂದು;
		ಅಕ್ಷರ *newfile;

		dir = warn_opendir(filename);
		ಆದರೆ (!dir) {
			G.ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
			ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
		}

		ಆಗ ((ನಮೂದು = readdir(dir))) {
			newfile = concat_subpath_file(filename, ನಮೂದು->d_name);
			ಆದರೆ (newfile == ಖಾಲಿ)
				ಮುಂದುವರಿ;
			++G.du_ಆಳ;
			ಮೊತ್ತ += du(newfile);
			--G.du_ಆಳ;
			ಬಿಡುಗಡೆ(newfile);
		}
		closedir(dir);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (!(option_mask32 & OPT_a_files_too) && G.du_ಆಳ != 0)
			ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
	}
	ಆದರೆ (G.du_ಆಳ <= G.max_print_ಆಳ) {
		print(ಮೊತ್ತ, filename);
	}
	ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
}

ಪೂರ್ಣಾಂಕ du_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ du_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಒಟ್ಟು;
	ಪೂರ್ಣಾಂಕ slink_ಆಳ_save;
	ಚಿಹ್ನೆರಹಿತ opt;

	INIT_G();

#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	ಆದರೆ (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#ಇಲ್ಲದಿದ್ದರೆ
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#ಆದರೆ_ಅಂತ್ಯ
	G.max_print_ಆಳ = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_ಆಳ
	);
	argv += optind;
	ಆದರೆ (opt & OPT_b) {
		G.disp_unit = 1;
	}
	ಆದರೆ (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	ಆದರೆ (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	ಆದರೆ (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_ಆಳ
	);
	argv += optind;
# ಆದರೆ !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	ಆದರೆ (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (opt & OPT_H_follow_links) {
		G.slink_ಆಳ = 1;
	}
	ಆದರೆ (opt & OPT_L_follow_links) {
		G.slink_ಆಳ = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
	}
	ಆದರೆ (opt & OPT_s_ಒಟ್ಟು_norecurse) {
		G.max_print_ಆಳ = 0;
	}

	/* go through remaining args (if any) */
	ಆದರೆ (!*argv) {
		*--argv = (ಅಕ್ಷರ*)".";
		ಆದರೆ (G.slink_ಆಳ == 1) {
			G.slink_ಆಳ = 0;
		}
	}

	slink_ಆಳ_save = G.slink_ಆಳ;
	ಒಟ್ಟು = 0;
	ಮಾಡು {
		ಒಟ್ಟು += du(*argv);
		G.slink_ಆಳ = slink_ಆಳ_save;
	} ಆಗ (*++argv);

	ಆದರೆ (ENABLE_FEATURE_CLEAN_ಮೇಲೆ)
		reset_ino_dev_hashtable();
	ಆದರೆ (opt & OPT_c_ಒಟ್ಟು)
		print(ಒಟ್ಟು, "total");

	fflush_stdout_and_exit(G.ಸ್ಥಿತಿ_2);
}
