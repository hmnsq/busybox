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
#అన్తర్భావ "libbb.h"
#అన్తర్భావ "coreutils.h"

పూర్ణాంకం FAST_FUNC cp_mv_stat2(స్థిరరాశి అక్షరం *fn, నిర్మాణం stat *fn_stat, stat_func sf)
{
	ఒకవేళ (sf(fn, fn_stat) < 0) {
		ఒకవేళ (errno != ENOENT) {
#ఒకవేళ ENABLE_FEATURE_VERBOSE_CP_సందేశం
			ఒకవేళ (errno == ENOTDIR) {
				bb_error_సందేశం("can't stat '%s': Path has non-directory component", fn);
				తిరిగి -1;
			}
#అయితే_ముగింపు
			bb_perror_సందేశం("can't stat '%s'", fn);
			తిరిగి -1;
		}
		తిరిగి 0;
	}
	ఒకవేళ (S_ISDIR(fn_stat->st_బహుళకం)) {
		తిరిగి 3;
	}
	తిరిగి 1;
}

పూర్ణాంకం FAST_FUNC cp_mv_stat(స్థిరరాశి అక్షరం *fn, నిర్మాణం stat *fn_stat)
{
	తిరిగి cp_mv_stat2(fn, fn_stat, stat);
}
