/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ଠେଲ_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stat_func)(ଅଚଳ ଅକ୍ଷର *fn, ସଂରଚନା stat *ps);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cp_mv_stat2(ଅଚଳ ଅକ୍ଷର *fn, ସଂରଚନା stat *fn_stat, stat_func sf) FAST_FUNC;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cp_mv_stat(ଅଚଳ ଅକ୍ଷର *fn, ସଂରଚନା stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(ଅକ୍ଷର **argv) FAST_FUNC;

କାଢ଼_SAVED_FUNCTION_VISIBILITY

#endif
