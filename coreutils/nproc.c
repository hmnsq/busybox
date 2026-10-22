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

#অন্তর্ভাব "libbb.h"

পূর্ণসংখ্যা nproc_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা nproc_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv UNUSED_PARAM)
{
	পূর্ণসংখ্যা গুনতি = 0;
#যদি ENABLE_LONG_OPTS
	পূর্ণসংখ্যা ignore = 0;
	পূর্ণসংখ্যা opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	যদি (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		যদি (cpusd) {
			কাঠামো dirent *de;
			যখন (ফাঁকা != (de = readdir(cpusd))) {
				অক্ষর *cpuid = strstr(de->d_name, "cpu");
				যদি (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					গুনতি++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} অন্যথা
#endif
	{
		পূর্ণসংখ্যা i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= আকার(long);
		জন্য (i = 0; i < sz; i++) {
			যদি (mask[i] != 0) /* most mask[i] are usually 0 */
				গুনতি += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(মুক্ত(mask);)
	}

	IF_LONG_OPTS(গুনতি -= ignore;)
	যদি (গুনতি <= 0)
		গুনতি = 1;

	printf("%u\n", গুনতি);

	ফেরত 0;
}
