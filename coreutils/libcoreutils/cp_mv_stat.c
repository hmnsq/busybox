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
#अन्तर्भाव "libbb.h"
#अन्तर्भाव "coreutils.h"

पूर्णांक FAST_FUNC cp_mv_stat2(स्थिरांक अक्षर *fn, संरचना stat *fn_stat, stat_func sf)
{
	जर (sf(fn, fn_stat) < 0) {
		जर (त्रुटि_सं != ENOENT) {
#जर ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			जर (त्रुटि_सं == ENOTDIR) {
				bb_error_संदेश("can't stat '%s': Path has non-directory component", fn);
				परत -1;
			}
#endif
			bb_perror_संदेश("can't stat '%s'", fn);
			परत -1;
		}
		परत 0;
	}
	जर (S_ISDIR(fn_stat->st_mode)) {
		परत 3;
	}
	परत 1;
}

पूर्णांक FAST_FUNC cp_mv_stat(स्थिरांक अक्षर *fn, संरचना stat *fn_stat)
{
	परत cp_mv_stat2(fn, fn_stat, stat);
}
