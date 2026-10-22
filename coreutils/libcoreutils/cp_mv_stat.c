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

정수 FAST_FUNC cp_mv_stat2(const char *fn, struct stat *fn_stat, stat_func sf)
{
	만약 (sf(fn, fn_stat) < 0) {
		만약 (errno != ENOENT) {
#만약 ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			만약 (errno == ENOTDIR) {
				bb_error_msg("can't stat '%s': Path has non-directory component", fn);
				반환 -1;
			}
#endif
			bb_perror_msg("can't stat '%s'", fn);
			반환 -1;
		}
		반환 0;
	}
	만약 (S_ISDIR(fn_stat->st_mode)) {
		반환 3;
	}
	반환 1;
}

정수 FAST_FUNC cp_mv_stat(const char *fn, struct stat *fn_stat)
{
	반환 cp_mv_stat2(fn, fn_stat, stat);
}
