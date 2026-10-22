/* vi: set sw=4 ts=4: */
/*
 * stty -- change and print terminal line settings
 * Copyright (C) 1990-1999 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* David MacKenzie <djm@gnu.ai.mit.edu>
 *
 * Special for busybox ported by Vladimir Oleynik <dzo@simtreas.ru> 2001
 */
//config:config STTY
//config:	bool "stty (9.2 kb)"
//config:	default y
//config:	help
//config:	stty is used to change and print terminal line settings.

//applet:IF_STTY(APPLET_NOEXEC(stty, stty, BB_DIR_BIN, BB_SUID_DROP, stty))

//kbuild:lib-$(CONFIG_STTY) += stty.o

//usage:#define stty_trivial_usage
//usage:       "[-a|g] [-F DEVICE] [SETTING]..."
//usage:#define stty_full_usage "\n\n"
//usage:       "Without arguments, prints baud rate, line discipline,\n"
//usage:       "and deviations from stty sane\n"
//usage:     "\n	-F DEVICE	Open device instead of stdin"
//usage:     "\n	-a		Print all current settings in human-readable form"
//usage:     "\n	-g		Print in stty-readable form"
//usage:     "\n	[SETTING]	See manpage"

/* If no args are given, write to stdout the baud rate and settings that
 * have been changed from their defaults.  Mode reading and changes
 * are done on the specified device, or stdin if none was specified.
 */

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((अचिन्हित अक्षर) 0)
#endif

#define Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#ifndef CINTR
# define CINTR Control('c')
#endif
#ifndef CQUIT
# define CQUIT 28
#endif
#ifndef CERASE
# define CERASE 127
#endif
#ifndef CKILL
# define CKILL Control('u')
#endif
#ifndef CEOF
# define CEOF Control('d')
#endif
#ifndef CEOL
# define CEOL _POSIX_VDISABLE
#endif
#ifndef CSTART
# define CSTART Control('q')
#endif
#ifndef CSTOP
# define CSTOP Control('s')
#endif
#ifndef CSUSP
# define CSUSP Control('z')
#endif
#जर defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#जर defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#जर defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#जर defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#जर defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#जर defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#जर defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#जर !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#जर defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#जर defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#जर defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#जर defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#जर defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#जर defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#जर defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#जर defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#जर defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#जर defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#endif

/* Save us from #ifdef forest plague */
#ifndef BSDLY
# define BSDLY 0
#endif
#ifndef CIBAUD
# define CIBAUD 0
#endif
#ifndef CRDLY
# define CRDLY 0
#endif
#ifndef CMSPAR
# define CMSPAR 0
#endif
#ifndef CRTSCTS
# define CRTSCTS 0
#endif
#ifndef ECHOCTL
# define ECHOCTL 0
#endif
#ifndef ECHOKE
# define ECHOKE 0
#endif
#ifndef ECHOPRT
# define ECHOPRT 0
#endif
#ifndef FFDLY
# define FFDLY 0
#endif
#ifndef IEXTEN
# define IEXTEN 0
#endif
#ifndef IMAXBEL
# define IMAXBEL 0
#endif
#ifndef IUCLC
# define IUCLC 0
#endif
#ifndef IXANY
# define IXANY 0
#endif
#ifndef NLDLY
# define NLDLY 0
#endif
#ifndef OCRNL
# define OCRNL 0
#endif
#ifndef OFDEL
# define OFDEL 0
#endif
#ifndef OFILL
# define OFILL 0
#endif
#ifndef OLCUC
# define OLCUC 0
#endif
#ifndef ONLCR
# define ONLCR 0
#endif
#ifndef ONLRET
# define ONLRET 0
#endif
#ifndef ONOCR
# define ONOCR 0
#endif
#ifndef OXTABS
# define OXTABS 0
#endif
#ifndef TABDLY
# define TABDLY 0
#endif
#ifndef TAB1
# define TAB1 0
#endif
#ifndef TAB2
# define TAB2 0
#endif
#ifndef TOSTOP
# define TOSTOP 0
#endif
#ifndef VDSUSP
# define VDSUSP 0
#endif
#ifndef VEOL2
# define VEOL2 0
#endif
#ifndef VFLUSHO
# define VFLUSHO 0
#endif
#ifndef VLNEXT
# define VLNEXT 0
#endif
#ifndef VREPRINT
# define VREPRINT 0
#endif
#ifndef VSTATUS
# define VSTATUS 0
#endif
#ifndef VSWTCH
# define VSWTCH 0
#endif
#ifndef VTDLY
# define VTDLY 0
#endif
#ifndef VWERASE
# define VWERASE 0
#endif
#ifndef XCASE
# define XCASE 0
#endif
#ifndef IUTF8
# define IUTF8 0
#endif

/* Which speeds to set */
गणना speed_setting {
	आदान_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
गणना {
	control, आदान, output, local, combination
};
स्थिर tcflag_t *get_ptr_to_tcflag(अचिन्हित type, स्थिरांक संरचना termios *mode)
{
	स्थिर स्थिरांक uint8_t tcflag_offsets[] ALIGN1 = {
		दुरत्व(संरचना termios, c_cflag), /* control */
		दुरत्व(संरचना termios, c_iflag), /*आदान*/
		दुरत्व(संरचना termios, c_oflag), /* output */
		दुरत्व(संरचना termios, c_lflag)  /* local */
	};
	जर (type <= local) {
		परत (tcflag_t*) (((अक्षर*)mode) + tcflag_offsets[type]);
	}
	परत रिक्त;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
संरचना mode_info {
	स्थिरांक uint8_t type;           /* Which structure element to change    */
	स्थिरांक uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#जर   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	स्थिरांक uint8_t mask;
#किंवाजर (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	स्थिरांक uint16_t mask;
#अन्यथा
	स्थिरांक tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	स्थिरांक tcflag_t bits;          /* Bits to set for this mode            */
};

गणना {
	/* Must match mode_name[] and mode_info[] order! */
	IDX_evenp = 0,
	IDX_parity,
	IDX_oddp,
	IDX_nl,
	IDX_ek,
	IDX_sane,
	IDX_cooked,
	IDX_raw,
	IDX_pass8,
	IDX_litout,
	IDX_cbreak,
	IDX_crt,
	IDX_dec,
#जर IXANY
	IDX_decctlq,
#endif
#जर TABDLY || OXTABS
	IDX_tabs,
#endif
#जर XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
स्थिर स्थिरांक अक्षर mode_name[] ALIGN1 =
	MI_ENTRY("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("ek",       combination, OMIT,              0,          0 )
	MI_ENTRY("sane",     combination, OMIT,              0,          0 )
	MI_ENTRY("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("crt",      combination, OMIT,              0,          0 )
	MI_ENTRY("dec",      combination, OMIT,              0,          0 )
#जर IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#जर TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#जर XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#जर CMSPAR
	MI_ENTRY("cmspar",   control,     REV,               CMSPAR,     0 )
#endif
	MI_ENTRY("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ENTRY("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ENTRY("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ENTRY("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ENTRY("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ENTRY("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ENTRY("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ENTRY("cread",    control,     SANE_SET   | REV,  CREAD,      0 )
	MI_ENTRY("clocal",   control,     REV,               CLOCAL,     0 )
#जर CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   आदान,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   आदान,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   आदान,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   आदान,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    आदान,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   आदान,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    आदान,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    आदान,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    आदान,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     आदान,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    आदान,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   आदान,       OMIT       | REV,  IXOFF,      0 )
#जर IUCLC
	MI_ENTRY("iuclc",    आदान,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#जर IXANY
	MI_ENTRY("ixany",    आदान,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#जर IMAXBEL
	MI_ENTRY("imaxbel",  आदान,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#जर IUTF8
	MI_ENTRY("iutf8",    आदान,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#जर OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#जर OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#जर ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#जर ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#जर ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#जर OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#जर OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#जर NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#जर CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#जर TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# जर TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# जर TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# जर OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#जर BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#जर VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#जर FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#जर IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#जर XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#जर TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#जर ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#जर ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#जर ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
	;

#undef MI_ENTRY
#define MI_ENTRY(N,T,F,B,M) { T, F, M, B },

स्थिर स्थिरांक संरचना mode_info mode_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_ENTRY("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("ek",       combination, OMIT,              0,          0 )
	MI_ENTRY("sane",     combination, OMIT,              0,          0 )
	MI_ENTRY("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("crt",      combination, OMIT,              0,          0 )
	MI_ENTRY("dec",      combination, OMIT,              0,          0 )
#जर IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#जर TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#जर XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#जर CMSPAR
	MI_ENTRY("cmspar",   control,     REV,               CMSPAR,     0 )
#endif
	MI_ENTRY("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ENTRY("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ENTRY("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ENTRY("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ENTRY("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ENTRY("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ENTRY("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ENTRY("cread",    control,     SANE_SET   | REV,  CREAD,      0 )
	MI_ENTRY("clocal",   control,     REV,               CLOCAL,     0 )
#जर CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   आदान,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   आदान,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   आदान,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   आदान,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    आदान,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   आदान,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    आदान,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    आदान,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    आदान,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     आदान,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    आदान,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   आदान,       OMIT       | REV,  IXOFF,      0 )
#जर IUCLC
	MI_ENTRY("iuclc",    आदान,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#जर IXANY
	MI_ENTRY("ixany",    आदान,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#जर IMAXBEL
	MI_ENTRY("imaxbel",  आदान,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#जर IUTF8
	MI_ENTRY("iutf8",    आदान,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#जर OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#जर OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#जर ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#जर ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#जर ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#जर OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#जर OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#जर NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#जर CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#जर TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# जर TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# जर TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# जर OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#जर BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#जर VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#जर FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#जर IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#जर XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#जर TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#जर ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#जर ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#जर ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

गणना {
	अंक_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
संरचना control_info {
	स्थिरांक uint8_t saneval;  /* Value to set for 'stty sane' */
	स्थिरांक uint8_t offset;   /* Offset in c_cc */
};

गणना {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#जर VEOL2
	CIDX_eol2,
#endif
#जर VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#जर VDSUSP
	CIDX_dsusp,
#endif
#जर VREPRINT
	CIDX_rprnt,
#endif
#जर VWERASE
	CIDX_werase,
#endif
#जर VLNEXT
	CIDX_lnext,
#endif
#जर VFLUSHO
	CIDX_flush,
#endif
#जर VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
स्थिर स्थिरांक अक्षर control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#जर VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#जर VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#जर VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#जर VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#जर VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#जर VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#जर VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#जर VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

स्थिर स्थिरांक संरचना control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#जर VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#जर VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#जर VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#जर VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#जर VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#जर VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#जर VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#जर VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

गणना {
	अंक_control_info = ARRAY_SIZE(control_info)
};


संरचना globals {
	स्थिरांक अक्षर *device_name;
	/* The width of the screen, for output wrapping */
	अचिन्हित max_col;
	/* Current position, to know when to wrap */
	अचिन्हित current_col;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करा { \
	setup_common_bufsiz(); \
	G.device_name = bb_संदेश_standard_आदान; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} पर्यंत (0)

स्थिर शून्य set_speed_or_die(गणना speed_setting type, स्थिरांक अक्षर *arg,
					संरचना termios *mode)
{
	speed_t baud;

	baud = tty_मूल्य_to_baud(xatou(arg));

	जर (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	जर (type != आदान_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

स्थिर NORETURN शून्य perror_on_device_and_die(स्थिरांक अक्षर *fmt)
{
	bb_perror_संदेश_and_die(fmt, G.device_name);
}

स्थिर शून्य perror_on_device(स्थिरांक अक्षर *fmt)
{
	bb_perror_संदेश(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
स्थिर शून्य wrapf(स्थिरांक अक्षर *message, ...)
{
	अक्षर buf[128];
	बहु_सूची args;
	अचिन्हित buflen;

	बहु_शुरू(args, message);
	buflen = vsnprintf(buf, आकार(buf), message, args);
	बहु_पूर्ण(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	जर (!buflen || buflen >= आकार(buf)) परत;

	जर (G.current_col > 0) {
		G.current_col++;
		जर (buf[0] != '\n') {
			जर (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} अन्यथा {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	जर (buf[buflen-1] == '\n')
		G.current_col = 0;
}

स्थिर शून्य newline(शून्य)
{
	जर (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
स्थिर शून्य set_window_size(पूर्णांक rows, पूर्णांक cols)
{
	संरचना winsize win = { 0, 0, 0, 0 };

	जर (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		जर (त्रुटि_सं != EINVAL) {
			जा bail;
		}
		स_रखो(&win, 0, आकार(win));
	}

	जर (rows >= 0)
		win.ws_row = rows;
	जर (cols >= 0)
		win.ws_col = cols;

	जर (ioctl(STDIN_FILENO, TIOCSWINSZ, (अक्षर *) &win))
bail:
		perror_on_device("%s");
}
#endif

स्थिर शून्य display_window_size(पूर्णांक fancy)
{
	स्थिरांक अक्षर *fmt_str = "%s\0%s: no size information for this device";
	अचिन्हित width, height;

	जर (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		जर ((त्रुटि_सं != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} अन्यथा {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

स्थिर स्थिरांक संरचना suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

स्थिर स्थिरांक संरचना mode_info *find_mode(स्थिरांक अक्षर *name)
{
	पूर्णांक i = index_in_strings(mode_name, name);
	परत i >= 0 ? &mode_info[i] : रिक्त;
}

स्थिर स्थिरांक संरचना control_info *find_control(स्थिरांक अक्षर *name)
{
	पूर्णांक i = index_in_strings(control_name, name);
	परत i >= 0 ? &control_info[i] : रिक्त;
}

गणना {
	param_need_arg = 0x80,
	param_ओळ    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

स्थिर पूर्णांक find_param(स्थिरांक अक्षर *name)
{
	स्थिर स्थिरांक अक्षर params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	पूर्णांक i = index_in_strings(params, name) + 1;
	जर (i == 0)
		परत 0;
	जर (i != 5 && i != 6)
		i |= 0x80;
	परत i;
}

स्थिर पूर्णांक recover_mode(स्थिरांक अक्षर *arg, संरचना termios *mode)
{
	पूर्णांक i, n;
	अचिन्हित chr;
	अचिन्हित दीर्घ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	जर (माला_पूछो(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		परत 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	साठी (i = 0; i < NCCS; ++i) {
		जर (माला_पूछो(arg, ":%x%n", &chr, &n) != 1)
			परत 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	जर (*arg != '\0')
		परत 0;

	परत 1;
}

स्थिर शून्य display_recoverable(स्थिरांक संरचना termios *mode,
				पूर्णांक UNUSED_PARAM dummy)
{
	पूर्णांक i;
	म_लिखो("%lx:%lx:%lx:%lx",
		   (अचिन्हित दीर्घ) mode->c_iflag, (अचिन्हित दीर्घ) mode->c_oflag,
		   (अचिन्हित दीर्घ) mode->c_cflag, (अचिन्हित दीर्घ) mode->c_lflag);
	साठी (i = 0; i < NCCS; ++i)
		म_लिखो(":%x", (अचिन्हित पूर्णांक) mode->c_cc[i]);
	bb_putchar('\n');
}

स्थिर शून्य display_speed(स्थिरांक संरचना termios *mode, पूर्णांक fancy)
{
	//____________________ 01234567 8 9
	स्थिरांक अक्षर *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	अचिन्हित दीर्घ ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	जर (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	जर (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_मूल्य(ispeed), tty_baud_to_मूल्य(ospeed));
}

स्थिर शून्य do_display(स्थिरांक संरचना termios *mode, पूर्णांक all)
{
	पूर्णांक i;
	tcflag_t *bitsp;
	अचिन्हित दीर्घ mask;
	पूर्णांक prev_type = control;

	display_speed(mode, 1);
	जर (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_ओळ);
#अन्यथा
	newline();
#endif

	साठी (i = 0; i != CIDX_min; ++i) {
		अक्षर ch;
		अक्षर buf10[10];

		/* If swtch is the same as susp, don't print both */
#जर VSWTCH == VSUSP
		जर (i == CIDX_swtch)
			सुरूठेवा;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#जर VEOF == VMIN
		जर (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			सुरूठेवा;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		जर (ch == _POSIX_VDISABLE)
			म_नकल(buf10, "<undef>");
		अन्यथा
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#जर VEOF == VMIN
	जर ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	साठी (i = 0; i < अंक_mode_info; ++i) {
		जर (mode_info[i].flags & OMIT)
			सुरूठेवा;
		जर (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		जर ((*bitsp & mask) == mode_info[i].bits) {
			जर (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} अन्यथा {
			जर ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

स्थिर शून्य sane_mode(संरचना termios *mode)
{
	पूर्णांक i;

	साठी (i = 0; i < अंक_control_info; ++i) {
#जर VMIN == VEOF
		जर (i == CIDX_min)
			थांबा;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	साठी (i = 0; i < अंक_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		जर (!bitsp)
			सुरूठेवा;
		val = *bitsp & ~((अचिन्हित दीर्घ)mode_info[i].mask);
		जर (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} अन्यथा
		जर (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

स्थिर शून्य set_mode(स्थिरांक संरचना mode_info *info, पूर्णांक reversed,
					संरचना termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	जर (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		जर (reversed)
			*bitsp = val & ~info->bits;
		अन्यथा
			*bitsp = val | info->bits;
		परत;
	}

	/* !bitsp - it's a "combination" mode */
	जर (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		जर (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} अन्यथा जर (info == &mode_info[IDX_oddp]) {
		जर (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} अन्यथा जर (info == &mode_info[IDX_nl]) {
		जर (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} अन्यथा {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			जर (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} अन्यथा जर (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} अन्यथा जर (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} अन्यथा जर (info == &mode_info[IDX_cbreak]) {
		जर (reversed)
			mode->c_lflag |= ICANON;
		अन्यथा
			mode->c_lflag &= ~ICANON;
	} अन्यथा जर (info == &mode_info[IDX_pass8]) {
		जर (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} अन्यथा जर (info == &mode_info[IDX_litout]) {
		जर (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} अन्यथा जर (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		जर ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#जर VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#जर VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} अन्यथा {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#जर IXANY
	अन्यथा जर (info == &mode_info[IDX_decctlq]) {
		जर (reversed)
			mode->c_iflag |= IXANY;
		अन्यथा
			mode->c_iflag &= ~IXANY;
	}
#endif
#जर TABDLY
	अन्यथा जर (info == &mode_info[IDX_tabs]) {
		जर (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		अन्यथा
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#जर OXTABS
	अन्यथा जर (info == &mode_info[IDX_tabs]) {
		जर (reversed)
			mode->c_oflag |= OXTABS;
		अन्यथा
			mode->c_oflag &= ~OXTABS;
	}
#endif
#जर XCASE && IUCLC && OLCUC
	अन्यथा जर (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		जर (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} अन्यथा {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	अन्यथा जर (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} अन्यथा जर (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		जर (IXANY) mode->c_iflag &= ~IXANY;
	}
}

स्थिर शून्य set_control_char_or_die(स्थिरांक संरचना control_info *info,
			स्थिरांक अक्षर *arg, संरचना termios *mode)
{
	अचिन्हित अक्षर मूल्य_2;

	जर (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		मूल्य_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	अन्यथा जर (arg[0] == '\0' || arg[1] == '\0')
		मूल्य_2 = arg[0];
	अन्यथा जर (म_भेद(arg, "^-") == 0 || म_भेद(arg, "undef") == 0)
		मूल्य_2 = _POSIX_VDISABLE;
	अन्यथा जर (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		मूल्य_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		जर (arg[1] == '?')
			मूल्य_2 = 127;
	} अन्यथा
		मूल्य_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = मूल्य_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

पूर्णांक stty_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक stty_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	संरचना termios mode;
	शून्य (*output_func)(स्थिरांक संरचना termios *, पूर्णांक);
	स्थिरांक अक्षर *file_name = रिक्त;
	पूर्णांक display_all = 0;
	पूर्णांक stty_state;
	पूर्णांक k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	पर्यंत (argv[++k]) {
		स्थिरांक संरचना mode_info *mp;
		स्थिरांक संरचना control_info *cp;
		स्थिरांक अक्षर *arg = argv[k];
		स्थिरांक अक्षर *argnext = argv[k+1];
		पूर्णांक param;

		जर (arg[0] == '-') {
			पूर्णांक i;
			mp = find_mode(arg+1);
			जर (mp) {
				जर (!(mp->flags & REV))
					जा invalid_argument;
				stty_state &= ~STTY_noargs;
				सुरूठेवा;
			}
			/* It is an option - parse it */
			i = 0;
			पर्यंत (arg[++i]) {
				निवड (arg[i]) {
				स्थिती 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					थांबा;
				स्थिती 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					थांबा;
				स्थिती 'F':
					जर (file_name)
						bb_simple_error_संदेश_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					जर (!file_name[0]) { /* nope, "-F device" */
						पूर्णांक p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						जर (!file_name)
							bb_error_संदेश_and_die(bb_संदेश_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						पर्यंत (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					जा end_option;
				पूर्वनिर्धारित:
					जा invalid_argument;
				}
			}
 end_option:
			सुरूठेवा;
		}

		mp = find_mode(arg);
		जर (mp) {
			stty_state &= ~STTY_noargs;
			सुरूठेवा;
		}

		cp = find_control(arg);
		जर (cp) {
			जर (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			सुरूठेवा;
		}

		param = find_param(arg);
		जर (param & param_need_arg) {
			जर (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			++k;
		}

		निवड (param) {
#ifdef __linux__
		स्थिती param_ओळ:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			थांबा;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		स्थिती param_rows:
		स्थिती param_cols:
		स्थिती param_columns:
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			थांबा;
		स्थिती param_size:
#endif
		स्थिती param_speed:
			थांबा;
		स्थिती param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(आदान_speed, argnext, &mode);
			थांबा;
		स्थिती param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			थांबा;
		पूर्वनिर्धारित:
			जर (recover_mode(arg, &mode) == 1) थांबा;
			जर (tty_मूल्य_to_baud(xatou(arg)) != (speed_t) -1) थांबा;
 invalid_argument:
			bb_error_संदेश_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	जर ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_संदेश_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	जर ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_संदेश_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	जर (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	स_रखो(&mode, 0, आकार(mode));
	जर (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	जर (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		परत निकास_सफल;
	}

	/* Second pass: perform actions */
	k = 0;
	पर्यंत (argv[++k]) {
		स्थिरांक संरचना mode_info *mp;
		स्थिरांक संरचना control_info *cp;
		स्थिरांक अक्षर *arg = argv[k];
		स्थिरांक अक्षर *argnext = argv[k+1];
		पूर्णांक param;

		जर (arg[0] == '-') {
			mp = find_mode(arg+1);
			जर (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			सुरूठेवा;
		}

		mp = find_mode(arg);
		जर (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			सुरूठेवा;
		}

		cp = find_control(arg);
		जर (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			सुरूठेवा;
		}

		param = find_param(arg);
		जर (param & param_need_arg) {
			++k;
		}

		निवड (param) {
#ifdef __linux__
		स्थिती param_ओळ:
			mode.c_ओळ = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			थांबा;
#endif
#ifdef TIOCGWINSZ
		स्थिती param_cols:
		स्थिती param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			थांबा;
		स्थिती param_size:
			display_window_size(0);
			थांबा;
		स्थिती param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			थांबा;
#endif
		स्थिती param_speed:
			display_speed(&mode, 0);
			थांबा;
		स्थिती param_ispeed:
			set_speed_or_die(आदान_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			थांबा;
		स्थिती param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			थांबा;
		पूर्वनिर्धारित:
			जर (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			अन्यथा /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	जर (stty_state & STTY_require_set_attr) {
		संरचना termios new_mode;

		जर (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		स_रखो(&new_mode, 0, आकार(new_mode));
		जर (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		जर (स_भेद(&mode, &new_mode, आकार(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#जर 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			जर ((stty_state & STTY_speed_was_set)
			 || स_भेद(&mode, &new_mode, आकार(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	परत निकास_सफल;
}
