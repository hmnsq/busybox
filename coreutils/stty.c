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

#که_نه_تعریف_شوی _POSIX_VDISABLE
# define _POSIX_VDISABLE ((بینښه حرف) 0)
#د_که_پای

#تعریف_کړه Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#که_نه_تعریف_شوی CINTR
# define CINTR Control('c')
#د_که_پای
#که_نه_تعریف_شوی CQUIT
# define CQUIT 28
#د_که_پای
#که_نه_تعریف_شوی CERASE
# define CERASE 127
#د_که_پای
#که_نه_تعریف_شوی CKILL
# define CKILL Control('u')
#د_که_پای
#که_نه_تعریف_شوی CEOF
# define CEOF Control('d')
#د_که_پای
#که_نه_تعریف_شوی CEOL
# define CEOL _POSIX_VDISABLE
#د_که_پای
#که_نه_تعریف_شوی CSTART
# define CSTART Control('q')
#د_که_پای
#که_نه_تعریف_شوی CSTOP
# define CSTOP Control('s')
#د_که_پای
#که_نه_تعریف_شوی CSUSP
# define CSUSP Control('z')
#د_که_پای
#إذا defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#د_که_پای
/* glibc-2.12.1 uses only VSWTC name */
#إذا defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#د_که_پای
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#إذا defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#د_که_پای
#إذا defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#د_که_پای

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#إذا defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#د_که_پای

#إذا defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#د_که_پای
#إذا defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#د_که_پای
#إذا !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#د_که_پای
#إذا defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#د_که_پای
#إذا defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#د_که_پای
#إذا defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#د_که_پای
#إذا defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#د_که_پای
#إذا defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#د_که_پای
#إذا defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#د_که_پای
#إذا defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#د_که_پای
#إذا defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#د_که_پای
#إذا defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#د_که_پای
#إذا defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#د_که_پای

/* Save us from #ifdef forest plague */
#که_نه_تعریف_شوی BSDLY
# define BSDLY 0
#د_که_پای
#که_نه_تعریف_شوی CIBAUD
# define CIBAUD 0
#د_که_پای
#که_نه_تعریف_شوی CRDLY
# define CRDLY 0
#د_که_پای
#که_نه_تعریف_شوی CMSPAR
# define CMSPAR 0
#د_که_پای
#که_نه_تعریف_شوی CRTSCTS
# define CRTSCTS 0
#د_که_پای
#که_نه_تعریف_شوی ECHOCTL
# define ECHOCTL 0
#د_که_پای
#که_نه_تعریف_شوی ECHOKE
# define ECHOKE 0
#د_که_پای
#که_نه_تعریف_شوی ECHOPRT
# define ECHOPRT 0
#د_که_پای
#که_نه_تعریف_شوی FFDLY
# define FFDLY 0
#د_که_پای
#که_نه_تعریف_شوی IEXTEN
# define IEXTEN 0
#د_که_پای
#که_نه_تعریف_شوی IMAXBEL
# define IMAXBEL 0
#د_که_پای
#که_نه_تعریف_شوی IUCLC
# define IUCLC 0
#د_که_پای
#که_نه_تعریف_شوی IXANY
# define IXANY 0
#د_که_پای
#که_نه_تعریف_شوی NLDLY
# define NLDLY 0
#د_که_پای
#که_نه_تعریف_شوی OCRNL
# define OCRNL 0
#د_که_پای
#که_نه_تعریف_شوی OFDEL
# define OFDEL 0
#د_که_پای
#که_نه_تعریف_شوی OFILL
# define OFILL 0
#د_که_پای
#که_نه_تعریف_شوی OLCUC
# define OLCUC 0
#د_که_پای
#که_نه_تعریف_شوی ONLCR
# define ONLCR 0
#د_که_پای
#که_نه_تعریف_شوی ONLRET
# define ONLRET 0
#د_که_پای
#که_نه_تعریف_شوی ONOCR
# define ONOCR 0
#د_که_پای
#که_نه_تعریف_شوی OXTABS
# define OXTABS 0
#د_که_پای
#که_نه_تعریف_شوی TABDLY
# define TABDLY 0
#د_که_پای
#که_نه_تعریف_شوی TAB1
# define TAB1 0
#د_که_پای
#که_نه_تعریف_شوی TAB2
# define TAB2 0
#د_که_پای
#که_نه_تعریف_شوی TOSTOP
# define TOSTOP 0
#د_که_پای
#که_نه_تعریف_شوی VDSUSP
# define VDSUSP 0
#د_که_پای
#که_نه_تعریف_شوی VEOL2
# define VEOL2 0
#د_که_پای
#که_نه_تعریف_شوی VFLUSHO
# define VFLUSHO 0
#د_که_پای
#که_نه_تعریف_شوی VLNEXT
# define VLNEXT 0
#د_که_پای
#که_نه_تعریف_شوی VREPRINT
# define VREPRINT 0
#د_که_پای
#که_نه_تعریف_شوی VSTATUS
# define VSTATUS 0
#د_که_پای
#که_نه_تعریف_شوی VSWTCH
# define VSWTCH 0
#د_که_پای
#که_نه_تعریف_شوی VTDLY
# define VTDLY 0
#د_که_پای
#که_نه_تعریف_شوی VWERASE
# define VWERASE 0
#د_که_پای
#که_نه_تعریف_شوی XCASE
# define XCASE 0
#د_که_پای
#که_نه_تعریف_شوی IUTF8
# define IUTF8 0
#د_که_پای

/* Which speeds to set */
تعداد چټکتیا_setting {
	ننوت_چټکتیا, output_چټکتیا, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
تعداد {
	control, ننوت, output, local, combination
};
ساكن tcflag_t *get_ptr_منزل_tcflag(بینښه type, ثابت بنية termios *ډېرپېښ)
{
	ساكن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(بنية termios, c_cflag), /* control */
		offsetof(بنية termios, c_iflag), /*ننوت*/
		offsetof(بنية termios, c_oflag), /* output */
		offsetof(بنية termios, c_lflag)  /* local */
	};
	إذا (type <= local) {
		أرجع (tcflag_t*) (((حرف*)ډېرپېښ) + tcflag_offsets[type]);
	}
	أرجع عدم;
}

/* Flags for 'struct mode_info' */
#تعریف_کړه SANE_سېټ 1              /* Set in 'sane' mode                  */
#تعریف_کړه SANE_UNSET 2            /* Unset in 'sane' mode                */
#تعریف_کړه REV 4                   /* Can be turned off by prepending '-' */
#تعریف_کړه OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
بنية ډېرپېښ_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#إذا   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t ماسک;
#وإلاإذا (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t ماسک;
#وإلا
	ثابت tcflag_t ماسک;          /* Other bits to turn off for this mode */
#د_که_پای
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t بټونه;          /* Bits to set for this mode            */
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
#د_که_پای
#إذا TABDLY || OXTABS
	IDX_tabs,
#د_که_پای
#إذا XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#د_که_پای
};

#تعریف_کړه MI_ثبت(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساكن ثابت حرف ډېرپېښ_name[] ALIGN1 =
	MI_ثبت("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ثبت("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ثبت("ek",       combination, OMIT,              0,          0 )
	MI_ثبت("sane",     combination, OMIT,              0,          0 )
	MI_ثبت("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ثبت("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("crt",      combination, OMIT,              0,          0 )
	MI_ثبت("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_ثبت("decctlq",  combination, REV        | OMIT, 0,          0 )
#د_که_پای
#إذا TABDLY || OXTABS
	MI_ثبت("tabs",     combination, REV        | OMIT, 0,          0 )
#د_که_پای
#إذا XCASE && IUCLC && OLCUC
	MI_ثبت("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("LCASE",    combination, REV        | OMIT, 0,          0 )
#د_که_پای
	MI_ثبت("parenb",   control,     REV,               PARENB,     0 )
	MI_ثبت("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_ثبت("cmspar",   control,     REV,               CMSPAR,     0 )
#د_که_پای
	MI_ثبت("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ثبت("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ثبت("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ثبت("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ثبت("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ثبت("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ثبت("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ثبت("cread",    control,     SANE_سېټ   | REV,  CREAD,      0 )
	MI_ثبت("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_ثبت("crtscts",  control,     REV,               CRTSCTS,    0 )
#د_که_پای
	MI_ثبت("ignbrk",   ننوت,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ثبت("brkint",   ننوت,       SANE_سېټ   | REV,  BRKINT,     0 )
	MI_ثبت("ignpar",   ننوت,       REV,               IGNPAR,     0 )
	MI_ثبت("parmrk",   ننوت,       REV,               PARMRK,     0 )
	MI_ثبت("inpck",    ننوت,       REV,               INPCK,      0 )
	MI_ثبت("istrip",   ننوت,       REV,               ISTRIP,     0 )
	MI_ثبت("inlcr",    ننوت,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ثبت("igncr",    ننوت,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ثبت("icrnl",    ننوت,       SANE_سېټ   | REV,  ICRNL,      0 )
	MI_ثبت("ixon",     ننوت,       REV,               IXON,       0 )
	MI_ثبت("ixoff",    ننوت,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ثبت("tandem",   ننوت,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_ثبت("iuclc",    ننوت,       SANE_UNSET | REV,  IUCLC,      0 )
#د_که_پای
#إذا IXANY
	MI_ثبت("ixany",    ننوت,       SANE_UNSET | REV,  IXANY,      0 )
#د_که_پای
#إذا IMAXBEL
	MI_ثبت("imaxbel",  ننوت,       SANE_سېټ   | REV,  IMAXBEL,    0 )
#د_که_پای
#إذا IUTF8
	MI_ثبت("iutf8",    ننوت,       SANE_UNSET | REV,  IUTF8,      0 )
#د_که_پای
	MI_ثبت("opost",    output,      SANE_سېټ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_ثبت("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#د_که_پای
#إذا OCRNL
	MI_ثبت("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#د_که_پای
#إذا ONLCR
	MI_ثبت("onlcr",    output,      SANE_سېټ   | REV,  ONLCR,      0 )
#د_که_پای
#إذا ONOCR
	MI_ثبت("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#د_که_پای
#إذا ONLRET
	MI_ثبت("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#د_که_پای
#إذا OFILL
	MI_ثبت("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#د_که_پای
#إذا OFDEL
	MI_ثبت("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#د_که_پای
#إذا NLDLY
	MI_ثبت("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ثبت("nl0",      output,      SANE_سېټ,          NL0,     NLDLY)
#د_که_پای
#إذا CRDLY
	MI_ثبت("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ثبت("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ثبت("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ثبت("cr0",      output,      SANE_سېټ,          CR0,     CRDLY)
#د_که_پای

#إذا TABDLY
	MI_ثبت("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_ثبت("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_ثبت("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ثبت("tab0",     output,      SANE_سېټ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_ثبت("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#د_که_پای

#إذا BSDLY
	MI_ثبت("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ثبت("bs0",      output,      SANE_سېټ,          BS0,     BSDLY)
#د_که_پای
#إذا VTDLY
	MI_ثبت("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ثبت("vt0",      output,      SANE_سېټ,          VT0,     VTDLY)
#د_که_پای
#إذا FFDLY
	MI_ثبت("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ثبت("ff0",      output,      SANE_سېټ,          FF0,     FFDLY)
#د_که_پای
	MI_ثبت("isig",     local,       SANE_سېټ   | REV,  ISIG,       0 )
	MI_ثبت("icanon",   local,       SANE_سېټ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_ثبت("iexten",   local,       SANE_سېټ   | REV,  IEXTEN,     0 )
#د_که_پای
	MI_ثبت("echo",     local,       SANE_سېټ   | REV,  ECHO,       0 )
	MI_ثبت("echoe",    local,       SANE_سېټ   | REV,  ECHOE,      0 )
	MI_ثبت("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ثبت("echok",    local,       SANE_سېټ   | REV,  ECHOK,      0 )
	MI_ثبت("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ثبت("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_ثبت("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#د_که_پای
#إذا TOSTOP
	MI_ثبت("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#د_که_پای
#إذا ECHOPRT
	MI_ثبت("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ثبت("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#د_که_پای
#إذا ECHOCTL
	MI_ثبت("echoctl",  local,       SANE_سېټ   | REV,  ECHOCTL,    0 )
	MI_ثبت("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#د_که_پای
#إذا ECHOKE
	MI_ثبت("echoke",   local,       SANE_سېټ   | REV,  ECHOKE,     0 )
	MI_ثبت("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#د_که_پای
	MI_ثبت("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#که_تعریف_شوی EXTPROC
	MI_ثبت("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#د_که_پای
	;

#تعریف_لغوه MI_ثبت
#تعریف_کړه MI_ثبت(N,T,F,B,M) { T, F, M, B },

ساكن ثابت بنية ډېرپېښ_info ډېرپېښ_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_ثبت("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ثبت("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ثبت("ek",       combination, OMIT,              0,          0 )
	MI_ثبت("sane",     combination, OMIT,              0,          0 )
	MI_ثبت("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ثبت("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ثبت("crt",      combination, OMIT,              0,          0 )
	MI_ثبت("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_ثبت("decctlq",  combination, REV        | OMIT, 0,          0 )
#د_که_پای
#إذا TABDLY || OXTABS
	MI_ثبت("tabs",     combination, REV        | OMIT, 0,          0 )
#د_که_پای
#إذا XCASE && IUCLC && OLCUC
	MI_ثبت("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ثبت("LCASE",    combination, REV        | OMIT, 0,          0 )
#د_که_پای
	MI_ثبت("parenb",   control,     REV,               PARENB,     0 )
	MI_ثبت("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_ثبت("cmspar",   control,     REV,               CMSPAR,     0 )
#د_که_پای
	MI_ثبت("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ثبت("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ثبت("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ثبت("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ثبت("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ثبت("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ثبت("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ثبت("cread",    control,     SANE_سېټ   | REV,  CREAD,      0 )
	MI_ثبت("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_ثبت("crtscts",  control,     REV,               CRTSCTS,    0 )
#د_که_پای
	MI_ثبت("ignbrk",   ننوت,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ثبت("brkint",   ننوت,       SANE_سېټ   | REV,  BRKINT,     0 )
	MI_ثبت("ignpar",   ننوت,       REV,               IGNPAR,     0 )
	MI_ثبت("parmrk",   ننوت,       REV,               PARMRK,     0 )
	MI_ثبت("inpck",    ننوت,       REV,               INPCK,      0 )
	MI_ثبت("istrip",   ننوت,       REV,               ISTRIP,     0 )
	MI_ثبت("inlcr",    ننوت,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ثبت("igncr",    ننوت,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ثبت("icrnl",    ننوت,       SANE_سېټ   | REV,  ICRNL,      0 )
	MI_ثبت("ixon",     ننوت,       REV,               IXON,       0 )
	MI_ثبت("ixoff",    ننوت,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ثبت("tandem",   ننوت,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_ثبت("iuclc",    ننوت,       SANE_UNSET | REV,  IUCLC,      0 )
#د_که_پای
#إذا IXANY
	MI_ثبت("ixany",    ننوت,       SANE_UNSET | REV,  IXANY,      0 )
#د_که_پای
#إذا IMAXBEL
	MI_ثبت("imaxbel",  ننوت,       SANE_سېټ   | REV,  IMAXBEL,    0 )
#د_که_پای
#إذا IUTF8
	MI_ثبت("iutf8",    ننوت,       SANE_UNSET | REV,  IUTF8,      0 )
#د_که_پای
	MI_ثبت("opost",    output,      SANE_سېټ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_ثبت("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#د_که_پای
#إذا OCRNL
	MI_ثبت("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#د_که_پای
#إذا ONLCR
	MI_ثبت("onlcr",    output,      SANE_سېټ   | REV,  ONLCR,      0 )
#د_که_پای
#إذا ONOCR
	MI_ثبت("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#د_که_پای
#إذا ONLRET
	MI_ثبت("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#د_که_پای
#إذا OFILL
	MI_ثبت("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#د_که_پای
#إذا OFDEL
	MI_ثبت("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#د_که_پای
#إذا NLDLY
	MI_ثبت("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ثبت("nl0",      output,      SANE_سېټ,          NL0,     NLDLY)
#د_که_پای
#إذا CRDLY
	MI_ثبت("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ثبت("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ثبت("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ثبت("cr0",      output,      SANE_سېټ,          CR0,     CRDLY)
#د_که_پای

#إذا TABDLY
	MI_ثبت("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_ثبت("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_ثبت("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ثبت("tab0",     output,      SANE_سېټ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_ثبت("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#د_که_پای

#إذا BSDLY
	MI_ثبت("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ثبت("bs0",      output,      SANE_سېټ,          BS0,     BSDLY)
#د_که_پای
#إذا VTDLY
	MI_ثبت("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ثبت("vt0",      output,      SANE_سېټ,          VT0,     VTDLY)
#د_که_پای
#إذا FFDLY
	MI_ثبت("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ثبت("ff0",      output,      SANE_سېټ,          FF0,     FFDLY)
#د_که_پای
	MI_ثبت("isig",     local,       SANE_سېټ   | REV,  ISIG,       0 )
	MI_ثبت("icanon",   local,       SANE_سېټ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_ثبت("iexten",   local,       SANE_سېټ   | REV,  IEXTEN,     0 )
#د_که_پای
	MI_ثبت("echo",     local,       SANE_سېټ   | REV,  ECHO,       0 )
	MI_ثبت("echoe",    local,       SANE_سېټ   | REV,  ECHOE,      0 )
	MI_ثبت("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ثبت("echok",    local,       SANE_سېټ   | REV,  ECHOK,      0 )
	MI_ثبت("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ثبت("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_ثبت("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#د_که_پای
#إذا TOSTOP
	MI_ثبت("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#د_که_پای
#إذا ECHOPRT
	MI_ثبت("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ثبت("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#د_که_پای
#إذا ECHOCTL
	MI_ثبت("echoctl",  local,       SANE_سېټ   | REV,  ECHOCTL,    0 )
	MI_ثبت("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#د_که_پای
#إذا ECHOKE
	MI_ثبت("echoke",   local,       SANE_سېټ   | REV,  ECHOKE,     0 )
	MI_ثبت("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#د_که_پای
	MI_ثبت("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#که_تعریف_شوی EXTPROC
	MI_ثبت("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#د_که_پای
};

تعداد {
	عدد_ډېرپېښ_info = ARRAY_اندازه(ډېرپېښ_info)
};


/* Control characters */
بنية control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t دننه_واټن;   /* Offset in c_cc */
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
#د_که_پای
#إذا VSWTCH
	CIDX_swtch,
#د_که_پای
	CIDX_پیل,
	CIDX_stop,
	CIDX_susp,
#إذا VDSUSP
	CIDX_dsusp,
#د_که_پای
#إذا VREPRINT
	CIDX_rprnt,
#د_که_پای
#إذا VWERASE
	CIDX_werase,
#د_که_پای
#إذا VLNEXT
	CIDX_lnext,
#د_که_پای
#إذا VFLUSHO
	CIDX_flush,
#د_که_پای
#إذا VSTATUS
	CIDX_حالت,
#د_که_پای
	CIDX_min,
	CIDX_وخت,
};

#تعریف_کړه CI_ثبت(n,s,o) n "\0"

/* Name given on command line */
ساكن ثابت حرف control_name[] ALIGN1 =
	CI_ثبت("intr",     CINTR,   VINTR   )
	CI_ثبت("quit",     CQUIT,   VQUIT   )
	CI_ثبت("erase",    CERASE,  VERASE  )
	CI_ثبت("kill",     CKILL,   VKILL   )
	CI_ثبت("eof",      CEOF,    VEOF    )
	CI_ثبت("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_ثبت("eol2",     CEOL2,   VEOL2   )
#د_که_پای
#إذا VSWTCH
	CI_ثبت("swtch",    CSWTCH,  VSWTCH  )
#د_که_پای
	CI_ثبت("start",    CSTART,  VSTART  )
	CI_ثبت("stop",     CSTOP,   VSTOP   )
	CI_ثبت("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_ثبت("dsusp",    CDSUSP,  VDSUSP  )
#د_که_پای
#إذا VREPRINT
	CI_ثبت("rprnt",    CRPRNT,  VREPRINT)
#د_که_پای
#إذا VWERASE
	CI_ثبت("werase",   CWERASE, VWERASE )
#د_که_پای
#إذا VLNEXT
	CI_ثبت("lnext",    CLNEXT,  VLNEXT  )
#د_که_پای
#إذا VFLUSHO
	CI_ثبت("flush",    CFLUSHO, VFLUSHO )
#د_که_پای
#إذا VSTATUS
	CI_ثبت("status",   CSTATUS, VSTATUS )
#د_که_پای
	/* These must be last because of the display routines */
	CI_ثبت("min",      1,       VMIN    )
	CI_ثبت("time",     0,       VTIME   )
	;

#تعریف_لغوه CI_ثبت
#تعریف_کړه CI_ثبت(n,s,o) { s, o },

ساكن ثابت بنية control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ثبت("intr",     CINTR,   VINTR   )
	CI_ثبت("quit",     CQUIT,   VQUIT   )
	CI_ثبت("erase",    CERASE,  VERASE  )
	CI_ثبت("kill",     CKILL,   VKILL   )
	CI_ثبت("eof",      CEOF,    VEOF    )
	CI_ثبت("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_ثبت("eol2",     CEOL2,   VEOL2   )
#د_که_پای
#إذا VSWTCH
	CI_ثبت("swtch",    CSWTCH,  VSWTCH  )
#د_که_پای
	CI_ثبت("start",    CSTART,  VSTART  )
	CI_ثبت("stop",     CSTOP,   VSTOP   )
	CI_ثبت("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_ثبت("dsusp",    CDSUSP,  VDSUSP  )
#د_که_پای
#إذا VREPRINT
	CI_ثبت("rprnt",    CRPRNT,  VREPRINT)
#د_که_پای
#إذا VWERASE
	CI_ثبت("werase",   CWERASE, VWERASE )
#د_که_پای
#إذا VLNEXT
	CI_ثبت("lnext",    CLNEXT,  VLNEXT  )
#د_که_پای
#إذا VFLUSHO
	CI_ثبت("flush",    CFLUSHO, VFLUSHO )
#د_که_پای
#إذا VSTATUS
	CI_ثبت("status",   CSTATUS, VSTATUS )
#د_که_پای
	/* These must be last because of the display routines */
	CI_ثبت("min",      1,       VMIN    )
	CI_ثبت("time",     0,       VTIME   )
};

تعداد {
	عدد_control_info = ARRAY_اندازه(control_info)
};


بنية globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	بینښه max_ستون;
	/* Current position, to know when to wrap */
	بینښه current_ستون;
} FIX_ALIASING;
#تعریف_کړه G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کړه INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	G.device_name = bb_پیغام_standard_ننوت; \
	G.max_ستون = 80; \
	G.current_ستون = 0; /* we are noexec, must clear */ \
} طالما (0)

ساكن فراغ سېټ_چټکتیا_or_die(تعداد چټکتیا_setting type, ثابت حرف *arg,
					بنية termios *ډېرپېښ)
{
	چټکتیا_t baud;

	baud = tty_ارزښت_منزل_baud(xatou(arg));

	إذا (type != output_چټکتیا) {     /* either input or both */
		cfsetispeed(ډېرپېښ, baud);
	}
	إذا (type != ننوت_چټکتیا) {      /* either output or both */
		cfsetospeed(ډېرپېښ, baud);
	}
}

ساكن NORETURN فراغ perror_on_device_and_die(ثابت حرف *fmt)
{
	bb_perror_پیغام_and_die(fmt, G.device_name);
}

ساكن فراغ perror_on_device(ثابت حرف *fmt)
{
	bb_perror_پیغام(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ساكن فراغ wrapf(ثابت حرف *پیغام_2, ...)
{
	حرف buf[128];
	va_list دلیلونه;
	بینښه buflen;

	va_پیل(دلیلونه, پیغام_2);
	buflen = vsnprintf(buf, حجم(buf), پیغام_2, دلیلونه);
	va_پای_ټکی(دلیلونه);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	إذا (!buflen || buflen >= حجم(buf)) أرجع;

	إذا (G.current_ستون > 0) {
		G.current_ستون++;
		إذا (buf[0] != '\n') {
			إذا (G.current_ستون + buflen >= G.max_ستون) {
				G.current_ستون = 0;
				bb_putchar('\n');
			} وإلا {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_ستون += buflen;
	إذا (buf[buflen-1] == '\n')
		G.current_ستون = 0;
}

ساكن فراغ newline(فراغ)
{
	إذا (G.current_ستون != 0)
		wrapf("\n");
}

#که_تعریف_شوی TIOCGWINSZ
ساكن فراغ سېټ_کړکۍ_اندازه(صحيح لیکې, صحيح ستون_شمېر)
{
	بنية winsize win = { 0, 0, 0, 0 };

	إذا (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		إذا (errno != EINVAL) {
			اذهب bail;
		}
		memset(&win, 0, حجم(win));
	}

	إذا (لیکې >= 0)
		win.ws_لیکه = لیکې;
	إذا (ستون_شمېر >= 0)
		win.ws_ستون = ستون_شمېر;

	إذا (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#د_که_پای

ساكن فراغ display_کړکۍ_اندازه(صحيح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	بینښه پلنوالی, لوړوالی;

	إذا (get_terminal_پلنوالی_لوړوالی(STDIN_FILENO, &پلنوالی, &لوړوالی)) {
		إذا ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} وإلا {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				لوړوالی, پلنوالی);
	}
}

ساكن ثابت بنية suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساكن ثابت بنية ډېرپېښ_info *پیدا_کړه_ډېرپېښ(ثابت حرف *name)
{
	صحيح i = شاخص_دننه_مزي_ګانې(ډېرپېښ_name, name);
	أرجع i >= 0 ? &ډېرپېښ_info[i] : عدم;
}

ساكن ثابت بنية control_info *پیدا_کړه_control(ثابت حرف *name)
{
	صحيح i = شاخص_دننه_مزي_ګانې(control_name, name);
	أرجع i >= 0 ? &control_info[i] : عدم;
}

تعداد {
	param_need_arg = 0x80,
	param_کرښه    = 1 | 0x80,
	param_لیکې    = 2 | 0x80,
	param_ستون_شمېر    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_اندازه    = 5,
	param_چټکتیا   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساكن صحيح پیدا_کړه_param(ثابت حرف *name)
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
	صحيح i = شاخص_دننه_مزي_ګانې(params, name) + 1;
	إذا (i == 0)
		أرجع 0;
	إذا (i != 5 && i != 6)
		i |= 0x80;
	أرجع i;
}

ساكن صحيح recover_ډېرپېښ(ثابت حرف *arg, بنية termios *ډېرپېښ)
{
	صحيح i, n;
	بینښه chr;
	بینښه اوږد iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	إذا (له_مزي_ولوله(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		أرجع 0;
	ډېرپېښ->c_iflag = iflag;
	ډېرپېښ->c_oflag = oflag;
	ډېرپېښ->c_cflag = cflag;
	ډېرپېښ->c_lflag = lflag;
	arg += n;
	لكل (i = 0; i < NCCS; ++i) {
		إذا (له_مزي_ولوله(arg, ":%x%n", &chr, &n) != 1)
			أرجع 0;
		ډېرپېښ->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	إذا (*arg != '\0')
		أرجع 0;

	أرجع 1;
}

ساكن فراغ display_recoverable(ثابت بنية termios *ډېرپېښ,
				صحيح ناکارول_شوی_PARAM dummy)
{
	صحيح i;
	چاپ_کړه("%lx:%lx:%lx:%lx",
		   (بینښه اوږد) ډېرپېښ->c_iflag, (بینښه اوږد) ډېرپېښ->c_oflag,
		   (بینښه اوږد) ډېرپېښ->c_cflag, (بینښه اوږد) ډېرپېښ->c_lflag);
	لكل (i = 0; i < NCCS; ++i)
		چاپ_کړه(":%x", (بینښه صحيح) ډېرپېښ->c_cc[i]);
	bb_putchar('\n');
}

ساكن فراغ display_چټکتیا(ثابت بنية termios *ډېرپېښ, صحيح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	بینښه اوږد ispeed, ospeed;

	ispeed = cfgetispeed(ډېرپېښ);
	ospeed = cfgetospeed(ډېرپېښ);
	إذا (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	إذا (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_منزل_ارزښت(ispeed), tty_baud_منزل_ارزښت(ospeed));
}

ساكن فراغ do_display(ثابت بنية termios *ډېرپېښ, صحيح all)
{
	صحيح i;
	tcflag_t *bitsp;
	بینښه اوږد ماسک;
	صحيح prev_type = control;

	display_چټکتیا(ډېرپېښ, 1);
	إذا (all)
		display_کړکۍ_اندازه(1);
#که_تعریف_شوی __linux__
	wrapf("line = %u;\n", ډېرپېښ->c_کرښه);
#وإلا
	newline();
#د_که_پای

	لكل (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#إذا VSWTCH == VSUSP
		إذا (i == CIDX_swtch)
			استمر;
#د_که_پای
		/* If eof uses the same slot as min, only print whichever applies */
#إذا VEOF == VMIN
		إذا (!(ډېرپېښ->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			استمر;
		}
#د_که_پای
		ch = ډېرپېښ->c_cc[control_info[i].دننه_واټن];
		إذا (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		وإلا
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#إذا VEOF == VMIN
	إذا ((ډېرپېښ->c_lflag & ICANON) == 0)
#د_که_پای
		wrapf("min = %u; time = %u;", ډېرپېښ->c_cc[VMIN], ډېرپېښ->c_cc[VTIME]);
	newline();

	لكل (i = 0; i < عدد_ډېرپېښ_info; ++i) {
		إذا (ډېرپېښ_info[i].flags & OMIT)
			استمر;
		إذا (ډېرپېښ_info[i].type != prev_type) {
			newline();
			prev_type = ډېرپېښ_info[i].type;
		}

		bitsp = get_ptr_منزل_tcflag(ډېرپېښ_info[i].type, ډېرپېښ);
		ماسک = ډېرپېښ_info[i].ماسک ? ډېرپېښ_info[i].ماسک : ډېرپېښ_info[i].بټونه;
		إذا ((*bitsp & ماسک) == ډېرپېښ_info[i].بټونه) {
			إذا (all || (ډېرپېښ_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(ډېرپېښ_name, i));
		} وإلا {
			إذا ((all && ډېرپېښ_info[i].flags & REV)
			 || (!all && (ډېرپېښ_info[i].flags & (SANE_سېټ | REV)) == (SANE_سېټ | REV))
			) {
				wrapf("-%s", nth_string(ډېرپېښ_name, i));
			}
		}
	}
	newline();
}

ساكن فراغ sane_ډېرپېښ(بنية termios *ډېرپېښ)
{
	صحيح i;

	لكل (i = 0; i < عدد_control_info; ++i) {
#إذا VMIN == VEOF
		إذا (i == CIDX_min)
			اكسر;
#د_که_پای
		ډېرپېښ->c_cc[control_info[i].دننه_واټن] = control_info[i].saneval;
	}

	لكل (i = 0; i < عدد_ډېرپېښ_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_منزل_tcflag(ډېرپېښ_info[i].type, ډېرپېښ);

		إذا (!bitsp)
			استمر;
		val = *bitsp & ~((بینښه اوږد)ډېرپېښ_info[i].ماسک);
		إذا (ډېرپېښ_info[i].flags & SANE_سېټ) {
			*bitsp = val | ډېرپېښ_info[i].بټونه;
		} وإلا
		إذا (ډېرپېښ_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~ډېرپېښ_info[i].بټونه;
		}
	}
}

ساكن فراغ سېټ_ډېرپېښ(ثابت بنية ډېرپېښ_info *info, صحيح reversed,
					بنية termios *ډېرپېښ)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_منزل_tcflag(info->type, ډېرپېښ);

	إذا (bitsp) {
		tcflag_t val = *bitsp & ~info->ماسک;
		إذا (reversed)
			*bitsp = val & ~info->بټونه;
		وإلا
			*bitsp = val | info->بټونه;
		أرجع;
	}

	/* !bitsp - it's a "combination" mode */
	إذا (info == &ډېرپېښ_info[IDX_evenp] || info == &ډېرپېښ_info[IDX_parity]) {
		إذا (reversed)
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} وإلا إذا (info == &ډېرپېښ_info[IDX_oddp]) {
		إذا (reversed)
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} وإلا إذا (info == &ډېرپېښ_info[IDX_nl]) {
		إذا (reversed) {
			ډېرپېښ->c_iflag = (ډېرپېښ->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			ډېرپېښ->c_oflag = (ډېرپېښ->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} وإلا {
			ډېرپېښ->c_iflag = ډېرپېښ->c_iflag & ~ICRNL;
			إذا (ONLCR) ډېرپېښ->c_oflag = ډېرپېښ->c_oflag & ~ONLCR;
		}
	} وإلا إذا (info == &ډېرپېښ_info[IDX_ek]) {
		ډېرپېښ->c_cc[VERASE] = CERASE;
		ډېرپېښ->c_cc[VKILL] = CKILL;
	} وإلا إذا (info == &ډېرپېښ_info[IDX_sane]) {
		sane_ډېرپېښ(ډېرپېښ);
	} وإلا إذا (info == &ډېرپېښ_info[IDX_cbreak]) {
		إذا (reversed)
			ډېرپېښ->c_lflag |= ICANON;
		وإلا
			ډېرپېښ->c_lflag &= ~ICANON;
	} وإلا إذا (info == &ډېرپېښ_info[IDX_pass8]) {
		إذا (reversed) {
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ډېرپېښ->c_iflag |= ISTRIP;
		} وإلا {
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ډېرپېښ->c_iflag &= ~ISTRIP;
		}
	} وإلا إذا (info == &ډېرپېښ_info[IDX_litout]) {
		إذا (reversed) {
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ډېرپېښ->c_iflag |= ISTRIP;
			ډېرپېښ->c_oflag |= OPOST;
		} وإلا {
			ډېرپېښ->c_cflag = (ډېرپېښ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ډېرپېښ->c_iflag &= ~ISTRIP;
			ډېرپېښ->c_oflag &= ~OPOST;
		}
	} وإلا إذا (info == &ډېرپېښ_info[IDX_raw] || info == &ډېرپېښ_info[IDX_cooked]) {
		إذا ((info == &ډېرپېښ_info[IDX_raw] && reversed)
		 || (info == &ډېرپېښ_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			ډېرپېښ->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			ډېرپېښ->c_oflag |= OPOST;
			ډېرپېښ->c_lflag |= ISIG | ICANON;
#إذا VMIN == VEOF
			ډېرپېښ->c_cc[VEOF] = CEOF;
#د_که_پای
#إذا VTIME == VEOL
			ډېرپېښ->c_cc[VEOL] = CEOL;
#د_که_پای
		} وإلا {
			/* Raw mode */
			ډېرپېښ->c_iflag = 0;
			ډېرپېښ->c_oflag &= ~OPOST;
			ډېرپېښ->c_lflag &= ~(ISIG | ICANON | XCASE);
			ډېرپېښ->c_cc[VMIN] = 1;
			ډېرپېښ->c_cc[VTIME] = 0;
		}
	}
#إذا IXANY
	وإلا إذا (info == &ډېرپېښ_info[IDX_decctlq]) {
		إذا (reversed)
			ډېرپېښ->c_iflag |= IXANY;
		وإلا
			ډېرپېښ->c_iflag &= ~IXANY;
	}
#د_که_پای
#إذا TABDLY
	وإلا إذا (info == &ډېرپېښ_info[IDX_tabs]) {
		إذا (reversed)
			ډېرپېښ->c_oflag = (ډېرپېښ->c_oflag & ~TABDLY) | TAB3;
		وإلا
			ډېرپېښ->c_oflag = (ډېرپېښ->c_oflag & ~TABDLY) | TAB0;
	}
#د_که_پای
#إذا OXTABS
	وإلا إذا (info == &ډېرپېښ_info[IDX_tabs]) {
		إذا (reversed)
			ډېرپېښ->c_oflag |= OXTABS;
		وإلا
			ډېرپېښ->c_oflag &= ~OXTABS;
	}
#د_که_پای
#إذا XCASE && IUCLC && OLCUC
	وإلا إذا (info==&ډېرپېښ_info[IDX_lcase] || info==&ډېرپېښ_info[IDX_LCASE]) {
		إذا (reversed) {
			ډېرپېښ->c_lflag &= ~XCASE;
			ډېرپېښ->c_iflag &= ~IUCLC;
			ډېرپېښ->c_oflag &= ~OLCUC;
		} وإلا {
			ډېرپېښ->c_lflag |= XCASE;
			ډېرپېښ->c_iflag |= IUCLC;
			ډېرپېښ->c_oflag |= OLCUC;
		}
	}
#د_که_پای
	وإلا إذا (info == &ډېرپېښ_info[IDX_crt]) {
		ډېرپېښ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} وإلا إذا (info == &ډېرپېښ_info[IDX_dec]) {
		ډېرپېښ->c_cc[VINTR] = 3; /* ^C */
		ډېرپېښ->c_cc[VERASE] = 127; /* DEL */
		ډېرپېښ->c_cc[VKILL] = 21; /* ^U */
		ډېرپېښ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		إذا (IXANY) ډېرپېښ->c_iflag &= ~IXANY;
	}
}

ساكن فراغ سېټ_control_char_or_die(ثابت بنية control_info *info,
			ثابت حرف *arg, بنية termios *ډېرپېښ)
{
	بینښه حرف ارزښت_2;

	إذا (info == &control_info[CIDX_min] || info == &control_info[CIDX_وخت])
		ارزښت_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	وإلا إذا (arg[0] == '\0' || arg[1] == '\0')
		ارزښت_2 = arg[0];
	وإلا إذا (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		ارزښت_2 = _POSIX_VDISABLE;
	وإلا إذا (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		ارزښت_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		إذا (arg[1] == '?')
			ارزښت_2 = 127;
	} وإلا
		ارزښت_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ډېرپېښ->c_cc[info->دننه_واټن] = ارزښت_2;
}

#تعریف_کړه STTY_require_سېټ_attr   (1 << 0)
#تعریف_کړه STTY_چټکتیا_was_سېټ      (1 << 1)
#تعریف_کړه STTY_verbose_output     (1 << 2)
#تعریف_کړه STTY_recoverable_output (1 << 3)
#تعریف_کړه STTY_noargs             (1 << 4)

صحيح stty_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stty_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	بنية termios ډېرپېښ;
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
	طالما (argv[++k]) {
		ثابت بنية ډېرپېښ_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			صحيح i;
			mp = پیدا_کړه_ډېرپېښ(arg+1);
			إذا (mp) {
				إذا (!(mp->flags & REV))
					اذهب invalid_دلیل;
				stty_state &= ~STTY_noargs;
				استمر;
			}
			/* It is an option - parse it */
			i = 0;
			طالما (arg[++i]) {
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
						bb_simple_error_پیغام_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					إذا (!file_name[0]) { /* nope, "-F device" */
						صحيح p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						إذا (!file_name)
							bb_error_پیغام_and_die(bb_پیغام_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						طالما (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					اذهب پای_ټکی_option;
				افتراضي:
					اذهب invalid_دلیل;
				}
			}
 پای_ټکی_option:
			استمر;
		}

		mp = پیدا_کړه_ډېرپېښ(arg);
		إذا (mp) {
			stty_state &= ~STTY_noargs;
			استمر;
		}

		cp = پیدا_کړه_control(arg);
		إذا (cp) {
			إذا (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			سېټ_control_char_or_die(cp, argnext, &ډېرپېښ);
			stty_state &= ~STTY_noargs;
			++k;
			استمر;
		}

		param = پیدا_کړه_param(arg);
		إذا (param & param_need_arg) {
			إذا (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			++k;
		}

		بدل (param) {
#که_تعریف_شوی __linux__
		حالة param_کرښه:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, لوی_عدد, stty_suffixes);
			اكسر;
# endif /* else fall-through */
#د_که_پای
#که_تعریف_شوی TIOCGWINSZ
		حالة param_لیکې:
		حالة param_ستون_شمېر:
		حالة param_columns:
			xatoul_range_sfx(argnext, 1, لوی_عدد, stty_suffixes);
			اكسر;
		حالة param_اندازه:
#د_که_پای
		حالة param_چټکتیا:
			اكسر;
		حالة param_ispeed:
			/* called for the side effect of xfunc death only */
			سېټ_چټکتیا_or_die(ننوت_چټکتیا, argnext, &ډېرپېښ);
			اكسر;
		حالة param_ospeed:
			/* called for the side effect of xfunc death only */
			سېټ_چټکتیا_or_die(output_چټکتیا, argnext, &ډېرپېښ);
			اكسر;
		افتراضي:
			إذا (recover_ډېرپېښ(arg, &ډېرپېښ) == 1) اكسر;
			إذا (tty_ارزښت_منزل_baud(xatou(arg)) != (چټکتیا_t) -1) اكسر;
 invalid_دلیل:
			bb_error_پیغام_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_پیغام_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_پیغام_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	إذا (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&ډېرپېښ, 0, حجم(ډېرپېښ));
	إذا (tcgetattr(STDIN_FILENO, &ډېرپېښ))
		perror_on_device_and_die("%s");

	إذا (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_ستون = get_terminal_پلنوالی(STDOUT_FILENO);
		output_func(&ډېرپېښ, display_all);
		أرجع EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	طالما (argv[++k]) {
		ثابت بنية ډېرپېښ_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			mp = پیدا_کړه_ډېرپېښ(arg+1);
			إذا (mp) {
				سېټ_ډېرپېښ(mp, 1 /* reversed */, &ډېرپېښ);
				stty_state |= STTY_require_سېټ_attr;
			}
			/* It is an option - already parsed. Skip it */
			استمر;
		}

		mp = پیدا_کړه_ډېرپېښ(arg);
		إذا (mp) {
			سېټ_ډېرپېښ(mp, 0 /* non-reversed */, &ډېرپېښ);
			stty_state |= STTY_require_سېټ_attr;
			استمر;
		}

		cp = پیدا_کړه_control(arg);
		إذا (cp) {
			++k;
			سېټ_control_char_or_die(cp, argnext, &ډېرپېښ);
			stty_state |= STTY_require_سېټ_attr;
			استمر;
		}

		param = پیدا_کړه_param(arg);
		إذا (param & param_need_arg) {
			++k;
		}

		بدل (param) {
#که_تعریف_شوی __linux__
		حالة param_کرښه:
			ډېرپېښ.c_کرښه = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_سېټ_attr;
			اكسر;
#د_که_پای
#که_تعریف_شوی TIOCGWINSZ
		حالة param_ستون_شمېر:
		حالة param_columns:
			سېټ_کړکۍ_اندازه(-1, xatoul_sfx(argnext, stty_suffixes));
			اكسر;
		حالة param_اندازه:
			display_کړکۍ_اندازه(0);
			اكسر;
		حالة param_لیکې:
			سېټ_کړکۍ_اندازه(xatoul_sfx(argnext, stty_suffixes), -1);
			اكسر;
#د_که_پای
		حالة param_چټکتیا:
			display_چټکتیا(&ډېرپېښ, 0);
			اكسر;
		حالة param_ispeed:
			سېټ_چټکتیا_or_die(ننوت_چټکتیا, argnext, &ډېرپېښ);
			stty_state |= (STTY_require_سېټ_attr | STTY_چټکتیا_was_سېټ);
			اكسر;
		حالة param_ospeed:
			سېټ_چټکتیا_or_die(output_چټکتیا, argnext, &ډېرپېښ);
			stty_state |= (STTY_require_سېټ_attr | STTY_چټکتیا_was_سېټ);
			اكسر;
		افتراضي:
			إذا (recover_ډېرپېښ(arg, &ډېرپېښ) == 1)
				stty_state |= STTY_require_سېټ_attr;
			وإلا /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				سېټ_چټکتیا_or_die(both_speeds, arg, &ډېرپېښ);
				stty_state |= (STTY_require_سېټ_attr | STTY_چټکتیا_was_سېټ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	إذا (stty_state & STTY_require_سېټ_attr) {
		بنية termios new_ډېرپېښ;

		إذا (tcsetattr(STDIN_FILENO, TCSADRAIN, &ډېرپېښ))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_ډېرپېښ, 0, حجم(new_ډېرپېښ));
		إذا (tcgetattr(STDIN_FILENO, &new_ډېرپېښ))
			perror_on_device_and_die("%s");

		إذا (memcmp(&ډېرپېښ, &new_ډېرپېښ, حجم(ډېرپېښ)) != 0) {
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

			new_ډېرپېښ.c_cflag &= (~CIBAUD);
			إذا ((stty_state & STTY_چټکتیا_was_سېټ)
			 || memcmp(&ډېرپېښ, &new_ډېرپېښ, حجم(ډېرپېښ)) != 0)
#د_که_پای
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	أرجع EXIT_SUCCESS;
}
