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

#اگر_تعریف_نشده _POSIX_VDISABLE
# define _POSIX_VDISABLE ((بیعلامت حرف) 0)
#پایان_اگر

#تعریف_کن Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#اگر_تعریف_نشده CINTR
# define CINTR Control('c')
#پایان_اگر
#اگر_تعریف_نشده CQUIT
# define CQUIT 28
#پایان_اگر
#اگر_تعریف_نشده CERASE
# define CERASE 127
#پایان_اگر
#اگر_تعریف_نشده CKILL
# define CKILL Control('u')
#پایان_اگر
#اگر_تعریف_نشده CEOF
# define CEOF Control('d')
#پایان_اگر
#اگر_تعریف_نشده CEOL
# define CEOL _POSIX_VDISABLE
#پایان_اگر
#اگر_تعریف_نشده CSTART
# define CSTART Control('q')
#پایان_اگر
#اگر_تعریف_نشده CSTOP
# define CSTOP Control('s')
#پایان_اگر
#اگر_تعریف_نشده CSUSP
# define CSUSP Control('z')
#پایان_اگر
#إذا defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#پایان_اگر
/* glibc-2.12.1 uses only VSWTC name */
#إذا defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#پایان_اگر
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#إذا defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#پایان_اگر
#إذا defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#پایان_اگر

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#إذا defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#پایان_اگر

#إذا defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#پایان_اگر
#إذا defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#پایان_اگر
#إذا !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#پایان_اگر
#إذا defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#پایان_اگر
#إذا defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#پایان_اگر
#إذا defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#پایان_اگر
#إذا defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#پایان_اگر
#إذا defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#پایان_اگر
#إذا defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#پایان_اگر
#إذا defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#پایان_اگر
#إذا defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#پایان_اگر
#إذا defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#پایان_اگر
#إذا defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#پایان_اگر

/* Save us from #ifdef forest plague */
#اگر_تعریف_نشده BSDLY
# define BSDLY 0
#پایان_اگر
#اگر_تعریف_نشده CIBAUD
# define CIBAUD 0
#پایان_اگر
#اگر_تعریف_نشده CRDLY
# define CRDLY 0
#پایان_اگر
#اگر_تعریف_نشده CMSPAR
# define CMSPAR 0
#پایان_اگر
#اگر_تعریف_نشده CRTSCTS
# define CRTSCTS 0
#پایان_اگر
#اگر_تعریف_نشده ECHOCTL
# define ECHOCTL 0
#پایان_اگر
#اگر_تعریف_نشده ECHOKE
# define ECHOKE 0
#پایان_اگر
#اگر_تعریف_نشده ECHOPRT
# define ECHOPRT 0
#پایان_اگر
#اگر_تعریف_نشده FFDLY
# define FFDLY 0
#پایان_اگر
#اگر_تعریف_نشده IEXTEN
# define IEXTEN 0
#پایان_اگر
#اگر_تعریف_نشده IMAXBEL
# define IMAXBEL 0
#پایان_اگر
#اگر_تعریف_نشده IUCLC
# define IUCLC 0
#پایان_اگر
#اگر_تعریف_نشده IXANY
# define IXANY 0
#پایان_اگر
#اگر_تعریف_نشده NLDLY
# define NLDLY 0
#پایان_اگر
#اگر_تعریف_نشده OCRNL
# define OCRNL 0
#پایان_اگر
#اگر_تعریف_نشده OFDEL
# define OFDEL 0
#پایان_اگر
#اگر_تعریف_نشده OFILL
# define OFILL 0
#پایان_اگر
#اگر_تعریف_نشده OLCUC
# define OLCUC 0
#پایان_اگر
#اگر_تعریف_نشده ONLCR
# define ONLCR 0
#پایان_اگر
#اگر_تعریف_نشده ONLRET
# define ONLRET 0
#پایان_اگر
#اگر_تعریف_نشده ONOCR
# define ONOCR 0
#پایان_اگر
#اگر_تعریف_نشده OXTABS
# define OXTABS 0
#پایان_اگر
#اگر_تعریف_نشده TABDLY
# define TABDLY 0
#پایان_اگر
#اگر_تعریف_نشده TAB1
# define TAB1 0
#پایان_اگر
#اگر_تعریف_نشده TAB2
# define TAB2 0
#پایان_اگر
#اگر_تعریف_نشده TOSTOP
# define TOSTOP 0
#پایان_اگر
#اگر_تعریف_نشده VDSUSP
# define VDSUSP 0
#پایان_اگر
#اگر_تعریف_نشده VEOL2
# define VEOL2 0
#پایان_اگر
#اگر_تعریف_نشده VFLUSHO
# define VFLUSHO 0
#پایان_اگر
#اگر_تعریف_نشده VLNEXT
# define VLNEXT 0
#پایان_اگر
#اگر_تعریف_نشده VREPRINT
# define VREPRINT 0
#پایان_اگر
#اگر_تعریف_نشده VSTATUS
# define VSTATUS 0
#پایان_اگر
#اگر_تعریف_نشده VSWTCH
# define VSWTCH 0
#پایان_اگر
#اگر_تعریف_نشده VTDLY
# define VTDLY 0
#پایان_اگر
#اگر_تعریف_نشده VWERASE
# define VWERASE 0
#پایان_اگر
#اگر_تعریف_نشده XCASE
# define XCASE 0
#پایان_اگر
#اگر_تعریف_نشده IUTF8
# define IUTF8 0
#پایان_اگر

/* Which speeds to set */
تعداد سرعت_setting {
	درون_داد_سرعت, output_سرعت, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
تعداد {
	control, درون_داد, output, local, combination
};
ساكن tcflag_t *get_ptr_مقصد_tcflag(بیعلامت type, ثابت بنية termios *نما)
{
	ساكن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(بنية termios, c_cflag), /* control */
		offsetof(بنية termios, c_iflag), /*درون داد*/
		offsetof(بنية termios, c_oflag), /* output */
		offsetof(بنية termios, c_lflag)  /* local */
	};
	إذا (type <= local) {
		أرجع (tcflag_t*) (((حرف*)نما) + tcflag_offsets[type]);
	}
	أرجع عدم;
}

/* Flags for 'struct mode_info' */
#تعریف_کن SANE_مجموعه 1              /* Set in 'sane' mode                  */
#تعریف_کن SANE_UNSET 2            /* Unset in 'sane' mode                */
#تعریف_کن REV 4                   /* Can be turned off by prepending '-' */
#تعریف_کن OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
بنية نما_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#إذا   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t ماسک;
#وإلاإذا (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t ماسک;
#وإلا
	ثابت tcflag_t ماسک;          /* Other bits to turn off for this mode */
#پایان_اگر
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t بیتها;          /* Bits to set for this mode            */
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
#پایان_اگر
#إذا TABDLY || OXTABS
	IDX_tabs,
#پایان_اگر
#إذا XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#پایان_اگر
};

#تعریف_کن MI_مدخل(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساكن ثابت حرف نما_name[] ALIGN1 =
	MI_مدخل("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("parity",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_مدخل("nl",       combination, REV        | OMIT, 0,          0 )
	MI_مدخل("ek",       combination, OMIT,              0,          0 )
	MI_مدخل("sane",     combination, OMIT,              0,          0 )
	MI_مدخل("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("raw",      combination, REV        | OMIT, 0,          0 )
	MI_مدخل("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("litout",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("crt",      combination, OMIT,              0,          0 )
	MI_مدخل("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_مدخل("decctlq",  combination, REV        | OMIT, 0,          0 )
#پایان_اگر
#إذا TABDLY || OXTABS
	MI_مدخل("tabs",     combination, REV        | OMIT, 0,          0 )
#پایان_اگر
#إذا XCASE && IUCLC && OLCUC
	MI_مدخل("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("LCASE",    combination, REV        | OMIT, 0,          0 )
#پایان_اگر
	MI_مدخل("parenb",   control,     REV,               PARENB,     0 )
	MI_مدخل("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_مدخل("cmspar",   control,     REV,               CMSPAR,     0 )
#پایان_اگر
	MI_مدخل("cs5",      control,     0,                 CS5,     CSIZE)
	MI_مدخل("cs6",      control,     0,                 CS6,     CSIZE)
	MI_مدخل("cs7",      control,     0,                 CS7,     CSIZE)
	MI_مدخل("cs8",      control,     0,                 CS8,     CSIZE)
	MI_مدخل("hupcl",    control,     REV,               HUPCL,      0 )
	MI_مدخل("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_مدخل("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_مدخل("cread",    control,     SANE_مجموعه   | REV,  CREAD,      0 )
	MI_مدخل("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_مدخل("crtscts",  control,     REV,               CRTSCTS,    0 )
#پایان_اگر
	MI_مدخل("ignbrk",   درون_داد,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_مدخل("brkint",   درون_داد,       SANE_مجموعه   | REV,  BRKINT,     0 )
	MI_مدخل("ignpar",   درون_داد,       REV,               IGNPAR,     0 )
	MI_مدخل("parmrk",   درون_داد,       REV,               PARMRK,     0 )
	MI_مدخل("inpck",    درون_داد,       REV,               INPCK,      0 )
	MI_مدخل("istrip",   درون_داد,       REV,               ISTRIP,     0 )
	MI_مدخل("inlcr",    درون_داد,       SANE_UNSET | REV,  INLCR,      0 )
	MI_مدخل("igncr",    درون_داد,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_مدخل("icrnl",    درون_داد,       SANE_مجموعه   | REV,  ICRNL,      0 )
	MI_مدخل("ixon",     درون_داد,       REV,               IXON,       0 )
	MI_مدخل("ixoff",    درون_داد,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_مدخل("tandem",   درون_داد,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_مدخل("iuclc",    درون_داد,       SANE_UNSET | REV,  IUCLC,      0 )
#پایان_اگر
#إذا IXANY
	MI_مدخل("ixany",    درون_داد,       SANE_UNSET | REV,  IXANY,      0 )
#پایان_اگر
#إذا IMAXBEL
	MI_مدخل("imaxbel",  درون_داد,       SANE_مجموعه   | REV,  IMAXBEL,    0 )
#پایان_اگر
#إذا IUTF8
	MI_مدخل("iutf8",    درون_داد,       SANE_UNSET | REV,  IUTF8,      0 )
#پایان_اگر
	MI_مدخل("opost",    output,      SANE_مجموعه   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_مدخل("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#پایان_اگر
#إذا OCRNL
	MI_مدخل("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#پایان_اگر
#إذا ONLCR
	MI_مدخل("onlcr",    output,      SANE_مجموعه   | REV,  ONLCR,      0 )
#پایان_اگر
#إذا ONOCR
	MI_مدخل("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#پایان_اگر
#إذا ONLRET
	MI_مدخل("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#پایان_اگر
#إذا OFILL
	MI_مدخل("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#پایان_اگر
#إذا OFDEL
	MI_مدخل("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#پایان_اگر
#إذا NLDLY
	MI_مدخل("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_مدخل("nl0",      output,      SANE_مجموعه,          NL0,     NLDLY)
#پایان_اگر
#إذا CRDLY
	MI_مدخل("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_مدخل("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_مدخل("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_مدخل("cr0",      output,      SANE_مجموعه,          CR0,     CRDLY)
#پایان_اگر

#إذا TABDLY
	MI_مدخل("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_مدخل("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_مدخل("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_مدخل("tab0",     output,      SANE_مجموعه,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_مدخل("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#پایان_اگر

#إذا BSDLY
	MI_مدخل("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_مدخل("bs0",      output,      SANE_مجموعه,          BS0,     BSDLY)
#پایان_اگر
#إذا VTDLY
	MI_مدخل("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_مدخل("vt0",      output,      SANE_مجموعه,          VT0,     VTDLY)
#پایان_اگر
#إذا FFDLY
	MI_مدخل("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_مدخل("ff0",      output,      SANE_مجموعه,          FF0,     FFDLY)
#پایان_اگر
	MI_مدخل("isig",     local,       SANE_مجموعه   | REV,  ISIG,       0 )
	MI_مدخل("icanon",   local,       SANE_مجموعه   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_مدخل("iexten",   local,       SANE_مجموعه   | REV,  IEXTEN,     0 )
#پایان_اگر
	MI_مدخل("echo",     local,       SANE_مجموعه   | REV,  ECHO,       0 )
	MI_مدخل("echoe",    local,       SANE_مجموعه   | REV,  ECHOE,      0 )
	MI_مدخل("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_مدخل("echok",    local,       SANE_مجموعه   | REV,  ECHOK,      0 )
	MI_مدخل("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_مدخل("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_مدخل("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#پایان_اگر
#إذا TOSTOP
	MI_مدخل("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#پایان_اگر
#إذا ECHOPRT
	MI_مدخل("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_مدخل("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#پایان_اگر
#إذا ECHOCTL
	MI_مدخل("echoctl",  local,       SANE_مجموعه   | REV,  ECHOCTL,    0 )
	MI_مدخل("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#پایان_اگر
#إذا ECHOKE
	MI_مدخل("echoke",   local,       SANE_مجموعه   | REV,  ECHOKE,     0 )
	MI_مدخل("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#پایان_اگر
	MI_مدخل("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#اگر_تعریف_شده EXTPROC
	MI_مدخل("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#پایان_اگر
	;

#لغو_تعریف MI_مدخل
#تعریف_کن MI_مدخل(N,T,F,B,M) { T, F, M, B },

ساكن ثابت بنية نما_info نما_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_مدخل("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("parity",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_مدخل("nl",       combination, REV        | OMIT, 0,          0 )
	MI_مدخل("ek",       combination, OMIT,              0,          0 )
	MI_مدخل("sane",     combination, OMIT,              0,          0 )
	MI_مدخل("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("raw",      combination, REV        | OMIT, 0,          0 )
	MI_مدخل("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("litout",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_مدخل("crt",      combination, OMIT,              0,          0 )
	MI_مدخل("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_مدخل("decctlq",  combination, REV        | OMIT, 0,          0 )
#پایان_اگر
#إذا TABDLY || OXTABS
	MI_مدخل("tabs",     combination, REV        | OMIT, 0,          0 )
#پایان_اگر
#إذا XCASE && IUCLC && OLCUC
	MI_مدخل("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_مدخل("LCASE",    combination, REV        | OMIT, 0,          0 )
#پایان_اگر
	MI_مدخل("parenb",   control,     REV,               PARENB,     0 )
	MI_مدخل("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_مدخل("cmspar",   control,     REV,               CMSPAR,     0 )
#پایان_اگر
	MI_مدخل("cs5",      control,     0,                 CS5,     CSIZE)
	MI_مدخل("cs6",      control,     0,                 CS6,     CSIZE)
	MI_مدخل("cs7",      control,     0,                 CS7,     CSIZE)
	MI_مدخل("cs8",      control,     0,                 CS8,     CSIZE)
	MI_مدخل("hupcl",    control,     REV,               HUPCL,      0 )
	MI_مدخل("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_مدخل("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_مدخل("cread",    control,     SANE_مجموعه   | REV,  CREAD,      0 )
	MI_مدخل("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_مدخل("crtscts",  control,     REV,               CRTSCTS,    0 )
#پایان_اگر
	MI_مدخل("ignbrk",   درون_داد,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_مدخل("brkint",   درون_داد,       SANE_مجموعه   | REV,  BRKINT,     0 )
	MI_مدخل("ignpar",   درون_داد,       REV,               IGNPAR,     0 )
	MI_مدخل("parmrk",   درون_داد,       REV,               PARMRK,     0 )
	MI_مدخل("inpck",    درون_داد,       REV,               INPCK,      0 )
	MI_مدخل("istrip",   درون_داد,       REV,               ISTRIP,     0 )
	MI_مدخل("inlcr",    درون_داد,       SANE_UNSET | REV,  INLCR,      0 )
	MI_مدخل("igncr",    درون_داد,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_مدخل("icrnl",    درون_داد,       SANE_مجموعه   | REV,  ICRNL,      0 )
	MI_مدخل("ixon",     درون_داد,       REV,               IXON,       0 )
	MI_مدخل("ixoff",    درون_داد,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_مدخل("tandem",   درون_داد,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_مدخل("iuclc",    درون_داد,       SANE_UNSET | REV,  IUCLC,      0 )
#پایان_اگر
#إذا IXANY
	MI_مدخل("ixany",    درون_داد,       SANE_UNSET | REV,  IXANY,      0 )
#پایان_اگر
#إذا IMAXBEL
	MI_مدخل("imaxbel",  درون_داد,       SANE_مجموعه   | REV,  IMAXBEL,    0 )
#پایان_اگر
#إذا IUTF8
	MI_مدخل("iutf8",    درون_داد,       SANE_UNSET | REV,  IUTF8,      0 )
#پایان_اگر
	MI_مدخل("opost",    output,      SANE_مجموعه   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_مدخل("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#پایان_اگر
#إذا OCRNL
	MI_مدخل("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#پایان_اگر
#إذا ONLCR
	MI_مدخل("onlcr",    output,      SANE_مجموعه   | REV,  ONLCR,      0 )
#پایان_اگر
#إذا ONOCR
	MI_مدخل("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#پایان_اگر
#إذا ONLRET
	MI_مدخل("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#پایان_اگر
#إذا OFILL
	MI_مدخل("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#پایان_اگر
#إذا OFDEL
	MI_مدخل("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#پایان_اگر
#إذا NLDLY
	MI_مدخل("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_مدخل("nl0",      output,      SANE_مجموعه,          NL0,     NLDLY)
#پایان_اگر
#إذا CRDLY
	MI_مدخل("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_مدخل("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_مدخل("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_مدخل("cr0",      output,      SANE_مجموعه,          CR0,     CRDLY)
#پایان_اگر

#إذا TABDLY
	MI_مدخل("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_مدخل("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_مدخل("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_مدخل("tab0",     output,      SANE_مجموعه,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_مدخل("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#پایان_اگر

#إذا BSDLY
	MI_مدخل("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_مدخل("bs0",      output,      SANE_مجموعه,          BS0,     BSDLY)
#پایان_اگر
#إذا VTDLY
	MI_مدخل("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_مدخل("vt0",      output,      SANE_مجموعه,          VT0,     VTDLY)
#پایان_اگر
#إذا FFDLY
	MI_مدخل("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_مدخل("ff0",      output,      SANE_مجموعه,          FF0,     FFDLY)
#پایان_اگر
	MI_مدخل("isig",     local,       SANE_مجموعه   | REV,  ISIG,       0 )
	MI_مدخل("icanon",   local,       SANE_مجموعه   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_مدخل("iexten",   local,       SANE_مجموعه   | REV,  IEXTEN,     0 )
#پایان_اگر
	MI_مدخل("echo",     local,       SANE_مجموعه   | REV,  ECHO,       0 )
	MI_مدخل("echoe",    local,       SANE_مجموعه   | REV,  ECHOE,      0 )
	MI_مدخل("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_مدخل("echok",    local,       SANE_مجموعه   | REV,  ECHOK,      0 )
	MI_مدخل("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_مدخل("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_مدخل("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#پایان_اگر
#إذا TOSTOP
	MI_مدخل("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#پایان_اگر
#إذا ECHOPRT
	MI_مدخل("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_مدخل("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#پایان_اگر
#إذا ECHOCTL
	MI_مدخل("echoctl",  local,       SANE_مجموعه   | REV,  ECHOCTL,    0 )
	MI_مدخل("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#پایان_اگر
#إذا ECHOKE
	MI_مدخل("echoke",   local,       SANE_مجموعه   | REV,  ECHOKE,     0 )
	MI_مدخل("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#پایان_اگر
	MI_مدخل("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#اگر_تعریف_شده EXTPROC
	MI_مدخل("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#پایان_اگر
};

تعداد {
	رقم_نما_info = ARRAY_اندازه(نما_info)
};


/* Control characters */
بنية control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t جابه_جایی;   /* Offset in c_cc */
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
#پایان_اگر
#إذا VSWTCH
	CIDX_swtch,
#پایان_اگر
	CIDX_شروع,
	CIDX_stop,
	CIDX_susp,
#إذا VDSUSP
	CIDX_dsusp,
#پایان_اگر
#إذا VREPRINT
	CIDX_rprnt,
#پایان_اگر
#إذا VWERASE
	CIDX_werase,
#پایان_اگر
#إذا VLNEXT
	CIDX_lnext,
#پایان_اگر
#إذا VFLUSHO
	CIDX_flush,
#پایان_اگر
#إذا VSTATUS
	CIDX_وضعیت,
#پایان_اگر
	CIDX_min,
	CIDX_زمان,
};

#تعریف_کن CI_مدخل(n,s,o) n "\0"

/* Name given on command line */
ساكن ثابت حرف control_name[] ALIGN1 =
	CI_مدخل("intr",     CINTR,   VINTR   )
	CI_مدخل("quit",     CQUIT,   VQUIT   )
	CI_مدخل("erase",    CERASE,  VERASE  )
	CI_مدخل("kill",     CKILL,   VKILL   )
	CI_مدخل("eof",      CEOF,    VEOF    )
	CI_مدخل("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_مدخل("eol2",     CEOL2,   VEOL2   )
#پایان_اگر
#إذا VSWTCH
	CI_مدخل("swtch",    CSWTCH,  VSWTCH  )
#پایان_اگر
	CI_مدخل("start",    CSTART,  VSTART  )
	CI_مدخل("stop",     CSTOP,   VSTOP   )
	CI_مدخل("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_مدخل("dsusp",    CDSUSP,  VDSUSP  )
#پایان_اگر
#إذا VREPRINT
	CI_مدخل("rprnt",    CRPRNT,  VREPRINT)
#پایان_اگر
#إذا VWERASE
	CI_مدخل("werase",   CWERASE, VWERASE )
#پایان_اگر
#إذا VLNEXT
	CI_مدخل("lnext",    CLNEXT,  VLNEXT  )
#پایان_اگر
#إذا VFLUSHO
	CI_مدخل("flush",    CFLUSHO, VFLUSHO )
#پایان_اگر
#إذا VSTATUS
	CI_مدخل("status",   CSTATUS, VSTATUS )
#پایان_اگر
	/* These must be last because of the display routines */
	CI_مدخل("min",      1,       VMIN    )
	CI_مدخل("time",     0,       VTIME   )
	;

#لغو_تعریف CI_مدخل
#تعریف_کن CI_مدخل(n,s,o) { s, o },

ساكن ثابت بنية control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_مدخل("intr",     CINTR,   VINTR   )
	CI_مدخل("quit",     CQUIT,   VQUIT   )
	CI_مدخل("erase",    CERASE,  VERASE  )
	CI_مدخل("kill",     CKILL,   VKILL   )
	CI_مدخل("eof",      CEOF,    VEOF    )
	CI_مدخل("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_مدخل("eol2",     CEOL2,   VEOL2   )
#پایان_اگر
#إذا VSWTCH
	CI_مدخل("swtch",    CSWTCH,  VSWTCH  )
#پایان_اگر
	CI_مدخل("start",    CSTART,  VSTART  )
	CI_مدخل("stop",     CSTOP,   VSTOP   )
	CI_مدخل("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_مدخل("dsusp",    CDSUSP,  VDSUSP  )
#پایان_اگر
#إذا VREPRINT
	CI_مدخل("rprnt",    CRPRNT,  VREPRINT)
#پایان_اگر
#إذا VWERASE
	CI_مدخل("werase",   CWERASE, VWERASE )
#پایان_اگر
#إذا VLNEXT
	CI_مدخل("lnext",    CLNEXT,  VLNEXT  )
#پایان_اگر
#إذا VFLUSHO
	CI_مدخل("flush",    CFLUSHO, VFLUSHO )
#پایان_اگر
#إذا VSTATUS
	CI_مدخل("status",   CSTATUS, VSTATUS )
#پایان_اگر
	/* These must be last because of the display routines */
	CI_مدخل("min",      1,       VMIN    )
	CI_مدخل("time",     0,       VTIME   )
};

تعداد {
	رقم_control_info = ARRAY_اندازه(control_info)
};


بنية globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	بیعلامت max_ستون;
	/* Current position, to know when to wrap */
	بیعلامت current_ستون;
} FIX_ALIASING;
#تعریف_کن G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کن INIT_G() افعل { \
	راهاندازی_common_bufsiz(); \
	G.device_name = bb_پیام_standard_درون_داد; \
	G.max_ستون = 80; \
	G.current_ستون = 0; /* we are noexec, must clear */ \
} طالما (0)

ساكن فراغ مجموعه_سرعت_or_die(تعداد سرعت_setting type, ثابت حرف *arg,
					بنية termios *نما)
{
	سرعت_t baud;

	baud = tty_مقدار_مقصد_baud(xatou(arg));

	إذا (type != output_سرعت) {     /* either input or both */
		cfsetispeed(نما, baud);
	}
	إذا (type != درون_داد_سرعت) {      /* either output or both */
		cfsetospeed(نما, baud);
	}
}

ساكن NORETURN فراغ perror_on_device_and_die(ثابت حرف *fmt)
{
	bb_perror_پیام_and_die(fmt, G.device_name);
}

ساكن فراغ perror_on_device(ثابت حرف *fmt)
{
	bb_perror_پیام(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ساكن فراغ wrapf(ثابت حرف *پیام_2, ...)
{
	حرف buf[128];
	va_list آرگومانها;
	بیعلامت buflen;

	va_شروع(آرگومانها, پیام_2);
	buflen = vsnprintf(buf, حجم(buf), پیام_2, آرگومانها);
	va_انتها(آرگومانها);
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

#اگر_تعریف_شده TIOCGWINSZ
ساكن فراغ مجموعه_پنجره_اندازه(صحيح ردیفها, صحيح تعداد_ستون)
{
	بنية winsize win = { 0, 0, 0, 0 };

	إذا (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		إذا (errno != EINVAL) {
			اذهب bail;
		}
		memset(&win, 0, حجم(win));
	}

	إذا (ردیفها >= 0)
		win.ws_ردیف = ردیفها;
	إذا (تعداد_ستون >= 0)
		win.ws_ستون = تعداد_ستون;

	إذا (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#پایان_اگر

ساكن فراغ display_پنجره_اندازه(صحيح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	بیعلامت عرض, ارتفاع;

	إذا (get_terminal_عرض_ارتفاع(STDIN_FILENO, &عرض, &ارتفاع)) {
		إذا ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} وإلا {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				ارتفاع, عرض);
	}
}

ساكن ثابت بنية suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساكن ثابت بنية نما_info *پیدا_کن_نما(ثابت حرف *name)
{
	صحيح i = شاخص_درون_رشتهها(نما_name, name);
	أرجع i >= 0 ? &نما_info[i] : عدم;
}

ساكن ثابت بنية control_info *پیدا_کن_control(ثابت حرف *name)
{
	صحيح i = شاخص_درون_رشتهها(control_name, name);
	أرجع i >= 0 ? &control_info[i] : عدم;
}

تعداد {
	param_need_arg = 0x80,
	param_سطر    = 1 | 0x80,
	param_ردیفها    = 2 | 0x80,
	param_تعداد_ستون    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_اندازه    = 5,
	param_سرعت   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساكن صحيح پیدا_کن_param(ثابت حرف *name)
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
	صحيح i = شاخص_درون_رشتهها(params, name) + 1;
	إذا (i == 0)
		أرجع 0;
	إذا (i != 5 && i != 6)
		i |= 0x80;
	أرجع i;
}

ساكن صحيح recover_نما(ثابت حرف *arg, بنية termios *نما)
{
	صحيح i, n;
	بیعلامت chr;
	بیعلامت دراز iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	إذا (از_رشته_بخوان(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		أرجع 0;
	نما->c_iflag = iflag;
	نما->c_oflag = oflag;
	نما->c_cflag = cflag;
	نما->c_lflag = lflag;
	arg += n;
	لكل (i = 0; i < NCCS; ++i) {
		إذا (از_رشته_بخوان(arg, ":%x%n", &chr, &n) != 1)
			أرجع 0;
		نما->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	إذا (*arg != '\0')
		أرجع 0;

	أرجع 1;
}

ساكن فراغ display_recoverable(ثابت بنية termios *نما,
				صحيح استفادهنشده_PARAM dummy)
{
	صحيح i;
	چاپ_کن("%lx:%lx:%lx:%lx",
		   (بیعلامت دراز) نما->c_iflag, (بیعلامت دراز) نما->c_oflag,
		   (بیعلامت دراز) نما->c_cflag, (بیعلامت دراز) نما->c_lflag);
	لكل (i = 0; i < NCCS; ++i)
		چاپ_کن(":%x", (بیعلامت صحيح) نما->c_cc[i]);
	bb_putchar('\n');
}

ساكن فراغ display_سرعت(ثابت بنية termios *نما, صحيح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	بیعلامت دراز ispeed, ospeed;

	ispeed = cfgetispeed(نما);
	ospeed = cfgetospeed(نما);
	إذا (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	إذا (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_مقصد_مقدار(ispeed), tty_baud_مقصد_مقدار(ospeed));
}

ساكن فراغ do_display(ثابت بنية termios *نما, صحيح all)
{
	صحيح i;
	tcflag_t *bitsp;
	بیعلامت دراز ماسک;
	صحيح prev_type = control;

	display_سرعت(نما, 1);
	إذا (all)
		display_پنجره_اندازه(1);
#اگر_تعریف_شده __linux__
	wrapf("line = %u;\n", نما->c_سطر);
#وإلا
	newline();
#پایان_اگر

	لكل (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#إذا VSWTCH == VSUSP
		إذا (i == CIDX_swtch)
			استمر;
#پایان_اگر
		/* If eof uses the same slot as min, only print whichever applies */
#إذا VEOF == VMIN
		إذا (!(نما->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			استمر;
		}
#پایان_اگر
		ch = نما->c_cc[control_info[i].جابه_جایی];
		إذا (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		وإلا
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#إذا VEOF == VMIN
	إذا ((نما->c_lflag & ICANON) == 0)
#پایان_اگر
		wrapf("min = %u; time = %u;", نما->c_cc[VMIN], نما->c_cc[VTIME]);
	newline();

	لكل (i = 0; i < رقم_نما_info; ++i) {
		إذا (نما_info[i].flags & OMIT)
			استمر;
		إذا (نما_info[i].type != prev_type) {
			newline();
			prev_type = نما_info[i].type;
		}

		bitsp = get_ptr_مقصد_tcflag(نما_info[i].type, نما);
		ماسک = نما_info[i].ماسک ? نما_info[i].ماسک : نما_info[i].بیتها;
		إذا ((*bitsp & ماسک) == نما_info[i].بیتها) {
			إذا (all || (نما_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(نما_name, i));
		} وإلا {
			إذا ((all && نما_info[i].flags & REV)
			 || (!all && (نما_info[i].flags & (SANE_مجموعه | REV)) == (SANE_مجموعه | REV))
			) {
				wrapf("-%s", nth_string(نما_name, i));
			}
		}
	}
	newline();
}

ساكن فراغ sane_نما(بنية termios *نما)
{
	صحيح i;

	لكل (i = 0; i < رقم_control_info; ++i) {
#إذا VMIN == VEOF
		إذا (i == CIDX_min)
			اكسر;
#پایان_اگر
		نما->c_cc[control_info[i].جابه_جایی] = control_info[i].saneval;
	}

	لكل (i = 0; i < رقم_نما_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_مقصد_tcflag(نما_info[i].type, نما);

		إذا (!bitsp)
			استمر;
		val = *bitsp & ~((بیعلامت دراز)نما_info[i].ماسک);
		إذا (نما_info[i].flags & SANE_مجموعه) {
			*bitsp = val | نما_info[i].بیتها;
		} وإلا
		إذا (نما_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~نما_info[i].بیتها;
		}
	}
}

ساكن فراغ مجموعه_نما(ثابت بنية نما_info *info, صحيح reversed,
					بنية termios *نما)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_مقصد_tcflag(info->type, نما);

	إذا (bitsp) {
		tcflag_t val = *bitsp & ~info->ماسک;
		إذا (reversed)
			*bitsp = val & ~info->بیتها;
		وإلا
			*bitsp = val | info->بیتها;
		أرجع;
	}

	/* !bitsp - it's a "combination" mode */
	إذا (info == &نما_info[IDX_evenp] || info == &نما_info[IDX_parity]) {
		إذا (reversed)
			نما->c_cflag = (نما->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			نما->c_cflag = (نما->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} وإلا إذا (info == &نما_info[IDX_oddp]) {
		إذا (reversed)
			نما->c_cflag = (نما->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			نما->c_cflag = (نما->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} وإلا إذا (info == &نما_info[IDX_nl]) {
		إذا (reversed) {
			نما->c_iflag = (نما->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			نما->c_oflag = (نما->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} وإلا {
			نما->c_iflag = نما->c_iflag & ~ICRNL;
			إذا (ONLCR) نما->c_oflag = نما->c_oflag & ~ONLCR;
		}
	} وإلا إذا (info == &نما_info[IDX_ek]) {
		نما->c_cc[VERASE] = CERASE;
		نما->c_cc[VKILL] = CKILL;
	} وإلا إذا (info == &نما_info[IDX_sane]) {
		sane_نما(نما);
	} وإلا إذا (info == &نما_info[IDX_cbreak]) {
		إذا (reversed)
			نما->c_lflag |= ICANON;
		وإلا
			نما->c_lflag &= ~ICANON;
	} وإلا إذا (info == &نما_info[IDX_pass8]) {
		إذا (reversed) {
			نما->c_cflag = (نما->c_cflag & ~CSIZE) | CS7 | PARENB;
			نما->c_iflag |= ISTRIP;
		} وإلا {
			نما->c_cflag = (نما->c_cflag & ~PARENB & ~CSIZE) | CS8;
			نما->c_iflag &= ~ISTRIP;
		}
	} وإلا إذا (info == &نما_info[IDX_litout]) {
		إذا (reversed) {
			نما->c_cflag = (نما->c_cflag & ~CSIZE) | CS7 | PARENB;
			نما->c_iflag |= ISTRIP;
			نما->c_oflag |= OPOST;
		} وإلا {
			نما->c_cflag = (نما->c_cflag & ~PARENB & ~CSIZE) | CS8;
			نما->c_iflag &= ~ISTRIP;
			نما->c_oflag &= ~OPOST;
		}
	} وإلا إذا (info == &نما_info[IDX_raw] || info == &نما_info[IDX_cooked]) {
		إذا ((info == &نما_info[IDX_raw] && reversed)
		 || (info == &نما_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			نما->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			نما->c_oflag |= OPOST;
			نما->c_lflag |= ISIG | ICANON;
#إذا VMIN == VEOF
			نما->c_cc[VEOF] = CEOF;
#پایان_اگر
#إذا VTIME == VEOL
			نما->c_cc[VEOL] = CEOL;
#پایان_اگر
		} وإلا {
			/* Raw mode */
			نما->c_iflag = 0;
			نما->c_oflag &= ~OPOST;
			نما->c_lflag &= ~(ISIG | ICANON | XCASE);
			نما->c_cc[VMIN] = 1;
			نما->c_cc[VTIME] = 0;
		}
	}
#إذا IXANY
	وإلا إذا (info == &نما_info[IDX_decctlq]) {
		إذا (reversed)
			نما->c_iflag |= IXANY;
		وإلا
			نما->c_iflag &= ~IXANY;
	}
#پایان_اگر
#إذا TABDLY
	وإلا إذا (info == &نما_info[IDX_tabs]) {
		إذا (reversed)
			نما->c_oflag = (نما->c_oflag & ~TABDLY) | TAB3;
		وإلا
			نما->c_oflag = (نما->c_oflag & ~TABDLY) | TAB0;
	}
#پایان_اگر
#إذا OXTABS
	وإلا إذا (info == &نما_info[IDX_tabs]) {
		إذا (reversed)
			نما->c_oflag |= OXTABS;
		وإلا
			نما->c_oflag &= ~OXTABS;
	}
#پایان_اگر
#إذا XCASE && IUCLC && OLCUC
	وإلا إذا (info==&نما_info[IDX_lcase] || info==&نما_info[IDX_LCASE]) {
		إذا (reversed) {
			نما->c_lflag &= ~XCASE;
			نما->c_iflag &= ~IUCLC;
			نما->c_oflag &= ~OLCUC;
		} وإلا {
			نما->c_lflag |= XCASE;
			نما->c_iflag |= IUCLC;
			نما->c_oflag |= OLCUC;
		}
	}
#پایان_اگر
	وإلا إذا (info == &نما_info[IDX_crt]) {
		نما->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} وإلا إذا (info == &نما_info[IDX_dec]) {
		نما->c_cc[VINTR] = 3; /* ^C */
		نما->c_cc[VERASE] = 127; /* DEL */
		نما->c_cc[VKILL] = 21; /* ^U */
		نما->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		إذا (IXANY) نما->c_iflag &= ~IXANY;
	}
}

ساكن فراغ مجموعه_control_char_or_die(ثابت بنية control_info *info,
			ثابت حرف *arg, بنية termios *نما)
{
	بیعلامت حرف مقدار_2;

	إذا (info == &control_info[CIDX_min] || info == &control_info[CIDX_زمان])
		مقدار_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	وإلا إذا (arg[0] == '\0' || arg[1] == '\0')
		مقدار_2 = arg[0];
	وإلا إذا (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		مقدار_2 = _POSIX_VDISABLE;
	وإلا إذا (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		مقدار_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		إذا (arg[1] == '?')
			مقدار_2 = 127;
	} وإلا
		مقدار_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	نما->c_cc[info->جابه_جایی] = مقدار_2;
}

#تعریف_کن STTY_require_مجموعه_attr   (1 << 0)
#تعریف_کن STTY_سرعت_was_مجموعه      (1 << 1)
#تعریف_کن STTY_verbose_output     (1 << 2)
#تعریف_کن STTY_recoverable_output (1 << 3)
#تعریف_کن STTY_noargs             (1 << 4)

صحيح stty_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stty_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	بنية termios نما;
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
		ثابت بنية نما_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			صحيح i;
			mp = پیدا_کن_نما(arg+1);
			إذا (mp) {
				إذا (!(mp->flags & REV))
					اذهب invalid_آرگومان;
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
						bb_simple_error_پیام_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					إذا (!file_name[0]) { /* nope, "-F device" */
						صحيح p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						إذا (!file_name)
							bb_error_پیام_and_die(bb_پیام_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						طالما (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					اذهب انتها_option;
				افتراضي:
					اذهب invalid_آرگومان;
				}
			}
 انتها_option:
			استمر;
		}

		mp = پیدا_کن_نما(arg);
		إذا (mp) {
			stty_state &= ~STTY_noargs;
			استمر;
		}

		cp = پیدا_کن_control(arg);
		إذا (cp) {
			إذا (!argnext)
				bb_error_پیام_and_die(bb_پیام_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			مجموعه_control_char_or_die(cp, argnext, &نما);
			stty_state &= ~STTY_noargs;
			++k;
			استمر;
		}

		param = پیدا_کن_param(arg);
		إذا (param & param_need_arg) {
			إذا (!argnext)
				bb_error_پیام_and_die(bb_پیام_requires_arg, arg);
			++k;
		}

		بدل (param) {
#اگر_تعریف_شده __linux__
		حالة param_سطر:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, بیشترین_صحیح, stty_suffixes);
			اكسر;
# endif /* else fall-through */
#پایان_اگر
#اگر_تعریف_شده TIOCGWINSZ
		حالة param_ردیفها:
		حالة param_تعداد_ستون:
		حالة param_columns:
			xatoul_range_sfx(argnext, 1, بیشترین_صحیح, stty_suffixes);
			اكسر;
		حالة param_اندازه:
#پایان_اگر
		حالة param_سرعت:
			اكسر;
		حالة param_ispeed:
			/* called for the side effect of xfunc death only */
			مجموعه_سرعت_or_die(درون_داد_سرعت, argnext, &نما);
			اكسر;
		حالة param_ospeed:
			/* called for the side effect of xfunc death only */
			مجموعه_سرعت_or_die(output_سرعت, argnext, &نما);
			اكسر;
		افتراضي:
			إذا (recover_نما(arg, &نما) == 1) اكسر;
			إذا (tty_مقدار_مقصد_baud(xatou(arg)) != (سرعت_t) -1) اكسر;
 invalid_آرگومان:
			bb_error_پیام_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_پیام_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_پیام_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	إذا (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&نما, 0, حجم(نما));
	إذا (tcgetattr(STDIN_FILENO, &نما))
		perror_on_device_and_die("%s");

	إذا (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_ستون = get_terminal_عرض(STDOUT_FILENO);
		output_func(&نما, display_all);
		أرجع EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	طالما (argv[++k]) {
		ثابت بنية نما_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			mp = پیدا_کن_نما(arg+1);
			إذا (mp) {
				مجموعه_نما(mp, 1 /* reversed */, &نما);
				stty_state |= STTY_require_مجموعه_attr;
			}
			/* It is an option - already parsed. Skip it */
			استمر;
		}

		mp = پیدا_کن_نما(arg);
		إذا (mp) {
			مجموعه_نما(mp, 0 /* non-reversed */, &نما);
			stty_state |= STTY_require_مجموعه_attr;
			استمر;
		}

		cp = پیدا_کن_control(arg);
		إذا (cp) {
			++k;
			مجموعه_control_char_or_die(cp, argnext, &نما);
			stty_state |= STTY_require_مجموعه_attr;
			استمر;
		}

		param = پیدا_کن_param(arg);
		إذا (param & param_need_arg) {
			++k;
		}

		بدل (param) {
#اگر_تعریف_شده __linux__
		حالة param_سطر:
			نما.c_سطر = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_مجموعه_attr;
			اكسر;
#پایان_اگر
#اگر_تعریف_شده TIOCGWINSZ
		حالة param_تعداد_ستون:
		حالة param_columns:
			مجموعه_پنجره_اندازه(-1, xatoul_sfx(argnext, stty_suffixes));
			اكسر;
		حالة param_اندازه:
			display_پنجره_اندازه(0);
			اكسر;
		حالة param_ردیفها:
			مجموعه_پنجره_اندازه(xatoul_sfx(argnext, stty_suffixes), -1);
			اكسر;
#پایان_اگر
		حالة param_سرعت:
			display_سرعت(&نما, 0);
			اكسر;
		حالة param_ispeed:
			مجموعه_سرعت_or_die(درون_داد_سرعت, argnext, &نما);
			stty_state |= (STTY_require_مجموعه_attr | STTY_سرعت_was_مجموعه);
			اكسر;
		حالة param_ospeed:
			مجموعه_سرعت_or_die(output_سرعت, argnext, &نما);
			stty_state |= (STTY_require_مجموعه_attr | STTY_سرعت_was_مجموعه);
			اكسر;
		افتراضي:
			إذا (recover_نما(arg, &نما) == 1)
				stty_state |= STTY_require_مجموعه_attr;
			وإلا /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				مجموعه_سرعت_or_die(both_speeds, arg, &نما);
				stty_state |= (STTY_require_مجموعه_attr | STTY_سرعت_was_مجموعه);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	إذا (stty_state & STTY_require_مجموعه_attr) {
		بنية termios new_نما;

		إذا (tcsetattr(STDIN_FILENO, TCSADRAIN, &نما))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_نما, 0, حجم(new_نما));
		إذا (tcgetattr(STDIN_FILENO, &new_نما))
			perror_on_device_and_die("%s");

		إذا (memcmp(&نما, &new_نما, حجم(نما)) != 0) {
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

			new_نما.c_cflag &= (~CIBAUD);
			إذا ((stty_state & STTY_سرعت_was_مجموعه)
			 || memcmp(&نما, &new_نما, حجم(نما)) != 0)
#پایان_اگر
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	أرجع EXIT_SUCCESS;
}
