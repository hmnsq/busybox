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

#ಅನ್ತರ್ಭಾವ <mntent.h>
#ಅನ್ತರ್ಭಾವ <sys/statvfs.h>
#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "unicode.h"

#ಆದರೆ !ENABLE_FEATURE_HUMAN_READABLE
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ kscale(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ b, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bs)
{
	ಹಿಂತಿರುಗಿ (b * (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ) bs + 1024/2) / 1024;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ df_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ df_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ df_disp_hr = 1024;
	exitcode_t ಸ್ಥಿತಿ_2 = EXIT_SUCCESS;
	ಚಿಹ್ನೆರಹಿತ opt;
	FILE *mount_ಕೋಷ್ಟಕ;
	ರಚನೆ mntent *mount_ನಮೂದು;
	ರಚನೆ statvfs s;
	ಎಣಿಕೆ {
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
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *disp_units_hdr = ಖಾಲಿ;
	ಅಕ್ಷರ *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	ಆದರೆ (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#ಅಥವಾಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#ಆದರೆ_ಅಂತ್ಯ
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	ಆದರೆ (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	ಆದರೆ (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		ಪೂರ್ಣಾಂಕ i;
		ಗಾಗಿ (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			ಆದರೆ (strcmp(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				ಹೋಗು got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, ULONG_MAX, kmg_i_suffixes);
 got_it: ;
	}

	ಆದರೆ (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	ಆದರೆ (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	ಆದರೆ (disp_units_hdr == ಖಾಲಿ) {
#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#ಇಲ್ಲದಿದ್ದರೆ
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಬರೆ("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_ಕೋಷ್ಟಕ = ಖಾಲಿ;
	argv += optind;
	ಆದರೆ (!argv[0]) {
		mount_ಕೋಷ್ಟಕ = setmntent(bb_ಮಾರ್ಗ_mtab_file, "r");
		ಆದರೆ (!mount_ಕೋಷ್ಟಕ)
			bb_simple_perror_ಸಂದೇಶ_and_die(bb_ಮಾರ್ಗ_mtab_file);
	}

	ಆಗ (1) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mount_ಬಿಂದು;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fs_type;

		ಆದರೆ (mount_ಕೋಷ್ಟಕ) {
			mount_ನಮೂದು = getmntent(mount_ಕೋಷ್ಟಕ);
			ಆದರೆ (!mount_ನಮೂದು) {
				endmntent(mount_ಕೋಷ್ಟಕ);
				ನಿಲ್ಲಿಸು;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			mount_ಬಿಂದು = *argv++;
			ಆದರೆ (!mount_ಬಿಂದು)
				ನಿಲ್ಲಿಸು;
			mount_ನಮೂದು = ಕಂಡುಹಿಡಿ_mount_ಬಿಂದು(mount_ಬಿಂದು, 1);
			ಆದರೆ (!mount_ನಮೂದು) {
				bb_error_ಸಂದೇಶ("%s: can't find mount point", mount_ಬಿಂದು);
 ಗಣ_error:
				ಸ್ಥಿತಿ_2 = EXIT_FAILURE;
				ಮುಂದುವರಿ;
			}
		}

		device = mount_ನಮೂದು->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		ಆದರೆ (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(device, "rootfs") == 0)
			ಮುಂದುವರಿ;

		mount_ಬಿಂದು = mount_ನಮೂದು->mnt_dir;
		fs_type = mount_ನಮೂದು->mnt_type;

		ಆದರೆ (opt & OPT_t) {
			ಆದರೆ (strcmp(fs_type, opt_t) != 0)
				ಮುಂದುವರಿ;
		}

		ಆದರೆ (statvfs(mount_ಬಿಂದು, &s) != 0) {
			bb_simple_perror_ಸಂದೇಶ(mount_ಬಿಂದು);
			ಹೋಗು ಗಣ_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		ಆದರೆ (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		ಆದರೆ ((s.f_blocks > 0) || !mount_ಕೋಷ್ಟಕ || (opt & OPT_ALL)) {
			ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಖಂಡಗಳು_ಬಳಸಿದ;
			ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಖಂಡಗಳು_ಒಟ್ಟು;
			ಚಿಹ್ನೆರಹಿತ ಖಂಡಗಳು_percent_ಬಳಸಿದ;

			ಆದರೆ (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				ಆದರೆ (df_disp_hr)
					df_disp_hr = 1;
			}
			ಖಂಡಗಳು_ಬಳಸಿದ = s.f_blocks - s.f_bfree;
			ಖಂಡಗಳು_ಒಟ್ಟು = ಖಂಡಗಳು_ಬಳಸಿದ + s.f_bavail;
			ಖಂಡಗಳು_percent_ಬಳಸಿದ = ಖಂಡಗಳು_ಒಟ್ಟು; /* 0% if blocks_total == 0, else... */
			ಆದರೆ (ಖಂಡಗಳು_ಒಟ್ಟು != 0) {
				/* Downscale sizes for narrower division */
				ಚಿಹ್ನೆರಹಿತ u;
				ಆಗ (ಖಂಡಗಳು_ಒಟ್ಟು >= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 101) {
					ಖಂಡಗಳು_ಒಟ್ಟು >>= 1;
					ಖಂಡಗಳು_ಬಳಸಿದ >>= 1;
				}
				u = (ಚಿಹ್ನೆರಹಿತ)ಖಂಡಗಳು_ಬಳಸಿದ * 100u + (ಚಿಹ್ನೆರಹಿತ)ಖಂಡಗಳು_ಒಟ್ಟು / 2;
				ಖಂಡಗಳು_percent_ಬಳಸಿದ = u / (ಚಿಹ್ನೆರಹಿತ)ಖಂಡಗಳು_ಒಟ್ಟು;
			}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHY_WE_DO_IT_FOR_DEV_ಬೇರು_ONLY
			ಆದರೆ (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = ಕಂಡುಹಿಡಿ_ಖಂಡ_device("/");
				ಆದರೆ (!device) {
					ಹೋಗು ಗಣ_error;
				}
			}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				ಅಕ್ಷರ *uni_dev = unicode_conv_ಗಮ್ಯ_printable(&uni_stat, device);
				ಆದರೆ (uni_stat.unicode_ಅಗಲ > 20 && !(opt & OPT_POSIX)) {
					ಬರೆ("%s\n%20s", uni_dev, "");
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಬರೆ("%s%*s", uni_dev, 20 - (ಪೂರ್ಣಾಂಕ)uni_stat.unicode_ಅಗಲ, "");
				}
				ಬಿಡುಗಡೆ(uni_dev);
				ಆದರೆ (opt & OPT_FSTYPE) {
					ಅಕ್ಷರ *uni_type = unicode_conv_ಗಮ್ಯ_printable(&uni_stat, fs_type);
					ಆದರೆ (uni_stat.unicode_ಅಗಲ > 10 && !(opt & OPT_POSIX))
						ಬರೆ(" %s\n%31s", uni_type, "");
					ಇಲ್ಲದಿದ್ದರೆ
						ಬರೆ(" %s%*s", uni_type, 10 - (ಪೂರ್ಣಾಂಕ)uni_stat.unicode_ಅಗಲ, "");
					ಬಿಡುಗಡೆ(uni_type);
				}
			}
#ಇಲ್ಲದಿದ್ದರೆ
			ಆದರೆ (ಬರೆ("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				ಬರೆ("\n%-20s", "");
			ಆದರೆ (opt & OPT_FSTYPE) {
				ಆದರೆ (ಬರೆ(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					ಬರೆ("\n%-30s", "");
			}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_HUMAN_READABLE
			ಬರೆ(" %9s ",
				/* f_blocks x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_blocks, s.f_frsize, df_disp_hr));

			ಬರೆ(" %9s " + 1,
				/* EXPR x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_frsize, df_disp_hr));

			ಬರೆ("%9s %3u%% %s\n",
				/* f_bavail x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_bavail, s.f_frsize, df_disp_hr),
				ಖಂಡಗಳು_percent_ಬಳಸಿದ, mount_ಬಿಂದು);
#ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				ಖಂಡಗಳು_percent_ಬಳಸಿದ, mount_ಬಿಂದು);
#ಆದರೆ_ಅಂತ್ಯ
		}
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}
