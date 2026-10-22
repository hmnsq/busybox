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
#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "coreutils.h"

ਪੂਰਨ FAST_FUNC cp_mv_stat2(ਅਚਲ ਅੱਖਰ *fn, ਬਣਤਰ stat *fn_stat, stat_func sf)
{
	ਜੇ (sf(fn, fn_stat) < 0) {
		ਜੇ (errno != ENOENT) {
#ਜੇ ENABLE_FEATURE_VERBOSE_CP_ਸੁਨੇਹਾ
			ਜੇ (errno == ENOTDIR) {
				bb_error_ਸੁਨੇਹਾ("can't stat '%s': Path has non-directory component", fn);
				ਵਾਪਸ -1;
			}
#ਜੇ_ਅੰਤ
			bb_perror_ਸੁਨੇਹਾ("can't stat '%s'", fn);
			ਵਾਪਸ -1;
		}
		ਵਾਪਸ 0;
	}
	ਜੇ (S_ISDIR(fn_stat->st_ਬਹੁਲਕ)) {
		ਵਾਪਸ 3;
	}
	ਵਾਪਸ 1;
}

ਪੂਰਨ FAST_FUNC cp_mv_stat(ਅਚਲ ਅੱਖਰ *fn, ਬਣਤਰ stat *fn_stat)
{
	ਵਾਪਸ cp_mv_stat2(fn, fn_stat, stat);
}
