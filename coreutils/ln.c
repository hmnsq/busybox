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

#अन्तर्भाव "libbb.h"

/* This is a NOEXEC applet. Be very careful! */

#परिभाषय LN_SYMLINK          (1 << 0)
#परिभाषय LN_FORCE            (1 << 1)
#परिभाषय LN_NODEREFERENCE    (1 << 2)
#परिभाषय LN_BACKUP           (1 << 3)
#परिभाषय LN_SUFFIX           (1 << 4)
#परिभाषय LN_VERBOSE          (1 << 5)
#परिभाषय LN_LINKFILE         (1 << 6)

पूर्णाङ्क ln_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क ln_main(पूर्णाङ्क argc, वर्ण **argv)
{
	exitcode_t स्थितिः = EXIT_SUCCESS;
	पूर्णाङ्क opts;
	वर्ण *चरमः;
	वर्ण *src_name;
	वर्ण *src;
	वर्ण *suffix = (वर्ण*)"~";
	संरचना stat statbuf;
	पूर्णाङ्क (*link_func)(ध्रुव वर्ण *, ध्रुव वर्ण *);

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
	चरमः = argv[argc - 1];
	argv += optind;
	argc -= optind;

	यदि ((opts & LN_LINKFILE) && argc > 2) {
		bb_simple_error_सन्देशः_and_die("-T accepts 2 args max");
	}

	यदि (!argv[1]) {
		/* "ln PATH/TO/FILE" -> "ln PATH/TO/FILE FILE" */
		*--argv = चरमः;
		/* xstrdup is needed: "ln -s PATH/TO/FILE/" is equivalent to
		 * "ln -s PATH/TO/FILE/ FILE", not "ln -s PATH/TO/FILE FILE"
		 */
		चरमः = bb_get_चरमः_मार्गः_component_strip(xstrdup(चरमः));
	}

	कुरु {
		src_name = रिक्त;
		src = चरमः;

		यदि (is_directory(src,
				/*followlinks:*/ !(opts & (LN_NODEREFERENCE|LN_LINKFILE))
				/* Why LN_LINKFILE does not follow links:
				 * -T/--no-target-directory implies -n/--no-dereference
				 */
				)
		) {
			यदि (opts & LN_LINKFILE) {
				bb_error_सन्देशः_and_die("'%s' is a directory", src);
			}
			src_name = xstrdup(*argv);
			src = concat_मार्गः_file(src, bb_get_चरमः_मार्गः_component_strip(src_name));
			मुक्त(src_name);
			src_name = src;
		}
		यदि (!(opts & LN_SYMLINK) && stat(*argv, &statbuf)) {
			// coreutils: "ln dangling_symlink new_hardlink" works
			यदि (lstat(*argv, &statbuf) || !S_ISLNK(statbuf.st_बहुलकम्)) {
				bb_simple_perror_सन्देशः(*argv);
				स्थितिः = EXIT_FAILURE;
				मुक्त(src_name);
				अनुवर्त;
			}
		}

		यदि (opts & LN_BACKUP) {
			वर्ण *backup;
			backup = xasprintf("%s%s", src, suffix);
			यदि (rename(src, backup) < 0 && errno != ENOENT) {
				bb_simple_perror_सन्देशः(src);
				स्थितिः = EXIT_FAILURE;
				मुक्त(backup);
				अनुवर्त;
			}
			मुक्त(backup);
			/*
			 * When the source and dest are both hard links to the same
			 * inode, a rename may succeed even though nothing happened.
			 * Therefore, always unlink().
			 */
			unlink(src);
		} अन्यथा यदि (opts & LN_FORCE) {
			unlink(src);
		}

		link_func = link;
		यदि (opts & LN_SYMLINK) {
			link_func = symlink;
		}

		यदि (opts & LN_VERBOSE) {
			प्रारूप_लिख("'%s' -> '%s'\n", src, *argv);
		}

		यदि (link_func(*argv, src) != 0) {
			bb_simple_perror_सन्देशः(src);
			स्थितिः = EXIT_FAILURE;
		}

		मुक्त(src_name);
	} यावत् ((++argv)[1]);

	प्रत्यागम स्थितिः;
}
