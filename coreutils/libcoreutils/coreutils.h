/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ঠেলো_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

প্রকারনির্দেশ পূর্ণাঙ্ক (*stat_func)(ধ্রুৱ ৱর্ণ *fn, সংরচনা stat *ps);

পূর্ণাঙ্ক cp_mv_stat2(ধ্রুৱ ৱর্ণ *fn, সংরচনা stat *fn_stat, stat_func sf) FAST_FUNC;
পূর্ণাঙ্ক cp_mv_stat(ধ্রুৱ ৱর্ণ *fn, সংরচনা stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(ৱর্ণ **argv) FAST_FUNC;

বেৰ_কৰো_SAVED_FUNCTION_VISIBILITY

#endif
