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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

গণনা {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_যোগ_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_যোগ        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

কাঠামো globals {
#যদি ENABLE_FEATURE_HUMAN_READABLE
	unsigned long disp_unit;
#অন্যথা
	unsigned disp_k;
#endif
	পূর্ণসংখ্যা max_print_depth;
	bool status;
	পূর্ণসংখ্যা slink_depth;
	পূর্ণসংখ্যা du_depth;
	dev_t dir_dev;
} FIX_ALIASING;
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { setup_common_bufsiz(); } যখন (0)


স্থির শূন্য print(unsigned long long আকার_2, ধ্রুবক অক্ষর *filename)
{
	/* TODO - May not want to defer error checking here. */
#যদি ENABLE_FEATURE_HUMAN_READABLE
# যদি ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	যদি (G.disp_unit)
		আকার_2 += (G.disp_unit-1) / (unsigned)(512 * 2);
# endif
	printf("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(আকার_2, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#অন্যথা
	যদি (G.disp_k) {
		যদি (!(option_mask32 & OPT_b)) {
			আকার_2++;
			আকার_2 >>= 1;
		} অন্যথা {
			আকার_2 >>= 10;
		}
	}
	printf("%llu\t%s\n", আকার_2, filename);
#endif
}

/* tiny recursive du */
স্থির unsigned long long du(ধ্রুবক অক্ষর *filename)
{
	কাঠামো stat statbuf;
	unsigned long long যোগ;

	যদি (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_বার্তা(filename);
		G.status = EXIT_FAILURE;
		ফেরত 0;
	}

	যদি (option_mask32 & OPT_x_one_FS) {
		যদি (G.du_depth == 0) {
			G.dir_dev = statbuf.st_dev;
		} অন্যথা যদি (G.dir_dev != statbuf.st_dev) {
			ফেরত 0;
		}
	}

	যোগ = ((option_mask32 & OPT_b) ? statbuf.st_আকার : statbuf.st_blocks);

	যদি (S_ISLNK(statbuf.st_mode)) {
		যদি (G.slink_depth > G.du_depth) { /* -H or -L */
			যদি (stat(filename, &statbuf) != 0) {
				bb_simple_perror_বার্তা(filename);
				G.status = EXIT_FAILURE;
				ফেরত 0;
			}
			যোগ = ((option_mask32 & OPT_b) ? statbuf.st_আকার : statbuf.st_blocks);
			যদি (G.slink_depth == 1) {
				/* Convert -H to -L */
				G.slink_depth = INT_MAX;
			}
		}
	}

	যদি (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		যদি (is_in_ino_dev_hashtable(&statbuf)) {
			ফেরত 0;
		}
		add_to_ino_dev_hashtable(&statbuf, ফাঁকা);
	}

	যদি (S_ISDIR(statbuf.st_mode)) {
		DIR *dir;
		কাঠামো dirent *entry;
		অক্ষর *newfile;

		dir = warn_opendir(filename);
		যদি (!dir) {
			G.status = EXIT_FAILURE;
			ফেরত যোগ;
		}

		যখন ((entry = readdir(dir))) {
			newfile = concat_subpath_file(filename, entry->d_name);
			যদি (newfile == ফাঁকা)
				চালু;
			++G.du_depth;
			যোগ += du(newfile);
			--G.du_depth;
			মুক্ত(newfile);
		}
		closedir(dir);
	} অন্যথা {
		যদি (!(option_mask32 & OPT_a_files_too) && G.du_depth != 0)
			ফেরত যোগ;
	}
	যদি (G.du_depth <= G.max_print_depth) {
		print(যোগ, filename);
	}
	ফেরত যোগ;
}

পূর্ণসংখ্যা du_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা du_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	unsigned long long যোগ_2;
	পূর্ণসংখ্যা slink_depth_save;
	unsigned opt;

	INIT_G();

#যদি ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	যদি (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#অন্যথা
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
#যদি ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
	যদি (opt & OPT_b) {
		G.disp_unit = 1;
	}
	যদি (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	যদি (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	যদি (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#অন্যথা
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
# যদি !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	যদি (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#endif
	যদি (opt & OPT_H_follow_links) {
		G.slink_depth = 1;
	}
	যদি (opt & OPT_L_follow_links) {
		G.slink_depth = INT_MAX;
	}
	যদি (opt & OPT_s_যোগ_norecurse) {
		G.max_print_depth = 0;
	}

	/* go through remaining args (if any) */
	যদি (!*argv) {
		*--argv = (অক্ষর*)".";
		যদি (G.slink_depth == 1) {
			G.slink_depth = 0;
		}
	}

	slink_depth_save = G.slink_depth;
	যোগ_2 = 0;
	করো {
		যোগ_2 += du(*argv);
		G.slink_depth = slink_depth_save;
	} যখন (*++argv);

	যদি (ENABLE_FEATURE_CLEAN_UP)
		reset_ino_dev_hashtable();
	যদি (opt & OPT_c_যোগ)
		print(যোগ_2, "total");

	fflush_stdout_and_exit(G.status);
}
