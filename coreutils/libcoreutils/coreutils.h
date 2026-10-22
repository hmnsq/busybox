/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ঠেলো_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

প্রকারলেখ পূর্ণসংখ্যা (*stat_func)(ধ্রুবক অক্ষর *fn, কাঠামো stat *ps);

পূর্ণসংখ্যা cp_mv_stat2(ধ্রুবক অক্ষর *fn, কাঠামো stat *fn_stat, stat_func sf) FAST_FUNC;
পূর্ণসংখ্যা cp_mv_stat(ধ্রুবক অক্ষর *fn, কাঠামো stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(অক্ষর **argv) FAST_FUNC;

বের_করো_SAVED_FUNCTION_VISIBILITY

#endif
