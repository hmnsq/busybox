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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE ((unsigned حرف) 0)
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
#إذا defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#endif
/* glibc-2.12.1 uses only VSWTC name */
#إذا defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#endif
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#إذا defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#endif
#إذا defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#endif

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#إذا defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#endif

#إذا defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#endif
#إذا defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#endif
#إذا !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#endif
#إذا defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#endif
#إذا defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#endif
#إذا defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#endif
#إذا defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#endif
#إذا defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#endif
#إذا defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#endif
#إذا defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#endif
#إذا defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#endif
#إذا defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#endif
#إذا defined(VSTATUS) && !defined(CSTATUS)
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
تعداد speed_setting {
	إدخال_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
تعداد {
	control, إدخال, output, local, combination
};
ساكن tcflag_t *get_ptr_to_tcflag(unsigned type, ثابت بنية termios *mode)
{
	ساكن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(بنية termios, c_cflag), /* control */
		offsetof(بنية termios, c_iflag), /*إدخال*/
		offsetof(بنية termios, c_oflag), /* output */
		offsetof(بنية termios, c_lflag)  /* local */
	};
	إذا (type <= local) {
		إرجاع (tcflag_t*) (((حرف*)mode) + tcflag_offsets[type]);
	}
	إرجاع عدم;
}

/* Flags for 'struct mode_info' */
#define SANE_SET 1              /* Set in 'sane' mode                  */
#define SANE_UNSET 2            /* Unset in 'sane' mode                */
#define REV 4                   /* Can be turned off by prepending '-' */
#define OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
بنية mode_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#إذا   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t mask;
#وإلاإذا (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t mask;
#وإلا
	ثابت tcflag_t mask;          /* Other bits to turn off for this mode */
#endif
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t bits;          /* Bits to set for this mode            */
};

تعداد {
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
#إذا IXANY
	IDX_decctlq,
#endif
#إذا TABDLY || OXTABS
	IDX_tabs,
#endif
#إذا XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#endif
};

#define MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساكن ثابت حرف mode_name[] ALIGN1 =
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
#إذا IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#إذا TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#إذا XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
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
#إذا CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   إدخال,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   إدخال,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   إدخال,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   إدخال,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    إدخال,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   إدخال,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    إدخال,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    إدخال,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    إدخال,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     إدخال,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    إدخال,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   إدخال,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_ENTRY("iuclc",    إدخال,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#إذا IXANY
	MI_ENTRY("ixany",    إدخال,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#إذا IMAXBEL
	MI_ENTRY("imaxbel",  إدخال,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#إذا IUTF8
	MI_ENTRY("iutf8",    إدخال,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#إذا OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#إذا ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#إذا ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#إذا ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#إذا OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#إذا OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#إذا NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#إذا CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#إذا TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#إذا BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#إذا VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#إذا FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#إذا TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#إذا ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#إذا ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#إذا ECHOKE
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

ساكن ثابت بنية mode_info mode_info[] ALIGN4 = {
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
#إذا IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#endif
#إذا TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#endif
#إذا XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#endif
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
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
#إذا CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#endif
	MI_ENTRY("ignbrk",   إدخال,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   إدخال,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   إدخال,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   إدخال,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    إدخال,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   إدخال,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    إدخال,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    إدخال,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    إدخال,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     إدخال,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    إدخال,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   إدخال,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_ENTRY("iuclc",    إدخال,       SANE_UNSET | REV,  IUCLC,      0 )
#endif
#إذا IXANY
	MI_ENTRY("ixany",    إدخال,       SANE_UNSET | REV,  IXANY,      0 )
#endif
#إذا IMAXBEL
	MI_ENTRY("imaxbel",  إدخال,       SANE_SET   | REV,  IMAXBEL,    0 )
#endif
#إذا IUTF8
	MI_ENTRY("iutf8",    إدخال,       SANE_UNSET | REV,  IUTF8,      0 )
#endif
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#endif
#إذا OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#endif
#إذا ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#endif
#إذا ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#endif
#إذا ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#endif
#إذا OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#endif
#إذا OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#endif
#إذا NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#endif
#إذا CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#endif

#إذا TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#endif

#إذا BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#endif
#إذا VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#endif
#إذا FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#endif
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#endif
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#endif
#إذا TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#endif
#إذا ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#endif
#إذا ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#endif
#إذا ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#endif
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ifdef EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#endif
};

تعداد {
	رقم_mode_info = ARRAY_SIZE(mode_info)
};


/* Control characters */
بنية control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t offset;   /* Offset in c_cc */
};

تعداد {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#إذا VEOL2
	CIDX_eol2,
#endif
#إذا VSWTCH
	CIDX_swtch,
#endif
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#إذا VDSUSP
	CIDX_dsusp,
#endif
#إذا VREPRINT
	CIDX_rprnt,
#endif
#إذا VWERASE
	CIDX_werase,
#endif
#إذا VLNEXT
	CIDX_lnext,
#endif
#إذا VFLUSHO
	CIDX_flush,
#endif
#إذا VSTATUS
	CIDX_status,
#endif
	CIDX_min,
	CIDX_time,
};

#define CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
ساكن ثابت حرف control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#إذا VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#إذا VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#إذا VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#إذا VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#إذا VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#إذا VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#undef CI_ENTRY
#define CI_ENTRY(n,s,o) { s, o },

ساكن ثابت بنية control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#endif
#إذا VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#endif
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#endif
#إذا VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#endif
#إذا VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#endif
#إذا VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#endif
#إذا VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#endif
#إذا VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#endif
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

تعداد {
	رقم_control_info = ARRAY_SIZE(control_info)
};


بنية globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	unsigned max_col;
	/* Current position, to know when to wrap */
	unsigned current_col;
} FIX_ALIASING;
#define G (*(بنية globals*)bb_common_bufsiz1)
#define INIT_G() افعل { \
	setup_common_bufsiz(); \
	G.device_name = bb_رسالة_standard_إدخال; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} بينما (0)

ساكن فراغ set_speed_or_die(تعداد speed_setting type, ثابت حرف *arg,
					بنية termios *mode)
{
	speed_t baud;

	baud = tty_قيمة_to_baud(xatou(arg));

	إذا (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	إذا (type != إدخال_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

ساكن NORETURN فراغ perror_on_device_and_die(ثابت حرف *fmt)
{
	bb_perror_رسالة_and_die(fmt, G.device_name);
}

ساكن فراغ perror_on_device(ثابت حرف *fmt)
{
	bb_perror_رسالة(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ساكن فراغ wrapf(ثابت حرف *message, ...)
{
	حرف buf[128];
	va_list args;
	unsigned buflen;

	va_start(args, message);
	buflen = vsnprintf(buf, حجم(buf), message, args);
	va_end(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	إذا (!buflen || buflen >= حجم(buf)) إرجاع;

	إذا (G.current_col > 0) {
		G.current_col++;
		إذا (buf[0] != '\n') {
			إذا (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} وإلا {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	إذا (buf[buflen-1] == '\n')
		G.current_col = 0;
}

ساكن فراغ newline(فراغ)
{
	إذا (G.current_col != 0)
		wrapf("\n");
}

#ifdef TIOCGWINSZ
ساكن فراغ set_window_size(صحيح rows, صحيح cols)
{
	بنية winsize win = { 0, 0, 0, 0 };

	إذا (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		إذا (errno != EINVAL) {
			اذهب bail;
		}
		memset(&win, 0, حجم(win));
	}

	إذا (rows >= 0)
		win.ws_row = rows;
	إذا (cols >= 0)
		win.ws_col = cols;

	إذا (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#endif

ساكن فراغ display_window_size(صحيح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	unsigned width, height;

	إذا (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		إذا ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} وإلا {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

ساكن ثابت بنية suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساكن ثابت بنية mode_info *find_mode(ثابت حرف *name)
{
	صحيح i = index_in_strings(mode_name, name);
	إرجاع i >= 0 ? &mode_info[i] : عدم;
}

ساكن ثابت بنية control_info *find_control(ثابت حرف *name)
{
	صحيح i = index_in_strings(control_name, name);
	إرجاع i >= 0 ? &control_info[i] : عدم;
}

تعداد {
	param_need_arg = 0x80,
	param_سطر    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_size    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساكن صحيح find_param(ثابت حرف *name)
{
	ساكن ثابت حرف params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	صحيح i = index_in_strings(params, name) + 1;
	إذا (i == 0)
		إرجاع 0;
	إذا (i != 5 && i != 6)
		i |= 0x80;
	إرجاع i;
}

ساكن صحيح recover_mode(ثابت حرف *arg, بنية termios *mode)
{
	صحيح i, n;
	unsigned chr;
	unsigned long iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	إذا (sscanf(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		إرجاع 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	لكل (i = 0; i < NCCS; ++i) {
		إذا (sscanf(arg, ":%x%n", &chr, &n) != 1)
			إرجاع 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	إذا (*arg != '\0')
		إرجاع 0;

	إرجاع 1;
}

ساكن فراغ display_recoverable(ثابت بنية termios *mode,
				صحيح UNUSED_PARAM dummy)
{
	صحيح i;
	printf("%lx:%lx:%lx:%lx",
		   (unsigned long) mode->c_iflag, (unsigned long) mode->c_oflag,
		   (unsigned long) mode->c_cflag, (unsigned long) mode->c_lflag);
	لكل (i = 0; i < NCCS; ++i)
		printf(":%x", (unsigned صحيح) mode->c_cc[i]);
	bb_putchar('\n');
}

ساكن فراغ display_speed(ثابت بنية termios *mode, صحيح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	unsigned long ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	إذا (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	إذا (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_قيمة(ispeed), tty_baud_to_قيمة(ospeed));
}

ساكن فراغ do_display(ثابت بنية termios *mode, صحيح all)
{
	صحيح i;
	tcflag_t *bitsp;
	unsigned long mask;
	صحيح prev_type = control;

	display_speed(mode, 1);
	إذا (all)
		display_window_size(1);
#ifdef __linux__
	wrapf("line = %u;\n", mode->c_سطر);
#وإلا
	newline();
#endif

	لكل (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#إذا VSWTCH == VSUSP
		إذا (i == CIDX_swtch)
			استمر;
#endif
		/* If eof uses the same slot as min, only print whichever applies */
#إذا VEOF == VMIN
		إذا (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			استمر;
		}
#endif
		ch = mode->c_cc[control_info[i].offset];
		إذا (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		وإلا
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#إذا VEOF == VMIN
	إذا ((mode->c_lflag & ICANON) == 0)
#endif
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	لكل (i = 0; i < رقم_mode_info; ++i) {
		إذا (mode_info[i].flags & OMIT)
			استمر;
		إذا (mode_info[i].type != prev_type) {
			newline();
			prev_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		إذا ((*bitsp & mask) == mode_info[i].bits) {
			إذا (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} وإلا {
			إذا ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

ساكن فراغ sane_mode(بنية termios *mode)
{
	صحيح i;

	لكل (i = 0; i < رقم_control_info; ++i) {
#إذا VMIN == VEOF
		إذا (i == CIDX_min)
			اكسر;
#endif
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	لكل (i = 0; i < رقم_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		إذا (!bitsp)
			استمر;
		val = *bitsp & ~((unsigned long)mode_info[i].mask);
		إذا (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} وإلا
		إذا (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

ساكن فراغ set_mode(ثابت بنية mode_info *info, صحيح reversed,
					بنية termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	إذا (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		إذا (reversed)
			*bitsp = val & ~info->bits;
		وإلا
			*bitsp = val | info->bits;
		إرجاع;
	}

	/* !bitsp - it's a "combination" mode */
	إذا (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		إذا (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} وإلا إذا (info == &mode_info[IDX_oddp]) {
		إذا (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} وإلا إذا (info == &mode_info[IDX_nl]) {
		إذا (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} وإلا {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			إذا (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} وإلا إذا (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} وإلا إذا (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} وإلا إذا (info == &mode_info[IDX_cbreak]) {
		إذا (reversed)
			mode->c_lflag |= ICANON;
		وإلا
			mode->c_lflag &= ~ICANON;
	} وإلا إذا (info == &mode_info[IDX_pass8]) {
		إذا (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} وإلا {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} وإلا إذا (info == &mode_info[IDX_litout]) {
		إذا (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} وإلا {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} وإلا إذا (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		إذا ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#إذا VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#endif
#إذا VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#endif
		} وإلا {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#إذا IXANY
	وإلا إذا (info == &mode_info[IDX_decctlq]) {
		إذا (reversed)
			mode->c_iflag |= IXANY;
		وإلا
			mode->c_iflag &= ~IXANY;
	}
#endif
#إذا TABDLY
	وإلا إذا (info == &mode_info[IDX_tabs]) {
		إذا (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		وإلا
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#endif
#إذا OXTABS
	وإلا إذا (info == &mode_info[IDX_tabs]) {
		إذا (reversed)
			mode->c_oflag |= OXTABS;
		وإلا
			mode->c_oflag &= ~OXTABS;
	}
#endif
#إذا XCASE && IUCLC && OLCUC
	وإلا إذا (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		إذا (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} وإلا {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#endif
	وإلا إذا (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} وإلا إذا (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		إذا (IXANY) mode->c_iflag &= ~IXANY;
	}
}

ساكن فراغ set_control_char_or_die(ثابت بنية control_info *info,
			ثابت حرف *arg, بنية termios *mode)
{
	unsigned حرف قيمة_2;

	إذا (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		قيمة_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	وإلا إذا (arg[0] == '\0' || arg[1] == '\0')
		قيمة_2 = arg[0];
	وإلا إذا (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		قيمة_2 = _POSIX_VDISABLE;
	وإلا إذا (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		قيمة_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		إذا (arg[1] == '?')
			قيمة_2 = 127;
	} وإلا
		قيمة_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = قيمة_2;
}

#define STTY_require_set_attr   (1 << 0)
#define STTY_speed_was_set      (1 << 1)
#define STTY_verbose_output     (1 << 2)
#define STTY_recoverable_output (1 << 3)
#define STTY_noargs             (1 << 4)

صحيح stty_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stty_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	بنية termios mode;
	فراغ (*output_func)(ثابت بنية termios *, صحيح);
	ثابت حرف *file_name = عدم;
	صحيح display_all = 0;
	صحيح stty_state;
	صحيح k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	بينما (argv[++k]) {
		ثابت بنية mode_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			صحيح i;
			mp = find_mode(arg+1);
			إذا (mp) {
				إذا (!(mp->flags & REV))
					اذهب invalid_argument;
				stty_state &= ~STTY_noargs;
				استمر;
			}
			/* It is an option - parse it */
			i = 0;
			بينما (arg[++i]) {
				بدل (arg[i]) {
				حالة 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					اكسر;
				حالة 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					اكسر;
				حالة 'F':
					إذا (file_name)
						bb_simple_error_رسالة_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					إذا (!file_name[0]) { /* nope, "-F device" */
						صحيح p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						إذا (!file_name)
							bb_error_رسالة_and_die(bb_رسالة_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						بينما (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					اذهب end_option;
				افتراضي:
					اذهب invalid_argument;
				}
			}
 end_option:
			استمر;
		}

		mp = find_mode(arg);
		إذا (mp) {
			stty_state &= ~STTY_noargs;
			استمر;
		}

		cp = find_control(arg);
		إذا (cp) {
			إذا (!argnext)
				bb_error_رسالة_and_die(bb_رسالة_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			استمر;
		}

		param = find_param(arg);
		إذا (param & param_need_arg) {
			إذا (!argnext)
				bb_error_رسالة_and_die(bb_رسالة_requires_arg, arg);
			++k;
		}

		بدل (param) {
#ifdef __linux__
		حالة param_سطر:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			اكسر;
# endif /* else fall-through */
#endif
#ifdef TIOCGWINSZ
		حالة param_rows:
		حالة param_cols:
		حالة param_columns:
			xatoul_range_sfx(argnext, 1, INT_MAX, stty_suffixes);
			اكسر;
		حالة param_size:
#endif
		حالة param_speed:
			اكسر;
		حالة param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(إدخال_speed, argnext, &mode);
			اكسر;
		حالة param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			اكسر;
		افتراضي:
			إذا (recover_mode(arg, &mode) == 1) اكسر;
			إذا (tty_قيمة_to_baud(xatou(arg)) != (speed_t) -1) اكسر;
 invalid_argument:
			bb_error_رسالة_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_رسالة_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_رسالة_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	إذا (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&mode, 0, حجم(mode));
	إذا (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	إذا (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		إرجاع EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	بينما (argv[++k]) {
		ثابت بنية mode_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			mp = find_mode(arg+1);
			إذا (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			استمر;
		}

		mp = find_mode(arg);
		إذا (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			استمر;
		}

		cp = find_control(arg);
		إذا (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			استمر;
		}

		param = find_param(arg);
		إذا (param & param_need_arg) {
			++k;
		}

		بدل (param) {
#ifdef __linux__
		حالة param_سطر:
			mode.c_سطر = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			اكسر;
#endif
#ifdef TIOCGWINSZ
		حالة param_cols:
		حالة param_columns:
			set_window_size(-1, xatoul_sfx(argnext, stty_suffixes));
			اكسر;
		حالة param_size:
			display_window_size(0);
			اكسر;
		حالة param_rows:
			set_window_size(xatoul_sfx(argnext, stty_suffixes), -1);
			اكسر;
#endif
		حالة param_speed:
			display_speed(&mode, 0);
			اكسر;
		حالة param_ispeed:
			set_speed_or_die(إدخال_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			اكسر;
		حالة param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			اكسر;
		افتراضي:
			إذا (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			وإلا /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	إذا (stty_state & STTY_require_set_attr) {
		بنية termios new_mode;

		إذا (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_mode, 0, حجم(new_mode));
		إذا (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		إذا (memcmp(&mode, &new_mode, حجم(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#إذا 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			إذا ((stty_state & STTY_speed_was_set)
			 || memcmp(&mode, &new_mode, حجم(mode)) != 0)
#endif
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	إرجاع EXIT_SUCCESS;
}
