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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

क्रमागत {
	OPT_a_files_too    = (1 << 0),
	OPT_H_follow_links = (1 << 1),
	OPT_k_kbytes       = (1 << 2),
	OPT_L_follow_links = (1 << 3),
	OPT_s_योग_norecurse = (1 << 4),
	OPT_x_one_FS       = (1 << 5),
	OPT_d_maxdepth     = (1 << 6),
	OPT_l_hardlinks    = (1 << 7),
	OPT_c_योग        = (1 << 8),
	OPT_b              = (1 << 9),
	OPT_h_for_humans   = (1 << 10),
	OPT_m_mbytes       = (1 << 11),
};

काष्ठा globals {
#अगर ENABLE_FEATURE_HUMAN_READABLE
	अचिन्हित दीर्घ disp_unit;
#अन्यथा
	अचिन्हित disp_k;
#पूर्ण_अगर
	पूर्णांक max_print_depth;
	bool status;
	पूर्णांक slink_depth;
	पूर्णांक du_depth;
	dev_t dir_dev;
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { setup_common_bufsiz(); } जबतक (0)


अटल व्योम print(अचिन्हित दीर्घ दीर्घ आकार, स्थिर अक्षर *filename)
{
	/* TODO - May not want to defer error checking here. */
#अगर ENABLE_FEATURE_HUMAN_READABLE
# अगर ENABLE_DESKTOP
	/* ~30 bytes of code for extra compat:
	 * coreutils' du rounds sizes up:
	 * for example,  1025k file is shown as "2" by du -m.
	 * We round to nearest if human-readable [too hard to fix],
	 * else (fixed scale such as -m), we round up. To that end,
	 * add yet another half of the unit before displaying:
	 */
	अगर (G.disp_unit)
		आकार += (G.disp_unit-1) / (अचिन्हित)(512 * 2);
# endif
	म_लिखो("%s\t%s\n",
			/* size x 512 / G.disp_unit.
			 * If G.disp_unit == 0, show one fractional
			 * and use suffixes
			 */
			make_human_readable_str(आकार, (option_mask32 & OPT_b) ? 1 : 512, G.disp_unit),
			filename);
#अन्यथा
	अगर (G.disp_k) {
		अगर (!(option_mask32 & OPT_b)) {
			आकार++;
			आकार >>= 1;
		} अन्यथा {
			आकार >>= 10;
		}
	}
	म_लिखो("%llu\t%s\n", आकार, filename);
#पूर्ण_अगर
}

/* tiny recursive du */
अटल अचिन्हित दीर्घ दीर्घ du(स्थिर अक्षर *filename)
{
	काष्ठा stat statbuf;
	अचिन्हित दीर्घ दीर्घ योग;

	अगर (lstat(filename, &statbuf) != 0) {
		bb_simple_perror_संदेश(filename);
		G.status = निकास_त्रुटि;
		वापस 0;
	}

	अगर (option_mask32 & OPT_x_one_FS) {
		अगर (G.du_depth == 0) {
			G.dir_dev = statbuf.st_dev;
		} अन्यथा अगर (G.dir_dev != statbuf.st_dev) {
			वापस 0;
		}
	}

	योग = ((option_mask32 & OPT_b) ? statbuf.st_आकार : statbuf.st_blocks);

	अगर (S_ISLNK(statbuf.st_mode)) {
		अगर (G.slink_depth > G.du_depth) { /* -H or -L */
			अगर (stat(filename, &statbuf) != 0) {
				bb_simple_perror_संदेश(filename);
				G.status = निकास_त्रुटि;
				वापस 0;
			}
			योग = ((option_mask32 & OPT_b) ? statbuf.st_आकार : statbuf.st_blocks);
			अगर (G.slink_depth == 1) {
				/* Convert -H to -L */
				G.slink_depth = पूर्णांक_उच्च;
			}
		}
	}

	अगर (!(option_mask32 & OPT_l_hardlinks)
	 && statbuf.st_nlink > 1
	) {
		/* Add files/directories with links only once */
		अगर (is_in_ino_dev_hashtable(&statbuf)) {
			वापस 0;
		}
		add_to_ino_dev_hashtable(&statbuf, शून्य);
	}

	अगर (S_ISDIR(statbuf.st_mode)) {
		सूची *dir;
		काष्ठा dirent *entry;
		अक्षर *newfile;

		dir = warn_opendir(filename);
		अगर (!dir) {
			G.status = निकास_त्रुटि;
			वापस योग;
		}

		जबतक ((entry = सूची_पढ़ो(dir))) {
			newfile = concat_subpath_file(filename, entry->d_name);
			अगर (newfile == शून्य)
				जारी;
			++G.du_depth;
			योग += du(newfile);
			--G.du_depth;
			मुक्त(newfile);
		}
		बंद_सूची(dir);
	} अन्यथा {
		अगर (!(option_mask32 & OPT_a_files_too) && G.du_depth != 0)
			वापस योग;
	}
	अगर (G.du_depth <= G.max_print_depth) {
		print(योग, filename);
	}
	वापस योग;
}

पूर्णांक du_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक du_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित दीर्घ दीर्घ योग_2;
	पूर्णांक slink_depth_save;
	अचिन्हित opt;

	INIT_G();

#अगर ENABLE_FEATURE_HUMAN_READABLE
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 1024;)
	IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_unit = 512;)
	अगर (दो_पर्या("POSIXLY_CORRECT"))  /* TODO - a new libbb function? */
		G.disp_unit = 512;
#अन्यथा
	IF_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 1;)
	/* IF_NOT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K(G.disp_k = 0;) - G is pre-zeroed */
#पूर्ण_अगर
	G.max_print_depth = पूर्णांक_उच्च;

	/* Note: SUSv3 specifies that -a and -s options cannot be used together
	 * in strictly conforming applications.  However, it also says that some
	 * du implementations may produce output when -a and -s are used together.
	 * gnu du exits with an error code in this case.  We choose to simply
	 * ignore -a.  This is consistent with -s being equivalent to -d 0.
	 */
#अगर ENABLE_FEATURE_HUMAN_READABLE
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcbhm"
			"\0" "h-km:k-hm:m-hk:H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
	अगर (opt & OPT_b) {
		G.disp_unit = 1;
	}
	अगर (opt & OPT_h_for_humans) {
		G.disp_unit = 0;
	}
	अगर (opt & OPT_m_mbytes) {
		G.disp_unit = 1024*1024;
	}
	अगर (opt & OPT_k_kbytes) {
		G.disp_unit = 1024;
	}
#अन्यथा
	opt = getopt32(argv, "^"
			"aHkLsxd:+lcb"
			"\0" "H-L:L-H:s-d:d-s",
			&G.max_print_depth
	);
	argv += optind;
# अगर !ENABLE_FEATURE_DU_DEFAULT_BLOCKSIZE_1K
	अगर (opt & OPT_k_kbytes) {
		G.disp_k = 1;
	}
# endif
#पूर्ण_अगर
	अगर (opt & OPT_H_follow_links) {
		G.slink_depth = 1;
	}
	अगर (opt & OPT_L_follow_links) {
		G.slink_depth = पूर्णांक_उच्च;
	}
	अगर (opt & OPT_s_योग_norecurse) {
		G.max_print_depth = 0;
	}

	/* go through remaining args (if any) */
	अगर (!*argv) {
		*--argv = (अक्षर*)".";
		अगर (G.slink_depth == 1) {
			G.slink_depth = 0;
		}
	}

	slink_depth_save = G.slink_depth;
	योग_2 = 0;
	करो {
		योग_2 += du(*argv);
		G.slink_depth = slink_depth_save;
	} जबतक (*++argv);

	अगर (ENABLE_FEATURE_CLEAN_UP)
		reset_ino_dev_hashtable();
	अगर (opt & OPT_c_योग)
		print(योग_2, "total");

	fflush_stdout_and_exit(G.status);
}
