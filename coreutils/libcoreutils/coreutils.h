/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

ತಳ್ಳು_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*stat_func)(ಧ್ರುವ ವರ್ಣ *fn, ಸಂರಚನಾ stat *ps);

ಪೂರ್ಣಾಙ್ಕ cp_mv_stat2(ಧ್ರುವ ವರ್ಣ *fn, ಸಂರಚನಾ stat *fn_stat, stat_func sf) FAST_FUNC;
ಪೂರ್ಣಾಙ್ಕ cp_mv_stat(ಧ್ರುವ ವರ್ಣ *fn, ಸಂರಚನಾ stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(ವರ್ಣ **argv) FAST_FUNC;

ತೆಗೆ_SAVED_FUNCTION_VISIBILITY

#endif
