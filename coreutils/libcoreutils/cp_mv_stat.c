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

puur_naa_nka FAST_FUNC cp_mv_stat2(const var_na *fn, sa_nracanaa stat *fn_stat, stat_func sf)
{
	ᬬᬾᬦ᭄ (sf(fn, fn_stat) < 0) {
		ᬬᬾᬦ᭄ (errno != ENOENT) {
#ᬬᬾᬦ᭄ ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			ᬬᬾᬦ᭄ (errno == ENOTDIR) {
				bb_error_msg("can't stat '%s': Path has non-directory component", fn);
				ᬯᬮᬶ -1;
			}
#endif
			bb_perror_msg("can't stat '%s'", fn);
			ᬯᬮᬶ -1;
		}
		ᬯᬮᬶ 0;
	}
	ᬬᬾᬦ᭄ (S_ISDIR(fn_stat->st_mode)) {
		ᬯᬮᬶ 3;
	}
	ᬯᬮᬶ 1;
}

puur_naa_nka FAST_FUNC cp_mv_stat(const var_na *fn, sa_nracanaa stat *fn_stat)
{
	ᬯᬮᬶ cp_mv_stat2(fn, fn_stat, stat);
}
