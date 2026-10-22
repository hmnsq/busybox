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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "libcoreutils/coreutils.h"

పూర్ణాంకం mv_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం mv_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	నిర్మాణం stat statbuf;
	స్థిరరాశి అక్షరం *చివరి;
	స్థిరరాశి అక్షరం *dest;
	గుర్తులేని flags;
	పూర్ణాంకం dest_exists;
	పూర్ణాంకం స్థితి_2 = 0;
	పూర్ణాంకం copy_flag = 0;

#నిర్వచించు OPT_FORCE       (1 << 0)
#నిర్వచించు OPT_INTERACTIVE (1 << 1)
#నిర్వచించు OPT_NOCLOBBER   (1 << 2)
#నిర్వచించు OPT_DESTNOTDIR  (1 << 3)
#నిర్వచించు OPT_DESTDIR     (1 << 4)
#నిర్వచించు OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_వాదం "i"
			"force\0"       No_వాదం "f"
			"no-clobber\0"  No_వాదం "n"
			"no-target-directory\0" No_వాదం "T"
			"target-directory\0" Required_వాదం "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_వాదం "v"
			)
			, &చివరి
	);
	argc -= optind;
	argv += optind;

	ఒకవేళ (!(flags & OPT_DESTDIR)) {
		చివరి = argv[argc - 1];
		ఒకవేళ (argc < 2)
			bb_show_usage();
		ఒకవేళ (argc != 2) {
			ఒకవేళ (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} లేకపోతే /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(చివరి, &statbuf);
			ఒకవేళ (dest_exists < 0) { /* error other than ENOENT */
				తిరిగి EXIT_FAILURE;
			}
			ఒకవేళ (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = చివరి;
				వెళ్ళు DO_MOVE;
			}
			/* last is a directory */
			ఒకవేళ (flags & OPT_DESTNOTDIR) {
				ఒకవేళ (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_బహుళకం))
					bb_error_సందేశం_and_die("'%s' is a directory", చివరి);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = చివరి;
				వెళ్ళు DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	చేయి {
		dest = concat_మార్గం_file(చివరి, bb_get_చివరి_మార్గం_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		ఒకవేళ (dest_exists < 0) {
			వెళ్ళు RET_1;
		}

 DO_MOVE:
		ఒకవేళ (dest_exists) {
			ఒకవేళ (flags & OPT_NOCLOBBER)
				వెళ్ళు RET_0;
			ఒకవేళ (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				ఒకవేళ (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					వెళ్ళు RET_1;  /* Ouch! fprintf failed! */
				}
				ఒకవేళ (!bb_ask_y_confirmation()) {
					వెళ్ళు RET_0;
				}
			}
		}

		ఒకవేళ (rename(*argv, dest) < 0) {
			పూర్ణాంకం ఆధారం_exists;

			ఒకవేళ (errno != EXDEV
			 || (ఆధారం_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_సందేశం("can't rename '%s'", *argv);
			} లేకపోతే {
				స్థిర స్థిరరాశి అక్షరం fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				ఒకవేళ (dest_exists) {
					ఒకవేళ (dest_exists == 3) {
						ఒకవేళ (ఆధారం_exists != 3) {
							bb_error_సందేశం(fmt, "", "non-");
							వెళ్ళు RET_1;
						}
					} లేకపోతే {
						ఒకవేళ (ఆధారం_exists == 3) {
							bb_error_సందేశం(fmt, "non-", "");
							వెళ్ళు RET_1;
						}
					}
					ఒకవేళ (unlink(dest) < 0) {
						bb_perror_సందేశం("can't remove '%s'", dest);
						వెళ్ళు RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_స్థితి;
#ఒకవేళ ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#అయితే_ముగింపు
				ఒకవేళ ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					వెళ్ళు RET_0;
				}
			}
 RET_1:
			స్థితి_2 = 1;
		}
 RET_0:
		ఒకవేళ (flags & OPT_VERBOSE) {
			రాయి("'%s' -> '%s'\n", *argv, dest);
		}
		ఒకవేళ (dest != చివరి) {
			విడుదల((శూన్య *) dest);
		}
	} అయితే (*++argv && *argv != చివరి);

	తిరిగి స్థితి_2;
}
