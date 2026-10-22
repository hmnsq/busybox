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

#جے_بنا_تعریف _POSIX_VDISABLE
# define _POSIX_VDISABLE ((بنانشانی حرف) 0)
#جے_انت

#تعریف_کرو Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#جے_بنا_تعریف CINTR
# define CINTR Control('c')
#جے_انت
#جے_بنا_تعریف CQUIT
# define CQUIT 28
#جے_انت
#جے_بنا_تعریف CERASE
# define CERASE 127
#جے_انت
#جے_بنا_تعریف CKILL
# define CKILL Control('u')
#جے_انت
#جے_بنا_تعریف CEOF
# define CEOF Control('d')
#جے_انت
#جے_بنا_تعریف CEOL
# define CEOL _POSIX_VDISABLE
#جے_انت
#جے_بنا_تعریف CSTART
# define CSTART Control('q')
#جے_انت
#جے_بنا_تعریف CSTOP
# define CSTOP Control('s')
#جے_انت
#جے_بنا_تعریف CSUSP
# define CSUSP Control('z')
#جے_انت
#إذا defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#جے_انت
/* glibc-2.12.1 uses only VSWTC name */
#إذا defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#جے_انت
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#إذا defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#جے_انت
#إذا defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#جے_انت

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#إذا defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#جے_انت

#إذا defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#جے_انت
#إذا defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#جے_انت
#إذا !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#جے_انت
#إذا defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#جے_انت
#إذا defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#جے_انت
#إذا defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#جے_انت
#إذا defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#جے_انت
#إذا defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#جے_انت
#إذا defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#جے_انت
#إذا defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#جے_انت
#إذا defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#جے_انت
#إذا defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#جے_انت
#إذا defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#جے_انت

/* Save us from #ifdef forest plague */
#جے_بنا_تعریف BSDLY
# define BSDLY 0
#جے_انت
#جے_بنا_تعریف CIBAUD
# define CIBAUD 0
#جے_انت
#جے_بنا_تعریف CRDLY
# define CRDLY 0
#جے_انت
#جے_بنا_تعریف CMSPAR
# define CMSPAR 0
#جے_انت
#جے_بنا_تعریف CRTSCTS
# define CRTSCTS 0
#جے_انت
#جے_بنا_تعریف ECHOCTL
# define ECHOCTL 0
#جے_انت
#جے_بنا_تعریف ECHOKE
# define ECHOKE 0
#جے_انت
#جے_بنا_تعریف ECHOPRT
# define ECHOPRT 0
#جے_انت
#جے_بنا_تعریف FFDLY
# define FFDLY 0
#جے_انت
#جے_بنا_تعریف IEXTEN
# define IEXTEN 0
#جے_انت
#جے_بنا_تعریف IMAXBEL
# define IMAXBEL 0
#جے_انت
#جے_بنا_تعریف IUCLC
# define IUCLC 0
#جے_انت
#جے_بنا_تعریف IXANY
# define IXANY 0
#جے_انت
#جے_بنا_تعریف NLDLY
# define NLDLY 0
#جے_انت
#جے_بنا_تعریف OCRNL
# define OCRNL 0
#جے_انت
#جے_بنا_تعریف OFDEL
# define OFDEL 0
#جے_انت
#جے_بنا_تعریف OFILL
# define OFILL 0
#جے_انت
#جے_بنا_تعریف OLCUC
# define OLCUC 0
#جے_انت
#جے_بنا_تعریف ONLCR
# define ONLCR 0
#جے_انت
#جے_بنا_تعریف ONLRET
# define ONLRET 0
#جے_انت
#جے_بنا_تعریف ONOCR
# define ONOCR 0
#جے_انت
#جے_بنا_تعریف OXTABS
# define OXTABS 0
#جے_انت
#جے_بنا_تعریف TABDLY
# define TABDLY 0
#جے_انت
#جے_بنا_تعریف TAB1
# define TAB1 0
#جے_انت
#جے_بنا_تعریف TAB2
# define TAB2 0
#جے_انت
#جے_بنا_تعریف TOSTOP
# define TOSTOP 0
#جے_انت
#جے_بنا_تعریف VDSUSP
# define VDSUSP 0
#جے_انت
#جے_بنا_تعریف VEOL2
# define VEOL2 0
#جے_انت
#جے_بنا_تعریف VFLUSHO
# define VFLUSHO 0
#جے_انت
#جے_بنا_تعریف VLNEXT
# define VLNEXT 0
#جے_انت
#جے_بنا_تعریف VREPRINT
# define VREPRINT 0
#جے_انت
#جے_بنا_تعریف VSTATUS
# define VSTATUS 0
#جے_انت
#جے_بنا_تعریف VSWTCH
# define VSWTCH 0
#جے_انت
#جے_بنا_تعریف VTDLY
# define VTDLY 0
#جے_انت
#جے_بنا_تعریف VWERASE
# define VWERASE 0
#جے_انت
#جے_بنا_تعریف XCASE
# define XCASE 0
#جے_انت
#جے_بنا_تعریف IUTF8
# define IUTF8 0
#جے_انت

/* Which speeds to set */
تعداد رفتار_setting {
	داخلہ_رفتار, output_رفتار, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
تعداد {
	control, داخلہ, output, local, combination
};
ساكن tcflag_t *get_ptr_منزل_tcflag(بنانشانی type, ثابت بنية termios *منوال)
{
	ساكن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(بنية termios, c_cflag), /* control */
		offsetof(بنية termios, c_iflag), /*داخلہ*/
		offsetof(بنية termios, c_oflag), /* output */
		offsetof(بنية termios, c_lflag)  /* local */
	};
	إذا (type <= local) {
		أرجع (tcflag_t*) (((حرف*)منوال) + tcflag_offsets[type]);
	}
	أرجع عدم;
}

/* Flags for 'struct mode_info' */
#تعریف_کرو SANE_سیٹ 1              /* Set in 'sane' mode                  */
#تعریف_کرو SANE_UNSET 2            /* Unset in 'sane' mode                */
#تعریف_کرو REV 4                   /* Can be turned off by prepending '-' */
#تعریف_کرو OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
بنية منوال_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#إذا   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t نقاب;
#وإلاإذا (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t نقاب;
#وإلا
	ثابت tcflag_t نقاب;          /* Other bits to turn off for this mode */
#جے_انت
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t بٹ;          /* Bits to set for this mode            */
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
#جے_انت
#إذا TABDLY || OXTABS
	IDX_tabs,
#جے_انت
#إذا XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#جے_انت
};

#تعریف_کرو MI_اندراج(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساكن ثابت حرف منوال_name[] ALIGN1 =
	MI_اندراج("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("parity",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_اندراج("nl",       combination, REV        | OMIT, 0,          0 )
	MI_اندراج("ek",       combination, OMIT,              0,          0 )
	MI_اندراج("sane",     combination, OMIT,              0,          0 )
	MI_اندراج("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("raw",      combination, REV        | OMIT, 0,          0 )
	MI_اندراج("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("litout",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("crt",      combination, OMIT,              0,          0 )
	MI_اندراج("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_اندراج("decctlq",  combination, REV        | OMIT, 0,          0 )
#جے_انت
#إذا TABDLY || OXTABS
	MI_اندراج("tabs",     combination, REV        | OMIT, 0,          0 )
#جے_انت
#إذا XCASE && IUCLC && OLCUC
	MI_اندراج("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("LCASE",    combination, REV        | OMIT, 0,          0 )
#جے_انت
	MI_اندراج("parenb",   control,     REV,               PARENB,     0 )
	MI_اندراج("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_اندراج("cmspar",   control,     REV,               CMSPAR,     0 )
#جے_انت
	MI_اندراج("cs5",      control,     0,                 CS5,     CSIZE)
	MI_اندراج("cs6",      control,     0,                 CS6,     CSIZE)
	MI_اندراج("cs7",      control,     0,                 CS7,     CSIZE)
	MI_اندراج("cs8",      control,     0,                 CS8,     CSIZE)
	MI_اندراج("hupcl",    control,     REV,               HUPCL,      0 )
	MI_اندراج("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_اندراج("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_اندراج("cread",    control,     SANE_سیٹ   | REV,  CREAD,      0 )
	MI_اندراج("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_اندراج("crtscts",  control,     REV,               CRTSCTS,    0 )
#جے_انت
	MI_اندراج("ignbrk",   داخلہ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_اندراج("brkint",   داخلہ,       SANE_سیٹ   | REV,  BRKINT,     0 )
	MI_اندراج("ignpar",   داخلہ,       REV,               IGNPAR,     0 )
	MI_اندراج("parmrk",   داخلہ,       REV,               PARMRK,     0 )
	MI_اندراج("inpck",    داخلہ,       REV,               INPCK,      0 )
	MI_اندراج("istrip",   داخلہ,       REV,               ISTRIP,     0 )
	MI_اندراج("inlcr",    داخلہ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_اندراج("igncr",    داخلہ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_اندراج("icrnl",    داخلہ,       SANE_سیٹ   | REV,  ICRNL,      0 )
	MI_اندراج("ixon",     داخلہ,       REV,               IXON,       0 )
	MI_اندراج("ixoff",    داخلہ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_اندراج("tandem",   داخلہ,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_اندراج("iuclc",    داخلہ,       SANE_UNSET | REV,  IUCLC,      0 )
#جے_انت
#إذا IXANY
	MI_اندراج("ixany",    داخلہ,       SANE_UNSET | REV,  IXANY,      0 )
#جے_انت
#إذا IMAXBEL
	MI_اندراج("imaxbel",  داخلہ,       SANE_سیٹ   | REV,  IMAXBEL,    0 )
#جے_انت
#إذا IUTF8
	MI_اندراج("iutf8",    داخلہ,       SANE_UNSET | REV,  IUTF8,      0 )
#جے_انت
	MI_اندراج("opost",    output,      SANE_سیٹ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_اندراج("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#جے_انت
#إذا OCRNL
	MI_اندراج("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#جے_انت
#إذا ONLCR
	MI_اندراج("onlcr",    output,      SANE_سیٹ   | REV,  ONLCR,      0 )
#جے_انت
#إذا ONOCR
	MI_اندراج("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#جے_انت
#إذا ONLRET
	MI_اندراج("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#جے_انت
#إذا OFILL
	MI_اندراج("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#جے_انت
#إذا OFDEL
	MI_اندراج("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#جے_انت
#إذا NLDLY
	MI_اندراج("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_اندراج("nl0",      output,      SANE_سیٹ,          NL0,     NLDLY)
#جے_انت
#إذا CRDLY
	MI_اندراج("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_اندراج("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_اندراج("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_اندراج("cr0",      output,      SANE_سیٹ,          CR0,     CRDLY)
#جے_انت

#إذا TABDLY
	MI_اندراج("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_اندراج("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_اندراج("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_اندراج("tab0",     output,      SANE_سیٹ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_اندراج("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#جے_انت

#إذا BSDLY
	MI_اندراج("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_اندراج("bs0",      output,      SANE_سیٹ,          BS0,     BSDLY)
#جے_انت
#إذا VTDLY
	MI_اندراج("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_اندراج("vt0",      output,      SANE_سیٹ,          VT0,     VTDLY)
#جے_انت
#إذا FFDLY
	MI_اندراج("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_اندراج("ff0",      output,      SANE_سیٹ,          FF0,     FFDLY)
#جے_انت
	MI_اندراج("isig",     local,       SANE_سیٹ   | REV,  ISIG,       0 )
	MI_اندراج("icanon",   local,       SANE_سیٹ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_اندراج("iexten",   local,       SANE_سیٹ   | REV,  IEXTEN,     0 )
#جے_انت
	MI_اندراج("echo",     local,       SANE_سیٹ   | REV,  ECHO,       0 )
	MI_اندراج("echoe",    local,       SANE_سیٹ   | REV,  ECHOE,      0 )
	MI_اندراج("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_اندراج("echok",    local,       SANE_سیٹ   | REV,  ECHOK,      0 )
	MI_اندراج("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_اندراج("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_اندراج("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#جے_انت
#إذا TOSTOP
	MI_اندراج("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#جے_انت
#إذا ECHOPRT
	MI_اندراج("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_اندراج("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#جے_انت
#إذا ECHOCTL
	MI_اندراج("echoctl",  local,       SANE_سیٹ   | REV,  ECHOCTL,    0 )
	MI_اندراج("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#جے_انت
#إذا ECHOKE
	MI_اندراج("echoke",   local,       SANE_سیٹ   | REV,  ECHOKE,     0 )
	MI_اندراج("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#جے_انت
	MI_اندراج("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#جے_تعریف_شدہ EXTPROC
	MI_اندراج("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#جے_انت
	;

#تعریف_رد MI_اندراج
#تعریف_کرو MI_اندراج(N,T,F,B,M) { T, F, M, B },

ساكن ثابت بنية منوال_info منوال_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_اندراج("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("parity",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_اندراج("nl",       combination, REV        | OMIT, 0,          0 )
	MI_اندراج("ek",       combination, OMIT,              0,          0 )
	MI_اندراج("sane",     combination, OMIT,              0,          0 )
	MI_اندراج("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("raw",      combination, REV        | OMIT, 0,          0 )
	MI_اندراج("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("litout",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_اندراج("crt",      combination, OMIT,              0,          0 )
	MI_اندراج("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_اندراج("decctlq",  combination, REV        | OMIT, 0,          0 )
#جے_انت
#إذا TABDLY || OXTABS
	MI_اندراج("tabs",     combination, REV        | OMIT, 0,          0 )
#جے_انت
#إذا XCASE && IUCLC && OLCUC
	MI_اندراج("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("LCASE",    combination, REV        | OMIT, 0,          0 )
#جے_انت
	MI_اندراج("parenb",   control,     REV,               PARENB,     0 )
	MI_اندراج("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_اندراج("cmspar",   control,     REV,               CMSPAR,     0 )
#جے_انت
	MI_اندراج("cs5",      control,     0,                 CS5,     CSIZE)
	MI_اندراج("cs6",      control,     0,                 CS6,     CSIZE)
	MI_اندراج("cs7",      control,     0,                 CS7,     CSIZE)
	MI_اندراج("cs8",      control,     0,                 CS8,     CSIZE)
	MI_اندراج("hupcl",    control,     REV,               HUPCL,      0 )
	MI_اندراج("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_اندراج("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_اندراج("cread",    control,     SANE_سیٹ   | REV,  CREAD,      0 )
	MI_اندراج("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_اندراج("crtscts",  control,     REV,               CRTSCTS,    0 )
#جے_انت
	MI_اندراج("ignbrk",   داخلہ,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_اندراج("brkint",   داخلہ,       SANE_سیٹ   | REV,  BRKINT,     0 )
	MI_اندراج("ignpar",   داخلہ,       REV,               IGNPAR,     0 )
	MI_اندراج("parmrk",   داخلہ,       REV,               PARMRK,     0 )
	MI_اندراج("inpck",    داخلہ,       REV,               INPCK,      0 )
	MI_اندراج("istrip",   داخلہ,       REV,               ISTRIP,     0 )
	MI_اندراج("inlcr",    داخلہ,       SANE_UNSET | REV,  INLCR,      0 )
	MI_اندراج("igncr",    داخلہ,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_اندراج("icrnl",    داخلہ,       SANE_سیٹ   | REV,  ICRNL,      0 )
	MI_اندراج("ixon",     داخلہ,       REV,               IXON,       0 )
	MI_اندراج("ixoff",    داخلہ,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_اندراج("tandem",   داخلہ,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_اندراج("iuclc",    داخلہ,       SANE_UNSET | REV,  IUCLC,      0 )
#جے_انت
#إذا IXANY
	MI_اندراج("ixany",    داخلہ,       SANE_UNSET | REV,  IXANY,      0 )
#جے_انت
#إذا IMAXBEL
	MI_اندراج("imaxbel",  داخلہ,       SANE_سیٹ   | REV,  IMAXBEL,    0 )
#جے_انت
#إذا IUTF8
	MI_اندراج("iutf8",    داخلہ,       SANE_UNSET | REV,  IUTF8,      0 )
#جے_انت
	MI_اندراج("opost",    output,      SANE_سیٹ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_اندراج("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#جے_انت
#إذا OCRNL
	MI_اندراج("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#جے_انت
#إذا ONLCR
	MI_اندراج("onlcr",    output,      SANE_سیٹ   | REV,  ONLCR,      0 )
#جے_انت
#إذا ONOCR
	MI_اندراج("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#جے_انت
#إذا ONLRET
	MI_اندراج("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#جے_انت
#إذا OFILL
	MI_اندراج("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#جے_انت
#إذا OFDEL
	MI_اندراج("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#جے_انت
#إذا NLDLY
	MI_اندراج("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_اندراج("nl0",      output,      SANE_سیٹ,          NL0,     NLDLY)
#جے_انت
#إذا CRDLY
	MI_اندراج("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_اندراج("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_اندراج("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_اندراج("cr0",      output,      SANE_سیٹ,          CR0,     CRDLY)
#جے_انت

#إذا TABDLY
	MI_اندراج("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_اندراج("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_اندراج("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_اندراج("tab0",     output,      SANE_سیٹ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_اندراج("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#جے_انت

#إذا BSDLY
	MI_اندراج("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_اندراج("bs0",      output,      SANE_سیٹ,          BS0,     BSDLY)
#جے_انت
#إذا VTDLY
	MI_اندراج("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_اندراج("vt0",      output,      SANE_سیٹ,          VT0,     VTDLY)
#جے_انت
#إذا FFDLY
	MI_اندراج("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_اندراج("ff0",      output,      SANE_سیٹ,          FF0,     FFDLY)
#جے_انت
	MI_اندراج("isig",     local,       SANE_سیٹ   | REV,  ISIG,       0 )
	MI_اندراج("icanon",   local,       SANE_سیٹ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_اندراج("iexten",   local,       SANE_سیٹ   | REV,  IEXTEN,     0 )
#جے_انت
	MI_اندراج("echo",     local,       SANE_سیٹ   | REV,  ECHO,       0 )
	MI_اندراج("echoe",    local,       SANE_سیٹ   | REV,  ECHOE,      0 )
	MI_اندراج("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_اندراج("echok",    local,       SANE_سیٹ   | REV,  ECHOK,      0 )
	MI_اندراج("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_اندراج("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_اندراج("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#جے_انت
#إذا TOSTOP
	MI_اندراج("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#جے_انت
#إذا ECHOPRT
	MI_اندراج("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_اندراج("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#جے_انت
#إذا ECHOCTL
	MI_اندراج("echoctl",  local,       SANE_سیٹ   | REV,  ECHOCTL,    0 )
	MI_اندراج("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#جے_انت
#إذا ECHOKE
	MI_اندراج("echoke",   local,       SANE_سیٹ   | REV,  ECHOKE,     0 )
	MI_اندراج("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#جے_انت
	MI_اندراج("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#جے_تعریف_شدہ EXTPROC
	MI_اندراج("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#جے_انت
};

تعداد {
	عدد_منوال_info = ARRAY_ناپ(منوال_info)
};


/* Control characters */
بنية control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t وتھ;   /* Offset in c_cc */
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
#جے_انت
#إذا VSWTCH
	CIDX_swtch,
#جے_انت
	CIDX_شروع,
	CIDX_stop,
	CIDX_susp,
#إذا VDSUSP
	CIDX_dsusp,
#جے_انت
#إذا VREPRINT
	CIDX_rprnt,
#جے_انت
#إذا VWERASE
	CIDX_werase,
#جے_انت
#إذا VLNEXT
	CIDX_lnext,
#جے_انت
#إذا VFLUSHO
	CIDX_flush,
#جے_انت
#إذا VSTATUS
	CIDX_حالت,
#جے_انت
	CIDX_min,
	CIDX_ویلا,
};

#تعریف_کرو CI_اندراج(n,s,o) n "\0"

/* Name given on command line */
ساكن ثابت حرف control_name[] ALIGN1 =
	CI_اندراج("intr",     CINTR,   VINTR   )
	CI_اندراج("quit",     CQUIT,   VQUIT   )
	CI_اندراج("erase",    CERASE,  VERASE  )
	CI_اندراج("kill",     CKILL,   VKILL   )
	CI_اندراج("eof",      CEOF,    VEOF    )
	CI_اندراج("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_اندراج("eol2",     CEOL2,   VEOL2   )
#جے_انت
#إذا VSWTCH
	CI_اندراج("swtch",    CSWTCH,  VSWTCH  )
#جے_انت
	CI_اندراج("start",    CSTART,  VSTART  )
	CI_اندراج("stop",     CSTOP,   VSTOP   )
	CI_اندراج("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_اندراج("dsusp",    CDSUSP,  VDSUSP  )
#جے_انت
#إذا VREPRINT
	CI_اندراج("rprnt",    CRPRNT,  VREPRINT)
#جے_انت
#إذا VWERASE
	CI_اندراج("werase",   CWERASE, VWERASE )
#جے_انت
#إذا VLNEXT
	CI_اندراج("lnext",    CLNEXT,  VLNEXT  )
#جے_انت
#إذا VFLUSHO
	CI_اندراج("flush",    CFLUSHO, VFLUSHO )
#جے_انت
#إذا VSTATUS
	CI_اندراج("status",   CSTATUS, VSTATUS )
#جے_انت
	/* These must be last because of the display routines */
	CI_اندراج("min",      1,       VMIN    )
	CI_اندراج("time",     0,       VTIME   )
	;

#تعریف_رد CI_اندراج
#تعریف_کرو CI_اندراج(n,s,o) { s, o },

ساكن ثابت بنية control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_اندراج("intr",     CINTR,   VINTR   )
	CI_اندراج("quit",     CQUIT,   VQUIT   )
	CI_اندراج("erase",    CERASE,  VERASE  )
	CI_اندراج("kill",     CKILL,   VKILL   )
	CI_اندراج("eof",      CEOF,    VEOF    )
	CI_اندراج("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_اندراج("eol2",     CEOL2,   VEOL2   )
#جے_انت
#إذا VSWTCH
	CI_اندراج("swtch",    CSWTCH,  VSWTCH  )
#جے_انت
	CI_اندراج("start",    CSTART,  VSTART  )
	CI_اندراج("stop",     CSTOP,   VSTOP   )
	CI_اندراج("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_اندراج("dsusp",    CDSUSP,  VDSUSP  )
#جے_انت
#إذا VREPRINT
	CI_اندراج("rprnt",    CRPRNT,  VREPRINT)
#جے_انت
#إذا VWERASE
	CI_اندراج("werase",   CWERASE, VWERASE )
#جے_انت
#إذا VLNEXT
	CI_اندراج("lnext",    CLNEXT,  VLNEXT  )
#جے_انت
#إذا VFLUSHO
	CI_اندراج("flush",    CFLUSHO, VFLUSHO )
#جے_انت
#إذا VSTATUS
	CI_اندراج("status",   CSTATUS, VSTATUS )
#جے_انت
	/* These must be last because of the display routines */
	CI_اندراج("min",      1,       VMIN    )
	CI_اندراج("time",     0,       VTIME   )
};

تعداد {
	عدد_control_info = ARRAY_ناپ(control_info)
};


بنية globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	بنانشانی max_خانہ;
	/* Current position, to know when to wrap */
	بنانشانی current_خانہ;
} FIX_ALIASING;
#تعریف_کرو G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	G.device_name = bb_پیغام_standard_داخلہ; \
	G.max_خانہ = 80; \
	G.current_خانہ = 0; /* we are noexec, must clear */ \
} طالما (0)

ساكن فراغ سیٹ_رفتار_or_die(تعداد رفتار_setting type, ثابت حرف *arg,
					بنية termios *منوال)
{
	رفتار_t baud;

	baud = tty_قدر_منزل_baud(xatou(arg));

	إذا (type != output_رفتار) {     /* either input or both */
		cfsetispeed(منوال, baud);
	}
	إذا (type != داخلہ_رفتار) {      /* either output or both */
		cfsetospeed(منوال, baud);
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
ساكن فراغ wrapf(ثابت حرف *سنیہا, ...)
{
	حرف buf[128];
	va_list دلیلاں;
	بنانشانی buflen;

	va_شروع(دلیلاں, سنیہا);
	buflen = vsnprintf(buf, حجم(buf), سنیہا, دلیلاں);
	va_سرا(دلیلاں);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	إذا (!buflen || buflen >= حجم(buf)) أرجع;

	إذا (G.current_خانہ > 0) {
		G.current_خانہ++;
		إذا (buf[0] != '\n') {
			إذا (G.current_خانہ + buflen >= G.max_خانہ) {
				G.current_خانہ = 0;
				bb_putchar('\n');
			} وإلا {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_خانہ += buflen;
	إذا (buf[buflen-1] == '\n')
		G.current_خانہ = 0;
}

ساكن فراغ newline(فراغ)
{
	إذا (G.current_خانہ != 0)
		wrapf("\n");
}

#جے_تعریف_شدہ TIOCGWINSZ
ساكن فراغ سیٹ_کھڑکی_ناپ(صحيح صفاں, صحيح خانے_گنتی)
{
	بنية winsize win = { 0, 0, 0, 0 };

	إذا (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		إذا (errno != EINVAL) {
			اذهب bail;
		}
		memset(&win, 0, حجم(win));
	}

	إذا (صفاں >= 0)
		win.ws_صف = صفاں;
	إذا (خانے_گنتی >= 0)
		win.ws_خانہ = خانے_گنتی;

	إذا (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#جے_انت

ساكن فراغ display_کھڑکی_ناپ(صحيح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	بنانشانی چوڑائی, اچائی;

	إذا (get_terminal_چوڑائی_اچائی(STDIN_FILENO, &چوڑائی, &اچائی)) {
		إذا ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} وإلا {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				اچائی, چوڑائی);
	}
}

ساكن ثابت بنية suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساكن ثابت بنية منوال_info *لبھو_منوال(ثابت حرف *name)
{
	صحيح i = اشاریہ_وچ_لڑیاں(منوال_name, name);
	أرجع i >= 0 ? &منوال_info[i] : عدم;
}

ساكن ثابت بنية control_info *لبھو_control(ثابت حرف *name)
{
	صحيح i = اشاریہ_وچ_لڑیاں(control_name, name);
	أرجع i >= 0 ? &control_info[i] : عدم;
}

تعداد {
	param_need_arg = 0x80,
	param_سطر    = 1 | 0x80,
	param_صفاں    = 2 | 0x80,
	param_خانے_گنتی    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_ناپ    = 5,
	param_رفتار   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساكن صحيح لبھو_param(ثابت حرف *name)
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
	صحيح i = اشاریہ_وچ_لڑیاں(params, name) + 1;
	إذا (i == 0)
		أرجع 0;
	إذا (i != 5 && i != 6)
		i |= 0x80;
	أرجع i;
}

ساكن صحيح recover_منوال(ثابت حرف *arg, بنية termios *منوال)
{
	صحيح i, n;
	بنانشانی chr;
	بنانشانی لما iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	إذا (لڑی_توں_پڑھو(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		أرجع 0;
	منوال->c_iflag = iflag;
	منوال->c_oflag = oflag;
	منوال->c_cflag = cflag;
	منوال->c_lflag = lflag;
	arg += n;
	لكل (i = 0; i < NCCS; ++i) {
		إذا (لڑی_توں_پڑھو(arg, ":%x%n", &chr, &n) != 1)
			أرجع 0;
		منوال->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	إذا (*arg != '\0')
		أرجع 0;

	أرجع 1;
}

ساكن فراغ display_recoverable(ثابت بنية termios *منوال,
				صحيح بن_ورتیا_PARAM dummy)
{
	صحيح i;
	لکھو("%lx:%lx:%lx:%lx",
		   (بنانشانی لما) منوال->c_iflag, (بنانشانی لما) منوال->c_oflag,
		   (بنانشانی لما) منوال->c_cflag, (بنانشانی لما) منوال->c_lflag);
	لكل (i = 0; i < NCCS; ++i)
		لکھو(":%x", (بنانشانی صحيح) منوال->c_cc[i]);
	bb_putchar('\n');
}

ساكن فراغ display_رفتار(ثابت بنية termios *منوال, صحيح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	بنانشانی لما ispeed, ospeed;

	ispeed = cfgetispeed(منوال);
	ospeed = cfgetospeed(منوال);
	إذا (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	إذا (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_منزل_قدر(ispeed), tty_baud_منزل_قدر(ospeed));
}

ساكن فراغ do_display(ثابت بنية termios *منوال, صحيح all)
{
	صحيح i;
	tcflag_t *bitsp;
	بنانشانی لما نقاب;
	صحيح prev_type = control;

	display_رفتار(منوال, 1);
	إذا (all)
		display_کھڑکی_ناپ(1);
#جے_تعریف_شدہ __linux__
	wrapf("line = %u;\n", منوال->c_سطر);
#وإلا
	newline();
#جے_انت

	لكل (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#إذا VSWTCH == VSUSP
		إذا (i == CIDX_swtch)
			استمر;
#جے_انت
		/* If eof uses the same slot as min, only print whichever applies */
#إذا VEOF == VMIN
		إذا (!(منوال->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			استمر;
		}
#جے_انت
		ch = منوال->c_cc[control_info[i].وتھ];
		إذا (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		وإلا
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#إذا VEOF == VMIN
	إذا ((منوال->c_lflag & ICANON) == 0)
#جے_انت
		wrapf("min = %u; time = %u;", منوال->c_cc[VMIN], منوال->c_cc[VTIME]);
	newline();

	لكل (i = 0; i < عدد_منوال_info; ++i) {
		إذا (منوال_info[i].flags & OMIT)
			استمر;
		إذا (منوال_info[i].type != prev_type) {
			newline();
			prev_type = منوال_info[i].type;
		}

		bitsp = get_ptr_منزل_tcflag(منوال_info[i].type, منوال);
		نقاب = منوال_info[i].نقاب ? منوال_info[i].نقاب : منوال_info[i].بٹ;
		إذا ((*bitsp & نقاب) == منوال_info[i].بٹ) {
			إذا (all || (منوال_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(منوال_name, i));
		} وإلا {
			إذا ((all && منوال_info[i].flags & REV)
			 || (!all && (منوال_info[i].flags & (SANE_سیٹ | REV)) == (SANE_سیٹ | REV))
			) {
				wrapf("-%s", nth_string(منوال_name, i));
			}
		}
	}
	newline();
}

ساكن فراغ sane_منوال(بنية termios *منوال)
{
	صحيح i;

	لكل (i = 0; i < عدد_control_info; ++i) {
#إذا VMIN == VEOF
		إذا (i == CIDX_min)
			اكسر;
#جے_انت
		منوال->c_cc[control_info[i].وتھ] = control_info[i].saneval;
	}

	لكل (i = 0; i < عدد_منوال_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_منزل_tcflag(منوال_info[i].type, منوال);

		إذا (!bitsp)
			استمر;
		val = *bitsp & ~((بنانشانی لما)منوال_info[i].نقاب);
		إذا (منوال_info[i].flags & SANE_سیٹ) {
			*bitsp = val | منوال_info[i].بٹ;
		} وإلا
		إذا (منوال_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~منوال_info[i].بٹ;
		}
	}
}

ساكن فراغ سیٹ_منوال(ثابت بنية منوال_info *info, صحيح reversed,
					بنية termios *منوال)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_منزل_tcflag(info->type, منوال);

	إذا (bitsp) {
		tcflag_t val = *bitsp & ~info->نقاب;
		إذا (reversed)
			*bitsp = val & ~info->بٹ;
		وإلا
			*bitsp = val | info->بٹ;
		أرجع;
	}

	/* !bitsp - it's a "combination" mode */
	إذا (info == &منوال_info[IDX_evenp] || info == &منوال_info[IDX_parity]) {
		إذا (reversed)
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			منوال->c_cflag = (منوال->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} وإلا إذا (info == &منوال_info[IDX_oddp]) {
		إذا (reversed)
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} وإلا إذا (info == &منوال_info[IDX_nl]) {
		إذا (reversed) {
			منوال->c_iflag = (منوال->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			منوال->c_oflag = (منوال->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} وإلا {
			منوال->c_iflag = منوال->c_iflag & ~ICRNL;
			إذا (ONLCR) منوال->c_oflag = منوال->c_oflag & ~ONLCR;
		}
	} وإلا إذا (info == &منوال_info[IDX_ek]) {
		منوال->c_cc[VERASE] = CERASE;
		منوال->c_cc[VKILL] = CKILL;
	} وإلا إذا (info == &منوال_info[IDX_sane]) {
		sane_منوال(منوال);
	} وإلا إذا (info == &منوال_info[IDX_cbreak]) {
		إذا (reversed)
			منوال->c_lflag |= ICANON;
		وإلا
			منوال->c_lflag &= ~ICANON;
	} وإلا إذا (info == &منوال_info[IDX_pass8]) {
		إذا (reversed) {
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARENB;
			منوال->c_iflag |= ISTRIP;
		} وإلا {
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
			منوال->c_iflag &= ~ISTRIP;
		}
	} وإلا إذا (info == &منوال_info[IDX_litout]) {
		إذا (reversed) {
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARENB;
			منوال->c_iflag |= ISTRIP;
			منوال->c_oflag |= OPOST;
		} وإلا {
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
			منوال->c_iflag &= ~ISTRIP;
			منوال->c_oflag &= ~OPOST;
		}
	} وإلا إذا (info == &منوال_info[IDX_raw] || info == &منوال_info[IDX_cooked]) {
		إذا ((info == &منوال_info[IDX_raw] && reversed)
		 || (info == &منوال_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			منوال->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			منوال->c_oflag |= OPOST;
			منوال->c_lflag |= ISIG | ICANON;
#إذا VMIN == VEOF
			منوال->c_cc[VEOF] = CEOF;
#جے_انت
#إذا VTIME == VEOL
			منوال->c_cc[VEOL] = CEOL;
#جے_انت
		} وإلا {
			/* Raw mode */
			منوال->c_iflag = 0;
			منوال->c_oflag &= ~OPOST;
			منوال->c_lflag &= ~(ISIG | ICANON | XCASE);
			منوال->c_cc[VMIN] = 1;
			منوال->c_cc[VTIME] = 0;
		}
	}
#إذا IXANY
	وإلا إذا (info == &منوال_info[IDX_decctlq]) {
		إذا (reversed)
			منوال->c_iflag |= IXANY;
		وإلا
			منوال->c_iflag &= ~IXANY;
	}
#جے_انت
#إذا TABDLY
	وإلا إذا (info == &منوال_info[IDX_tabs]) {
		إذا (reversed)
			منوال->c_oflag = (منوال->c_oflag & ~TABDLY) | TAB3;
		وإلا
			منوال->c_oflag = (منوال->c_oflag & ~TABDLY) | TAB0;
	}
#جے_انت
#إذا OXTABS
	وإلا إذا (info == &منوال_info[IDX_tabs]) {
		إذا (reversed)
			منوال->c_oflag |= OXTABS;
		وإلا
			منوال->c_oflag &= ~OXTABS;
	}
#جے_انت
#إذا XCASE && IUCLC && OLCUC
	وإلا إذا (info==&منوال_info[IDX_lcase] || info==&منوال_info[IDX_LCASE]) {
		إذا (reversed) {
			منوال->c_lflag &= ~XCASE;
			منوال->c_iflag &= ~IUCLC;
			منوال->c_oflag &= ~OLCUC;
		} وإلا {
			منوال->c_lflag |= XCASE;
			منوال->c_iflag |= IUCLC;
			منوال->c_oflag |= OLCUC;
		}
	}
#جے_انت
	وإلا إذا (info == &منوال_info[IDX_crt]) {
		منوال->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} وإلا إذا (info == &منوال_info[IDX_dec]) {
		منوال->c_cc[VINTR] = 3; /* ^C */
		منوال->c_cc[VERASE] = 127; /* DEL */
		منوال->c_cc[VKILL] = 21; /* ^U */
		منوال->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		إذا (IXANY) منوال->c_iflag &= ~IXANY;
	}
}

ساكن فراغ سیٹ_control_char_or_die(ثابت بنية control_info *info,
			ثابت حرف *arg, بنية termios *منوال)
{
	بنانشانی حرف قدر_2;

	إذا (info == &control_info[CIDX_min] || info == &control_info[CIDX_ویلا])
		قدر_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	وإلا إذا (arg[0] == '\0' || arg[1] == '\0')
		قدر_2 = arg[0];
	وإلا إذا (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		قدر_2 = _POSIX_VDISABLE;
	وإلا إذا (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		قدر_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		إذا (arg[1] == '?')
			قدر_2 = 127;
	} وإلا
		قدر_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	منوال->c_cc[info->وتھ] = قدر_2;
}

#تعریف_کرو STTY_require_سیٹ_attr   (1 << 0)
#تعریف_کرو STTY_رفتار_was_سیٹ      (1 << 1)
#تعریف_کرو STTY_verbose_output     (1 << 2)
#تعریف_کرو STTY_recoverable_output (1 << 3)
#تعریف_کرو STTY_noargs             (1 << 4)

صحيح stty_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stty_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	بنية termios منوال;
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
		ثابت بنية منوال_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			صحيح i;
			mp = لبھو_منوال(arg+1);
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
					اذهب سرا_option;
				افتراضي:
					اذهب invalid_دلیل;
				}
			}
 سرا_option:
			استمر;
		}

		mp = لبھو_منوال(arg);
		إذا (mp) {
			stty_state &= ~STTY_noargs;
			استمر;
		}

		cp = لبھو_control(arg);
		إذا (cp) {
			إذا (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			سیٹ_control_char_or_die(cp, argnext, &منوال);
			stty_state &= ~STTY_noargs;
			++k;
			استمر;
		}

		param = لبھو_param(arg);
		إذا (param & param_need_arg) {
			إذا (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			++k;
		}

		بدل (param) {
#جے_تعریف_شدہ __linux__
		حالة param_سطر:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, ودھ_توں_ودھ_عدد, stty_suffixes);
			اكسر;
# endif /* else fall-through */
#جے_انت
#جے_تعریف_شدہ TIOCGWINSZ
		حالة param_صفاں:
		حالة param_خانے_گنتی:
		حالة param_columns:
			xatoul_range_sfx(argnext, 1, ودھ_توں_ودھ_عدد, stty_suffixes);
			اكسر;
		حالة param_ناپ:
#جے_انت
		حالة param_رفتار:
			اكسر;
		حالة param_ispeed:
			/* called for the side effect of xfunc death only */
			سیٹ_رفتار_or_die(داخلہ_رفتار, argnext, &منوال);
			اكسر;
		حالة param_ospeed:
			/* called for the side effect of xfunc death only */
			سیٹ_رفتار_or_die(output_رفتار, argnext, &منوال);
			اكسر;
		افتراضي:
			إذا (recover_منوال(arg, &منوال) == 1) اكسر;
			إذا (tty_قدر_منزل_baud(xatou(arg)) != (رفتار_t) -1) اكسر;
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
	memset(&منوال, 0, حجم(منوال));
	إذا (tcgetattr(STDIN_FILENO, &منوال))
		perror_on_device_and_die("%s");

	إذا (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_خانہ = get_terminal_چوڑائی(STDOUT_FILENO);
		output_func(&منوال, display_all);
		أرجع EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	طالما (argv[++k]) {
		ثابت بنية منوال_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			mp = لبھو_منوال(arg+1);
			إذا (mp) {
				سیٹ_منوال(mp, 1 /* reversed */, &منوال);
				stty_state |= STTY_require_سیٹ_attr;
			}
			/* It is an option - already parsed. Skip it */
			استمر;
		}

		mp = لبھو_منوال(arg);
		إذا (mp) {
			سیٹ_منوال(mp, 0 /* non-reversed */, &منوال);
			stty_state |= STTY_require_سیٹ_attr;
			استمر;
		}

		cp = لبھو_control(arg);
		إذا (cp) {
			++k;
			سیٹ_control_char_or_die(cp, argnext, &منوال);
			stty_state |= STTY_require_سیٹ_attr;
			استمر;
		}

		param = لبھو_param(arg);
		إذا (param & param_need_arg) {
			++k;
		}

		بدل (param) {
#جے_تعریف_شدہ __linux__
		حالة param_سطر:
			منوال.c_سطر = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_سیٹ_attr;
			اكسر;
#جے_انت
#جے_تعریف_شدہ TIOCGWINSZ
		حالة param_خانے_گنتی:
		حالة param_columns:
			سیٹ_کھڑکی_ناپ(-1, xatoul_sfx(argnext, stty_suffixes));
			اكسر;
		حالة param_ناپ:
			display_کھڑکی_ناپ(0);
			اكسر;
		حالة param_صفاں:
			سیٹ_کھڑکی_ناپ(xatoul_sfx(argnext, stty_suffixes), -1);
			اكسر;
#جے_انت
		حالة param_رفتار:
			display_رفتار(&منوال, 0);
			اكسر;
		حالة param_ispeed:
			سیٹ_رفتار_or_die(داخلہ_رفتار, argnext, &منوال);
			stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			اكسر;
		حالة param_ospeed:
			سیٹ_رفتار_or_die(output_رفتار, argnext, &منوال);
			stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			اكسر;
		افتراضي:
			إذا (recover_منوال(arg, &منوال) == 1)
				stty_state |= STTY_require_سیٹ_attr;
			وإلا /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				سیٹ_رفتار_or_die(both_speeds, arg, &منوال);
				stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	إذا (stty_state & STTY_require_سیٹ_attr) {
		بنية termios new_منوال;

		إذا (tcsetattr(STDIN_FILENO, TCSADRAIN, &منوال))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_منوال, 0, حجم(new_منوال));
		إذا (tcgetattr(STDIN_FILENO, &new_منوال))
			perror_on_device_and_die("%s");

		إذا (memcmp(&منوال, &new_منوال, حجم(منوال)) != 0) {
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

			new_منوال.c_cflag &= (~CIBAUD);
			إذا ((stty_state & STTY_رفتار_was_سیٹ)
			 || memcmp(&منوال, &new_منوال, حجم(منوال)) != 0)
#جے_انت
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	أرجع EXIT_SUCCESS;
}
