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

int nproc_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int nproc_main(int argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	int बिलांग = 0;
#if ENABLE_LONG_OPTS
	int ignore = 0;
	int opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	if (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		if (cpusd) {
			struct dirent *de;
			while (NULL != (de = readdir(cpusd))) {
				char *cpuid = strstr(de->d_name, "cpu");
				if (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					बिलांग++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} else
#endif
	{
		int i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= sizeof(long);
		for (i = 0; i < sz; i++) {
			if (mask[i] != 0) /* most mask[i] are usually 0 */
				बिलांग += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(बिलांग -= ignore;)
	if (बिलांग <= 0)
		बिलांग = 1;

	printf("%u\n", बिलांग);

	return 0;
}
