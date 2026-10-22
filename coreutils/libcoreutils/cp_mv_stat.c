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

पूर्णाङ्क FAST_FUNC cp_mv_stat2(ध्रुव वर्ण *fn, संरचना stat *fn_stat, stat_func sf)
{
	सचे (sf(fn, fn_stat) < 0) {
		सचे (errno != ENOENT) {
#सचे ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			सचे (errno == ENOTDIR) {
				bb_error_सन्देशः("can't stat '%s': Path has non-directory component", fn);
				पटिनिवत्त -1;
			}
#endif
			bb_perror_सन्देशः("can't stat '%s'", fn);
			पटिनिवत्त -1;
		}
		पटिनिवत्त 0;
	}
	सचे (S_ISDIR(fn_stat->st_mode)) {
		पटिनिवत्त 3;
	}
	पटिनिवत्त 1;
}

पूर्णाङ्क FAST_FUNC cp_mv_stat(ध्रुव वर्ण *fn, संरचना stat *fn_stat)
{
	पटिनिवत्त cp_mv_stat2(fn, fn_stat, stat);
}
