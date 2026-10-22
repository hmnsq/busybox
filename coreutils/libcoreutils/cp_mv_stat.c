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
#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "coreutils.h"

പൂർണ്ണസംഖ്യ FAST_FUNC cp_mv_stat2(സ്ഥിരം അക്ഷരം *fn, ഘടന stat *fn_stat, stat_func sf)
{
	എങ്കിൽ (sf(fn, fn_stat) < 0) {
		എങ്കിൽ (errno != ENOENT) {
#എങ്കിൽ ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			എങ്കിൽ (errno == ENOTDIR) {
				bb_error_സന്ദേശം("can't stat '%s': Path has non-directory component", fn);
				തിരികെ -1;
			}
#endif
			bb_perror_സന്ദേശം("can't stat '%s'", fn);
			തിരികെ -1;
		}
		തിരികെ 0;
	}
	എങ്കിൽ (S_ISDIR(fn_stat->st_mode)) {
		തിരികെ 3;
	}
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ FAST_FUNC cp_mv_stat(സ്ഥിരം അക്ഷരം *fn, ഘടന stat *fn_stat)
{
	തിരികെ cp_mv_stat2(fn, fn_stat, stat);
}
