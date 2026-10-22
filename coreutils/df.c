/* vi: set sw=4 ts=4: */
/*
 * Mini df implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * based on original code by (I think) Bruce Perens <bruce@pixar.com>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reduction.  Removed floating point dependency.  Added error checking
 * on output.  Output stats on 0-sized filesystems if specifically listed on
 * the command line.  Properly round *-blocks, Used, and Available quantities.
 *
 * Aug 28, 2008      Bernhard Reutner-Fischer
 *
 * Implement -P and -B; better coreutils compat; cleanup
 */
//config:config DF
//config:	bool "df (7.1 kb)"
//config:	default y
//config:	help
//config:	df reports the amount of disk space used and available
//config:	on filesystems.
//config:
//config:config FEATURE_DF_FANCY
//config:	bool "Enable -a, -i, -B"
//config:	default y
//config:	depends on DF
//config:	help
//config:	-a Show all filesystems
//config:	-i Inodes
//config:	-B <SIZE> Blocksize
//config:
//config:config FEATURE_SKIP_ROOTFS
//config:	bool "Skip rootfs in mount table"
//config:	default y
//config:	depends on DF
//config:	help
//config:	Ignore rootfs entry in mount table.
//config:
//config:	In Linux, kernel has a special filesystem, rootfs, which is initially
//config:	mounted on /. It contains initramfs data, if kernel is configured
//config:	to have one. Usually, another file system is mounted over / early
//config:	in boot process, and therefore most tools which manipulate
//config:	mount table, such as df, will skip rootfs entry.
//config:
//config:	However, some systems do not mount anything on /.
//config:	If you need to configure busybox for one of these systems,
//config:	you may find it useful to turn this option off to make df show
//config:	initramfs statistics.
//config:
//config:	Otherwise, choose Y.

//applet:IF_DF(APPLET_NOEXEC(df, df, BB_DIR_BIN, BB_SUID_DROP, df))

//kbuild:lib-$(CONFIG_DF) += df.o

/* BB_AUDIT SUSv3 _NOT_ compliant -- option -t missing. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/df.html */

//usage:#define df_trivial_usage
//usage:	"[-Pk"
//usage:	IF_FEATURE_HUMAN_READABLE("mh")
//usage:	"T"
//usage:	IF_FEATURE_DF_FANCY("ai] [-B SIZE")
//usage:	"] [-t TYPE] [FILESYSTEM]..."
//usage:#define df_full_usage "\n\n"
//usage:       "Print filesystem usage statistics\n"
//usage:     "\n	-P	POSIX output format"
//usage:     "\n	-k	1024-byte blocks (default)"
//usage:	IF_FEATURE_HUMAN_READABLE(
//usage:     "\n	-m	1M-byte blocks"
//usage:     "\n	-h	Human readable (e.g. 1K 243M 2G)"
//usage:	)
//usage:     "\n	-T	Print filesystem type"
//usage:     "\n	-t TYPE	Print only mounts of this type"
//usage:	IF_FEATURE_DF_FANCY(
//usage:     "\n	-a	Show all filesystems"
//usage:     "\n	-i	Inodes"
//usage:     "\n	-B SIZE	Blocksize"
//usage:	)
//usage:
//usage:#define df_example_usage
//usage:       "$ df\n"
//usage:       "Filesystem           1K-blocks      Used Available Use% Mounted on\n"
//usage:       "/dev/sda3              8690864   8553540    137324  98% /\n"
//usage:       "/dev/sda1                64216     36364     27852  57% /boot\n"
//usage:       "$ df /dev/sda3\n"
//usage:       "Filesystem           1K-blocks      Used Available Use% Mounted on\n"
//usage:       "/dev/sda3              8690864   8553540    137324  98% /\n"
//usage:       "$ POSIXLY_CORRECT=sure df /dev/sda3\n"
//usage:       "Filesystem         512B-blocks      Used Available Use% Mounted on\n"
//usage:       "/dev/sda3             17381728  17107080    274648  98% /\n"
//usage:       "$ POSIXLY_CORRECT=yep df -P /dev/sda3\n"
//usage:       "Filesystem          512-blocks      Used Available Capacity Mounted on\n"
//usage:       "/dev/sda3             17381728  17107080    274648      98% /\n"

#समावेश <mntent.h>
#समावेश <sys/statvfs.h>
#समावेश "libbb.h"
#समावेश "unicode.h"

#अगर !ENABLE_FEATURE_HUMAN_READABLE
अटल अचिन्हित दीर्घ kscale(अचिन्हित दीर्घ b, अचिन्हित दीर्घ bs)
{
	वापस (b * (अचिन्हित दीर्घ दीर्घ) bs + 1024/2) / 1024;
}
#पूर्ण_अगर

पूर्णांक df_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक df_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अचिन्हित दीर्घ df_disp_hr = 1024;
	exitcode_t status = निकास_सफल;
	अचिन्हित opt;
	खाता *mount_table;
	काष्ठा mntent *mount_entry;
	काष्ठा statvfs s;
	क्रमागत {
		OPT_KILO     = (1 << 0),
		OPT_POSIX    = (1 << 1),
		OPT_FSTYPE   = (1 << 2),
		OPT_t        = (1 << 3),
		OPT_ALL      = (1 << 4) * ENABLE_FEATURE_DF_FANCY,
		OPT_INODE    = (1 << 5) * ENABLE_FEATURE_DF_FANCY,
		OPT_BSIZE    = (1 << 6) * ENABLE_FEATURE_DF_FANCY,
		OPT_HUMAN    = (1 << (4 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
		OPT_HUMANDEC = (1 << (5 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
		OPT_MEGA     = (1 << (6 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
	};
	स्थिर अक्षर *disp_units_hdr = शून्य;
	अक्षर *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	अगर (दो_पर्या("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#अगर ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#या_अगर ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#पूर्ण_अगर
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	अगर (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	अगर (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		पूर्णांक i;
		क्रम (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			अगर (म_भेद(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				जाओ got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, अच_दीर्घ_उच्च, kmg_i_suffixes);
 got_it: ;
	}

	अगर (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	अगर (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	अगर (disp_units_hdr == शून्य) {
#अगर ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#अन्यथा
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#पूर्ण_अगर
	}

	म_लिखो("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_table = शून्य;
	argv += optind;
	अगर (!argv[0]) {
		mount_table = setmntent(bb_path_mtab_file, "r");
		अगर (!mount_table)
			bb_simple_perror_संदेश_and_die(bb_path_mtab_file);
	}

	जबतक (1) {
		स्थिर अक्षर *device;
		स्थिर अक्षर *mount_point;
		स्थिर अक्षर *fs_type;

		अगर (mount_table) {
			mount_entry = getmntent(mount_table);
			अगर (!mount_entry) {
				endmntent(mount_table);
				अवरोध;
			}
		} अन्यथा {
			mount_point = *argv++;
			अगर (!mount_point)
				अवरोध;
			mount_entry = find_mount_point(mount_point, 1);
			अगर (!mount_entry) {
				bb_error_संदेश("%s: can't find mount point", mount_point);
 set_error:
				status = निकास_त्रुटि;
				जारी;
			}
		}

		device = mount_entry->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		अगर (ENABLE_FEATURE_SKIP_ROOTFS && म_भेद(device, "rootfs") == 0)
			जारी;

		mount_point = mount_entry->mnt_dir;
		fs_type = mount_entry->mnt_type;

		अगर (opt & OPT_t) {
			अगर (म_भेद(fs_type, opt_t) != 0)
				जारी;
		}

		अगर (statvfs(mount_point, &s) != 0) {
			bb_simple_perror_संदेश(mount_point);
			जाओ set_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		अगर (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		अगर ((s.f_blocks > 0) || !mount_table || (opt & OPT_ALL)) {
			अचिन्हित दीर्घ दीर्घ blocks_used;
			अचिन्हित दीर्घ दीर्घ blocks_योग;
			अचिन्हित blocks_percent_used;

			अगर (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				अगर (df_disp_hr)
					df_disp_hr = 1;
			}
			blocks_used = s.f_blocks - s.f_bfree;
			blocks_योग = blocks_used + s.f_bavail;
			blocks_percent_used = blocks_योग; /* 0% if blocks_total == 0, else... */
			अगर (blocks_योग != 0) {
				/* Downscale sizes for narrower division */
				अचिन्हित u;
				जबतक (blocks_योग >= पूर्णांक_उच्च / 101) {
					blocks_योग >>= 1;
					blocks_used >>= 1;
				}
				u = (अचिन्हित)blocks_used * 100u + (अचिन्हित)blocks_योग / 2;
				blocks_percent_used = u / (अचिन्हित)blocks_योग;
			}

#अगर_घोषित WHY_WE_DO_IT_FOR_DEV_मूल_ONLY
			अगर (म_भेद(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = find_block_device("/");
				अगर (!device) {
					जाओ set_error;
				}
			}
#पूर्ण_अगर

#अगर ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				अक्षर *uni_dev = unicode_conv_to_printable(&uni_stat, device);
				अगर (uni_stat.unicode_width > 20 && !(opt & OPT_POSIX)) {
					म_लिखो("%s\n%20s", uni_dev, "");
				} अन्यथा {
					म_लिखो("%s%*s", uni_dev, 20 - (पूर्णांक)uni_stat.unicode_width, "");
				}
				मुक्त(uni_dev);
				अगर (opt & OPT_FSTYPE) {
					अक्षर *uni_type = unicode_conv_to_printable(&uni_stat, fs_type);
					अगर (uni_stat.unicode_width > 10 && !(opt & OPT_POSIX))
						म_लिखो(" %s\n%31s", uni_type, "");
					अन्यथा
						म_लिखो(" %s%*s", uni_type, 10 - (पूर्णांक)uni_stat.unicode_width, "");
					मुक्त(uni_type);
				}
			}
#अन्यथा
			अगर (म_लिखो("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				म_लिखो("\n%-20s", "");
			अगर (opt & OPT_FSTYPE) {
				अगर (म_लिखो(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					म_लिखो("\n%-30s", "");
			}
#पूर्ण_अगर

#अगर ENABLE_FEATURE_HUMAN_READABLE
			म_लिखो(" %9s ",
				/* f_blocks x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_blocks, s.f_frsize, df_disp_hr));

			म_लिखो(" %9s " + 1,
				/* EXPR x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_frsize, df_disp_hr));

			म_लिखो("%9s %3u%% %s\n",
				/* f_bavail x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_bavail, s.f_frsize, df_disp_hr),
				blocks_percent_used, mount_point);
#अन्यथा
			म_लिखो(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				blocks_percent_used, mount_point);
#पूर्ण_अगर
		}
	}

	वापस status;
}
