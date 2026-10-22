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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "libcoreutils/coreutils.h"

ਪੂਰਨ mv_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ mv_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਬਣਤਰ stat statbuf;
	ਅਚਲ ਅੱਖਰ *ਆਖਰੀ;
	ਅਚਲ ਅੱਖਰ *dest;
	ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
	ਪੂਰਨ dest_exists;
	ਪੂਰਨ ਸਥਿਤੀ = 0;
	ਪੂਰਨ copy_flag = 0;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_FORCE       (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_INTERACTIVE (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_NOCLOBBER   (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_DESTNOTDIR  (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_DESTDIR     (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_ਦਲੀਲ "i"
			"force\0"       No_ਦਲੀਲ "f"
			"no-clobber\0"  No_ਦਲੀਲ "n"
			"no-target-directory\0" No_ਦਲੀਲ "T"
			"target-directory\0" Required_ਦਲੀਲ "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_ਦਲੀਲ "v"
			)
			, &ਆਖਰੀ
	);
	argc -= optind;
	argv += optind;

	ਜੇ (!(flags & OPT_DESTDIR)) {
		ਆਖਰੀ = argv[argc - 1];
		ਜੇ (argc < 2)
			bb_show_usage();
		ਜੇ (argc != 2) {
			ਜੇ (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} ਨਹੀਂਤਾਂ /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(ਆਖਰੀ, &statbuf);
			ਜੇ (dest_exists < 0) { /* error other than ENOENT */
				ਵਾਪਸ EXIT_FAILURE;
			}
			ਜੇ (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = ਆਖਰੀ;
				ਜਾਓ DO_MOVE;
			}
			/* last is a directory */
			ਜੇ (flags & OPT_DESTNOTDIR) {
				ਜੇ (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_ਬਹੁਲਕ))
					bb_error_ਸੁਨੇਹਾ_and_die("'%s' is a directory", ਆਖਰੀ);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = ਆਖਰੀ;
				ਜਾਓ DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	ਕਰੋ {
		dest = concat_ਰਸਤਾ_file(ਆਖਰੀ, bb_get_ਆਖਰੀ_ਰਸਤਾ_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		ਜੇ (dest_exists < 0) {
			ਜਾਓ RET_1;
		}

 DO_MOVE:
		ਜੇ (dest_exists) {
			ਜੇ (flags & OPT_NOCLOBBER)
				ਜਾਓ RET_0;
			ਜੇ (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				ਜੇ (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					ਜਾਓ RET_1;  /* Ouch! fprintf failed! */
				}
				ਜੇ (!bb_ask_y_confirmation()) {
					ਜਾਓ RET_0;
				}
			}
		}

		ਜੇ (rename(*argv, dest) < 0) {
			ਪੂਰਨ ਸਰੋਤ_exists;

			ਜੇ (errno != EXDEV
			 || (ਸਰੋਤ_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_ਸੁਨੇਹਾ("can't rename '%s'", *argv);
			} ਨਹੀਂਤਾਂ {
				ਸਥਿਰ ਅਚਲ ਅੱਖਰ fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				ਜੇ (dest_exists) {
					ਜੇ (dest_exists == 3) {
						ਜੇ (ਸਰੋਤ_exists != 3) {
							bb_error_ਸੁਨੇਹਾ(fmt, "", "non-");
							ਜਾਓ RET_1;
						}
					} ਨਹੀਂਤਾਂ {
						ਜੇ (ਸਰੋਤ_exists == 3) {
							bb_error_ਸੁਨੇਹਾ(fmt, "non-", "");
							ਜਾਓ RET_1;
						}
					}
					ਜੇ (unlink(dest) < 0) {
						bb_perror_ਸੁਨੇਹਾ("can't remove '%s'", dest);
						ਜਾਓ RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_ਸਥਿਤੀ;
#ਜੇ ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#ਜੇ_ਅੰਤ
				ਜੇ ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					ਜਾਓ RET_0;
				}
			}
 RET_1:
			ਸਥਿਤੀ = 1;
		}
 RET_0:
		ਜੇ (flags & OPT_VERBOSE) {
			ਲਿਖੋ("'%s' -> '%s'\n", *argv, dest);
		}
		ਜੇ (dest != ਆਖਰੀ) {
			ਮੁਕਤ((ਖਾਲੀ *) dest);
		}
	} ਜਦੋਂ (*++argv && *argv != ਆਖਰੀ);

	ਵਾਪਸ ਸਥਿਤੀ;
}
