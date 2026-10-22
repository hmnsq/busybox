/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#جيڪڏهن_وصف_نه_ٿيل COREUTILS_H
#وصف_ڪريو COREUTILS_H 1

ڌڪو_AND_سيٽ_FUNCTION_VISIBILITY_منزل_HIDDEN

تعريفنوع صحيح (*stat_func)(ثابت حرف *fn, بنية stat *ps);

صحيح cp_mv_stat2(ثابت حرف *fn, بنية stat *fn_stat, stat_func sf) FAST_FUNC;
صحيح cp_mv_stat(ثابت حرف *fn, بنية stat *fn_stat) FAST_FUNC;

گهڻائي_t getopt_mk_fifo_nod(حرف **argv) FAST_FUNC;

ڪڍو_SAVED_FUNCTION_VISIBILITY

#جيڪڏهن_پورو
