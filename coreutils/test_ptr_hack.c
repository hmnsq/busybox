/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

ساخت test_statics;

#اگر_غیر_تعریف_شدہ GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
ساخت test_statics *test_ptr_مقصد_statics;

#ورنہ

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
ساخت test_statics *ثابت test_ptr_مقصد_statics __attribute__ ((section (".data")));

#اگر_ختم
