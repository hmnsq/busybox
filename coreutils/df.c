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

#شامل <mntent.h>
#شامل <sys/statvfs.h>
#شامل "libbb.h"
#شامل "unicode.h"

#اگر !ENABLE_FEATURE_HUMAN_READABLE
ساکن غیرعلامتی طویل kscale(غیرعلامتی طویل b, غیرعلامتی طویل bs)
{
	واپس (b * (غیرعلامتی طویل طویل) bs + 1024/2) / 1024;
}
#اگر_ختم

صحیح df_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح df_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	غیرعلامتی طویل df_disp_hr = 1024;
	exitcode_t حالت_2 = EXIT_SUCCESS;
	غیرعلامتی opt;
	FILE *mount_جدول;
	ساخت mntent *mount_اندراج;
	ساخت statvfs s;
	شمار {
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
	ثابت حرف *disp_units_hdr = تہی;
	حرف *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	اگر (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#اگر ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#وگرنہ_اگر ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#اگر_ختم
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	اگر (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	اگر (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		صحیح i;
		برائے (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			اگر (strcmp(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				جاؤ got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, ULONG_MAX, kmg_i_suffixes);
 got_it: ;
	}

	اگر (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	اگر (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	اگر (disp_units_hdr == تہی) {
#اگر ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#ورنہ
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#اگر_ختم
	}

	لکھو("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_جدول = تہی;
	argv += optind;
	اگر (!argv[0]) {
		mount_جدول = setmntent(bb_راستہ_mtab_file, "r");
		اگر (!mount_جدول)
			bb_simple_perror_پیغام_and_die(bb_راستہ_mtab_file);
	}

	جب (1) {
		ثابت حرف *device;
		ثابت حرف *mount_نقطہ;
		ثابت حرف *fs_type;

		اگر (mount_جدول) {
			mount_اندراج = getmntent(mount_جدول);
			اگر (!mount_اندراج) {
				endmntent(mount_جدول);
				توڑو;
			}
		} ورنہ {
			mount_نقطہ = *argv++;
			اگر (!mount_نقطہ)
				توڑو;
			mount_اندراج = ڈھونڈو_mount_نقطہ(mount_نقطہ, 1);
			اگر (!mount_اندراج) {
				bb_error_پیغام("%s: can't find mount point", mount_نقطہ);
 سیٹ_error:
				حالت_2 = EXIT_FAILURE;
				جاری;
			}
		}

		device = mount_اندراج->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		اگر (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(device, "rootfs") == 0)
			جاری;

		mount_نقطہ = mount_اندراج->mnt_dir;
		fs_type = mount_اندراج->mnt_type;

		اگر (opt & OPT_t) {
			اگر (strcmp(fs_type, opt_t) != 0)
				جاری;
		}

		اگر (statvfs(mount_نقطہ, &s) != 0) {
			bb_simple_perror_پیغام(mount_نقطہ);
			جاؤ سیٹ_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		اگر (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		اگر ((s.f_blocks > 0) || !mount_جدول || (opt & OPT_ALL)) {
			غیرعلامتی طویل طویل بلاکیں_مستعمل;
			غیرعلامتی طویل طویل بلاکیں_کل;
			غیرعلامتی بلاکیں_percent_مستعمل;

			اگر (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				اگر (df_disp_hr)
					df_disp_hr = 1;
			}
			بلاکیں_مستعمل = s.f_blocks - s.f_bfree;
			بلاکیں_کل = بلاکیں_مستعمل + s.f_bavail;
			بلاکیں_percent_مستعمل = بلاکیں_کل; /* 0% if blocks_total == 0, else... */
			اگر (بلاکیں_کل != 0) {
				/* Downscale sizes for narrower division */
				غیرعلامتی u;
				جب (بلاکیں_کل >= زیادہ_سے_زیادہ_عدد / 101) {
					بلاکیں_کل >>= 1;
					بلاکیں_مستعمل >>= 1;
				}
				u = (غیرعلامتی)بلاکیں_مستعمل * 100u + (غیرعلامتی)بلاکیں_کل / 2;
				بلاکیں_percent_مستعمل = u / (غیرعلامتی)بلاکیں_کل;
			}

#اگر_تعریف_شدہ WHY_WE_DO_IT_FOR_DEV_جڑ_ONLY
			اگر (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = ڈھونڈو_بلاک_device("/");
				اگر (!device) {
					جاؤ سیٹ_error;
				}
			}
#اگر_ختم

#اگر ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				حرف *uni_dev = unicode_conv_مقصد_printable(&uni_stat, device);
				اگر (uni_stat.unicode_چوڑائی > 20 && !(opt & OPT_POSIX)) {
					لکھو("%s\n%20s", uni_dev, "");
				} ورنہ {
					لکھو("%s%*s", uni_dev, 20 - (صحیح)uni_stat.unicode_چوڑائی, "");
				}
				آزاد(uni_dev);
				اگر (opt & OPT_FSTYPE) {
					حرف *uni_type = unicode_conv_مقصد_printable(&uni_stat, fs_type);
					اگر (uni_stat.unicode_چوڑائی > 10 && !(opt & OPT_POSIX))
						لکھو(" %s\n%31s", uni_type, "");
					ورنہ
						لکھو(" %s%*s", uni_type, 10 - (صحیح)uni_stat.unicode_چوڑائی, "");
					آزاد(uni_type);
				}
			}
#ورنہ
			اگر (لکھو("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				لکھو("\n%-20s", "");
			اگر (opt & OPT_FSTYPE) {
				اگر (لکھو(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					لکھو("\n%-30s", "");
			}
#اگر_ختم

#اگر ENABLE_FEATURE_HUMAN_READABLE
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
				بلاکیں_percent_مستعمل, mount_نقطہ);
#ورنہ
			لکھو(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				بلاکیں_percent_مستعمل, mount_نقطہ);
#اگر_ختم
		}
	}

	واپس حالت_2;
}
