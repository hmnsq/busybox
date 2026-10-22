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
#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ "coreutils.h"

પૂર્ણાંક FAST_FUNC cp_mv_stat2(અચલ અક્ષર *fn, સંરચના stat *fn_stat, stat_func sf)
{
	જો (sf(fn, fn_stat) < 0) {
		જો (errno != ENOENT) {
#જો ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			જો (errno == ENOTDIR) {
				bb_error_સંદેશ("can't stat '%s': Path has non-directory component", fn);
				પાછા -1;
			}
#endif
			bb_perror_સંદેશ("can't stat '%s'", fn);
			પાછા -1;
		}
		પાછા 0;
	}
	જો (S_ISDIR(fn_stat->st_mode)) {
		પાછા 3;
	}
	પાછા 1;
}

પૂર્ણાંક FAST_FUNC cp_mv_stat(અચલ અક્ષર *fn, સંરચના stat *fn_stat)
{
	પાછા cp_mv_stat2(fn, fn_stat, stat);
}
