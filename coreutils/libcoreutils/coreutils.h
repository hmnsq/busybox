/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ધકેલો_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

પ્રકારવ્યાખ્યા પૂર્ણાંક (*stat_func)(અચલ અક્ષર *fn, સંરચના stat *ps);

પૂર્ણાંક cp_mv_stat2(અચલ અક્ષર *fn, સંરચના stat *fn_stat, stat_func sf) FAST_FUNC;
પૂર્ણાંક cp_mv_stat(અચલ અક્ષર *fn, સંરચના stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(અક્ષર **argv) FAST_FUNC;

કાઢો_SAVED_FUNCTION_VISIBILITY

#endif
