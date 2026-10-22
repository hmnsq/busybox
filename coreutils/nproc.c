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

صحیح nproc_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح nproc_main(صحیح argc UNUSED_PARAM, نویسه **argv UNUSED_PARAM)
{
	صحیح شمار = 0;
#اگر ENABLE_LONG_OPTS
	صحیح ignore = 0;
	صحیح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	اگر (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		اگر (cpusd) {
			ساختار dirent *de;
			وقتی (پوچ != (de = readdir(cpusd))) {
				نویسه *cpuid = strstr(de->d_name, "cpu");
				اگر (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					شمار++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} وگرنه
#endif
	{
		صحیح i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= اندازه(long);
		برای (i = 0; i < sz; i++) {
			اگر (mask[i] != 0) /* most mask[i] are usually 0 */
				شمار += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(آزاد(mask);)
	}

	IF_LONG_OPTS(شمار -= ignore;)
	اگر (شمار <= 0)
		شمار = 1;

	printf("%u\n", شمار);

	بازگشت 0;
}
