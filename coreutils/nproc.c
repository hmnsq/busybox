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

#ଅନ୍ତର୍ଭାଵ "libbb.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା nproc_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା nproc_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
#ଯଦି ENABLE_LONG_OPTS
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ignore = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	ଯଦି (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ଯଦି (cpusd) {
			ସଂରଚନା dirent *de;
			ଯେତେବେଳେ (ଖାଲି != (de = readdir(cpusd))) {
				ଅକ୍ଷର *cpuid = strstr(de->d_name, "cpu");
				ଯଦି (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					ଗଣନା_2++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} ଅନ୍ୟଥା
#endif
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= ଆକାର(long);
		ପାଇଁ (i = 0; i < sz; i++) {
			ଯଦି (mask[i] != 0) /* most mask[i] are usually 0 */
				ଗଣନା_2 += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(ମୁକ୍ତ(mask);)
	}

	IF_LONG_OPTS(ଗଣନା_2 -= ignore;)
	ଯଦି (ଗଣନା_2 <= 0)
		ଗଣନା_2 = 1;

	printf("%u\n", ଗଣନା_2);

	ଫେରନ୍ତୁ 0;
}
