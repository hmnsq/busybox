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
#شامل "libbb.h"
#شامل "coreutils.h"

صحیح FAST_FUNC cp_mv_stat2(ثابت حرف *fn, ساخت stat *fn_stat, stat_func sf)
{
	اگر (sf(fn, fn_stat) < 0) {
		اگر (errno != ENOENT) {
#اگر ENABLE_FEATURE_VERBOSE_CP_پیغام
			اگر (errno == ENOTDIR) {
				bb_error_پیغام("can't stat '%s': Path has non-directory component", fn);
				واپس -1;
			}
#اگر_ختم
			bb_perror_پیغام("can't stat '%s'", fn);
			واپس -1;
		}
		واپس 0;
	}
	اگر (S_ISDIR(fn_stat->st_منوال)) {
		واپس 3;
	}
	واپس 1;
}

صحیح FAST_FUNC cp_mv_stat(ثابت حرف *fn, ساخت stat *fn_stat)
{
	واپس cp_mv_stat2(fn, fn_stat, stat);
}
