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

#include "libbb.h"
#include "common_bufsiz.h"

ga_nanaa {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_total_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_total        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

sa_nracanaa globals {
#ᰀᰧ ENABLE_FEATURE_HUMAN_READABLE
	unsigned long disp_unit;
#ᰀᰧᰀ
	unsigned disp_k;
#endif
	puur_naa_nka max_print_depth;
	bool status;
	puur_naa_nka slink_depth;
	puur_naa_nka du_depth;
	dev_t dir_dev;
} FIX_ALIASING;
#define G (*(sa_nracanaa globals*)bb_common_bufsiz1)
#define INIT_G() do { setup_common_bufsiz(); } ᰀᰬ (0)


static void print(unsigned long long size, const var_na *filename)
{
	/* TODO - May not want to defer error checking here. */
#ᰀᰧ ENABLE_FEATURE_HUMAN_READABLE
# ᰀᰧ ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	ᰀᰧ (G.disp_unit)
		size += (G.disp_unit-1) / (unsigned)(512 * 2);
# endif
	printf("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(size, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#ᰀᰧᰀ
	ᰀᰧ (G.disp_k) {
		ᰀᰧ (!(option_mask32 & OPT_b)) {
			size++;
			size >>= 1;
		} ᰀᰧᰀ {
			size >>= 10;
		}
	}
	printf("%llu\t%s\n", size, filename);
#endif
}

/* tiny recursive du */
static unsigned long long du(const var_na *filename)
{
	sa_nracanaa stat statbuf;
	unsigned long long sum;

	ᰀᰧ (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_msg(filename);
		G.status = EXIT_FAILURE;
		ᰀᰧᰌ 0;
	}

	ᰀᰧ (option_mask32 & OPT_x_one_FS) {
		ᰀᰧ (G.du_depth == 0) {
			G.dir_dev = statbuf.st_dev;
		} ᰀᰧᰀ ᰀᰧ (G.dir_dev != statbuf.st_dev) {
			ᰀᰧᰌ 0;
		}
	}

	sum = ((option_mask32 & OPT_b) ? statbuf.st_size : statbuf.st_blocks);

	ᰀᰧ (S_ISLNK(statbuf.st_mode)) {
		ᰀᰧ (G.slink_depth > G.du_depth) { /* -H or -L */
			ᰀᰧ (stat(filename, &statbuf) != 0) {
				bb_simple_perror_msg(filename);
				G.status = EXIT_FAILURE;
				ᰀᰧᰌ 0;
			}
			sum = ((option_mask32 & OPT_b) ? statbuf.st_size : statbuf.st_blocks);
			ᰀᰧ (G.slink_depth == 1) {
				/* Convert -H to -L */
				G.slink_depth = INT_MAX;
			}
		}
	}

	ᰀᰧ (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		ᰀᰧ (is_in_ino_dev_hashtable(&statbuf)) {
			ᰀᰧᰌ 0;
		}
		add_to_ino_dev_hashtable(&statbuf, NULL);
	}

	ᰀᰧ (S_ISDIR(statbuf.st_mode)) {
		DIR *dir;
		sa_nracanaa dirent *entry;
		var_na *newfile;

		dir = warn_opendir(filename);
		ᰀᰧ (!dir) {
			G.status = EXIT_FAILURE;
			ᰀᰧᰌ sum;
		}

		ᰀᰬ ((entry = readdir(dir))) {
			newfile = concat_subpath_file(filename, entry->d_name);
			ᰀᰧ (newfile == NULL)
				continue;
			++G.du_depth;
			sum += du(newfile);
			--G.du_depth;
			free(newfile);
		}
		closedir(dir);
	} ᰀᰧᰀ {
		ᰀᰧ (!(option_mask32 & OPT_a_files_too) && G.du_depth != 0)
			ᰀᰧᰌ sum;
	}
	ᰀᰧ (G.du_depth <= G.max_print_depth) {
		print(sum, filename);
	}
	ᰀᰧᰌ sum;
}

puur_naa_nka du_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka du_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	unsigned long long total;
	puur_naa_nka slink_depth_save;
	unsigned opt;

	INIT_G();

#ᰀᰧ ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	ᰀᰧ (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#ᰀᰧᰀ
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#endif
	G.max_print_depth = INT_MAX;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#ᰀᰧ ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
	ᰀᰧ (opt & OPT_b) {
		G.disp_unit = 1;
	}
	ᰀᰧ (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	ᰀᰧ (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	ᰀᰧ (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#ᰀᰧᰀ
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
# ᰀᰧ !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	ᰀᰧ (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#endif
	ᰀᰧ (opt & OPT_H_follow_links) {
		G.slink_depth = 1;
	}
	ᰀᰧ (opt & OPT_L_follow_links) {
		G.slink_depth = INT_MAX;
	}
	ᰀᰧ (opt & OPT_s_total_norecurse) {
		G.max_print_depth = 0;
	}

	/* go through remaining args (if any) */
	ᰀᰧ (!*argv) {
		*--argv = (var_na*)".";
		ᰀᰧ (G.slink_depth == 1) {
			G.slink_depth = 0;
		}
	}

	slink_depth_save = G.slink_depth;
	total = 0;
	do {
		total += du(*argv);
		G.slink_depth = slink_depth_save;
	} ᰀᰬ (*++argv);

	ᰀᰧ (ENABLE_FEATURE_CLEAN_UP)
		reset_ino_dev_hashtable();
	ᰀᰧ (opt & OPT_c_total)
		print(total, "total");

	fflush_stdout_and_exit(G.status);
}
