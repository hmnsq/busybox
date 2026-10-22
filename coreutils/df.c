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

#ضمن <mntent.h>
#ضمن <sys/statvfs.h>
#ضمن "libbb.h"
#ضمن "unicode.h"

#إذا !ENABLE_FEATURE_HUMAN_READABLE
ساكن بنانشانی لما kscale(بنانشانی لما b, بنانشانی لما bs)
{
	أرجع (b * (بنانشانی لما لما) bs + 1024/2) / 1024;
}
#جے_انت

صحيح df_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح df_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	بنانشانی لما df_disp_hr = 1024;
	exitcode_t حالت = EXIT_SUCCESS;
	بنانشانی opt;
	FILE *mount_جدول;
	بنية mntent *mount_اندراج;
	بنية statvfs s;
	تعداد {
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
	ثابت حرف *disp_units_hdr = عدم;
	حرف *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	إذا (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#إذا ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#وإلاإذا ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#جے_انت
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	إذا (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	إذا (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		صحيح i;
		لكل (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			إذا (strcmp(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				اذهب got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, ULONG_MAX, kmg_i_suffixes);
 got_it: ;
	}

	إذا (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	إذا (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	إذا (disp_units_hdr == عدم) {
#إذا ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#وإلا
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#جے_انت
	}

	لکھو("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_جدول = عدم;
	argv += optind;
	إذا (!argv[0]) {
		mount_جدول = setmntent(bb_رستہ_mtab_file, "r");
		إذا (!mount_جدول)
			bb_simple_perror_پیغام_and_die(bb_رستہ_mtab_file);
	}

	طالما (1) {
		ثابت حرف *device;
		ثابت حرف *mount_نقطہ;
		ثابت حرف *fs_type;

		إذا (mount_جدول) {
			mount_اندراج = getmntent(mount_جدول);
			إذا (!mount_اندراج) {
				endmntent(mount_جدول);
				اكسر;
			}
		} وإلا {
			mount_نقطہ = *argv++;
			إذا (!mount_نقطہ)
				اكسر;
			mount_اندراج = لبھو_mount_نقطہ(mount_نقطہ, 1);
			إذا (!mount_اندراج) {
				bb_error_پیغام("%s: can't find mount point", mount_نقطہ);
 سیٹ_error:
				حالت = EXIT_FAILURE;
				استمر;
			}
		}

		device = mount_اندراج->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		إذا (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(device, "rootfs") == 0)
			استمر;

		mount_نقطہ = mount_اندراج->mnt_dir;
		fs_type = mount_اندراج->mnt_type;

		إذا (opt & OPT_t) {
			إذا (strcmp(fs_type, opt_t) != 0)
				استمر;
		}

		إذا (statvfs(mount_نقطہ, &s) != 0) {
			bb_simple_perror_پیغام(mount_نقطہ);
			اذهب سیٹ_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		إذا (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		إذا ((s.f_blocks > 0) || !mount_جدول || (opt & OPT_ALL)) {
			بنانشانی لما لما بلاکاں_ورتیا;
			بنانشانی لما لما بلاکاں_کل;
			بنانشانی بلاکاں_percent_ورتیا;

			إذا (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				إذا (df_disp_hr)
					df_disp_hr = 1;
			}
			بلاکاں_ورتیا = s.f_blocks - s.f_bfree;
			بلاکاں_کل = بلاکاں_ورتیا + s.f_bavail;
			بلاکاں_percent_ورتیا = بلاکاں_کل; /* 0% if blocks_total == 0, else... */
			إذا (بلاکاں_کل != 0) {
				/* Downscale sizes for narrower division */
				بنانشانی u;
				طالما (بلاکاں_کل >= ودھ_توں_ودھ_عدد / 101) {
					بلاکاں_کل >>= 1;
					بلاکاں_ورتیا >>= 1;
				}
				u = (بنانشانی)بلاکاں_ورتیا * 100u + (بنانشانی)بلاکاں_کل / 2;
				بلاکاں_percent_ورتیا = u / (بنانشانی)بلاکاں_کل;
			}

#جے_تعریف_شدہ WHY_WE_DO_IT_FOR_DEV_جڑ_ONLY
			إذا (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = لبھو_بلاک_device("/");
				إذا (!device) {
					اذهب سیٹ_error;
				}
			}
#جے_انت

#إذا ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				حرف *uni_dev = unicode_conv_منزل_printable(&uni_stat, device);
				إذا (uni_stat.unicode_چوڑائی > 20 && !(opt & OPT_POSIX)) {
					لکھو("%s\n%20s", uni_dev, "");
				} وإلا {
					لکھو("%s%*s", uni_dev, 20 - (صحيح)uni_stat.unicode_چوڑائی, "");
				}
				حرر(uni_dev);
				إذا (opt & OPT_FSTYPE) {
					حرف *uni_type = unicode_conv_منزل_printable(&uni_stat, fs_type);
					إذا (uni_stat.unicode_چوڑائی > 10 && !(opt & OPT_POSIX))
						لکھو(" %s\n%31s", uni_type, "");
					وإلا
						لکھو(" %s%*s", uni_type, 10 - (صحيح)uni_stat.unicode_چوڑائی, "");
					حرر(uni_type);
				}
			}
#وإلا
			إذا (لکھو("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				لکھو("\n%-20s", "");
			إذا (opt & OPT_FSTYPE) {
				إذا (لکھو(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					لکھو("\n%-30s", "");
			}
#جے_انت

#إذا ENABLE_FEATURE_HUMAN_READABLE
			لکھو(" %9s ",
				/* f_blocks x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_blocks, s.f_frsize, df_disp_hr));

			لکھو(" %9s " + 1,
				/* EXPR x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_frsize, df_disp_hr));

			لکھو("%9s %3u%% %s\n",
				/* f_bavail x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_bavail, s.f_frsize, df_disp_hr),
				بلاکاں_percent_ورتیا, mount_نقطہ);
#وإلا
			لکھو(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				بلاکاں_percent_ورتیا, mount_نقطہ);
#جے_انت
		}
	}

	أرجع حالت;
}
