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

ಪೂರ್ಣಾಂಕ FAST_FUNC cp_mv_stat2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fn, ರಚನೆ stat *fn_stat, stat_func sf)
{
	ಆದರೆ (sf(fn, fn_stat) < 0) {
		ಆದರೆ (errno != ENOENT) {
#ಆದರೆ ENABLE_FEATURE_VERBOSE_CP_ಸಂದೇಶ
			ಆದರೆ (errno == ENOTDIR) {
				bb_error_ಸಂದೇಶ("can't stat '%s': Path has non-directory component", fn);
				ಹಿಂತಿರುಗಿ -1;
			}
#ಆದರೆ_ಅಂತ್ಯ
			bb_perror_ಸಂದೇಶ("can't stat '%s'", fn);
			ಹಿಂತಿರುಗಿ -1;
		}
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ (S_ISDIR(fn_stat->st_ಬಹುಲಕ)) {
		ಹಿಂತಿರುಗಿ 3;
	}
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ FAST_FUNC cp_mv_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fn, ರಚನೆ stat *fn_stat)
{
	ಹಿಂತಿರುಗಿ cp_mv_stat2(fn, fn_stat, stat);
}
