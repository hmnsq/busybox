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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "libcoreutils/coreutils.h"

पूर्णाङ्क mv_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क mv_main(पूर्णाङ्क argc, वर्ण **argv)
{
	संरचना stat statbuf;
	ध्रुव वर्ण *चरमः;
	ध्रुव वर्ण *dest;
	अचिह्नित flags;
	पूर्णाङ्क dest_exists;
	पूर्णाङ्क स्थितिः = 0;
	पूर्णाङ्क copy_flag = 0;

#परिभाषय OPT_FORCE       (1 << 0)
#परिभाषय OPT_INTERACTIVE (1 << 1)
#परिभाषय OPT_NOCLOBBER   (1 << 2)
#परिभाषय OPT_DESTNOTDIR  (1 << 3)
#परिभाषय OPT_DESTDIR     (1 << 4)
#परिभाषय OPT_VERBOSE     ((1 << 5) * ENABLE_FEATURE_VERBOSE)
	flags = getopt32long(argv, "^"
			"finTt:v"
			"\0"
	/* At least one argument. (Usually two+, but -t DIR can have only one) */
			"-1"
	/* only the final one of -f, -i, -n takes effect */
			":f-in:i-fn:n-fi"
	/* -t and -T don't mix */
			":t--T:T--t",
			"interactive\0" No_तर्कः "i"
			"force\0"       No_तर्कः "f"
			"no-clobber\0"  No_तर्कः "n"
			"no-target-directory\0" No_तर्कः "T"
			"target-directory\0" Required_तर्कः "t"
			IF_FEATURE_VERBOSE(
			"verbose\0"     No_तर्कः "v"
			)
			, &चरमः
	);
	argc -= optind;
	argv += optind;

	यदि (!(flags & OPT_DESTDIR)) {
		चरमः = argv[argc - 1];
		यदि (argc < 2)
			bb_show_usage();
		यदि (argc != 2) {
			यदि (flags & OPT_DESTNOTDIR)
				bb_show_usage();
			/* "mv A B C... DIR" - target must be dir */
		} अन्यथा /* argc == 2 */ {
			/* "mv A B" - only case where target can be not a dir */
			dest_exists = cp_mv_stat(चरमः, &statbuf);
			यदि (dest_exists < 0) { /* error other than ENOENT */
				प्रत्यागम EXIT_FAILURE;
			}
			यदि (!(dest_exists & 2)) {
				/* last is not a directory */
				dest = चरमः;
				गच्छ DO_MOVE;
			}
			/* last is a directory */
			यदि (flags & OPT_DESTNOTDIR) {
				यदि (stat(argv[0], &statbuf) == 0 && !S_ISDIR(statbuf.st_बहुलकम्))
					bb_error_सन्देशः_and_die("'%s' is a directory", चरमः);
				/* "mv -T DIR1 DIR2" is allowed (renames a dir) */
				dest = चरमः;
				गच्छ DO_MOVE;
			}
			/* else: fall through into "do { move SRC to DIR/SRC } while" loop */
		}
	}
	/* else: last is DIR from "-t DIR" */

	कुरु {
		dest = concat_मार्गः_file(चरमः, bb_get_चरमः_मार्गः_component_strip(*argv));
		dest_exists = cp_mv_stat(dest, &statbuf);
		यदि (dest_exists < 0) {
			गच्छ RET_1;
		}

 DO_MOVE:
		यदि (dest_exists) {
			यदि (flags & OPT_NOCLOBBER)
				गच्छ RET_0;
			यदि (!(flags & OPT_FORCE)
			 && ((access(dest, W_OK) < 0 && isatty(0))
			    || (flags & OPT_INTERACTIVE))
			) {
				यदि (fprintf(stderr, "mv: overwrite '%s'? ", dest) < 0) {
					गच्छ RET_1;  /* Ouch! fprintf failed! */
				}
				यदि (!bb_ask_y_confirmation()) {
					गच्छ RET_0;
				}
			}
		}

		यदि (rename(*argv, dest) < 0) {
			पूर्णाङ्क source_exists;

			यदि (errno != EXDEV
			 || (source_exists = cp_mv_stat2(*argv, &statbuf, lstat)) < 1
			) {
				bb_perror_सन्देशः("can't rename '%s'", *argv);
			} अन्यथा {
				स्थिर ध्रुव वर्ण fmt[] ALIGN1 =
					"can't overwrite %sdirectory with %sdirectory";

				यदि (dest_exists) {
					यदि (dest_exists == 3) {
						यदि (source_exists != 3) {
							bb_error_सन्देशः(fmt, "", "non-");
							गच्छ RET_1;
						}
					} अन्यथा {
						यदि (source_exists == 3) {
							bb_error_सन्देशः(fmt, "non-", "");
							गच्छ RET_1;
						}
					}
					यदि (unlink(dest) < 0) {
						bb_perror_सन्देशः("can't remove '%s'", dest);
						गच्छ RET_1;
					}
				}
				/* FILEUTILS_RECUR also prevents nasties like
				 * "read from device and write contents to dst"
				 * instead of "create same device node" */
				copy_flag = FILEUTILS_RECUR | FILEUTILS_PRESERVE_स्थितिः;
#यदि ENABLE_SELINUX
				copy_flag |= FILEUTILS_PRESERVE_SECURITY_CONTEXT;
#यदि_अन्तः
				यदि ((copy_file(*argv, dest, copy_flag) >= 0)
				 && (remove_file(*argv, FILEUTILS_RECUR | FILEUTILS_FORCE) >= 0)
				) {
					गच्छ RET_0;
				}
			}
 RET_1:
			स्थितिः = 1;
		}
 RET_0:
		यदि (flags & OPT_VERBOSE) {
			प्रारूप_लिख("'%s' -> '%s'\n", *argv, dest);
		}
		यदि (dest != चरमः) {
			मुक्त((शून्य *) dest);
		}
	} यावत् (*++argv && *argv != चरमः);

	प्रत्यागम स्थितिः;
}
