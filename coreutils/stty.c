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

#include "libbb.h"
#include "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((unsigned var_na) 0)
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
#ถ้า defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#ถ้า defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#ถ้า defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#ถ้า defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#ถ้า defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#ถ้า defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#ถ้า defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#ถ้า !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#ถ้า defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#ถ้า defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#ถ้า defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#ถ้า defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#ถ้า defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#ถ้า defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#ถ้า defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#ถ้า defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#ถ้า defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#ถ้า defined(VSTATUS) && !defined(CSTATUS)
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
ga_nanaa speed_setting {
	input_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
ga_nanaa {
	control, input, output, local, combination
};
static tcflag_t *get_ptr_to_tcflag(unsigned type, const sa_nracanaa termios *mode)
{
	static const uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(sa_nracanaa termios, c_cflag), /* control */
		offsetof(sa_nracanaa termios, c_iflag), /* input */
		offsetof(sa_nracanaa termios, c_oflag), /* output */
		offsetof(sa_nracanaa termios, c_lflag)  /* local */
	};
	ถ้า (type <= local) {
		คืน (tcflag_t*) (((var_na*)mode) + tcflag_offsets[type]);
	}
	คืน NULL;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
sa_nracanaa mode_info {
	const uint8_t type;           /* Which structure element to change    */
	const uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#ถ้า   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	const uint8_t mask;
#elif (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	const uint16_t mask;
#มิฉะนั้น
	const tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	const tcflag_t bits;          /* Bits to set for this mode            */
};

ga_nanaa {
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
#ถ้า IXANY
	IDX_decctlq,
#endif
#ถ้า TABDLY || OXTABS
	IDX_tabs,
#endif
#ถ้า XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
static const var_na mode_name[] ALIGN1 =
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
#ถ้า IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#ถ้า TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#ถ้า XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#ถ้า CMSPAR
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
#ถ้า CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   input,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   input,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   input,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   input,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    input,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   input,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    input,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    input,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    input,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     input,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    input,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   input,       OMIT       | REV,  IXOFF,      0 )
#ถ้า IUCLC
	MI_ENTRY("iuclc",    input,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#ถ้า IXANY
	MI_ENTRY("ixany",    input,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#ถ้า IMAXBEL
	MI_ENTRY("imaxbel",  input,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#ถ้า IUTF8
	MI_ENTRY("iutf8",    input,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#ถ้า OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#ถ้า OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#ถ้า ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#ถ้า ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#ถ้า ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#ถ้า OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#ถ้า OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#ถ้า NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#ถ้า CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#ถ้า TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ถ้า TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ถ้า TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#มิฉะนั้น
# ถ้า OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#ถ้า BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#ถ้า VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#ถ้า FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#ถ้า IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ถ้า XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#ถ้า TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#ถ้า ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#ถ้า ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#ถ้า ECHOKE
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

static const sa_nracanaa mode_info mode_info[] ALIGN4 = {
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
#ถ้า IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#ถ้า TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#ถ้า XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#ถ้า CMSPAR
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
#ถ้า CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   input,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   input,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   input,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   input,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    input,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   input,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    input,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    input,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    input,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     input,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    input,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   input,       OMIT       | REV,  IXOFF,      0 )
#ถ้า IUCLC
	MI_ENTRY("iuclc",    input,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#ถ้า IXANY
	MI_ENTRY("ixany",    input,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#ถ้า IMAXBEL
	MI_ENTRY("imaxbel",  input,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#ถ้า IUTF8
	MI_ENTRY("iutf8",    input,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#ถ้า OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#ถ้า OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#ถ้า ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#ถ้า ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#ถ้า ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#ถ้า OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#ถ้า OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#ถ้า NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#ถ้า CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#ถ้า TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ถ้า TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ถ้า TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#มิฉะนั้น
# ถ้า OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#ถ้า BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#ถ้า VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#ถ้า FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#ถ้า IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ถ้า XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#ถ้า TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#ถ้า ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#ถ้า ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#ถ้า ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

ga_nanaa {
	NUM_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
sa_nracanaa control_info {
	const uint8_t saneval;  /* Value to set for 'stty sane' */
	const uint8_t offset;   /* Offset in c_cc */
};

ga_nanaa {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#ถ้า VEOL2
	CIDX_eol2,
#endif
#ถ้า VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#ถ้า VDSUSP
	CIDX_dsusp,
#endif
#ถ้า VREPRINT
	CIDX_rprnt,
#endif
#ถ้า VWERASE
	CIDX_werase,
#endif
#ถ้า VLNEXT
	CIDX_lnext,
#endif
#ถ้า VFLUSHO
	CIDX_flush,
#endif
#ถ้า VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
static const var_na control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#ถ้า VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#ถ้า VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#ถ้า VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#ถ้า VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#ถ้า VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#ถ้า VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#ถ้า VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#ถ้า VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

static const sa_nracanaa control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#ถ้า VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#ถ้า VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#ถ้า VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#ถ้า VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#ถ้า VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#ถ้า VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#ถ้า VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#ถ้า VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

ga_nanaa {
	NUM_control_info = ARRAY_SIZE(control_info)
};


sa_nracanaa globals {
	const var_na *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(sa_nracanaa globals*)bb_common_bufsiz1)
#define INIT_G() do { \
	setup_common_bufsiz(); \
	G.device_name = bb_msg_standard_input; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} ขณะ (0)

static void set_speed_or_die(ga_nanaa speed_setting type, const var_na *arg,
					sa_nracanaa termios *mode)
{
	speed_t baud;

	baud = tty_value_to_baud(xatou(arg));

	ถ้า (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	ถ้า (type != input_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

static NORETURN void perror_on_device_and_die(const var_na *fmt)
{
	bb_perror_msg_and_die(fmt, G.device_name);
}

static void perror_on_device(const var_na *fmt)
{
	bb_perror_msg(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
static void wrapf(const var_na *message, ...)
{
	var_na buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, parimaa_na(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	ถ้า (!buflen || buflen >= parimaa_na(buf)) คืน;

	ถ้า (G.current_col > 0) {
		G.current_col++;
		ถ้า (buf[0] != '\n') {
			ถ้า (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} มิฉะนั้น {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	ถ้า (buf[buflen-1] == '\n')
		G.current_col = 0;
}

static void newline(void)
{
	ถ้า (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
static void set_window_size(จำนวนเต็ม rows, จำนวนเต็ม cols)
{
	sa_nracanaa winsize win = { 0, 0, 0, 0 };

	ถ้า (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		ถ้า (errno != EINVAL) {
			goto bail;
		}
		memset(&win, 0, parimaa_na(win));
	}

	ถ้า (rows >= 0)
		win.ws_row = rows;
	ถ้า (cols >= 0)
		win.ws_col = cols;

	ถ้า (ioctl(STDIN_FILENO, TIOCSWINSZ, (var_na *) &win))
bail:
		perror_on_device("%s");
}
#endif

static void display_window_size(จำนวนเต็ม fancy)
{
	const var_na *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	ถ้า (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		ถ้า ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} มิฉะนั้น {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

static const sa_nracanaa suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

static const sa_nracanaa mode_info *find_mode(const var_na *name)
{
	จำนวนเต็ม i = index_in_strings(mode_name, name);
	คืน i >= 0 ? &mode_info[i] : NULL;
}

static const sa_nracanaa control_info *find_control(const var_na *name)
{
	จำนวนเต็ม i = index_in_strings(control_name, name);
	คืน i >= 0 ? &control_info[i] : NULL;
}

ga_nanaa {
	param_need_arg = 0x80,
	param_line    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

static จำนวนเต็ม find_param(const var_na *name)
{
	static const var_na params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	จำนวนเต็ม i = index_in_strings(params, name) + 1;
	ถ้า (i == 0)
		คืน 0;
	ถ้า (i != 5 && i != 6)
		i |= 0x80;
	คืน i;
}

static จำนวนเต็ม recover_mode(const var_na *arg, sa_nracanaa termios *mode)
{
	จำนวนเต็ม i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	ถ้า (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		คืน 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	สำหรับ (i = 0; i < NCCS; ++i) {
		ถ้า (sscanf(arg, ":%x%n", &chr, &n) != 1)
			คืน 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	ถ้า (*arg != '\0')
		คืน 0;

	คืน 1;
}

static void display_recoverable(const sa_nracanaa termios *mode,
				จำนวนเต็ม UNUSED_PARAM dummy)
{
	จำนวนเต็ม i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	สำหรับ (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned จำนวนเต็ม) mode->c_cc[i]);
	bb_putchar('\n');
}

static void display_speed(const sa_nracanaa termios *mode, จำนวนเต็ม fancy)
{
	//____________________ 01234567 8 9
	const var_na *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	ถ้า (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	ถ้า (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_value(ispeed), tty_baud_to_value(ospeed));
}

static void do_display(const sa_nracanaa termios *mode, จำนวนเต็ม all)
{
	จำนวนเต็ม i;
	tcflag_t *bitsp;
	unsigned long mask;
	จำนวนเต็ม prev_type = control;

	display_speed(mode, 1);
	ถ้า (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_line);
#มิฉะนั้น
	newline();
#endif

	สำหรับ (i = 0; i != CIDX_min; ++i) {
		var_na ch;
		var_na buf10[10];

		/* If swtch is the same as susp, don't print both */
#ถ้า VSWTCH == VSUSP
		ถ้า (i == CIDX_swtch)
			continue;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#ถ้า VEOF == VMIN
		ถ้า (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			continue;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		ถ้า (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		มิฉะนั้น
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#ถ้า VEOF == VMIN
	ถ้า ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	สำหรับ (i = 0; i < NUM_mode_info; ++i) {
		ถ้า (mode_info[i].flags & OMIT)
			continue;
		ถ้า (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		ถ้า ((*bitsp & mask) == mode_info[i].bits) {
			ถ้า (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} มิฉะนั้น {
			ถ้า ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

static void sane_mode(sa_nracanaa termios *mode)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < NUM_control_info; ++i) {
#ถ้า VMIN == VEOF
		ถ้า (i == CIDX_min)
			bha_nga;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	สำหรับ (i = 0; i < NUM_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		ถ้า (!bitsp)
			continue;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		ถ้า (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} มิฉะนั้น
		ถ้า (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

static void set_mode(const sa_nracanaa mode_info *info, จำนวนเต็ม reversed,
					sa_nracanaa termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	ถ้า (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		ถ้า (reversed)
			*bitsp = val & ~info->bits;
		มิฉะนั้น
			*bitsp = val | info->bits;
		คืน;
	}

	/* !bitsp - it's a "combination" mode */
	ถ้า (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		ถ้า (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		มิฉะนั้น
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_oddp]) {
		ถ้า (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		มิฉะนั้น
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_nl]) {
		ถ้า (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} มิฉะนั้น {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			ถ้า (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_cbreak]) {
		ถ้า (reversed)
			mode->c_lflag |= ICANON;
		มิฉะนั้น
			mode->c_lflag &= ~ICANON;
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_pass8]) {
		ถ้า (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} มิฉะนั้น {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_litout]) {
		ถ้า (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} มิฉะนั้น {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		ถ้า ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#ถ้า VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#ถ้า VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} มิฉะนั้น {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#ถ้า IXANY
	มิฉะนั้น ถ้า (info == &mode_info[IDX_decctlq]) {
		ถ้า (reversed)
			mode->c_iflag |= IXANY;
		มิฉะนั้น
			mode->c_iflag &= ~IXANY;
	}
#endif
#ถ้า TABDLY
	มิฉะนั้น ถ้า (info == &mode_info[IDX_tabs]) {
		ถ้า (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		มิฉะนั้น
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#ถ้า OXTABS
	มิฉะนั้น ถ้า (info == &mode_info[IDX_tabs]) {
		ถ้า (reversed)
			mode->c_oflag |= OXTABS;
		มิฉะนั้น
			mode->c_oflag &= ~OXTABS;
	}
#endif
#ถ้า XCASE && IUCLC && OLCUC
	มิฉะนั้น ถ้า (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		ถ้า (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} มิฉะนั้น {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	มิฉะนั้น ถ้า (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} มิฉะนั้น ถ้า (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		ถ้า (IXANY) mode->c_iflag &= ~IXANY;
	}
}

static void set_control_char_or_die(const sa_nracanaa control_info *info,
			const var_na *arg, sa_nracanaa termios *mode)
{
	unsigned var_na value;

	ถ้า (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		value = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	มิฉะนั้น ถ้า (arg[0] == '\0' || arg[1] == '\0')
		value = arg[0];
	มิฉะนั้น ถ้า (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		value = _POSIX_VDISABLE;
	มิฉะนั้น ถ้า (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		value = arg[1] & 0x1f; /* Non-letters get weird results */
		ถ้า (arg[1] == '?')
			value = 127;
	} มิฉะนั้น
		value = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = value;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

จำนวนเต็ม stty_main(จำนวนเต็ม argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
จำนวนเต็ม stty_main(จำนวนเต็ม argc UNUSED_PARAM, var_na **argv)
{
	sa_nracanaa termios mode;
	void (*output_func)(const sa_nracanaa termios *, จำนวนเต็ม);
	const var_na *file_name = NULL;
	จำนวนเต็ม display_all = 0;
	จำนวนเต็ม stty_state;
	จำนวนเต็ม k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	ขณะ (argv[++k]) {
		const sa_nracanaa mode_info *mp;
		const sa_nracanaa control_info *cp;
		const var_na *arg = argv[k];
		const var_na *argnext = argv[k+1];
		จำนวนเต็ม param;

		ถ้า (arg[0] == '-') {
			จำนวนเต็ม i;
			mp = find_mode(arg+1);
			ถ้า (mp) {
				ถ้า (!(mp->flags & REV))
					goto invalid_argument;
				stty_state &= ~STTY_noargs;
				continue;
			}
			/* It is an option - parse it */
			i = 0;
			ขณะ (arg[++i]) {
				switch (arg[i]) {
				pak_sa 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					bha_nga;
				pak_sa 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					bha_nga;
				pak_sa 'F':
					ถ้า (file_name)
						bb_simple_error_msg_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					ถ้า (!file_name[0]) { /* nope, "-F device" */
						จำนวนเต็ม p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						ถ้า (!file_name)
							bb_error_msg_and_die(bb_msg_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						ขณะ (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					goto end_option;
				default:
					goto invalid_argument;
				}
			}
 end_option:
			continue;
		}

		mp = find_mode(arg);
		ถ้า (mp) {
			stty_state &= ~STTY_noargs;
			continue;
		}

		cp = find_control(arg);
		ถ้า (cp) {
			ถ้า (!argnext)
				bb_error_msg_and_die(bb_msg_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			continue;
		}

		param = find_param(arg);
		ถ้า (param & param_need_arg) {
			ถ้า (!argnext)
				bb_error_msg_and_die(bb_msg_requires_arg, arg);
			++k;
		}

		switch (param) {
#ifdef __linux__
		pak_sa param_line:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			bha_nga;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		pak_sa param_rows:
		pak_sa param_cols:
		pak_sa param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			bha_nga;
		pak_sa param_size:
#endif
		pak_sa param_speed:
			bha_nga;
		pak_sa param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(input_speed, argnext, &mode);
			bha_nga;
		pak_sa param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			bha_nga;
		default:
			ถ้า (recover_mode(arg, &mode) == 1) bha_nga;
			ถ้า (tty_value_to_baud(xatou(arg)) != (speed_t) -1) bha_nga;
 invalid_argument:
			bb_error_msg_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	ถ้า ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_msg_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	ถ้า ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_msg_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	ถ้า (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, parimaa_na(mode));
	ถ้า (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	ถ้า (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		คืน EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	ขณะ (argv[++k]) {
		const sa_nracanaa mode_info *mp;
		const sa_nracanaa control_info *cp;
		const var_na *arg = argv[k];
		const var_na *argnext = argv[k+1];
		จำนวนเต็ม param;

		ถ้า (arg[0] == '-') {
			mp = find_mode(arg+1);
			ถ้า (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			continue;
		}

		mp = find_mode(arg);
		ถ้า (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			continue;
		}

		cp = find_control(arg);
		ถ้า (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			continue;
		}

		param = find_param(arg);
		ถ้า (param & param_need_arg) {
			++k;
		}

		switch (param) {
#ifdef __linux__
		pak_sa param_line:
			mode.c_line = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			bha_nga;
#endif
#ifdef TIOCGWINSZ
		pak_sa param_cols:
		pak_sa param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			bha_nga;
		pak_sa param_size:
			display_window_size(0);
			bha_nga;
		pak_sa param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			bha_nga;
#endif
		pak_sa param_speed:
			display_speed(&mode, 0);
			bha_nga;
		pak_sa param_ispeed:
			set_speed_or_die(input_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			bha_nga;
		pak_sa param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			bha_nga;
		default:
			ถ้า (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			มิฉะนั้น /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	ถ้า (stty_state & STTY_require_set_attr) {
		sa_nracanaa termios new_mode;

		ถ้า (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, parimaa_na(new_mode));
		ถ้า (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		ถ้า (memcmp(&mode, &new_mode, parimaa_na(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#ถ้า 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			ถ้า ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, parimaa_na(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	คืน EXIT_SUCCESS;
}
