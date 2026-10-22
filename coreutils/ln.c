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

#అన్తర్భావ "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#నిర్వచించు LN_SYMLINK          (1 << 0)
#నిర్వచించు LN_FORCE            (1 << 1)
#నిర్వచించు LN_NODEREFERENCE    (1 << 2)
#నిర్వచించు LN_BACKUP           (1 << 3)
#నిర్వచించు LN_SUFFIX           (1 << 4)
#నిర్వచించు LN_VERBOSE          (1 << 5)
#నిర్వచించు LN_LINKFILE         (1 << 6)

పూర్ణాంకం ln_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం ln_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	exitcode_t స్థితి_2 = EXIT_SUCCESS;
	పూర్ణాంకం opts;
	అక్షరం *చివరి;
	అక్షరం *src_name;
	అక్షరం *src;
	అక్షరం *suffix = (అక్షరం*)"~";
	నిర్మాణం stat statbuf;
	పూర్ణాంకం (*link_func)(స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);

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
	చివరి = argv[argc - 1];
	argv += optind;
	argc -= optind;

	ఒకవేళ ((opts & LN_LINKFILE) && argc > 2) {
		bb_simple_error_సందేశం_and_die("-T accepts 2 args max");
	}

	ఒకవేళ (!argv[1]) {
		/* "ln PATH/TO/FILE" -> "ln PATH/TO/FILE FILE" */
		*--argv = చివరి;
		/* xstrdup is needed: "ln -s PATH/TO/FILE/" is equivalent to
		 * "ln -s PATH/TO/FILE/ FILE", not "ln -s PATH/TO/FILE FILE"
		 */
		చివరి = bb_get_చివరి_మార్గం_component_strip(xstrdup(చివరి));
	}

	చేయి {
		src_name = ఖాళీ;
		src = చివరి;

		ఒకవేళ (is_directory(src,
				/*followlinks:*/ !(opts & (LN_NODEREFERENCE|LN_LINKFILE))
				/* Why LN_LINKFILE does not follow links:
				 * -T/--no-target-directory implies -n/--no-dereference
				 */
				)
		) {
			ఒకవేళ (opts & LN_LINKFILE) {
				bb_error_సందేశం_and_die("'%s' is a directory", src);
			}
			src_name = xstrdup(*argv);
			src = concat_మార్గం_file(src, bb_get_చివరి_మార్గం_component_strip(src_name));
			విడుదల(src_name);
			src_name = src;
		}
		ఒకవేళ (!(opts & LN_SYMLINK) && stat(*argv, &statbuf)) {
			// coreutils: "ln dangling_symlink new_hardlink" works
			ఒకవేళ (lstat(*argv, &statbuf) || !S_ISLNK(statbuf.st_బహుళకం)) {
				bb_simple_perror_సందేశం(*argv);
				స్థితి_2 = EXIT_FAILURE;
				విడుదల(src_name);
				కొనసాగు;
			}
		}

		ఒకవేళ (opts & LN_BACKUP) {
			అక్షరం *backup;
			backup = xasprintf("%s%s", src, suffix);
			ఒకవేళ (rename(src, backup) < 0 && errno != ENOENT) {
				bb_simple_perror_సందేశం(src);
				స్థితి_2 = EXIT_FAILURE;
				విడుదల(backup);
				కొనసాగు;
			}
			విడుదల(backup);
			/*
			 * When the source and dest are both hard links to the same
			 * inode, a rename may succeed even though nothing happened.
			 * Therefore, always unlink().
			 */
			unlink(src);
		} లేకపోతే ఒకవేళ (opts & LN_FORCE) {
			unlink(src);
		}

		link_func = link;
		ఒకవేళ (opts & LN_SYMLINK) {
			link_func = symlink;
		}

		ఒకవేళ (opts & LN_VERBOSE) {
			రాయి("'%s' -> '%s'\n", src, *argv);
		}

		ఒకవేళ (link_func(*argv, src) != 0) {
			bb_simple_perror_సందేశం(src);
			స్థితి_2 = EXIT_FAILURE;
		}

		విడుదల(src_name);
	} అయితే ((++argv)[1]);

	తిరిగి స్థితి_2;
}
