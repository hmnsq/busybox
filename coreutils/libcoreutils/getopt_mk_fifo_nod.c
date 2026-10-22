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
#ضمن "libbb.h"
#ضمن "coreutils.h"

نما_t FAST_FUNC getopt_mk_fifo_nod(حرف **argv)
{
	نما_t نما = 0666;
	حرف *smode = عدم;
#إذا ENABLE_SELINUX
	security_context_t scontext;
#پایان_اگر
	صحيح opt;
	opt = getopt32(argv, "m:" IF_SELINUX("Z:"), &smode IF_SELINUX(,&scontext));
	إذا (opt & 1) {
		نما = bb_parse_نما(smode, نما);
		إذا (نما != (نما_t)-1) /* if mode is valid */
			/* make future mknod/mkfifo set mode bits exactly */
			umask(0);
	}

#إذا ENABLE_SELINUX
	إذا (opt & 2) {
		selinux_or_die();
		setfscreatecon_or_die(scontext);
	}
#پایان_اگر

	أرجع نما;
}
