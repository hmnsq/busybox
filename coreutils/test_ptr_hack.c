/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2008 by Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

काष्ठा test_statics;

#अगर_अघोषित GCC_COMBINE

/* We cheat here. It is declared as const ptr in libbb.h,
 * but here we make it live in R/W memory */
काष्ठा test_statics *test_ptr_to_statics;

#अन्यथा

/* gcc -combine will see through and complain */
/* Using alternative method which is more likely to break
 * on weird architectures, compilers, linkers and so on */
काष्ठा test_statics *स्थिर test_ptr_to_statics __attribute__ ((section (".data")));

#पूर्ण_अगर
