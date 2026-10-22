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
#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "coreutils.h"

முழு FAST_FUNC cp_mv_stat2(மாறிலி எழுத்து *fn, கட்டமைப்பு stat *fn_stat, stat_func sf)
{
	எனில் (sf(fn, fn_stat) < 0) {
		எனில் (errno != ENOENT) {
#எனில் ENABLE_FEATURE_VERBOSE_CP_MESSAGE
			எனில் (errno == ENOTDIR) {
				bb_error_செய்தி("can't stat '%s': Path has non-directory component", fn);
				திரும்பு -1;
			}
#endif
			bb_perror_செய்தி("can't stat '%s'", fn);
			திரும்பு -1;
		}
		திரும்பு 0;
	}
	எனில் (S_ISDIR(fn_stat->st_mode)) {
		திரும்பு 3;
	}
	திரும்பு 1;
}

முழு FAST_FUNC cp_mv_stat(மாறிலி எழுத்து *fn, கட்டமைப்பு stat *fn_stat)
{
	திரும்பு cp_mv_stat2(fn, fn_stat, stat);
}
