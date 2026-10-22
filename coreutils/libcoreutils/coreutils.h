/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ COREUTILS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COREUTILS_H 1

ਧੱਕੋ_AND_ਸੈੱਟ_FUNCTION_VISIBILITY_ਮੰਜ਼ਿਲ_HIDDEN

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*stat_func)(ਅਚਲ ਅੱਖਰ *fn, ਬਣਤਰ stat *ps);

ਪੂਰਨ cp_mv_stat2(ਅਚਲ ਅੱਖਰ *fn, ਬਣਤਰ stat *fn_stat, stat_func sf) FAST_FUNC;
ਪੂਰਨ cp_mv_stat(ਅਚਲ ਅੱਖਰ *fn, ਬਣਤਰ stat *fn_stat) FAST_FUNC;

ਬਹੁਲਕ_t getopt_mk_fifo_nod(ਅੱਖਰ **argv) FAST_FUNC;

ਕੱਢੋ_SAVED_FUNCTION_VISIBILITY

#ਜੇ_ਅੰਤ
