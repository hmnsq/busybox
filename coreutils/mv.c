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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "libcoreutils/coreutils.h"

ಪೂರ್ಣಾಂಕ mv_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ mv_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ರಚನೆ stat statbuf;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆಯ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dest;
	ಚಿಹ್ನೆರಹಿತ flags;
	ಪೂರ್ಣಾಂಕ dest_exists;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;
	ಪೂರ್ಣಾಂಕ copy_flag = 0;

#ವ್ಯಾಖ್ಯಾನಿಸು OPT_FORCE       (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_INTERACTIVE (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_NOCLOBBER   (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_DESTNOTDIR  (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_DESTDIR     (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_ವಾದ "i"
			"force\0"       No_ವಾದ "f"
			"no-clobber\0"  No_ವಾದ "n"
			"no-target-directory\0" No_ವಾದ "T"
			"target-directory\0" Required_ವಾದ "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_ವಾದ "v"
			)
			, &ಕೊನೆಯ
	);
	argc -= optind;
	argv += optind;

	ಆದರೆ (!(flags & OPT_DESTDIR)) {
		ಕೊನೆಯ = argv[argc - 1];
		ಆದರೆ (argc < 2)
			bb_show_usage();
		ಆದರೆ (argc != 2) {
			ಆದರೆ (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} ಇಲ್ಲದಿದ್ದರೆ /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(ಕೊನೆಯ, &statbuf);
			ಆದರೆ (dest_exists < 0) { /* error other than ENOENT */
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			}
			ಆದರೆ (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = ಕೊನೆಯ;
				ಹೋಗು DO_MOVE;
			}
			/* last is a directory */
			ಆದರೆ (flags & OPT_DESTNOTDIR) {
				ಆದರೆ (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_ಬಹುಲಕ))
					bb_error_ಸಂದೇಶ_and_die("'%s' is a directory", ಕೊನೆಯ);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = ಕೊನೆಯ;
				ಹೋಗು DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	ಮಾಡು {
		dest = concat_ಮಾರ್ಗ_file(ಕೊನೆಯ, bb_get_ಕೊನೆಯ_ಮಾರ್ಗ_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		ಆದರೆ (dest_exists < 0) {
			ಹೋಗು RET_1;
		}

 DO_MOVE:
		ಆದರೆ (dest_exists) {
			ಆದರೆ (flags & OPT_NOCLOBBER)
				ಹೋಗು RET_0;
			ಆದರೆ (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				ಆದರೆ (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					ಹೋಗು RET_1;  /* Ouch! fprintf failed! */
				}
				ಆದರೆ (!bb_ask_y_confirmation()) {
					ಹೋಗು RET_0;
				}
			}
		}

		ಆದರೆ (rename(*argv, dest) < 0) {
			ಪೂರ್ಣಾಂಕ ಮೂಲ_exists;

			ಆದರೆ (errno != EXDEV
			 || (ಮೂಲ_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_ಸಂದೇಶ("can't rename '%s'", *argv);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				ಆದರೆ (dest_exists) {
					ಆದರೆ (dest_exists == 3) {
						ಆದರೆ (ಮೂಲ_exists != 3) {
							bb_error_ಸಂದೇಶ(fmt, "", "non-");
							ಹೋಗು RET_1;
						}
					} ಇಲ್ಲದಿದ್ದರೆ {
						ಆದರೆ (ಮೂಲ_exists == 3) {
							bb_error_ಸಂದೇಶ(fmt, "non-", "");
							ಹೋಗು RET_1;
						}
					}
					ಆದರೆ (unlink(dest) < 0) {
						bb_perror_ಸಂದೇಶ("can't remove '%s'", dest);
						ಹೋಗು RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_ಸ್ಥಿತಿ;
#ಆದರೆ ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#ಆದರೆ_ಅಂತ್ಯ
				ಆದರೆ ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					ಹೋಗು RET_0;
				}
			}
 RET_1:
			ಸ್ಥಿತಿ_2 = 1;
		}
 RET_0:
		ಆದರೆ (flags & OPT_VERBOSE) {
			ಬರೆ("'%s' -> '%s'\n", *argv, dest);
		}
		ಆದರೆ (dest != ಕೊನೆಯ) {
			ಬಿಡುಗಡೆ((ಶೂನ್ಯ *) dest);
		}
	} ಆಗ (*++argv && *argv != ಕೊನೆಯ);

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}
