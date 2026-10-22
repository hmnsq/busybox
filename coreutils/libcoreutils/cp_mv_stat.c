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
#समावेश "libbb.h"
#समावेश "coreutils.h"

पूर्णांक FAST_FUNC cp_mv_stat2(स्थिर अक्षर *fn, काष्ठा stat *fn_stat, stat_func sf)
{
	अगर (sf(fn, fn_stat) < 0) {
		अगर (त्रुटि_सं != ENOENT) {
#अगर ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			अगर (त्रुटि_सं == ENOTDIR) {
				bb_error_संदेश("can't stat '%s': Path has non-directory component", fn);
				वापस -1;
			}
#पूर्ण_अगर
			bb_perror_संदेश("can't stat '%s'", fn);
			वापस -1;
		}
		वापस 0;
	}
	अगर (S_ISDIR(fn_stat->st_mode)) {
		वापस 3;
	}
	वापस 1;
}

पूर्णांक FAST_FUNC cp_mv_stat(स्थिर अक्षर *fn, काष्ठा stat *fn_stat)
{
	वापस cp_mv_stat2(fn, fn_stat, stat);
}
