/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

sa_nracanaa test_statics;

#ifndef GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
sa_nracanaa test_statics *test_ptr_to_statics;

#มิฉะนั้น

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
sa_nracanaa test_statics *const test_ptr_to_statics __attribute__ ((section (".data")));

#endif
