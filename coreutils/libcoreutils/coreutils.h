/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#अगर_अघोषित COREUTILS_H
#घोषणा COREUTILS_H 1

धकेलो_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

प्रकार पूर्णांक (*stat_func)(स्थिर अक्षर *fn, काष्ठा stat *ps);

पूर्णांक cp_mv_stat2(स्थिर अक्षर *fn, काष्ठा stat *fn_stat, stat_func sf) FAST_FUNC;
पूर्णांक cp_mv_stat(स्थिर अक्षर *fn, काष्ठा stat *fn_stat) FAST_FUNC;

mode_t getopt_mk_fifo_nod(अक्षर **argv) FAST_FUNC;

निकालो_SAVED_FUNCTION_VISIBILITY

#पूर्ण_अगर
