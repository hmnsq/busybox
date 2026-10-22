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

#ਅਨ੍ਤਰ੍ਭਾਵ <mntent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/statvfs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"

#ਜੇ !ENABLE_FEATURE_HUMAN_READABLE
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ kscale(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ b, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bs)
{
	ਵਾਪਸ (b * (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) bs + 1024/2) / 1024;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ df_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ df_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ df_disp_hr = 1024;
	exitcode_t ਸਥਿਤੀ = EXIT_SUCCESS;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt;
	FILE *mount_ਸਾਰਣੀ;
	ਬਣਤਰ mntent *mount_ਇੰਦਰਾਜ;
	ਬਣਤਰ statvfs s;
	ਗਿਣਤੀ {
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
	ਅਚਲ ਅੱਖਰ *disp_units_hdr = ਨਿਰਾ;
	ਅੱਖਰ *chp, *opt_t;

	init_unicode();

	/* From the manpage of df from coreutils-6.10:
	 * Disk space is shown in 1K blocks by default, unless the environment
	 * variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	 */
	ਜੇ (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	opt = getopt32(argv, "^"
			"kPTt:"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hHm")
			"\0"
#ਜੇ ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
			"k-mB:m-Bk:B-km"
#ਜਾਂਜੇ ENABLE_FEATURE_HUMAN_READABLE
			"k-m:m-k"
#ਜੇ_ਅੰਤ
			, &opt_t
			IF_FEATURE_DF_FANCY(, &chp)
	);
	ਜੇ (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	ਜੇ (opt & OPT_BSIZE) {
		/* GNU coreutils 8.25 accepts "-BMiB" form too */
		ਪੂਰਨ i;
		ਲਈ (i = 0; kmg_i_suffixes[i].suffix[0]; i++) {
			ਜੇ (strcmp(kmg_i_suffixes[i].suffix, chp) == 0) {
				df_disp_hr = kmg_i_suffixes[i].mult;
				ਜਾਓ got_it;
			}
		}
		/* Range used to disallow 0 */
		df_disp_hr = xatoul_range_sfx(chp, 1, ULONG_MAX, kmg_i_suffixes);
 got_it: ;
	}

	ਜੇ (opt & (OPT_HUMAN|OPT_HUMANDEC)) {
		df_disp_hr = 0;
//TODO: need to add support in make_human_readable_str() for "decimal human readable"
		//if (opt & OPT_HUMANDEC)
		//	df_disp_hr--;
		disp_units_hdr = "     Size";
	}
	ਜੇ (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	ਜੇ (disp_units_hdr == ਨਿਰਾ) {
#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			/* print df_disp_hr, show no fractionals,
			 * use suffixes if OPT_POSIX is set in opt */
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX))
		);
#ਨਹੀਂਤਾਂ
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#ਜੇ_ਅੰਤ
	}

	ਲਿਖੋ("Filesystem           %s%-15sUsed Available %s Mounted on\n",
			(opt & OPT_FSTYPE) ? "Type       " : "",
			disp_units_hdr,
			(opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_ਸਾਰਣੀ = ਨਿਰਾ;
	argv += optind;
	ਜੇ (!argv[0]) {
		mount_ਸਾਰਣੀ = setmntent(bb_ਰਸਤਾ_mtab_file, "r");
		ਜੇ (!mount_ਸਾਰਣੀ)
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(bb_ਰਸਤਾ_mtab_file);
	}

	ਜਦੋਂ (1) {
		ਅਚਲ ਅੱਖਰ *device;
		ਅਚਲ ਅੱਖਰ *mount_ਬਿੰਦੂ;
		ਅਚਲ ਅੱਖਰ *fs_type;

		ਜੇ (mount_ਸਾਰਣੀ) {
			mount_ਇੰਦਰਾਜ = getmntent(mount_ਸਾਰਣੀ);
			ਜੇ (!mount_ਇੰਦਰਾਜ) {
				endmntent(mount_ਸਾਰਣੀ);
				ਰੋਕੋ;
			}
		} ਨਹੀਂਤਾਂ {
			mount_ਬਿੰਦੂ = *argv++;
			ਜੇ (!mount_ਬਿੰਦੂ)
				ਰੋਕੋ;
			mount_ਇੰਦਰਾਜ = ਲੱਭੋ_mount_ਬਿੰਦੂ(mount_ਬਿੰਦੂ, 1);
			ਜੇ (!mount_ਇੰਦਰਾਜ) {
				bb_error_ਸੁਨੇਹਾ("%s: can't find mount point", mount_ਬਿੰਦੂ);
 ਸੈੱਟ_error:
				ਸਥਿਤੀ = EXIT_FAILURE;
				ਜਾਰੀ;
			}
		}

		device = mount_ਇੰਦਰਾਜ->mnt_fsname;

		/* GNU coreutils 6.10 skips certain mounts, try to be compatible */
		ਜੇ (ENABLE_FEATURE_SKIP_ROOTFS && strcmp(device, "rootfs") == 0)
			ਜਾਰੀ;

		mount_ਬਿੰਦੂ = mount_ਇੰਦਰਾਜ->mnt_dir;
		fs_type = mount_ਇੰਦਰਾਜ->mnt_type;

		ਜੇ (opt & OPT_t) {
			ਜੇ (strcmp(fs_type, opt_t) != 0)
				ਜਾਰੀ;
		}

		ਜੇ (statvfs(mount_ਬਿੰਦੂ, &s) != 0) {
			bb_simple_perror_ਸੁਨੇਹਾ(mount_ਬਿੰਦੂ);
			ਜਾਓ ਸੈੱਟ_error;
		}
		/* Some uclibc versions were seen to lose f_frsize
		 * (kernel does return it, but then uclibc does not copy it)
		 */
		ਜੇ (s.f_frsize == 0)
			s.f_frsize = s.f_bsize;

		ਜੇ ((s.f_blocks > 0) || !mount_ਸਾਰਣੀ || (opt & OPT_ALL)) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਖੰਡਾਂ_ਵਰਤਿਆ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਖੰਡਾਂ_ਕੁੱਲ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡਾਂ_percent_ਵਰਤਿਆ;

			ਜੇ (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_frsize = 1;
				ਜੇ (df_disp_hr)
					df_disp_hr = 1;
			}
			ਖੰਡਾਂ_ਵਰਤਿਆ = s.f_blocks - s.f_bfree;
			ਖੰਡਾਂ_ਕੁੱਲ = ਖੰਡਾਂ_ਵਰਤਿਆ + s.f_bavail;
			ਖੰਡਾਂ_percent_ਵਰਤਿਆ = ਖੰਡਾਂ_ਕੁੱਲ; /* 0% if blocks_total == 0, else... */
			ਜੇ (ਖੰਡਾਂ_ਕੁੱਲ != 0) {
				/* Downscale sizes for narrower division */
				ਬਿਨਾਨਿਸ਼ਾਨੀ u;
				ਜਦੋਂ (ਖੰਡਾਂ_ਕੁੱਲ >= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 101) {
					ਖੰਡਾਂ_ਕੁੱਲ >>= 1;
					ਖੰਡਾਂ_ਵਰਤਿਆ >>= 1;
				}
				u = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡਾਂ_ਵਰਤਿਆ * 100u + (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡਾਂ_ਕੁੱਲ / 2;
				ਖੰਡਾਂ_percent_ਵਰਤਿਆ = u / (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡਾਂ_ਕੁੱਲ;
			}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHY_WE_DO_IT_FOR_DEV_ਜੜ੍ਹ_ONLY
			ਜੇ (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				 * or leaves device alone if it can't find it */
				device = ਲੱਭੋ_ਖੰਡ_device("/");
				ਜੇ (!device) {
					ਜਾਓ ਸੈੱਟ_error;
				}
			}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_UNICODE_SUPPORT
			{
				uni_stat_t uni_stat;
				ਅੱਖਰ *uni_dev = unicode_conv_ਮੰਜ਼ਿਲ_printable(&uni_stat, device);
				ਜੇ (uni_stat.unicode_ਚੌੜਾਈ > 20 && !(opt & OPT_POSIX)) {
					ਲਿਖੋ("%s\n%20s", uni_dev, "");
				} ਨਹੀਂਤਾਂ {
					ਲਿਖੋ("%s%*s", uni_dev, 20 - (ਪੂਰਨ)uni_stat.unicode_ਚੌੜਾਈ, "");
				}
				ਮੁਕਤ(uni_dev);
				ਜੇ (opt & OPT_FSTYPE) {
					ਅੱਖਰ *uni_type = unicode_conv_ਮੰਜ਼ਿਲ_printable(&uni_stat, fs_type);
					ਜੇ (uni_stat.unicode_ਚੌੜਾਈ > 10 && !(opt & OPT_POSIX))
						ਲਿਖੋ(" %s\n%31s", uni_type, "");
					ਨਹੀਂਤਾਂ
						ਲਿਖੋ(" %s%*s", uni_type, 10 - (ਪੂਰਨ)uni_stat.unicode_ਚੌੜਾਈ, "");
					ਮੁਕਤ(uni_type);
				}
			}
#ਨਹੀਂਤਾਂ
			ਜੇ (ਲਿਖੋ("\n%-20s" + 1, device) > 20 && !(opt & OPT_POSIX))
				ਲਿਖੋ("\n%-20s", "");
			ਜੇ (opt & OPT_FSTYPE) {
				ਜੇ (ਲਿਖੋ(" %-10s", fs_type) > 11 && !(opt & OPT_POSIX))
					ਲਿਖੋ("\n%-30s", "");
			}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_HUMAN_READABLE
			ਲਿਖੋ(" %9s ",
				/* f_blocks x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_blocks, s.f_frsize, df_disp_hr));

			ਲਿਖੋ(" %9s " + 1,
				/* EXPR x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_frsize, df_disp_hr));

			ਲਿਖੋ("%9s %3u%% %s\n",
				/* f_bavail x f_frsize / df_disp_hr, show one fractional,
				 * use suffixes if df_disp_hr == 0 */
				make_human_readable_str(s.f_bavail, s.f_frsize, df_disp_hr),
				ਖੰਡਾਂ_percent_ਵਰਤਿਆ, mount_ਬਿੰਦੂ);
#ਨਹੀਂਤਾਂ
			ਲਿਖੋ(" %9lu %9lu %9lu %3u%% %s\n",
				kscale(s.f_blocks, s.f_frsize),
				kscale(s.f_blocks - s.f_bfree, s.f_frsize),
				kscale(s.f_bavail, s.f_frsize),
				ਖੰਡਾਂ_percent_ਵਰਤਿਆ, mount_ਬਿੰਦੂ);
#ਜੇ_ਅੰਤ
		}
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}
