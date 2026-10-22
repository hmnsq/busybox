/*
 * Copyright (C) 2017 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see LICENSE in this source tree
 */
//config:config NPROC
//config:	bool "nproc (3.9 kb)"
//config:	default y
//config:	help
//config:	Print number of CPUs

//applet:IF_NPROC(APPLET_NOFORK(nproc, nproc, BB_DIR_USR_BIN, BB_SUID_DROP, nproc))

//kbuild:lib-$(CONFIG_NPROC) += nproc.o

//usage:#define nproc_trivial_usage
//usage:	""IF_LONG_OPTS("[--all] [--ignore=N]")
//usage:#define nproc_full_usage "\n\n"
//usage:	"Print number of available CPUs"
//usage:	IF_LONG_OPTS(
//usage:     "\n"
//usage:     "\n	--all		Number of installed CPUs"
//usage:     "\n	--ignore=N	Exclude N CPUs"
//usage:	)

#شامل "libbb.h"

صحیح nproc_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح nproc_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv غیر_مستعمل_PARAM)
{
	صحیح گنتی = 0;
#اگر ENABLE_LONG_OPTS
	صحیح ignore = 0;
	صحیح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_دلیل "\xfe"
			"all\0"    No_دلیل       "\xff"
			, &ignore
	);

	اگر (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		اگر (cpusd) {
			ساخت dirent *de;
			جب (تہی != (de = readdir(cpusd))) {
				حرف *cpuid = لڑی_تلاش(de->d_name, "cpu");
				اگر (cpuid && ہندسہ_ہے(cpuid[لڑی_لمبائی(cpuid) - 1]))
					گنتی++;
			}
			IF_FEATURE_CLEAN_اوپر(closedir(cpusd);)
		}
	} ورنہ
#اگر_ختم
	{
		صحیح i;
		غیرعلامتی sz = 2 * 1024;
		غیرعلامتی طویل *نقاب = get_malloc_cpu_affinity(0, &sz);
		sz /= حجم(طویل);
		برائے (i = 0; i < sz; i++) {
			اگر (نقاب[i] != 0) /* most mask[i] are usually 0 */
				گنتی += bb_popcnt_long(نقاب[i]);
		}
		IF_FEATURE_CLEAN_اوپر(آزاد(نقاب);)
	}

	IF_LONG_OPTS(گنتی -= ignore;)
	اگر (گنتی <= 0)
		گنتی = 1;

	لکھو("%u\n", گنتی);

	واپس 0;
}
