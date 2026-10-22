/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef COREUTILS_H
#define COREUTILS_H 1

തള്ളുക_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

തരനിർവചനം പൂർണ്ണസംഖ്യ (*stat_func)(സ്ഥിരം അക്ഷരം *fn, ഘടന stat *ps);

പൂർണ്ണസംഖ്യ cp_mv_stat2(സ്ഥിരം അക്ഷരം *fn, ഘടന stat *fn_stat, stat_func sf) FAST_FUNC;
പൂർണ്ണസംഖ്യ cp_mv_stat(സ്ഥിരം അക്ഷരം *fn, ഘടന stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(അക്ഷരം **argv) FAST_FUNC;

എടുക്കുക_SAVED_FUNCTION_VISIBILITY

#endif
