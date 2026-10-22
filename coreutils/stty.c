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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

#اگر_غیر_تعریف_شدہ _POSIX_VDISABLE
# define _POSIX_VDISABLE ((غیرعلامتی حرف) 0)
#اگر_ختم

#تعریف_کرو Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#اگر_غیر_تعریف_شدہ CINTR
# define CINTR Control('c')
#اگر_ختم
#اگر_غیر_تعریف_شدہ CQUIT
# define CQUIT 28
#اگر_ختم
#اگر_غیر_تعریف_شدہ CERASE
# define CERASE 127
#اگر_ختم
#اگر_غیر_تعریف_شدہ CKILL
# define CKILL Control('u')
#اگر_ختم
#اگر_غیر_تعریف_شدہ CEOF
# define CEOF Control('d')
#اگر_ختم
#اگر_غیر_تعریف_شدہ CEOL
# define CEOL _POSIX_VDISABLE
#اگر_ختم
#اگر_غیر_تعریف_شدہ CSTART
# define CSTART Control('q')
#اگر_ختم
#اگر_غیر_تعریف_شدہ CSTOP
# define CSTOP Control('s')
#اگر_ختم
#اگر_غیر_تعریف_شدہ CSUSP
# define CSUSP Control('z')
#اگر_ختم
#اگر defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#اگر_ختم
/* glibc-2.12.1 uses only VSWTC name */
#اگر defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#اگر_ختم
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#اگر defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#اگر_ختم
#اگر defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#اگر_ختم

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#اگر defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#اگر_ختم

#اگر defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#اگر_ختم
#اگر defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#اگر_ختم
#اگر !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#اگر_ختم
#اگر defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#اگر_ختم
#اگر defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#اگر_ختم
#اگر defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#اگر_ختم
#اگر defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#اگر_ختم
#اگر defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#اگر_ختم
#اگر defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#اگر_ختم
#اگر defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#اگر_ختم
#اگر defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#اگر_ختم
#اگر defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#اگر_ختم
#اگر defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#اگر_ختم

/* Save us from #ifdef forest plague */
#اگر_غیر_تعریف_شدہ BSDLY
# define BSDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ CIBAUD
# define CIBAUD 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ CRDLY
# define CRDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ CMSPAR
# define CMSPAR 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ CRTSCTS
# define CRTSCTS 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ECHOCTL
# define ECHOCTL 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ECHOKE
# define ECHOKE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ECHOPRT
# define ECHOPRT 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ FFDLY
# define FFDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ IEXTEN
# define IEXTEN 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ IMAXBEL
# define IMAXBEL 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ IUCLC
# define IUCLC 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ IXANY
# define IXANY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ NLDLY
# define NLDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ OCRNL
# define OCRNL 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ OFDEL
# define OFDEL 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ OFILL
# define OFILL 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ OLCUC
# define OLCUC 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ONLCR
# define ONLCR 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ONLRET
# define ONLRET 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ONOCR
# define ONOCR 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ OXTABS
# define OXTABS 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ TABDLY
# define TABDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ TAB1
# define TAB1 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ TAB2
# define TAB2 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ TOSTOP
# define TOSTOP 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VDSUSP
# define VDSUSP 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VEOL2
# define VEOL2 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VFLUSHO
# define VFLUSHO 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VLNEXT
# define VLNEXT 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VREPRINT
# define VREPRINT 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VSTATUS
# define VSTATUS 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VSWTCH
# define VSWTCH 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VTDLY
# define VTDLY 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ VWERASE
# define VWERASE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ XCASE
# define XCASE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ IUTF8
# define IUTF8 0
#اگر_ختم

/* Which speeds to set */
شمار رفتار_setting {
	داخلہ_رفتار, output_رفتار, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
شمار {
	control, داخلہ, output, local, combination
};
ساکن tcflag_t *get_ptr_مقصد_tcflag(غیرعلامتی type, ثابت ساخت termios *منوال)
{
	ساکن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(ساخت termios, c_cflag), /* control */
		offsetof(ساخت termios, c_iflag), /*داخلہ*/
		offsetof(ساخت termios, c_oflag), /* output */
		offsetof(ساخت termios, c_lflag)  /* local */
	};
	اگر (type <= local) {
		واپس (tcflag_t*) (((حرف*)منوال) + tcflag_offsets[type]);
	}
	واپس تہی;
}

/* Flags for 'struct mode_info' */
#تعریف_کرو SANE_سیٹ 1              /* Set in 'sane' mode                  */
#تعریف_کرو SANE_UNSET 2            /* Unset in 'sane' mode                */
#تعریف_کرو REV 4                   /* Can be turned off by prepending '-' */
#تعریف_کرو OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
ساخت منوال_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#اگر   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t نقاب;
#وگرنہ_اگر (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t نقاب;
#ورنہ
	ثابت tcflag_t نقاب;          /* Other bits to turn off for this mode */
#اگر_ختم
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t بٹ;          /* Bits to set for this mode            */
};

شمار {
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
#اگر IXANY
	IDX_decctlq,
#اگر_ختم
#اگر TABDLY || OXTABS
	IDX_tabs,
#اگر_ختم
#اگر XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#اگر_ختم
};

#تعریف_کرو MI_اندراج(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساکن ثابت حرف منوال_name[] ALIGN1 =
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
#اگر IXANY
	MI_اندراج("decctlq",  combination, REV        | OMIT, 0,          0 )
#اگر_ختم
#اگر TABDLY || OXTABS
	MI_اندراج("tabs",     combination, REV        | OMIT, 0,          0 )
#اگر_ختم
#اگر XCASE && IUCLC && OLCUC
	MI_اندراج("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("LCASE",    combination, REV        | OMIT, 0,          0 )
#اگر_ختم
	MI_اندراج("parenb",   control,     REV,               PARENB,     0 )
	MI_اندراج("parodd",   control,     REV,               PARODD,     0 )
#اگر CMSPAR
	MI_اندراج("cmspar",   control,     REV,               CMSPAR,     0 )
#اگر_ختم
	MI_اندراج("cs5",      control,     0,                 CS5,     CSIZE)
	MI_اندراج("cs6",      control,     0,                 CS6,     CSIZE)
	MI_اندراج("cs7",      control,     0,                 CS7,     CSIZE)
	MI_اندراج("cs8",      control,     0,                 CS8,     CSIZE)
	MI_اندراج("hupcl",    control,     REV,               HUPCL,      0 )
	MI_اندراج("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_اندراج("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_اندراج("cread",    control,     SANE_سیٹ   | REV,  CREAD,      0 )
	MI_اندراج("clocal",   control,     REV,               CLOCAL,     0 )
#اگر CRTSCTS
	MI_اندراج("crtscts",  control,     REV,               CRTSCTS,    0 )
#اگر_ختم
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
#اگر IUCLC
	MI_اندراج("iuclc",    داخلہ,       SANE_UNSET | REV,  IUCLC,      0 )
#اگر_ختم
#اگر IXANY
	MI_اندراج("ixany",    داخلہ,       SANE_UNSET | REV,  IXANY,      0 )
#اگر_ختم
#اگر IMAXBEL
	MI_اندراج("imaxbel",  داخلہ,       SANE_سیٹ   | REV,  IMAXBEL,    0 )
#اگر_ختم
#اگر IUTF8
	MI_اندراج("iutf8",    داخلہ,       SANE_UNSET | REV,  IUTF8,      0 )
#اگر_ختم
	MI_اندراج("opost",    output,      SANE_سیٹ   | REV,  OPOST,      0 )
#اگر OLCUC
	MI_اندراج("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#اگر_ختم
#اگر OCRNL
	MI_اندراج("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#اگر_ختم
#اگر ONLCR
	MI_اندراج("onlcr",    output,      SANE_سیٹ   | REV,  ONLCR,      0 )
#اگر_ختم
#اگر ONOCR
	MI_اندراج("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#اگر_ختم
#اگر ONLRET
	MI_اندراج("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#اگر_ختم
#اگر OFILL
	MI_اندراج("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#اگر_ختم
#اگر OFDEL
	MI_اندراج("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#اگر_ختم
#اگر NLDLY
	MI_اندراج("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_اندراج("nl0",      output,      SANE_سیٹ,          NL0,     NLDLY)
#اگر_ختم
#اگر CRDLY
	MI_اندراج("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_اندراج("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_اندراج("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_اندراج("cr0",      output,      SANE_سیٹ,          CR0,     CRDLY)
#اگر_ختم

#اگر TABDLY
	MI_اندراج("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# اگر TAB2
	MI_اندراج("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# اگر TAB1
	MI_اندراج("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_اندراج("tab0",     output,      SANE_سیٹ,          TAB0,   TABDLY)
#ورنہ
# اگر OXTABS
	MI_اندراج("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#اگر_ختم

#اگر BSDLY
	MI_اندراج("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_اندراج("bs0",      output,      SANE_سیٹ,          BS0,     BSDLY)
#اگر_ختم
#اگر VTDLY
	MI_اندراج("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_اندراج("vt0",      output,      SANE_سیٹ,          VT0,     VTDLY)
#اگر_ختم
#اگر FFDLY
	MI_اندراج("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_اندراج("ff0",      output,      SANE_سیٹ,          FF0,     FFDLY)
#اگر_ختم
	MI_اندراج("isig",     local,       SANE_سیٹ   | REV,  ISIG,       0 )
	MI_اندراج("icanon",   local,       SANE_سیٹ   | REV,  ICANON,     0 )
#اگر IEXTEN
	MI_اندراج("iexten",   local,       SANE_سیٹ   | REV,  IEXTEN,     0 )
#اگر_ختم
	MI_اندراج("echo",     local,       SANE_سیٹ   | REV,  ECHO,       0 )
	MI_اندراج("echoe",    local,       SANE_سیٹ   | REV,  ECHOE,      0 )
	MI_اندراج("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_اندراج("echok",    local,       SANE_سیٹ   | REV,  ECHOK,      0 )
	MI_اندراج("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_اندراج("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#اگر XCASE
	MI_اندراج("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#اگر_ختم
#اگر TOSTOP
	MI_اندراج("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#اگر_ختم
#اگر ECHOPRT
	MI_اندراج("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_اندراج("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#اگر_ختم
#اگر ECHOCTL
	MI_اندراج("echoctl",  local,       SANE_سیٹ   | REV,  ECHOCTL,    0 )
	MI_اندراج("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#اگر_ختم
#اگر ECHOKE
	MI_اندراج("echoke",   local,       SANE_سیٹ   | REV,  ECHOKE,     0 )
	MI_اندراج("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#اگر_ختم
	MI_اندراج("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#اگر_تعریف_شدہ EXTPROC
	MI_اندراج("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#اگر_ختم
	;

#تعریف_منسوخ MI_اندراج
#تعریف_کرو MI_اندراج(N,T,F,B,M) { T, F, M, B },

ساکن ثابت ساخت منوال_info منوال_info[] ALIGN4 = {
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
#اگر IXANY
	MI_اندراج("decctlq",  combination, REV        | OMIT, 0,          0 )
#اگر_ختم
#اگر TABDLY || OXTABS
	MI_اندراج("tabs",     combination, REV        | OMIT, 0,          0 )
#اگر_ختم
#اگر XCASE && IUCLC && OLCUC
	MI_اندراج("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_اندراج("LCASE",    combination, REV        | OMIT, 0,          0 )
#اگر_ختم
	MI_اندراج("parenb",   control,     REV,               PARENB,     0 )
	MI_اندراج("parodd",   control,     REV,               PARODD,     0 )
#اگر CMSPAR
	MI_اندراج("cmspar",   control,     REV,               CMSPAR,     0 )
#اگر_ختم
	MI_اندراج("cs5",      control,     0,                 CS5,     CSIZE)
	MI_اندراج("cs6",      control,     0,                 CS6,     CSIZE)
	MI_اندراج("cs7",      control,     0,                 CS7,     CSIZE)
	MI_اندراج("cs8",      control,     0,                 CS8,     CSIZE)
	MI_اندراج("hupcl",    control,     REV,               HUPCL,      0 )
	MI_اندراج("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_اندراج("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_اندراج("cread",    control,     SANE_سیٹ   | REV,  CREAD,      0 )
	MI_اندراج("clocal",   control,     REV,               CLOCAL,     0 )
#اگر CRTSCTS
	MI_اندراج("crtscts",  control,     REV,               CRTSCTS,    0 )
#اگر_ختم
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
#اگر IUCLC
	MI_اندراج("iuclc",    داخلہ,       SANE_UNSET | REV,  IUCLC,      0 )
#اگر_ختم
#اگر IXANY
	MI_اندراج("ixany",    داخلہ,       SANE_UNSET | REV,  IXANY,      0 )
#اگر_ختم
#اگر IMAXBEL
	MI_اندراج("imaxbel",  داخلہ,       SANE_سیٹ   | REV,  IMAXBEL,    0 )
#اگر_ختم
#اگر IUTF8
	MI_اندراج("iutf8",    داخلہ,       SANE_UNSET | REV,  IUTF8,      0 )
#اگر_ختم
	MI_اندراج("opost",    output,      SANE_سیٹ   | REV,  OPOST,      0 )
#اگر OLCUC
	MI_اندراج("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#اگر_ختم
#اگر OCRNL
	MI_اندراج("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#اگر_ختم
#اگر ONLCR
	MI_اندراج("onlcr",    output,      SANE_سیٹ   | REV,  ONLCR,      0 )
#اگر_ختم
#اگر ONOCR
	MI_اندراج("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#اگر_ختم
#اگر ONLRET
	MI_اندراج("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#اگر_ختم
#اگر OFILL
	MI_اندراج("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#اگر_ختم
#اگر OFDEL
	MI_اندراج("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#اگر_ختم
#اگر NLDLY
	MI_اندراج("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_اندراج("nl0",      output,      SANE_سیٹ,          NL0,     NLDLY)
#اگر_ختم
#اگر CRDLY
	MI_اندراج("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_اندراج("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_اندراج("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_اندراج("cr0",      output,      SANE_سیٹ,          CR0,     CRDLY)
#اگر_ختم

#اگر TABDLY
	MI_اندراج("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# اگر TAB2
	MI_اندراج("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# اگر TAB1
	MI_اندراج("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_اندراج("tab0",     output,      SANE_سیٹ,          TAB0,   TABDLY)
#ورنہ
# اگر OXTABS
	MI_اندراج("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#اگر_ختم

#اگر BSDLY
	MI_اندراج("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_اندراج("bs0",      output,      SANE_سیٹ,          BS0,     BSDLY)
#اگر_ختم
#اگر VTDLY
	MI_اندراج("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_اندراج("vt0",      output,      SANE_سیٹ,          VT0,     VTDLY)
#اگر_ختم
#اگر FFDLY
	MI_اندراج("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_اندراج("ff0",      output,      SANE_سیٹ,          FF0,     FFDLY)
#اگر_ختم
	MI_اندراج("isig",     local,       SANE_سیٹ   | REV,  ISIG,       0 )
	MI_اندراج("icanon",   local,       SANE_سیٹ   | REV,  ICANON,     0 )
#اگر IEXTEN
	MI_اندراج("iexten",   local,       SANE_سیٹ   | REV,  IEXTEN,     0 )
#اگر_ختم
	MI_اندراج("echo",     local,       SANE_سیٹ   | REV,  ECHO,       0 )
	MI_اندراج("echoe",    local,       SANE_سیٹ   | REV,  ECHOE,      0 )
	MI_اندراج("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_اندراج("echok",    local,       SANE_سیٹ   | REV,  ECHOK,      0 )
	MI_اندراج("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_اندراج("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#اگر XCASE
	MI_اندراج("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#اگر_ختم
#اگر TOSTOP
	MI_اندراج("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#اگر_ختم
#اگر ECHOPRT
	MI_اندراج("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_اندراج("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#اگر_ختم
#اگر ECHOCTL
	MI_اندراج("echoctl",  local,       SANE_سیٹ   | REV,  ECHOCTL,    0 )
	MI_اندراج("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#اگر_ختم
#اگر ECHOKE
	MI_اندراج("echoke",   local,       SANE_سیٹ   | REV,  ECHOKE,     0 )
	MI_اندراج("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#اگر_ختم
	MI_اندراج("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#اگر_تعریف_شدہ EXTPROC
	MI_اندراج("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#اگر_ختم
};

شمار {
	عدد_منوال_info = ARRAY_سائز(منوال_info)
};


/* Control characters */
ساخت control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t انحراف;   /* Offset in c_cc */
};

شمار {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#اگر VEOL2
	CIDX_eol2,
#اگر_ختم
#اگر VSWTCH
	CIDX_swtch,
#اگر_ختم
	CIDX_آغاز,
	CIDX_stop,
	CIDX_susp,
#اگر VDSUSP
	CIDX_dsusp,
#اگر_ختم
#اگر VREPRINT
	CIDX_rprnt,
#اگر_ختم
#اگر VWERASE
	CIDX_werase,
#اگر_ختم
#اگر VLNEXT
	CIDX_lnext,
#اگر_ختم
#اگر VFLUSHO
	CIDX_flush,
#اگر_ختم
#اگر VSTATUS
	CIDX_حالت,
#اگر_ختم
	CIDX_min,
	CIDX_وقت,
};

#تعریف_کرو CI_اندراج(n,s,o) n "\0"

/* Name given on command line */
ساکن ثابت حرف control_name[] ALIGN1 =
	CI_اندراج("intr",     CINTR,   VINTR   )
	CI_اندراج("quit",     CQUIT,   VQUIT   )
	CI_اندراج("erase",    CERASE,  VERASE  )
	CI_اندراج("kill",     CKILL,   VKILL   )
	CI_اندراج("eof",      CEOF,    VEOF    )
	CI_اندراج("eol",      CEOL,    VEOL    )
#اگر VEOL2
	CI_اندراج("eol2",     CEOL2,   VEOL2   )
#اگر_ختم
#اگر VSWTCH
	CI_اندراج("swtch",    CSWTCH,  VSWTCH  )
#اگر_ختم
	CI_اندراج("start",    CSTART,  VSTART  )
	CI_اندراج("stop",     CSTOP,   VSTOP   )
	CI_اندراج("susp",     CSUSP,   VSUSP   )
#اگر VDSUSP
	CI_اندراج("dsusp",    CDSUSP,  VDSUSP  )
#اگر_ختم
#اگر VREPRINT
	CI_اندراج("rprnt",    CRPRNT,  VREPRINT)
#اگر_ختم
#اگر VWERASE
	CI_اندراج("werase",   CWERASE, VWERASE )
#اگر_ختم
#اگر VLNEXT
	CI_اندراج("lnext",    CLNEXT,  VLNEXT  )
#اگر_ختم
#اگر VFLUSHO
	CI_اندراج("flush",    CFLUSHO, VFLUSHO )
#اگر_ختم
#اگر VSTATUS
	CI_اندراج("status",   CSTATUS, VSTATUS )
#اگر_ختم
	/* These must be last because of the display routines */
	CI_اندراج("min",      1,       VMIN    )
	CI_اندراج("time",     0,       VTIME   )
	;

#تعریف_منسوخ CI_اندراج
#تعریف_کرو CI_اندراج(n,s,o) { s, o },

ساکن ثابت ساخت control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_اندراج("intr",     CINTR,   VINTR   )
	CI_اندراج("quit",     CQUIT,   VQUIT   )
	CI_اندراج("erase",    CERASE,  VERASE  )
	CI_اندراج("kill",     CKILL,   VKILL   )
	CI_اندراج("eof",      CEOF,    VEOF    )
	CI_اندراج("eol",      CEOL,    VEOL    )
#اگر VEOL2
	CI_اندراج("eol2",     CEOL2,   VEOL2   )
#اگر_ختم
#اگر VSWTCH
	CI_اندراج("swtch",    CSWTCH,  VSWTCH  )
#اگر_ختم
	CI_اندراج("start",    CSTART,  VSTART  )
	CI_اندراج("stop",     CSTOP,   VSTOP   )
	CI_اندراج("susp",     CSUSP,   VSUSP   )
#اگر VDSUSP
	CI_اندراج("dsusp",    CDSUSP,  VDSUSP  )
#اگر_ختم
#اگر VREPRINT
	CI_اندراج("rprnt",    CRPRNT,  VREPRINT)
#اگر_ختم
#اگر VWERASE
	CI_اندراج("werase",   CWERASE, VWERASE )
#اگر_ختم
#اگر VLNEXT
	CI_اندراج("lnext",    CLNEXT,  VLNEXT  )
#اگر_ختم
#اگر VFLUSHO
	CI_اندراج("flush",    CFLUSHO, VFLUSHO )
#اگر_ختم
#اگر VSTATUS
	CI_اندراج("status",   CSTATUS, VSTATUS )
#اگر_ختم
	/* These must be last because of the display routines */
	CI_اندراج("min",      1,       VMIN    )
	CI_اندراج("time",     0,       VTIME   )
};

شمار {
	عدد_control_info = ARRAY_سائز(control_info)
};


ساخت globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	غیرعلامتی max_ستون;
	/* Current position, to know when to wrap */
	غیرعلامتی current_ستون;
} FIX_ALIASING;
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	G.device_name = bb_پیغام_standard_داخلہ; \
	G.max_ستون = 80; \
	G.current_ستون = 0; /* we are noexec, must clear */ \
} جب (0)

ساکن خالی سیٹ_رفتار_or_die(شمار رفتار_setting type, ثابت حرف *arg,
					ساخت termios *منوال)
{
	رفتار_t baud;

	baud = tty_قدر_مقصد_baud(xatou(arg));

	اگر (type != output_رفتار) {     /* either input or both */
		cfsetispeed(منوال, baud);
	}
	اگر (type != داخلہ_رفتار) {      /* either output or both */
		cfsetospeed(منوال, baud);
	}
}

ساکن NORETURN خالی perror_on_device_and_die(ثابت حرف *fmt)
{
	bb_perror_پیغام_and_die(fmt, G.device_name);
}

ساکن خالی perror_on_device(ثابت حرف *fmt)
{
	bb_perror_پیغام(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ساکن خالی wrapf(ثابت حرف *پیغام_2, ...)
{
	حرف buf[128];
	va_list دلائل;
	غیرعلامتی buflen;

	va_آغاز(دلائل, پیغام_2);
	buflen = vsnprintf(buf, حجم(buf), پیغام_2, دلائل);
	va_انتہا(دلائل);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	اگر (!buflen || buflen >= حجم(buf)) واپس;

	اگر (G.current_ستون > 0) {
		G.current_ستون++;
		اگر (buf[0] != '\n') {
			اگر (G.current_ستون + buflen >= G.max_ستون) {
				G.current_ستون = 0;
				bb_putchar('\n');
			} ورنہ {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_ستون += buflen;
	اگر (buf[buflen-1] == '\n')
		G.current_ستون = 0;
}

ساکن خالی newline(خالی)
{
	اگر (G.current_ستون != 0)
		wrapf("\n");
}

#اگر_تعریف_شدہ TIOCGWINSZ
ساکن خالی سیٹ_کھڑکی_سائز(صحیح صفیں, صحیح ستون_تعداد)
{
	ساخت winsize win = { 0, 0, 0, 0 };

	اگر (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		اگر (errno != EINVAL) {
			جاؤ bail;
		}
		memset(&win, 0, حجم(win));
	}

	اگر (صفیں >= 0)
		win.ws_صف = صفیں;
	اگر (ستون_تعداد >= 0)
		win.ws_ستون = ستون_تعداد;

	اگر (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#اگر_ختم

ساکن خالی display_کھڑکی_سائز(صحیح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	غیرعلامتی چوڑائی, اونچائی;

	اگر (get_terminal_چوڑائی_اونچائی(STDIN_FILENO, &چوڑائی, &اونچائی)) {
		اگر ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} ورنہ {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				اونچائی, چوڑائی);
	}
}

ساکن ثابت ساخت suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساکن ثابت ساخت منوال_info *ڈھونڈو_منوال(ثابت حرف *name)
{
	صحیح i = اشاریہ_اندر_لڑیاں(منوال_name, name);
	واپس i >= 0 ? &منوال_info[i] : تہی;
}

ساکن ثابت ساخت control_info *ڈھونڈو_control(ثابت حرف *name)
{
	صحیح i = اشاریہ_اندر_لڑیاں(control_name, name);
	واپس i >= 0 ? &control_info[i] : تہی;
}

شمار {
	param_need_arg = 0x80,
	param_سطر    = 1 | 0x80,
	param_صفیں    = 2 | 0x80,
	param_ستون_تعداد    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_سائز    = 5,
	param_رفتار   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساکن صحیح ڈھونڈو_param(ثابت حرف *name)
{
	ساکن ثابت حرف params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	صحیح i = اشاریہ_اندر_لڑیاں(params, name) + 1;
	اگر (i == 0)
		واپس 0;
	اگر (i != 5 && i != 6)
		i |= 0x80;
	واپس i;
}

ساکن صحیح recover_منوال(ثابت حرف *arg, ساخت termios *منوال)
{
	صحیح i, n;
	غیرعلامتی chr;
	غیرعلامتی طویل iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	اگر (لڑی_سے_پڑھو(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		واپس 0;
	منوال->c_iflag = iflag;
	منوال->c_oflag = oflag;
	منوال->c_cflag = cflag;
	منوال->c_lflag = lflag;
	arg += n;
	برائے (i = 0; i < NCCS; ++i) {
		اگر (لڑی_سے_پڑھو(arg, ":%x%n", &chr, &n) != 1)
			واپس 0;
		منوال->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	اگر (*arg != '\0')
		واپس 0;

	واپس 1;
}

ساکن خالی display_recoverable(ثابت ساخت termios *منوال,
				صحیح غیر_مستعمل_PARAM dummy)
{
	صحیح i;
	لکھو("%lx:%lx:%lx:%lx",
		   (غیرعلامتی طویل) منوال->c_iflag, (غیرعلامتی طویل) منوال->c_oflag,
		   (غیرعلامتی طویل) منوال->c_cflag, (غیرعلامتی طویل) منوال->c_lflag);
	برائے (i = 0; i < NCCS; ++i)
		لکھو(":%x", (غیرعلامتی صحیح) منوال->c_cc[i]);
	bb_putchar('\n');
}

ساکن خالی display_رفتار(ثابت ساخت termios *منوال, صحیح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	غیرعلامتی طویل ispeed, ospeed;

	ispeed = cfgetispeed(منوال);
	ospeed = cfgetospeed(منوال);
	اگر (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	اگر (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_مقصد_قدر(ispeed), tty_baud_مقصد_قدر(ospeed));
}

ساکن خالی do_display(ثابت ساخت termios *منوال, صحیح all)
{
	صحیح i;
	tcflag_t *bitsp;
	غیرعلامتی طویل نقاب;
	صحیح پچھلا_type = control;

	display_رفتار(منوال, 1);
	اگر (all)
		display_کھڑکی_سائز(1);
#اگر_تعریف_شدہ __linux__
	wrapf("line = %u;\n", منوال->c_سطر);
#ورنہ
	newline();
#اگر_ختم

	برائے (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#اگر VSWTCH == VSUSP
		اگر (i == CIDX_swtch)
			جاری;
#اگر_ختم
		/* If eof uses the same slot as min, only print whichever applies */
#اگر VEOF == VMIN
		اگر (!(منوال->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			جاری;
		}
#اگر_ختم
		ch = منوال->c_cc[control_info[i].انحراف];
		اگر (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		ورنہ
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#اگر VEOF == VMIN
	اگر ((منوال->c_lflag & ICANON) == 0)
#اگر_ختم
		wrapf("min = %u; time = %u;", منوال->c_cc[VMIN], منوال->c_cc[VTIME]);
	newline();

	برائے (i = 0; i < عدد_منوال_info; ++i) {
		اگر (منوال_info[i].flags & OMIT)
			جاری;
		اگر (منوال_info[i].type != پچھلا_type) {
			newline();
			پچھلا_type = منوال_info[i].type;
		}

		bitsp = get_ptr_مقصد_tcflag(منوال_info[i].type, منوال);
		نقاب = منوال_info[i].نقاب ? منوال_info[i].نقاب : منوال_info[i].بٹ;
		اگر ((*bitsp & نقاب) == منوال_info[i].بٹ) {
			اگر (all || (منوال_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(منوال_name, i));
		} ورنہ {
			اگر ((all && منوال_info[i].flags & REV)
			 || (!all && (منوال_info[i].flags & (SANE_سیٹ | REV)) == (SANE_سیٹ | REV))
			) {
				wrapf("-%s", nth_string(منوال_name, i));
			}
		}
	}
	newline();
}

ساکن خالی sane_منوال(ساخت termios *منوال)
{
	صحیح i;

	برائے (i = 0; i < عدد_control_info; ++i) {
#اگر VMIN == VEOF
		اگر (i == CIDX_min)
			توڑو;
#اگر_ختم
		منوال->c_cc[control_info[i].انحراف] = control_info[i].saneval;
	}

	برائے (i = 0; i < عدد_منوال_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_مقصد_tcflag(منوال_info[i].type, منوال);

		اگر (!bitsp)
			جاری;
		val = *bitsp & ~((غیرعلامتی طویل)منوال_info[i].نقاب);
		اگر (منوال_info[i].flags & SANE_سیٹ) {
			*bitsp = val | منوال_info[i].بٹ;
		} ورنہ
		اگر (منوال_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~منوال_info[i].بٹ;
		}
	}
}

ساکن خالی سیٹ_منوال(ثابت ساخت منوال_info *info, صحیح reversed,
					ساخت termios *منوال)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_مقصد_tcflag(info->type, منوال);

	اگر (bitsp) {
		tcflag_t val = *bitsp & ~info->نقاب;
		اگر (reversed)
			*bitsp = val & ~info->بٹ;
		ورنہ
			*bitsp = val | info->بٹ;
		واپس;
	}

	/* !bitsp - it's a "combination" mode */
	اگر (info == &منوال_info[IDX_evenp] || info == &منوال_info[IDX_parity]) {
		اگر (reversed)
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ورنہ
			منوال->c_cflag = (منوال->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} ورنہ اگر (info == &منوال_info[IDX_oddp]) {
		اگر (reversed)
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ورنہ
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} ورنہ اگر (info == &منوال_info[IDX_nl]) {
		اگر (reversed) {
			منوال->c_iflag = (منوال->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			منوال->c_oflag = (منوال->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} ورنہ {
			منوال->c_iflag = منوال->c_iflag & ~ICRNL;
			اگر (ONLCR) منوال->c_oflag = منوال->c_oflag & ~ONLCR;
		}
	} ورنہ اگر (info == &منوال_info[IDX_ek]) {
		منوال->c_cc[VERASE] = CERASE;
		منوال->c_cc[VKILL] = CKILL;
	} ورنہ اگر (info == &منوال_info[IDX_sane]) {
		sane_منوال(منوال);
	} ورنہ اگر (info == &منوال_info[IDX_cbreak]) {
		اگر (reversed)
			منوال->c_lflag |= ICANON;
		ورنہ
			منوال->c_lflag &= ~ICANON;
	} ورنہ اگر (info == &منوال_info[IDX_pass8]) {
		اگر (reversed) {
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARENB;
			منوال->c_iflag |= ISTRIP;
		} ورنہ {
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
			منوال->c_iflag &= ~ISTRIP;
		}
	} ورنہ اگر (info == &منوال_info[IDX_litout]) {
		اگر (reversed) {
			منوال->c_cflag = (منوال->c_cflag & ~CSIZE) | CS7 | PARENB;
			منوال->c_iflag |= ISTRIP;
			منوال->c_oflag |= OPOST;
		} ورنہ {
			منوال->c_cflag = (منوال->c_cflag & ~PARENB & ~CSIZE) | CS8;
			منوال->c_iflag &= ~ISTRIP;
			منوال->c_oflag &= ~OPOST;
		}
	} ورنہ اگر (info == &منوال_info[IDX_raw] || info == &منوال_info[IDX_cooked]) {
		اگر ((info == &منوال_info[IDX_raw] && reversed)
		 || (info == &منوال_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			منوال->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			منوال->c_oflag |= OPOST;
			منوال->c_lflag |= ISIG | ICANON;
#اگر VMIN == VEOF
			منوال->c_cc[VEOF] = CEOF;
#اگر_ختم
#اگر VTIME == VEOL
			منوال->c_cc[VEOL] = CEOL;
#اگر_ختم
		} ورنہ {
			/* Raw mode */
			منوال->c_iflag = 0;
			منوال->c_oflag &= ~OPOST;
			منوال->c_lflag &= ~(ISIG | ICANON | XCASE);
			منوال->c_cc[VMIN] = 1;
			منوال->c_cc[VTIME] = 0;
		}
	}
#اگر IXANY
	ورنہ اگر (info == &منوال_info[IDX_decctlq]) {
		اگر (reversed)
			منوال->c_iflag |= IXANY;
		ورنہ
			منوال->c_iflag &= ~IXANY;
	}
#اگر_ختم
#اگر TABDLY
	ورنہ اگر (info == &منوال_info[IDX_tabs]) {
		اگر (reversed)
			منوال->c_oflag = (منوال->c_oflag & ~TABDLY) | TAB3;
		ورنہ
			منوال->c_oflag = (منوال->c_oflag & ~TABDLY) | TAB0;
	}
#اگر_ختم
#اگر OXTABS
	ورنہ اگر (info == &منوال_info[IDX_tabs]) {
		اگر (reversed)
			منوال->c_oflag |= OXTABS;
		ورنہ
			منوال->c_oflag &= ~OXTABS;
	}
#اگر_ختم
#اگر XCASE && IUCLC && OLCUC
	ورنہ اگر (info==&منوال_info[IDX_lcase] || info==&منوال_info[IDX_LCASE]) {
		اگر (reversed) {
			منوال->c_lflag &= ~XCASE;
			منوال->c_iflag &= ~IUCLC;
			منوال->c_oflag &= ~OLCUC;
		} ورنہ {
			منوال->c_lflag |= XCASE;
			منوال->c_iflag |= IUCLC;
			منوال->c_oflag |= OLCUC;
		}
	}
#اگر_ختم
	ورنہ اگر (info == &منوال_info[IDX_crt]) {
		منوال->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} ورنہ اگر (info == &منوال_info[IDX_dec]) {
		منوال->c_cc[VINTR] = 3; /* ^C */
		منوال->c_cc[VERASE] = 127; /* DEL */
		منوال->c_cc[VKILL] = 21; /* ^U */
		منوال->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		اگر (IXANY) منوال->c_iflag &= ~IXANY;
	}
}

ساکن خالی سیٹ_control_char_or_die(ثابت ساخت control_info *info,
			ثابت حرف *arg, ساخت termios *منوال)
{
	غیرعلامتی حرف قدر_2;

	اگر (info == &control_info[CIDX_min] || info == &control_info[CIDX_وقت])
		قدر_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ورنہ اگر (arg[0] == '\0' || arg[1] == '\0')
		قدر_2 = arg[0];
	ورنہ اگر (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		قدر_2 = _POSIX_VDISABLE;
	ورنہ اگر (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		قدر_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		اگر (arg[1] == '?')
			قدر_2 = 127;
	} ورنہ
		قدر_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	منوال->c_cc[info->انحراف] = قدر_2;
}

#تعریف_کرو STTY_require_سیٹ_attr   (1 << 0)
#تعریف_کرو STTY_رفتار_was_سیٹ      (1 << 1)
#تعریف_کرو STTY_verbose_output     (1 << 2)
#تعریف_کرو STTY_recoverable_output (1 << 3)
#تعریف_کرو STTY_noargs             (1 << 4)

صحیح stty_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح stty_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	ساخت termios منوال;
	خالی (*output_func)(ثابت ساخت termios *, صحیح);
	ثابت حرف *file_name = تہی;
	صحیح display_all = 0;
	صحیح stty_state;
	صحیح k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	جب (argv[++k]) {
		ثابت ساخت منوال_info *mp;
		ثابت ساخت control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحیح param;

		اگر (arg[0] == '-') {
			صحیح i;
			mp = ڈھونڈو_منوال(arg+1);
			اگر (mp) {
				اگر (!(mp->flags & REV))
					جاؤ invalid_دلیل;
				stty_state &= ~STTY_noargs;
				جاری;
			}
			/* It is an option - parse it */
			i = 0;
			جب (arg[++i]) {
				انتخاب (arg[i]) {
				حالت 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					توڑو;
				حالت 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					توڑو;
				حالت 'F':
					اگر (file_name)
						bb_simple_error_پیغام_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					اگر (!file_name[0]) { /* nope, "-F device" */
						صحیح p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						اگر (!file_name)
							bb_error_پیغام_and_die(bb_پیغام_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						جب (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					جاؤ انتہا_option;
				طےشدہ:
					جاؤ invalid_دلیل;
				}
			}
 انتہا_option:
			جاری;
		}

		mp = ڈھونڈو_منوال(arg);
		اگر (mp) {
			stty_state &= ~STTY_noargs;
			جاری;
		}

		cp = ڈھونڈو_control(arg);
		اگر (cp) {
			اگر (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			سیٹ_control_char_or_die(cp, argnext, &منوال);
			stty_state &= ~STTY_noargs;
			++k;
			جاری;
		}

		param = ڈھونڈو_param(arg);
		اگر (param & param_need_arg) {
			اگر (!argnext)
				bb_error_پیغام_and_die(bb_پیغام_requires_arg, arg);
			++k;
		}

		انتخاب (param) {
#اگر_تعریف_شدہ __linux__
		حالت param_سطر:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, زیادہ_سے_زیادہ_عدد, stty_suffixes);
			توڑو;
# endif /* else fall-through */
#اگر_ختم
#اگر_تعریف_شدہ TIOCGWINSZ
		حالت param_صفیں:
		حالت param_ستون_تعداد:
		حالت param_columns:
			xatoul_range_sfx(argnext, 1, زیادہ_سے_زیادہ_عدد, stty_suffixes);
			توڑو;
		حالت param_سائز:
#اگر_ختم
		حالت param_رفتار:
			توڑو;
		حالت param_ispeed:
			/* called for the side effect of xfunc death only */
			سیٹ_رفتار_or_die(داخلہ_رفتار, argnext, &منوال);
			توڑو;
		حالت param_ospeed:
			/* called for the side effect of xfunc death only */
			سیٹ_رفتار_or_die(output_رفتار, argnext, &منوال);
			توڑو;
		طےشدہ:
			اگر (recover_منوال(arg, &منوال) == 1) توڑو;
			اگر (tty_قدر_مقصد_baud(xatou(arg)) != (رفتار_t) -1) توڑو;
 invalid_دلیل:
			bb_error_پیغام_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	اگر ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_پیغام_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	اگر ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_پیغام_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	اگر (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&منوال, 0, حجم(منوال));
	اگر (tcgetattr(STDIN_FILENO, &منوال))
		perror_on_device_and_die("%s");

	اگر (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_ستون = get_terminal_چوڑائی(STDOUT_FILENO);
		output_func(&منوال, display_all);
		واپس EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	جب (argv[++k]) {
		ثابت ساخت منوال_info *mp;
		ثابت ساخت control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحیح param;

		اگر (arg[0] == '-') {
			mp = ڈھونڈو_منوال(arg+1);
			اگر (mp) {
				سیٹ_منوال(mp, 1 /* reversed */, &منوال);
				stty_state |= STTY_require_سیٹ_attr;
			}
			/* It is an option - already parsed. Skip it */
			جاری;
		}

		mp = ڈھونڈو_منوال(arg);
		اگر (mp) {
			سیٹ_منوال(mp, 0 /* non-reversed */, &منوال);
			stty_state |= STTY_require_سیٹ_attr;
			جاری;
		}

		cp = ڈھونڈو_control(arg);
		اگر (cp) {
			++k;
			سیٹ_control_char_or_die(cp, argnext, &منوال);
			stty_state |= STTY_require_سیٹ_attr;
			جاری;
		}

		param = ڈھونڈو_param(arg);
		اگر (param & param_need_arg) {
			++k;
		}

		انتخاب (param) {
#اگر_تعریف_شدہ __linux__
		حالت param_سطر:
			منوال.c_سطر = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_سیٹ_attr;
			توڑو;
#اگر_ختم
#اگر_تعریف_شدہ TIOCGWINSZ
		حالت param_ستون_تعداد:
		حالت param_columns:
			سیٹ_کھڑکی_سائز(-1, xatoul_sfx(argnext, stty_suffixes));
			توڑو;
		حالت param_سائز:
			display_کھڑکی_سائز(0);
			توڑو;
		حالت param_صفیں:
			سیٹ_کھڑکی_سائز(xatoul_sfx(argnext, stty_suffixes), -1);
			توڑو;
#اگر_ختم
		حالت param_رفتار:
			display_رفتار(&منوال, 0);
			توڑو;
		حالت param_ispeed:
			سیٹ_رفتار_or_die(داخلہ_رفتار, argnext, &منوال);
			stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			توڑو;
		حالت param_ospeed:
			سیٹ_رفتار_or_die(output_رفتار, argnext, &منوال);
			stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			توڑو;
		طےشدہ:
			اگر (recover_منوال(arg, &منوال) == 1)
				stty_state |= STTY_require_سیٹ_attr;
			ورنہ /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				سیٹ_رفتار_or_die(both_speeds, arg, &منوال);
				stty_state |= (STTY_require_سیٹ_attr | STTY_رفتار_was_سیٹ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	اگر (stty_state & STTY_require_سیٹ_attr) {
		ساخت termios new_منوال;

		اگر (tcsetattr(STDIN_FILENO, TCSADRAIN, &منوال))
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
		اگر (tcgetattr(STDIN_FILENO, &new_منوال))
			perror_on_device_and_die("%s");

		اگر (memcmp(&منوال, &new_منوال, حجم(منوال)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#اگر 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_منوال.c_cflag &= (~CIBAUD);
			اگر ((stty_state & STTY_رفتار_was_سیٹ)
			 || memcmp(&منوال, &new_منوال, حجم(منوال)) != 0)
#اگر_ختم
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	واپس EXIT_SUCCESS;
}
