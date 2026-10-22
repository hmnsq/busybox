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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क nproc_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क nproc_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv UNUSED_PARAM)
{
	पूर्णाङ्क गणनम् = 0;
#जइ ENABLE_LONG_OPTS
	पूर्णाङ्क ignore = 0;
	पूर्णाङ्क opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	जइ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		जइ (cpusd) {
			संरचना dirent *de;
			जाव (रिक्त != (de = readdir(cpusd))) {
				वर्ण *cpuid = strstr(de->d_name, "cpu");
				जइ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					गणनम्++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} अण्णहा
#endif
	{
		पूर्णाङ्क i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= परिमाण(long);
		प्रति (i = 0; i < sz; i++) {
			जइ (mask[i] != 0) /* most mask[i] are usually 0 */
				गणनम् += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(मुक्त(mask);)
	}

	IF_LONG_OPTS(गणनम् -= ignore;)
	जइ (गणनम् <= 0)
		गणनम् = 1;

	printf("%u\n", गणनम्);

	प्रत्यागम 0;
}
