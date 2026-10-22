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
#অন্তর্ভাৱ "libbb.h"
#অন্তর্ভাৱ "coreutils.h"

পূর্ণাঙ্ক FAST_FUNC cp_mv_stat2(ধ্রুৱ ৱর্ণ *fn, সংরচনা stat *fn_stat, stat_func sf)
{
	যদি (sf(fn, fn_stat) < 0) {
		যদি (errno != ENOENT) {
#যদি ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			যদি (errno == ENOTDIR) {
				bb_error_বাৰ্তা("can't stat '%s': Path has non-directory component", fn);
				ঘূৰাই -1;
			}
#endif
			bb_perror_বাৰ্তা("can't stat '%s'", fn);
			ঘূৰাই -1;
		}
		ঘূৰাই 0;
	}
	যদি (S_ISDIR(fn_stat->st_mode)) {
		ঘূৰাই 3;
	}
	ঘূৰাই 1;
}

পূর্ণাঙ্ক FAST_FUNC cp_mv_stat(ধ্রুৱ ৱর্ণ *fn, সংরচনা stat *fn_stat)
{
	ঘূৰাই cp_mv_stat2(fn, fn_stat, stat);
}
