/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

ರಚನೆ test_statics;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
ರಚನೆ test_statics *test_ptr_ಗಮ್ಯ_statics;

#ಇಲ್ಲದಿದ್ದರೆ

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
ರಚನೆ test_statics *ಸ್ಥಿರಾಂಕ test_ptr_ಗಮ್ಯ_statics __attribute__ ((section (".data")));

#ಆದರೆ_ಅಂತ್ಯ
