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
	यदि (sf(fn, fn_stat) < 0) {
		यदि (errno != ENOENT) {
#यदि ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			यदि (errno == ENOTDIR) {
				bb_error_संदेश("can't stat '%s': Path has non-directory component", fn);
				प्रत्यागम -1;
			}
#endif
			bb_perror_संदेश("can't stat '%s'", fn);
			प्रत्यागम -1;
		}
		प्रत्यागम 0;
	}
	यदि (S_ISDIR(fn_stat->st_mode)) {
		प्रत्यागम 3;
	}
	प्रत्यागम 1;
}

पूर्णाङ्क FAST_FUNC cp_mv_stat(ध्रुव वर्ण *fn, संरचना stat *fn_stat)
{
	प्रत्यागम cp_mv_stat2(fn, fn_stat, stat);
}
