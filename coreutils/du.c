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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

ਗਿਣਤੀ {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_ਕੁੱਲ_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_ਕੁੱਲ        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

ਬਣਤਰ globals {
#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ disp_unit;
#ਨਹੀਂਤਾਂ
	ਬਿਨਾਨਿਸ਼ਾਨੀ disp_k;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ max_print_ਡੂੰਘਾਈ;
	bool ਸਥਿਤੀ;
	ਪੂਰਨ slink_ਡੂੰਘਾਈ;
	ਪੂਰਨ du_ਡੂੰਘਾਈ;
	dev_t dir_dev;
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { ਤਿਆਰੀ_common_bufsiz(); } ਜਦੋਂ (0)


ਸਥਿਰ ਖਾਲੀ print(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਮਾਪ, ਅਚਲ ਅੱਖਰ *filename)
{
	/* TODO - May not want to defer error checking here. */
#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
# ਜੇ ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	ਜੇ (G.disp_unit)
		ਮਾਪ += (G.disp_unit-1) / (ਬਿਨਾਨਿਸ਼ਾਨੀ)(512 * 2);
# endif
	ਲਿਖੋ("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(ਮਾਪ, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#ਨਹੀਂਤਾਂ
	ਜੇ (G.disp_k) {
		ਜੇ (!(option_mask32 & OPT_b)) {
			ਮਾਪ++;
			ਮਾਪ >>= 1;
		} ਨਹੀਂਤਾਂ {
			ਮਾਪ >>= 10;
		}
	}
	ਲਿਖੋ("%llu\t%s\n", ਮਾਪ, filename);
#ਜੇ_ਅੰਤ
}

/* tiny recursive du */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ du(ਅਚਲ ਅੱਖਰ *filename)
{
	ਬਣਤਰ stat statbuf;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਜੋੜ;

	ਜੇ (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_ਸੁਨੇਹਾ(filename);
		G.ਸਥਿਤੀ = EXIT_FAILURE;
		ਵਾਪਸ 0;
	}

	ਜੇ (option_mask32 & OPT_x_one_FS) {
		ਜੇ (G.du_ਡੂੰਘਾਈ == 0) {
			G.dir_dev = statbuf.st_dev;
		} ਨਹੀਂਤਾਂ ਜੇ (G.dir_dev != statbuf.st_dev) {
			ਵਾਪਸ 0;
		}
	}

	ਜੋੜ = ((option_mask32 & OPT_b) ? statbuf.st_ਮਾਪ : statbuf.st_ਖੰਡਾਂ);

	ਜੇ (S_ISLNK(statbuf.st_ਬਹੁਲਕ)) {
		ਜੇ (G.slink_ਡੂੰਘਾਈ > G.du_ਡੂੰਘਾਈ) { /* -H or -L */
			ਜੇ (stat(filename, &statbuf) != 0) {
				bb_simple_perror_ਸੁਨੇਹਾ(filename);
				G.ਸਥਿਤੀ = EXIT_FAILURE;
				ਵਾਪਸ 0;
			}
			ਜੋੜ = ((option_mask32 & OPT_b) ? statbuf.st_ਮਾਪ : statbuf.st_ਖੰਡਾਂ);
			ਜੇ (G.slink_ਡੂੰਘਾਈ == 1) {
				/* Convert -H to -L */
				G.slink_ਡੂੰਘਾਈ = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
			}
		}
	}

	ਜੇ (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		ਜੇ (is_ਵਿੱਚ_ino_dev_hashtable(&statbuf)) {
			ਵਾਪਸ 0;
		}
		ਜੋੜੋ_ਮੰਜ਼ਿਲ_ino_dev_hashtable(&statbuf, ਨਿਰਾ);
	}

	ਜੇ (S_ISDIR(statbuf.st_ਬਹੁਲਕ)) {
		DIR *dir;
		ਬਣਤਰ dirent *ਇੰਦਰਾਜ;
		ਅੱਖਰ *newfile;

		dir = warn_opendir(filename);
		ਜੇ (!dir) {
			G.ਸਥਿਤੀ = EXIT_FAILURE;
			ਵਾਪਸ ਜੋੜ;
		}

		ਜਦੋਂ ((ਇੰਦਰਾਜ = readdir(dir))) {
			newfile = concat_subpath_file(filename, ਇੰਦਰਾਜ->d_name);
			ਜੇ (newfile == ਨਿਰਾ)
				ਜਾਰੀ;
			++G.du_ਡੂੰਘਾਈ;
			ਜੋੜ += du(newfile);
			--G.du_ਡੂੰਘਾਈ;
			ਮੁਕਤ(newfile);
		}
		closedir(dir);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (!(option_mask32 & OPT_a_files_too) && G.du_ਡੂੰਘਾਈ != 0)
			ਵਾਪਸ ਜੋੜ;
	}
	ਜੇ (G.du_ਡੂੰਘਾਈ <= G.max_print_ਡੂੰਘਾਈ) {
		print(ਜੋੜ, filename);
	}
	ਵਾਪਸ ਜੋੜ;
}

ਪੂਰਨ du_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ du_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਕੁੱਲ;
	ਪੂਰਨ slink_ਡੂੰਘਾਈ_save;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;

	INIT_G();

#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	ਜੇ (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#ਨਹੀਂਤਾਂ
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#ਜੇ_ਅੰਤ
	G.max_print_ਡੂੰਘਾਈ = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_ਡੂੰਘਾਈ
	);
	argv += optind;
	ਜੇ (opt & OPT_b) {
		G.disp_unit = 1;
	}
	ਜੇ (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	ਜੇ (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	ਜੇ (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#ਨਹੀਂਤਾਂ
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_ਡੂੰਘਾਈ
	);
	argv += optind;
# ਜੇ !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	ਜੇ (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#ਜੇ_ਅੰਤ
	ਜੇ (opt & OPT_H_follow_links) {
		G.slink_ਡੂੰਘਾਈ = 1;
	}
	ਜੇ (opt & OPT_L_follow_links) {
		G.slink_ਡੂੰਘਾਈ = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
	}
	ਜੇ (opt & OPT_s_ਕੁੱਲ_norecurse) {
		G.max_print_ਡੂੰਘਾਈ = 0;
	}

	/* go through remaining args (if any) */
	ਜੇ (!*argv) {
		*--argv = (ਅੱਖਰ*)".";
		ਜੇ (G.slink_ਡੂੰਘਾਈ == 1) {
			G.slink_ਡੂੰਘਾਈ = 0;
		}
	}

	slink_ਡੂੰਘਾਈ_save = G.slink_ਡੂੰਘਾਈ;
	ਕੁੱਲ = 0;
	ਕਰੋ {
		ਕੁੱਲ += du(*argv);
		G.slink_ਡੂੰਘਾਈ = slink_ਡੂੰਘਾਈ_save;
	} ਜਦੋਂ (*++argv);

	ਜੇ (ENABLE_FEATURE_CLEAN_ਉੱਪਰ)
		reset_ino_dev_hashtable();
	ਜੇ (opt & OPT_c_ਕੁੱਲ)
		print(ਕੁੱਲ, "total");

	fflush_stdout_and_exit(G.ਸਥਿਤੀ);
}
