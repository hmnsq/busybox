/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

typedef จำนวนเต็ม (*stat_func)(const var_na *fn, sa_nracanaa stat *ps);

จำนวนเต็ม cp_mv_stat2(const var_na *fn, sa_nracanaa stat *fn_stat, stat_func sf) FAST_FUNC;
จำนวนเต็ม cp_mv_stat(const var_na *fn, sa_nracanaa stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(var_na **argv) FAST_FUNC;

POP_SAVED_FUNCTION_VISIBILITY

#endif
