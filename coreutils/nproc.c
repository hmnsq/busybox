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

ကိန်းပြည့် nproc_main(ကိန်းပြည့် argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ကိန်းပြည့် nproc_main(ကိန်းပြည့် argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	ကိန်းပြည့် count = 0;
#အကယ်၍ ENABLE_LONG_OPTS
	ကိန်းပြည့် ignore = 0;
	ကိန်းပြည့် opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	အကယ်၍ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		အကယ်၍ (cpusd) {
			sa_nracanaa dirent *de;
			နေစဉ် (NULL != (de = readdir(cpusd))) {
				var_na *cpuid = strstr(de->d_name, "cpu");
				အကယ်၍ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} မဟုတ်ပါက
#endif
	{
		ကိန်းပြည့် i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= parimaa_na(long);
		အတွက် (i = 0; i < sz; i++) {
			အကယ်၍ (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	အကယ်၍ (count <= 0)
		count = 1;

	printf("%u\n", count);

	ပြန်ပေး 0;
}
