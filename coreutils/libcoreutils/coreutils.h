/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#اگر_غیر_تعریف_شدہ COREUTILS_H
#تعریف_کرو COREUTILS_H 1

ڈالو_AND_سیٹ_FUNCTION_VISIBILITY_مقصد_HIDDEN

قسمتعریف صحیح (*stat_func)(ثابت حرف *fn, ساخت stat *ps);

صحیح cp_mv_stat2(ثابت حرف *fn, ساخت stat *fn_stat, stat_func sf) FAST_FUNC;
صحیح cp_mv_stat(ثابت حرف *fn, ساخت stat *fn_stat) FAST_FUNC;

منوال_t getopt_mk_fifo_nod(حرف **argv) FAST_FUNC;

نکالو_SAVED_FUNCTION_VISIBILITY

#اگر_ختم
