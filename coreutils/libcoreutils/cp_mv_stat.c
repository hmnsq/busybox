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
#ضمن "libbb.h"
#ضمن "coreutils.h"

صحيح FAST_FUNC cp_mv_stat2(ثابت حرف *fn, بنية stat *fn_stat, stat_func sf)
{
	إذا (sf(fn, fn_stat) < 0) {
		إذا (errno != ENOENT) {
#إذا ENABLE_FEATURE_VERBOSE_CP_پیام
			إذا (errno == ENOTDIR) {
				bb_error_پیام("can't stat '%s': Path has non-directory component", fn);
				أرجع -1;
			}
#پایان_اگر
			bb_perror_پیام("can't stat '%s'", fn);
			أرجع -1;
		}
		أرجع 0;
	}
	إذا (S_ISDIR(fn_stat->st_نما)) {
		أرجع 3;
	}
	أرجع 1;
}

صحيح FAST_FUNC cp_mv_stat(ثابت حرف *fn, بنية stat *fn_stat)
{
	أرجع cp_mv_stat2(fn, fn_stat, stat);
}
