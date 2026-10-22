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
#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "coreutils.h"

পূর্ণসংখ্যা FAST_FUNC cp_mv_stat2(ধ্রুবক অক্ষর *fn, কাঠামো stat *fn_stat, stat_func sf)
{
	যদি (sf(fn, fn_stat) < 0) {
		যদি (errno != ENOENT) {
#যদি ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			যদি (errno == ENOTDIR) {
				bb_error_বার্তা("can't stat '%s': Path has non-directory component", fn);
				ফেরত -1;
			}
#endif
			bb_perror_বার্তা("can't stat '%s'", fn);
			ফেরত -1;
		}
		ফেরত 0;
	}
	যদি (S_ISDIR(fn_stat->st_mode)) {
		ফেরত 3;
	}
	ফেরত 1;
}

পূর্ণসংখ্যা FAST_FUNC cp_mv_stat(ধ্রুবক অক্ষর *fn, কাঠামো stat *fn_stat)
{
	ফেরত cp_mv_stat2(fn, fn_stat, stat);
}
