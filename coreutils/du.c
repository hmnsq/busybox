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
	OPT_s_کل_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_کل        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

بنية globals {
#إذا ENABLE_FEATURE_HUMAN_READABLE
	بنانشانی لما disp_unit;
#وإلا
	بنانشانی disp_k;
#جے_انت
	صحيح max_print_ڈونگھائی;
	bool حالت;
	صحيح slink_ڈونگھائی;
	صحيح du_ڈونگھائی;
	dev_t dir_dev;
} FIX_ALIASING;
#تعریف_کرو G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() افعل { تیاری_common_bufsiz(); } طالما (0)


ساكن فراغ print(بنانشانی لما لما ناپ, ثابت حرف *filename)
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
		ناپ += (G.disp_unit-1) / (بنانشانی)(512 * 2);
# endif
	لکھو("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(ناپ, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#وإلا
	إذا (G.disp_k) {
		إذا (!(option_mask32 & OPT_b)) {
			ناپ++;
			ناپ >>= 1;
		} وإلا {
			ناپ >>= 10;
		}
	}
	لکھو("%llu\t%s\n", ناپ, filename);
#جے_انت
}

/* tiny recursive du */
ساكن بنانشانی لما لما du(ثابت حرف *filename)
{
	بنية stat statbuf;
	بنانشانی لما لما جوڑ;

	إذا (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_پیغام(filename);
		G.حالت = EXIT_FAILURE;
		أرجع 0;
	}

	إذا (option_mask32 & OPT_x_one_FS) {
		إذا (G.du_ڈونگھائی == 0) {
			G.dir_dev = statbuf.st_dev;
		} وإلا إذا (G.dir_dev != statbuf.st_dev) {
			أرجع 0;
		}
	}

	جوڑ = ((option_mask32 & OPT_b) ? statbuf.st_ناپ : statbuf.st_بلاکاں);

	إذا (S_ISLNK(statbuf.st_منوال)) {
		إذا (G.slink_ڈونگھائی > G.du_ڈونگھائی) { /* -H or -L */
			إذا (stat(filename, &statbuf) != 0) {
				bb_simple_perror_پیغام(filename);
				G.حالت = EXIT_FAILURE;
				أرجع 0;
			}
			جوڑ = ((option_mask32 & OPT_b) ? statbuf.st_ناپ : statbuf.st_بلاکاں);
			إذا (G.slink_ڈونگھائی == 1) {
				/* Convert -H to -L */
				G.slink_ڈونگھائی = ودھ_توں_ودھ_عدد;
			}
		}
	}

	إذا (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		إذا (is_وچ_ino_dev_hashtable(&statbuf)) {
			أرجع 0;
		}
		جوڑو_منزل_ino_dev_hashtable(&statbuf, عدم);
	}

	إذا (S_ISDIR(statbuf.st_منوال)) {
		DIR *dir;
		بنية dirent *اندراج;
		حرف *newfile;

		dir = warn_opendir(filename);
		إذا (!dir) {
			G.حالت = EXIT_FAILURE;
			أرجع جوڑ;
		}

		طالما ((اندراج = readdir(dir))) {
			newfile = concat_subpath_file(filename, اندراج->d_name);
			إذا (newfile == عدم)
				استمر;
			++G.du_ڈونگھائی;
			جوڑ += du(newfile);
			--G.du_ڈونگھائی;
			حرر(newfile);
		}
		closedir(dir);
	} وإلا {
		إذا (!(option_mask32 & OPT_a_files_too) && G.du_ڈونگھائی != 0)
			أرجع جوڑ;
	}
	إذا (G.du_ڈونگھائی <= G.max_print_ڈونگھائی) {
		print(جوڑ, filename);
	}
	أرجع جوڑ;
}

صحيح du_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح du_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	بنانشانی لما لما کل;
	صحيح slink_ڈونگھائی_save;
	بنانشانی opt;

	INIT_G();

#إذا ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	إذا (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#وإلا
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#جے_انت
	G.max_print_ڈونگھائی = ودھ_توں_ودھ_عدد;

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
			&G.max_print_ڈونگھائی
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
			&G.max_print_ڈونگھائی
	);
	argv += optind;
# إذا !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	إذا (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#جے_انت
	إذا (opt & OPT_H_follow_links) {
		G.slink_ڈونگھائی = 1;
	}
	إذا (opt & OPT_L_follow_links) {
		G.slink_ڈونگھائی = ودھ_توں_ودھ_عدد;
	}
	إذا (opt & OPT_s_کل_norecurse) {
		G.max_print_ڈونگھائی = 0;
	}

	/* go through remaining args (if any) */
	إذا (!*argv) {
		*--argv = (حرف*)".";
		إذا (G.slink_ڈونگھائی == 1) {
			G.slink_ڈونگھائی = 0;
		}
	}

	slink_ڈونگھائی_save = G.slink_ڈونگھائی;
	کل = 0;
	افعل {
		کل += du(*argv);
		G.slink_ڈونگھائی = slink_ڈونگھائی_save;
	} طالما (*++argv);

	إذا (ENABLE_FEATURE_CLEAN_اتے)
		reset_ino_dev_hashtable();
	إذا (opt & OPT_c_کل)
		print(کل, "total");

	fflush_stdout_and_exit(G.حالت);
}
