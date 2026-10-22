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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

#నిర్వచితం_కాకపోతే _POSIX_VDISABLE
# define _POSIX_VDISABLE ((గుర్తులేని అక్షరం) 0)
#అయితే_ముగింపు

#నిర్వచించు Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#నిర్వచితం_కాకపోతే CINTR
# define CINTR Control('c')
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CQUIT
# define CQUIT 28
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CERASE
# define CERASE 127
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CKILL
# define CKILL Control('u')
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CEOF
# define CEOF Control('d')
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CEOL
# define CEOL _POSIX_VDISABLE
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CSTART
# define CSTART Control('q')
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CSTOP
# define CSTOP Control('s')
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CSUSP
# define CSUSP Control('z')
#అయితే_ముగింపు
#ఒకవేళ defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#అయితే_ముగింపు
/* glibc-2.12.1 uses only VSWTC name */
#ఒకవేళ defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#అయితే_ముగింపు
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#ఒకవేళ defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#అయితే_ముగింపు
#ఒకవేళ defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#అయితే_ముగింపు

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#ఒకవేళ defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#అయితే_ముగింపు

#ఒకవేళ defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#అయితే_ముగింపు
#ఒకవేళ defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#అయితే_ముగింపు
#ఒకవేళ !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#అయితే_ముగింపు
#ఒకవేళ defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#అయితే_ముగింపు
#ఒకవేళ defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#అయితే_ముగింపు
#ఒకవేళ defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#అయితే_ముగింపు
#ఒకవేళ defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#అయితే_ముగింపు
#ఒకవేళ defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#అయితే_ముగింపు
#ఒకవేళ defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#అయితే_ముగింపు
#ఒకవేళ defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#అయితే_ముగింపు
#ఒకవేళ defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#అయితే_ముగింపు
#ఒకవేళ defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#అయితే_ముగింపు
#ఒకవేళ defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#అయితే_ముగింపు

/* Save us from #ifdef forest plague */
#నిర్వచితం_కాకపోతే BSDLY
# define BSDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CIBAUD
# define CIBAUD 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CRDLY
# define CRDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CMSPAR
# define CMSPAR 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CRTSCTS
# define CRTSCTS 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ECHOCTL
# define ECHOCTL 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ECHOKE
# define ECHOKE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ECHOPRT
# define ECHOPRT 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే FFDLY
# define FFDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే IEXTEN
# define IEXTEN 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే IMAXBEL
# define IMAXBEL 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే IUCLC
# define IUCLC 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే IXANY
# define IXANY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే NLDLY
# define NLDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే OCRNL
# define OCRNL 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే OFDEL
# define OFDEL 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే OFILL
# define OFILL 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే OLCUC
# define OLCUC 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ONLCR
# define ONLCR 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ONLRET
# define ONLRET 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ONOCR
# define ONOCR 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే OXTABS
# define OXTABS 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే TABDLY
# define TABDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే TAB1
# define TAB1 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే TAB2
# define TAB2 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే TOSTOP
# define TOSTOP 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VDSUSP
# define VDSUSP 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VEOL2
# define VEOL2 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VFLUSHO
# define VFLUSHO 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VLNEXT
# define VLNEXT 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VREPRINT
# define VREPRINT 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VSTATUS
# define VSTATUS 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VSWTCH
# define VSWTCH 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VTDLY
# define VTDLY 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే VWERASE
# define VWERASE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే XCASE
# define XCASE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే IUTF8
# define IUTF8 0
#అయితే_ముగింపు

/* Which speeds to set */
గణన వేగం_setting {
	ఆదానం_వేగం, output_వేగం, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
గణన {
	control, ఆదానం, output, local, combination
};
స్థిర tcflag_t *get_ptr_గమ్యం_tcflag(గుర్తులేని type, స్థిరరాశి నిర్మాణం termios *బహుళకం)
{
	స్థిర స్థిరరాశి uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(నిర్మాణం termios, c_cflag), /* control */
		offsetof(నిర్మాణం termios, c_iflag), /*ఆదానం*/
		offsetof(నిర్మాణం termios, c_oflag), /* output */
		offsetof(నిర్మాణం termios, c_lflag)  /* local */
	};
	ఒకవేళ (type <= local) {
		తిరిగి (tcflag_t*) (((అక్షరం*)బహుళకం) + tcflag_offsets[type]);
	}
	తిరిగి ఖాళీ;
}

/* Flags for 'struct mode_info' */
#నిర్వచించు SANE_సమితి 1              /* Set in 'sane' mode                  */
#నిర్వచించు SANE_UNSET 2            /* Unset in 'sane' mode                */
#నిర్వచించు REV 4                   /* Can be turned off by prepending '-' */
#నిర్వచించు OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
నిర్మాణం బహుళకం_info {
	స్థిరరాశి uint8_t type;           /* Which structure element to change    */
	స్థిరరాశి uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#ఒకవేళ   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	స్థిరరాశి uint8_t ముసుగు;
#లేదావేళ (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	స్థిరరాశి uint16_t ముసుగు;
#లేకపోతే
	స్థిరరాశి tcflag_t ముసుగు;          /* Other bits to turn off for this mode */
#అయితే_ముగింపు
	/* was using short here, but ppc32 was unhappy */
	స్థిరరాశి tcflag_t బిట్లు;          /* Bits to set for this mode            */
};

గణన {
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
#ఒకవేళ IXANY
	IDX_decctlq,
#అయితే_ముగింపు
#ఒకవేళ TABDLY || OXTABS
	IDX_tabs,
#అయితే_ముగింపు
#ఒకవేళ XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#అయితే_ముగింపు
};

#నిర్వచించు MI_నమోదు(N,T,F,B,M) N "\0"

/* Mode names given on command line */
స్థిర స్థిరరాశి అక్షరం బహుళకం_name[] ALIGN1 =
	MI_నమోదు("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("parity",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("nl",       combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("ek",       combination, OMIT,              0,          0 )
	MI_నమోదు("sane",     combination, OMIT,              0,          0 )
	MI_నమోదు("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("raw",      combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("litout",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("crt",      combination, OMIT,              0,          0 )
	MI_నమోదు("dec",      combination, OMIT,              0,          0 )
#ఒకవేళ IXANY
	MI_నమోదు("decctlq",  combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
#ఒకవేళ TABDLY || OXTABS
	MI_నమోదు("tabs",     combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
#ఒకవేళ XCASE && IUCLC && OLCUC
	MI_నమోదు("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("LCASE",    combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
	MI_నమోదు("parenb",   control,     REV,               PARENB,     0 )
	MI_నమోదు("parodd",   control,     REV,               PARODD,     0 )
#ఒకవేళ CMSPAR
	MI_నమోదు("cmspar",   control,     REV,               CMSPAR,     0 )
#అయితే_ముగింపు
	MI_నమోదు("cs5",      control,     0,                 CS5,     CSIZE)
	MI_నమోదు("cs6",      control,     0,                 CS6,     CSIZE)
	MI_నమోదు("cs7",      control,     0,                 CS7,     CSIZE)
	MI_నమోదు("cs8",      control,     0,                 CS8,     CSIZE)
	MI_నమోదు("hupcl",    control,     REV,               HUPCL,      0 )
	MI_నమోదు("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_నమోదు("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_నమోదు("cread",    control,     SANE_సమితి   | REV,  CREAD,      0 )
	MI_నమోదు("clocal",   control,     REV,               CLOCAL,     0 )
#ఒకవేళ CRTSCTS
	MI_నమోదు("crtscts",  control,     REV,               CRTSCTS,    0 )
#అయితే_ముగింపు
	MI_నమోదు("ignbrk",   ఆదానం,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_నమోదు("brkint",   ఆదానం,       SANE_సమితి   | REV,  BRKINT,     0 )
	MI_నమోదు("ignpar",   ఆదానం,       REV,               IGNPAR,     0 )
	MI_నమోదు("parmrk",   ఆదానం,       REV,               PARMRK,     0 )
	MI_నమోదు("inpck",    ఆదానం,       REV,               INPCK,      0 )
	MI_నమోదు("istrip",   ఆదానం,       REV,               ISTRIP,     0 )
	MI_నమోదు("inlcr",    ఆదానం,       SANE_UNSET | REV,  INLCR,      0 )
	MI_నమోదు("igncr",    ఆదానం,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_నమోదు("icrnl",    ఆదానం,       SANE_సమితి   | REV,  ICRNL,      0 )
	MI_నమోదు("ixon",     ఆదానం,       REV,               IXON,       0 )
	MI_నమోదు("ixoff",    ఆదానం,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_నమోదు("tandem",   ఆదానం,       OMIT       | REV,  IXOFF,      0 )
#ఒకవేళ IUCLC
	MI_నమోదు("iuclc",    ఆదానం,       SANE_UNSET | REV,  IUCLC,      0 )
#అయితే_ముగింపు
#ఒకవేళ IXANY
	MI_నమోదు("ixany",    ఆదానం,       SANE_UNSET | REV,  IXANY,      0 )
#అయితే_ముగింపు
#ఒకవేళ IMAXBEL
	MI_నమోదు("imaxbel",  ఆదానం,       SANE_సమితి   | REV,  IMAXBEL,    0 )
#అయితే_ముగింపు
#ఒకవేళ IUTF8
	MI_నమోదు("iutf8",    ఆదానం,       SANE_UNSET | REV,  IUTF8,      0 )
#అయితే_ముగింపు
	MI_నమోదు("opost",    output,      SANE_సమితి   | REV,  OPOST,      0 )
#ఒకవేళ OLCUC
	MI_నమోదు("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#అయితే_ముగింపు
#ఒకవేళ OCRNL
	MI_నమోదు("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONLCR
	MI_నమోదు("onlcr",    output,      SANE_సమితి   | REV,  ONLCR,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONOCR
	MI_నమోదు("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONLRET
	MI_నమోదు("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#అయితే_ముగింపు
#ఒకవేళ OFILL
	MI_నమోదు("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#అయితే_ముగింపు
#ఒకవేళ OFDEL
	MI_నమోదు("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#అయితే_ముగింపు
#ఒకవేళ NLDLY
	MI_నమోదు("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_నమోదు("nl0",      output,      SANE_సమితి,          NL0,     NLDLY)
#అయితే_ముగింపు
#ఒకవేళ CRDLY
	MI_నమోదు("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_నమోదు("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_నమోదు("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_నమోదు("cr0",      output,      SANE_సమితి,          CR0,     CRDLY)
#అయితే_ముగింపు

#ఒకవేళ TABDLY
	MI_నమోదు("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ఒకవేళ TAB2
	MI_నమోదు("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ఒకవేళ TAB1
	MI_నమోదు("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_నమోదు("tab0",     output,      SANE_సమితి,          TAB0,   TABDLY)
#లేకపోతే
# ఒకవేళ OXTABS
	MI_నమోదు("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#అయితే_ముగింపు

#ఒకవేళ BSDLY
	MI_నమోదు("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_నమోదు("bs0",      output,      SANE_సమితి,          BS0,     BSDLY)
#అయితే_ముగింపు
#ఒకవేళ VTDLY
	MI_నమోదు("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_నమోదు("vt0",      output,      SANE_సమితి,          VT0,     VTDLY)
#అయితే_ముగింపు
#ఒకవేళ FFDLY
	MI_నమోదు("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_నమోదు("ff0",      output,      SANE_సమితి,          FF0,     FFDLY)
#అయితే_ముగింపు
	MI_నమోదు("isig",     local,       SANE_సమితి   | REV,  ISIG,       0 )
	MI_నమోదు("icanon",   local,       SANE_సమితి   | REV,  ICANON,     0 )
#ఒకవేళ IEXTEN
	MI_నమోదు("iexten",   local,       SANE_సమితి   | REV,  IEXTEN,     0 )
#అయితే_ముగింపు
	MI_నమోదు("echo",     local,       SANE_సమితి   | REV,  ECHO,       0 )
	MI_నమోదు("echoe",    local,       SANE_సమితి   | REV,  ECHOE,      0 )
	MI_నమోదు("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_నమోదు("echok",    local,       SANE_సమితి   | REV,  ECHOK,      0 )
	MI_నమోదు("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_నమోదు("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ఒకవేళ XCASE
	MI_నమోదు("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#అయితే_ముగింపు
#ఒకవేళ TOSTOP
	MI_నమోదు("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOPRT
	MI_నమోదు("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_నమోదు("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOCTL
	MI_నమోదు("echoctl",  local,       SANE_సమితి   | REV,  ECHOCTL,    0 )
	MI_నమోదు("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOKE
	MI_నమోదు("echoke",   local,       SANE_సమితి   | REV,  ECHOKE,     0 )
	MI_నమోదు("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#అయితే_ముగింపు
	MI_నమోదు("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#నిర్వచితమైతే EXTPROC
	MI_నమోదు("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#అయితే_ముగింపు
	;

#నిర్వచనం_రద్దు MI_నమోదు
#నిర్వచించు MI_నమోదు(N,T,F,B,M) { T, F, M, B },

స్థిర స్థిరరాశి నిర్మాణం బహుళకం_info బహుళకం_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_నమోదు("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("parity",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("nl",       combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("ek",       combination, OMIT,              0,          0 )
	MI_నమోదు("sane",     combination, OMIT,              0,          0 )
	MI_నమోదు("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("raw",      combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("litout",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("crt",      combination, OMIT,              0,          0 )
	MI_నమోదు("dec",      combination, OMIT,              0,          0 )
#ఒకవేళ IXANY
	MI_నమోదు("decctlq",  combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
#ఒకవేళ TABDLY || OXTABS
	MI_నమోదు("tabs",     combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
#ఒకవేళ XCASE && IUCLC && OLCUC
	MI_నమోదు("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_నమోదు("LCASE",    combination, REV        | OMIT, 0,          0 )
#అయితే_ముగింపు
	MI_నమోదు("parenb",   control,     REV,               PARENB,     0 )
	MI_నమోదు("parodd",   control,     REV,               PARODD,     0 )
#ఒకవేళ CMSPAR
	MI_నమోదు("cmspar",   control,     REV,               CMSPAR,     0 )
#అయితే_ముగింపు
	MI_నమోదు("cs5",      control,     0,                 CS5,     CSIZE)
	MI_నమోదు("cs6",      control,     0,                 CS6,     CSIZE)
	MI_నమోదు("cs7",      control,     0,                 CS7,     CSIZE)
	MI_నమోదు("cs8",      control,     0,                 CS8,     CSIZE)
	MI_నమోదు("hupcl",    control,     REV,               HUPCL,      0 )
	MI_నమోదు("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_నమోదు("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_నమోదు("cread",    control,     SANE_సమితి   | REV,  CREAD,      0 )
	MI_నమోదు("clocal",   control,     REV,               CLOCAL,     0 )
#ఒకవేళ CRTSCTS
	MI_నమోదు("crtscts",  control,     REV,               CRTSCTS,    0 )
#అయితే_ముగింపు
	MI_నమోదు("ignbrk",   ఆదానం,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_నమోదు("brkint",   ఆదానం,       SANE_సమితి   | REV,  BRKINT,     0 )
	MI_నమోదు("ignpar",   ఆదానం,       REV,               IGNPAR,     0 )
	MI_నమోదు("parmrk",   ఆదానం,       REV,               PARMRK,     0 )
	MI_నమోదు("inpck",    ఆదానం,       REV,               INPCK,      0 )
	MI_నమోదు("istrip",   ఆదానం,       REV,               ISTRIP,     0 )
	MI_నమోదు("inlcr",    ఆదానం,       SANE_UNSET | REV,  INLCR,      0 )
	MI_నమోదు("igncr",    ఆదానం,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_నమోదు("icrnl",    ఆదానం,       SANE_సమితి   | REV,  ICRNL,      0 )
	MI_నమోదు("ixon",     ఆదానం,       REV,               IXON,       0 )
	MI_నమోదు("ixoff",    ఆదానం,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_నమోదు("tandem",   ఆదానం,       OMIT       | REV,  IXOFF,      0 )
#ఒకవేళ IUCLC
	MI_నమోదు("iuclc",    ఆదానం,       SANE_UNSET | REV,  IUCLC,      0 )
#అయితే_ముగింపు
#ఒకవేళ IXANY
	MI_నమోదు("ixany",    ఆదానం,       SANE_UNSET | REV,  IXANY,      0 )
#అయితే_ముగింపు
#ఒకవేళ IMAXBEL
	MI_నమోదు("imaxbel",  ఆదానం,       SANE_సమితి   | REV,  IMAXBEL,    0 )
#అయితే_ముగింపు
#ఒకవేళ IUTF8
	MI_నమోదు("iutf8",    ఆదానం,       SANE_UNSET | REV,  IUTF8,      0 )
#అయితే_ముగింపు
	MI_నమోదు("opost",    output,      SANE_సమితి   | REV,  OPOST,      0 )
#ఒకవేళ OLCUC
	MI_నమోదు("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#అయితే_ముగింపు
#ఒకవేళ OCRNL
	MI_నమోదు("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONLCR
	MI_నమోదు("onlcr",    output,      SANE_సమితి   | REV,  ONLCR,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONOCR
	MI_నమోదు("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#అయితే_ముగింపు
#ఒకవేళ ONLRET
	MI_నమోదు("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#అయితే_ముగింపు
#ఒకవేళ OFILL
	MI_నమోదు("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#అయితే_ముగింపు
#ఒకవేళ OFDEL
	MI_నమోదు("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#అయితే_ముగింపు
#ఒకవేళ NLDLY
	MI_నమోదు("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_నమోదు("nl0",      output,      SANE_సమితి,          NL0,     NLDLY)
#అయితే_ముగింపు
#ఒకవేళ CRDLY
	MI_నమోదు("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_నమోదు("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_నమోదు("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_నమోదు("cr0",      output,      SANE_సమితి,          CR0,     CRDLY)
#అయితే_ముగింపు

#ఒకవేళ TABDLY
	MI_నమోదు("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# ఒకవేళ TAB2
	MI_నమోదు("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# ఒకవేళ TAB1
	MI_నమోదు("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_నమోదు("tab0",     output,      SANE_సమితి,          TAB0,   TABDLY)
#లేకపోతే
# ఒకవేళ OXTABS
	MI_నమోదు("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#అయితే_ముగింపు

#ఒకవేళ BSDLY
	MI_నమోదు("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_నమోదు("bs0",      output,      SANE_సమితి,          BS0,     BSDLY)
#అయితే_ముగింపు
#ఒకవేళ VTDLY
	MI_నమోదు("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_నమోదు("vt0",      output,      SANE_సమితి,          VT0,     VTDLY)
#అయితే_ముగింపు
#ఒకవేళ FFDLY
	MI_నమోదు("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_నమోదు("ff0",      output,      SANE_సమితి,          FF0,     FFDLY)
#అయితే_ముగింపు
	MI_నమోదు("isig",     local,       SANE_సమితి   | REV,  ISIG,       0 )
	MI_నమోదు("icanon",   local,       SANE_సమితి   | REV,  ICANON,     0 )
#ఒకవేళ IEXTEN
	MI_నమోదు("iexten",   local,       SANE_సమితి   | REV,  IEXTEN,     0 )
#అయితే_ముగింపు
	MI_నమోదు("echo",     local,       SANE_సమితి   | REV,  ECHO,       0 )
	MI_నమోదు("echoe",    local,       SANE_సమితి   | REV,  ECHOE,      0 )
	MI_నమోదు("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_నమోదు("echok",    local,       SANE_సమితి   | REV,  ECHOK,      0 )
	MI_నమోదు("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_నమోదు("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#ఒకవేళ XCASE
	MI_నమోదు("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#అయితే_ముగింపు
#ఒకవేళ TOSTOP
	MI_నమోదు("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOPRT
	MI_నమోదు("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_నమోదు("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOCTL
	MI_నమోదు("echoctl",  local,       SANE_సమితి   | REV,  ECHOCTL,    0 )
	MI_నమోదు("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#అయితే_ముగింపు
#ఒకవేళ ECHOKE
	MI_నమోదు("echoke",   local,       SANE_సమితి   | REV,  ECHOKE,     0 )
	MI_నమోదు("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#అయితే_ముగింపు
	MI_నమోదు("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#నిర్వచితమైతే EXTPROC
	MI_నమోదు("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#అయితే_ముగింపు
};

గణన {
	అంకె_బహుళకం_info = ARRAY_కొలత(బహుళకం_info)
};


/* Control characters */
నిర్మాణం control_info {
	స్థిరరాశి uint8_t saneval;  /* Value to set for 'stty sane' */
	స్థిరరాశి uint8_t స్థానభ్రంశం;   /* Offset in c_cc */
};

గణన {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#ఒకవేళ VEOL2
	CIDX_eol2,
#అయితే_ముగింపు
#ఒకవేళ VSWTCH
	CIDX_swtch,
#అయితే_ముగింపు
	CIDX_ప్రారంభం,
	CIDX_stop,
	CIDX_susp,
#ఒకవేళ VDSUSP
	CIDX_dsusp,
#అయితే_ముగింపు
#ఒకవేళ VREPRINT
	CIDX_rprnt,
#అయితే_ముగింపు
#ఒకవేళ VWERASE
	CIDX_werase,
#అయితే_ముగింపు
#ఒకవేళ VLNEXT
	CIDX_lnext,
#అయితే_ముగింపు
#ఒకవేళ VFLUSHO
	CIDX_flush,
#అయితే_ముగింపు
#ఒకవేళ VSTATUS
	CIDX_స్థితి,
#అయితే_ముగింపు
	CIDX_min,
	CIDX_సమయం,
};

#నిర్వచించు CI_నమోదు(n,s,o) n "\0"

/* Name given on command line */
స్థిర స్థిరరాశి అక్షరం control_name[] ALIGN1 =
	CI_నమోదు("intr",     CINTR,   VINTR   )
	CI_నమోదు("quit",     CQUIT,   VQUIT   )
	CI_నమోదు("erase",    CERASE,  VERASE  )
	CI_నమోదు("kill",     CKILL,   VKILL   )
	CI_నమోదు("eof",      CEOF,    VEOF    )
	CI_నమోదు("eol",      CEOL,    VEOL    )
#ఒకవేళ VEOL2
	CI_నమోదు("eol2",     CEOL2,   VEOL2   )
#అయితే_ముగింపు
#ఒకవేళ VSWTCH
	CI_నమోదు("swtch",    CSWTCH,  VSWTCH  )
#అయితే_ముగింపు
	CI_నమోదు("start",    CSTART,  VSTART  )
	CI_నమోదు("stop",     CSTOP,   VSTOP   )
	CI_నమోదు("susp",     CSUSP,   VSUSP   )
#ఒకవేళ VDSUSP
	CI_నమోదు("dsusp",    CDSUSP,  VDSUSP  )
#అయితే_ముగింపు
#ఒకవేళ VREPRINT
	CI_నమోదు("rprnt",    CRPRNT,  VREPRINT)
#అయితే_ముగింపు
#ఒకవేళ VWERASE
	CI_నమోదు("werase",   CWERASE, VWERASE )
#అయితే_ముగింపు
#ఒకవేళ VLNEXT
	CI_నమోదు("lnext",    CLNEXT,  VLNEXT  )
#అయితే_ముగింపు
#ఒకవేళ VFLUSHO
	CI_నమోదు("flush",    CFLUSHO, VFLUSHO )
#అయితే_ముగింపు
#ఒకవేళ VSTATUS
	CI_నమోదు("status",   CSTATUS, VSTATUS )
#అయితే_ముగింపు
	/* These must be last because of the display routines */
	CI_నమోదు("min",      1,       VMIN    )
	CI_నమోదు("time",     0,       VTIME   )
	;

#నిర్వచనం_రద్దు CI_నమోదు
#నిర్వచించు CI_నమోదు(n,s,o) { s, o },

స్థిర స్థిరరాశి నిర్మాణం control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_నమోదు("intr",     CINTR,   VINTR   )
	CI_నమోదు("quit",     CQUIT,   VQUIT   )
	CI_నమోదు("erase",    CERASE,  VERASE  )
	CI_నమోదు("kill",     CKILL,   VKILL   )
	CI_నమోదు("eof",      CEOF,    VEOF    )
	CI_నమోదు("eol",      CEOL,    VEOL    )
#ఒకవేళ VEOL2
	CI_నమోదు("eol2",     CEOL2,   VEOL2   )
#అయితే_ముగింపు
#ఒకవేళ VSWTCH
	CI_నమోదు("swtch",    CSWTCH,  VSWTCH  )
#అయితే_ముగింపు
	CI_నమోదు("start",    CSTART,  VSTART  )
	CI_నమోదు("stop",     CSTOP,   VSTOP   )
	CI_నమోదు("susp",     CSUSP,   VSUSP   )
#ఒకవేళ VDSUSP
	CI_నమోదు("dsusp",    CDSUSP,  VDSUSP  )
#అయితే_ముగింపు
#ఒకవేళ VREPRINT
	CI_నమోదు("rprnt",    CRPRNT,  VREPRINT)
#అయితే_ముగింపు
#ఒకవేళ VWERASE
	CI_నమోదు("werase",   CWERASE, VWERASE )
#అయితే_ముగింపు
#ఒకవేళ VLNEXT
	CI_నమోదు("lnext",    CLNEXT,  VLNEXT  )
#అయితే_ముగింపు
#ఒకవేళ VFLUSHO
	CI_నమోదు("flush",    CFLUSHO, VFLUSHO )
#అయితే_ముగింపు
#ఒకవేళ VSTATUS
	CI_నమోదు("status",   CSTATUS, VSTATUS )
#అయితే_ముగింపు
	/* These must be last because of the display routines */
	CI_నమోదు("min",      1,       VMIN    )
	CI_నమోదు("time",     0,       VTIME   )
};

గణన {
	అంకె_control_info = ARRAY_కొలత(control_info)
};


నిర్మాణం globals {
	స్థిరరాశి అక్షరం *device_name;
	/* The width of the screen, for output wrapping */
	గుర్తులేని max_నిలువు;
	/* Current position, to know when to wrap */
	గుర్తులేని current_నిలువు;
} FIX_ALIASING;
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	G.device_name = bb_సందేశం_standard_ఆదానం; \
	G.max_నిలువు = 80; \
	G.current_నిలువు = 0; /* we are noexec, must clear */ \
} అయితే (0)

స్థిర శూన్య సమితి_వేగం_or_die(గణన వేగం_setting type, స్థిరరాశి అక్షరం *arg,
					నిర్మాణం termios *బహుళకం)
{
	వేగం_t baud;

	baud = tty_విలువ_గమ్యం_baud(xatou(arg));

	ఒకవేళ (type != output_వేగం) {     /* either input or both */
		cfsetispeed(బహుళకం, baud);
	}
	ఒకవేళ (type != ఆదానం_వేగం) {      /* either output or both */
		cfsetospeed(బహుళకం, baud);
	}
}

స్థిర NORETURN శూన్య perror_on_device_and_die(స్థిరరాశి అక్షరం *fmt)
{
	bb_perror_సందేశం_and_die(fmt, G.device_name);
}

స్థిర శూన్య perror_on_device(స్థిరరాశి అక్షరం *fmt)
{
	bb_perror_సందేశం(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
స్థిర శూన్య wrapf(స్థిరరాశి అక్షరం *సందేశం_2, ...)
{
	అక్షరం buf[128];
	va_list వాదనలు;
	గుర్తులేని buflen;

	va_ప్రారంభం(వాదనలు, సందేశం_2);
	buflen = vsnprintf(buf, పరిమాణం(buf), సందేశం_2, వాదనలు);
	va_చివర(వాదనలు);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	ఒకవేళ (!buflen || buflen >= పరిమాణం(buf)) తిరిగి;

	ఒకవేళ (G.current_నిలువు > 0) {
		G.current_నిలువు++;
		ఒకవేళ (buf[0] != '\n') {
			ఒకవేళ (G.current_నిలువు + buflen >= G.max_నిలువు) {
				G.current_నిలువు = 0;
				bb_putchar('\n');
			} లేకపోతే {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_నిలువు += buflen;
	ఒకవేళ (buf[buflen-1] == '\n')
		G.current_నిలువు = 0;
}

స్థిర శూన్య newline(శూన్య)
{
	ఒకవేళ (G.current_నిలువు != 0)
		wrapf("\n");
}

#నిర్వచితమైతే TIOCGWINSZ
స్థిర శూన్య సమితి_కిటికీ_కొలత(పూర్ణాంకం అడ్డువరుసలు, పూర్ణాంకం నిలువుల_సంఖ్య)
{
	నిర్మాణం winsize win = { 0, 0, 0, 0 };

	ఒకవేళ (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		ఒకవేళ (errno != EINVAL) {
			వెళ్ళు bail;
		}
		memset(&win, 0, పరిమాణం(win));
	}

	ఒకవేళ (అడ్డువరుసలు >= 0)
		win.ws_అడ్డువరుస = అడ్డువరుసలు;
	ఒకవేళ (నిలువుల_సంఖ్య >= 0)
		win.ws_నిలువు = నిలువుల_సంఖ్య;

	ఒకవేళ (ioctl(STDIN_FILENO, TIOCSWINSZ, (అక్షరం *) &win))
bail:
		perror_on_device("%s");
}
#అయితే_ముగింపు

స్థిర శూన్య display_కిటికీ_కొలత(పూర్ణాంకం fancy)
{
	స్థిరరాశి అక్షరం *fmt_str = "%s\0%s: no size information for this device";
	గుర్తులేని వెడల్పు, ఎత్తు;

	ఒకవేళ (get_terminal_వెడల్పు_ఎత్తు(STDIN_FILENO, &వెడల్పు, &ఎత్తు)) {
		ఒకవేళ ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} లేకపోతే {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				ఎత్తు, వెడల్పు);
	}
}

స్థిర స్థిరరాశి నిర్మాణం suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

స్థిర స్థిరరాశి నిర్మాణం బహుళకం_info *కనుగొను_బహుళకం(స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం i = సూచీ_లోపల_అక్షరశ్రేణులు(బహుళకం_name, name);
	తిరిగి i >= 0 ? &బహుళకం_info[i] : ఖాళీ;
}

స్థిర స్థిరరాశి నిర్మాణం control_info *కనుగొను_control(స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం i = సూచీ_లోపల_అక్షరశ్రేణులు(control_name, name);
	తిరిగి i >= 0 ? &control_info[i] : ఖాళీ;
}

గణన {
	param_need_arg = 0x80,
	param_పంక్తి    = 1 | 0x80,
	param_అడ్డువరుసలు    = 2 | 0x80,
	param_నిలువుల_సంఖ్య    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_కొలత    = 5,
	param_వేగం   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

స్థిర పూర్ణాంకం కనుగొను_param(స్థిరరాశి అక్షరం *name)
{
	స్థిర స్థిరరాశి అక్షరం params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	పూర్ణాంకం i = సూచీ_లోపల_అక్షరశ్రేణులు(params, name) + 1;
	ఒకవేళ (i == 0)
		తిరిగి 0;
	ఒకవేళ (i != 5 && i != 6)
		i |= 0x80;
	తిరిగి i;
}

స్థిర పూర్ణాంకం recover_బహుళకం(స్థిరరాశి అక్షరం *arg, నిర్మాణం termios *బహుళకం)
{
	పూర్ణాంకం i, n;
	గుర్తులేని chr;
	గుర్తులేని దీర్ఘ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	ఒకవేళ (అక్షరశ్రేణి_చదువు(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		తిరిగి 0;
	బహుళకం->c_iflag = iflag;
	బహుళకం->c_oflag = oflag;
	బహుళకం->c_cflag = cflag;
	బహుళకం->c_lflag = lflag;
	arg += n;
	కొరకు (i = 0; i < NCCS; ++i) {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(arg, ":%x%n", &chr, &n) != 1)
			తిరిగి 0;
		బహుళకం->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	ఒకవేళ (*arg != '\0')
		తిరిగి 0;

	తిరిగి 1;
}

స్థిర శూన్య display_recoverable(స్థిరరాశి నిర్మాణం termios *బహుళకం,
				పూర్ణాంకం వాడని_PARAM dummy)
{
	పూర్ణాంకం i;
	రాయి("%lx:%lx:%lx:%lx",
		   (గుర్తులేని దీర్ఘ) బహుళకం->c_iflag, (గుర్తులేని దీర్ఘ) బహుళకం->c_oflag,
		   (గుర్తులేని దీర్ఘ) బహుళకం->c_cflag, (గుర్తులేని దీర్ఘ) బహుళకం->c_lflag);
	కొరకు (i = 0; i < NCCS; ++i)
		రాయి(":%x", (గుర్తులేని పూర్ణాంకం) బహుళకం->c_cc[i]);
	bb_putchar('\n');
}

స్థిర శూన్య display_వేగం(స్థిరరాశి నిర్మాణం termios *బహుళకం, పూర్ణాంకం fancy)
{
	//____________________ 01234567 8 9
	స్థిరరాశి అక్షరం *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	గుర్తులేని దీర్ఘ ispeed, ospeed;

	ispeed = cfgetispeed(బహుళకం);
	ospeed = cfgetospeed(బహుళకం);
	ఒకవేళ (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	ఒకవేళ (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_గమ్యం_విలువ(ispeed), tty_baud_గమ్యం_విలువ(ospeed));
}

స్థిర శూన్య do_display(స్థిరరాశి నిర్మాణం termios *బహుళకం, పూర్ణాంకం all)
{
	పూర్ణాంకం i;
	tcflag_t *bitsp;
	గుర్తులేని దీర్ఘ ముసుగు;
	పూర్ణాంకం prev_type = control;

	display_వేగం(బహుళకం, 1);
	ఒకవేళ (all)
		display_కిటికీ_కొలత(1);
#నిర్వచితమైతే __linux__
	wrapf("line = %u;\n", బహుళకం->c_పంక్తి);
#లేకపోతే
	newline();
#అయితే_ముగింపు

	కొరకు (i = 0; i != CIDX_min; ++i) {
		అక్షరం ch;
		అక్షరం buf10[10];

		/* If swtch is the same as susp, don't print both */
#ఒకవేళ VSWTCH == VSUSP
		ఒకవేళ (i == CIDX_swtch)
			కొనసాగు;
#అయితే_ముగింపు
		/* If eof uses the same slot as min, only print whichever applies */
#ఒకవేళ VEOF == VMIN
		ఒకవేళ (!(బహుళకం->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			కొనసాగు;
		}
#అయితే_ముగింపు
		ch = బహుళకం->c_cc[control_info[i].స్థానభ్రంశం];
		ఒకవేళ (ch == _POSIX_VDISABLE)
			strcpy(buf10, "<undef>");
		లేకపోతే
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#ఒకవేళ VEOF == VMIN
	ఒకవేళ ((బహుళకం->c_lflag & ICANON) == 0)
#అయితే_ముగింపు
		wrapf("min = %u; time = %u;", బహుళకం->c_cc[VMIN], బహుళకం->c_cc[VTIME]);
	newline();

	కొరకు (i = 0; i < అంకె_బహుళకం_info; ++i) {
		ఒకవేళ (బహుళకం_info[i].flags & OMIT)
			కొనసాగు;
		ఒకవేళ (బహుళకం_info[i].type != prev_type) {
			newline();
			prev_type = బహుళకం_info[i].type;
		}

		bitsp = get_ptr_గమ్యం_tcflag(బహుళకం_info[i].type, బహుళకం);
		ముసుగు = బహుళకం_info[i].ముసుగు ? బహుళకం_info[i].ముసుగు : బహుళకం_info[i].బిట్లు;
		ఒకవేళ ((*bitsp & ముసుగు) == బహుళకం_info[i].బిట్లు) {
			ఒకవేళ (all || (బహుళకం_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(బహుళకం_name, i));
		} లేకపోతే {
			ఒకవేళ ((all && బహుళకం_info[i].flags & REV)
			 || (!all && (బహుళకం_info[i].flags & (SANE_సమితి | REV)) == (SANE_సమితి | REV))
			) {
				wrapf("-%s", nth_string(బహుళకం_name, i));
			}
		}
	}
	newline();
}

స్థిర శూన్య sane_బహుళకం(నిర్మాణం termios *బహుళకం)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < అంకె_control_info; ++i) {
#ఒకవేళ VMIN == VEOF
		ఒకవేళ (i == CIDX_min)
			ఆపు;
#అయితే_ముగింపు
		బహుళకం->c_cc[control_info[i].స్థానభ్రంశం] = control_info[i].saneval;
	}

	కొరకు (i = 0; i < అంకె_బహుళకం_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_గమ్యం_tcflag(బహుళకం_info[i].type, బహుళకం);

		ఒకవేళ (!bitsp)
			కొనసాగు;
		val = *bitsp & ~((గుర్తులేని దీర్ఘ)బహుళకం_info[i].ముసుగు);
		ఒకవేళ (బహుళకం_info[i].flags & SANE_సమితి) {
			*bitsp = val | బహుళకం_info[i].బిట్లు;
		} లేకపోతే
		ఒకవేళ (బహుళకం_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~బహుళకం_info[i].బిట్లు;
		}
	}
}

స్థిర శూన్య సమితి_బహుళకం(స్థిరరాశి నిర్మాణం బహుళకం_info *info, పూర్ణాంకం reversed,
					నిర్మాణం termios *బహుళకం)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_గమ్యం_tcflag(info->type, బహుళకం);

	ఒకవేళ (bitsp) {
		tcflag_t val = *bitsp & ~info->ముసుగు;
		ఒకవేళ (reversed)
			*bitsp = val & ~info->బిట్లు;
		లేకపోతే
			*bitsp = val | info->బిట్లు;
		తిరిగి;
	}

	/* !bitsp - it's a "combination" mode */
	ఒకవేళ (info == &బహుళకం_info[IDX_evenp] || info == &బహుళకం_info[IDX_parity]) {
		ఒకవేళ (reversed)
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~PARENB & ~CSIZE) | CS8;
		లేకపోతే
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_oddp]) {
		ఒకవేళ (reversed)
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~PARENB & ~CSIZE) | CS8;
		లేకపోతే
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_nl]) {
		ఒకవేళ (reversed) {
			బహుళకం->c_iflag = (బహుళకం->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			బహుళకం->c_oflag = (బహుళకం->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} లేకపోతే {
			బహుళకం->c_iflag = బహుళకం->c_iflag & ~ICRNL;
			ఒకవేళ (ONLCR) బహుళకం->c_oflag = బహుళకం->c_oflag & ~ONLCR;
		}
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_ek]) {
		బహుళకం->c_cc[VERASE] = CERASE;
		బహుళకం->c_cc[VKILL] = CKILL;
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_sane]) {
		sane_బహుళకం(బహుళకం);
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_cbreak]) {
		ఒకవేళ (reversed)
			బహుళకం->c_lflag |= ICANON;
		లేకపోతే
			బహుళకం->c_lflag &= ~ICANON;
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_pass8]) {
		ఒకవేళ (reversed) {
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~CSIZE) | CS7 | PARENB;
			బహుళకం->c_iflag |= ISTRIP;
		} లేకపోతే {
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~PARENB & ~CSIZE) | CS8;
			బహుళకం->c_iflag &= ~ISTRIP;
		}
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_litout]) {
		ఒకవేళ (reversed) {
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~CSIZE) | CS7 | PARENB;
			బహుళకం->c_iflag |= ISTRIP;
			బహుళకం->c_oflag |= OPOST;
		} లేకపోతే {
			బహుళకం->c_cflag = (బహుళకం->c_cflag & ~PARENB & ~CSIZE) | CS8;
			బహుళకం->c_iflag &= ~ISTRIP;
			బహుళకం->c_oflag &= ~OPOST;
		}
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_raw] || info == &బహుళకం_info[IDX_cooked]) {
		ఒకవేళ ((info == &బహుళకం_info[IDX_raw] && reversed)
		 || (info == &బహుళకం_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			బహుళకం->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			బహుళకం->c_oflag |= OPOST;
			బహుళకం->c_lflag |= ISIG | ICANON;
#ఒకవేళ VMIN == VEOF
			బహుళకం->c_cc[VEOF] = CEOF;
#అయితే_ముగింపు
#ఒకవేళ VTIME == VEOL
			బహుళకం->c_cc[VEOL] = CEOL;
#అయితే_ముగింపు
		} లేకపోతే {
			/* Raw mode */
			బహుళకం->c_iflag = 0;
			బహుళకం->c_oflag &= ~OPOST;
			బహుళకం->c_lflag &= ~(ISIG | ICANON | XCASE);
			బహుళకం->c_cc[VMIN] = 1;
			బహుళకం->c_cc[VTIME] = 0;
		}
	}
#ఒకవేళ IXANY
	లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_decctlq]) {
		ఒకవేళ (reversed)
			బహుళకం->c_iflag |= IXANY;
		లేకపోతే
			బహుళకం->c_iflag &= ~IXANY;
	}
#అయితే_ముగింపు
#ఒకవేళ TABDLY
	లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_tabs]) {
		ఒకవేళ (reversed)
			బహుళకం->c_oflag = (బహుళకం->c_oflag & ~TABDLY) | TAB3;
		లేకపోతే
			బహుళకం->c_oflag = (బహుళకం->c_oflag & ~TABDLY) | TAB0;
	}
#అయితే_ముగింపు
#ఒకవేళ OXTABS
	లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_tabs]) {
		ఒకవేళ (reversed)
			బహుళకం->c_oflag |= OXTABS;
		లేకపోతే
			బహుళకం->c_oflag &= ~OXTABS;
	}
#అయితే_ముగింపు
#ఒకవేళ XCASE && IUCLC && OLCUC
	లేకపోతే ఒకవేళ (info==&బహుళకం_info[IDX_lcase] || info==&బహుళకం_info[IDX_LCASE]) {
		ఒకవేళ (reversed) {
			బహుళకం->c_lflag &= ~XCASE;
			బహుళకం->c_iflag &= ~IUCLC;
			బహుళకం->c_oflag &= ~OLCUC;
		} లేకపోతే {
			బహుళకం->c_lflag |= XCASE;
			బహుళకం->c_iflag |= IUCLC;
			బహుళకం->c_oflag |= OLCUC;
		}
	}
#అయితే_ముగింపు
	లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_crt]) {
		బహుళకం->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} లేకపోతే ఒకవేళ (info == &బహుళకం_info[IDX_dec]) {
		బహుళకం->c_cc[VINTR] = 3; /* ^C */
		బహుళకం->c_cc[VERASE] = 127; /* DEL */
		బహుళకం->c_cc[VKILL] = 21; /* ^U */
		బహుళకం->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		ఒకవేళ (IXANY) బహుళకం->c_iflag &= ~IXANY;
	}
}

స్థిర శూన్య సమితి_control_char_or_die(స్థిరరాశి నిర్మాణం control_info *info,
			స్థిరరాశి అక్షరం *arg, నిర్మాణం termios *బహుళకం)
{
	గుర్తులేని అక్షరం విలువ_2;

	ఒకవేళ (info == &control_info[CIDX_min] || info == &control_info[CIDX_సమయం])
		విలువ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	లేకపోతే ఒకవేళ (arg[0] == '\0' || arg[1] == '\0')
		విలువ_2 = arg[0];
	లేకపోతే ఒకవేళ (strcmp(arg, "^-") == 0 || strcmp(arg, "undef") == 0)
		విలువ_2 = _POSIX_VDISABLE;
	లేకపోతే ఒకవేళ (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		విలువ_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		ఒకవేళ (arg[1] == '?')
			విలువ_2 = 127;
	} లేకపోతే
		విలువ_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	బహుళకం->c_cc[info->స్థానభ్రంశం] = విలువ_2;
}

#నిర్వచించు STTY_require_సమితి_attr   (1 << 0)
#నిర్వచించు STTY_వేగం_was_సమితి      (1 << 1)
#నిర్వచించు STTY_verbose_output     (1 << 2)
#నిర్వచించు STTY_recoverable_output (1 << 3)
#నిర్వచించు STTY_noargs             (1 << 4)

పూర్ణాంకం stty_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం stty_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	నిర్మాణం termios బహుళకం;
	శూన్య (*output_func)(స్థిరరాశి నిర్మాణం termios *, పూర్ణాంకం);
	స్థిరరాశి అక్షరం *file_name = ఖాళీ;
	పూర్ణాంకం display_all = 0;
	పూర్ణాంకం stty_state;
	పూర్ణాంకం k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	అయితే (argv[++k]) {
		స్థిరరాశి నిర్మాణం బహుళకం_info *mp;
		స్థిరరాశి నిర్మాణం control_info *cp;
		స్థిరరాశి అక్షరం *arg = argv[k];
		స్థిరరాశి అక్షరం *argnext = argv[k+1];
		పూర్ణాంకం param;

		ఒకవేళ (arg[0] == '-') {
			పూర్ణాంకం i;
			mp = కనుగొను_బహుళకం(arg+1);
			ఒకవేళ (mp) {
				ఒకవేళ (!(mp->flags & REV))
					వెళ్ళు invalid_వాదం;
				stty_state &= ~STTY_noargs;
				కొనసాగు;
			}
			/* It is an option - parse it */
			i = 0;
			అయితే (arg[++i]) {
				ఎంపిక (arg[i]) {
				స్థితి 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					ఆపు;
				స్థితి 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					ఆపు;
				స్థితి 'F':
					ఒకవేళ (file_name)
						bb_simple_error_సందేశం_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					ఒకవేళ (!file_name[0]) { /* nope, "-F device" */
						పూర్ణాంకం p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						ఒకవేళ (!file_name)
							bb_error_సందేశం_and_die(bb_సందేశం_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						అయితే (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					వెళ్ళు చివర_option;
				అప్రమేయ:
					వెళ్ళు invalid_వాదం;
				}
			}
 చివర_option:
			కొనసాగు;
		}

		mp = కనుగొను_బహుళకం(arg);
		ఒకవేళ (mp) {
			stty_state &= ~STTY_noargs;
			కొనసాగు;
		}

		cp = కనుగొను_control(arg);
		ఒకవేళ (cp) {
			ఒకవేళ (!argnext)
				bb_error_సందేశం_and_die(bb_సందేశం_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			సమితి_control_char_or_die(cp, argnext, &బహుళకం);
			stty_state &= ~STTY_noargs;
			++k;
			కొనసాగు;
		}

		param = కనుగొను_param(arg);
		ఒకవేళ (param & param_need_arg) {
			ఒకవేళ (!argnext)
				bb_error_సందేశం_and_die(bb_సందేశం_requires_arg, arg);
			++k;
		}

		ఎంపిక (param) {
#నిర్వచితమైతే __linux__
		స్థితి param_పంక్తి:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, గరిష్ఠ_పూర్ణాంకం, stty_suffixes);
			ఆపు;
# endif /* else fall-through */
#అయితే_ముగింపు
#నిర్వచితమైతే TIOCGWINSZ
		స్థితి param_అడ్డువరుసలు:
		స్థితి param_నిలువుల_సంఖ్య:
		స్థితి param_columns:
			xatoul_range_sfx(argnext, 1, గరిష్ఠ_పూర్ణాంకం, stty_suffixes);
			ఆపు;
		స్థితి param_కొలత:
#అయితే_ముగింపు
		స్థితి param_వేగం:
			ఆపు;
		స్థితి param_ispeed:
			/* called for the side effect of xfunc death only */
			సమితి_వేగం_or_die(ఆదానం_వేగం, argnext, &బహుళకం);
			ఆపు;
		స్థితి param_ospeed:
			/* called for the side effect of xfunc death only */
			సమితి_వేగం_or_die(output_వేగం, argnext, &బహుళకం);
			ఆపు;
		అప్రమేయ:
			ఒకవేళ (recover_బహుళకం(arg, &బహుళకం) == 1) ఆపు;
			ఒకవేళ (tty_విలువ_గమ్యం_baud(xatou(arg)) != (వేగం_t) -1) ఆపు;
 invalid_వాదం:
			bb_error_సందేశం_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	ఒకవేళ ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_సందేశం_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	ఒకవేళ ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_సందేశం_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	ఒకవేళ (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	memset(&బహుళకం, 0, పరిమాణం(బహుళకం));
	ఒకవేళ (tcgetattr(STDIN_FILENO, &బహుళకం))
		perror_on_device_and_die("%s");

	ఒకవేళ (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_నిలువు = get_terminal_వెడల్పు(STDOUT_FILENO);
		output_func(&బహుళకం, display_all);
		తిరిగి EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	అయితే (argv[++k]) {
		స్థిరరాశి నిర్మాణం బహుళకం_info *mp;
		స్థిరరాశి నిర్మాణం control_info *cp;
		స్థిరరాశి అక్షరం *arg = argv[k];
		స్థిరరాశి అక్షరం *argnext = argv[k+1];
		పూర్ణాంకం param;

		ఒకవేళ (arg[0] == '-') {
			mp = కనుగొను_బహుళకం(arg+1);
			ఒకవేళ (mp) {
				సమితి_బహుళకం(mp, 1 /* reversed */, &బహుళకం);
				stty_state |= STTY_require_సమితి_attr;
			}
			/* It is an option - already parsed. Skip it */
			కొనసాగు;
		}

		mp = కనుగొను_బహుళకం(arg);
		ఒకవేళ (mp) {
			సమితి_బహుళకం(mp, 0 /* non-reversed */, &బహుళకం);
			stty_state |= STTY_require_సమితి_attr;
			కొనసాగు;
		}

		cp = కనుగొను_control(arg);
		ఒకవేళ (cp) {
			++k;
			సమితి_control_char_or_die(cp, argnext, &బహుళకం);
			stty_state |= STTY_require_సమితి_attr;
			కొనసాగు;
		}

		param = కనుగొను_param(arg);
		ఒకవేళ (param & param_need_arg) {
			++k;
		}

		ఎంపిక (param) {
#నిర్వచితమైతే __linux__
		స్థితి param_పంక్తి:
			బహుళకం.c_పంక్తి = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_సమితి_attr;
			ఆపు;
#అయితే_ముగింపు
#నిర్వచితమైతే TIOCGWINSZ
		స్థితి param_నిలువుల_సంఖ్య:
		స్థితి param_columns:
			సమితి_కిటికీ_కొలత(-1, xatoul_sfx(argnext, stty_suffixes));
			ఆపు;
		స్థితి param_కొలత:
			display_కిటికీ_కొలత(0);
			ఆపు;
		స్థితి param_అడ్డువరుసలు:
			సమితి_కిటికీ_కొలత(xatoul_sfx(argnext, stty_suffixes), -1);
			ఆపు;
#అయితే_ముగింపు
		స్థితి param_వేగం:
			display_వేగం(&బహుళకం, 0);
			ఆపు;
		స్థితి param_ispeed:
			సమితి_వేగం_or_die(ఆదానం_వేగం, argnext, &బహుళకం);
			stty_state |= (STTY_require_సమితి_attr | STTY_వేగం_was_సమితి);
			ఆపు;
		స్థితి param_ospeed:
			సమితి_వేగం_or_die(output_వేగం, argnext, &బహుళకం);
			stty_state |= (STTY_require_సమితి_attr | STTY_వేగం_was_సమితి);
			ఆపు;
		అప్రమేయ:
			ఒకవేళ (recover_బహుళకం(arg, &బహుళకం) == 1)
				stty_state |= STTY_require_సమితి_attr;
			లేకపోతే /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				సమితి_వేగం_or_die(both_speeds, arg, &బహుళకం);
				stty_state |= (STTY_require_సమితి_attr | STTY_వేగం_was_సమితి);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	ఒకవేళ (stty_state & STTY_require_సమితి_attr) {
		నిర్మాణం termios new_బహుళకం;

		ఒకవేళ (tcsetattr(STDIN_FILENO, TCSADRAIN, &బహుళకం))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		memset(&new_బహుళకం, 0, పరిమాణం(new_బహుళకం));
		ఒకవేళ (tcgetattr(STDIN_FILENO, &new_బహుళకం))
			perror_on_device_and_die("%s");

		ఒకవేళ (memcmp(&బహుళకం, &new_బహుళకం, పరిమాణం(బహుళకం)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#ఒకవేళ 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_బహుళకం.c_cflag &= (~CIBAUD);
			ఒకవేళ ((stty_state & STTY_వేగం_was_సమితి)
			 || memcmp(&బహుళకం, &new_బహుళకం, పరిమాణం(బహుళకం)) != 0)
#అయితే_ముగింపు
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	తిరిగి EXIT_SUCCESS;
}
