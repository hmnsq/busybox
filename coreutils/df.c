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

#అన్తర్భావ <mntent.h>
#అన్తర్భావ <sys/statvfs.h>
#అన్తర్భావ "libbb.h"
#అన్తర్భావ "unicode.h"

#ఒకవేళ !ENABLE_FEATURE_HUMAN_READABLE
స్థిర గుర్తులేని దీర్ఘ kscale(గుర్తులేని దీర్ఘ b, గుర్తులేని దీర్ఘ bs)
{
	తిరిగి (b * (గుర్తులేని దీర్ఘ దీర్ఘ) bs + 1024/2) / 1024;
}
#అయితే_ముగింపు

పూర్ణాంకం df_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం df_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	గుర్తులేని దీర్ఘ df_disp_hr = 1024;
	exitcode_t స్థితి_2 = EXIT_SUCCESS;
	గుర్తులేని opt;
	FILE *mount_పట్టిక;
	నిర్మాణం mntent *mount_నమోదు;
	నిర్మాణం statvfs s;
	గణన {
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
	స్థిరరాశి అక్షరం *disp_units_hdr = ఖాళీ;
	అక్షరం *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	ఒకవేళ (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#లేదావేళ ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#అయితే_ముగింపు
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	ఒకవేళ (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	ఒకవేళ (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		పూర్ణాంకం i;
		కొరకు (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			ఒకవేళ (strcmp(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				వెళ్ళు got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, ULONG_MAX, kmg_i_suffixes);
 got_it: ;
	}

	ఒకవేళ (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	ఒకవేళ (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	ఒకవేళ (disp_units_hdr == ఖాళీ) {
#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#లేకపోతే
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#అయితే_ముగింపు
	}

	రాయి("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_పట్టిక = ఖాళీ;
	argv += optind;
	ఒకవేళ (!argv[0]) {
		mount_పట్టిక = setmntent(bb_మార్గం_mtab_file, "r");
		ఒకవేళ (!mount_పట్టిక)
			bb_simple_perror_సందేశం_and_die(bb_మార్గం_mtab_file);
	}

	అయితే (1) {
		స్థిరరాశి అక్షరం *device;
		స్థిరరాశి అక్షరం *mount_బిందువు;
		స్థిరరాశి అక్షరం *fs_type;

		ఒకవేళ (mount_పట్టిక) {
			mount_నమోదు = getmntent(mount_పట్టిక);
			ఒకవేళ (!mount_నమోదు) {
				endmntent(mount_పట్టిక);
				ఆపు;
			}
		} లేకపోతే {
			mount_బిందువు = *argv++;
			ఒకవేళ (!mount_బిందువు)
				ఆపు;
			mount_నమోదు = కనుగొను_mount_బిందువు(mount_బిందువు, 1);
			ఒకవేళ (!mount_నమోదు) {
				bb_error_సందేశం("%s: can't find mount point", mount_బిందువు);
 సమితి_error:
				స్థితి_2 = EXIT_FAILURE;
				కొనసాగు;
			}
		}

		device = mount_నమోదు->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		ఒకవేళ (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(device, "rootfs") == 0)
			కొనసాగు;

		mount_బిందువు = mount_నమోదు->mnt_dir;
		fs_type = mount_నమోదు->mnt_type;

		ఒకవేళ (opt & OPT_t) {
			ఒకవేళ (strcmp(fs_type, opt_t) != 0)
				కొనసాగు;
		}

		ఒకవేళ (statvfs(mount_బిందువు, &s) != 0) {
			bb_simple_perror_సందేశం(mount_బిందువు);
			వెళ్ళు సమితి_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		ఒకవేళ (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		ఒకవేళ ((s.f_blocks > 0) || !mount_పట్టిక || (opt & OPT_ALL)) {
			గుర్తులేని దీర్ఘ దీర్ఘ ఖండాలు_వాడిన;
			గుర్తులేని దీర్ఘ దీర్ఘ ఖండాలు_మొత్తం;
			గుర్తులేని ఖండాలు_percent_వాడిన;

			ఒకవేళ (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				ఒకవేళ (df_disp_hr)
					df_disp_hr = 1;
			}
			ఖండాలు_వాడిన = s.f_blocks - s.f_bfree;
			ఖండాలు_మొత్తం = ఖండాలు_వాడిన + s.f_bavail;
			ఖండాలు_percent_వాడిన = ఖండాలు_మొత్తం; /* 0% if blocks_total == 0, else... */
			ఒకవేళ (ఖండాలు_మొత్తం != 0) {
				/* Downscale sizes for narrower division */
				గుర్తులేని u;
				అయితే (ఖండాలు_మొత్తం >= గరిష్ఠ_పూర్ణాంకం / 101) {
					ఖండాలు_మొత్తం >>= 1;
					ఖండాలు_వాడిన >>= 1;
				}
				u = (గుర్తులేని)ఖండాలు_వాడిన * 100u + (గుర్తులేని)ఖండాలు_మొత్తం / 2;
				ఖండాలు_percent_వాడిన = u / (గుర్తులేని)ఖండాలు_మొత్తం;
			}

#నిర్వచితమైతే WHY_WE_DO_IT_FOR_DEV_మూలం_ONLY
			ఒకవేళ (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = కనుగొను_ఖండం_device("/");
				ఒకవేళ (!device) {
					వెళ్ళు సమితి_error;
				}
			}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				అక్షరం *uni_dev = unicode_conv_గమ్యం_printable(&uni_stat, device);
				ఒకవేళ (uni_stat.unicode_వెడల్పు > 20 && !(opt & OPT_POSIX)) {
					రాయి("%s\n%20s", uni_dev, "");
				} లేకపోతే {
					రాయి("%s%*s", uni_dev, 20 - (పూర్ణాంకం)uni_stat.unicode_వెడల్పు, "");
				}
				విడుదల(uni_dev);
				ఒకవేళ (opt & OPT_FSTYPE) {
					అక్షరం *uni_type = unicode_conv_గమ్యం_printable(&uni_stat, fs_type);
					ఒకవేళ (uni_stat.unicode_వెడల్పు > 10 && !(opt & OPT_POSIX))
						రాయి(" %s\n%31s", uni_type, "");
					లేకపోతే
						రాయి(" %s%*s", uni_type, 10 - (పూర్ణాంకం)uni_stat.unicode_వెడల్పు, "");
					విడుదల(uni_type);
				}
			}
#లేకపోతే
			ఒకవేళ (రాయి("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				రాయి("\n%-20s", "");
			ఒకవేళ (opt & OPT_FSTYPE) {
				ఒకవేళ (రాయి(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					రాయి("\n%-30s", "");
			}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_HUMAN_READABLE
			రాయి(" %9s ",
				/* f_blocks x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_blocks, s.f_frsize, df_disp_hr));

			రాయి(" %9s " + 1,
				/* EXPR x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_frsize, df_disp_hr));

			రాయి("%9s %3u%% %s\n",
				/* f_bavail x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_bavail, s.f_frsize, df_disp_hr),
				ఖండాలు_percent_వాడిన, mount_బిందువు);
#లేకపోతే
			రాయి(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				ఖండాలు_percent_వాడిన, mount_బిందువు);
#అయితే_ముగింపు
		}
	}

	తిరిగి స్థితి_2;
}
