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

ຈຳນວນເຕັມ nproc_main(ຈຳນວນເຕັມ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ຈຳນວນເຕັມ nproc_main(ຈຳນວນເຕັມ argc UNUSED_PARAM, var_na **argv UNUSED_PARAM)
{
	ຈຳນວນເຕັມ count = 0;
#ຖ້າ ENABLE_LONG_OPTS
	ຈຳນວນເຕັມ ignore = 0;
	ຈຳນວນເຕັມ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	ຖ້າ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ຖ້າ (cpusd) {
			sa_nracanaa dirent *de;
			ໃນຂະນະທີ່ (NULL != (de = readdir(cpusd))) {
				var_na *cpuid = strstr(de->d_name, "cpu");
				ຖ້າ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} ບໍ່ດັ່ງນັ້ນ
#endif
	{
		ຈຳນວນເຕັມ i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= parimaa_na(long);
		ສຳລັບ (i = 0; i < sz; i++) {
			ຖ້າ (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(free(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	ຖ້າ (count <= 0)
		count = 1;

	printf("%u\n", count);

	ສົ່ງຄືນ 0;
}
