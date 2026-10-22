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

#അന്തര്ഭാവ "libbb.h"

പൂർണ്ണസംഖ്യ nproc_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ nproc_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv UNUSED_PARAM)
{
	പൂർണ്ണസംഖ്യ എണ്ണം = 0;
#എങ്കിൽ ENABLE_LONG_OPTS
	പൂർണ്ണസംഖ്യ ignore = 0;
	പൂർണ്ണസംഖ്യ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	എങ്കിൽ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		എങ്കിൽ (cpusd) {
			ഘടന dirent *de;
			ആകുമ്പോൾ (ശൂന്യ != (de = readdir(cpusd))) {
				അക്ഷരം *cpuid = strstr(de->d_name, "cpu");
				എങ്കിൽ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					എണ്ണം++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} അല്ലെങ്കിൽ
#endif
	{
		പൂർണ്ണസംഖ്യ i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= വലുപ്പം(long);
		വേണ്ടി (i = 0; i < sz; i++) {
			എങ്കിൽ (mask[i] != 0) /* most mask[i] are usually 0 */
				എണ്ണം += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(സ്വതന്ത്രം(mask);)
	}

	IF_LONG_OPTS(എണ്ണം -= ignore;)
	എങ്കിൽ (എണ്ണം <= 0)
		എണ്ണം = 1;

	printf("%u\n", എണ്ണം);

	തിരികെ 0;
}
