/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

संरचना test_statics;

#यदि_अपरिभाषितम् GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
संरचना test_statics *test_ptr_गन्तव्यम्_statics;

#अन्यथा

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
संरचना test_statics *ध्रुव test_ptr_गन्तव्यम्_statics __attribute__ ((section (".data")));

#यदि_अन्तः
