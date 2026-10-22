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

#include "libbb.h"

ཧྲིལ nproc_main(ཧྲིལ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ཧྲིལ nproc_main(ཧྲིལ argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	ཧྲིལ count = 0;
#གལ་སྲིད ENABLE_LONG_OPTS
	ཧྲིལ ignore = 0;
	ཧྲིལ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	གལ་སྲིད (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		གལ་སྲིད (cpusd) {
			sa_nracanaa dirent *de;
			སྐབས (NULL != (de = readdir(cpusd))) {
				var_na *cpuid = strstr(de->d_name, "cpu");
				གལ་སྲིད (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} དེ་མིན
#endif
	{
		ཧྲིལ i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= parimaa_na(long);
		དོན་ལུ (i = 0; i < sz; i++) {
			གལ་སྲིད (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	གལ་སྲིད (count <= 0)
		count = 1;

	printf("%u\n", count);

	ལོག 0;
}
