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

#అన్తర్భావ "libbb.h"

పూర్ణాంకం nproc_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం nproc_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv వాడని_PARAM)
{
	పూర్ణాంకం లెక్క = 0;
#ఒకవేళ ENABLE_LONG_OPTS
	పూర్ణాంకం ignore = 0;
	పూర్ణాంకం opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_వాదం "\xfe"
			"all\0"    No_వాదం       "\xff"
			, &ignore
	);

	ఒకవేళ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ఒకవేళ (cpusd) {
			నిర్మాణం dirent *de;
			అయితే (ఖాళీ != (de = readdir(cpusd))) {
				అక్షరం *cpuid = అక్షరశ్రేణి_శోధన(de->d_name, "cpu");
				ఒకవేళ (cpuid && అంకెనా(cpuid[అక్షరశ్రేణి_పొడవు(cpuid) - 1]))
					లెక్క++;
			}
			IF_FEATURE_CLEAN_పైకి(closedir(cpusd);)
		}
	} లేకపోతే
#అయితే_ముగింపు
	{
		పూర్ణాంకం i;
		గుర్తులేని sz = 2 * 1024;
		గుర్తులేని దీర్ఘ *ముసుగు = get_malloc_cpu_affinity(0, &sz);
		sz /= పరిమాణం(దీర్ఘ);
		కొరకు (i = 0; i < sz; i++) {
			ఒకవేళ (ముసుగు[i] != 0) /* most mask[i] are usually 0 */
				లెక్క += bb_popcnt_long(ముసుగు[i]);
		}
		IF_FEATURE_CLEAN_పైకి(విడుదల(ముసుగు);)
	}

	IF_LONG_OPTS(లెక్క -= ignore;)
	ఒకవేళ (లెక్క <= 0)
		లెక్క = 1;

	రాయి("%u\n", లెక్క);

	తిరిగి 0;
}
