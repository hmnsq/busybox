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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((unsigned অক্ষর) 0)
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
#যদি defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#যদি defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#যদি defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#যদি defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#যদি defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#যদি defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#যদি defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#যদি !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#যদি defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#যদি defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#যদি defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#যদি defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#যদি defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#যদি defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#যদি defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#যদি defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#যদি defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#যদি defined(VSTATUS) && !defined(CSTATUS)
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
গণনা speed_setting {
	প্রবেশ_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
গণনা {
	control, প্রবেশ, output, local, combination
};
স্থির tcflag_t *get_ptr_to_tcflag(unsigned type, ধ্রুবক কাঠামো termios *mode)
{
	স্থির ধ্রুবক uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(কাঠামো termios, c_cflag), /* control */
		offsetof(কাঠামো termios, c_iflag), /*প্রবেশ*/
		offsetof(কাঠামো termios, c_oflag), /* output */
		offsetof(কাঠামো termios, c_lflag)  /* local */
	};
	যদি (type <= local) {
		ফেরত (tcflag_t*) (((অক্ষর*)mode) + tcflag_offsets[type]);
	}
	ফেরত ফাঁকা;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
কাঠামো mode_info {
	ধ্রুবক uint8_t type;           /* Which structure element to change    */
	ধ্রুবক uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#যদি   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ধ্রুবক uint8_t mask;
#নতুবা_যদি (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ধ্রুবক uint16_t mask;
#অন্যথা
	ধ্রুবক tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	ধ্রুবক tcflag_t bits;          /* Bits to set for this mode            */
};

গণনা {
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
#যদি IXANY
	IDX_decctlq,
#endif
#যদি TABDLY || OXTABS
	IDX_tabs,
#endif
#যদি XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
স্থির ধ্রুবক অক্ষর mode_name[] ALIGN1 =
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
#যদি IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#যদি TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#যদি XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#যদি CMSPAR
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
#যদি CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   প্রবেশ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   প্রবেশ,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   প্রবেশ,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   প্রবেশ,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    প্রবেশ,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   প্রবেশ,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    প্রবেশ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    প্রবেশ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    প্রবেশ,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     প্রবেশ,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    প্রবেশ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   প্রবেশ,       OMIT       | REV,  IXOFF,      0 )
#যদি IUCLC
	MI_ENTRY("iuclc",    প্রবেশ,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#যদি IXANY
	MI_ENTRY("ixany",    প্রবেশ,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#যদি IMAXBEL
	MI_ENTRY("imaxbel",  প্রবেশ,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#যদি IUTF8
	MI_ENTRY("iutf8",    প্রবেশ,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#যদি OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#যদি OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#যদি ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#যদি ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#যদি ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#যদি OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#যদি OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#যদি NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#যদি CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#যদি TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# যদি TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# যদি TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#অন্যথা
# যদি OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#যদি BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#যদি VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#যদি FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#যদি IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#যদি XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#যদি TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#যদি ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#যদি ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#যদি ECHOKE
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

স্থির ধ্রুবক কাঠামো mode_info mode_info[] ALIGN4 = {
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
#যদি IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#যদি TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#যদি XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#যদি CMSPAR
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
#যদি CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   প্রবেশ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   প্রবেশ,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   প্রবেশ,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   প্রবেশ,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    প্রবেশ,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   প্রবেশ,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    প্রবেশ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    প্রবেশ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    প্রবেশ,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     প্রবেশ,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    প্রবেশ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   প্রবেশ,       OMIT       | REV,  IXOFF,      0 )
#যদি IUCLC
	MI_ENTRY("iuclc",    প্রবেশ,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#যদি IXANY
	MI_ENTRY("ixany",    প্রবেশ,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#যদি IMAXBEL
	MI_ENTRY("imaxbel",  প্রবেশ,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#যদি IUTF8
	MI_ENTRY("iutf8",    প্রবেশ,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#যদি OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#যদি OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#যদি ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#যদি ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#যদি ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#যদি OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#যদি OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#যদি NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#যদি CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#যদি TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# যদি TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# যদি TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#অন্যথা
# যদি OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#যদি BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#যদি VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#যদি FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#যদি IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#যদি XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#যদি TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#যদি ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#যদি ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#যদি ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

গণনা {
	অঙ্ক_mode_info = ARRAY_আকার(mode_info)
};


/* Control characters */
কাঠামো control_info {
	ধ্রুবক uint8_t saneval;  /* Value to set for 'stty sane' */
	ধ্রুবক uint8_t offset;   /* Offset in c_cc */
};

গণনা {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#যদি VEOL2
	CIDX_eol2,
#endif
#যদি VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#যদি VDSUSP
	CIDX_dsusp,
#endif
#যদি VREPRINT
	CIDX_rprnt,
#endif
#যদি VWERASE
	CIDX_werase,
#endif
#যদি VLNEXT
	CIDX_lnext,
#endif
#যদি VFLUSHO
	CIDX_flush,
#endif
#যদি VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
স্থির ধ্রুবক অক্ষর control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#যদি VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#যদি VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#যদি VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#যদি VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#যদি VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#যদি VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#যদি VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#যদি VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

স্থির ধ্রুবক কাঠামো control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#যদি VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#যদি VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#যদি VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#যদি VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#যদি VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#যদি VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#যদি VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#যদি VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

গণনা {
	অঙ্ক_control_info = ARRAY_আকার(control_info)
};


কাঠামো globals {
	ধ্রুবক অক্ষর *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { \
	setup_common_bufsiz(); \
	G.device_name = bb_বার্তা_standard_প্রবেশ; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} যখন (0)

স্থির শূন্য set_speed_or_die(গণনা speed_setting type, ধ্রুবক অক্ষর *arg,
					কাঠামো termios *mode)
{
	speed_t baud;

	baud = tty_মান_to_baud(xatou(arg));

	যদি (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	যদি (type != প্রবেশ_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

স্থির NORETURN শূন্য perror_on_device_and_die(ধ্রুবক অক্ষর *fmt)
{
	bb_perror_বার্তা_and_die(fmt, G.device_name);
}

স্থির শূন্য perror_on_device(ধ্রুবক অক্ষর *fmt)
{
	bb_perror_বার্তা(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
স্থির শূন্য wrapf(ধ্রুবক অক্ষর *message, ...)
{
	অক্ষর buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, আকার(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	যদি (!buflen || buflen >= আকার(buf)) ফেরত;

	যদি (G.current_col > 0) {
		G.current_col++;
		যদি (buf[0] != '\n') {
			যদি (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} অন্যথা {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	যদি (buf[buflen-1] == '\n')
		G.current_col = 0;
}

স্থির শূন্য newline(শূন্য)
{
	যদি (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
স্থির শূন্য set_window_আকার(পূর্ণসংখ্যা rows, পূর্ণসংখ্যা cols)
{
	কাঠামো winsize win = { 0, 0, 0, 0 };

	যদি (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		যদি (errno != EINVAL) {
			যাও bail;
		}
		memset(&win, 0, আকার(win));
	}

	যদি (rows >= 0)
		win.ws_row = rows;
	যদি (cols >= 0)
		win.ws_col = cols;

	যদি (ioctl(STDIN_FILENO, TIOCSWINSZ, (অক্ষর *) &win))
bail:
		perror_on_device("%s");
}
#endif

স্থির শূন্য display_window_আকার(পূর্ণসংখ্যা fancy)
{
	ধ্রুবক অক্ষর *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	যদি (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		যদি ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} অন্যথা {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

স্থির ধ্রুবক কাঠামো suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

স্থির ধ্রুবক কাঠামো mode_info *find_mode(ধ্রুবক অক্ষর *name)
{
	পূর্ণসংখ্যা i = সূচক_in_strings(mode_name, name);
	ফেরত i >= 0 ? &mode_info[i] : ফাঁকা;
}

স্থির ধ্রুবক কাঠামো control_info *find_control(ধ্রুবক অক্ষর *name)
{
	পূর্ণসংখ্যা i = সূচক_in_strings(control_name, name);
	ফেরত i >= 0 ? &control_info[i] : ফাঁকা;
}

গণনা {
	param_need_arg = 0x80,
	param_সারি    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_আকার    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

স্থির পূর্ণসংখ্যা find_param(ধ্রুবক অক্ষর *name)
{
	স্থির ধ্রুবক অক্ষর params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	পূর্ণসংখ্যা i = সূচক_in_strings(params, name) + 1;
	যদি (i == 0)
		ফেরত 0;
	যদি (i != 5 && i != 6)
		i |= 0x80;
	ফেরত i;
}

স্থির পূর্ণসংখ্যা recover_mode(ধ্রুবক অক্ষর *arg, কাঠামো termios *mode)
{
	পূর্ণসংখ্যা i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	যদি (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		ফেরত 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	জন্য (i = 0; i < NCCS; ++i) {
		যদি (sscanf(arg, ":%x%n", &chr, &n) != 1)
			ফেরত 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	যদি (*arg != '\0')
		ফেরত 0;

	ফেরত 1;
}

স্থির শূন্য display_recoverable(ধ্রুবক কাঠামো termios *mode,
				পূর্ণসংখ্যা UNUSED_PARAM dummy)
{
	পূর্ণসংখ্যা i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	জন্য (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned পূর্ণসংখ্যা) mode->c_cc[i]);
	bb_putchar('\n');
}

স্থির শূন্য display_speed(ধ্রুবক কাঠামো termios *mode, পূর্ণসংখ্যা fancy)
{
	//____________________ 01234567 8 9
	ধ্রুবক অক্ষর *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	যদি (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	যদি (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_মান(ispeed), tty_baud_to_মান(ospeed));
}

স্থির শূন্য do_display(ধ্রুবক কাঠামো termios *mode, পূর্ণসংখ্যা all)
{
	পূর্ণসংখ্যা i;
	tcflag_t *bitsp;
	unsigned long mask;
	পূর্ণসংখ্যা পূর্ব_type = control;

	display_speed(mode, 1);
	যদি (all)
		display_window_আকার(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_সারি);
#অন্যথা
	newline();
#endif

	জন্য (i = 0; i != CIDX_min; ++i) {
		অক্ষর ch;
		অক্ষর buf10[10];

		/* If swtch is the same as susp, don't print both */
#যদি VSWTCH == VSUSP
		যদি (i == CIDX_swtch)
			চালু;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#যদি VEOF == VMIN
		যদি (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			চালু;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		যদি (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		অন্যথা
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#যদি VEOF == VMIN
	যদি ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	জন্য (i = 0; i < অঙ্ক_mode_info; ++i) {
		যদি (mode_info[i].flags & OMIT)
			চালু;
		যদি (mode_info[i].type != পূর্ব_type) {
			newline();
			পূর্ব_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		যদি ((*bitsp & mask) == mode_info[i].bits) {
			যদি (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} অন্যথা {
			যদি ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

স্থির শূন্য sane_mode(কাঠামো termios *mode)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < অঙ্ক_control_info; ++i) {
#যদি VMIN == VEOF
		যদি (i == CIDX_min)
			বিরতি;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	জন্য (i = 0; i < অঙ্ক_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		যদি (!bitsp)
			চালু;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		যদি (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} অন্যথা
		যদি (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

স্থির শূন্য set_mode(ধ্রুবক কাঠামো mode_info *info, পূর্ণসংখ্যা reversed,
					কাঠামো termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	যদি (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		যদি (reversed)
			*bitsp = val & ~info->bits;
		অন্যথা
			*bitsp = val | info->bits;
		ফেরত;
	}

	/* !bitsp - it's a "combination" mode */
	যদি (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		যদি (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		অন্যথা
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} অন্যথা যদি (info == &mode_info[IDX_oddp]) {
		যদি (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		অন্যথা
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} অন্যথা যদি (info == &mode_info[IDX_nl]) {
		যদি (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} অন্যথা {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			যদি (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} অন্যথা যদি (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} অন্যথা যদি (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} অন্যথা যদি (info == &mode_info[IDX_cbreak]) {
		যদি (reversed)
			mode->c_lflag |= ICANON;
		অন্যথা
			mode->c_lflag &= ~ICANON;
	} অন্যথা যদি (info == &mode_info[IDX_pass8]) {
		যদি (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} অন্যথা {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} অন্যথা যদি (info == &mode_info[IDX_litout]) {
		যদি (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} অন্যথা {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} অন্যথা যদি (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		যদি ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#যদি VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#যদি VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} অন্যথা {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#যদি IXANY
	অন্যথা যদি (info == &mode_info[IDX_decctlq]) {
		যদি (reversed)
			mode->c_iflag |= IXANY;
		অন্যথা
			mode->c_iflag &= ~IXANY;
	}
#endif
#যদি TABDLY
	অন্যথা যদি (info == &mode_info[IDX_tabs]) {
		যদি (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		অন্যথা
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#যদি OXTABS
	অন্যথা যদি (info == &mode_info[IDX_tabs]) {
		যদি (reversed)
			mode->c_oflag |= OXTABS;
		অন্যথা
			mode->c_oflag &= ~OXTABS;
	}
#endif
#যদি XCASE && IUCLC && OLCUC
	অন্যথা যদি (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		যদি (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} অন্যথা {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	অন্যথা যদি (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} অন্যথা যদি (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		যদি (IXANY) mode->c_iflag &= ~IXANY;
	}
}

স্থির শূন্য set_control_char_or_die(ধ্রুবক কাঠামো control_info *info,
			ধ্রুবক অক্ষর *arg, কাঠামো termios *mode)
{
	unsigned অক্ষর মান_2;

	যদি (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		মান_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	অন্যথা যদি (arg[0] == '\0' || arg[1] == '\0')
		মান_2 = arg[0];
	অন্যথা যদি (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		মান_2 = _POSIX_VDISABLE;
	অন্যথা যদি (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		মান_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		যদি (arg[1] == '?')
			মান_2 = 127;
	} অন্যথা
		মান_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = মান_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

পূর্ণসংখ্যা stty_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা stty_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	কাঠামো termios mode;
	শূন্য (*output_func)(ধ্রুবক কাঠামো termios *, পূর্ণসংখ্যা);
	ধ্রুবক অক্ষর *file_name = ফাঁকা;
	পূর্ণসংখ্যা display_all = 0;
	পূর্ণসংখ্যা stty_state;
	পূর্ণসংখ্যা k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	যখন (argv[++k]) {
		ধ্রুবক কাঠামো mode_info *mp;
		ধ্রুবক কাঠামো control_info *cp;
		ধ্রুবক অক্ষর *arg = argv[k];
		ধ্রুবক অক্ষর *argnext = argv[k+1];
		পূর্ণসংখ্যা param;

		যদি (arg[0] == '-') {
			পূর্ণসংখ্যা i;
			mp = find_mode(arg+1);
			যদি (mp) {
				যদি (!(mp->flags & REV))
					যাও invalid_argument;
				stty_state &= ~STTY_noargs;
				চালু;
			}
			/* It is an option - parse it */
			i = 0;
			যখন (arg[++i]) {
				নির্বাচন (arg[i]) {
				ক্ষেত্র 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					বিরতি;
				ক্ষেত্র 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					বিরতি;
				ক্ষেত্র 'F':
					যদি (file_name)
						bb_simple_error_বার্তা_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					যদি (!file_name[0]) { /* nope, "-F device" */
						পূর্ণসংখ্যা p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						যদি (!file_name)
							bb_error_বার্তা_and_die(bb_বার্তা_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						যখন (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					যাও end_option;
				পূর্বনির্ধারিত:
					যাও invalid_argument;
				}
			}
 end_option:
			চালু;
		}

		mp = find_mode(arg);
		যদি (mp) {
			stty_state &= ~STTY_noargs;
			চালু;
		}

		cp = find_control(arg);
		যদি (cp) {
			যদি (!argnext)
				bb_error_বার্তা_and_die(bb_বার্তা_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			চালু;
		}

		param = find_param(arg);
		যদি (param & param_need_arg) {
			যদি (!argnext)
				bb_error_বার্তা_and_die(bb_বার্তা_requires_arg, arg);
			++k;
		}

		নির্বাচন (param) {
#ifdef __linux__
		ক্ষেত্র param_সারি:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			বিরতি;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		ক্ষেত্র param_rows:
		ক্ষেত্র param_cols:
		ক্ষেত্র param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			বিরতি;
		ক্ষেত্র param_আকার:
#endif
		ক্ষেত্র param_speed:
			বিরতি;
		ক্ষেত্র param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(প্রবেশ_speed, argnext, &mode);
			বিরতি;
		ক্ষেত্র param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			বিরতি;
		পূর্বনির্ধারিত:
			যদি (recover_mode(arg, &mode) == 1) বিরতি;
			যদি (tty_মান_to_baud(xatou(arg)) != (speed_t) -1) বিরতি;
 invalid_argument:
			bb_error_বার্তা_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	যদি ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_বার্তা_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	যদি ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_বার্তা_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	যদি (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, আকার(mode));
	যদি (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	যদি (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		ফেরত EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	যখন (argv[++k]) {
		ধ্রুবক কাঠামো mode_info *mp;
		ধ্রুবক কাঠামো control_info *cp;
		ধ্রুবক অক্ষর *arg = argv[k];
		ধ্রুবক অক্ষর *argnext = argv[k+1];
		পূর্ণসংখ্যা param;

		যদি (arg[0] == '-') {
			mp = find_mode(arg+1);
			যদি (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			চালু;
		}

		mp = find_mode(arg);
		যদি (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			চালু;
		}

		cp = find_control(arg);
		যদি (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			চালু;
		}

		param = find_param(arg);
		যদি (param & param_need_arg) {
			++k;
		}

		নির্বাচন (param) {
#ifdef __linux__
		ক্ষেত্র param_সারি:
			mode.c_সারি = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			বিরতি;
#endif
#ifdef TIOCGWINSZ
		ক্ষেত্র param_cols:
		ক্ষেত্র param_columns:
			set_window_আকার(-1, xatoul_sfx(argnext, stty_suffixes));
			বিরতি;
		ক্ষেত্র param_আকার:
			display_window_আকার(0);
			বিরতি;
		ক্ষেত্র param_rows:
			set_window_আকার(xatoul_sfx(argnext, stty_suffixes), -1);
			বিরতি;
#endif
		ক্ষেত্র param_speed:
			display_speed(&mode, 0);
			বিরতি;
		ক্ষেত্র param_ispeed:
			set_speed_or_die(প্রবেশ_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			বিরতি;
		ক্ষেত্র param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			বিরতি;
		পূর্বনির্ধারিত:
			যদি (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			অন্যথা /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	যদি (stty_state & STTY_require_set_attr) {
		কাঠামো termios new_mode;

		যদি (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, আকার(new_mode));
		যদি (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		যদি (memcmp(&mode, &new_mode, আকার(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#যদি 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			যদি ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, আকার(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	ফেরত EXIT_SUCCESS;
}
