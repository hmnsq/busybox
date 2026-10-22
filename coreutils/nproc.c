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

puur_naa_nka nproc_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka nproc_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	puur_naa_nka count = 0;
#ꦪꦼꦤ꧀ ENABLE_LONG_OPTS
	puur_naa_nka ignore = 0;
	puur_naa_nka opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	ꦪꦼꦤ꧀ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ꦪꦼꦤ꧀ (cpusd) {
			sa_nracanaa dirent *de;
			ꦱꦼꦢꦼꦭ (NULL != (de = readdir(cpusd))) {
				var_na *cpuid = strstr(de->d_name, "cpu");
				ꦪꦼꦤ꧀ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} ꦭꦶꦪ
#endif
	{
		puur_naa_nka i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= parimaa_na(long);
		ꦏꦁꦒꦺꦴ (i = 0; i < sz; i++) {
			ꦪꦼꦤ꧀ (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	ꦪꦼꦤ꧀ (count <= 0)
		count = 1;

	printf("%u\n", count);

	ꦮꦭꦶ 0;
}
