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

#अन्तर्भाव "libbb.h"

पूर्णाङ्क nproc_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क nproc_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv अप्रयुक्तम्_PARAM)
{
	पूर्णाङ्क गणनम् = 0;
#यदि ENABLE_LONG_OPTS
	पूर्णाङ्क ignore = 0;
	पूर्णाङ्क opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_तर्कः "\xfe"
			"all\0"    No_तर्कः       "\xff"
			, &ignore
	);

	यदि (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		यदि (cpusd) {
			संरचना dirent *de;
			यावत् (रिक्त != (de = readdir(cpusd))) {
				वर्ण *cpuid = वाक्ये_अन्विष(de->d_name, "cpu");
				यदि (cpuid && अङ्कः_वा(cpuid[वाक्यदैर्घ्यम्(cpuid) - 1]))
					गणनम्++;
			}
			IF_FEATURE_CLEAN_ऊर्ध्वम्(closedir(cpusd);)
		}
	} अन्यथा
#यदि_अन्तः
	{
		पूर्णाङ्क i;
		अचिह्नित sz = 2 * 1024;
		अचिह्नित दीर्घ *आवरणम् = get_malloc_cpu_affinity(0, &sz);
		sz /= परिमाण(दीर्घ);
		प्रति (i = 0; i < sz; i++) {
			यदि (आवरणम्[i] != 0) /* most mask[i] are usually 0 */
				गणनम् += bb_popcnt_long(आवरणम्[i]);
		}
		IF_FEATURE_CLEAN_ऊर्ध्वम्(मुक्त(आवरणम्);)
	}

	IF_LONG_OPTS(गणनम् -= ignore;)
	यदि (गणनम् <= 0)
		गणनम् = 1;

	प्रारूप_लिख("%u\n", गणनम्);

	प्रत्यागम 0;
}
