/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ڈالو_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

تعريفنوع صحيح (*stat_func)(ثابت حرف *fn, بنية stat *ps);

صحيح cp_mv_stat2(ثابت حرف *fn, بنية stat *fn_stat, stat_func sf) FAST_FUNC;
صحيح cp_mv_stat(ثابت حرف *fn, بنية stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(حرف **argv) FAST_FUNC;

نکالو_SAVED_FUNCTION_VISIBILITY

#endif
