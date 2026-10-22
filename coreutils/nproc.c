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
صحيح nproc_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv اڻ_استعمال_ٿيل_PARAM)
{
	صحيح ڳڻپ = 0;
#إذا ENABLE_LONG_OPTS
	صحيح ignore = 0;
	صحيح opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_دليل "\xfe"
			"all\0"    No_دليل       "\xff"
			, &ignore
	);

	إذا (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		إذا (cpusd) {
			بنية dirent *de;
			طالما (عدم != (de = readdir(cpusd))) {
				حرف *cpuid = لڙي_ڳولا(de->d_name, "cpu");
				إذا (cpuid && انگ_آهي(cpuid[لڙي_ڊيگهه(cpuid) - 1]))
					ڳڻپ++;
			}
			IF_FEATURE_CLEAN_مٿي(closedir(cpusd);)
		}
	} وإلا
#جيڪڏهن_پورو
	{
		صحيح i;
		بنانشان sz = 2 * 1024;
		بنانشان ڊگهو *نقاب = get_malloc_cpu_affinity(0, &sz);
		sz /= حجم(ڊگهو);
		لكل (i = 0; i < sz; i++) {
			إذا (نقاب[i] != 0) /* most mask[i] are usually 0 */
				ڳڻپ += bb_popcnt_long(نقاب[i]);
		}
		IF_FEATURE_CLEAN_مٿي(حرر(نقاب);)
	}

	IF_LONG_OPTS(ڳڻپ -= ignore;)
	إذا (ڳڻپ <= 0)
		ڳڻپ = 1;

	لکو("%u\n", ڳڻپ);

	أرجع 0;
}
