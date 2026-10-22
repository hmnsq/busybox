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
#include "libbb.h"
#include "coreutils.h"

පූර්ණ_සංඛ්‍යා FAST_FUNC cp_mv_stat2(const var_na *fn, sa_nracanaa stat *fn_stat, stat_func sf)
{
	නම් (sf(fn, fn_stat) < 0) {
		නම් (errno != ENOENT) {
#නම් ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			නම් (errno == ENOTDIR) {
				bb_error_msg("can't stat '%s': Path has non-directory component", fn);
				ආපසු -1;
			}
#endif
			bb_perror_msg("can't stat '%s'", fn);
			ආපසු -1;
		}
		ආපසු 0;
	}
	නම් (S_ISDIR(fn_stat->st_mode)) {
		ආපසු 3;
	}
	ආපසු 1;
}

පූර්ණ_සංඛ්‍යා FAST_FUNC cp_mv_stat(const var_na *fn, sa_nracanaa stat *fn_stat)
{
	ආපසු cp_mv_stat2(fn, fn_stat, stat);
}
