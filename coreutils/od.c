/* vi: set sw=4 ts=4: */
/*
 * od implementation for busybox
 * Based on code from util-linux v 2.11l
 *
 * Copyright (c) 1990
 * The Regents of the University of California.  All rights reserved.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * Original copyright notice is retained at the end of this file.
 */
//config:config OD
//config:	bool "od (11 kb)"
//config:	default y
//config:	help
//config:	od is used to dump binary files in octal and other formats.

//applet:IF_OD(APPLET(od, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_OD) += od.o

//usage:#if !ENABLE_DESKTOP
//usage:#define od_trivial_usage
//usage:       "[-abcdeFfhiloxsv] [FILE]"
// We also support -BDOHXIL, but they are not documented in coreutils 9.1
// manpage/help, so don't show them either.
//usage:#define od_full_usage "\n\n"
//usage:       "Print FILE (or stdin) unambiguously, as octal bytes by default"
//usage:#endif

#समावेश "libbb.h"
#अगर ENABLE_DESKTOP
/* This one provides -t (busybox's own build script needs it) */
#समावेश "od_bloaty.c"
#अन्यथा

#समावेश "dump.h"

अटल व्योम
odoffset(dumper_t *dumper, पूर्णांक argc, अक्षर ***argvp)
{
	अक्षर *अंक, *p;
	पूर्णांक base;
	अक्षर *end;

	/*
	 * The offset syntax of od(1) was genuinely bizarre.  First, if
	 * it started with a plus it had to be an offset.  Otherwise, if
	 * there were at least two arguments, a number or lower-case 'x'
	 * followed by a number makes it an offset.  By default it was
	 * octal; if it started with 'x' or '0x' it was hex.  If it ended
	 * in a '.', it was decimal.  If a 'b' or 'B' was appended, it
	 * multiplied the number by 512 or 1024 byte units.  There was
	 * no way to assign a block count to a hex offset.
	 *
	 * We assumes it's a file if the offset is bad.
	 */
	p = **argvp;

	अगर (!p) {
		/* hey someone is probably piping to us ... */
		वापस;
	}

	अगर ((*p != '+')
		&& (argc < 2
			|| (!है_अंक(p[0])
				&& ((p[0] != 'x') || !है_षष्ठादशक(p[1])))))
		वापस;

	base = 0;
	/*
	 * skip over leading '+', 'x[0-9a-fA-f]' or '0x', and
	 * set base.
	 */
	अगर (p[0] == '+')
		++p;
	अगर (p[0] == 'x' && है_षष्ठादशक(p[1])) {
		++p;
		base = 16;
	} अन्यथा अगर (p[0] == '0' && p[1] == 'x') {
		p += 2;
		base = 16;
	}

	/* skip over the number */
	अगर (base == 16)
		क्रम (अंक = p; है_षष्ठादशक(*p); ++p)
			जारी;
	अन्यथा
		क्रम (अंक = p; है_अंक(*p); ++p)
			जारी;

	/* check for no number */
	अगर (अंक == p)
		वापस;

	/* if terminates with a '.', base is decimal */
	अगर (*p == '.') {
		अगर (base)
			वापस;
		base = 10;
	}

	dumper->dump_skip = म_से_दीर्घ(अंक, &end, base ? base : 8);

	/* if end isn't the same as p, we got a non-octal digit */
	अगर (end != p)
		dumper->dump_skip = 0;
	अन्यथा {
		अगर (*p) {
			अगर (*p == 'b') {
				dumper->dump_skip *= 512;
				++p;
			} अन्यथा अगर (*p == 'B') {
				dumper->dump_skip *= 1024;
				++p;
			}
		}
		अगर (*p)
			dumper->dump_skip = 0;
		अन्यथा {
			++*argvp;
			/*
			 * If the offset uses a non-octal base, the base of
			 * the offset is changed as well.  This isn't pretty,
			 * but it's easy.
			 */
#घोषणा TYPE_OFFSET 7
			{
				अक्षर x_or_d;
				अगर (base == 16) {
					x_or_d = 'x';
					जाओ DO_X_OR_D;
				}
				अगर (base == 10) {
					x_or_d = 'd';
 DO_X_OR_D:
					dumper->fshead->nextfu->fmt[TYPE_OFFSET]
						= dumper->fshead->nextfs->nextfu->fmt[TYPE_OFFSET]
						= x_or_d;
				}
			}
		}
	}
}

// bb_dump_add():
// A format string contains format units separated by [optional] whitespace.
// A format unit contains up to three items: an iteration count, a byte count,
// and a format.
// The iteration count is an optional integer (default 1).
// Each format is applied iteration count times.
// The byte count is an optional integer. It defines the number
// of bytes to be interpreted by each iteration of the format.
// If an iteration count and/or a byte count is specified, a slash must be
// placed after the iteration count and/or before the byte count
// to disambiguate them.
// The printf-style format is required and must be surrounded by " "s.
// (Below, each string contains two format units)
अटल स्थिर अक्षर *स्थिर add_strings[] ALIGN_PTR = {
	"16/1 \" %3_u\""   "\"\n\"",            /* 0: a */
	"8/2 \" %06o\""    "\"\n\"",            /* 1: B (undocumented in od), o */
	"16/1 \" %03o\""   "\"\n\"",            /* 2: b */
	"16/1 \" %3_c\""   "\"\n\"",            /* 3: c */
	"8/2 \" %5u\""     "\"\n\"",            /* 4: d */
	"4/4 \" %10u\""    "\"\n\"",            /* 5: D */
	"2/8 \" %24.14e\"" "\"\n\"",            /* 6: e (undocumented in od), F */
	"4/4 \" %15.7e\""  "\"\n\"",            /* 7: f */
	"4/4 \" %08x\""    "\"\n\"",            /* 8: H, X */
	"8/2 \" %04x\""    "\"\n\"",            /* 9: h, x */
	"4/4 \" %11d\""    "\"\n\"",            /* 10: i */
	"4/4 \" %011o\""   "\"\n\"",            /* 11: O */
	"8/2 \" %6d\""     "\"\n\"",            /* 12: s */
	/* -I,L,l: depend on word width of the arch (what is "long"?) */
#अगर अच_दीर्घ_उच्च > 0xffffffff
	"2/8 \" %20lld\""  "\"\n\"",            /* 13: I, L, l */
#घोषणा L_ 13
#अन्यथा
	/* 32-bit arch: -I,L,l are the same as -i */
#घोषणा L_ 10
#पूर्ण_अगर
};

अटल स्थिर अक्षर od_opts[] ALIGN1 = "aBbcDdeFfHhIiLlOoXxsv";

अटल स्थिर अक्षर od_o2si[] ALIGN1 = {
	0, 1, 2, 3, 5,     /* aBbcD */
	4, 6, 6, 7, 8,     /* deFfH */
	9, L_, 10, L_, L_, /* hIiLl */
	11, 1, 8, 9, 12    /* OoXxs */
};

पूर्णांक od_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक od_main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक ch;
	पूर्णांक first = 1;
	अक्षर *p;
	dumper_t *dumper = alloc_dumper();

	जबतक ((ch = getopt(argc, argv, od_opts)) > 0) {
		अगर (ch == 'v') {
			dumper->dump_vflag = ALL;
		} अन्यथा अगर (((p = म_अक्षर(od_opts, ch)) != शून्य) && (*p != '\0')) {
			अगर (first) {
				first = 0;
				bb_dump_add(dumper, "\"%07.7_Ao\n\"");
				bb_dump_add(dumper, "\"%07.7_ao\"");
			} अन्यथा {
				bb_dump_add(dumper, "\"       \"");
			}
			bb_dump_add(dumper, add_strings[(पूर्णांक)od_o2si[(p - od_opts)]]);
		} अन्यथा {  /* P, p, w, or other unhandled */
			bb_show_usage();
		}
	}
	अगर (!dumper->fshead) {
		bb_dump_add(dumper, "\"%07.7_Ao\n\"");
		bb_dump_add(dumper, "\"%07.7_ao\"");
		bb_dump_add(dumper, add_strings[1]); /* -o format is default */
	}
	dumper->od_eofstring = "\n";

	argc -= optind;
	argv += optind;

	odoffset(dumper, argc, &argv);

	वापस bb_dump_dump(dumper, argv);
}
#पूर्ण_अगर /* !ENABLE_DESKTOP */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
