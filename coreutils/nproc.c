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

#அந்தர்பாவ "libbb.h"

முழு nproc_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு nproc_main(முழு argc UNUSED_PARAM, எழுத்து **argv UNUSED_PARAM)
{
	முழு மொத்தம் = 0;
#எனில் ENABLE_LONG_OPTS
	முழு ignore = 0;
	முழு opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	எனில் (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		எனில் (cpusd) {
			கட்டமைப்பு dirent *de;
			போது (வெற்று != (de = readdir(cpusd))) {
				எழுத்து *cpuid = strstr(de->d_name, "cpu");
				எனில் (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					மொத்தம்++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} இல்லையெனில்
#endif
	{
		முழு i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= அளவு(long);
		க்கு (i = 0; i < sz; i++) {
			எனில் (mask[i] != 0) /* most mask[i] are usually 0 */
				மொத்தம் += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(விடுவி(mask);)
	}

	IF_LONG_OPTS(மொத்தம் -= ignore;)
	எனில் (மொத்தம் <= 0)
		மொத்தம் = 1;

	printf("%u\n", மொத்தம்);

	திரும்பு 0;
}
