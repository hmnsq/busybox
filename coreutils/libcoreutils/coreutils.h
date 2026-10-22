/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ढकला_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

प्रकारव्याख्या पूर्णांक (*stat_func)(स्थिरांक अक्षर *fn, संरचना stat *ps);

पूर्णांक cp_mv_stat2(स्थिरांक अक्षर *fn, संरचना stat *fn_stat, stat_func sf) FAST_FUNC;
पूर्णांक cp_mv_stat(स्थिरांक अक्षर *fn, संरचना stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(अक्षर **argv) FAST_FUNC;

काढा_SAVED_FUNCTION_VISIBILITY

#endif
