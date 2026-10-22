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
# define _POSIX_VDISABLE ((unsigned वर्ण) 0)
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
#सचे defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#सचे defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#सचे defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#सचे defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#सचे defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#सचे defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#सचे defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#सचे !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#सचे defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#सचे defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#सचे defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#सचे defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#सचे defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#सचे defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#सचे defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#सचे defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#सचे defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#सचे defined(VSTATUS) && !defined(CSTATUS)
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
	निवेशः_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
गणना {
	control, निवेशः, output, local, combination
};
स्थिर tcflag_t *get_ptr_to_tcflag(unsigned type, ध्रुव संरचना termios *mode)
{
	स्थिर ध्रुव uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(संरचना termios, c_cflag), /* control */
		offsetof(संरचना termios, c_iflag), /*निवेशः*/
		offsetof(संरचना termios, c_oflag), /* output */
		offsetof(संरचना termios, c_lflag)  /* local */
	};
	सचे (type <= local) {
		पटिनिवत्त (tcflag_t*) (((वर्ण*)mode) + tcflag_offsets[type]);
	}
	पटिनिवत्त रिक्त;
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
	ध्रुव uint8_t type;           /* Which structure element to change    */
	ध्रुव uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#सचे   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ध्रुव uint8_t mask;
#नोचेद्यदि (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ध्रुव uint16_t mask;
#अञ्ञथा
	ध्रुव tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	ध्रुव tcflag_t bits;          /* Bits to set for this mode            */
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
#सचे IXANY
	IDX_decctlq,
#endif
#सचे TABDLY || OXTABS
	IDX_tabs,
#endif
#सचे XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
स्थिर ध्रुव वर्ण mode_name[] ALIGN1 =
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
#सचे IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#सचे TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#सचे XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#सचे CMSPAR
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
#सचे CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   निवेशः,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेशः,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेशः,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेशः,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेशः,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेशः,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेशः,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेशः,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेशः,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेशः,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेशः,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेशः,       OMIT       | REV,  IXOFF,      0 )
#सचे IUCLC
	MI_ENTRY("iuclc",    निवेशः,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#सचे IXANY
	MI_ENTRY("ixany",    निवेशः,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#सचे IMAXBEL
	MI_ENTRY("imaxbel",  निवेशः,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#सचे IUTF8
	MI_ENTRY("iutf8",    निवेशः,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#सचे OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#सचे OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#सचे ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#सचे ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#सचे ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#सचे OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#सचे OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#सचे NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#सचे CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#सचे TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# सचे TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# सचे TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अञ्ञथा
# सचे OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#सचे BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#सचे VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#सचे FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#सचे IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#सचे XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#सचे TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#सचे ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#सचे ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#सचे ECHOKE
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

स्थिर ध्रुव संरचना mode_info mode_info[] ALIGN4 = {
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
#सचे IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#सचे TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#सचे XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#सचे CMSPAR
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
#सचे CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   निवेशः,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेशः,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेशः,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेशः,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेशः,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेशः,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेशः,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेशः,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेशः,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेशः,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेशः,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेशः,       OMIT       | REV,  IXOFF,      0 )
#सचे IUCLC
	MI_ENTRY("iuclc",    निवेशः,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#सचे IXANY
	MI_ENTRY("ixany",    निवेशः,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#सचे IMAXBEL
	MI_ENTRY("imaxbel",  निवेशः,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#सचे IUTF8
	MI_ENTRY("iutf8",    निवेशः,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#सचे OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#सचे OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#सचे ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#सचे ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#सचे ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#सचे OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#सचे OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#सचे NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#सचे CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#सचे TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# सचे TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# सचे TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अञ्ञथा
# सचे OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#सचे BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#सचे VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#सचे FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#सचे IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#सचे XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#सचे TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#सचे ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#सचे ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#सचे ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

गणना {
	अङ्कः_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
संरचना control_info {
	ध्रुव uint8_t saneval;  /* Value to set for 'stty sane' */
	ध्रुव uint8_t offset;   /* Offset in c_cc */
};

गणना {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#सचे VEOL2
	CIDX_eol2,
#endif
#सचे VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#सचे VDSUSP
	CIDX_dsusp,
#endif
#सचे VREPRINT
	CIDX_rprnt,
#endif
#सचे VWERASE
	CIDX_werase,
#endif
#सचे VLNEXT
	CIDX_lnext,
#endif
#सचे VFLUSHO
	CIDX_flush,
#endif
#सचे VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
स्थिर ध्रुव वर्ण control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#सचे VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#सचे VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#सचे VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#सचे VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#सचे VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#सचे VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#सचे VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#सचे VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

स्थिर ध्रुव संरचना control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#सचे VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#सचे VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#सचे VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#सचे VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#सचे VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#सचे VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#सचे VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#सचे VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

गणना {
	अङ्कः_control_info = ARRAY_SIZE(control_info)
};


संरचना globals {
	ध्रुव वर्ण *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करोहि { \
	setup_common_bufsiz(); \
	G.device_name = bb_सन्देशः_standard_निवेशः; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} याव (0)

स्थिर शून्य set_speed_or_die(गणना speed_setting type, ध्रुव वर्ण *arg,
					संरचना termios *mode)
{
	speed_t baud;

	baud = tty_मानम्_to_baud(xatou(arg));

	सचे (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	सचे (type != निवेशः_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

स्थिर NORETURN शून्य perror_on_device_and_die(ध्रुव वर्ण *fmt)
{
	bb_perror_सन्देशः_and_die(fmt, G.device_name);
}

स्थिर शून्य perror_on_device(ध्रुव वर्ण *fmt)
{
	bb_perror_सन्देशः(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
स्थिर शून्य wrapf(ध्रुव वर्ण *message, ...)
{
	वर्ण buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, परिमाण(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	सचे (!buflen || buflen >= परिमाण(buf)) पटिनिवत्त;

	सचे (G.current_col > 0) {
		G.current_col++;
		सचे (buf[0] != '\n') {
			सचे (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} अञ्ञथा {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	सचे (buf[buflen-1] == '\n')
		G.current_col = 0;
}

स्थिर शून्य newline(शून्य)
{
	सचे (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
स्थिर शून्य set_window_size(पूर्णाङ्क rows, पूर्णाङ्क cols)
{
	संरचना winsize win = { 0, 0, 0, 0 };

	सचे (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		सचे (errno != EINVAL) {
			गच्छ bail;
		}
		memset(&win, 0, परिमाण(win));
	}

	सचे (rows >= 0)
		win.ws_row = rows;
	सचे (cols >= 0)
		win.ws_col = cols;

	सचे (ioctl(STDIN_FILENO, TIOCSWINSZ, (वर्ण *) &win))
bail:
		perror_on_device("%s");
}
#endif

स्थिर शून्य display_window_size(पूर्णाङ्क fancy)
{
	ध्रुव वर्ण *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	सचे (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		सचे ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} अञ्ञथा {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

स्थिर ध्रुव संरचना suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

स्थिर ध्रुव संरचना mode_info *find_mode(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i = index_in_strings(mode_name, name);
	पटिनिवत्त i >= 0 ? &mode_info[i] : रिक्त;
}

स्थिर ध्रुव संरचना control_info *find_control(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i = index_in_strings(control_name, name);
	पटिनिवत्त i >= 0 ? &control_info[i] : रिक्त;
}

गणना {
	param_need_arg = 0x80,
	param_पङ्क्तिः    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

स्थिर पूर्णाङ्क find_param(ध्रुव वर्ण *name)
{
	स्थिर ध्रुव वर्ण params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	पूर्णाङ्क i = index_in_strings(params, name) + 1;
	सचे (i == 0)
		पटिनिवत्त 0;
	सचे (i != 5 && i != 6)
		i |= 0x80;
	पटिनिवत्त i;
}

स्थिर पूर्णाङ्क recover_mode(ध्रुव वर्ण *arg, संरचना termios *mode)
{
	पूर्णाङ्क i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	सचे (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		पटिनिवत्त 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	अत्थाय (i = 0; i < NCCS; ++i) {
		सचे (sscanf(arg, ":%x%n", &chr, &n) != 1)
			पटिनिवत्त 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	सचे (*arg != '\0')
		पटिनिवत्त 0;

	पटिनिवत्त 1;
}

स्थिर शून्य display_recoverable(ध्रुव संरचना termios *mode,
				पूर्णाङ्क UNUSED_PARAM dummy)
{
	पूर्णाङ्क i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	अत्थाय (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned पूर्णाङ्क) mode->c_cc[i]);
	bb_putchar('\n');
}

स्थिर शून्य display_speed(ध्रुव संरचना termios *mode, पूर्णाङ्क fancy)
{
	//____________________ 01234567 8 9
	ध्रुव वर्ण *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	सचे (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	सचे (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_मानम्(ispeed), tty_baud_to_मानम्(ospeed));
}

स्थिर शून्य do_display(ध्रुव संरचना termios *mode, पूर्णाङ्क all)
{
	पूर्णाङ्क i;
	tcflag_t *bitsp;
	unsigned long mask;
	पूर्णाङ्क prev_type = control;

	display_speed(mode, 1);
	सचे (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_पङ्क्तिः);
#अञ्ञथा
	newline();
#endif

	अत्थाय (i = 0; i != CIDX_min; ++i) {
		वर्ण ch;
		वर्ण buf10[10];

		/* If swtch is the same as susp, don't print both */
#सचे VSWTCH == VSUSP
		सचे (i == CIDX_swtch)
			अनुवर्त;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#सचे VEOF == VMIN
		सचे (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			अनुवर्त;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		सचे (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		अञ्ञथा
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#सचे VEOF == VMIN
	सचे ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	अत्थाय (i = 0; i < अङ्कः_mode_info; ++i) {
		सचे (mode_info[i].flags & OMIT)
			अनुवर्त;
		सचे (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		सचे ((*bitsp & mask) == mode_info[i].bits) {
			सचे (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} अञ्ञथा {
			सचे ((all && mode_info[i].flags & REV)
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
	पूर्णाङ्क i;

	अत्थाय (i = 0; i < अङ्कः_control_info; ++i) {
#सचे VMIN == VEOF
		सचे (i == CIDX_min)
			भङ्ग;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	अत्थाय (i = 0; i < अङ्कः_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		सचे (!bitsp)
			अनुवर्त;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		सचे (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} अञ्ञथा
		सचे (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

स्थिर शून्य set_mode(ध्रुव संरचना mode_info *info, पूर्णाङ्क reversed,
					संरचना termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	सचे (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		सचे (reversed)
			*bitsp = val & ~info->bits;
		अञ्ञथा
			*bitsp = val | info->bits;
		पटिनिवत्त;
	}

	/* !bitsp - it's a "combination" mode */
	सचे (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		सचे (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अञ्ञथा
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} अञ्ञथा सचे (info == &mode_info[IDX_oddp]) {
		सचे (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अञ्ञथा
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} अञ्ञथा सचे (info == &mode_info[IDX_nl]) {
		सचे (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} अञ्ञथा {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			सचे (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} अञ्ञथा सचे (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} अञ्ञथा सचे (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} अञ्ञथा सचे (info == &mode_info[IDX_cbreak]) {
		सचे (reversed)
			mode->c_lflag |= ICANON;
		अञ्ञथा
			mode->c_lflag &= ~ICANON;
	} अञ्ञथा सचे (info == &mode_info[IDX_pass8]) {
		सचे (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} अञ्ञथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} अञ्ञथा सचे (info == &mode_info[IDX_litout]) {
		सचे (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} अञ्ञथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} अञ्ञथा सचे (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		सचे ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#सचे VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#सचे VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} अञ्ञथा {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#सचे IXANY
	अञ्ञथा सचे (info == &mode_info[IDX_decctlq]) {
		सचे (reversed)
			mode->c_iflag |= IXANY;
		अञ्ञथा
			mode->c_iflag &= ~IXANY;
	}
#endif
#सचे TABDLY
	अञ्ञथा सचे (info == &mode_info[IDX_tabs]) {
		सचे (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		अञ्ञथा
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#सचे OXTABS
	अञ्ञथा सचे (info == &mode_info[IDX_tabs]) {
		सचे (reversed)
			mode->c_oflag |= OXTABS;
		अञ्ञथा
			mode->c_oflag &= ~OXTABS;
	}
#endif
#सचे XCASE && IUCLC && OLCUC
	अञ्ञथा सचे (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		सचे (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} अञ्ञथा {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	अञ्ञथा सचे (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} अञ्ञथा सचे (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		सचे (IXANY) mode->c_iflag &= ~IXANY;
	}
}

स्थिर शून्य set_control_char_or_die(ध्रुव संरचना control_info *info,
			ध्रुव वर्ण *arg, संरचना termios *mode)
{
	unsigned वर्ण मानम्_2;

	सचे (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		मानम्_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	अञ्ञथा सचे (arg[0] == '\0' || arg[1] == '\0')
		मानम्_2 = arg[0];
	अञ्ञथा सचे (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		मानम्_2 = _POSIX_VDISABLE;
	अञ्ञथा सचे (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		मानम्_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		सचे (arg[1] == '?')
			मानम्_2 = 127;
	} अञ्ञथा
		मानम्_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = मानम्_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

पूर्णाङ्क stty_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क stty_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	संरचना termios mode;
	शून्य (*output_func)(ध्रुव संरचना termios *, पूर्णाङ्क);
	ध्रुव वर्ण *file_name = रिक्त;
	पूर्णाङ्क display_all = 0;
	पूर्णाङ्क stty_state;
	पूर्णाङ्क k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	याव (argv[++k]) {
		ध्रुव संरचना mode_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		सचे (arg[0] == '-') {
			पूर्णाङ्क i;
			mp = find_mode(arg+1);
			सचे (mp) {
				सचे (!(mp->flags & REV))
					गच्छ invalid_argument;
				stty_state &= ~STTY_noargs;
				अनुवर्त;
			}
			/* It is an option - parse it */
			i = 0;
			याव (arg[++i]) {
				विकल्प (arg[i]) {
				पक्ष 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					भङ्ग;
				पक्ष 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					भङ्ग;
				पक्ष 'F':
					सचे (file_name)
						bb_simple_error_सन्देशः_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					सचे (!file_name[0]) { /* nope, "-F device" */
						पूर्णाङ्क p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						सचे (!file_name)
							bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						याव (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					गच्छ end_option;
				स्वभाव:
					गच्छ invalid_argument;
				}
			}
 end_option:
			अनुवर्त;
		}

		mp = find_mode(arg);
		सचे (mp) {
			stty_state &= ~STTY_noargs;
			अनुवर्त;
		}

		cp = find_control(arg);
		सचे (cp) {
			सचे (!argnext)
				bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			अनुवर्त;
		}

		param = find_param(arg);
		सचे (param & param_need_arg) {
			सचे (!argnext)
				bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, arg);
			++k;
		}

		विकल्प (param) {
#ifdef __linux__
		पक्ष param_पङ्क्तिः:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			भङ्ग;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		पक्ष param_rows:
		पक्ष param_cols:
		पक्ष param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			भङ्ग;
		पक्ष param_size:
#endif
		पक्ष param_speed:
			भङ्ग;
		पक्ष param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(निवेशः_speed, argnext, &mode);
			भङ्ग;
		पक्ष param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			भङ्ग;
		स्वभाव:
			सचे (recover_mode(arg, &mode) == 1) भङ्ग;
			सचे (tty_मानम्_to_baud(xatou(arg)) != (speed_t) -1) भङ्ग;
 invalid_argument:
			bb_error_सन्देशः_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	सचे ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_सन्देशः_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	सचे ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_सन्देशः_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	सचे (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, परिमाण(mode));
	सचे (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	सचे (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		पटिनिवत्त EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	याव (argv[++k]) {
		ध्रुव संरचना mode_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		सचे (arg[0] == '-') {
			mp = find_mode(arg+1);
			सचे (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			अनुवर्त;
		}

		mp = find_mode(arg);
		सचे (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			अनुवर्त;
		}

		cp = find_control(arg);
		सचे (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			अनुवर्त;
		}

		param = find_param(arg);
		सचे (param & param_need_arg) {
			++k;
		}

		विकल्प (param) {
#ifdef __linux__
		पक्ष param_पङ्क्तिः:
			mode.c_पङ्क्तिः = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			भङ्ग;
#endif
#ifdef TIOCGWINSZ
		पक्ष param_cols:
		पक्ष param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			भङ्ग;
		पक्ष param_size:
			display_window_size(0);
			भङ्ग;
		पक्ष param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			भङ्ग;
#endif
		पक्ष param_speed:
			display_speed(&mode, 0);
			भङ्ग;
		पक्ष param_ispeed:
			set_speed_or_die(निवेशः_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			भङ्ग;
		पक्ष param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			भङ्ग;
		स्वभाव:
			सचे (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			अञ्ञथा /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	सचे (stty_state & STTY_require_set_attr) {
		संरचना termios new_mode;

		सचे (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, परिमाण(new_mode));
		सचे (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		सचे (memcmp(&mode, &new_mode, परिमाण(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#सचे 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			सचे ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, परिमाण(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	पटिनिवत्त EXIT_SUCCESS;
}
