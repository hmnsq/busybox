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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((unsigned ଅକ୍ଷର) 0)
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
#ଯଦି defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#ଯଦି defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#ଯଦି defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#ଯଦି defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#ଯଦି defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#ଯଦି defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#ଯଦି defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#ଯଦି !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#ଯଦି defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#ଯଦି defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#ଯଦି defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#ଯଦି defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#ଯଦି defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#ଯଦି defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#ଯଦି defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#ଯଦି defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#ଯଦି defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#ଯଦି defined(VSTATUS) && !defined(CSTATUS)
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
ଗଣନା speed_setting {
	ନିବେଶ_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
ଗଣନା {
	control, ନିବେଶ, output, local, combination
};
ସ୍ଥିର tcflag_t *get_ptr_to_tcflag(unsigned type, ଅଚଳ ସଂରଚନା termios *mode)
{
	ସ୍ଥିର ଅଚଳ uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(ସଂରଚନା termios, c_cflag), /* control */
		offsetof(ସଂରଚନା termios, c_iflag), /*ନିବେଶ*/
		offsetof(ସଂରଚନା termios, c_oflag), /* output */
		offsetof(ସଂରଚନା termios, c_lflag)  /* local */
	};
	ଯଦି (type <= local) {
		ଫେରନ୍ତୁ (tcflag_t*) (((ଅକ୍ଷର*)mode) + tcflag_offsets[type]);
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
ସଂରଚନା mode_info {
	ଅଚଳ uint8_t type;           /* Which structure element to change    */
	ଅଚଳ uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#ଯଦି   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ଅଚଳ uint8_t mask;
#ନଚେତଯଦି (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ଅଚଳ uint16_t mask;
#ଅନ୍ୟଥା
	ଅଚଳ tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	ଅଚଳ tcflag_t bits;          /* Bits to set for this mode            */
};

ଗଣନା {
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
#ଯଦି IXANY
	IDX_decctlq,
#endif
#ଯଦି TABDLY || OXTABS
	IDX_tabs,
#endif
#ଯଦି XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର mode_name[] ALIGN1 =
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
#ଯଦି IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#ଯଦି TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#ଯଦି XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#ଯଦି CMSPAR
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
#ଯଦି CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   ନିବେଶ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   ନିବେଶ,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   ନିବେଶ,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   ନିବେଶ,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    ନିବେଶ,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   ନିବେଶ,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    ନିବେଶ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    ନିବେଶ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    ନିବେଶ,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     ନିବେଶ,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    ନିବେଶ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   ନିବେଶ,       OMIT       | REV,  IXOFF,      0 )
#ଯଦି IUCLC
	MI_ENTRY("iuclc",    ନିବେଶ,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#ଯଦି IXANY
	MI_ENTRY("ixany",    ନିବେଶ,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#ଯଦି IMAXBEL
	MI_ENTRY("imaxbel",  ନିବେଶ,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#ଯଦି IUTF8
	MI_ENTRY("iutf8",    ନିବେଶ,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#ଯଦି OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#ଯଦି OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#ଯଦି ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#ଯଦି ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#ଯଦି ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#ଯଦି OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#ଯଦି OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#ଯଦି NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#ଯଦି CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#ଯଦି TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ଯଦି TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ଯଦି TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#ଅନ୍ୟଥା
# ଯଦି OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#ଯଦି BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#ଯଦି VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#ଯଦି FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#ଯଦି IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ଯଦି XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#ଯଦି TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#ଯଦି ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#ଯଦି ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#ଯଦି ECHOKE
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

ସ୍ଥିର ଅଚଳ ସଂରଚନା mode_info mode_info[] ALIGN4 = {
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
#ଯଦି IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#ଯଦି TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#ଯଦି XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#ଯଦି CMSPAR
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
#ଯଦି CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   ନିବେଶ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   ନିବେଶ,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   ନିବେଶ,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   ନିବେଶ,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    ନିବେଶ,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   ନିବେଶ,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    ନିବେଶ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    ନିବେଶ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    ନିବେଶ,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     ନିବେଶ,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    ନିବେଶ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   ନିବେଶ,       OMIT       | REV,  IXOFF,      0 )
#ଯଦି IUCLC
	MI_ENTRY("iuclc",    ନିବେଶ,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#ଯଦି IXANY
	MI_ENTRY("ixany",    ନିବେଶ,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#ଯଦି IMAXBEL
	MI_ENTRY("imaxbel",  ନିବେଶ,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#ଯଦି IUTF8
	MI_ENTRY("iutf8",    ନିବେଶ,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#ଯଦି OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#ଯଦି OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#ଯଦି ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#ଯଦି ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#ଯଦି ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#ଯଦି OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#ଯଦି OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#ଯଦି NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#ଯଦି CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#ଯଦି TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ଯଦି TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ଯଦି TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#ଅନ୍ୟଥା
# ଯଦି OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#ଯଦି BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#ଯଦି VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#ଯଦି FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#ଯଦି IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ଯଦି XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#ଯଦି TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#ଯଦି ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#ଯଦି ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#ଯଦି ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

ଗଣନା {
	ଅଙ୍କ_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
ସଂରଚନା control_info {
	ଅଚଳ uint8_t saneval;  /* Value to set for 'stty sane' */
	ଅଚଳ uint8_t offset;   /* Offset in c_cc */
};

ଗଣନା {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#ଯଦି VEOL2
	CIDX_eol2,
#endif
#ଯଦି VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#ଯଦି VDSUSP
	CIDX_dsusp,
#endif
#ଯଦି VREPRINT
	CIDX_rprnt,
#endif
#ଯଦି VWERASE
	CIDX_werase,
#endif
#ଯଦି VLNEXT
	CIDX_lnext,
#endif
#ଯଦି VFLUSHO
	CIDX_flush,
#endif
#ଯଦି VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#ଯଦି VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#ଯଦି VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#ଯଦି VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#ଯଦି VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#ଯଦି VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#ଯଦି VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#ଯଦି VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#ଯଦି VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

ସ୍ଥିର ଅଚଳ ସଂରଚନା control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#ଯଦି VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#ଯଦି VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#ଯଦି VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#ଯଦି VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#ଯଦି VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#ଯଦି VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#ଯଦି VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#ଯଦି VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

ଗଣନା {
	ଅଙ୍କ_control_info = ARRAY_SIZE(control_info)
};


ସଂରଚନା globals {
	ଅଚଳ ଅକ୍ଷର *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(ସଂରଚନା globals*)bb_common_bufsiz1)
#define INIT_G() କର { \
	setup_common_bufsiz(); \
	G.device_name = bb_ସନ୍ଦେଶ_standard_ନିବେଶ; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} ଯେତେବେଳେ (0)

ସ୍ଥିର ଶୂନ୍ୟ set_speed_or_die(ଗଣନା speed_setting type, ଅଚଳ ଅକ୍ଷର *arg,
					ସଂରଚନା termios *mode)
{
	speed_t baud;

	baud = tty_ମୂଲ୍ୟ_to_baud(xatou(arg));

	ଯଦି (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	ଯଦି (type != ନିବେଶ_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

ସ୍ଥିର NORETURN ଶୂନ୍ୟ perror_on_device_and_die(ଅଚଳ ଅକ୍ଷର *fmt)
{
	bb_perror_ସନ୍ଦେଶ_and_die(fmt, G.device_name);
}

ସ୍ଥିର ଶୂନ୍ୟ perror_on_device(ଅଚଳ ଅକ୍ଷର *fmt)
{
	bb_perror_ସନ୍ଦେଶ(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ସ୍ଥିର ଶୂନ୍ୟ wrapf(ଅଚଳ ଅକ୍ଷର *message, ...)
{
	ଅକ୍ଷର buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, ଆକାର(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	ଯଦି (!buflen || buflen >= ଆକାର(buf)) ଫେରନ୍ତୁ;

	ଯଦି (G.current_col > 0) {
		G.current_col++;
		ଯଦି (buf[0] != '\n') {
			ଯଦି (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} ଅନ୍ୟଥା {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	ଯଦି (buf[buflen-1] == '\n')
		G.current_col = 0;
}

ସ୍ଥିର ଶୂନ୍ୟ newline(ଶୂନ୍ୟ)
{
	ଯଦି (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
ସ୍ଥିର ଶୂନ୍ୟ set_window_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cols)
{
	ସଂରଚନା winsize win = { 0, 0, 0, 0 };

	ଯଦି (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		ଯଦି (errno != EINVAL) {
			ଯାଅ bail;
		}
		memset(&win, 0, ଆକାର(win));
	}

	ଯଦି (rows >= 0)
		win.ws_row = rows;
	ଯଦି (cols >= 0)
		win.ws_col = cols;

	ଯଦି (ioctl(STDIN_FILENO, TIOCSWINSZ, (ଅକ୍ଷର *) &win))
bail:
		perror_on_device("%s");
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ display_window_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fancy)
{
	ଅଚଳ ଅକ୍ଷର *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	ଯଦି (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		ଯଦି ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} ଅନ୍ୟଥା {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା mode_info *find_mode(ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = index_in_strings(mode_name, name);
	ଫେରନ୍ତୁ i >= 0 ? &mode_info[i] : ଖାଲି;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା control_info *find_control(ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = index_in_strings(control_name, name);
	ଫେରନ୍ତୁ i >= 0 ? &control_info[i] : ଖାଲି;
}

ଗଣନା {
	param_need_arg = 0x80,
	param_ଧାଡ଼ି    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା find_param(ଅଚଳ ଅକ୍ଷର *name)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = index_in_strings(params, name) + 1;
	ଯଦି (i == 0)
		ଫେରନ୍ତୁ 0;
	ଯଦି (i != 5 && i != 6)
		i |= 0x80;
	ଫେରନ୍ତୁ i;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା recover_mode(ଅଚଳ ଅକ୍ଷର *arg, ସଂରଚନା termios *mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	ଯଦି (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		ଫେରନ୍ତୁ 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	ପାଇଁ (i = 0; i < NCCS; ++i) {
		ଯଦି (sscanf(arg, ":%x%n", &chr, &n) != 1)
			ଫେରନ୍ତୁ 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	ଯଦି (*arg != '\0')
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ଶୂନ୍ୟ display_recoverable(ଅଚଳ ସଂରଚନା termios *mode,
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା UNUSED_PARAM dummy)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	ପାଇଁ (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) mode->c_cc[i]);
	bb_putchar('\n');
}

ସ୍ଥିର ଶୂନ୍ୟ display_speed(ଅଚଳ ସଂରଚନା termios *mode, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fancy)
{
	//____________________ 01234567 8 9
	ଅଚଳ ଅକ୍ଷର *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	ଯଦି (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	ଯଦି (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_ମୂଲ୍ୟ(ispeed), tty_baud_to_ମୂଲ୍ୟ(ospeed));
}

ସ୍ଥିର ଶୂନ୍ୟ do_display(ଅଚଳ ସଂରଚନା termios *mode, ପୂର୍ଣ୍ଣସଂଖ୍ୟା all)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	tcflag_t *bitsp;
	unsigned long mask;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev_type = control;

	display_speed(mode, 1);
	ଯଦି (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_ଧାଡ଼ି);
#ଅନ୍ୟଥା
	newline();
#endif

	ପାଇଁ (i = 0; i != CIDX_min; ++i) {
		ଅକ୍ଷର ch;
		ଅକ୍ଷର buf10[10];

		/* If swtch is the same as susp, don't print both */
#ଯଦି VSWTCH == VSUSP
		ଯଦି (i == CIDX_swtch)
			ଜାରି;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#ଯଦି VEOF == VMIN
		ଯଦି (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			ଜାରି;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		ଯଦି (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		ଅନ୍ୟଥା
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#ଯଦି VEOF == VMIN
	ଯଦି ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	ପାଇଁ (i = 0; i < ଅଙ୍କ_mode_info; ++i) {
		ଯଦି (mode_info[i].flags & OMIT)
			ଜାରି;
		ଯଦି (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		ଯଦି ((*bitsp & mask) == mode_info[i].bits) {
			ଯଦି (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} ଅନ୍ୟଥା {
			ଯଦି ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

ସ୍ଥିର ଶୂନ୍ୟ sane_mode(ସଂରଚନା termios *mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ଅଙ୍କ_control_info; ++i) {
#ଯଦି VMIN == VEOF
		ଯଦି (i == CIDX_min)
			ଭାଙ୍ଗ;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	ପାଇଁ (i = 0; i < ଅଙ୍କ_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		ଯଦି (!bitsp)
			ଜାରି;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		ଯଦି (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} ଅନ୍ୟଥା
		ଯଦି (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

ସ୍ଥିର ଶୂନ୍ୟ set_mode(ଅଚଳ ସଂରଚନା mode_info *info, ପୂର୍ଣ୍ଣସଂଖ୍ୟା reversed,
					ସଂରଚନା termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	ଯଦି (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		ଯଦି (reversed)
			*bitsp = val & ~info->bits;
		ଅନ୍ୟଥା
			*bitsp = val | info->bits;
		ଫେରନ୍ତୁ;
	}

	/* !bitsp - it's a "combination" mode */
	ଯଦି (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		ଯଦି (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ଅନ୍ୟଥା
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_oddp]) {
		ଯଦି (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ଅନ୍ୟଥା
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_nl]) {
		ଯଦି (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} ଅନ୍ୟଥା {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			ଯଦି (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_cbreak]) {
		ଯଦି (reversed)
			mode->c_lflag |= ICANON;
		ଅନ୍ୟଥା
			mode->c_lflag &= ~ICANON;
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_pass8]) {
		ଯଦି (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} ଅନ୍ୟଥା {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_litout]) {
		ଯଦି (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} ଅନ୍ୟଥା {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		ଯଦି ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#ଯଦି VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#ଯଦି VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} ଅନ୍ୟଥା {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#ଯଦି IXANY
	ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_decctlq]) {
		ଯଦି (reversed)
			mode->c_iflag |= IXANY;
		ଅନ୍ୟଥା
			mode->c_iflag &= ~IXANY;
	}
#endif
#ଯଦି TABDLY
	ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_tabs]) {
		ଯଦି (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		ଅନ୍ୟଥା
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#ଯଦି OXTABS
	ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_tabs]) {
		ଯଦି (reversed)
			mode->c_oflag |= OXTABS;
		ଅନ୍ୟଥା
			mode->c_oflag &= ~OXTABS;
	}
#endif
#ଯଦି XCASE && IUCLC && OLCUC
	ଅନ୍ୟଥା ଯଦି (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		ଯଦି (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} ଅନ୍ୟଥା {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} ଅନ୍ୟଥା ଯଦି (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		ଯଦି (IXANY) mode->c_iflag &= ~IXANY;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ set_control_char_or_die(ଅଚଳ ସଂରଚନା control_info *info,
			ଅଚଳ ଅକ୍ଷର *arg, ସଂରଚନା termios *mode)
{
	unsigned ଅକ୍ଷର ମୂଲ୍ୟ_2;

	ଯଦି (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		ମୂଲ୍ୟ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ଅନ୍ୟଥା ଯଦି (arg[0] == '\0' || arg[1] == '\0')
		ମୂଲ୍ୟ_2 = arg[0];
	ଅନ୍ୟଥା ଯଦି (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		ମୂଲ୍ୟ_2 = _POSIX_VDISABLE;
	ଅନ୍ୟଥା ଯଦି (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		ମୂଲ୍ୟ_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		ଯଦି (arg[1] == '?')
			ମୂଲ୍ୟ_2 = 127;
	} ଅନ୍ୟଥା
		ମୂଲ୍ୟ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = ମୂଲ୍ୟ_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stty_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା stty_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ସଂରଚନା termios mode;
	ଶୂନ୍ୟ (*output_func)(ଅଚଳ ସଂରଚନା termios *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	ଅଚଳ ଅକ୍ଷର *file_name = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା display_all = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stty_state;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	ଯେତେବେଳେ (argv[++k]) {
		ଅଚଳ ସଂରଚନା mode_info *mp;
		ଅଚଳ ସଂରଚନା control_info *cp;
		ଅଚଳ ଅକ୍ଷର *arg = argv[k];
		ଅଚଳ ଅକ୍ଷର *argnext = argv[k+1];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା param;

		ଯଦି (arg[0] == '-') {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
			mp = find_mode(arg+1);
			ଯଦି (mp) {
				ଯଦି (!(mp->flags & REV))
					ଯାଅ invalid_argument;
				stty_state &= ~STTY_noargs;
				ଜାରି;
			}
			/* It is an option - parse it */
			i = 0;
			ଯେତେବେଳେ (arg[++i]) {
				ବାଛ (arg[i]) {
				ସ୍ଥିତି 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					ଭାଙ୍ଗ;
				ସ୍ଥିତି 'F':
					ଯଦି (file_name)
						bb_simple_error_ସନ୍ଦେଶ_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					ଯଦି (!file_name[0]) { /* nope, "-F device" */
						ପୂର୍ଣ୍ଣସଂଖ୍ୟା p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						ଯଦି (!file_name)
							bb_error_ସନ୍ଦେଶ_and_die(bb_ସନ୍ଦେଶ_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						ଯେତେବେଳେ (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					ଯାଅ end_option;
				ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
					ଯାଅ invalid_argument;
				}
			}
 end_option:
			ଜାରି;
		}

		mp = find_mode(arg);
		ଯଦି (mp) {
			stty_state &= ~STTY_noargs;
			ଜାରି;
		}

		cp = find_control(arg);
		ଯଦି (cp) {
			ଯଦି (!argnext)
				bb_error_ସନ୍ଦେଶ_and_die(bb_ସନ୍ଦେଶ_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			ଜାରି;
		}

		param = find_param(arg);
		ଯଦି (param & param_need_arg) {
			ଯଦି (!argnext)
				bb_error_ସନ୍ଦେଶ_and_die(bb_ସନ୍ଦେଶ_requires_arg, arg);
			++k;
		}

		ବାଛ (param) {
#ifdef __linux__
		ସ୍ଥିତି param_ଧାଡ଼ି:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			ଭାଙ୍ଗ;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		ସ୍ଥିତି param_rows:
		ସ୍ଥିତି param_cols:
		ସ୍ଥିତି param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_size:
#endif
		ସ୍ଥିତି param_speed:
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(ନିବେଶ_speed, argnext, &mode);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (recover_mode(arg, &mode) == 1) ଭାଙ୍ଗ;
			ଯଦି (tty_ମୂଲ୍ୟ_to_baud(xatou(arg)) != (speed_t) -1) ଭାଙ୍ଗ;
 invalid_argument:
			bb_error_ସନ୍ଦେଶ_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	ଯଦି ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_ସନ୍ଦେଶ_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	ଯଦି ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_ସନ୍ଦେଶ_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	ଯଦି (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, ଆକାର(mode));
	ଯଦି (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	ଯଦି (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	ଯେତେବେଳେ (argv[++k]) {
		ଅଚଳ ସଂରଚନା mode_info *mp;
		ଅଚଳ ସଂରଚନା control_info *cp;
		ଅଚଳ ଅକ୍ଷର *arg = argv[k];
		ଅଚଳ ଅକ୍ଷର *argnext = argv[k+1];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା param;

		ଯଦି (arg[0] == '-') {
			mp = find_mode(arg+1);
			ଯଦି (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			ଜାରି;
		}

		mp = find_mode(arg);
		ଯଦି (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			ଜାରି;
		}

		cp = find_control(arg);
		ଯଦି (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			ଜାରି;
		}

		param = find_param(arg);
		ଯଦି (param & param_need_arg) {
			++k;
		}

		ବାଛ (param) {
#ifdef __linux__
		ସ୍ଥିତି param_ଧାଡ଼ି:
			mode.c_ଧାଡ଼ି = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			ଭାଙ୍ଗ;
#endif
#ifdef TIOCGWINSZ
		ସ୍ଥିତି param_cols:
		ସ୍ଥିତି param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_size:
			display_window_size(0);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			ଭାଙ୍ଗ;
#endif
		ସ୍ଥିତି param_speed:
			display_speed(&mode, 0);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_ispeed:
			set_speed_or_die(ନିବେଶ_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			ଅନ୍ୟଥା /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	ଯଦି (stty_state & STTY_require_set_attr) {
		ସଂରଚନା termios new_mode;

		ଯଦି (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, ଆକାର(new_mode));
		ଯଦି (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		ଯଦି (memcmp(&mode, &new_mode, ଆକାର(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#ଯଦି 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			ଯଦି ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, ଆକାର(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
