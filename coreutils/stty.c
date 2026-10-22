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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _POSIX_VDISABLE
# define _POSIX_VDISABLE ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) 0)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CINTR
# define CINTR Control('c')
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CQUIT
# define CQUIT 28
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CERASE
# define CERASE 127
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CKILL
# define CKILL Control('u')
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CEOF
# define CEOF Control('d')
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CEOL
# define CEOL _POSIX_VDISABLE
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CSTART
# define CSTART Control('q')
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CSTOP
# define CSTOP Control('s')
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CSUSP
# define CSUSP Control('z')
#ਜੇ_ਅੰਤ
#ਜੇ defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#ਜੇ_ਅੰਤ
/* glibc-2.12.1 uses only VSWTC name */
#ਜੇ defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#ਜੇ_ਅੰਤ
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#ਜੇ defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#ਜੇ_ਅੰਤ
#ਜੇ defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#ਜੇ_ਅੰਤ

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#ਜੇ defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#ਜੇ_ਅੰਤ

#ਜੇ defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#ਜੇ_ਅੰਤ
#ਜੇ defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#ਜੇ_ਅੰਤ
#ਜੇ !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#ਜੇ_ਅੰਤ
#ਜੇ defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#ਜੇ_ਅੰਤ
#ਜੇ defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#ਜੇ_ਅੰਤ
#ਜੇ defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#ਜੇ_ਅੰਤ
#ਜੇ defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#ਜੇ_ਅੰਤ
#ਜੇ defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#ਜੇ_ਅੰਤ
#ਜੇ defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#ਜੇ_ਅੰਤ
#ਜੇ defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#ਜੇ_ਅੰਤ
#ਜੇ defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#ਜੇ_ਅੰਤ
#ਜੇ defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#ਜੇ_ਅੰਤ
#ਜੇ defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#ਜੇ_ਅੰਤ

/* Save us from #ifdef forest plague */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BSDLY
# define BSDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CIBAUD
# define CIBAUD 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CRDLY
# define CRDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CMSPAR
# define CMSPAR 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CRTSCTS
# define CRTSCTS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ECHOCTL
# define ECHOCTL 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ECHOKE
# define ECHOKE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ECHOPRT
# define ECHOPRT 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FFDLY
# define FFDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ IEXTEN
# define IEXTEN 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ IMAXBEL
# define IMAXBEL 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ IUCLC
# define IUCLC 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ IXANY
# define IXANY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NLDLY
# define NLDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OCRNL
# define OCRNL 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OFDEL
# define OFDEL 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OFILL
# define OFILL 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OLCUC
# define OLCUC 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ONLCR
# define ONLCR 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ONLRET
# define ONLRET 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ONOCR
# define ONOCR 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OXTABS
# define OXTABS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TABDLY
# define TABDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TAB1
# define TAB1 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TAB2
# define TAB2 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TOSTOP
# define TOSTOP 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VDSUSP
# define VDSUSP 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VEOL2
# define VEOL2 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VFLUSHO
# define VFLUSHO 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VLNEXT
# define VLNEXT 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VREPRINT
# define VREPRINT 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VSTATUS
# define VSTATUS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VSWTCH
# define VSWTCH 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VTDLY
# define VTDLY 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VWERASE
# define VWERASE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ XCASE
# define XCASE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ IUTF8
# define IUTF8 0
#ਜੇ_ਅੰਤ

/* Which speeds to set */
ਗਿਣਤੀ ਰਫਤਾਰ_setting {
	ਨਿਵੇਸ਼_ਰਫਤਾਰ, output_ਰਫਤਾਰ, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
ਗਿਣਤੀ {
	control, ਨਿਵੇਸ਼, output, local, combination
};
ਸਥਿਰ tcflag_t *get_ptr_ਮੰਜ਼ਿਲ_tcflag(ਬਿਨਾਨਿਸ਼ਾਨੀ type, ਅਚਲ ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	ਸਥਿਰ ਅਚਲ uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(ਬਣਤਰ termios, c_cflag), /* control */
		offsetof(ਬਣਤਰ termios, c_iflag), /*ਨਿਵੇਸ਼*/
		offsetof(ਬਣਤਰ termios, c_oflag), /* output */
		offsetof(ਬਣਤਰ termios, c_lflag)  /* local */
	};
	ਜੇ (type <= local) {
		ਵਾਪਸ (tcflag_t*) (((ਅੱਖਰ*)ਬਹੁਲਕ) + tcflag_offsets[type]);
	}
	ਵਾਪਸ ਨਿਰਾ;
}

/* Flags for 'struct mode_info' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SANE_ਸੈੱਟ 1              /* Set in 'sane' mode                  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SANE_UNSET 2            /* Unset in 'sane' mode                */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REV 4                   /* Can be turned off by prepending '-' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
ਬਣਤਰ ਬਹੁਲਕ_info {
	ਅਚਲ uint8_t type;           /* Which structure element to change    */
	ਅਚਲ uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#ਜੇ   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ਅਚਲ uint8_t ਨਕਾਬ;
#ਜਾਂਜੇ (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ਅਚਲ uint16_t ਨਕਾਬ;
#ਨਹੀਂਤਾਂ
	ਅਚਲ tcflag_t ਨਕਾਬ;          /* Other bits to turn off for this mode */
#ਜੇ_ਅੰਤ
	/* was using short here, but ppc32 was unhappy */
	ਅਚਲ tcflag_t ਬਿੱਟ;          /* Bits to set for this mode            */
};

ਗਿਣਤੀ {
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
#ਜੇ IXANY
	IDX_decctlq,
#ਜੇ_ਅੰਤ
#ਜੇ TABDLY || OXTABS
	IDX_tabs,
#ਜੇ_ਅੰਤ
#ਜੇ XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#ਜੇ_ਅੰਤ
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MI_ਇੰਦਰਾਜ(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ ਬਹੁਲਕ_name[] ALIGN1 =
	MI_ਇੰਦਰਾਜ("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("ek",       combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("sane",     combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("crt",      combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("dec",      combination, OMIT,              0,          0 )
#ਜੇ IXANY
	MI_ਇੰਦਰਾਜ("decctlq",  combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
#ਜੇ TABDLY || OXTABS
	MI_ਇੰਦਰਾਜ("tabs",     combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
#ਜੇ XCASE && IUCLC && OLCUC
	MI_ਇੰਦਰਾਜ("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("LCASE",    combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("parenb",   control,     REV,               PARENB,     0 )
	MI_ਇੰਦਰਾਜ("parodd",   control,     REV,               PARODD,     0 )
#ਜੇ CMSPAR
	MI_ਇੰਦਰਾਜ("cmspar",   control,     REV,               CMSPAR,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ਇੰਦਰਾਜ("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ਇੰਦਰਾਜ("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ਇੰਦਰਾਜ("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ਇੰਦਰਾਜ("cread",    control,     SANE_ਸੈੱਟ   | REV,  CREAD,      0 )
	MI_ਇੰਦਰਾਜ("clocal",   control,     REV,               CLOCAL,     0 )
#ਜੇ CRTSCTS
	MI_ਇੰਦਰਾਜ("crtscts",  control,     REV,               CRTSCTS,    0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("ignbrk",   ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ਇੰਦਰਾਜ("brkint",   ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  BRKINT,     0 )
	MI_ਇੰਦਰਾਜ("ignpar",   ਨਿਵੇਸ਼,       REV,               IGNPAR,     0 )
	MI_ਇੰਦਰਾਜ("parmrk",   ਨਿਵੇਸ਼,       REV,               PARMRK,     0 )
	MI_ਇੰਦਰਾਜ("inpck",    ਨਿਵੇਸ਼,       REV,               INPCK,      0 )
	MI_ਇੰਦਰਾਜ("istrip",   ਨਿਵੇਸ਼,       REV,               ISTRIP,     0 )
	MI_ਇੰਦਰਾਜ("inlcr",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ਇੰਦਰਾਜ("igncr",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ਇੰਦਰਾਜ("icrnl",    ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  ICRNL,      0 )
	MI_ਇੰਦਰਾਜ("ixon",     ਨਿਵੇਸ਼,       REV,               IXON,       0 )
	MI_ਇੰਦਰਾਜ("ixoff",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ਇੰਦਰਾਜ("tandem",   ਨਿਵੇਸ਼,       OMIT       | REV,  IXOFF,      0 )
#ਜੇ IUCLC
	MI_ਇੰਦਰਾਜ("iuclc",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IUCLC,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ IXANY
	MI_ਇੰਦਰਾਜ("ixany",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IXANY,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ IMAXBEL
	MI_ਇੰਦਰਾਜ("imaxbel",  ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  IMAXBEL,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ IUTF8
	MI_ਇੰਦਰਾਜ("iutf8",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IUTF8,      0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("opost",    output,      SANE_ਸੈੱਟ   | REV,  OPOST,      0 )
#ਜੇ OLCUC
	MI_ਇੰਦਰਾਜ("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ OCRNL
	MI_ਇੰਦਰਾਜ("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONLCR
	MI_ਇੰਦਰਾਜ("onlcr",    output,      SANE_ਸੈੱਟ   | REV,  ONLCR,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONOCR
	MI_ਇੰਦਰਾਜ("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONLRET
	MI_ਇੰਦਰਾਜ("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#ਜੇ_ਅੰਤ
#ਜੇ OFILL
	MI_ਇੰਦਰਾਜ("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ OFDEL
	MI_ਇੰਦਰਾਜ("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ NLDLY
	MI_ਇੰਦਰਾਜ("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ਇੰਦਰਾਜ("nl0",      output,      SANE_ਸੈੱਟ,          NL0,     NLDLY)
#ਜੇ_ਅੰਤ
#ਜੇ CRDLY
	MI_ਇੰਦਰਾਜ("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr0",      output,      SANE_ਸੈੱਟ,          CR0,     CRDLY)
#ਜੇ_ਅੰਤ

#ਜੇ TABDLY
	MI_ਇੰਦਰਾਜ("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ਜੇ TAB2
	MI_ਇੰਦਰਾਜ("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ਜੇ TAB1
	MI_ਇੰਦਰਾਜ("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ਇੰਦਰਾਜ("tab0",     output,      SANE_ਸੈੱਟ,          TAB0,   TABDLY)
#ਨਹੀਂਤਾਂ
# ਜੇ OXTABS
	MI_ਇੰਦਰਾਜ("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#ਜੇ_ਅੰਤ

#ਜੇ BSDLY
	MI_ਇੰਦਰਾਜ("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ਇੰਦਰਾਜ("bs0",      output,      SANE_ਸੈੱਟ,          BS0,     BSDLY)
#ਜੇ_ਅੰਤ
#ਜੇ VTDLY
	MI_ਇੰਦਰਾਜ("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ਇੰਦਰਾਜ("vt0",      output,      SANE_ਸੈੱਟ,          VT0,     VTDLY)
#ਜੇ_ਅੰਤ
#ਜੇ FFDLY
	MI_ਇੰਦਰਾਜ("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ਇੰਦਰਾਜ("ff0",      output,      SANE_ਸੈੱਟ,          FF0,     FFDLY)
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("isig",     local,       SANE_ਸੈੱਟ   | REV,  ISIG,       0 )
	MI_ਇੰਦਰਾਜ("icanon",   local,       SANE_ਸੈੱਟ   | REV,  ICANON,     0 )
#ਜੇ IEXTEN
	MI_ਇੰਦਰਾਜ("iexten",   local,       SANE_ਸੈੱਟ   | REV,  IEXTEN,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("echo",     local,       SANE_ਸੈੱਟ   | REV,  ECHO,       0 )
	MI_ਇੰਦਰਾਜ("echoe",    local,       SANE_ਸੈੱਟ   | REV,  ECHOE,      0 )
	MI_ਇੰਦਰਾਜ("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ਇੰਦਰਾਜ("echok",    local,       SANE_ਸੈੱਟ   | REV,  ECHOK,      0 )
	MI_ਇੰਦਰਾਜ("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ਇੰਦਰਾਜ("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ਜੇ XCASE
	MI_ਇੰਦਰਾਜ("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ TOSTOP
	MI_ਇੰਦਰਾਜ("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOPRT
	MI_ਇੰਦਰਾਜ("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ਇੰਦਰਾਜ("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOCTL
	MI_ਇੰਦਰਾਜ("echoctl",  local,       SANE_ਸੈੱਟ   | REV,  ECHOCTL,    0 )
	MI_ਇੰਦਰਾਜ("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOKE
	MI_ਇੰਦਰਾਜ("echoke",   local,       SANE_ਸੈੱਟ   | REV,  ECHOKE,     0 )
	MI_ਇੰਦਰਾਜ("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EXTPROC
	MI_ਇੰਦਰਾਜ("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#ਜੇ_ਅੰਤ
	;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MI_ਇੰਦਰਾਜ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MI_ਇੰਦਰਾਜ(N,T,F,B,M) { T, F, M, B },

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ਬਹੁਲਕ_info ਬਹੁਲਕ_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_ਇੰਦਰਾਜ("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("ek",       combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("sane",     combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("crt",      combination, OMIT,              0,          0 )
	MI_ਇੰਦਰਾਜ("dec",      combination, OMIT,              0,          0 )
#ਜੇ IXANY
	MI_ਇੰਦਰਾਜ("decctlq",  combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
#ਜੇ TABDLY || OXTABS
	MI_ਇੰਦਰਾਜ("tabs",     combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
#ਜੇ XCASE && IUCLC && OLCUC
	MI_ਇੰਦਰਾਜ("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ਇੰਦਰਾਜ("LCASE",    combination, REV        | OMIT, 0,          0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("parenb",   control,     REV,               PARENB,     0 )
	MI_ਇੰਦਰਾਜ("parodd",   control,     REV,               PARODD,     0 )
#ਜੇ CMSPAR
	MI_ਇੰਦਰਾਜ("cmspar",   control,     REV,               CMSPAR,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ਇੰਦਰਾਜ("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ਇੰਦਰਾਜ("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ਇੰਦਰਾਜ("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ਇੰਦਰਾਜ("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ਇੰਦਰਾਜ("cread",    control,     SANE_ਸੈੱਟ   | REV,  CREAD,      0 )
	MI_ਇੰਦਰਾਜ("clocal",   control,     REV,               CLOCAL,     0 )
#ਜੇ CRTSCTS
	MI_ਇੰਦਰਾਜ("crtscts",  control,     REV,               CRTSCTS,    0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("ignbrk",   ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ਇੰਦਰਾਜ("brkint",   ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  BRKINT,     0 )
	MI_ਇੰਦਰਾਜ("ignpar",   ਨਿਵੇਸ਼,       REV,               IGNPAR,     0 )
	MI_ਇੰਦਰਾਜ("parmrk",   ਨਿਵੇਸ਼,       REV,               PARMRK,     0 )
	MI_ਇੰਦਰਾਜ("inpck",    ਨਿਵੇਸ਼,       REV,               INPCK,      0 )
	MI_ਇੰਦਰਾਜ("istrip",   ਨਿਵੇਸ਼,       REV,               ISTRIP,     0 )
	MI_ਇੰਦਰਾਜ("inlcr",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ਇੰਦਰਾਜ("igncr",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ਇੰਦਰਾਜ("icrnl",    ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  ICRNL,      0 )
	MI_ਇੰਦਰਾਜ("ixon",     ਨਿਵੇਸ਼,       REV,               IXON,       0 )
	MI_ਇੰਦਰਾਜ("ixoff",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ਇੰਦਰਾਜ("tandem",   ਨਿਵੇਸ਼,       OMIT       | REV,  IXOFF,      0 )
#ਜੇ IUCLC
	MI_ਇੰਦਰਾਜ("iuclc",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IUCLC,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ IXANY
	MI_ਇੰਦਰਾਜ("ixany",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IXANY,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ IMAXBEL
	MI_ਇੰਦਰਾਜ("imaxbel",  ਨਿਵੇਸ਼,       SANE_ਸੈੱਟ   | REV,  IMAXBEL,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ IUTF8
	MI_ਇੰਦਰਾਜ("iutf8",    ਨਿਵੇਸ਼,       SANE_UNSET | REV,  IUTF8,      0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("opost",    output,      SANE_ਸੈੱਟ   | REV,  OPOST,      0 )
#ਜੇ OLCUC
	MI_ਇੰਦਰਾਜ("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ OCRNL
	MI_ਇੰਦਰਾਜ("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONLCR
	MI_ਇੰਦਰਾਜ("onlcr",    output,      SANE_ਸੈੱਟ   | REV,  ONLCR,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONOCR
	MI_ਇੰਦਰਾਜ("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ ONLRET
	MI_ਇੰਦਰਾਜ("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#ਜੇ_ਅੰਤ
#ਜੇ OFILL
	MI_ਇੰਦਰਾਜ("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ OFDEL
	MI_ਇੰਦਰਾਜ("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ NLDLY
	MI_ਇੰਦਰਾਜ("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ਇੰਦਰਾਜ("nl0",      output,      SANE_ਸੈੱਟ,          NL0,     NLDLY)
#ਜੇ_ਅੰਤ
#ਜੇ CRDLY
	MI_ਇੰਦਰਾਜ("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ਇੰਦਰਾਜ("cr0",      output,      SANE_ਸੈੱਟ,          CR0,     CRDLY)
#ਜੇ_ਅੰਤ

#ਜੇ TABDLY
	MI_ਇੰਦਰਾਜ("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ਜੇ TAB2
	MI_ਇੰਦਰਾਜ("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ਜੇ TAB1
	MI_ਇੰਦਰਾਜ("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ਇੰਦਰਾਜ("tab0",     output,      SANE_ਸੈੱਟ,          TAB0,   TABDLY)
#ਨਹੀਂਤਾਂ
# ਜੇ OXTABS
	MI_ਇੰਦਰਾਜ("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#ਜੇ_ਅੰਤ

#ਜੇ BSDLY
	MI_ਇੰਦਰਾਜ("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ਇੰਦਰਾਜ("bs0",      output,      SANE_ਸੈੱਟ,          BS0,     BSDLY)
#ਜੇ_ਅੰਤ
#ਜੇ VTDLY
	MI_ਇੰਦਰਾਜ("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ਇੰਦਰਾਜ("vt0",      output,      SANE_ਸੈੱਟ,          VT0,     VTDLY)
#ਜੇ_ਅੰਤ
#ਜੇ FFDLY
	MI_ਇੰਦਰਾਜ("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ਇੰਦਰਾਜ("ff0",      output,      SANE_ਸੈੱਟ,          FF0,     FFDLY)
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("isig",     local,       SANE_ਸੈੱਟ   | REV,  ISIG,       0 )
	MI_ਇੰਦਰਾਜ("icanon",   local,       SANE_ਸੈੱਟ   | REV,  ICANON,     0 )
#ਜੇ IEXTEN
	MI_ਇੰਦਰਾਜ("iexten",   local,       SANE_ਸੈੱਟ   | REV,  IEXTEN,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("echo",     local,       SANE_ਸੈੱਟ   | REV,  ECHO,       0 )
	MI_ਇੰਦਰਾਜ("echoe",    local,       SANE_ਸੈੱਟ   | REV,  ECHOE,      0 )
	MI_ਇੰਦਰਾਜ("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ਇੰਦਰਾਜ("echok",    local,       SANE_ਸੈੱਟ   | REV,  ECHOK,      0 )
	MI_ਇੰਦਰਾਜ("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ਇੰਦਰਾਜ("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ਜੇ XCASE
	MI_ਇੰਦਰਾਜ("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#ਜੇ_ਅੰਤ
#ਜੇ TOSTOP
	MI_ਇੰਦਰਾਜ("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOPRT
	MI_ਇੰਦਰਾਜ("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ਇੰਦਰਾਜ("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOCTL
	MI_ਇੰਦਰਾਜ("echoctl",  local,       SANE_ਸੈੱਟ   | REV,  ECHOCTL,    0 )
	MI_ਇੰਦਰਾਜ("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#ਜੇ_ਅੰਤ
#ਜੇ ECHOKE
	MI_ਇੰਦਰਾਜ("echoke",   local,       SANE_ਸੈੱਟ   | REV,  ECHOKE,     0 )
	MI_ਇੰਦਰਾਜ("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#ਜੇ_ਅੰਤ
	MI_ਇੰਦਰਾਜ("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EXTPROC
	MI_ਇੰਦਰਾਜ("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#ਜੇ_ਅੰਤ
};

ਗਿਣਤੀ {
	ਅੰਕ_ਬਹੁਲਕ_info = ARRAY_ਮਾਪ(ਬਹੁਲਕ_info)
};


/* Control characters */
ਬਣਤਰ control_info {
	ਅਚਲ uint8_t saneval;  /* Value to set for 'stty sane' */
	ਅਚਲ uint8_t ਵਿੱਥ;   /* Offset in c_cc */
};

ਗਿਣਤੀ {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#ਜੇ VEOL2
	CIDX_eol2,
#ਜੇ_ਅੰਤ
#ਜੇ VSWTCH
	CIDX_swtch,
#ਜੇ_ਅੰਤ
	CIDX_ਸ਼ੁਰੂ,
	CIDX_stop,
	CIDX_susp,
#ਜੇ VDSUSP
	CIDX_dsusp,
#ਜੇ_ਅੰਤ
#ਜੇ VREPRINT
	CIDX_rprnt,
#ਜੇ_ਅੰਤ
#ਜੇ VWERASE
	CIDX_werase,
#ਜੇ_ਅੰਤ
#ਜੇ VLNEXT
	CIDX_lnext,
#ਜੇ_ਅੰਤ
#ਜੇ VFLUSHO
	CIDX_flush,
#ਜੇ_ਅੰਤ
#ਜੇ VSTATUS
	CIDX_ਸਥਿਤੀ,
#ਜੇ_ਅੰਤ
	CIDX_min,
	CIDX_ਸਮਾਂ,
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CI_ਇੰਦਰਾਜ(n,s,o) n "\0"

/* Name given on command line */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ control_name[] ALIGN1 =
	CI_ਇੰਦਰਾਜ("intr",     CINTR,   VINTR   )
	CI_ਇੰਦਰਾਜ("quit",     CQUIT,   VQUIT   )
	CI_ਇੰਦਰਾਜ("erase",    CERASE,  VERASE  )
	CI_ਇੰਦਰਾਜ("kill",     CKILL,   VKILL   )
	CI_ਇੰਦਰਾਜ("eof",      CEOF,    VEOF    )
	CI_ਇੰਦਰਾਜ("eol",      CEOL,    VEOL    )
#ਜੇ VEOL2
	CI_ਇੰਦਰਾਜ("eol2",     CEOL2,   VEOL2   )
#ਜੇ_ਅੰਤ
#ਜੇ VSWTCH
	CI_ਇੰਦਰਾਜ("swtch",    CSWTCH,  VSWTCH  )
#ਜੇ_ਅੰਤ
	CI_ਇੰਦਰਾਜ("start",    CSTART,  VSTART  )
	CI_ਇੰਦਰਾਜ("stop",     CSTOP,   VSTOP   )
	CI_ਇੰਦਰਾਜ("susp",     CSUSP,   VSUSP   )
#ਜੇ VDSUSP
	CI_ਇੰਦਰਾਜ("dsusp",    CDSUSP,  VDSUSP  )
#ਜੇ_ਅੰਤ
#ਜੇ VREPRINT
	CI_ਇੰਦਰਾਜ("rprnt",    CRPRNT,  VREPRINT)
#ਜੇ_ਅੰਤ
#ਜੇ VWERASE
	CI_ਇੰਦਰਾਜ("werase",   CWERASE, VWERASE )
#ਜੇ_ਅੰਤ
#ਜੇ VLNEXT
	CI_ਇੰਦਰਾਜ("lnext",    CLNEXT,  VLNEXT  )
#ਜੇ_ਅੰਤ
#ਜੇ VFLUSHO
	CI_ਇੰਦਰਾਜ("flush",    CFLUSHO, VFLUSHO )
#ਜੇ_ਅੰਤ
#ਜੇ VSTATUS
	CI_ਇੰਦਰਾਜ("status",   CSTATUS, VSTATUS )
#ਜੇ_ਅੰਤ
	/* These must be last because of the display routines */
	CI_ਇੰਦਰਾਜ("min",      1,       VMIN    )
	CI_ਇੰਦਰਾਜ("time",     0,       VTIME   )
	;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ CI_ਇੰਦਰਾਜ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CI_ਇੰਦਰਾਜ(n,s,o) { s, o },

ਸਥਿਰ ਅਚਲ ਬਣਤਰ control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ਇੰਦਰਾਜ("intr",     CINTR,   VINTR   )
	CI_ਇੰਦਰਾਜ("quit",     CQUIT,   VQUIT   )
	CI_ਇੰਦਰਾਜ("erase",    CERASE,  VERASE  )
	CI_ਇੰਦਰਾਜ("kill",     CKILL,   VKILL   )
	CI_ਇੰਦਰਾਜ("eof",      CEOF,    VEOF    )
	CI_ਇੰਦਰਾਜ("eol",      CEOL,    VEOL    )
#ਜੇ VEOL2
	CI_ਇੰਦਰਾਜ("eol2",     CEOL2,   VEOL2   )
#ਜੇ_ਅੰਤ
#ਜੇ VSWTCH
	CI_ਇੰਦਰਾਜ("swtch",    CSWTCH,  VSWTCH  )
#ਜੇ_ਅੰਤ
	CI_ਇੰਦਰਾਜ("start",    CSTART,  VSTART  )
	CI_ਇੰਦਰਾਜ("stop",     CSTOP,   VSTOP   )
	CI_ਇੰਦਰਾਜ("susp",     CSUSP,   VSUSP   )
#ਜੇ VDSUSP
	CI_ਇੰਦਰਾਜ("dsusp",    CDSUSP,  VDSUSP  )
#ਜੇ_ਅੰਤ
#ਜੇ VREPRINT
	CI_ਇੰਦਰਾਜ("rprnt",    CRPRNT,  VREPRINT)
#ਜੇ_ਅੰਤ
#ਜੇ VWERASE
	CI_ਇੰਦਰਾਜ("werase",   CWERASE, VWERASE )
#ਜੇ_ਅੰਤ
#ਜੇ VLNEXT
	CI_ਇੰਦਰਾਜ("lnext",    CLNEXT,  VLNEXT  )
#ਜੇ_ਅੰਤ
#ਜੇ VFLUSHO
	CI_ਇੰਦਰਾਜ("flush",    CFLUSHO, VFLUSHO )
#ਜੇ_ਅੰਤ
#ਜੇ VSTATUS
	CI_ਇੰਦਰਾਜ("status",   CSTATUS, VSTATUS )
#ਜੇ_ਅੰਤ
	/* These must be last because of the display routines */
	CI_ਇੰਦਰਾਜ("min",      1,       VMIN    )
	CI_ਇੰਦਰਾਜ("time",     0,       VTIME   )
};

ਗਿਣਤੀ {
	ਅੰਕ_control_info = ARRAY_ਮਾਪ(control_info)
};


ਬਣਤਰ globals {
	ਅਚਲ ਅੱਖਰ *device_name;
	/* The width of the screen, for output wrapping */
	ਬਿਨਾਨਿਸ਼ਾਨੀ max_ਖਾਨਾ;
	/* Current position, to know when to wrap */
	ਬਿਨਾਨਿਸ਼ਾਨੀ current_ਖਾਨਾ;
} FIX_ALIASING;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	G.device_name = bb_ਸੁਨੇਹਾ_standard_ਨਿਵੇਸ਼; \
	G.max_ਖਾਨਾ = 80; \
	G.current_ਖਾਨਾ = 0; /* we are noexec, must clear */ \
} ਜਦੋਂ (0)

ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਰਫਤਾਰ_or_die(ਗਿਣਤੀ ਰਫਤਾਰ_setting type, ਅਚਲ ਅੱਖਰ *arg,
					ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	ਰਫਤਾਰ_t baud;

	baud = tty_ਮੁੱਲ_ਮੰਜ਼ਿਲ_baud(xatou(arg));

	ਜੇ (type != output_ਰਫਤਾਰ) {     /* either input or both */
		cfsetispeed(ਬਹੁਲਕ, baud);
	}
	ਜੇ (type != ਨਿਵੇਸ਼_ਰਫਤਾਰ) {      /* either output or both */
		cfsetospeed(ਬਹੁਲਕ, baud);
	}
}

ਸਥਿਰ NORETURN ਖਾਲੀ perror_on_device_and_die(ਅਚਲ ਅੱਖਰ *fmt)
{
	bb_perror_ਸੁਨੇਹਾ_and_die(fmt, G.device_name);
}

ਸਥਿਰ ਖਾਲੀ perror_on_device(ਅਚਲ ਅੱਖਰ *fmt)
{
	bb_perror_ਸੁਨੇਹਾ(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ਸਥਿਰ ਖਾਲੀ wrapf(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ...)
{
	ਅੱਖਰ buf[128];
	va_list ਦਲੀਲਾਂ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ buflen;

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, ਸੁਨੇਹਾ_2);
	buflen = vsnprintf(buf, ਆਕਾਰ(buf), ਸੁਨੇਹਾ_2, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	ਜੇ (!buflen || buflen >= ਆਕਾਰ(buf)) ਵਾਪਸ;

	ਜੇ (G.current_ਖਾਨਾ > 0) {
		G.current_ਖਾਨਾ++;
		ਜੇ (buf[0] != '\n') {
			ਜੇ (G.current_ਖਾਨਾ + buflen >= G.max_ਖਾਨਾ) {
				G.current_ਖਾਨਾ = 0;
				bb_putchar('\n');
			} ਨਹੀਂਤਾਂ {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_ਖਾਨਾ += buflen;
	ਜੇ (buf[buflen-1] == '\n')
		G.current_ਖਾਨਾ = 0;
}

ਸਥਿਰ ਖਾਲੀ newline(ਖਾਲੀ)
{
	ਜੇ (G.current_ਖਾਨਾ != 0)
		wrapf("\n");
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCGWINSZ
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਖਿੜਕੀ_ਮਾਪ(ਪੂਰਨ ਪੰਕਤੀਆਂ, ਪੂਰਨ ਖਾਨੇ_ਗਿਣਤੀ)
{
	ਬਣਤਰ winsize win = { 0, 0, 0, 0 };

	ਜੇ (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		ਜੇ (errno != EINVAL) {
			ਜਾਓ bail;
		}
		memset(&win, 0, ਆਕਾਰ(win));
	}

	ਜੇ (ਪੰਕਤੀਆਂ >= 0)
		win.ws_ਪੰਕਤੀ = ਪੰਕਤੀਆਂ;
	ਜੇ (ਖਾਨੇ_ਗਿਣਤੀ >= 0)
		win.ws_ਖਾਨਾ = ਖਾਨੇ_ਗਿਣਤੀ;

	ਜੇ (ioctl(STDIN_FILENO, TIOCSWINSZ, (ਅੱਖਰ *) &win))
bail:
		perror_on_device("%s");
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ display_ਖਿੜਕੀ_ਮਾਪ(ਪੂਰਨ fancy)
{
	ਅਚਲ ਅੱਖਰ *fmt_str = "%s\0%s: no size information for this device";
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ, ਉਚਾਈ;

	ਜੇ (get_terminal_ਚੌੜਾਈ_ਉਚਾਈ(STDIN_FILENO, &ਚੌੜਾਈ, &ਉਚਾਈ)) {
		ਜੇ ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} ਨਹੀਂਤਾਂ {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				ਉਚਾਈ, ਚੌੜਾਈ);
	}
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ਬਹੁਲਕ_info *ਲੱਭੋ_ਬਹੁਲਕ(ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ i = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(ਬਹੁਲਕ_name, name);
	ਵਾਪਸ i >= 0 ? &ਬਹੁਲਕ_info[i] : ਨਿਰਾ;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ control_info *ਲੱਭੋ_control(ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ i = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(control_name, name);
	ਵਾਪਸ i >= 0 ? &control_info[i] : ਨਿਰਾ;
}

ਗਿਣਤੀ {
	param_need_arg = 0x80,
	param_ਸਤਰ    = 1 | 0x80,
	param_ਪੰਕਤੀਆਂ    = 2 | 0x80,
	param_ਖਾਨੇ_ਗਿਣਤੀ    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_ਮਾਪ    = 5,
	param_ਰਫਤਾਰ   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_param(ਅਚਲ ਅੱਖਰ *name)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	ਪੂਰਨ i = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(params, name) + 1;
	ਜੇ (i == 0)
		ਵਾਪਸ 0;
	ਜੇ (i != 5 && i != 6)
		i |= 0x80;
	ਵਾਪਸ i;
}

ਸਥਿਰ ਪੂਰਨ recover_ਬਹੁਲਕ(ਅਚਲ ਅੱਖਰ *arg, ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	ਪੂਰਨ i, n;
	ਬਿਨਾਨਿਸ਼ਾਨੀ chr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		ਵਾਪਸ 0;
	ਬਹੁਲਕ->c_iflag = iflag;
	ਬਹੁਲਕ->c_oflag = oflag;
	ਬਹੁਲਕ->c_cflag = cflag;
	ਬਹੁਲਕ->c_lflag = lflag;
	arg += n;
	ਲਈ (i = 0; i < NCCS; ++i) {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(arg, ":%x%n", &chr, &n) != 1)
			ਵਾਪਸ 0;
		ਬਹੁਲਕ->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	ਜੇ (*arg != '\0')
		ਵਾਪਸ 0;

	ਵਾਪਸ 1;
}

ਸਥਿਰ ਖਾਲੀ display_recoverable(ਅਚਲ ਬਣਤਰ termios *ਬਹੁਲਕ,
				ਪੂਰਨ ਅਣਵਰਤਿਆ_PARAM dummy)
{
	ਪੂਰਨ i;
	ਲਿਖੋ("%lx:%lx:%lx:%lx",
		   (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਬਹੁਲਕ->c_iflag, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਬਹੁਲਕ->c_oflag,
		   (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਬਹੁਲਕ->c_cflag, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਬਹੁਲਕ->c_lflag);
	ਲਈ (i = 0; i < NCCS; ++i)
		ਲਿਖੋ(":%x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) ਬਹੁਲਕ->c_cc[i]);
	bb_putchar('\n');
}

ਸਥਿਰ ਖਾਲੀ display_ਰਫਤਾਰ(ਅਚਲ ਬਣਤਰ termios *ਬਹੁਲਕ, ਪੂਰਨ fancy)
{
	//____________________ 01234567 8 9
	ਅਚਲ ਅੱਖਰ *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ispeed, ospeed;

	ispeed = cfgetispeed(ਬਹੁਲਕ);
	ospeed = cfgetospeed(ਬਹੁਲਕ);
	ਜੇ (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	ਜੇ (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_ਮੰਜ਼ਿਲ_ਮੁੱਲ(ispeed), tty_baud_ਮੰਜ਼ਿਲ_ਮੁੱਲ(ospeed));
}

ਸਥਿਰ ਖਾਲੀ do_display(ਅਚਲ ਬਣਤਰ termios *ਬਹੁਲਕ, ਪੂਰਨ all)
{
	ਪੂਰਨ i;
	tcflag_t *bitsp;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਨਕਾਬ;
	ਪੂਰਨ prev_type = control;

	display_ਰਫਤਾਰ(ਬਹੁਲਕ, 1);
	ਜੇ (all)
		display_ਖਿੜਕੀ_ਮਾਪ(1);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	wrapf("line = %u;\n", ਬਹੁਲਕ->c_ਸਤਰ);
#ਨਹੀਂਤਾਂ
	newline();
#ਜੇ_ਅੰਤ

	ਲਈ (i = 0; i != CIDX_min; ++i) {
		ਅੱਖਰ ch;
		ਅੱਖਰ buf10[10];

		/* If swtch is the same as susp, don't print both */
#ਜੇ VSWTCH == VSUSP
		ਜੇ (i == CIDX_swtch)
			ਜਾਰੀ;
#ਜੇ_ਅੰਤ
		/* If eof uses the same slot as min, only print whichever applies */
#ਜੇ VEOF == VMIN
		ਜੇ (!(ਬਹੁਲਕ->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ
		ch = ਬਹੁਲਕ->c_cc[control_info[i].ਵਿੱਥ];
		ਜੇ (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		ਨਹੀਂਤਾਂ
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#ਜੇ VEOF == VMIN
	ਜੇ ((ਬਹੁਲਕ->c_lflag & ICANON) == 0)
#ਜੇ_ਅੰਤ
		wrapf("min = %u; time = %u;", ਬਹੁਲਕ->c_cc[VMIN], ਬਹੁਲਕ->c_cc[VTIME]);
	newline();

	ਲਈ (i = 0; i < ਅੰਕ_ਬਹੁਲਕ_info; ++i) {
		ਜੇ (ਬਹੁਲਕ_info[i].flags & OMIT)
			ਜਾਰੀ;
		ਜੇ (ਬਹੁਲਕ_info[i].type != prev_type) {
			newline();
			prev_type = ਬਹੁਲਕ_info[i].type;
		}

		bitsp = get_ptr_ਮੰਜ਼ਿਲ_tcflag(ਬਹੁਲਕ_info[i].type, ਬਹੁਲਕ);
		ਨਕਾਬ = ਬਹੁਲਕ_info[i].ਨਕਾਬ ? ਬਹੁਲਕ_info[i].ਨਕਾਬ : ਬਹੁਲਕ_info[i].ਬਿੱਟ;
		ਜੇ ((*bitsp & ਨਕਾਬ) == ਬਹੁਲਕ_info[i].ਬਿੱਟ) {
			ਜੇ (all || (ਬਹੁਲਕ_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(ਬਹੁਲਕ_name, i));
		} ਨਹੀਂਤਾਂ {
			ਜੇ ((all && ਬਹੁਲਕ_info[i].flags & REV)
			 || (!all && (ਬਹੁਲਕ_info[i].flags & (SANE_ਸੈੱਟ | REV)) == (SANE_ਸੈੱਟ | REV))
			) {
				wrapf("-%s", nth_string(ਬਹੁਲਕ_name, i));
			}
		}
	}
	newline();
}

ਸਥਿਰ ਖਾਲੀ sane_ਬਹੁਲਕ(ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ਅੰਕ_control_info; ++i) {
#ਜੇ VMIN == VEOF
		ਜੇ (i == CIDX_min)
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਬਹੁਲਕ->c_cc[control_info[i].ਵਿੱਥ] = control_info[i].saneval;
	}

	ਲਈ (i = 0; i < ਅੰਕ_ਬਹੁਲਕ_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_ਮੰਜ਼ਿਲ_tcflag(ਬਹੁਲਕ_info[i].type, ਬਹੁਲਕ);

		ਜੇ (!bitsp)
			ਜਾਰੀ;
		val = *bitsp & ~((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਬਹੁਲਕ_info[i].ਨਕਾਬ);
		ਜੇ (ਬਹੁਲਕ_info[i].flags & SANE_ਸੈੱਟ) {
			*bitsp = val | ਬਹੁਲਕ_info[i].ਬਿੱਟ;
		} ਨਹੀਂਤਾਂ
		ਜੇ (ਬਹੁਲਕ_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~ਬਹੁਲਕ_info[i].ਬਿੱਟ;
		}
	}
}

ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਬਹੁਲਕ(ਅਚਲ ਬਣਤਰ ਬਹੁਲਕ_info *info, ਪੂਰਨ reversed,
					ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_ਮੰਜ਼ਿਲ_tcflag(info->type, ਬਹੁਲਕ);

	ਜੇ (bitsp) {
		tcflag_t val = *bitsp & ~info->ਨਕਾਬ;
		ਜੇ (reversed)
			*bitsp = val & ~info->ਬਿੱਟ;
		ਨਹੀਂਤਾਂ
			*bitsp = val | info->ਬਿੱਟ;
		ਵਾਪਸ;
	}

	/* !bitsp - it's a "combination" mode */
	ਜੇ (info == &ਬਹੁਲਕ_info[IDX_evenp] || info == &ਬਹੁਲਕ_info[IDX_parity]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_oddp]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_nl]) {
		ਜੇ (reversed) {
			ਬਹੁਲਕ->c_iflag = (ਬਹੁਲਕ->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			ਬਹੁਲਕ->c_oflag = (ਬਹੁਲਕ->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} ਨਹੀਂਤਾਂ {
			ਬਹੁਲਕ->c_iflag = ਬਹੁਲਕ->c_iflag & ~ICRNL;
			ਜੇ (ONLCR) ਬਹੁਲਕ->c_oflag = ਬਹੁਲਕ->c_oflag & ~ONLCR;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_ek]) {
		ਬਹੁਲਕ->c_cc[VERASE] = CERASE;
		ਬਹੁਲਕ->c_cc[VKILL] = CKILL;
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_sane]) {
		sane_ਬਹੁਲਕ(ਬਹੁਲਕ);
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_cbreak]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_lflag |= ICANON;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_lflag &= ~ICANON;
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_pass8]) {
		ਜੇ (reversed) {
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ਬਹੁਲਕ->c_iflag |= ISTRIP;
		} ਨਹੀਂਤਾਂ {
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ਬਹੁਲਕ->c_iflag &= ~ISTRIP;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_litout]) {
		ਜੇ (reversed) {
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ਬਹੁਲਕ->c_iflag |= ISTRIP;
			ਬਹੁਲਕ->c_oflag |= OPOST;
		} ਨਹੀਂਤਾਂ {
			ਬਹੁਲਕ->c_cflag = (ਬਹੁਲਕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ਬਹੁਲਕ->c_iflag &= ~ISTRIP;
			ਬਹੁਲਕ->c_oflag &= ~OPOST;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_raw] || info == &ਬਹੁਲਕ_info[IDX_cooked]) {
		ਜੇ ((info == &ਬਹੁਲਕ_info[IDX_raw] && reversed)
		 || (info == &ਬਹੁਲਕ_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			ਬਹੁਲਕ->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			ਬਹੁਲਕ->c_oflag |= OPOST;
			ਬਹੁਲਕ->c_lflag |= ISIG | ICANON;
#ਜੇ VMIN == VEOF
			ਬਹੁਲਕ->c_cc[VEOF] = CEOF;
#ਜੇ_ਅੰਤ
#ਜੇ VTIME == VEOL
			ਬਹੁਲਕ->c_cc[VEOL] = CEOL;
#ਜੇ_ਅੰਤ
		} ਨਹੀਂਤਾਂ {
			/* Raw mode */
			ਬਹੁਲਕ->c_iflag = 0;
			ਬਹੁਲਕ->c_oflag &= ~OPOST;
			ਬਹੁਲਕ->c_lflag &= ~(ISIG | ICANON | XCASE);
			ਬਹੁਲਕ->c_cc[VMIN] = 1;
			ਬਹੁਲਕ->c_cc[VTIME] = 0;
		}
	}
#ਜੇ IXANY
	ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_decctlq]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_iflag |= IXANY;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_iflag &= ~IXANY;
	}
#ਜੇ_ਅੰਤ
#ਜੇ TABDLY
	ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_tabs]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_oflag = (ਬਹੁਲਕ->c_oflag & ~TABDLY) | TAB3;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_oflag = (ਬਹੁਲਕ->c_oflag & ~TABDLY) | TAB0;
	}
#ਜੇ_ਅੰਤ
#ਜੇ OXTABS
	ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_tabs]) {
		ਜੇ (reversed)
			ਬਹੁਲਕ->c_oflag |= OXTABS;
		ਨਹੀਂਤਾਂ
			ਬਹੁਲਕ->c_oflag &= ~OXTABS;
	}
#ਜੇ_ਅੰਤ
#ਜੇ XCASE && IUCLC && OLCUC
	ਨਹੀਂਤਾਂ ਜੇ (info==&ਬਹੁਲਕ_info[IDX_lcase] || info==&ਬਹੁਲਕ_info[IDX_LCASE]) {
		ਜੇ (reversed) {
			ਬਹੁਲਕ->c_lflag &= ~XCASE;
			ਬਹੁਲਕ->c_iflag &= ~IUCLC;
			ਬਹੁਲਕ->c_oflag &= ~OLCUC;
		} ਨਹੀਂਤਾਂ {
			ਬਹੁਲਕ->c_lflag |= XCASE;
			ਬਹੁਲਕ->c_iflag |= IUCLC;
			ਬਹੁਲਕ->c_oflag |= OLCUC;
		}
	}
#ਜੇ_ਅੰਤ
	ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_crt]) {
		ਬਹੁਲਕ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} ਨਹੀਂਤਾਂ ਜੇ (info == &ਬਹੁਲਕ_info[IDX_dec]) {
		ਬਹੁਲਕ->c_cc[VINTR] = 3; /* ^C */
		ਬਹੁਲਕ->c_cc[VERASE] = 127; /* DEL */
		ਬਹੁਲਕ->c_cc[VKILL] = 21; /* ^U */
		ਬਹੁਲਕ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		ਜੇ (IXANY) ਬਹੁਲਕ->c_iflag &= ~IXANY;
	}
}

ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_control_char_or_die(ਅਚਲ ਬਣਤਰ control_info *info,
			ਅਚਲ ਅੱਖਰ *arg, ਬਣਤਰ termios *ਬਹੁਲਕ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਮੁੱਲ_2;

	ਜੇ (info == &control_info[CIDX_min] || info == &control_info[CIDX_ਸਮਾਂ])
		ਮੁੱਲ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ਨਹੀਂਤਾਂ ਜੇ (arg[0] == '\0' || arg[1] == '\0')
		ਮੁੱਲ_2 = arg[0];
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		ਮੁੱਲ_2 = _POSIX_VDISABLE;
	ਨਹੀਂਤਾਂ ਜੇ (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		ਮੁੱਲ_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		ਜੇ (arg[1] == '?')
			ਮੁੱਲ_2 = 127;
	} ਨਹੀਂਤਾਂ
		ਮੁੱਲ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ਬਹੁਲਕ->c_cc[info->ਵਿੱਥ] = ਮੁੱਲ_2;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STTY_require_ਸੈੱਟ_attr   (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STTY_ਰਫਤਾਰ_was_ਸੈੱਟ      (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STTY_verbose_output     (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STTY_recoverable_output (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STTY_noargs             (1 << 4)

ਪੂਰਨ stty_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ stty_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਬਣਤਰ termios ਬਹੁਲਕ;
	ਖਾਲੀ (*output_func)(ਅਚਲ ਬਣਤਰ termios *, ਪੂਰਨ);
	ਅਚਲ ਅੱਖਰ *file_name = ਨਿਰਾ;
	ਪੂਰਨ display_all = 0;
	ਪੂਰਨ stty_state;
	ਪੂਰਨ k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	ਜਦੋਂ (argv[++k]) {
		ਅਚਲ ਬਣਤਰ ਬਹੁਲਕ_info *mp;
		ਅਚਲ ਬਣਤਰ control_info *cp;
		ਅਚਲ ਅੱਖਰ *arg = argv[k];
		ਅਚਲ ਅੱਖਰ *argnext = argv[k+1];
		ਪੂਰਨ param;

		ਜੇ (arg[0] == '-') {
			ਪੂਰਨ i;
			mp = ਲੱਭੋ_ਬਹੁਲਕ(arg+1);
			ਜੇ (mp) {
				ਜੇ (!(mp->flags & REV))
					ਜਾਓ invalid_ਦਲੀਲ;
				stty_state &= ~STTY_noargs;
				ਜਾਰੀ;
			}
			/* It is an option - parse it */
			i = 0;
			ਜਦੋਂ (arg[++i]) {
				ਚੋਣ (arg[i]) {
				ਹਾਲਤ 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					ਰੋਕੋ;
				ਹਾਲਤ 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					ਰੋਕੋ;
				ਹਾਲਤ 'F':
					ਜੇ (file_name)
						bb_simple_error_ਸੁਨੇਹਾ_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					ਜੇ (!file_name[0]) { /* nope, "-F device" */
						ਪੂਰਨ p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						ਜੇ (!file_name)
							bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						ਜਦੋਂ (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					ਜਾਓ ਸਿਰਾ_option;
				ਮੂਲ:
					ਜਾਓ invalid_ਦਲੀਲ;
				}
			}
 ਸਿਰਾ_option:
			ਜਾਰੀ;
		}

		mp = ਲੱਭੋ_ਬਹੁਲਕ(arg);
		ਜੇ (mp) {
			stty_state &= ~STTY_noargs;
			ਜਾਰੀ;
		}

		cp = ਲੱਭੋ_control(arg);
		ਜੇ (cp) {
			ਜੇ (!argnext)
				bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			ਸੈੱਟ_control_char_or_die(cp, argnext, &ਬਹੁਲਕ);
			stty_state &= ~STTY_noargs;
			++k;
			ਜਾਰੀ;
		}

		param = ਲੱਭੋ_param(arg);
		ਜੇ (param & param_need_arg) {
			ਜੇ (!argnext)
				bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_requires_arg, arg);
			++k;
		}

		ਚੋਣ (param) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		ਹਾਲਤ param_ਸਤਰ:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, stty_suffixes);
			ਰੋਕੋ;
# endif /* else fall-through */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCGWINSZ
		ਹਾਲਤ param_ਪੰਕਤੀਆਂ:
		ਹਾਲਤ param_ਖਾਨੇ_ਗਿਣਤੀ:
		ਹਾਲਤ param_columns:
			xatoul_range_sfx(argnext, 1, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, stty_suffixes);
			ਰੋਕੋ;
		ਹਾਲਤ param_ਮਾਪ:
#ਜੇ_ਅੰਤ
		ਹਾਲਤ param_ਰਫਤਾਰ:
			ਰੋਕੋ;
		ਹਾਲਤ param_ispeed:
			/* called for the side effect of xfunc death only */
			ਸੈੱਟ_ਰਫਤਾਰ_or_die(ਨਿਵੇਸ਼_ਰਫਤਾਰ, argnext, &ਬਹੁਲਕ);
			ਰੋਕੋ;
		ਹਾਲਤ param_ospeed:
			/* called for the side effect of xfunc death only */
			ਸੈੱਟ_ਰਫਤਾਰ_or_die(output_ਰਫਤਾਰ, argnext, &ਬਹੁਲਕ);
			ਰੋਕੋ;
		ਮੂਲ:
			ਜੇ (recover_ਬਹੁਲਕ(arg, &ਬਹੁਲਕ) == 1) ਰੋਕੋ;
			ਜੇ (tty_ਮੁੱਲ_ਮੰਜ਼ਿਲ_baud(xatou(arg)) != (ਰਫਤਾਰ_t) -1) ਰੋਕੋ;
 invalid_ਦਲੀਲ:
			bb_error_ਸੁਨੇਹਾ_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	ਜੇ ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_ਸੁਨੇਹਾ_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	ਜੇ ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_ਸੁਨੇਹਾ_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	ਜੇ (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&ਬਹੁਲਕ, 0, ਆਕਾਰ(ਬਹੁਲਕ));
	ਜੇ (tcgetattr(STDIN_FILENO, &ਬਹੁਲਕ))
		perror_on_device_and_die("%s");

	ਜੇ (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_ਖਾਨਾ = get_terminal_ਚੌੜਾਈ(STDOUT_FILENO);
		output_func(&ਬਹੁਲਕ, display_all);
		ਵਾਪਸ EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	ਜਦੋਂ (argv[++k]) {
		ਅਚਲ ਬਣਤਰ ਬਹੁਲਕ_info *mp;
		ਅਚਲ ਬਣਤਰ control_info *cp;
		ਅਚਲ ਅੱਖਰ *arg = argv[k];
		ਅਚਲ ਅੱਖਰ *argnext = argv[k+1];
		ਪੂਰਨ param;

		ਜੇ (arg[0] == '-') {
			mp = ਲੱਭੋ_ਬਹੁਲਕ(arg+1);
			ਜੇ (mp) {
				ਸੈੱਟ_ਬਹੁਲਕ(mp, 1 /* reversed */, &ਬਹੁਲਕ);
				stty_state |= STTY_require_ਸੈੱਟ_attr;
			}
			/* It is an option - already parsed. Skip it */
			ਜਾਰੀ;
		}

		mp = ਲੱਭੋ_ਬਹੁਲਕ(arg);
		ਜੇ (mp) {
			ਸੈੱਟ_ਬਹੁਲਕ(mp, 0 /* non-reversed */, &ਬਹੁਲਕ);
			stty_state |= STTY_require_ਸੈੱਟ_attr;
			ਜਾਰੀ;
		}

		cp = ਲੱਭੋ_control(arg);
		ਜੇ (cp) {
			++k;
			ਸੈੱਟ_control_char_or_die(cp, argnext, &ਬਹੁਲਕ);
			stty_state |= STTY_require_ਸੈੱਟ_attr;
			ਜਾਰੀ;
		}

		param = ਲੱਭੋ_param(arg);
		ਜੇ (param & param_need_arg) {
			++k;
		}

		ਚੋਣ (param) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		ਹਾਲਤ param_ਸਤਰ:
			ਬਹੁਲਕ.c_ਸਤਰ = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_ਸੈੱਟ_attr;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCGWINSZ
		ਹਾਲਤ param_ਖਾਨੇ_ਗਿਣਤੀ:
		ਹਾਲਤ param_columns:
			ਸੈੱਟ_ਖਿੜਕੀ_ਮਾਪ(-1, xatoul_sfx(argnext, stty_suffixes));
			ਰੋਕੋ;
		ਹਾਲਤ param_ਮਾਪ:
			display_ਖਿੜਕੀ_ਮਾਪ(0);
			ਰੋਕੋ;
		ਹਾਲਤ param_ਪੰਕਤੀਆਂ:
			ਸੈੱਟ_ਖਿੜਕੀ_ਮਾਪ(xatoul_sfx(argnext, stty_suffixes), -1);
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ param_ਰਫਤਾਰ:
			display_ਰਫਤਾਰ(&ਬਹੁਲਕ, 0);
			ਰੋਕੋ;
		ਹਾਲਤ param_ispeed:
			ਸੈੱਟ_ਰਫਤਾਰ_or_die(ਨਿਵੇਸ਼_ਰਫਤਾਰ, argnext, &ਬਹੁਲਕ);
			stty_state |= (STTY_require_ਸੈੱਟ_attr | STTY_ਰਫਤਾਰ_was_ਸੈੱਟ);
			ਰੋਕੋ;
		ਹਾਲਤ param_ospeed:
			ਸੈੱਟ_ਰਫਤਾਰ_or_die(output_ਰਫਤਾਰ, argnext, &ਬਹੁਲਕ);
			stty_state |= (STTY_require_ਸੈੱਟ_attr | STTY_ਰਫਤਾਰ_was_ਸੈੱਟ);
			ਰੋਕੋ;
		ਮੂਲ:
			ਜੇ (recover_ਬਹੁਲਕ(arg, &ਬਹੁਲਕ) == 1)
				stty_state |= STTY_require_ਸੈੱਟ_attr;
			ਨਹੀਂਤਾਂ /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				ਸੈੱਟ_ਰਫਤਾਰ_or_die(both_speeds, arg, &ਬਹੁਲਕ);
				stty_state |= (STTY_require_ਸੈੱਟ_attr | STTY_ਰਫਤਾਰ_was_ਸੈੱਟ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	ਜੇ (stty_state & STTY_require_ਸੈੱਟ_attr) {
		ਬਣਤਰ termios new_ਬਹੁਲਕ;

		ਜੇ (tcsetattr(STDIN_FILENO, TCSADRAIN, &ਬਹੁਲਕ))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_ਬਹੁਲਕ, 0, ਆਕਾਰ(new_ਬਹੁਲਕ));
		ਜੇ (tcgetattr(STDIN_FILENO, &new_ਬਹੁਲਕ))
			perror_on_device_and_die("%s");

		ਜੇ (memcmp(&ਬਹੁਲਕ, &new_ਬਹੁਲਕ, ਆਕਾਰ(ਬਹੁਲਕ)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#ਜੇ 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_ਬਹੁਲਕ.c_cflag &= (~CIBAUD);
			ਜੇ ((stty_state & STTY_ਰਫਤਾਰ_was_ਸੈੱਟ)
			 || memcmp(&ਬਹੁਲਕ, &new_ਬਹੁਲਕ, ਆਕਾਰ(ਬਹੁਲਕ)) != 0)
#ਜੇ_ਅੰਤ
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	ਵਾਪਸ EXIT_SUCCESS;
}
