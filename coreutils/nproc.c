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

#ಅನ್ತರ್ಭಾವ "libbb.h"

ಪೂರ್ಣಾಙ್ಕ nproc_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ nproc_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv UNUSED_PARAM)
{
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
#ಯದಿ ENABLE_LONG_OPTS
	ಪೂರ್ಣಾಙ್ಕ ignore = 0;
	ಪೂರ್ಣಾಙ್ಕ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	ಯದಿ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ಯದಿ (cpusd) {
			ಸಂರಚನಾ dirent *de;
			ಯಾವತ್ (ರಿಕ್ತ != (de = readdir(cpusd))) {
				ವರ್ಣ *cpuid = strstr(de->d_name, "cpu");
				ಯದಿ (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					ಎಣಿಕೆ++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} ಅನ್ಯಥಾ
#endif
	{
		ಪೂರ್ಣಾಙ್ಕ i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= ಪರಿಮಾಣ(long);
		ಪ್ರತಿ (i = 0; i < sz; i++) {
			ಯದಿ (mask[i] != 0) /* most mask[i] are usually 0 */
				ಎಣಿಕೆ += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(ಮುಕ್ತ(mask);)
	}

	IF_LONG_OPTS(ಎಣಿಕೆ -= ignore;)
	ಯದಿ (ಎಣಿಕೆ <= 0)
		ಎಣಿಕೆ = 1;

	printf("%u\n", ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ 0;
}
