/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

ਬਣਤਰ test_statics;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
ਬਣਤਰ test_statics *test_ptr_ਮੰਜ਼ਿਲ_statics;

#ਨਹੀਂਤਾਂ

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
ਬਣਤਰ test_statics *ਅਚਲ test_ptr_ਮੰਜ਼ਿਲ_statics __attribute__ ((section (".data")));

#ਜੇ_ਅੰਤ
