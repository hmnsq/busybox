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
# define _POSIX_VDISABLE ((अचिन्हित वर्ण) 0)
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
#यदि defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#यदि defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#यदि defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#यदि defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#यदि defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#यदि defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#यदि defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#यदि !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#यदि defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#यदि defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#यदि defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#यदि defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#यदि defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#यदि defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#यदि defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#यदि defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#यदि defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#यदि defined(VSTATUS) && !defined(CSTATUS)
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
	निवेश_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
गणना {
	control, निवेश, output, local, combination
};
स्थिर tcflag_t *get_ptr_to_tcflag(अचिन्हित type, ध्रुव संरचना termios *mode)
{
	स्थिर ध्रुव uint8_t tcflag_offsets[] ALIGN1 = {
		दुरत्व(संरचना termios, c_cflag), /* control */
		दुरत्व(संरचना termios, c_iflag), /*निवेश*/
		दुरत्व(संरचना termios, c_oflag), /* output */
		दुरत्व(संरचना termios, c_lflag)  /* local */
	};
	यदि (type <= local) {
		प्रत्यागम (tcflag_t*) (((वर्ण*)mode) + tcflag_offsets[type]);
	}
	प्रत्यागम रिक्त;
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
#यदि   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ध्रुव uint8_t mask;
#नोचेद्यदि (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ध्रुव uint16_t mask;
#अन्यथा
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
#यदि IXANY
	IDX_decctlq,
#endif
#यदि TABDLY || OXTABS
	IDX_tabs,
#endif
#यदि XCASE && IUCLC && OLCUC
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
#यदि IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#यदि TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#यदि XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#यदि CMSPAR
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
#यदि CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   निवेश,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेश,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेश,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेश,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेश,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेश,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेश,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेश,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेश,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेश,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेश,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेश,       OMIT       | REV,  IXOFF,      0 )
#यदि IUCLC
	MI_ENTRY("iuclc",    निवेश,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#यदि IXANY
	MI_ENTRY("ixany",    निवेश,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#यदि IMAXBEL
	MI_ENTRY("imaxbel",  निवेश,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#यदि IUTF8
	MI_ENTRY("iutf8",    निवेश,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#यदि OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#यदि OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#यदि ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#यदि ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#यदि ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#यदि OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#यदि OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#यदि NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#यदि CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#यदि TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# यदि TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# यदि TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# यदि OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#यदि BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#यदि VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#यदि FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#यदि IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#यदि XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#यदि TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#यदि ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#यदि ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#यदि ECHOKE
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
#यदि IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#यदि TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#यदि XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#यदि CMSPAR
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
#यदि CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   निवेश,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेश,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेश,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेश,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेश,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेश,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेश,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेश,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेश,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेश,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेश,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेश,       OMIT       | REV,  IXOFF,      0 )
#यदि IUCLC
	MI_ENTRY("iuclc",    निवेश,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#यदि IXANY
	MI_ENTRY("ixany",    निवेश,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#यदि IMAXBEL
	MI_ENTRY("imaxbel",  निवेश,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#यदि IUTF8
	MI_ENTRY("iutf8",    निवेश,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#यदि OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#यदि OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#यदि ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#यदि ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#यदि ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#यदि OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#यदि OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#यदि NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#यदि CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#यदि TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# यदि TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# यदि TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# यदि OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#यदि BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#यदि VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#यदि FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#यदि IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#यदि XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#यदि TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#यदि ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#यदि ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#यदि ECHOKE
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
#यदि VEOL2
	CIDX_eol2,
#endif
#यदि VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#यदि VDSUSP
	CIDX_dsusp,
#endif
#यदि VREPRINT
	CIDX_rprnt,
#endif
#यदि VWERASE
	CIDX_werase,
#endif
#यदि VLNEXT
	CIDX_lnext,
#endif
#यदि VFLUSHO
	CIDX_flush,
#endif
#यदि VSTATUS
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
#यदि VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#यदि VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#यदि VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#यदि VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#यदि VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#यदि VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#यदि VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#यदि VSTATUS
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
#यदि VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#यदि VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#यदि VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#यदि VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#यदि VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#यदि VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#यदि VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#यदि VSTATUS
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
	ध्रुव वर्ण *device_name;
	/* The width of the screen, for output wrapping */
	अचिन्हित max_col;
	/* Current position, to know when to wrap */
	अचिन्हित current_col;
} FIX_ALIASING;
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() कुरु { \
	setup_common_bufsiz(); \
	G.device_name = bb_संदेश_standard_निवेश; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} यावत् (0)

स्थिर शून्य set_speed_or_die(गणना speed_setting type, ध्रुव वर्ण *arg,
					संरचना termios *mode)
{
	speed_t baud;

	baud = tty_मूल्य_to_baud(xatou(arg));

	यदि (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	यदि (type != निवेश_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

स्थिर NORETURN शून्य perror_on_device_and_die(ध्रुव वर्ण *fmt)
{
	bb_perror_संदेश_and_die(fmt, G.device_name);
}

स्थिर शून्य perror_on_device(ध्रुव वर्ण *fmt)
{
	bb_perror_संदेश(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
स्थिर शून्य wrapf(ध्रुव वर्ण *message, ...)
{
	वर्ण buf[128];
	बहु_सूची args;
	अचिन्हित buflen;

	बहु_शुरू(args, message);
	buflen = vsnprintf(buf, परिमाण(buf), message, args);
	बहु_पूर्ण(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	यदि (!buflen || buflen >= परिमाण(buf)) प्रत्यागम;

	यदि (G.current_col > 0) {
		G.current_col++;
		यदि (buf[0] != '\n') {
			यदि (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} अन्यथा {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	यदि (buf[buflen-1] == '\n')
		G.current_col = 0;
}

स्थिर शून्य newline(शून्य)
{
	यदि (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
स्थिर शून्य set_window_size(पूर्णाङ्क rows, पूर्णाङ्क cols)
{
	संरचना winsize win = { 0, 0, 0, 0 };

	यदि (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		यदि (त्रुटि_सं != EINVAL) {
			गच्छ bail;
		}
		स_रखो(&win, 0, परिमाण(win));
	}

	यदि (rows >= 0)
		win.ws_row = rows;
	यदि (cols >= 0)
		win.ws_col = cols;

	यदि (ioctl(STDIN_FILENO, TIOCSWINSZ, (वर्ण *) &win))
bail:
		perror_on_device("%s");
}
#endif

स्थिर शून्य display_window_size(पूर्णाङ्क fancy)
{
	ध्रुव वर्ण *fmt_str = "%s\0%s: no size information for this device";
	अचिन्हित width, height;

	यदि (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		यदि ((त्रुटि_सं != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} अन्यथा {
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
	प्रत्यागम i >= 0 ? &mode_info[i] : रिक्त;
}

स्थिर ध्रुव संरचना control_info *find_control(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i = index_in_strings(control_name, name);
	प्रत्यागम i >= 0 ? &control_info[i] : रिक्त;
}

गणना {
	param_need_arg = 0x80,
	param_पंक्ति    = 1 | 0x80,
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
	यदि (i == 0)
		प्रत्यागम 0;
	यदि (i != 5 && i != 6)
		i |= 0x80;
	प्रत्यागम i;
}

स्थिर पूर्णाङ्क recover_mode(ध्रुव वर्ण *arg, संरचना termios *mode)
{
	पूर्णाङ्क i, n;
	अचिन्हित chr;
	अचिन्हित दीर्घ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	यदि (माला_पूछो(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		प्रत्यागम 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	प्रति (i = 0; i < NCCS; ++i) {
		यदि (माला_पूछो(arg, ":%x%n", &chr, &n) != 1)
			प्रत्यागम 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	यदि (*arg != '\0')
		प्रत्यागम 0;

	प्रत्यागम 1;
}

स्थिर शून्य display_recoverable(ध्रुव संरचना termios *mode,
				पूर्णाङ्क UNUSED_PARAM dummy)
{
	पूर्णाङ्क i;
	म_लिखो("%lx:%lx:%lx:%lx",
		   (अचिन्हित दीर्घ) mode->c_iflag, (अचिन्हित दीर्घ) mode->c_oflag,
		   (अचिन्हित दीर्घ) mode->c_cflag, (अचिन्हित दीर्घ) mode->c_lflag);
	प्रति (i = 0; i < NCCS; ++i)
		म_लिखो(":%x", (अचिन्हित पूर्णाङ्क) mode->c_cc[i]);
	bb_putchar('\n');
}

स्थिर शून्य display_speed(ध्रुव संरचना termios *mode, पूर्णाङ्क fancy)
{
	//____________________ 01234567 8 9
	ध्रुव वर्ण *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	अचिन्हित दीर्घ ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	यदि (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	यदि (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_मूल्य(ispeed), tty_baud_to_मूल्य(ospeed));
}

स्थिर शून्य do_display(ध्रुव संरचना termios *mode, पूर्णाङ्क all)
{
	पूर्णाङ्क i;
	tcflag_t *bitsp;
	अचिन्हित दीर्घ mask;
	पूर्णाङ्क prev_type = control;

	display_speed(mode, 1);
	यदि (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_पंक्ति);
#अन्यथा
	newline();
#endif

	प्रति (i = 0; i != CIDX_min; ++i) {
		वर्ण ch;
		वर्ण buf10[10];

		/* If swtch is the same as susp, don't print both */
#यदि VSWTCH == VSUSP
		यदि (i == CIDX_swtch)
			अनुवर्त;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#यदि VEOF == VMIN
		यदि (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			अनुवर्त;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		यदि (ch == _POSIX_VDISABLE)
			म_नकल(buf10, "<undef>");
		अन्यथा
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#यदि VEOF == VMIN
	यदि ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	प्रति (i = 0; i < अंक_mode_info; ++i) {
		यदि (mode_info[i].flags & OMIT)
			अनुवर्त;
		यदि (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		यदि ((*bitsp & mask) == mode_info[i].bits) {
			यदि (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} अन्यथा {
			यदि ((all && mode_info[i].flags & REV)
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

	प्रति (i = 0; i < अंक_control_info; ++i) {
#यदि VMIN == VEOF
		यदि (i == CIDX_min)
			भङ्ग;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	प्रति (i = 0; i < अंक_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		यदि (!bitsp)
			अनुवर्त;
		val = *bitsp & ~((अचिन्हित दीर्घ)mode_info[i].mask);
		यदि (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} अन्यथा
		यदि (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

स्थिर शून्य set_mode(ध्रुव संरचना mode_info *info, पूर्णाङ्क reversed,
					संरचना termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	यदि (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		यदि (reversed)
			*bitsp = val & ~info->bits;
		अन्यथा
			*bitsp = val | info->bits;
		प्रत्यागम;
	}

	/* !bitsp - it's a "combination" mode */
	यदि (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		यदि (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} अन्यथा यदि (info == &mode_info[IDX_oddp]) {
		यदि (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} अन्यथा यदि (info == &mode_info[IDX_nl]) {
		यदि (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} अन्यथा {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			यदि (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} अन्यथा यदि (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} अन्यथा यदि (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} अन्यथा यदि (info == &mode_info[IDX_cbreak]) {
		यदि (reversed)
			mode->c_lflag |= ICANON;
		अन्यथा
			mode->c_lflag &= ~ICANON;
	} अन्यथा यदि (info == &mode_info[IDX_pass8]) {
		यदि (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} अन्यथा यदि (info == &mode_info[IDX_litout]) {
		यदि (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} अन्यथा यदि (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		यदि ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#यदि VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#यदि VTIME == VEOL
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
#यदि IXANY
	अन्यथा यदि (info == &mode_info[IDX_decctlq]) {
		यदि (reversed)
			mode->c_iflag |= IXANY;
		अन्यथा
			mode->c_iflag &= ~IXANY;
	}
#endif
#यदि TABDLY
	अन्यथा यदि (info == &mode_info[IDX_tabs]) {
		यदि (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		अन्यथा
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#यदि OXTABS
	अन्यथा यदि (info == &mode_info[IDX_tabs]) {
		यदि (reversed)
			mode->c_oflag |= OXTABS;
		अन्यथा
			mode->c_oflag &= ~OXTABS;
	}
#endif
#यदि XCASE && IUCLC && OLCUC
	अन्यथा यदि (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		यदि (reversed) {
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
	अन्यथा यदि (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} अन्यथा यदि (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		यदि (IXANY) mode->c_iflag &= ~IXANY;
	}
}

स्थिर शून्य set_control_char_or_die(ध्रुव संरचना control_info *info,
			ध्रुव वर्ण *arg, संरचना termios *mode)
{
	अचिन्हित वर्ण मूल्य_2;

	यदि (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		मूल्य_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	अन्यथा यदि (arg[0] == '\0' || arg[1] == '\0')
		मूल्य_2 = arg[0];
	अन्यथा यदि (म_भेद(arg, "^-") == 0 || म_भेद(arg, "undef") == 0)
		मूल्य_2 = _POSIX_VDISABLE;
	अन्यथा यदि (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		मूल्य_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		यदि (arg[1] == '?')
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
	यावत् (argv[++k]) {
		ध्रुव संरचना mode_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		यदि (arg[0] == '-') {
			पूर्णाङ्क i;
			mp = find_mode(arg+1);
			यदि (mp) {
				यदि (!(mp->flags & REV))
					गच्छ invalid_argument;
				stty_state &= ~STTY_noargs;
				अनुवर्त;
			}
			/* It is an option - parse it */
			i = 0;
			यावत् (arg[++i]) {
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
					यदि (file_name)
						bb_simple_error_संदेश_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					यदि (!file_name[0]) { /* nope, "-F device" */
						पूर्णाङ्क p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						यदि (!file_name)
							bb_error_संदेश_and_die(bb_संदेश_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						यावत् (argv[p]) {
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
		यदि (mp) {
			stty_state &= ~STTY_noargs;
			अनुवर्त;
		}

		cp = find_control(arg);
		यदि (cp) {
			यदि (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			अनुवर्त;
		}

		param = find_param(arg);
		यदि (param & param_need_arg) {
			यदि (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			++k;
		}

		विकल्प (param) {
#ifdef __linux__
		पक्ष param_पंक्ति:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			भङ्ग;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		पक्ष param_rows:
		पक्ष param_cols:
		पक्ष param_columns:
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			भङ्ग;
		पक्ष param_size:
#endif
		पक्ष param_speed:
			भङ्ग;
		पक्ष param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(निवेश_speed, argnext, &mode);
			भङ्ग;
		पक्ष param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			भङ्ग;
		स्वभाव:
			यदि (recover_mode(arg, &mode) == 1) भङ्ग;
			यदि (tty_मूल्य_to_baud(xatou(arg)) != (speed_t) -1) भङ्ग;
 invalid_argument:
			bb_error_संदेश_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	यदि ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_संदेश_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	यदि ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_संदेश_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	यदि (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	स_रखो(&mode, 0, परिमाण(mode));
	यदि (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	यदि (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		प्रत्यागम निकास_सफल;
	}

	/* Second pass: perform actions */
	k = 0;
	यावत् (argv[++k]) {
		ध्रुव संरचना mode_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		यदि (arg[0] == '-') {
			mp = find_mode(arg+1);
			यदि (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			अनुवर्त;
		}

		mp = find_mode(arg);
		यदि (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			अनुवर्त;
		}

		cp = find_control(arg);
		यदि (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			अनुवर्त;
		}

		param = find_param(arg);
		यदि (param & param_need_arg) {
			++k;
		}

		विकल्प (param) {
#ifdef __linux__
		पक्ष param_पंक्ति:
			mode.c_पंक्ति = xatoul_sfx(argnext, stty_suffixes);
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
			set_speed_or_die(निवेश_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			भङ्ग;
		पक्ष param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			भङ्ग;
		स्वभाव:
			यदि (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			अन्यथा /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	यदि (stty_state & STTY_require_set_attr) {
		संरचना termios new_mode;

		यदि (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		स_रखो(&new_mode, 0, परिमाण(new_mode));
		यदि (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		यदि (स_भेद(&mode, &new_mode, परिमाण(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#यदि 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			यदि ((stty_state & STTY_speed_was_set)
			 || स_भेद(&mode, &new_mode, परिमाण(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	प्रत्यागम निकास_सफल;
}
