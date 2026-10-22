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
पूर्णाङ्क nproc_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv UNUSED_PARAM)
{
	पूर्णाङ्क गिनती = 0;
#यदि ENABLE_LONG_OPTS
	पूर्णाङ्क ignore = 0;
	पूर्णाङ्क opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	यदि (opts & (1 << 1)) {
		सूची *cpusd = सूची_खोलो("/sys/devices/system/cpu");
		यदि (cpusd) {
			संरचना dirent *de;
			यावत् (रिक्त != (de = सूची_पढ़ो(cpusd))) {
				वर्ण *cpuid = म_माला(de->d_name, "cpu");
				यदि (cpuid && है_अंक(cpuid[म_माप(cpuid) - 1]))
					गिनती++;
			}
			IF_FEATURE_CLEAN_UP(बंद_सूची(cpusd);)
		}
	} अन्यथा
#endif
	{
		पूर्णाङ्क i;
		अचिन्हित sz = 2 * 1024;
		अचिन्हित दीर्घ *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= परिमाण(दीर्घ);
		प्रति (i = 0; i < sz; i++) {
			यदि (mask[i] != 0) /* most mask[i] are usually 0 */
				गिनती += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(मुक्त(mask);)
	}

	IF_LONG_OPTS(गिनती -= ignore;)
	यदि (गिनती <= 0)
		गिनती = 1;

	म_लिखो("%u\n", गिनती);

	प्रत्यागम 0;
}
