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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _POSIX_VDISABLE
# define _POSIX_VDISABLE ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) 0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CINTR
# define CINTR Control('c')
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CQUIT
# define CQUIT 28
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CERASE
# define CERASE 127
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CKILL
# define CKILL Control('u')
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CEOF
# define CEOF Control('d')
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CEOL
# define CEOL _POSIX_VDISABLE
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CSTART
# define CSTART Control('q')
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CSTOP
# define CSTOP Control('s')
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CSUSP
# define CSUSP Control('z')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#ಆದರೆ_ಅಂತ್ಯ
/* glibc-2.12.1 uses only VSWTC name */
#ಆದರೆ defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#ಆದರೆ_ಅಂತ್ಯ
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#ಆದರೆ defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#ಆದರೆ_ಅಂತ್ಯ

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#ಆದರೆ defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#ಆದರೆ_ಅಂತ್ಯ

/* Save us from #ifdef forest plague */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ BSDLY
# define BSDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CIBAUD
# define CIBAUD 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CRDLY
# define CRDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CMSPAR
# define CMSPAR 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CRTSCTS
# define CRTSCTS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ECHOCTL
# define ECHOCTL 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ECHOKE
# define ECHOKE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ECHOPRT
# define ECHOPRT 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FFDLY
# define FFDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ IEXTEN
# define IEXTEN 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ IMAXBEL
# define IMAXBEL 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ IUCLC
# define IUCLC 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ IXANY
# define IXANY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NLDLY
# define NLDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OCRNL
# define OCRNL 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OFDEL
# define OFDEL 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OFILL
# define OFILL 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OLCUC
# define OLCUC 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ONLCR
# define ONLCR 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ONLRET
# define ONLRET 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ONOCR
# define ONOCR 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OXTABS
# define OXTABS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TABDLY
# define TABDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TAB1
# define TAB1 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TAB2
# define TAB2 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TOSTOP
# define TOSTOP 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VDSUSP
# define VDSUSP 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VEOL2
# define VEOL2 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VFLUSHO
# define VFLUSHO 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VLNEXT
# define VLNEXT 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VREPRINT
# define VREPRINT 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VSTATUS
# define VSTATUS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VSWTCH
# define VSWTCH 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VTDLY
# define VTDLY 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VWERASE
# define VWERASE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ XCASE
# define XCASE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ IUTF8
# define IUTF8 0
#ಆದರೆ_ಅಂತ್ಯ

/* Which speeds to set */
ಎಣಿಕೆ ವೇಗ_setting {
	ಒಳಹರಿವು_ವೇಗ, output_ವೇಗ, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
ಎಣಿಕೆ {
	control, ಒಳಹರಿವು, output, local, combination
};
ಸ್ಥಿರ tcflag_t *get_ptr_ಗಮ್ಯ_tcflag(ಚಿಹ್ನೆರಹಿತ type, ಸ್ಥಿರಾಂಕ ರಚನೆ termios *ಬಹುಲಕ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(ರಚನೆ termios, c_cflag), /* control */
		offsetof(ರಚನೆ termios, c_iflag), /*ಒಳಹರಿವು*/
		offsetof(ರಚನೆ termios, c_oflag), /* output */
		offsetof(ರಚನೆ termios, c_lflag)  /* local */
	};
	ಆದರೆ (type <= local) {
		ಹಿಂತಿರುಗಿ (tcflag_t*) (((ಅಕ್ಷರ*)ಬಹುಲಕ) + tcflag_offsets[type]);
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Flags for 'struct mode_info' */
#ವ್ಯಾಖ್ಯಾನಿಸು SANE_ಗಣ 1              /* Set in 'sane' mode                  */
#ವ್ಯಾಖ್ಯಾನಿಸು SANE_UNSET 2            /* Unset in 'sane' mode                */
#ವ್ಯಾಖ್ಯಾನಿಸು REV 4                   /* Can be turned off by prepending '-' */
#ವ್ಯಾಖ್ಯಾನಿಸು OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
ರಚನೆ ಬಹುಲಕ_info {
	ಸ್ಥಿರಾಂಕ uint8_t type;           /* Which structure element to change    */
	ಸ್ಥಿರಾಂಕ uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#ಆದರೆ   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ಸ್ಥಿರಾಂಕ uint8_t ಮುಖವಾಡ;
#ಅಥವಾಆದರೆ (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ಸ್ಥಿರಾಂಕ uint16_t ಮುಖವಾಡ;
#ಇಲ್ಲದಿದ್ದರೆ
	ಸ್ಥಿರಾಂಕ tcflag_t ಮುಖವಾಡ;          /* Other bits to turn off for this mode */
#ಆದರೆ_ಅಂತ್ಯ
	/* was using short here, but ppc32 was unhappy */
	ಸ್ಥಿರಾಂಕ tcflag_t ಬಿಟ್ಟುಗಳು;          /* Bits to set for this mode            */
};

ಎಣಿಕೆ {
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
#ಆದರೆ IXANY
	IDX_decctlq,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TABDLY || OXTABS
	IDX_tabs,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿಸು MI_ನಮೂದು(N,T,F,B,M) N "\0"

/* Mode names given on command line */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ಬಹುಲಕ_name[] ALIGN1 =
	MI_ನಮೂದು("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("ek",       combination, OMIT,              0,          0 )
	MI_ನಮೂದು("sane",     combination, OMIT,              0,          0 )
	MI_ನಮೂದು("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("crt",      combination, OMIT,              0,          0 )
	MI_ನಮೂದು("dec",      combination, OMIT,              0,          0 )
#ಆದರೆ IXANY
	MI_ನಮೂದು("decctlq",  combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TABDLY || OXTABS
	MI_ನಮೂದು("tabs",     combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ XCASE && IUCLC && OLCUC
	MI_ನಮೂದು("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("LCASE",    combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("parenb",   control,     REV,               PARENB,     0 )
	MI_ನಮೂದು("parodd",   control,     REV,               PARODD,     0 )
#ಆದರೆ CMSPAR
	MI_ನಮೂದು("cmspar",   control,     REV,               CMSPAR,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ನಮೂದು("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ನಮೂದು("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ನಮೂದು("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ನಮೂದು("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ನಮೂದು("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ನಮೂದು("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ನಮೂದು("cread",    control,     SANE_ಗಣ   | REV,  CREAD,      0 )
	MI_ನಮೂದು("clocal",   control,     REV,               CLOCAL,     0 )
#ಆದರೆ CRTSCTS
	MI_ನಮೂದು("crtscts",  control,     REV,               CRTSCTS,    0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("ignbrk",   ಒಳಹರಿವು,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ನಮೂದು("brkint",   ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  BRKINT,     0 )
	MI_ನಮೂದು("ignpar",   ಒಳಹರಿವು,       REV,               IGNPAR,     0 )
	MI_ನಮೂದು("parmrk",   ಒಳಹರಿವು,       REV,               PARMRK,     0 )
	MI_ನಮೂದು("inpck",    ಒಳಹರಿವು,       REV,               INPCK,      0 )
	MI_ನಮೂದು("istrip",   ಒಳಹರಿವು,       REV,               ISTRIP,     0 )
	MI_ನಮೂದು("inlcr",    ಒಳಹರಿವು,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ನಮೂದು("igncr",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ನಮೂದು("icrnl",    ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  ICRNL,      0 )
	MI_ನಮೂದು("ixon",     ಒಳಹರಿವು,       REV,               IXON,       0 )
	MI_ನಮೂದು("ixoff",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ನಮೂದು("tandem",   ಒಳಹರಿವು,       OMIT       | REV,  IXOFF,      0 )
#ಆದರೆ IUCLC
	MI_ನಮೂದು("iuclc",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IUCLC,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IXANY
	MI_ನಮೂದು("ixany",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IXANY,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IMAXBEL
	MI_ನಮೂದು("imaxbel",  ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  IMAXBEL,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IUTF8
	MI_ನಮೂದು("iutf8",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IUTF8,      0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("opost",    output,      SANE_ಗಣ   | REV,  OPOST,      0 )
#ಆದರೆ OLCUC
	MI_ನಮೂದು("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OCRNL
	MI_ನಮೂದು("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONLCR
	MI_ನಮೂದು("onlcr",    output,      SANE_ಗಣ   | REV,  ONLCR,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONOCR
	MI_ನಮೂದು("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONLRET
	MI_ನಮೂದು("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OFILL
	MI_ನಮೂದು("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OFDEL
	MI_ನಮೂದು("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ NLDLY
	MI_ನಮೂದು("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ನಮೂದು("nl0",      output,      SANE_ಗಣ,          NL0,     NLDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CRDLY
	MI_ನಮೂದು("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ನಮೂದು("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ನಮೂದು("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ನಮೂದು("cr0",      output,      SANE_ಗಣ,          CR0,     CRDLY)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ TABDLY
	MI_ನಮೂದು("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ಆದರೆ TAB2
	MI_ನಮೂದು("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ಆದರೆ TAB1
	MI_ನಮೂದು("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ನಮೂದು("tab0",     output,      SANE_ಗಣ,          TAB0,   TABDLY)
#ಇಲ್ಲದಿದ್ದರೆ
# ಆದರೆ OXTABS
	MI_ನಮೂದು("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ BSDLY
	MI_ನಮೂದು("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ನಮೂದು("bs0",      output,      SANE_ಗಣ,          BS0,     BSDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VTDLY
	MI_ನಮೂದು("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ನಮೂದು("vt0",      output,      SANE_ಗಣ,          VT0,     VTDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ FFDLY
	MI_ನಮೂದು("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ನಮೂದು("ff0",      output,      SANE_ಗಣ,          FF0,     FFDLY)
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("isig",     local,       SANE_ಗಣ   | REV,  ISIG,       0 )
	MI_ನಮೂದು("icanon",   local,       SANE_ಗಣ   | REV,  ICANON,     0 )
#ಆದರೆ IEXTEN
	MI_ನಮೂದು("iexten",   local,       SANE_ಗಣ   | REV,  IEXTEN,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("echo",     local,       SANE_ಗಣ   | REV,  ECHO,       0 )
	MI_ನಮೂದು("echoe",    local,       SANE_ಗಣ   | REV,  ECHOE,      0 )
	MI_ನಮೂದು("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ನಮೂದು("echok",    local,       SANE_ಗಣ   | REV,  ECHOK,      0 )
	MI_ನಮೂದು("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ನಮೂದು("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ಆದರೆ XCASE
	MI_ನಮೂದು("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TOSTOP
	MI_ನಮೂದು("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOPRT
	MI_ನಮೂದು("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ನಮೂದು("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOCTL
	MI_ನಮೂದು("echoctl",  local,       SANE_ಗಣ   | REV,  ECHOCTL,    0 )
	MI_ನಮೂದು("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOKE
	MI_ನಮೂದು("echoke",   local,       SANE_ಗಣ   | REV,  ECHOKE,     0 )
	MI_ನಮೂದು("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EXTPROC
	MI_ನಮೂದು("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#ಆದರೆ_ಅಂತ್ಯ
	;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MI_ನಮೂದು
#ವ್ಯಾಖ್ಯಾನಿಸು MI_ನಮೂದು(N,T,F,B,M) { T, F, M, B },

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಹುಲಕ_info ಬಹುಲಕ_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_ನಮೂದು("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("parity",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("nl",       combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("ek",       combination, OMIT,              0,          0 )
	MI_ನಮೂದು("sane",     combination, OMIT,              0,          0 )
	MI_ನಮೂದು("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("raw",      combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("litout",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("crt",      combination, OMIT,              0,          0 )
	MI_ನಮೂದು("dec",      combination, OMIT,              0,          0 )
#ಆದರೆ IXANY
	MI_ನಮೂದು("decctlq",  combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TABDLY || OXTABS
	MI_ನಮೂದು("tabs",     combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ XCASE && IUCLC && OLCUC
	MI_ನಮೂದು("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ನಮೂದು("LCASE",    combination, REV        | OMIT, 0,          0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("parenb",   control,     REV,               PARENB,     0 )
	MI_ನಮೂದು("parodd",   control,     REV,               PARODD,     0 )
#ಆದರೆ CMSPAR
	MI_ನಮೂದು("cmspar",   control,     REV,               CMSPAR,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ನಮೂದು("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ನಮೂದು("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ನಮೂದು("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ನಮೂದು("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ನಮೂದು("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ನಮೂದು("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ನಮೂದು("cread",    control,     SANE_ಗಣ   | REV,  CREAD,      0 )
	MI_ನಮೂದು("clocal",   control,     REV,               CLOCAL,     0 )
#ಆದರೆ CRTSCTS
	MI_ನಮೂದು("crtscts",  control,     REV,               CRTSCTS,    0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("ignbrk",   ಒಳಹರಿವು,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ನಮೂದು("brkint",   ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  BRKINT,     0 )
	MI_ನಮೂದು("ignpar",   ಒಳಹರಿವು,       REV,               IGNPAR,     0 )
	MI_ನಮೂದು("parmrk",   ಒಳಹರಿವು,       REV,               PARMRK,     0 )
	MI_ನಮೂದು("inpck",    ಒಳಹರಿವು,       REV,               INPCK,      0 )
	MI_ನಮೂದು("istrip",   ಒಳಹರಿವು,       REV,               ISTRIP,     0 )
	MI_ನಮೂದು("inlcr",    ಒಳಹರಿವು,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ನಮೂದು("igncr",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ನಮೂದು("icrnl",    ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  ICRNL,      0 )
	MI_ನಮೂದು("ixon",     ಒಳಹರಿವು,       REV,               IXON,       0 )
	MI_ನಮೂದು("ixoff",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ನಮೂದು("tandem",   ಒಳಹರಿವು,       OMIT       | REV,  IXOFF,      0 )
#ಆದರೆ IUCLC
	MI_ನಮೂದು("iuclc",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IUCLC,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IXANY
	MI_ನಮೂದು("ixany",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IXANY,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IMAXBEL
	MI_ನಮೂದು("imaxbel",  ಒಳಹರಿವು,       SANE_ಗಣ   | REV,  IMAXBEL,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ IUTF8
	MI_ನಮೂದು("iutf8",    ಒಳಹರಿವು,       SANE_UNSET | REV,  IUTF8,      0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("opost",    output,      SANE_ಗಣ   | REV,  OPOST,      0 )
#ಆದರೆ OLCUC
	MI_ನಮೂದು("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OCRNL
	MI_ನಮೂದು("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONLCR
	MI_ನಮೂದು("onlcr",    output,      SANE_ಗಣ   | REV,  ONLCR,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONOCR
	MI_ನಮೂದು("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ONLRET
	MI_ನಮೂದು("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OFILL
	MI_ನಮೂದು("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OFDEL
	MI_ನಮೂದು("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ NLDLY
	MI_ನಮೂದು("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ನಮೂದು("nl0",      output,      SANE_ಗಣ,          NL0,     NLDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CRDLY
	MI_ನಮೂದು("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ನಮೂದು("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ನಮೂದು("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ನಮೂದು("cr0",      output,      SANE_ಗಣ,          CR0,     CRDLY)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ TABDLY
	MI_ನಮೂದು("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ಆದರೆ TAB2
	MI_ನಮೂದು("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ಆದರೆ TAB1
	MI_ನಮೂದು("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ನಮೂದು("tab0",     output,      SANE_ಗಣ,          TAB0,   TABDLY)
#ಇಲ್ಲದಿದ್ದರೆ
# ಆದರೆ OXTABS
	MI_ನಮೂದು("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ BSDLY
	MI_ನಮೂದು("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ನಮೂದು("bs0",      output,      SANE_ಗಣ,          BS0,     BSDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VTDLY
	MI_ನಮೂದು("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ನಮೂದು("vt0",      output,      SANE_ಗಣ,          VT0,     VTDLY)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ FFDLY
	MI_ನಮೂದು("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ನಮೂದು("ff0",      output,      SANE_ಗಣ,          FF0,     FFDLY)
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("isig",     local,       SANE_ಗಣ   | REV,  ISIG,       0 )
	MI_ನಮೂದು("icanon",   local,       SANE_ಗಣ   | REV,  ICANON,     0 )
#ಆದರೆ IEXTEN
	MI_ನಮೂದು("iexten",   local,       SANE_ಗಣ   | REV,  IEXTEN,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("echo",     local,       SANE_ಗಣ   | REV,  ECHO,       0 )
	MI_ನಮೂದು("echoe",    local,       SANE_ಗಣ   | REV,  ECHOE,      0 )
	MI_ನಮೂದು("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ನಮೂದು("echok",    local,       SANE_ಗಣ   | REV,  ECHOK,      0 )
	MI_ನಮೂದು("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ನಮೂದು("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ಆದರೆ XCASE
	MI_ನಮೂದು("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TOSTOP
	MI_ನಮೂದು("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOPRT
	MI_ನಮೂದು("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ನಮೂದು("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOCTL
	MI_ನಮೂದು("echoctl",  local,       SANE_ಗಣ   | REV,  ECHOCTL,    0 )
	MI_ನಮೂದು("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ECHOKE
	MI_ನಮೂದು("echoke",   local,       SANE_ಗಣ   | REV,  ECHOKE,     0 )
	MI_ನಮೂದು("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#ಆದರೆ_ಅಂತ್ಯ
	MI_ನಮೂದು("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EXTPROC
	MI_ನಮೂದು("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#ಆದರೆ_ಅಂತ್ಯ
};

ಎಣಿಕೆ {
	ಅಂಕೆ_ಬಹುಲಕ_info = ARRAY_ಗಾತ್ರ(ಬಹುಲಕ_info)
};


/* Control characters */
ರಚನೆ control_info {
	ಸ್ಥಿರಾಂಕ uint8_t saneval;  /* Value to set for 'stty sane' */
	ಸ್ಥಿರಾಂಕ uint8_t ಸ್ಥಾನಾಂತರ;   /* Offset in c_cc */
};

ಎಣಿಕೆ {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#ಆದರೆ VEOL2
	CIDX_eol2,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSWTCH
	CIDX_swtch,
#ಆದರೆ_ಅಂತ್ಯ
	CIDX_ಆರಂಭ,
	CIDX_stop,
	CIDX_susp,
#ಆದರೆ VDSUSP
	CIDX_dsusp,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VREPRINT
	CIDX_rprnt,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VWERASE
	CIDX_werase,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VLNEXT
	CIDX_lnext,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VFLUSHO
	CIDX_flush,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSTATUS
	CIDX_ಸ್ಥಿತಿ,
#ಆದರೆ_ಅಂತ್ಯ
	CIDX_min,
	CIDX_ಸಮಯ,
};

#ವ್ಯಾಖ್ಯಾನಿಸು CI_ನಮೂದು(n,s,o) n "\0"

/* Name given on command line */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ control_name[] ALIGN1 =
	CI_ನಮೂದು("intr",     CINTR,   VINTR   )
	CI_ನಮೂದು("quit",     CQUIT,   VQUIT   )
	CI_ನಮೂದು("erase",    CERASE,  VERASE  )
	CI_ನಮೂದು("kill",     CKILL,   VKILL   )
	CI_ನಮೂದು("eof",      CEOF,    VEOF    )
	CI_ನಮೂದು("eol",      CEOL,    VEOL    )
#ಆದರೆ VEOL2
	CI_ನಮೂದು("eol2",     CEOL2,   VEOL2   )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSWTCH
	CI_ನಮೂದು("swtch",    CSWTCH,  VSWTCH  )
#ಆದರೆ_ಅಂತ್ಯ
	CI_ನಮೂದು("start",    CSTART,  VSTART  )
	CI_ನಮೂದು("stop",     CSTOP,   VSTOP   )
	CI_ನಮೂದು("susp",     CSUSP,   VSUSP   )
#ಆದರೆ VDSUSP
	CI_ನಮೂದು("dsusp",    CDSUSP,  VDSUSP  )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VREPRINT
	CI_ನಮೂದು("rprnt",    CRPRNT,  VREPRINT)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VWERASE
	CI_ನಮೂದು("werase",   CWERASE, VWERASE )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VLNEXT
	CI_ನಮೂದು("lnext",    CLNEXT,  VLNEXT  )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VFLUSHO
	CI_ನಮೂದು("flush",    CFLUSHO, VFLUSHO )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSTATUS
	CI_ನಮೂದು("status",   CSTATUS, VSTATUS )
#ಆದರೆ_ಅಂತ್ಯ
	/* These must be last because of the display routines */
	CI_ನಮೂದು("min",      1,       VMIN    )
	CI_ನಮೂದು("time",     0,       VTIME   )
	;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು CI_ನಮೂದು
#ವ್ಯಾಖ್ಯಾನಿಸು CI_ನಮೂದು(n,s,o) { s, o },

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ನಮೂದು("intr",     CINTR,   VINTR   )
	CI_ನಮೂದು("quit",     CQUIT,   VQUIT   )
	CI_ನಮೂದು("erase",    CERASE,  VERASE  )
	CI_ನಮೂದು("kill",     CKILL,   VKILL   )
	CI_ನಮೂದು("eof",      CEOF,    VEOF    )
	CI_ನಮೂದು("eol",      CEOL,    VEOL    )
#ಆದರೆ VEOL2
	CI_ನಮೂದು("eol2",     CEOL2,   VEOL2   )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSWTCH
	CI_ನಮೂದು("swtch",    CSWTCH,  VSWTCH  )
#ಆದರೆ_ಅಂತ್ಯ
	CI_ನಮೂದು("start",    CSTART,  VSTART  )
	CI_ನಮೂದು("stop",     CSTOP,   VSTOP   )
	CI_ನಮೂದು("susp",     CSUSP,   VSUSP   )
#ಆದರೆ VDSUSP
	CI_ನಮೂದು("dsusp",    CDSUSP,  VDSUSP  )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VREPRINT
	CI_ನಮೂದು("rprnt",    CRPRNT,  VREPRINT)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VWERASE
	CI_ನಮೂದು("werase",   CWERASE, VWERASE )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VLNEXT
	CI_ನಮೂದು("lnext",    CLNEXT,  VLNEXT  )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VFLUSHO
	CI_ನಮೂದು("flush",    CFLUSHO, VFLUSHO )
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VSTATUS
	CI_ನಮೂದು("status",   CSTATUS, VSTATUS )
#ಆದರೆ_ಅಂತ್ಯ
	/* These must be last because of the display routines */
	CI_ನಮೂದು("min",      1,       VMIN    )
	CI_ನಮೂದು("time",     0,       VTIME   )
};

ಎಣಿಕೆ {
	ಅಂಕೆ_control_info = ARRAY_ಗಾತ್ರ(control_info)
};


ರಚನೆ globals {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device_name;
	/* The width of the screen, for output wrapping */
	ಚಿಹ್ನೆರಹಿತ max_ಲಂಬಸಾಲು;
	/* Current position, to know when to wrap */
	ಚಿಹ್ನೆರಹಿತ current_ಲಂಬಸಾಲು;
} FIX_ALIASING;
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	G.device_name = bb_ಸಂದೇಶ_standard_ಒಳಹರಿವು; \
	G.max_ಲಂಬಸಾಲು = 80; \
	G.current_ಲಂಬಸಾಲು = 0; /* we are noexec, must clear */ \
} ಆಗ (0)

ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ವೇಗ_or_die(ಎಣಿಕೆ ವೇಗ_setting type, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg,
					ರಚನೆ termios *ಬಹುಲಕ)
{
	ವೇಗ_t baud;

	baud = tty_ಮೌಲ್ಯ_ಗಮ್ಯ_baud(xatou(arg));

	ಆದರೆ (type != output_ವೇಗ) {     /* either input or both */
		cfsetispeed(ಬಹುಲಕ, baud);
	}
	ಆದರೆ (type != ಒಳಹರಿವು_ವೇಗ) {      /* either output or both */
		cfsetospeed(ಬಹುಲಕ, baud);
	}
}

ಸ್ಥಿರ NORETURN ಶೂನ್ಯ perror_on_device_and_die(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt)
{
	bb_perror_ಸಂದೇಶ_and_die(fmt, G.device_name);
}

ಸ್ಥಿರ ಶೂನ್ಯ perror_on_device(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt)
{
	bb_perror_ಸಂದೇಶ(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
ಸ್ಥಿರ ಶೂನ್ಯ wrapf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ...)
{
	ಅಕ್ಷರ buf[128];
	va_list ವಾದಗಳು;
	ಚಿಹ್ನೆರಹಿತ buflen;

	va_ಆರಂಭ(ವಾದಗಳು, ಸಂದೇಶ_2);
	buflen = vsnprintf(buf, ಗಾತ್ರ(buf), ಸಂದೇಶ_2, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	ಆದರೆ (!buflen || buflen >= ಗಾತ್ರ(buf)) ಹಿಂತಿರುಗಿ;

	ಆದರೆ (G.current_ಲಂಬಸಾಲು > 0) {
		G.current_ಲಂಬಸಾಲು++;
		ಆದರೆ (buf[0] != '\n') {
			ಆದರೆ (G.current_ಲಂಬಸಾಲು + buflen >= G.max_ಲಂಬಸಾಲು) {
				G.current_ಲಂಬಸಾಲು = 0;
				bb_putchar('\n');
			} ಇಲ್ಲದಿದ್ದರೆ {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_ಲಂಬಸಾಲು += buflen;
	ಆದರೆ (buf[buflen-1] == '\n')
		G.current_ಲಂಬಸಾಲು = 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ newline(ಶೂನ್ಯ)
{
	ಆದರೆ (G.current_ಲಂಬಸಾಲು != 0)
		wrapf("\n");
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCGWINSZ
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ಕಿಟಕಿ_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು, ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ)
{
	ರಚನೆ winsize win = { 0, 0, 0, 0 };

	ಆದರೆ (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		ಆದರೆ (errno != EINVAL) {
			ಹೋಗು bail;
		}
		memset(&win, 0, ಗಾತ್ರ(win));
	}

	ಆದರೆ (ಅಡ್ಡಸಾಲುಗಳು >= 0)
		win.ws_ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲುಗಳು;
	ಆದರೆ (ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ >= 0)
		win.ws_ಲಂಬಸಾಲು = ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;

	ಆದರೆ (ioctl(STDIN_FILENO, TIOCSWINSZ, (ಅಕ್ಷರ *) &win))
bail:
		perror_on_device("%s");
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ display_ಕಿಟಕಿ_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fancy)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_str = "%s\0%s: no size information for this device";
	ಚಿಹ್ನೆರಹಿತ ಅಗಲ, ಎತ್ತರ;

	ಆದರೆ (get_terminal_ಅಗಲ_ಎತ್ತರ(STDIN_FILENO, &ಅಗಲ, &ಎತ್ತರ)) {
		ಆದರೆ ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				ಎತ್ತರ, ಅಗಲ);
	}
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಹುಲಕ_info *ಕಂಡುಹಿಡಿ_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ i = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(ಬಹುಲಕ_name, name);
	ಹಿಂತಿರುಗಿ i >= 0 ? &ಬಹುಲಕ_info[i] : ಖಾಲಿ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ control_info *ಕಂಡುಹಿಡಿ_control(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ i = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(control_name, name);
	ಹಿಂತಿರುಗಿ i >= 0 ? &control_info[i] : ಖಾಲಿ;
}

ಎಣಿಕೆ {
	param_need_arg = 0x80,
	param_ಸಾಲು    = 1 | 0x80,
	param_ಅಡ್ಡಸಾಲುಗಳು    = 2 | 0x80,
	param_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_ಗಾತ್ರ    = 5,
	param_ವೇಗ   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_param(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	ಪೂರ್ಣಾಂಕ i = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(params, name) + 1;
	ಆದರೆ (i == 0)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (i != 5 && i != 6)
		i |= 0x80;
	ಹಿಂತಿರುಗಿ i;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ recover_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ರಚನೆ termios *ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ i, n;
	ಚಿಹ್ನೆರಹಿತ chr;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		ಹಿಂತಿರುಗಿ 0;
	ಬಹುಲಕ->c_iflag = iflag;
	ಬಹುಲಕ->c_oflag = oflag;
	ಬಹುಲಕ->c_cflag = cflag;
	ಬಹುಲಕ->c_lflag = lflag;
	arg += n;
	ಗಾಗಿ (i = 0; i < NCCS; ++i) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(arg, ":%x%n", &chr, &n) != 1)
			ಹಿಂತಿರುಗಿ 0;
		ಬಹುಲಕ->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	ಆದರೆ (*arg != '\0')
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ display_recoverable(ಸ್ಥಿರಾಂಕ ರಚನೆ termios *ಬಹುಲಕ,
				ಪೂರ್ಣಾಂಕ ಬಳಸದ_PARAM dummy)
{
	ಪೂರ್ಣಾಂಕ i;
	ಬರೆ("%lx:%lx:%lx:%lx",
		   (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಬಹುಲಕ->c_iflag, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಬಹುಲಕ->c_oflag,
		   (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಬಹುಲಕ->c_cflag, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಬಹುಲಕ->c_lflag);
	ಗಾಗಿ (i = 0; i < NCCS; ++i)
		ಬರೆ(":%x", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) ಬಹುಲಕ->c_cc[i]);
	bb_putchar('\n');
}

ಸ್ಥಿರ ಶೂನ್ಯ display_ವೇಗ(ಸ್ಥಿರಾಂಕ ರಚನೆ termios *ಬಹುಲಕ, ಪೂರ್ಣಾಂಕ fancy)
{
	//____________________ 01234567 8 9
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ispeed, ospeed;

	ispeed = cfgetispeed(ಬಹುಲಕ);
	ospeed = cfgetospeed(ಬಹುಲಕ);
	ಆದರೆ (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	ಆದರೆ (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_ಗಮ್ಯ_ಮೌಲ್ಯ(ispeed), tty_baud_ಗಮ್ಯ_ಮೌಲ್ಯ(ospeed));
}

ಸ್ಥಿರ ಶೂನ್ಯ do_display(ಸ್ಥಿರಾಂಕ ರಚನೆ termios *ಬಹುಲಕ, ಪೂರ್ಣಾಂಕ all)
{
	ಪೂರ್ಣಾಂಕ i;
	tcflag_t *bitsp;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಮುಖವಾಡ;
	ಪೂರ್ಣಾಂಕ prev_type = control;

	display_ವೇಗ(ಬಹುಲಕ, 1);
	ಆದರೆ (all)
		display_ಕಿಟಕಿ_ಗಾತ್ರ(1);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	wrapf("line = %u;\n", ಬಹುಲಕ->c_ಸಾಲು);
#ಇಲ್ಲದಿದ್ದರೆ
	newline();
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (i = 0; i != CIDX_min; ++i) {
		ಅಕ್ಷರ ch;
		ಅಕ್ಷರ buf10[10];

		/* If swtch is the same as susp, don't print both */
#ಆದರೆ VSWTCH == VSUSP
		ಆದರೆ (i == CIDX_swtch)
			ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
		/* If eof uses the same slot as min, only print whichever applies */
#ಆದರೆ VEOF == VMIN
		ಆದರೆ (!(ಬಹುಲಕ->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
		ch = ಬಹುಲಕ->c_cc[control_info[i].ಸ್ಥಾನಾಂತರ];
		ಆದರೆ (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		ಇಲ್ಲದಿದ್ದರೆ
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#ಆದರೆ VEOF == VMIN
	ಆದರೆ ((ಬಹುಲಕ->c_lflag & ICANON) == 0)
#ಆದರೆ_ಅಂತ್ಯ
		wrapf("min = %u; time = %u;", ಬಹುಲಕ->c_cc[VMIN], ಬಹುಲಕ->c_cc[VTIME]);
	newline();

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_ಬಹುಲಕ_info; ++i) {
		ಆದರೆ (ಬಹುಲಕ_info[i].flags & OMIT)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಬಹುಲಕ_info[i].type != prev_type) {
			newline();
			prev_type = ಬಹುಲಕ_info[i].type;
		}

		bitsp = get_ptr_ಗಮ್ಯ_tcflag(ಬಹುಲಕ_info[i].type, ಬಹುಲಕ);
		ಮುಖವಾಡ = ಬಹುಲಕ_info[i].ಮುಖವಾಡ ? ಬಹುಲಕ_info[i].ಮುಖವಾಡ : ಬಹುಲಕ_info[i].ಬಿಟ್ಟುಗಳು;
		ಆದರೆ ((*bitsp & ಮುಖವಾಡ) == ಬಹುಲಕ_info[i].ಬಿಟ್ಟುಗಳು) {
			ಆದರೆ (all || (ಬಹುಲಕ_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(ಬಹುಲಕ_name, i));
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ ((all && ಬಹುಲಕ_info[i].flags & REV)
			 || (!all && (ಬಹುಲಕ_info[i].flags & (SANE_ಗಣ | REV)) == (SANE_ಗಣ | REV))
			) {
				wrapf("-%s", nth_string(ಬಹುಲಕ_name, i));
			}
		}
	}
	newline();
}

ಸ್ಥಿರ ಶೂನ್ಯ sane_ಬಹುಲಕ(ರಚನೆ termios *ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_control_info; ++i) {
#ಆದರೆ VMIN == VEOF
		ಆದರೆ (i == CIDX_min)
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಬಹುಲಕ->c_cc[control_info[i].ಸ್ಥಾನಾಂತರ] = control_info[i].saneval;
	}

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_ಬಹುಲಕ_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_ಗಮ್ಯ_tcflag(ಬಹುಲಕ_info[i].type, ಬಹುಲಕ);

		ಆದರೆ (!bitsp)
			ಮುಂದುವರಿ;
		val = *bitsp & ~((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಬಹುಲಕ_info[i].ಮುಖವಾಡ);
		ಆದರೆ (ಬಹುಲಕ_info[i].flags & SANE_ಗಣ) {
			*bitsp = val | ಬಹುಲಕ_info[i].ಬಿಟ್ಟುಗಳು;
		} ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (ಬಹುಲಕ_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~ಬಹುಲಕ_info[i].ಬಿಟ್ಟುಗಳು;
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಹುಲಕ_info *info, ಪೂರ್ಣಾಂಕ reversed,
					ರಚನೆ termios *ಬಹುಲಕ)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_ಗಮ್ಯ_tcflag(info->type, ಬಹುಲಕ);

	ಆದರೆ (bitsp) {
		tcflag_t val = *bitsp & ~info->ಮುಖವಾಡ;
		ಆದರೆ (reversed)
			*bitsp = val & ~info->ಬಿಟ್ಟುಗಳು;
		ಇಲ್ಲದಿದ್ದರೆ
			*bitsp = val | info->ಬಿಟ್ಟುಗಳು;
		ಹಿಂತಿರುಗಿ;
	}

	/* !bitsp - it's a "combination" mode */
	ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_evenp] || info == &ಬಹುಲಕ_info[IDX_parity]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_oddp]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_nl]) {
		ಆದರೆ (reversed) {
			ಬಹುಲಕ->c_iflag = (ಬಹುಲಕ->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			ಬಹುಲಕ->c_oflag = (ಬಹುಲಕ->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಹುಲಕ->c_iflag = ಬಹುಲಕ->c_iflag & ~ICRNL;
			ಆದರೆ (ONLCR) ಬಹುಲಕ->c_oflag = ಬಹುಲಕ->c_oflag & ~ONLCR;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_ek]) {
		ಬಹುಲಕ->c_cc[VERASE] = CERASE;
		ಬಹುಲಕ->c_cc[VKILL] = CKILL;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_sane]) {
		sane_ಬಹುಲಕ(ಬಹುಲಕ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_cbreak]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_lflag |= ICANON;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_lflag &= ~ICANON;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_pass8]) {
		ಆದರೆ (reversed) {
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ಬಹುಲಕ->c_iflag |= ISTRIP;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ಬಹುಲಕ->c_iflag &= ~ISTRIP;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_litout]) {
		ಆದರೆ (reversed) {
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~CSIZE) | CS7 | PARENB;
			ಬಹುಲಕ->c_iflag |= ISTRIP;
			ಬಹುಲಕ->c_oflag |= OPOST;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಹುಲಕ->c_cflag = (ಬಹುಲಕ->c_cflag & ~PARENB & ~CSIZE) | CS8;
			ಬಹುಲಕ->c_iflag &= ~ISTRIP;
			ಬಹುಲಕ->c_oflag &= ~OPOST;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_raw] || info == &ಬಹುಲಕ_info[IDX_cooked]) {
		ಆದರೆ ((info == &ಬಹುಲಕ_info[IDX_raw] && reversed)
		 || (info == &ಬಹುಲಕ_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			ಬಹುಲಕ->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			ಬಹುಲಕ->c_oflag |= OPOST;
			ಬಹುಲಕ->c_lflag |= ISIG | ICANON;
#ಆದರೆ VMIN == VEOF
			ಬಹುಲಕ->c_cc[VEOF] = CEOF;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ VTIME == VEOL
			ಬಹುಲಕ->c_cc[VEOL] = CEOL;
#ಆದರೆ_ಅಂತ್ಯ
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Raw mode */
			ಬಹುಲಕ->c_iflag = 0;
			ಬಹುಲಕ->c_oflag &= ~OPOST;
			ಬಹುಲಕ->c_lflag &= ~(ISIG | ICANON | XCASE);
			ಬಹುಲಕ->c_cc[VMIN] = 1;
			ಬಹುಲಕ->c_cc[VTIME] = 0;
		}
	}
#ಆದರೆ IXANY
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_decctlq]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_iflag |= IXANY;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_iflag &= ~IXANY;
	}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TABDLY
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_tabs]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_oflag = (ಬಹುಲಕ->c_oflag & ~TABDLY) | TAB3;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_oflag = (ಬಹುಲಕ->c_oflag & ~TABDLY) | TAB0;
	}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ OXTABS
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_tabs]) {
		ಆದರೆ (reversed)
			ಬಹುಲಕ->c_oflag |= OXTABS;
		ಇಲ್ಲದಿದ್ದರೆ
			ಬಹುಲಕ->c_oflag &= ~OXTABS;
	}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ XCASE && IUCLC && OLCUC
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info==&ಬಹುಲಕ_info[IDX_lcase] || info==&ಬಹುಲಕ_info[IDX_LCASE]) {
		ಆದರೆ (reversed) {
			ಬಹುಲಕ->c_lflag &= ~XCASE;
			ಬಹುಲಕ->c_iflag &= ~IUCLC;
			ಬಹುಲಕ->c_oflag &= ~OLCUC;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಹುಲಕ->c_lflag |= XCASE;
			ಬಹುಲಕ->c_iflag |= IUCLC;
			ಬಹುಲಕ->c_oflag |= OLCUC;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_crt]) {
		ಬಹುಲಕ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (info == &ಬಹುಲಕ_info[IDX_dec]) {
		ಬಹುಲಕ->c_cc[VINTR] = 3; /* ^C */
		ಬಹುಲಕ->c_cc[VERASE] = 127; /* DEL */
		ಬಹುಲಕ->c_cc[VKILL] = 21; /* ^U */
		ಬಹುಲಕ->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		ಆದರೆ (IXANY) ಬಹುಲಕ->c_iflag &= ~IXANY;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_control_char_or_die(ಸ್ಥಿರಾಂಕ ರಚನೆ control_info *info,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ರಚನೆ termios *ಬಹುಲಕ)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಮೌಲ್ಯ_2;

	ಆದರೆ (info == &control_info[CIDX_min] || info == &control_info[CIDX_ಸಮಯ])
		ಮೌಲ್ಯ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg[0] == '\0' || arg[1] == '\0')
		ಮೌಲ್ಯ_2 = arg[0];
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		ಮೌಲ್ಯ_2 = _POSIX_VDISABLE;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		ಮೌಲ್ಯ_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		ಆದರೆ (arg[1] == '?')
			ಮೌಲ್ಯ_2 = 127;
	} ಇಲ್ಲದಿದ್ದರೆ
		ಮೌಲ್ಯ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	ಬಹುಲಕ->c_cc[info->ಸ್ಥಾನಾಂತರ] = ಮೌಲ್ಯ_2;
}

#ವ್ಯಾಖ್ಯಾನಿಸು STTY_require_ಗಣ_attr   (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು STTY_ವೇಗ_was_ಗಣ      (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು STTY_verbose_output     (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು STTY_recoverable_output (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು STTY_noargs             (1 << 4)

ಪೂರ್ಣಾಂಕ stty_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ stty_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ರಚನೆ termios ಬಹುಲಕ;
	ಶೂನ್ಯ (*output_func)(ಸ್ಥಿರಾಂಕ ರಚನೆ termios *, ಪೂರ್ಣಾಂಕ);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file_name = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ display_all = 0;
	ಪೂರ್ಣಾಂಕ stty_state;
	ಪೂರ್ಣಾಂಕ k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	ಆಗ (argv[++k]) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಹುಲಕ_info *mp;
		ಸ್ಥಿರಾಂಕ ರಚನೆ control_info *cp;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg = argv[k];
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argnext = argv[k+1];
		ಪೂರ್ಣಾಂಕ param;

		ಆದರೆ (arg[0] == '-') {
			ಪೂರ್ಣಾಂಕ i;
			mp = ಕಂಡುಹಿಡಿ_ಬಹುಲಕ(arg+1);
			ಆದರೆ (mp) {
				ಆದರೆ (!(mp->flags & REV))
					ಹೋಗು invalid_ವಾದ;
				stty_state &= ~STTY_noargs;
				ಮುಂದುವರಿ;
			}
			/* It is an option - parse it */
			i = 0;
			ಆಗ (arg[++i]) {
				ಆಯ್ಕೆ (arg[i]) {
				ಸ್ಥಿತಿ 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ 'F':
					ಆದರೆ (file_name)
						bb_simple_error_ಸಂದೇಶ_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					ಆದರೆ (!file_name[0]) { /* nope, "-F device" */
						ಪೂರ್ಣಾಂಕ p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						ಆದರೆ (!file_name)
							bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						ಆಗ (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					ಹೋಗು ಕೊನೆ_option;
				ಪೂರ್ವನಿಯೋಜಿತ:
					ಹೋಗು invalid_ವಾದ;
				}
			}
 ಕೊನೆ_option:
			ಮುಂದುವರಿ;
		}

		mp = ಕಂಡುಹಿಡಿ_ಬಹುಲಕ(arg);
		ಆದರೆ (mp) {
			stty_state &= ~STTY_noargs;
			ಮುಂದುವರಿ;
		}

		cp = ಕಂಡುಹಿಡಿ_control(arg);
		ಆದರೆ (cp) {
			ಆದರೆ (!argnext)
				bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			ಗಣ_control_char_or_die(cp, argnext, &ಬಹುಲಕ);
			stty_state &= ~STTY_noargs;
			++k;
			ಮುಂದುವರಿ;
		}

		param = ಕಂಡುಹಿಡಿ_param(arg);
		ಆದರೆ (param & param_need_arg) {
			ಆದರೆ (!argnext)
				bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_requires_arg, arg);
			++k;
		}

		ಆಯ್ಕೆ (param) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		ಸ್ಥಿತಿ param_ಸಾಲು:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, stty_suffixes);
			ನಿಲ್ಲಿಸು;
# endif /* else fall-through */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCGWINSZ
		ಸ್ಥಿತಿ param_ಅಡ್ಡಸಾಲುಗಳು:
		ಸ್ಥಿತಿ param_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ:
		ಸ್ಥಿತಿ param_columns:
			xatoul_range_sfx(argnext, 1, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, stty_suffixes);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ಗಾತ್ರ:
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ param_ವೇಗ:
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ispeed:
			/* called for the side effect of xfunc death only */
			ಗಣ_ವೇಗ_or_die(ಒಳಹರಿವು_ವೇಗ, argnext, &ಬಹುಲಕ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ospeed:
			/* called for the side effect of xfunc death only */
			ಗಣ_ವೇಗ_or_die(output_ವೇಗ, argnext, &ಬಹುಲಕ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (recover_ಬಹುಲಕ(arg, &ಬಹುಲಕ) == 1) ನಿಲ್ಲಿಸು;
			ಆದರೆ (tty_ಮೌಲ್ಯ_ಗಮ್ಯ_baud(xatou(arg)) != (ವೇಗ_t) -1) ನಿಲ್ಲಿಸು;
 invalid_ವಾದ:
			bb_error_ಸಂದೇಶ_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	ಆದರೆ ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_ಸಂದೇಶ_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	ಆದರೆ ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_ಸಂದೇಶ_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	ಆದರೆ (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&ಬಹುಲಕ, 0, ಗಾತ್ರ(ಬಹುಲಕ));
	ಆದರೆ (tcgetattr(STDIN_FILENO, &ಬಹುಲಕ))
		perror_on_device_and_die("%s");

	ಆದರೆ (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_ಲಂಬಸಾಲು = get_terminal_ಅಗಲ(STDOUT_FILENO);
		output_func(&ಬಹುಲಕ, display_all);
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	ಆಗ (argv[++k]) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಹುಲಕ_info *mp;
		ಸ್ಥಿರಾಂಕ ರಚನೆ control_info *cp;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg = argv[k];
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argnext = argv[k+1];
		ಪೂರ್ಣಾಂಕ param;

		ಆದರೆ (arg[0] == '-') {
			mp = ಕಂಡುಹಿಡಿ_ಬಹುಲಕ(arg+1);
			ಆದರೆ (mp) {
				ಗಣ_ಬಹುಲಕ(mp, 1 /* reversed */, &ಬಹುಲಕ);
				stty_state |= STTY_require_ಗಣ_attr;
			}
			/* It is an option - already parsed. Skip it */
			ಮುಂದುವರಿ;
		}

		mp = ಕಂಡುಹಿಡಿ_ಬಹುಲಕ(arg);
		ಆದರೆ (mp) {
			ಗಣ_ಬಹುಲಕ(mp, 0 /* non-reversed */, &ಬಹುಲಕ);
			stty_state |= STTY_require_ಗಣ_attr;
			ಮುಂದುವರಿ;
		}

		cp = ಕಂಡುಹಿಡಿ_control(arg);
		ಆದರೆ (cp) {
			++k;
			ಗಣ_control_char_or_die(cp, argnext, &ಬಹುಲಕ);
			stty_state |= STTY_require_ಗಣ_attr;
			ಮುಂದುವರಿ;
		}

		param = ಕಂಡುಹಿಡಿ_param(arg);
		ಆದರೆ (param & param_need_arg) {
			++k;
		}

		ಆಯ್ಕೆ (param) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		ಸ್ಥಿತಿ param_ಸಾಲು:
			ಬಹುಲಕ.c_ಸಾಲು = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_ಗಣ_attr;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCGWINSZ
		ಸ್ಥಿತಿ param_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ:
		ಸ್ಥಿತಿ param_columns:
			ಗಣ_ಕಿಟಕಿ_ಗಾತ್ರ(-1, xatoul_sfx(argnext, stty_suffixes));
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ಗಾತ್ರ:
			display_ಕಿಟಕಿ_ಗಾತ್ರ(0);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ಅಡ್ಡಸಾಲುಗಳು:
			ಗಣ_ಕಿಟಕಿ_ಗಾತ್ರ(xatoul_sfx(argnext, stty_suffixes), -1);
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ param_ವೇಗ:
			display_ವೇಗ(&ಬಹುಲಕ, 0);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ispeed:
			ಗಣ_ವೇಗ_or_die(ಒಳಹರಿವು_ವೇಗ, argnext, &ಬಹುಲಕ);
			stty_state |= (STTY_require_ಗಣ_attr | STTY_ವೇಗ_was_ಗಣ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ param_ospeed:
			ಗಣ_ವೇಗ_or_die(output_ವೇಗ, argnext, &ಬಹುಲಕ);
			stty_state |= (STTY_require_ಗಣ_attr | STTY_ವೇಗ_was_ಗಣ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (recover_ಬಹುಲಕ(arg, &ಬಹುಲಕ) == 1)
				stty_state |= STTY_require_ಗಣ_attr;
			ಇಲ್ಲದಿದ್ದರೆ /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				ಗಣ_ವೇಗ_or_die(both_speeds, arg, &ಬಹುಲಕ);
				stty_state |= (STTY_require_ಗಣ_attr | STTY_ವೇಗ_was_ಗಣ);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	ಆದರೆ (stty_state & STTY_require_ಗಣ_attr) {
		ರಚನೆ termios new_ಬಹುಲಕ;

		ಆದರೆ (tcsetattr(STDIN_FILENO, TCSADRAIN, &ಬಹುಲಕ))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_ಬಹುಲಕ, 0, ಗಾತ್ರ(new_ಬಹುಲಕ));
		ಆದರೆ (tcgetattr(STDIN_FILENO, &new_ಬಹುಲಕ))
			perror_on_device_and_die("%s");

		ಆದರೆ (memcmp(&ಬಹುಲಕ, &new_ಬಹುಲಕ, ಗಾತ್ರ(ಬಹುಲಕ)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#ಆದರೆ 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_ಬಹುಲಕ.c_cflag &= (~CIBAUD);
			ಆದರೆ ((stty_state & STTY_ವೇಗ_was_ಗಣ)
			 || memcmp(&ಬಹುಲಕ, &new_ಬಹುಲಕ, ಗಾತ್ರ(ಬಹುಲಕ)) != 0)
#ಆದರೆ_ಅಂತ್ಯ
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
