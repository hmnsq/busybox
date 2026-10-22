/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

தள்ளு_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

வகைவரை முழு (*stat_func)(மாறிலி எழுத்து *fn, கட்டமைப்பு stat *ps);

முழு cp_mv_stat2(மாறிலி எழுத்து *fn, கட்டமைப்பு stat *fn_stat, stat_func sf) FAST_FUNC;
முழு cp_mv_stat(மாறிலி எழுத்து *fn, கட்டமைப்பு stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(எழுத்து **argv) FAST_FUNC;

எடு_SAVED_FUNCTION_VISIBILITY

#endif
