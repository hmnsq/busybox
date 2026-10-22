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

ಪೂರ್ಣಾಂಕ nproc_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ nproc_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
#ಆದರೆ ENABLE_LONG_OPTS
	ಪೂರ್ಣಾಂಕ ignore = 0;
	ಪೂರ್ಣಾಂಕ opts = getopt32long(argv, "\xfe:+",
			"ignore\0" Required_ವಾದ "\xfe"
			"all\0"    No_ವಾದ       "\xff"
			, &ignore
	);

	ಆದರೆ (opts & (1 << 1)) {
		DIR *cpusd = opendir("/sys/devices/system/cpu");
		ಆದರೆ (cpusd) {
			ರಚನೆ dirent *de;
			ಆಗ (ಖಾಲಿ != (de = readdir(cpusd))) {
				ಅಕ್ಷರ *cpuid = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(de->d_name, "cpu");
				ಆದರೆ (cpuid && ಅಂಕಿಯೇ(cpuid[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cpuid) - 1]))
					ಎಣಿಕೆ_2++;
			}
			IF_FEATURE_CLEAN_ಮೇಲೆ(closedir(cpusd);)
		}
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	{
		ಪೂರ್ಣಾಂಕ i;
		ಚಿಹ್ನೆರಹಿತ sz = 2 * 1024;
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಮುಖವಾಡ = get_malloc_cpu_affinity(0, &sz);
		sz /= ಗಾತ್ರ(ದೀರ್ಘ);
		ಗಾಗಿ (i = 0; i < sz; i++) {
			ಆದರೆ (ಮುಖವಾಡ[i] != 0) /* most mask[i] are usually 0 */
				ಎಣಿಕೆ_2 += bb_popcnt_long(ಮುಖವಾಡ[i]);
		}
		IF_FEATURE_CLEAN_ಮೇಲೆ(ಬಿಡುಗಡೆ(ಮುಖವಾಡ);)
	}

	IF_LONG_OPTS(ಎಣಿಕೆ_2 -= ignore;)
	ಆದರೆ (ಎಣಿಕೆ_2 <= 0)
		ಎಣಿಕೆ_2 = 1;

	ಬರೆ("%u\n", ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ 0;
}
