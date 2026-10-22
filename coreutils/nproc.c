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
صحيح nproc_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv ناکارول_شوی_PARAM)
{
	صحيح شمېر = 0;
#إذا ENABLE_LONG_OPTS
	صحيح ignore = 0;
	صحيح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_دلیل "\xfe"
			"all\0"    No_دلیل       "\xff"
			, &ignore
	);

	إذا (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		إذا (cpusd) {
			بنية dirent *de;
			طالما (عدم != (de = readdir(cpusd))) {
				حرف *cpuid = مزي_پلټنه(de->d_name, "cpu");
				إذا (cpuid && که_عدد(cpuid[مزي_اوږدوالی(cpuid) - 1]))
					شمېر++;
			}
			IF_FEATURE_CLEAN_پورته(closedir(cpusd);)
		}
	} وإلا
#د_که_پای
	{
		صحيح i;
		بینښه sz = 2 * 1024;
		بینښه اوږد *ماسک = get_malloc_cpu_affinity(0, &sz);
		sz /= حجم(اوږد);
		لكل (i = 0; i < sz; i++) {
			إذا (ماسک[i] != 0) /* most mask[i] are usually 0 */
				شمېر += bb_popcnt_long(ماسک[i]);
		}
		IF_FEATURE_CLEAN_پورته(حرر(ماسک);)
	}

	IF_LONG_OPTS(شمېر -= ignore;)
	إذا (شمېر <= 0)
		شمېر = 1;

	چاپ_کړه("%u\n", شمېر);

	أرجع 0;
}
