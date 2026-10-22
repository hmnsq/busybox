/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#यदि_अपरिभाषितम् COREUTILS_H
#परिभाषय COREUTILS_H 1

प्रक्षिप_AND_समुच्चयः_FUNCTION_VISIBILITY_गन्तव्यम्_HIDDEN

प्रकारनिर्देश पूर्णाङ्क (*stat_func)(ध्रुव वर्ण *fn, संरचना stat *ps);

पूर्णाङ्क cp_mv_stat2(ध्रुव वर्ण *fn, संरचना stat *fn_stat, stat_func sf) FAST_FUNC;
पूर्णाङ्क cp_mv_stat(ध्रुव वर्ण *fn, संरचना stat *fn_stat) FAST_FUNC;

बहुलकम्_t getopt_mk_fifo_nod(वर्ण **argv) FAST_FUNC;

निष्कासय_SAVED_FUNCTION_VISIBILITY

#यदि_अन्तः
