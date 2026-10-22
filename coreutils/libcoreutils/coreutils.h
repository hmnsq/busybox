/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

افزودن_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

تعریفنوع صحیح (*stat_func)(ثابت نویسه *fn, ساختار stat *ps);

صحیح cp_mv_stat2(ثابت نویسه *fn, ساختار stat *fn_stat, stat_func sf) FAST_FUNC;
صحیح cp_mv_stat(ثابت نویسه *fn, ساختار stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(نویسه **argv) FAST_FUNC;

برداشت_SAVED_FUNCTION_VISIBILITY

#endif
