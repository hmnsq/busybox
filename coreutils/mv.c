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

#ضمن "libbb.h"
#ضمن "libcoreutils/coreutils.h"

صحيح mv_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح mv_main(صحيح argc, حرف **argv)
{
	بنية stat statbuf;
	ثابت حرف *آخرین;
	ثابت حرف *dest;
	بیعلامت flags;
	صحيح dest_exists;
	صحيح وضعیت = 0;
	صحيح copy_flag = 0;

#تعریف_کن OPT_FORCE       (1 << 0)
#تعریف_کن OPT_INTERACTIVE (1 << 1)
#تعریف_کن OPT_NOCLOBBER   (1 << 2)
#تعریف_کن OPT_DESTNOTDIR  (1 << 3)
#تعریف_کن OPT_DESTDIR     (1 << 4)
#تعریف_کن OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_آرگومان "i"
			"force\0"       No_آرگومان "f"
			"no-clobber\0"  No_آرگومان "n"
			"no-target-directory\0" No_آرگومان "T"
			"target-directory\0" Required_آرگومان "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_آرگومان "v"
			)
			, &آخرین
	);
	argc -= optind;
	argv += optind;

	إذا (!(flags & OPT_DESTDIR)) {
		آخرین = argv[argc - 1];
		إذا (argc < 2)
			bb_show_usage();
		إذا (argc != 2) {
			إذا (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} وإلا /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(آخرین, &statbuf);
			إذا (dest_exists < 0) { /* error other than ENOENT */
				أرجع EXIT_FAILURE;
			}
			إذا (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = آخرین;
				اذهب DO_MOVE;
			}
			/* last is a directory */
			إذا (flags & OPT_DESTNOTDIR) {
				إذا (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_نما))
					bb_error_پیام_and_die("'%s' is a directory", آخرین);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = آخرین;
				اذهب DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	افعل {
		dest = concat_مسیر_file(آخرین, bb_get_آخرین_مسیر_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		إذا (dest_exists < 0) {
			اذهب RET_1;
		}

 DO_MOVE:
		إذا (dest_exists) {
			إذا (flags & OPT_NOCLOBBER)
				اذهب RET_0;
			إذا (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				إذا (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					اذهب RET_1;  /* Ouch! fprintf failed! */
				}
				إذا (!bb_ask_y_confirmation()) {
					اذهب RET_0;
				}
			}
		}

		إذا (rename(*argv, dest) < 0) {
			صحيح منبع_exists;

			إذا (errno != EXDEV
			 || (منبع_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_پیام("can't rename '%s'", *argv);
			} وإلا {
				ساكن ثابت حرف fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				إذا (dest_exists) {
					إذا (dest_exists == 3) {
						إذا (منبع_exists != 3) {
							bb_error_پیام(fmt, "", "non-");
							اذهب RET_1;
						}
					} وإلا {
						إذا (منبع_exists == 3) {
							bb_error_پیام(fmt, "non-", "");
							اذهب RET_1;
						}
					}
					إذا (unlink(dest) < 0) {
						bb_perror_پیام("can't remove '%s'", dest);
						اذهب RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_وضعیت;
#إذا ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#پایان_اگر
				إذا ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					اذهب RET_0;
				}
			}
 RET_1:
			وضعیت = 1;
		}
 RET_0:
		إذا (flags & OPT_VERBOSE) {
			چاپ_کن("'%s' -> '%s'\n", *argv, dest);
		}
		إذا (dest != آخرین) {
			حرر((فراغ *) dest);
		}
	} طالما (*++argv && *argv != آخرین);

	أرجع وضعیت;
}
