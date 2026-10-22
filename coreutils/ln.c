/* vi: set sw=4 ts=4: */
/*
 * Mini ln implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config LN
//config:	bool "ln (5.1 kb)"
//config:	default y
//config:	help
//config:	ln is used to create hard or soft links between files.

//applet:IF_LN(APPLET_NOEXEC(ln, ln, BB_DIR_BIN, BB_SUID_DROP, ln))

//kbuild:lib-$(CONFIG_LN) += ln.o

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU options missing: -d, -F, -i, and -v. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/ln.html */

//usage:#define ln_trivial_usage
//usage:       "[-sfnbtv] [-S SUF] TARGET... LINK|DIR"
//usage:#define ln_full_usage "\n\n"
//usage:       "Create a link LINK or DIR/TARGET to the specified TARGET(s)\n"
//usage:     "\n	-s	Make symlinks instead of hardlinks"
//usage:     "\n	-f	Remove existing destinations"
//usage:     "\n	-n	Don't dereference symlinks - treat like normal file"
//usage:     "\n	-b	Make a backup of the target (if exists) before link operation"
//usage:     "\n	-S SUF	Use suffix instead of ~ when making backup files"
//usage:     "\n	-T	Treat LINK as a file, not DIR"
//usage:     "\n	-v	Verbose"
//usage:
//usage:#define ln_example_usage
//usage:       "$ ln -s BusyBox /tmp/ls\n"
//usage:       "$ ls -l /tmp/ls\n"
//usage:       "lrwxrwxrwx    1 root     root            7 Apr 12 18:39 ls -> BusyBox*\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#define LN_SYMLINK          (1 << 0)
#define LN_FORCE            (1 << 1)
#define LN_NODEREFERENCE    (1 << 2)
#define LN_BACKUP           (1 << 3)
#define LN_SUFFIX           (1 << 4)
#define LN_VERBOSE          (1 << 5)
#define LN_LINKFILE         (1 << 6)

ಪೂರ್ಣಾಙ್ಕ ln_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ ln_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	exitcode_t status = EXIT_SUCCESS;
	ಪೂರ್ಣಾಙ್ಕ opts;
	ವರ್ಣ *last;
	ವರ್ಣ *src_name;
	ವರ್ಣ *src;
	ವರ್ಣ *suffix = (ವರ್ಣ*)"~";
	ಸಂರಚನಾ stat statbuf;
	ಪೂರ್ಣಾಙ್ಕ (*link_func)(ಧ್ರುವ ವರ್ಣ *, ಧ್ರುವ ವರ್ಣ *);

	opts = getopt32(argv, "^" "sfnbS:vT" "\0" "-1", &suffix);
/*
	-s, --symbolic		make symbolic links instead of hard links
	-f, --force		remove existing destination files
	-n, --no-dereference	treat LINK_NAME as a normal file if it is a symbolic link to a directory
	-b			like --backup but does not accept an argument
	--backup[=CONTROL]	make a backup of each existing destination file
	-S, --suffix=SUFFIX	override the usual backup suffix
	-v, --verbose
	-T, --no-target-directory
	-d, -F, --directory	allow the superuser to attempt to hard link directories
	-i, --interactive	prompt whether to remove destinations
	-L, --logical		dereference TARGETs that are symbolic links
	-P, --physical		make hard links directly to symbolic links
	-r, --relative		create symbolic links relative to link location
	-t, --target-directory=DIRECTORY	specify the DIRECTORY in which to create the links
 */
	last = argv[argc - 1];
	argv += optind;
	argc -= optind;

	ಯದಿ ((opts & LN_LINKFILE) && argc > 2) {
		bb_simple_error_ಸಂದೇಶ_and_die("-T accepts 2 args max");
	}

	ಯದಿ (!argv[1]) {
		/* "ln PATH/TO/FILE" -> "ln PATH/TO/FILE FILE" */
		*--argv = last;
		/* xstrdup is needed: "ln -s PATH/TO/FILE/" is equivalent to
		 * "ln -s PATH/TO/FILE/ FILE", not "ln -s PATH/TO/FILE FILE"
		 */
		last = bb_get_last_path_component_strip(xstrdup(last));
	}

	ಕುರು {
		src_name = ರಿಕ್ತ;
		src = last;

		ಯದಿ (is_directory(src,
				/*followlinks:*/ !(opts & (LN_NODEREFERENCE|LN_LINKFILE))
				/* Why LN_LINKFILE does not follow links:
				 * -T/--no-target-directory implies -n/--no-dereference
				 */
				)
		) {
			ಯದಿ (opts & LN_LINKFILE) {
				bb_error_ಸಂದೇಶ_and_die("'%s' is a directory", src);
			}
			src_name = xstrdup(*argv);
			src = concat_path_file(src, bb_get_last_path_component_strip(src_name));
			ಮುಕ್ತ(src_name);
			src_name = src;
		}
		ಯದಿ (!(opts & LN_SYMLINK) && stat(*argv, &statbuf)) {
			// coreutils: "ln dangling_symlink new_hardlink" works
			ಯದಿ (lstat(*argv, &statbuf) || !S_ISLNK(statbuf.st_mode)) {
				bb_simple_perror_ಸಂದೇಶ(*argv);
				status = EXIT_FAILURE;
				ಮುಕ್ತ(src_name);
				ಅನುವರ್ತ;
			}
		}

		ಯದಿ (opts & LN_BACKUP) {
			ವರ್ಣ *backup;
			backup = xasprintf("%s%s", src, suffix);
			ಯದಿ (rename(src, backup) < 0 && errno != ENOENT) {
				bb_simple_perror_ಸಂದೇಶ(src);
				status = EXIT_FAILURE;
				ಮುಕ್ತ(backup);
				ಅನುವರ್ತ;
			}
			ಮುಕ್ತ(backup);
			/*
			 * When the source and dest are both hard links to the same
			 * inode, a rename may succeed even though nothing happened.
			 * Therefore, always unlink().
			 */
			unlink(src);
		} ಅನ್ಯಥಾ ಯದಿ (opts & LN_FORCE) {
			unlink(src);
		}

		link_func = link;
		ಯದಿ (opts & LN_SYMLINK) {
			link_func = symlink;
		}

		ಯದಿ (opts & LN_VERBOSE) {
			printf("'%s' -> '%s'\n", src, *argv);
		}

		ಯದಿ (link_func(*argv, src) != 0) {
			bb_simple_perror_ಸಂದೇಶ(src);
			status = EXIT_FAILURE;
		}

		ಮುಕ್ತ(src_name);
	} ಯಾವತ್ ((++argv)[1]);

	ಪ್ರತ್ಯಾಗಮ status;
}
