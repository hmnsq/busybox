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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

شمار {
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

ساخت globals {
#اگر ENABLE_FEATURE_HUMAN_READABLE
	غیرعلامتی طویل disp_unit;
#ورنہ
	غیرعلامتی disp_k;
#اگر_ختم
	صحیح max_print_گہرائی;
	bool حالت_2;
	صحیح slink_گہرائی;
	صحیح du_گہرائی;
	dev_t dir_dev;
} FIX_ALIASING;
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { تیاری_common_bufsiz(); } جب (0)


ساکن خالی print(غیرعلامتی طویل طویل سائز, ثابت حرف *filename)
{
	/* TODO - May not want to defer error checking here. */
#اگر ENABLE_FEATURE_HUMAN_READABLE
# اگر ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	اگر (G.disp_unit)
		سائز += (G.disp_unit-1) / (غیرعلامتی)(512 * 2);
# endif
	لکھو("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(سائز, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#ورنہ
	اگر (G.disp_k) {
		اگر (!(option_mask32 & OPT_b)) {
			سائز++;
			سائز >>= 1;
		} ورنہ {
			سائز >>= 10;
		}
	}
	لکھو("%llu\t%s\n", سائز, filename);
#اگر_ختم
}

/* tiny recursive du */
ساکن غیرعلامتی طویل طویل du(ثابت حرف *filename)
{
	ساخت stat statbuf;
	غیرعلامتی طویل طویل مجموع;

	اگر (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_پیغام(filename);
		G.حالت_2 = EXIT_FAILURE;
		واپس 0;
	}

	اگر (option_mask32 & OPT_x_one_FS) {
		اگر (G.du_گہرائی == 0) {
			G.dir_dev = statbuf.st_dev;
		} ورنہ اگر (G.dir_dev != statbuf.st_dev) {
			واپس 0;
		}
	}

	مجموع = ((option_mask32 & OPT_b) ? statbuf.st_سائز : statbuf.st_بلاکیں);

	اگر (S_ISLNK(statbuf.st_منوال)) {
		اگر (G.slink_گہرائی > G.du_گہرائی) { /* -H or -L */
			اگر (stat(filename, &statbuf) != 0) {
				bb_simple_perror_پیغام(filename);
				G.حالت_2 = EXIT_FAILURE;
				واپس 0;
			}
			مجموع = ((option_mask32 & OPT_b) ? statbuf.st_سائز : statbuf.st_بلاکیں);
			اگر (G.slink_گہرائی == 1) {
				/* Convert -H to -L */
				G.slink_گہرائی = زیادہ_سے_زیادہ_عدد;
			}
		}
	}

	اگر (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		اگر (is_اندر_ino_dev_hashtable(&statbuf)) {
			واپس 0;
		}
		جوڑو_مقصد_ino_dev_hashtable(&statbuf, تہی);
	}

	اگر (S_ISDIR(statbuf.st_منوال)) {
		DIR *dir;
		ساخت dirent *اندراج_2;
		حرف *newfile;

		dir = warn_opendir(filename);
		اگر (!dir) {
			G.حالت_2 = EXIT_FAILURE;
			واپس مجموع;
		}

		جب ((اندراج_2 = readdir(dir))) {
			newfile = concat_subpath_file(filename, اندراج_2->d_name);
			اگر (newfile == تہی)
				جاری;
			++G.du_گہرائی;
			مجموع += du(newfile);
			--G.du_گہرائی;
			آزاد(newfile);
		}
		closedir(dir);
	} ورنہ {
		اگر (!(option_mask32 & OPT_a_files_too) && G.du_گہرائی != 0)
			واپس مجموع;
	}
	اگر (G.du_گہرائی <= G.max_print_گہرائی) {
		print(مجموع, filename);
	}
	واپس مجموع;
}

صحیح du_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح du_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی طویل طویل کل;
	صحیح slink_گہرائی_save;
	غیرعلامتی opt;

	INIT_G();

#اگر ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	اگر (getenv("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#ورنہ
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#اگر_ختم
	G.max_print_گہرائی = زیادہ_سے_زیادہ_عدد;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#اگر ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_گہرائی
	);
	argv += optind;
	اگر (opt & OPT_b) {
		G.disp_unit = 1;
	}
	اگر (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	اگر (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	اگر (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#ورنہ
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_گہرائی
	);
	argv += optind;
# اگر !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	اگر (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#اگر_ختم
	اگر (opt & OPT_H_follow_links) {
		G.slink_گہرائی = 1;
	}
	اگر (opt & OPT_L_follow_links) {
		G.slink_گہرائی = زیادہ_سے_زیادہ_عدد;
	}
	اگر (opt & OPT_s_کل_norecurse) {
		G.max_print_گہرائی = 0;
	}

	/* go through remaining args (if any) */
	اگر (!*argv) {
		*--argv = (حرف*)".";
		اگر (G.slink_گہرائی == 1) {
			G.slink_گہرائی = 0;
		}
	}

	slink_گہرائی_save = G.slink_گہرائی;
	کل = 0;
	کرو {
		کل += du(*argv);
		G.slink_گہرائی = slink_گہرائی_save;
	} جب (*++argv);

	اگر (ENABLE_FEATURE_CLEAN_اوپر)
		reset_ino_dev_hashtable();
	اگر (opt & OPT_c_کل)
		print(کل, "total");

	fflush_stdout_and_exit(G.حالت_2);
}
