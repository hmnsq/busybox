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

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((unsigned അക്ഷരം) 0)
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
#എങ്കിൽ defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#എങ്കിൽ defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#എങ്കിൽ defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#എങ്കിൽ defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#എങ്കിൽ defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#എങ്കിൽ defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#എങ്കിൽ defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#എങ്കിൽ !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#എങ്കിൽ defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#എങ്കിൽ defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#എങ്കിൽ defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#എങ്കിൽ defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#എങ്കിൽ defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#എങ്കിൽ defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#എങ്കിൽ defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#എങ്കിൽ defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#എങ്കിൽ defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#എങ്കിൽ defined(VSTATUS) && !defined(CSTATUS)
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
എണ്ണൽ speed_setting {
	നിവേശം_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
എണ്ണൽ {
	control, നിവേശം, output, local, combination
};
സ്ഥിതം tcflag_t *get_ptr_to_tcflag(unsigned type, സ്ഥിരം ഘടന termios *mode)
{
	സ്ഥിതം സ്ഥിരം uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(ഘടന termios, c_cflag), /* control */
		offsetof(ഘടന termios, c_iflag), /*നിവേശം*/
		offsetof(ഘടന termios, c_oflag), /* output */
		offsetof(ഘടന termios, c_lflag)  /* local */
	};
	എങ്കിൽ (type <= local) {
		തിരികെ (tcflag_t*) (((അക്ഷരം*)mode) + tcflag_offsets[type]);
	}
	തിരികെ ശൂന്യ;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
ഘടന mode_info {
	സ്ഥിരം uint8_t type;           /* Which structure element to change    */
	സ്ഥിരം uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#എങ്കിൽ   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	സ്ഥിരം uint8_t mask;
#അല്ലെങ്കിലെങ്കിൽ (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	സ്ഥിരം uint16_t mask;
#അല്ലെങ്കിൽ
	സ്ഥിരം tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	സ്ഥിരം tcflag_t bits;          /* Bits to set for this mode            */
};

എണ്ണൽ {
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
#എങ്കിൽ IXANY
	IDX_decctlq,
#endif
#എങ്കിൽ TABDLY || OXTABS
	IDX_tabs,
#endif
#എങ്കിൽ XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
സ്ഥിതം സ്ഥിരം അക്ഷരം mode_name[] ALIGN1 =
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
#എങ്കിൽ IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#എങ്കിൽ TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#എങ്കിൽ XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#എങ്കിൽ CMSPAR
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
#എങ്കിൽ CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   നിവേശം,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   നിവേശം,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   നിവേശം,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   നിവേശം,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    നിവേശം,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   നിവേശം,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    നിവേശം,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    നിവേശം,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    നിവേശം,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     നിവേശം,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    നിവേശം,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   നിവേശം,       OMIT       | REV,  IXOFF,      0 )
#എങ്കിൽ IUCLC
	MI_ENTRY("iuclc",    നിവേശം,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#എങ്കിൽ IXANY
	MI_ENTRY("ixany",    നിവേശം,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#എങ്കിൽ IMAXBEL
	MI_ENTRY("imaxbel",  നിവേശം,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#എങ്കിൽ IUTF8
	MI_ENTRY("iutf8",    നിവേശം,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#എങ്കിൽ OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#എങ്കിൽ OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#എങ്കിൽ ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#എങ്കിൽ ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#എങ്കിൽ ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#എങ്കിൽ OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#എങ്കിൽ OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#എങ്കിൽ NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#എങ്കിൽ CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#എങ്കിൽ TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# എങ്കിൽ TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# എങ്കിൽ TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#അല്ലെങ്കിൽ
# എങ്കിൽ OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#എങ്കിൽ BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#എങ്കിൽ VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#എങ്കിൽ FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#എങ്കിൽ IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#എങ്കിൽ XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#എങ്കിൽ TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#എങ്കിൽ ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#എങ്കിൽ ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#എങ്കിൽ ECHOKE
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

സ്ഥിതം സ്ഥിരം ഘടന mode_info mode_info[] ALIGN4 = {
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
#എങ്കിൽ IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#എങ്കിൽ TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#എങ്കിൽ XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#എങ്കിൽ CMSPAR
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
#എങ്കിൽ CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   നിവേശം,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   നിവേശം,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   നിവേശം,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   നിവേശം,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    നിവേശം,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   നിവേശം,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    നിവേശം,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    നിവേശം,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    നിവേശം,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     നിവേശം,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    നിവേശം,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   നിവേശം,       OMIT       | REV,  IXOFF,      0 )
#എങ്കിൽ IUCLC
	MI_ENTRY("iuclc",    നിവേശം,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#എങ്കിൽ IXANY
	MI_ENTRY("ixany",    നിവേശം,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#എങ്കിൽ IMAXBEL
	MI_ENTRY("imaxbel",  നിവേശം,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#എങ്കിൽ IUTF8
	MI_ENTRY("iutf8",    നിവേശം,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#എങ്കിൽ OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#എങ്കിൽ OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#എങ്കിൽ ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#എങ്കിൽ ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#എങ്കിൽ ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#എങ്കിൽ OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#എങ്കിൽ OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#എങ്കിൽ NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#എങ്കിൽ CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#എങ്കിൽ TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# എങ്കിൽ TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# എങ്കിൽ TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#അല്ലെങ്കിൽ
# എങ്കിൽ OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#എങ്കിൽ BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#എങ്കിൽ VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#എങ്കിൽ FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#എങ്കിൽ IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#എങ്കിൽ XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#എങ്കിൽ TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#എങ്കിൽ ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#എങ്കിൽ ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#എങ്കിൽ ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

എണ്ണൽ {
	അക്കം_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
ഘടന control_info {
	സ്ഥിരം uint8_t saneval;  /* Value to set for 'stty sane' */
	സ്ഥിരം uint8_t offset;   /* Offset in c_cc */
};

എണ്ണൽ {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#എങ്കിൽ VEOL2
	CIDX_eol2,
#endif
#എങ്കിൽ VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#എങ്കിൽ VDSUSP
	CIDX_dsusp,
#endif
#എങ്കിൽ VREPRINT
	CIDX_rprnt,
#endif
#എങ്കിൽ VWERASE
	CIDX_werase,
#endif
#എങ്കിൽ VLNEXT
	CIDX_lnext,
#endif
#എങ്കിൽ VFLUSHO
	CIDX_flush,
#endif
#എങ്കിൽ VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
സ്ഥിതം സ്ഥിരം അക്ഷരം control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#എങ്കിൽ VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#എങ്കിൽ VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#എങ്കിൽ VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#എങ്കിൽ VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#എങ്കിൽ VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#എങ്കിൽ VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#എങ്കിൽ VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#എങ്കിൽ VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

സ്ഥിതം സ്ഥിരം ഘടന control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#എങ്കിൽ VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#എങ്കിൽ VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#എങ്കിൽ VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#എങ്കിൽ VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#എങ്കിൽ VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#എങ്കിൽ VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#എങ്കിൽ VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#എങ്കിൽ VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

എണ്ണൽ {
	അക്കം_control_info = ARRAY_SIZE(control_info)
};


ഘടന globals {
	സ്ഥിരം അക്ഷരം *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(ഘടന globals*)bb_common_bufsiz1)
#define INIT_G() ചെയ്യുക { \
	setup_common_bufsiz(); \
	G.device_name = bb_സന്ദേശം_standard_നിവേശം; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} ആകുമ്പോൾ (0)

സ്ഥിതം ശൂന്യം set_speed_or_die(എണ്ണൽ speed_setting type, സ്ഥിരം അക്ഷരം *arg,
					ഘടന termios *mode)
{
	speed_t baud;

	baud = tty_മൂല്യം_to_baud(xatou(arg));

	എങ്കിൽ (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	എങ്കിൽ (type != നിവേശം_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

സ്ഥിതം NORETURN ശൂന്യം perror_on_device_and_die(സ്ഥിരം അക്ഷരം *fmt)
{
	bb_perror_സന്ദേശം_and_die(fmt, G.device_name);
}

സ്ഥിതം ശൂന്യം perror_on_device(സ്ഥിരം അക്ഷരം *fmt)
{
	bb_perror_സന്ദേശം(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
സ്ഥിതം ശൂന്യം wrapf(സ്ഥിരം അക്ഷരം *message, ...)
{
	അക്ഷരം buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, വലുപ്പം(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	എങ്കിൽ (!buflen || buflen >= വലുപ്പം(buf)) തിരികെ;

	എങ്കിൽ (G.current_col > 0) {
		G.current_col++;
		എങ്കിൽ (buf[0] != '\n') {
			എങ്കിൽ (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} അല്ലെങ്കിൽ {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	എങ്കിൽ (buf[buflen-1] == '\n')
		G.current_col = 0;
}

സ്ഥിതം ശൂന്യം newline(ശൂന്യം)
{
	എങ്കിൽ (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
സ്ഥിതം ശൂന്യം set_window_size(പൂർണ്ണസംഖ്യ rows, പൂർണ്ണസംഖ്യ cols)
{
	ഘടന winsize win = { 0, 0, 0, 0 };

	എങ്കിൽ (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		എങ്കിൽ (errno != EINVAL) {
			പോകുക bail;
		}
		memset(&win, 0, വലുപ്പം(win));
	}

	എങ്കിൽ (rows >= 0)
		win.ws_row = rows;
	എങ്കിൽ (cols >= 0)
		win.ws_col = cols;

	എങ്കിൽ (ioctl(STDIN_FILENO, TIOCSWINSZ, (അക്ഷരം *) &win))
bail:
		perror_on_device("%s");
}
#endif

സ്ഥിതം ശൂന്യം display_window_size(പൂർണ്ണസംഖ്യ fancy)
{
	സ്ഥിരം അക്ഷരം *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	എങ്കിൽ (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		എങ്കിൽ ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} അല്ലെങ്കിൽ {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

സ്ഥിതം സ്ഥിരം ഘടന suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

സ്ഥിതം സ്ഥിരം ഘടന mode_info *find_mode(സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ i = index_in_strings(mode_name, name);
	തിരികെ i >= 0 ? &mode_info[i] : ശൂന്യ;
}

സ്ഥിതം സ്ഥിരം ഘടന control_info *find_control(സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ i = index_in_strings(control_name, name);
	തിരികെ i >= 0 ? &control_info[i] : ശൂന്യ;
}

എണ്ണൽ {
	param_need_arg = 0x80,
	param_വരി    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

സ്ഥിതം പൂർണ്ണസംഖ്യ find_param(സ്ഥിരം അക്ഷരം *name)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	പൂർണ്ണസംഖ്യ i = index_in_strings(params, name) + 1;
	എങ്കിൽ (i == 0)
		തിരികെ 0;
	എങ്കിൽ (i != 5 && i != 6)
		i |= 0x80;
	തിരികെ i;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ recover_mode(സ്ഥിരം അക്ഷരം *arg, ഘടന termios *mode)
{
	പൂർണ്ണസംഖ്യ i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	എങ്കിൽ (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		തിരികെ 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	വേണ്ടി (i = 0; i < NCCS; ++i) {
		എങ്കിൽ (sscanf(arg, ":%x%n", &chr, &n) != 1)
			തിരികെ 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	എങ്കിൽ (*arg != '\0')
		തിരികെ 0;

	തിരികെ 1;
}

സ്ഥിതം ശൂന്യം display_recoverable(സ്ഥിരം ഘടന termios *mode,
				പൂർണ്ണസംഖ്യ UNUSED_PARAM dummy)
{
	പൂർണ്ണസംഖ്യ i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	വേണ്ടി (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned പൂർണ്ണസംഖ്യ) mode->c_cc[i]);
	bb_putchar('\n');
}

സ്ഥിതം ശൂന്യം display_speed(സ്ഥിരം ഘടന termios *mode, പൂർണ്ണസംഖ്യ fancy)
{
	//____________________ 01234567 8 9
	സ്ഥിരം അക്ഷരം *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	എങ്കിൽ (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	എങ്കിൽ (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_മൂല്യം(ispeed), tty_baud_to_മൂല്യം(ospeed));
}

സ്ഥിതം ശൂന്യം do_display(സ്ഥിരം ഘടന termios *mode, പൂർണ്ണസംഖ്യ all)
{
	പൂർണ്ണസംഖ്യ i;
	tcflag_t *bitsp;
	unsigned long mask;
	പൂർണ്ണസംഖ്യ prev_type = control;

	display_speed(mode, 1);
	എങ്കിൽ (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_വരി);
#അല്ലെങ്കിൽ
	newline();
#endif

	വേണ്ടി (i = 0; i != CIDX_min; ++i) {
		അക്ഷരം ch;
		അക്ഷരം buf10[10];

		/* If swtch is the same as susp, don't print both */
#എങ്കിൽ VSWTCH == VSUSP
		എങ്കിൽ (i == CIDX_swtch)
			തുടരുക;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#എങ്കിൽ VEOF == VMIN
		എങ്കിൽ (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			തുടരുക;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		എങ്കിൽ (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		അല്ലെങ്കിൽ
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#എങ്കിൽ VEOF == VMIN
	എങ്കിൽ ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	വേണ്ടി (i = 0; i < അക്കം_mode_info; ++i) {
		എങ്കിൽ (mode_info[i].flags & OMIT)
			തുടരുക;
		എങ്കിൽ (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		എങ്കിൽ ((*bitsp & mask) == mode_info[i].bits) {
			എങ്കിൽ (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} അല്ലെങ്കിൽ {
			എങ്കിൽ ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

സ്ഥിതം ശൂന്യം sane_mode(ഘടന termios *mode)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < അക്കം_control_info; ++i) {
#എങ്കിൽ VMIN == VEOF
		എങ്കിൽ (i == CIDX_min)
			നിർത്തുക;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	വേണ്ടി (i = 0; i < അക്കം_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		എങ്കിൽ (!bitsp)
			തുടരുക;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		എങ്കിൽ (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} അല്ലെങ്കിൽ
		എങ്കിൽ (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

സ്ഥിതം ശൂന്യം set_mode(സ്ഥിരം ഘടന mode_info *info, പൂർണ്ണസംഖ്യ reversed,
					ഘടന termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	എങ്കിൽ (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		എങ്കിൽ (reversed)
			*bitsp = val & ~info->bits;
		അല്ലെങ്കിൽ
			*bitsp = val | info->bits;
		തിരികെ;
	}

	/* !bitsp - it's a "combination" mode */
	എങ്കിൽ (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		എങ്കിൽ (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		അല്ലെങ്കിൽ
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_oddp]) {
		എങ്കിൽ (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		അല്ലെങ്കിൽ
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_nl]) {
		എങ്കിൽ (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} അല്ലെങ്കിൽ {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			എങ്കിൽ (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_cbreak]) {
		എങ്കിൽ (reversed)
			mode->c_lflag |= ICANON;
		അല്ലെങ്കിൽ
			mode->c_lflag &= ~ICANON;
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_pass8]) {
		എങ്കിൽ (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} അല്ലെങ്കിൽ {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_litout]) {
		എങ്കിൽ (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} അല്ലെങ്കിൽ {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		എങ്കിൽ ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#എങ്കിൽ VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#എങ്കിൽ VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} അല്ലെങ്കിൽ {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#എങ്കിൽ IXANY
	അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_decctlq]) {
		എങ്കിൽ (reversed)
			mode->c_iflag |= IXANY;
		അല്ലെങ്കിൽ
			mode->c_iflag &= ~IXANY;
	}
#endif
#എങ്കിൽ TABDLY
	അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_tabs]) {
		എങ്കിൽ (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		അല്ലെങ്കിൽ
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#എങ്കിൽ OXTABS
	അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_tabs]) {
		എങ്കിൽ (reversed)
			mode->c_oflag |= OXTABS;
		അല്ലെങ്കിൽ
			mode->c_oflag &= ~OXTABS;
	}
#endif
#എങ്കിൽ XCASE && IUCLC && OLCUC
	അല്ലെങ്കിൽ എങ്കിൽ (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		എങ്കിൽ (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} അല്ലെങ്കിൽ {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} അല്ലെങ്കിൽ എങ്കിൽ (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		എങ്കിൽ (IXANY) mode->c_iflag &= ~IXANY;
	}
}

സ്ഥിതം ശൂന്യം set_control_char_or_die(സ്ഥിരം ഘടന control_info *info,
			സ്ഥിരം അക്ഷരം *arg, ഘടന termios *mode)
{
	unsigned അക്ഷരം മൂല്യം_2;

	എങ്കിൽ (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		മൂല്യം_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	അല്ലെങ്കിൽ എങ്കിൽ (arg[0] == '\0' || arg[1] == '\0')
		മൂല്യം_2 = arg[0];
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		മൂല്യം_2 = _POSIX_VDISABLE;
	അല്ലെങ്കിൽ എങ്കിൽ (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		മൂല്യം_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		എങ്കിൽ (arg[1] == '?')
			മൂല്യം_2 = 127;
	} അല്ലെങ്കിൽ
		മൂല്യം_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = മൂല്യം_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

പൂർണ്ണസംഖ്യ stty_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ stty_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	ഘടന termios mode;
	ശൂന്യം (*output_func)(സ്ഥിരം ഘടന termios *, പൂർണ്ണസംഖ്യ);
	സ്ഥിരം അക്ഷരം *file_name = ശൂന്യ;
	പൂർണ്ണസംഖ്യ display_all = 0;
	പൂർണ്ണസംഖ്യ stty_state;
	പൂർണ്ണസംഖ്യ k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	ആകുമ്പോൾ (argv[++k]) {
		സ്ഥിരം ഘടന mode_info *mp;
		സ്ഥിരം ഘടന control_info *cp;
		സ്ഥിരം അക്ഷരം *arg = argv[k];
		സ്ഥിരം അക്ഷരം *argnext = argv[k+1];
		പൂർണ്ണസംഖ്യ param;

		എങ്കിൽ (arg[0] == '-') {
			പൂർണ്ണസംഖ്യ i;
			mp = find_mode(arg+1);
			എങ്കിൽ (mp) {
				എങ്കിൽ (!(mp->flags & REV))
					പോകുക invalid_argument;
				stty_state &= ~STTY_noargs;
				തുടരുക;
			}
			/* It is an option - parse it */
			i = 0;
			ആകുമ്പോൾ (arg[++i]) {
				തിരഞ്ഞെടുപ്പ് (arg[i]) {
				അവസ്ഥ 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					നിർത്തുക;
				അവസ്ഥ 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					നിർത്തുക;
				അവസ്ഥ 'F':
					എങ്കിൽ (file_name)
						bb_simple_error_സന്ദേശം_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					എങ്കിൽ (!file_name[0]) { /* nope, "-F device" */
						പൂർണ്ണസംഖ്യ p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						എങ്കിൽ (!file_name)
							bb_error_സന്ദേശം_and_die(bb_സന്ദേശം_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						ആകുമ്പോൾ (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					പോകുക end_option;
				സ്ഥിരസ്ഥിതി:
					പോകുക invalid_argument;
				}
			}
 end_option:
			തുടരുക;
		}

		mp = find_mode(arg);
		എങ്കിൽ (mp) {
			stty_state &= ~STTY_noargs;
			തുടരുക;
		}

		cp = find_control(arg);
		എങ്കിൽ (cp) {
			എങ്കിൽ (!argnext)
				bb_error_സന്ദേശം_and_die(bb_സന്ദേശം_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			തുടരുക;
		}

		param = find_param(arg);
		എങ്കിൽ (param & param_need_arg) {
			എങ്കിൽ (!argnext)
				bb_error_സന്ദേശം_and_die(bb_സന്ദേശം_requires_arg, arg);
			++k;
		}

		തിരഞ്ഞെടുപ്പ് (param) {
#ifdef __linux__
		അവസ്ഥ param_വരി:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			നിർത്തുക;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		അവസ്ഥ param_rows:
		അവസ്ഥ param_cols:
		അവസ്ഥ param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			നിർത്തുക;
		അവസ്ഥ param_size:
#endif
		അവസ്ഥ param_speed:
			നിർത്തുക;
		അവസ്ഥ param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(നിവേശം_speed, argnext, &mode);
			നിർത്തുക;
		അവസ്ഥ param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (recover_mode(arg, &mode) == 1) നിർത്തുക;
			എങ്കിൽ (tty_മൂല്യം_to_baud(xatou(arg)) != (speed_t) -1) നിർത്തുക;
 invalid_argument:
			bb_error_സന്ദേശം_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	എങ്കിൽ ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_സന്ദേശം_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	എങ്കിൽ ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_സന്ദേശം_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	എങ്കിൽ (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, വലുപ്പം(mode));
	എങ്കിൽ (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	എങ്കിൽ (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		തിരികെ EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	ആകുമ്പോൾ (argv[++k]) {
		സ്ഥിരം ഘടന mode_info *mp;
		സ്ഥിരം ഘടന control_info *cp;
		സ്ഥിരം അക്ഷരം *arg = argv[k];
		സ്ഥിരം അക്ഷരം *argnext = argv[k+1];
		പൂർണ്ണസംഖ്യ param;

		എങ്കിൽ (arg[0] == '-') {
			mp = find_mode(arg+1);
			എങ്കിൽ (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			തുടരുക;
		}

		mp = find_mode(arg);
		എങ്കിൽ (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			തുടരുക;
		}

		cp = find_control(arg);
		എങ്കിൽ (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			തുടരുക;
		}

		param = find_param(arg);
		എങ്കിൽ (param & param_need_arg) {
			++k;
		}

		തിരഞ്ഞെടുപ്പ് (param) {
#ifdef __linux__
		അവസ്ഥ param_വരി:
			mode.c_വരി = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			നിർത്തുക;
#endif
#ifdef TIOCGWINSZ
		അവസ്ഥ param_cols:
		അവസ്ഥ param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			നിർത്തുക;
		അവസ്ഥ param_size:
			display_window_size(0);
			നിർത്തുക;
		അവസ്ഥ param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			നിർത്തുക;
#endif
		അവസ്ഥ param_speed:
			display_speed(&mode, 0);
			നിർത്തുക;
		അവസ്ഥ param_ispeed:
			set_speed_or_die(നിവേശം_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			നിർത്തുക;
		അവസ്ഥ param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			അല്ലെങ്കിൽ /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	എങ്കിൽ (stty_state & STTY_require_set_attr) {
		ഘടന termios new_mode;

		എങ്കിൽ (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, വലുപ്പം(new_mode));
		എങ്കിൽ (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		എങ്കിൽ (memcmp(&mode, &new_mode, വലുപ്പം(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#എങ്കിൽ 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			എങ്കിൽ ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, വലുപ്പം(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	തിരികെ EXIT_SUCCESS;
}
