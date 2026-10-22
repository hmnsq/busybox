/* vi: set sw=4 ts=4: */
/*
 * coreutils utility routine
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "coreutils.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC cp_mv_stat2(ଅଚଳ ଅକ୍ଷର *fn, ସଂରଚନା stat *fn_stat, stat_func sf)
{
	ଯଦି (sf(fn, fn_stat) < 0) {
		ଯଦି (errno != ENOENT) {
#ଯଦି ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			ଯଦି (errno == ENOTDIR) {
				bb_error_ସନ୍ଦେଶ("can't stat '%s': Path has non-directory component", fn);
				ଫେରନ୍ତୁ -1;
			}
#endif
			bb_perror_ସନ୍ଦେଶ("can't stat '%s'", fn);
			ଫେରନ୍ତୁ -1;
		}
		ଫେରନ୍ତୁ 0;
	}
	ଯଦି (S_ISDIR(fn_stat->st_mode)) {
		ଫେରନ୍ତୁ 3;
	}
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC cp_mv_stat(ଅଚଳ ଅକ୍ଷର *fn, ସଂରଚନା stat *fn_stat)
{
	ଫେରନ୍ତୁ cp_mv_stat2(fn, fn_stat, stat);
}
