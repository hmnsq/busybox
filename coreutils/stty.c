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

#جيڪڏهن_وصف_نه_ٿيل _POSIX_VDISABLE
# define _POSIX_VDISABLE ((بنانشان حرف) 0)
#جيڪڏهن_پورو

#وصف_ڪريو Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#جيڪڏهن_وصف_نه_ٿيل CINTR
# define CINTR Control('c')
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CQUIT
# define CQUIT 28
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CERASE
# define CERASE 127
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CKILL
# define CKILL Control('u')
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CEOF
# define CEOF Control('d')
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CEOL
# define CEOL _POSIX_VDISABLE
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CSTART
# define CSTART Control('q')
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CSTOP
# define CSTOP Control('s')
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CSUSP
# define CSUSP Control('z')
#جيڪڏهن_پورو
#إذا defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#جيڪڏهن_پورو
/* glibc-2.12.1 uses only VSWTC name */
#إذا defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#جيڪڏهن_پورو
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#إذا defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#جيڪڏهن_پورو
#إذا defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#جيڪڏهن_پورو

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#إذا defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#جيڪڏهن_پورو

#إذا defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#جيڪڏهن_پورو
#إذا defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#جيڪڏهن_پورو
#إذا !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#جيڪڏهن_پورو
#إذا defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#جيڪڏهن_پورو
#إذا defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#جيڪڏهن_پورو
#إذا defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#جيڪڏهن_پورو
#إذا defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#جيڪڏهن_پورو
#إذا defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#جيڪڏهن_پورو
#إذا defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#جيڪڏهن_پورو
#إذا defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#جيڪڏهن_پورو
#إذا defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#جيڪڏهن_پورو
#إذا defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#جيڪڏهن_پورو
#إذا defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#جيڪڏهن_پورو

/* Save us from #ifdef forest plague */
#جيڪڏهن_وصف_نه_ٿيل BSDLY
# define BSDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CIBAUD
# define CIBAUD 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CRDLY
# define CRDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CMSPAR
# define CMSPAR 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل CRTSCTS
# define CRTSCTS 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ECHOCTL
# define ECHOCTL 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ECHOKE
# define ECHOKE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ECHOPRT
# define ECHOPRT 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل FFDLY
# define FFDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل IEXTEN
# define IEXTEN 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل IMAXBEL
# define IMAXBEL 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل IUCLC
# define IUCLC 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل IXANY
# define IXANY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل NLDLY
# define NLDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل OCRNL
# define OCRNL 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل OFDEL
# define OFDEL 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل OFILL
# define OFILL 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل OLCUC
# define OLCUC 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ONLCR
# define ONLCR 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ONLRET
# define ONLRET 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ONOCR
# define ONOCR 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل OXTABS
# define OXTABS 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل TABDLY
# define TABDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل TAB1
# define TAB1 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل TAB2
# define TAB2 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل TOSTOP
# define TOSTOP 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VDSUSP
# define VDSUSP 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VEOL2
# define VEOL2 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VFLUSHO
# define VFLUSHO 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VLNEXT
# define VLNEXT 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VREPRINT
# define VREPRINT 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VSTATUS
# define VSTATUS 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VSWTCH
# define VSWTCH 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VTDLY
# define VTDLY 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل VWERASE
# define VWERASE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل XCASE
# define XCASE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل IUTF8
# define IUTF8 0
#جيڪڏهن_پورو

/* Which speeds to set */
تعداد رفتار_setting {
	داخلا_رفتار, output_رفتار, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
تعداد {
	control, داخلا_2, output, local, combination
};
ساكن tcflag_t *get_ptr_منزل_tcflag(بنانشان type, ثابت بنية termios *گهڻائي)
{
	ساكن ثابت uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(بنية termios, c_cflag), /* control */
		offsetof(بنية termios, c_iflag), /*داخلا*/
		offsetof(بنية termios, c_oflag), /* output */
		offsetof(بنية termios, c_lflag)  /* local */
	};
	إذا (type <= local) {
		أرجع (tcflag_t*) (((حرف*)گهڻائي) + tcflag_offsets[type]);
	}
	أرجع عدم;
}

/* Flags for 'struct mode_info' */
#وصف_ڪريو SANE_سيٽ 1              /* Set in 'sane' mode                  */
#وصف_ڪريو SANE_UNSET 2            /* Unset in 'sane' mode                */
#وصف_ڪريو REV 4                   /* Can be turned off by prepending '-' */
#وصف_ڪريو OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
بنية گهڻائي_info {
	ثابت uint8_t type;           /* Which structure element to change    */
	ثابت uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#إذا   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ثابت uint8_t نقاب;
#وإلاإذا (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ثابت uint16_t نقاب;
#وإلا
	ثابت tcflag_t نقاب;          /* Other bits to turn off for this mode */
#جيڪڏهن_پورو
	/* was using short here, but ppc32 was unhappy */
	ثابت tcflag_t بٽ;          /* Bits to set for this mode            */
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
#جيڪڏهن_پورو
#إذا TABDLY || OXTABS
	IDX_tabs,
#جيڪڏهن_پورو
#إذا XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#جيڪڏهن_پورو
};

#وصف_ڪريو MI_داخلا(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ساكن ثابت حرف گهڻائي_name[] ALIGN1 =
	MI_داخلا("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("parity",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_داخلا("nl",       combination, REV        | OMIT, 0,          0 )
	MI_داخلا("ek",       combination, OMIT,              0,          0 )
	MI_داخلا("sane",     combination, OMIT,              0,          0 )
	MI_داخلا("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("raw",      combination, REV        | OMIT, 0,          0 )
	MI_داخلا("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("litout",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("crt",      combination, OMIT,              0,          0 )
	MI_داخلا("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_داخلا("decctlq",  combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
#إذا TABDLY || OXTABS
	MI_داخلا("tabs",     combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
#إذا XCASE && IUCLC && OLCUC
	MI_داخلا("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("LCASE",    combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
	MI_داخلا("parenb",   control,     REV,               PARENB,     0 )
	MI_داخلا("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_داخلا("cmspar",   control,     REV,               CMSPAR,     0 )
#جيڪڏهن_پورو
	MI_داخلا("cs5",      control,     0,                 CS5,     CSIZE)
	MI_داخلا("cs6",      control,     0,                 CS6,     CSIZE)
	MI_داخلا("cs7",      control,     0,                 CS7,     CSIZE)
	MI_داخلا("cs8",      control,     0,                 CS8,     CSIZE)
	MI_داخلا("hupcl",    control,     REV,               HUPCL,      0 )
	MI_داخلا("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_داخلا("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_داخلا("cread",    control,     SANE_سيٽ   | REV,  CREAD,      0 )
	MI_داخلا("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_داخلا("crtscts",  control,     REV,               CRTSCTS,    0 )
#جيڪڏهن_پورو
	MI_داخلا("ignbrk",   داخلا_2,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_داخلا("brkint",   داخلا_2,       SANE_سيٽ   | REV,  BRKINT,     0 )
	MI_داخلا("ignpar",   داخلا_2,       REV,               IGNPAR,     0 )
	MI_داخلا("parmrk",   داخلا_2,       REV,               PARMRK,     0 )
	MI_داخلا("inpck",    داخلا_2,       REV,               INPCK,      0 )
	MI_داخلا("istrip",   داخلا_2,       REV,               ISTRIP,     0 )
	MI_داخلا("inlcr",    داخلا_2,       SANE_UNSET | REV,  INLCR,      0 )
	MI_داخلا("igncr",    داخلا_2,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_داخلا("icrnl",    داخلا_2,       SANE_سيٽ   | REV,  ICRNL,      0 )
	MI_داخلا("ixon",     داخلا_2,       REV,               IXON,       0 )
	MI_داخلا("ixoff",    داخلا_2,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_داخلا("tandem",   داخلا_2,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_داخلا("iuclc",    داخلا_2,       SANE_UNSET | REV,  IUCLC,      0 )
#جيڪڏهن_پورو
#إذا IXANY
	MI_داخلا("ixany",    داخلا_2,       SANE_UNSET | REV,  IXANY,      0 )
#جيڪڏهن_پورو
#إذا IMAXBEL
	MI_داخلا("imaxbel",  داخلا_2,       SANE_سيٽ   | REV,  IMAXBEL,    0 )
#جيڪڏهن_پورو
#إذا IUTF8
	MI_داخلا("iutf8",    داخلا_2,       SANE_UNSET | REV,  IUTF8,      0 )
#جيڪڏهن_پورو
	MI_داخلا("opost",    output,      SANE_سيٽ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_داخلا("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#جيڪڏهن_پورو
#إذا OCRNL
	MI_داخلا("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#جيڪڏهن_پورو
#إذا ONLCR
	MI_داخلا("onlcr",    output,      SANE_سيٽ   | REV,  ONLCR,      0 )
#جيڪڏهن_پورو
#إذا ONOCR
	MI_داخلا("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#جيڪڏهن_پورو
#إذا ONLRET
	MI_داخلا("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#جيڪڏهن_پورو
#إذا OFILL
	MI_داخلا("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#جيڪڏهن_پورو
#إذا OFDEL
	MI_داخلا("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#جيڪڏهن_پورو
#إذا NLDLY
	MI_داخلا("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_داخلا("nl0",      output,      SANE_سيٽ,          NL0,     NLDLY)
#جيڪڏهن_پورو
#إذا CRDLY
	MI_داخلا("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_داخلا("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_داخلا("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_داخلا("cr0",      output,      SANE_سيٽ,          CR0,     CRDLY)
#جيڪڏهن_پورو

#إذا TABDLY
	MI_داخلا("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_داخلا("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_داخلا("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_داخلا("tab0",     output,      SANE_سيٽ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_داخلا("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#جيڪڏهن_پورو

#إذا BSDLY
	MI_داخلا("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_داخلا("bs0",      output,      SANE_سيٽ,          BS0,     BSDLY)
#جيڪڏهن_پورو
#إذا VTDLY
	MI_داخلا("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_داخلا("vt0",      output,      SANE_سيٽ,          VT0,     VTDLY)
#جيڪڏهن_پورو
#إذا FFDLY
	MI_داخلا("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_داخلا("ff0",      output,      SANE_سيٽ,          FF0,     FFDLY)
#جيڪڏهن_پورو
	MI_داخلا("isig",     local,       SANE_سيٽ   | REV,  ISIG,       0 )
	MI_داخلا("icanon",   local,       SANE_سيٽ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_داخلا("iexten",   local,       SANE_سيٽ   | REV,  IEXTEN,     0 )
#جيڪڏهن_پورو
	MI_داخلا("echo",     local,       SANE_سيٽ   | REV,  ECHO,       0 )
	MI_داخلا("echoe",    local,       SANE_سيٽ   | REV,  ECHOE,      0 )
	MI_داخلا("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_داخلا("echok",    local,       SANE_سيٽ   | REV,  ECHOK,      0 )
	MI_داخلا("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_داخلا("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_داخلا("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#جيڪڏهن_پورو
#إذا TOSTOP
	MI_داخلا("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#جيڪڏهن_پورو
#إذا ECHOPRT
	MI_داخلا("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_داخلا("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#جيڪڏهن_پورو
#إذا ECHOCTL
	MI_داخلا("echoctl",  local,       SANE_سيٽ   | REV,  ECHOCTL,    0 )
	MI_داخلا("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#جيڪڏهن_پورو
#إذا ECHOKE
	MI_داخلا("echoke",   local,       SANE_سيٽ   | REV,  ECHOKE,     0 )
	MI_داخلا("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#جيڪڏهن_پورو
	MI_داخلا("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#جيڪڏهن_وصف_ٿيل EXTPROC
	MI_داخلا("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#جيڪڏهن_پورو
	;

#وصف_رد MI_داخلا
#وصف_ڪريو MI_داخلا(N,T,F,B,M) { T, F, M, B },

ساكن ثابت بنية گهڻائي_info گهڻائي_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_داخلا("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("parity",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_داخلا("nl",       combination, REV        | OMIT, 0,          0 )
	MI_داخلا("ek",       combination, OMIT,              0,          0 )
	MI_داخلا("sane",     combination, OMIT,              0,          0 )
	MI_داخلا("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("raw",      combination, REV        | OMIT, 0,          0 )
	MI_داخلا("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("litout",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_داخلا("crt",      combination, OMIT,              0,          0 )
	MI_داخلا("dec",      combination, OMIT,              0,          0 )
#إذا IXANY
	MI_داخلا("decctlq",  combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
#إذا TABDLY || OXTABS
	MI_داخلا("tabs",     combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
#إذا XCASE && IUCLC && OLCUC
	MI_داخلا("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_داخلا("LCASE",    combination, REV        | OMIT, 0,          0 )
#جيڪڏهن_پورو
	MI_داخلا("parenb",   control,     REV,               PARENB,     0 )
	MI_داخلا("parodd",   control,     REV,               PARODD,     0 )
#إذا CMSPAR
	MI_داخلا("cmspar",   control,     REV,               CMSPAR,     0 )
#جيڪڏهن_پورو
	MI_داخلا("cs5",      control,     0,                 CS5,     CSIZE)
	MI_داخلا("cs6",      control,     0,                 CS6,     CSIZE)
	MI_داخلا("cs7",      control,     0,                 CS7,     CSIZE)
	MI_داخلا("cs8",      control,     0,                 CS8,     CSIZE)
	MI_داخلا("hupcl",    control,     REV,               HUPCL,      0 )
	MI_داخلا("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_داخلا("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_داخلا("cread",    control,     SANE_سيٽ   | REV,  CREAD,      0 )
	MI_داخلا("clocal",   control,     REV,               CLOCAL,     0 )
#إذا CRTSCTS
	MI_داخلا("crtscts",  control,     REV,               CRTSCTS,    0 )
#جيڪڏهن_پورو
	MI_داخلا("ignbrk",   داخلا_2,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_داخلا("brkint",   داخلا_2,       SANE_سيٽ   | REV,  BRKINT,     0 )
	MI_داخلا("ignpar",   داخلا_2,       REV,               IGNPAR,     0 )
	MI_داخلا("parmrk",   داخلا_2,       REV,               PARMRK,     0 )
	MI_داخلا("inpck",    داخلا_2,       REV,               INPCK,      0 )
	MI_داخلا("istrip",   داخلا_2,       REV,               ISTRIP,     0 )
	MI_داخلا("inlcr",    داخلا_2,       SANE_UNSET | REV,  INLCR,      0 )
	MI_داخلا("igncr",    داخلا_2,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_داخلا("icrnl",    داخلا_2,       SANE_سيٽ   | REV,  ICRNL,      0 )
	MI_داخلا("ixon",     داخلا_2,       REV,               IXON,       0 )
	MI_داخلا("ixoff",    داخلا_2,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_داخلا("tandem",   داخلا_2,       OMIT       | REV,  IXOFF,      0 )
#إذا IUCLC
	MI_داخلا("iuclc",    داخلا_2,       SANE_UNSET | REV,  IUCLC,      0 )
#جيڪڏهن_پورو
#إذا IXANY
	MI_داخلا("ixany",    داخلا_2,       SANE_UNSET | REV,  IXANY,      0 )
#جيڪڏهن_پورو
#إذا IMAXBEL
	MI_داخلا("imaxbel",  داخلا_2,       SANE_سيٽ   | REV,  IMAXBEL,    0 )
#جيڪڏهن_پورو
#إذا IUTF8
	MI_داخلا("iutf8",    داخلا_2,       SANE_UNSET | REV,  IUTF8,      0 )
#جيڪڏهن_پورو
	MI_داخلا("opost",    output,      SANE_سيٽ   | REV,  OPOST,      0 )
#إذا OLCUC
	MI_داخلا("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#جيڪڏهن_پورو
#إذا OCRNL
	MI_داخلا("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#جيڪڏهن_پورو
#إذا ONLCR
	MI_داخلا("onlcr",    output,      SANE_سيٽ   | REV,  ONLCR,      0 )
#جيڪڏهن_پورو
#إذا ONOCR
	MI_داخلا("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#جيڪڏهن_پورو
#إذا ONLRET
	MI_داخلا("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#جيڪڏهن_پورو
#إذا OFILL
	MI_داخلا("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#جيڪڏهن_پورو
#إذا OFDEL
	MI_داخلا("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#جيڪڏهن_پورو
#إذا NLDLY
	MI_داخلا("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_داخلا("nl0",      output,      SANE_سيٽ,          NL0,     NLDLY)
#جيڪڏهن_پورو
#إذا CRDLY
	MI_داخلا("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_داخلا("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_داخلا("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_داخلا("cr0",      output,      SANE_سيٽ,          CR0,     CRDLY)
#جيڪڏهن_پورو

#إذا TABDLY
	MI_داخلا("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# إذا TAB2
	MI_داخلا("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# إذا TAB1
	MI_داخلا("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_داخلا("tab0",     output,      SANE_سيٽ,          TAB0,   TABDLY)
#وإلا
# إذا OXTABS
	MI_داخلا("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#جيڪڏهن_پورو

#إذا BSDLY
	MI_داخلا("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_داخلا("bs0",      output,      SANE_سيٽ,          BS0,     BSDLY)
#جيڪڏهن_پورو
#إذا VTDLY
	MI_داخلا("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_داخلا("vt0",      output,      SANE_سيٽ,          VT0,     VTDLY)
#جيڪڏهن_پورو
#إذا FFDLY
	MI_داخلا("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_داخلا("ff0",      output,      SANE_سيٽ,          FF0,     FFDLY)
#جيڪڏهن_پورو
	MI_داخلا("isig",     local,       SANE_سيٽ   | REV,  ISIG,       0 )
	MI_داخلا("icanon",   local,       SANE_سيٽ   | REV,  ICANON,     0 )
#إذا IEXTEN
	MI_داخلا("iexten",   local,       SANE_سيٽ   | REV,  IEXTEN,     0 )
#جيڪڏهن_پورو
	MI_داخلا("echo",     local,       SANE_سيٽ   | REV,  ECHO,       0 )
	MI_داخلا("echoe",    local,       SANE_سيٽ   | REV,  ECHOE,      0 )
	MI_داخلا("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_داخلا("echok",    local,       SANE_سيٽ   | REV,  ECHOK,      0 )
	MI_داخلا("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_داخلا("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#إذا XCASE
	MI_داخلا("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#جيڪڏهن_پورو
#إذا TOSTOP
	MI_داخلا("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#جيڪڏهن_پورو
#إذا ECHOPRT
	MI_داخلا("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_داخلا("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#جيڪڏهن_پورو
#إذا ECHOCTL
	MI_داخلا("echoctl",  local,       SANE_سيٽ   | REV,  ECHOCTL,    0 )
	MI_داخلا("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#جيڪڏهن_پورو
#إذا ECHOKE
	MI_داخلا("echoke",   local,       SANE_سيٽ   | REV,  ECHOKE,     0 )
	MI_داخلا("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#جيڪڏهن_پورو
	MI_داخلا("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#جيڪڏهن_وصف_ٿيل EXTPROC
	MI_داخلا("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#جيڪڏهن_پورو
};

تعداد {
	انگ_گهڻائي_info = ARRAY_سائيز(گهڻائي_info)
};


/* Control characters */
بنية control_info {
	ثابت uint8_t saneval;  /* Value to set for 'stty sane' */
	ثابت uint8_t وٿي;   /* Offset in c_cc */
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
#جيڪڏهن_پورو
#إذا VSWTCH
	CIDX_swtch,
#جيڪڏهن_پورو
	CIDX_شروعات,
	CIDX_stop,
	CIDX_susp,
#إذا VDSUSP
	CIDX_dsusp,
#جيڪڏهن_پورو
#إذا VREPRINT
	CIDX_rprnt,
#جيڪڏهن_پورو
#إذا VWERASE
	CIDX_werase,
#جيڪڏهن_پورو
#إذا VLNEXT
	CIDX_lnext,
#جيڪڏهن_پورو
#إذا VFLUSHO
	CIDX_flush,
#جيڪڏهن_پورو
#إذا VSTATUS
	CIDX_حالت,
#جيڪڏهن_پورو
	CIDX_min,
	CIDX_وقت,
};

#وصف_ڪريو CI_داخلا(n,s,o) n "\0"

/* Name given on command line */
ساكن ثابت حرف control_name[] ALIGN1 =
	CI_داخلا("intr",     CINTR,   VINTR   )
	CI_داخلا("quit",     CQUIT,   VQUIT   )
	CI_داخلا("erase",    CERASE,  VERASE  )
	CI_داخلا("kill",     CKILL,   VKILL   )
	CI_داخلا("eof",      CEOF,    VEOF    )
	CI_داخلا("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_داخلا("eol2",     CEOL2,   VEOL2   )
#جيڪڏهن_پورو
#إذا VSWTCH
	CI_داخلا("swtch",    CSWTCH,  VSWTCH  )
#جيڪڏهن_پورو
	CI_داخلا("start",    CSTART,  VSTART  )
	CI_داخلا("stop",     CSTOP,   VSTOP   )
	CI_داخلا("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_داخلا("dsusp",    CDSUSP,  VDSUSP  )
#جيڪڏهن_پورو
#إذا VREPRINT
	CI_داخلا("rprnt",    CRPRNT,  VREPRINT)
#جيڪڏهن_پورو
#إذا VWERASE
	CI_داخلا("werase",   CWERASE, VWERASE )
#جيڪڏهن_پورو
#إذا VLNEXT
	CI_داخلا("lnext",    CLNEXT,  VLNEXT  )
#جيڪڏهن_پورو
#إذا VFLUSHO
	CI_داخلا("flush",    CFLUSHO, VFLUSHO )
#جيڪڏهن_پورو
#إذا VSTATUS
	CI_داخلا("status",   CSTATUS, VSTATUS )
#جيڪڏهن_پورو
	/* These must be last because of the display routines */
	CI_داخلا("min",      1,       VMIN    )
	CI_داخلا("time",     0,       VTIME   )
	;

#وصف_رد CI_داخلا
#وصف_ڪريو CI_داخلا(n,s,o) { s, o },

ساكن ثابت بنية control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_داخلا("intr",     CINTR,   VINTR   )
	CI_داخلا("quit",     CQUIT,   VQUIT   )
	CI_داخلا("erase",    CERASE,  VERASE  )
	CI_داخلا("kill",     CKILL,   VKILL   )
	CI_داخلا("eof",      CEOF,    VEOF    )
	CI_داخلا("eol",      CEOL,    VEOL    )
#إذا VEOL2
	CI_داخلا("eol2",     CEOL2,   VEOL2   )
#جيڪڏهن_پورو
#إذا VSWTCH
	CI_داخلا("swtch",    CSWTCH,  VSWTCH  )
#جيڪڏهن_پورو
	CI_داخلا("start",    CSTART,  VSTART  )
	CI_داخلا("stop",     CSTOP,   VSTOP   )
	CI_داخلا("susp",     CSUSP,   VSUSP   )
#إذا VDSUSP
	CI_داخلا("dsusp",    CDSUSP,  VDSUSP  )
#جيڪڏهن_پورو
#إذا VREPRINT
	CI_داخلا("rprnt",    CRPRNT,  VREPRINT)
#جيڪڏهن_پورو
#إذا VWERASE
	CI_داخلا("werase",   CWERASE, VWERASE )
#جيڪڏهن_پورو
#إذا VLNEXT
	CI_داخلا("lnext",    CLNEXT,  VLNEXT  )
#جيڪڏهن_پورو
#إذا VFLUSHO
	CI_داخلا("flush",    CFLUSHO, VFLUSHO )
#جيڪڏهن_پورو
#إذا VSTATUS
	CI_داخلا("status",   CSTATUS, VSTATUS )
#جيڪڏهن_پورو
	/* These must be last because of the display routines */
	CI_داخلا("min",      1,       VMIN    )
	CI_داخلا("time",     0,       VTIME   )
};

تعداد {
	انگ_control_info = ARRAY_سائيز(control_info)
};


بنية globals {
	ثابت حرف *device_name;
	/* The width of the screen, for output wrapping */
	بنانشان max_کانو;
	/* Current position, to know when to wrap */
	بنانشان current_کانو;
} FIX_ALIASING;
#وصف_ڪريو G (*(بنية globals*)bb_common_bufsiz1)
#وصف_ڪريو INIT_G() افعل { \
	تياري_common_bufsiz(); \
	G.device_name = bb_نياپو_standard_داخلا; \
	G.max_کانو = 80; \
	G.current_کانو = 0; /* we are noexec, must clear */ \
} طالما (0)

ساكن فراغ سيٽ_رفتار_or_die(تعداد رفتار_setting type, ثابت حرف *arg,
					بنية termios *گهڻائي)
{
	رفتار_t baud;

	baud = tty_قدر_منزل_baud(xatou(arg));

	إذا (type != output_رفتار) {     /* either input or both */
		cfsetispeed(گهڻائي, baud);
	}
	إذا (type != داخلا_رفتار) {      /* either output or both */
		cfsetospeed(گهڻائي, baud);
	}
}

ساكن NORETURN فراغ perror_on_device_and_die(ثابت حرف *fmt)
{
	bb_perror_نياپو_and_die(fmt, G.device_name);
}

ساكن فراغ perror_on_device(ثابت حرف *fmt)
{
	bb_perror_نياپو(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ساكن فراغ wrapf(ثابت حرف *پيغام, ...)
{
	حرف buf[128];
	va_list دليلون;
	بنانشان buflen;

	va_شروعات(دليلون, پيغام);
	buflen = vsnprintf(buf, حجم(buf), پيغام, دليلون);
	va_پڇاڙي(دليلون);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	إذا (!buflen || buflen >= حجم(buf)) أرجع;

	إذا (G.current_کانو > 0) {
		G.current_کانو++;
		إذا (buf[0] != '\n') {
			إذا (G.current_کانو + buflen >= G.max_کانو) {
				G.current_کانو = 0;
				bb_putchar('\n');
			} وإلا {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_کانو += buflen;
	إذا (buf[buflen-1] == '\n')
		G.current_کانو = 0;
}

ساكن فراغ newline(فراغ)
{
	إذا (G.current_کانو != 0)
		wrapf("\n");
}

#جيڪڏهن_وصف_ٿيل TIOCGWINSZ
ساكن فراغ سيٽ_دري_سائيز(صحيح صفون, صحيح کانن_ڳڻپ)
{
	بنية winsize win = { 0, 0, 0, 0 };

	إذا (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		إذا (errno != EINVAL) {
			اذهب bail;
		}
		memset(&win, 0, حجم(win));
	}

	إذا (صفون >= 0)
		win.ws_صف = صفون;
	إذا (کانن_ڳڻپ >= 0)
		win.ws_کانو = کانن_ڳڻپ;

	إذا (ioctl(STDIN_FILENO, TIOCSWINSZ, (حرف *) &win))
bail:
		perror_on_device("%s");
}
#جيڪڏهن_پورو

ساكن فراغ display_دري_سائيز(صحيح fancy)
{
	ثابت حرف *fmt_str = "%s\0%s: no size information for this device";
	بنانشان ويڪر, اوچائي;

	إذا (get_terminal_ويڪر_اوچائي(STDIN_FILENO, &ويڪر, &اوچائي)) {
		إذا ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} وإلا {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				اوچائي, ويڪر);
	}
}

ساكن ثابت بنية suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ساكن ثابت بنية گهڻائي_info *ڳوليو_گهڻائي(ثابت حرف *name)
{
	صحيح i = اشاريو_اندريون_لڙيون(گهڻائي_name, name);
	أرجع i >= 0 ? &گهڻائي_info[i] : عدم;
}

ساكن ثابت بنية control_info *ڳوليو_control(ثابت حرف *name)
{
	صحيح i = اشاريو_اندريون_لڙيون(control_name, name);
	أرجع i >= 0 ? &control_info[i] : عدم;
}

تعداد {
	param_need_arg = 0x80,
	param_سٽ    = 1 | 0x80,
	param_صفون    = 2 | 0x80,
	param_کانن_ڳڻپ    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_سائيز    = 5,
	param_رفتار   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ساكن صحيح ڳوليو_param(ثابت حرف *name)
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
	صحيح i = اشاريو_اندريون_لڙيون(params, name) + 1;
	إذا (i == 0)
		أرجع 0;
	إذا (i != 5 && i != 6)
		i |= 0x80;
	أرجع i;
}

ساكن صحيح recover_گهڻائي(ثابت حرف *arg, بنية termios *گهڻائي)
{
	صحيح i, n;
	بنانشان chr;
	بنانشان ڊگهو iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	إذا (لڙي_مان_پڙهو(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		أرجع 0;
	گهڻائي->c_iflag = iflag;
	گهڻائي->c_oflag = oflag;
	گهڻائي->c_cflag = cflag;
	گهڻائي->c_lflag = lflag;
	arg += n;
	لكل (i = 0; i < NCCS; ++i) {
		إذا (لڙي_مان_پڙهو(arg, ":%x%n", &chr, &n) != 1)
			أرجع 0;
		گهڻائي->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	إذا (*arg != '\0')
		أرجع 0;

	أرجع 1;
}

ساكن فراغ display_recoverable(ثابت بنية termios *گهڻائي,
				صحيح اڻ_استعمال_ٿيل_PARAM dummy)
{
	صحيح i;
	لکو("%lx:%lx:%lx:%lx",
		   (بنانشان ڊگهو) گهڻائي->c_iflag, (بنانشان ڊگهو) گهڻائي->c_oflag,
		   (بنانشان ڊگهو) گهڻائي->c_cflag, (بنانشان ڊگهو) گهڻائي->c_lflag);
	لكل (i = 0; i < NCCS; ++i)
		لکو(":%x", (بنانشان صحيح) گهڻائي->c_cc[i]);
	bb_putchar('\n');
}

ساكن فراغ display_رفتار(ثابت بنية termios *گهڻائي, صحيح fancy)
{
	//____________________ 01234567 8 9
	ثابت حرف *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	بنانشان ڊگهو ispeed, ospeed;

	ispeed = cfgetispeed(گهڻائي);
	ospeed = cfgetospeed(گهڻائي);
	إذا (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	إذا (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_منزل_قدر(ispeed), tty_baud_منزل_قدر(ospeed));
}

ساكن فراغ do_display(ثابت بنية termios *گهڻائي, صحيح all)
{
	صحيح i;
	tcflag_t *bitsp;
	بنانشان ڊگهو نقاب;
	صحيح prev_type = control;

	display_رفتار(گهڻائي, 1);
	إذا (all)
		display_دري_سائيز(1);
#جيڪڏهن_وصف_ٿيل __linux__
	wrapf("line = %u;\n", گهڻائي->c_سٽ);
#وإلا
	newline();
#جيڪڏهن_پورو

	لكل (i = 0; i != CIDX_min; ++i) {
		حرف ch;
		حرف buf10[10];

		/* If swtch is the same as susp, don't print both */
#إذا VSWTCH == VSUSP
		إذا (i == CIDX_swtch)
			استمر;
#جيڪڏهن_پورو
		/* If eof uses the same slot as min, only print whichever applies */
#إذا VEOF == VMIN
		إذا (!(گهڻائي->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			استمر;
		}
#جيڪڏهن_پورو
		ch = گهڻائي->c_cc[control_info[i].وٿي];
		إذا (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		وإلا
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#إذا VEOF == VMIN
	إذا ((گهڻائي->c_lflag & ICANON) == 0)
#جيڪڏهن_پورو
		wrapf("min = %u; time = %u;", گهڻائي->c_cc[VMIN], گهڻائي->c_cc[VTIME]);
	newline();

	لكل (i = 0; i < انگ_گهڻائي_info; ++i) {
		إذا (گهڻائي_info[i].flags & OMIT)
			استمر;
		إذا (گهڻائي_info[i].type != prev_type) {
			newline();
			prev_type = گهڻائي_info[i].type;
		}

		bitsp = get_ptr_منزل_tcflag(گهڻائي_info[i].type, گهڻائي);
		نقاب = گهڻائي_info[i].نقاب ? گهڻائي_info[i].نقاب : گهڻائي_info[i].بٽ;
		إذا ((*bitsp & نقاب) == گهڻائي_info[i].بٽ) {
			إذا (all || (گهڻائي_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(گهڻائي_name, i));
		} وإلا {
			إذا ((all && گهڻائي_info[i].flags & REV)
			 || (!all && (گهڻائي_info[i].flags & (SANE_سيٽ | REV)) == (SANE_سيٽ | REV))
			) {
				wrapf("-%s", nth_string(گهڻائي_name, i));
			}
		}
	}
	newline();
}

ساكن فراغ sane_گهڻائي(بنية termios *گهڻائي)
{
	صحيح i;

	لكل (i = 0; i < انگ_control_info; ++i) {
#إذا VMIN == VEOF
		إذا (i == CIDX_min)
			اكسر;
#جيڪڏهن_پورو
		گهڻائي->c_cc[control_info[i].وٿي] = control_info[i].saneval;
	}

	لكل (i = 0; i < انگ_گهڻائي_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_منزل_tcflag(گهڻائي_info[i].type, گهڻائي);

		إذا (!bitsp)
			استمر;
		val = *bitsp & ~((بنانشان ڊگهو)گهڻائي_info[i].نقاب);
		إذا (گهڻائي_info[i].flags & SANE_سيٽ) {
			*bitsp = val | گهڻائي_info[i].بٽ;
		} وإلا
		إذا (گهڻائي_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~گهڻائي_info[i].بٽ;
		}
	}
}

ساكن فراغ سيٽ_گهڻائي(ثابت بنية گهڻائي_info *info, صحيح reversed,
					بنية termios *گهڻائي)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_منزل_tcflag(info->type, گهڻائي);

	إذا (bitsp) {
		tcflag_t val = *bitsp & ~info->نقاب;
		إذا (reversed)
			*bitsp = val & ~info->بٽ;
		وإلا
			*bitsp = val | info->بٽ;
		أرجع;
	}

	/* !bitsp - it's a "combination" mode */
	إذا (info == &گهڻائي_info[IDX_evenp] || info == &گهڻائي_info[IDX_parity]) {
		إذا (reversed)
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} وإلا إذا (info == &گهڻائي_info[IDX_oddp]) {
		إذا (reversed)
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~PARENB & ~CSIZE) | CS8;
		وإلا
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} وإلا إذا (info == &گهڻائي_info[IDX_nl]) {
		إذا (reversed) {
			گهڻائي->c_iflag = (گهڻائي->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			گهڻائي->c_oflag = (گهڻائي->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} وإلا {
			گهڻائي->c_iflag = گهڻائي->c_iflag & ~ICRNL;
			إذا (ONLCR) گهڻائي->c_oflag = گهڻائي->c_oflag & ~ONLCR;
		}
	} وإلا إذا (info == &گهڻائي_info[IDX_ek]) {
		گهڻائي->c_cc[VERASE] = CERASE;
		گهڻائي->c_cc[VKILL] = CKILL;
	} وإلا إذا (info == &گهڻائي_info[IDX_sane]) {
		sane_گهڻائي(گهڻائي);
	} وإلا إذا (info == &گهڻائي_info[IDX_cbreak]) {
		إذا (reversed)
			گهڻائي->c_lflag |= ICANON;
		وإلا
			گهڻائي->c_lflag &= ~ICANON;
	} وإلا إذا (info == &گهڻائي_info[IDX_pass8]) {
		إذا (reversed) {
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~CSIZE) | CS7 | PARENB;
			گهڻائي->c_iflag |= ISTRIP;
		} وإلا {
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~PARENB & ~CSIZE) | CS8;
			گهڻائي->c_iflag &= ~ISTRIP;
		}
	} وإلا إذا (info == &گهڻائي_info[IDX_litout]) {
		إذا (reversed) {
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~CSIZE) | CS7 | PARENB;
			گهڻائي->c_iflag |= ISTRIP;
			گهڻائي->c_oflag |= OPOST;
		} وإلا {
			گهڻائي->c_cflag = (گهڻائي->c_cflag & ~PARENB & ~CSIZE) | CS8;
			گهڻائي->c_iflag &= ~ISTRIP;
			گهڻائي->c_oflag &= ~OPOST;
		}
	} وإلا إذا (info == &گهڻائي_info[IDX_raw] || info == &گهڻائي_info[IDX_cooked]) {
		إذا ((info == &گهڻائي_info[IDX_raw] && reversed)
		 || (info == &گهڻائي_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			گهڻائي->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			گهڻائي->c_oflag |= OPOST;
			گهڻائي->c_lflag |= ISIG | ICANON;
#إذا VMIN == VEOF
			گهڻائي->c_cc[VEOF] = CEOF;
#جيڪڏهن_پورو
#إذا VTIME == VEOL
			گهڻائي->c_cc[VEOL] = CEOL;
#جيڪڏهن_پورو
		} وإلا {
			/* Raw mode */
			گهڻائي->c_iflag = 0;
			گهڻائي->c_oflag &= ~OPOST;
			گهڻائي->c_lflag &= ~(ISIG | ICANON | XCASE);
			گهڻائي->c_cc[VMIN] = 1;
			گهڻائي->c_cc[VTIME] = 0;
		}
	}
#إذا IXANY
	وإلا إذا (info == &گهڻائي_info[IDX_decctlq]) {
		إذا (reversed)
			گهڻائي->c_iflag |= IXANY;
		وإلا
			گهڻائي->c_iflag &= ~IXANY;
	}
#جيڪڏهن_پورو
#إذا TABDLY
	وإلا إذا (info == &گهڻائي_info[IDX_tabs]) {
		إذا (reversed)
			گهڻائي->c_oflag = (گهڻائي->c_oflag & ~TABDLY) | TAB3;
		وإلا
			گهڻائي->c_oflag = (گهڻائي->c_oflag & ~TABDLY) | TAB0;
	}
#جيڪڏهن_پورو
#إذا OXTABS
	وإلا إذا (info == &گهڻائي_info[IDX_tabs]) {
		إذا (reversed)
			گهڻائي->c_oflag |= OXTABS;
		وإلا
			گهڻائي->c_oflag &= ~OXTABS;
	}
#جيڪڏهن_پورو
#إذا XCASE && IUCLC && OLCUC
	وإلا إذا (info==&گهڻائي_info[IDX_lcase] || info==&گهڻائي_info[IDX_LCASE]) {
		إذا (reversed) {
			گهڻائي->c_lflag &= ~XCASE;
			گهڻائي->c_iflag &= ~IUCLC;
			گهڻائي->c_oflag &= ~OLCUC;
		} وإلا {
			گهڻائي->c_lflag |= XCASE;
			گهڻائي->c_iflag |= IUCLC;
			گهڻائي->c_oflag |= OLCUC;
		}
	}
#جيڪڏهن_پورو
	وإلا إذا (info == &گهڻائي_info[IDX_crt]) {
		گهڻائي->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} وإلا إذا (info == &گهڻائي_info[IDX_dec]) {
		گهڻائي->c_cc[VINTR] = 3; /* ^C */
		گهڻائي->c_cc[VERASE] = 127; /* DEL */
		گهڻائي->c_cc[VKILL] = 21; /* ^U */
		گهڻائي->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		إذا (IXANY) گهڻائي->c_iflag &= ~IXANY;
	}
}

ساكن فراغ سيٽ_control_char_or_die(ثابت بنية control_info *info,
			ثابت حرف *arg, بنية termios *گهڻائي)
{
	بنانشان حرف قدر_2;

	إذا (info == &control_info[CIDX_min] || info == &control_info[CIDX_وقت])
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
	گهڻائي->c_cc[info->وٿي] = قدر_2;
}

#وصف_ڪريو STTY_require_سيٽ_attr   (1 << 0)
#وصف_ڪريو STTY_رفتار_was_سيٽ      (1 << 1)
#وصف_ڪريو STTY_verbose_output     (1 << 2)
#وصف_ڪريو STTY_recoverable_output (1 << 3)
#وصف_ڪريو STTY_noargs             (1 << 4)

صحيح stty_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح stty_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	بنية termios گهڻائي;
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
		ثابت بنية گهڻائي_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			صحيح i;
			mp = ڳوليو_گهڻائي(arg+1);
			إذا (mp) {
				إذا (!(mp->flags & REV))
					اذهب invalid_دليل;
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
						bb_simple_error_نياپو_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					إذا (!file_name[0]) { /* nope, "-F device" */
						صحيح p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						إذا (!file_name)
							bb_error_نياپو_and_die(bb_نياپو_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						طالما (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					اذهب پڇاڙي_option;
				افتراضي:
					اذهب invalid_دليل;
				}
			}
 پڇاڙي_option:
			استمر;
		}

		mp = ڳوليو_گهڻائي(arg);
		إذا (mp) {
			stty_state &= ~STTY_noargs;
			استمر;
		}

		cp = ڳوليو_control(arg);
		إذا (cp) {
			إذا (!argnext)
				bb_error_نياپو_and_die(bb_نياپو_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			سيٽ_control_char_or_die(cp, argnext, &گهڻائي);
			stty_state &= ~STTY_noargs;
			++k;
			استمر;
		}

		param = ڳوليو_param(arg);
		إذا (param & param_need_arg) {
			إذا (!argnext)
				bb_error_نياپو_and_die(bb_نياپو_requires_arg, arg);
			++k;
		}

		بدل (param) {
#جيڪڏهن_وصف_ٿيل __linux__
		حالة param_سٽ:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, وڌ_کان_وڌ_عدد, stty_suffixes);
			اكسر;
# endif /* else fall-through */
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل TIOCGWINSZ
		حالة param_صفون:
		حالة param_کانن_ڳڻپ:
		حالة param_columns:
			xatoul_range_sfx(argnext, 1, وڌ_کان_وڌ_عدد, stty_suffixes);
			اكسر;
		حالة param_سائيز:
#جيڪڏهن_پورو
		حالة param_رفتار:
			اكسر;
		حالة param_ispeed:
			/* called for the side effect of xfunc death only */
			سيٽ_رفتار_or_die(داخلا_رفتار, argnext, &گهڻائي);
			اكسر;
		حالة param_ospeed:
			/* called for the side effect of xfunc death only */
			سيٽ_رفتار_or_die(output_رفتار, argnext, &گهڻائي);
			اكسر;
		افتراضي:
			إذا (recover_گهڻائي(arg, &گهڻائي) == 1) اكسر;
			إذا (tty_قدر_منزل_baud(xatou(arg)) != (رفتار_t) -1) اكسر;
 invalid_دليل:
			bb_error_نياپو_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_نياپو_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	إذا ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_نياپو_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	إذا (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&گهڻائي, 0, حجم(گهڻائي));
	إذا (tcgetattr(STDIN_FILENO, &گهڻائي))
		perror_on_device_and_die("%s");

	إذا (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_کانو = get_terminal_ويڪر(STDOUT_FILENO);
		output_func(&گهڻائي, display_all);
		أرجع EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	طالما (argv[++k]) {
		ثابت بنية گهڻائي_info *mp;
		ثابت بنية control_info *cp;
		ثابت حرف *arg = argv[k];
		ثابت حرف *argnext = argv[k+1];
		صحيح param;

		إذا (arg[0] == '-') {
			mp = ڳوليو_گهڻائي(arg+1);
			إذا (mp) {
				سيٽ_گهڻائي(mp, 1 /* reversed */, &گهڻائي);
				stty_state |= STTY_require_سيٽ_attr;
			}
			/* It is an option - already parsed. Skip it */
			استمر;
		}

		mp = ڳوليو_گهڻائي(arg);
		إذا (mp) {
			سيٽ_گهڻائي(mp, 0 /* non-reversed */, &گهڻائي);
			stty_state |= STTY_require_سيٽ_attr;
			استمر;
		}

		cp = ڳوليو_control(arg);
		إذا (cp) {
			++k;
			سيٽ_control_char_or_die(cp, argnext, &گهڻائي);
			stty_state |= STTY_require_سيٽ_attr;
			استمر;
		}

		param = ڳوليو_param(arg);
		إذا (param & param_need_arg) {
			++k;
		}

		بدل (param) {
#جيڪڏهن_وصف_ٿيل __linux__
		حالة param_سٽ:
			گهڻائي.c_سٽ = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_سيٽ_attr;
			اكسر;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل TIOCGWINSZ
		حالة param_کانن_ڳڻپ:
		حالة param_columns:
			سيٽ_دري_سائيز(-1, xatoul_sfx(argnext, stty_suffixes));
			اكسر;
		حالة param_سائيز:
			display_دري_سائيز(0);
			اكسر;
		حالة param_صفون:
			سيٽ_دري_سائيز(xatoul_sfx(argnext, stty_suffixes), -1);
			اكسر;
#جيڪڏهن_پورو
		حالة param_رفتار:
			display_رفتار(&گهڻائي, 0);
			اكسر;
		حالة param_ispeed:
			سيٽ_رفتار_or_die(داخلا_رفتار, argnext, &گهڻائي);
			stty_state |= (STTY_require_سيٽ_attr | STTY_رفتار_was_سيٽ);
			اكسر;
		حالة param_ospeed:
			سيٽ_رفتار_or_die(output_رفتار, argnext, &گهڻائي);
			stty_state |= (STTY_require_سيٽ_attr | STTY_رفتار_was_سيٽ);
			اكسر;
		افتراضي:
			إذا (recover_گهڻائي(arg, &گهڻائي) == 1)
				stty_state |= STTY_require_سيٽ_attr;
			وإلا /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				سيٽ_رفتار_or_die(both_speeds, arg, &گهڻائي);
				stty_state |= (STTY_require_سيٽ_attr | STTY_رفتار_was_سيٽ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	إذا (stty_state & STTY_require_سيٽ_attr) {
		بنية termios new_گهڻائي;

		إذا (tcsetattr(STDIN_FILENO, TCSADRAIN, &گهڻائي))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_گهڻائي, 0, حجم(new_گهڻائي));
		إذا (tcgetattr(STDIN_FILENO, &new_گهڻائي))
			perror_on_device_and_die("%s");

		إذا (memcmp(&گهڻائي, &new_گهڻائي, حجم(گهڻائي)) != 0) {
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

			new_گهڻائي.c_cflag &= (~CIBAUD);
			إذا ((stty_state & STTY_رفتار_was_سيٽ)
			 || memcmp(&گهڻائي, &new_گهڻائي, حجم(گهڻائي)) != 0)
#جيڪڏهن_پورو
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	أرجع EXIT_SUCCESS;
}
