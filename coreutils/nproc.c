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

#समावेश "libbb.h"

पूर्णांक nproc_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक nproc_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv UNUSED_PARAM)
{
	पूर्णांक गिनती = 0;
#अगर ENABLE_LONG_OPTS
	पूर्णांक ignore = 0;
	पूर्णांक opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	अगर (opts & (1 << 1)) {
		सूची *cpusd = सूची_खोलो("/sys/devices/system/cpu");
		अगर (cpusd) {
			काष्ठा dirent *de;
			जबतक (शून्य != (de = सूची_पढ़ो(cpusd))) {
				अक्षर *cpuid = म_माला(de->d_name, "cpu");
				अगर (cpuid && है_अंक(cpuid[म_माप(cpuid) - 1]))
					गिनती++;
			}
			IF_FEATURE_CLEAN_UP(बंद_सूची(cpusd);)
		}
	} अन्यथा
#पूर्ण_अगर
	{
		पूर्णांक i;
		अचिन्हित sz = 2 * 1024;
		अचिन्हित दीर्घ *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= माप(दीर्घ);
		क्रम (i = 0; i < sz; i++) {
			अगर (mask[i] != 0) /* most mask[i] are usually 0 */
				गिनती += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(मुक्त(mask);)
	}

	IF_LONG_OPTS(गिनती -= ignore;)
	अगर (गिनती <= 0)
		गिनती = 1;

	म_लिखो("%u\n", गिनती);

	वापस 0;
}
