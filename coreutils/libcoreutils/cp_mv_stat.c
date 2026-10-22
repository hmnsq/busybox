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
#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "coreutils.h"

ಪೂರ್ಣಾಙ್ಕ FAST_FUNC cp_mv_stat2(ಧ್ರುವ ವರ್ಣ *fn, ಸಂರಚನಾ stat *fn_stat, stat_func sf)
{
	ಯದಿ (sf(fn, fn_stat) < 0) {
		ಯದಿ (errno != ENOENT) {
#ಯದಿ ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			ಯದಿ (errno == ENOTDIR) {
				bb_error_ಸಂದೇಶ("can't stat '%s': Path has non-directory component", fn);
				ಪ್ರತ್ಯಾಗಮ -1;
			}
#endif
			bb_perror_ಸಂದೇಶ("can't stat '%s'", fn);
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ (S_ISDIR(fn_stat->st_mode)) {
		ಪ್ರತ್ಯಾಗಮ 3;
	}
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ FAST_FUNC cp_mv_stat(ಧ್ರುವ ವರ್ಣ *fn, ಸಂರಚನಾ stat *fn_stat)
{
	ಪ್ರತ್ಯಾಗಮ cp_mv_stat2(fn, fn_stat, stat);
}
