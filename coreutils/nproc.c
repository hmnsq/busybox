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

#ضمن "libbb.h"

صحيح nproc_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح nproc_main(صحيح argc UNUSED_PARAM, حرف **argv UNUSED_PARAM)
{
	صحيح count = 0;
#𐡄𐡍 ENABLE_LONG_OPTS
	صحيح ignore = 0;
	صحيح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_argument "\xfe"
			"all\0"    No_argument       "\xff"
			, &ignore
	);

	𐡄𐡍 (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		𐡄𐡍 (cpusd) {
			بنية dirent *de;
			طالما (عدم != (de = readdir(cpusd))) {
				حرف *cpuid = strstr(de->d_name, "cpu");
				𐡄𐡍 (cpuid && isdigit(cpuid[strlen(cpuid) - 1]))
					count++;
			}
			IF_FEATURE_CLEAN_UP(closedir(cpusd);)
		}
	} وإلا
#endif
	{
		صحيح i;
		unsigned sz = 2 * 1024;
		unsigned long *mask = get_malloc_cpu_affinity(0, &sz);
		sz /= حجم(long);
		لكل (i = 0; i < sz; i++) {
			𐡄𐡍 (mask[i] != 0) /* most mask[i] are usually 0 */
				count += bb_popcnt_long(mask[i]);
		}
		IF_FEATURE_CLEAN_UP(حرر(mask);)
	}

	IF_LONG_OPTS(count -= ignore;)
	𐡄𐡍 (count <= 0)
		count = 1;

	printf("%u\n", count);

	أرجع 0;
}
