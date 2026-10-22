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

#અન્તર્ભાવ "libbb.h"

પૂર્ણાંક nproc_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક nproc_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv UNUSED_PARAM)
{
	પૂર્ણાંક ગણતરી = 0;
#જો ENABLE_LONG_OPTS
	પૂર્ણાંક ignore = 0;
	પૂર્ણાંક opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	જો (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		જો (cpusd) {
			સંરચના dirent *de;
			જ્યારે (ખાલી != (de = readdir(cpusd))) {
				અક્ષર *cpuid = strstr(de->d_name, "cpu");
				જો (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					ગણતરી++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} નહીંતર
#endif
	{
		પૂર્ણાંક i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= કદ(long);
		માટે (i = 0; i < sz; i++) {
			જો (mask[i] != 0) /* most mask[i] are usually 0 */
				ગણતરી += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(મુક્ત(mask);)
	}

	IF_LONG_OPTS(ગણતરી -= ignore;)
	જો (ગણતરી <= 0)
		ગણતરી = 1;

	printf("%u\n", ગણતરી);

	પાછા 0;
}
