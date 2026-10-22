/* vi: set sw=4 ts=4: */
/*
 * Mini mv implementation for busybox
 *
 * Copyright (C) 2000 by Matt Kraai <kraai@alumni.carnegiemellon.edu>
 * SELinux support by Yuichi Nakamura <ynakam@hitachisoft.jp>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reduction and improved error checking.
 */
//config:config MV
//config:	bool "mv (10 kb)"
//config:	default y
//config:	help
//config:	mv is used to move or rename files or directories.

//applet:IF_MV(APPLET_NOEXEC(mv, mv, BB_DIR_BIN, BB_SUID_DROP, mv))
/* NOEXEC despite cases when it can be a "runner" (mv LARGE_DIR OTHER_FS) */

//kbuild:lib-$(CONFIG_MV) += mv.o

//usage:#define mv_trivial_usage
//usage:       "[-finT] SOURCE DEST\n"
//usage:       "or: mv [-fin] SOURCE... { -t DIRECTORY | DIRECTORY }"
//usage:#define mv_full_usage "\n\n"
//usage:       "Rename SOURCE to DEST, or move SOURCEs to DIRECTORY\n"
//usage:     "\n	-f	Don't prompt before overwriting"
//usage:     "\n	-i	Interactive, prompt before overwrite"
//usage:     "\n	-n	Don't overwrite an existing file"
//usage:     "\n	-T	Refuse to move if DEST is a directory"
//usage:     "\n	-t DIR	Move all SOURCEs into DIR"
//usage:
//usage:#define mv_example_usage
//usage:       "$ mv /tmp/foo /bin/bar\n"

#شامل "libbb.h"
#شامل "libcoreutils/coreutils.h"

صحیح mv_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح mv_main(صحیح argc, نویسه **argv)
{
	ساختار stat statbuf;
	ثابت نویسه *last;
	ثابت نویسه *dest;
	unsigned flags;
	صحیح dest_exists;
	صحیح status = 0;
	صحیح copy_flag = 0;

#define OPT_FORCE       (1 << 0)
#define OPT_INTERACTIVE (1 << 1)
#define OPT_NOCLOBBER   (1 << 2)
#define OPT_DESTNOTDIR  (1 << 3)
#define OPT_DESTDIR     (1 << 4)
#define OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_argument "i"
			"force\0"       No_argument "f"
			"no-clobber\0"  No_argument "n"
			"no-target-directory\0" No_argument "T"
			"target-directory\0" Required_argument "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_argument "v"
			)
			, &last
	);
	argc -= optind;
	argv += optind;

	اگر (!(flags & OPT_DESTDIR)) {
		last = argv[argc - 1];
		اگر (argc < 2)
			bb_show_usage();
		اگر (argc != 2) {
			اگر (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} وگرنه /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(last, &statbuf);
			اگر (dest_exists < 0) { /* error other than ENOENT */
				بازگشت EXIT_FAILURE;
			}
			اگر (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = last;
				برو DO_MOVE;
			}
			/* last is a directory */
			اگر (flags & OPT_DESTNOTDIR) {
				اگر (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_mode))
					bb_error_پیام_and_die("'%s' is a directory", last);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = last;
				برو DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	انجامبده {
		dest = concat_path_file(last, bb_get_last_path_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		اگر (dest_exists < 0) {
			برو RET_1;
		}

 DO_MOVE:
		اگر (dest_exists) {
			اگر (flags & OPT_NOCLOBBER)
				برو RET_0;
			اگر (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				اگر (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					برو RET_1;  /* Ouch! fprintf failed! */
				}
				اگر (!bb_ask_y_confirmation()) {
					برو RET_0;
				}
			}
		}

		اگر (rename(*argv, dest) < 0) {
			صحیح source_exists;

			اگر (errno != EXDEV
			 || (source_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_پیام("can't rename '%s'", *argv);
			} وگرنه {
				ایستا ثابت نویسه fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				اگر (dest_exists) {
					اگر (dest_exists == 3) {
						اگر (source_exists != 3) {
							bb_error_پیام(fmt, "", "non-");
							برو RET_1;
						}
					} وگرنه {
						اگر (source_exists == 3) {
							bb_error_پیام(fmt, "non-", "");
							برو RET_1;
						}
					}
					اگر (unlink(dest) < 0) {
						bb_perror_پیام("can't remove '%s'", dest);
						برو RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_STATUS;
#اگر ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#endif
				اگر ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					برو RET_0;
				}
			}
 RET_1:
			status = 1;
		}
 RET_0:
		اگر (flags & OPT_VERBOSE) {
			printf("'%s' -> '%s'\n", *argv, dest);
		}
		اگر (dest != last) {
			آزاد((تهی *) dest);
		}
	} وقتی (*++argv && *argv != last);

	بازگشت status;
}
