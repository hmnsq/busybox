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

số_nguyên nproc_main(số_nguyên argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
số_nguyên nproc_main(số_nguyên argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	số_nguyên count = 0;
#nếu ENABLE_LONG_OPTS
	số_nguyên ignore = 0;
	số_nguyên opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	nếu (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		nếu (cpusd) {
			struct dirent *de;
			trong_khi (NULL != (de = readdir(cpusd))) {
				char *cpuid = strstr(de->d_name, "cpu");
				nếu (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} không_thì
#endif
	{
		số_nguyên i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= sizeof(long);
		cho (i = 0; i < sz; i++) {
			nếu (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	nếu (count <= 0)
		count = 1;

	printf("%u\n", count);

	trả_về 0;
}
