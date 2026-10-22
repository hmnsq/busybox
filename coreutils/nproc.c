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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"

ਪੂਰਨ nproc_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ nproc_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
#ਜੇ ENABLE_LONG_OPTS
	ਪੂਰਨ ignore = 0;
	ਪੂਰਨ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_ਦਲੀਲ "\xfe"
			"all\0"    No_ਦਲੀਲ       "\xff"
			, &ignore
	);

	ਜੇ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ਜੇ (cpusd) {
			ਬਣਤਰ dirent *de;
			ਜਦੋਂ (ਨਿਰਾ != (de = readdir(cpusd))) {
				ਅੱਖਰ *cpuid = ਲੜੀ_ਖੋਜ(de->d_name, "cpu");
				ਜੇ (cpuid && ਅੰਕ_ਹੈ(cpuid[ਲੜੀ_ਲੰਬਾਈ(cpuid) - 1]))
					ਗਿਣਤੀ_2++;
			}
			IF_FEATURE_CLEAN_ਉੱਪਰ(closedir(cpusd);)
		}
	} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	{
		ਪੂਰਨ i;
		ਬਿਨਾਨਿਸ਼ਾਨੀ sz = 2 * 1024;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਨਕਾਬ = get_malloc_cpu_affinity(0, &sz);
		sz /= ਆਕਾਰ(ਲੰਮਾ);
		ਲਈ (i = 0; i < sz; i++) {
			ਜੇ (ਨਕਾਬ[i] != 0) /* most mask[i] are usually 0 */
				ਗਿਣਤੀ_2 += bb_popcnt_long(ਨਕਾਬ[i]);
		}
		IF_FEATURE_CLEAN_ਉੱਪਰ(ਮੁਕਤ(ਨਕਾਬ);)
	}

	IF_LONG_OPTS(ਗਿਣਤੀ_2 -= ignore;)
	ਜੇ (ਗਿਣਤੀ_2 <= 0)
		ਗਿਣਤੀ_2 = 1;

	ਲਿਖੋ("%u\n", ਗਿਣਤੀ_2);

	ਵਾਪਸ 0;
}
