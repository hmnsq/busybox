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

ціле nproc_main(ціле argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
ціле nproc_main(ціле argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	ціле count = 0;
#якщо ENABLE_LONG_OPTS
	ціле ignore = 0;
	ціле opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	якщо (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		якщо (cpusd) {
			struct dirent *de;
			поки (NULL != (de = readdir(cpusd))) {
				char *cpuid = strstr(de->d_name, "cpu");
				якщо (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} інакше
#endif
	{
		ціле i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= sizeof(long);
		для (i = 0; i < sz; i++) {
			якщо (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	якщо (count <= 0)
		count = 1;

	printf("%u\n", count);

	повернути 0;
}
