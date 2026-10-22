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

#ضمن "libbb.h"

صحيح nproc_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح nproc_main(صحيح argc بن_ورتیا_PARAM, حرف **argv بن_ورتیا_PARAM)
{
	صحيح گنتی = 0;
#إذا ENABLE_LONG_OPTS
	صحيح ignore = 0;
	صحيح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_دلیل "\xfe"
			"all\0"    No_دلیل       "\xff"
			, &ignore
	);

	إذا (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		إذا (cpusd) {
			بنية dirent *de;
			طالما (عدم != (de = readdir(cpusd))) {
				حرف *cpuid = لڑی_کھوج(de->d_name, "cpu");
				إذا (cpuid && ہندسہ_اے(cpuid[لڑی_لمبائی(cpuid) - 1]))
					گنتی++;
			}
			IF_FEATURE_CLEAN_اتے(closedir(cpusd);)
		}
	} وإلا
#جے_انت
	{
		صحيح i;
		بنانشانی sz = 2 * 1024;
		بنانشانی لما *نقاب = get_malloc_cpu_affinity(0, &sz);
		sz /= حجم(لما);
		لكل (i = 0; i < sz; i++) {
			إذا (نقاب[i] != 0) /* most mask[i] are usually 0 */
				گنتی += bb_popcnt_long(نقاب[i]);
		}
		IF_FEATURE_CLEAN_اتے(حرر(نقاب);)
	}

	IF_LONG_OPTS(گنتی -= ignore;)
	إذا (گنتی <= 0)
		گنتی = 1;

	لکھو("%u\n", گنتی);

	أرجع 0;
}
