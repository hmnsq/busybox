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

#यदि_अपरिभाषितम् _POSIX_VDISABLE
# define _POSIX_VDISABLE ((अचिह्नित वर्ण) 0)
#यदि_अन्तः

#परिभाषय Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#यदि_अपरिभाषितम् CINTR
# define CINTR Control('c')
#यदि_अन्तः
#यदि_अपरिभाषितम् CQUIT
# define CQUIT 28
#यदि_अन्तः
#यदि_अपरिभाषितम् CERASE
# define CERASE 127
#यदि_अन्तः
#यदि_अपरिभाषितम् CKILL
# define CKILL Control('u')
#यदि_अन्तः
#यदि_अपरिभाषितम् CEOF
# define CEOF Control('d')
#यदि_अन्तः
#यदि_अपरिभाषितम् CEOL
# define CEOL _POSIX_VDISABLE
#यदि_अन्तः
#यदि_अपरिभाषितम् CSTART
# define CSTART Control('q')
#यदि_अन्तः
#यदि_अपरिभाषितम् CSTOP
# define CSTOP Control('s')
#यदि_अन्तः
#यदि_अपरिभाषितम् CSUSP
# define CSUSP Control('z')
#यदि_अन्तः
#यदि defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#यदि_अन्तः
/* glibc-2.12.1 uses only VSWTC name */
#यदि defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#यदि_अन्तः
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#यदि defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#यदि_अन्तः
#यदि defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#यदि_अन्तः

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#यदि defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#यदि_अन्तः

#यदि defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#यदि_अन्तः
#यदि defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#यदि_अन्तः
#यदि !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#यदि_अन्तः
#यदि defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#यदि_अन्तः
#यदि defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#यदि_अन्तः
#यदि defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#यदि_अन्तः
#यदि defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#यदि_अन्तः
#यदि defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#यदि_अन्तः
#यदि defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#यदि_अन्तः
#यदि defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#यदि_अन्तः
#यदि defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#यदि_अन्तः
#यदि defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#यदि_अन्तः
#यदि defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#यदि_अन्तः

/* Save us from #ifdef forest plague */
#यदि_अपरिभाषितम् BSDLY
# define BSDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् CIBAUD
# define CIBAUD 0
#यदि_अन्तः
#यदि_अपरिभाषितम् CRDLY
# define CRDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् CMSPAR
# define CMSPAR 0
#यदि_अन्तः
#यदि_अपरिभाषितम् CRTSCTS
# define CRTSCTS 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ECHOCTL
# define ECHOCTL 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ECHOKE
# define ECHOKE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ECHOPRT
# define ECHOPRT 0
#यदि_अन्तः
#यदि_अपरिभाषितम् FFDLY
# define FFDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् IEXTEN
# define IEXTEN 0
#यदि_अन्तः
#यदि_अपरिभाषितम् IMAXBEL
# define IMAXBEL 0
#यदि_अन्तः
#यदि_अपरिभाषितम् IUCLC
# define IUCLC 0
#यदि_अन्तः
#यदि_अपरिभाषितम् IXANY
# define IXANY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् NLDLY
# define NLDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् OCRNL
# define OCRNL 0
#यदि_अन्तः
#यदि_अपरिभाषितम् OFDEL
# define OFDEL 0
#यदि_अन्तः
#यदि_अपरिभाषितम् OFILL
# define OFILL 0
#यदि_अन्तः
#यदि_अपरिभाषितम् OLCUC
# define OLCUC 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ONLCR
# define ONLCR 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ONLRET
# define ONLRET 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ONOCR
# define ONOCR 0
#यदि_अन्तः
#यदि_अपरिभाषितम् OXTABS
# define OXTABS 0
#यदि_अन्तः
#यदि_अपरिभाषितम् TABDLY
# define TABDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् TAB1
# define TAB1 0
#यदि_अन्तः
#यदि_अपरिभाषितम् TAB2
# define TAB2 0
#यदि_अन्तः
#यदि_अपरिभाषितम् TOSTOP
# define TOSTOP 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VDSUSP
# define VDSUSP 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VEOL2
# define VEOL2 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VFLUSHO
# define VFLUSHO 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VLNEXT
# define VLNEXT 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VREPRINT
# define VREPRINT 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VSTATUS
# define VSTATUS 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VSWTCH
# define VSWTCH 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VTDLY
# define VTDLY 0
#यदि_अन्तः
#यदि_अपरिभाषितम् VWERASE
# define VWERASE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् XCASE
# define XCASE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् IUTF8
# define IUTF8 0
#यदि_अन्तः

/* Which speeds to set */
गणना जवः_setting {
	निवेशः_जवः, output_जवः, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
गणना {
	control, निवेशः, output, local, combination
};
स्थिर tcflag_t *get_ptr_गन्तव्यम्_tcflag(अचिह्नित type, ध्रुव संरचना termios *बहुलकम्)
{
	स्थिर ध्रुव uint8_t tcflag_offsets[] ALIGN1 = {
		offsetof(संरचना termios, c_cflag), /* control */
		offsetof(संरचना termios, c_iflag), /*निवेशः*/
		offsetof(संरचना termios, c_oflag), /* output */
		offsetof(संरचना termios, c_lflag)  /* local */
	};
	यदि (type <= local) {
		प्रत्यागम (tcflag_t*) (((वर्ण*)बहुलकम्) + tcflag_offsets[type]);
	}
	प्रत्यागम रिक्त;
}

/* Flags for 'struct mode_info' */
#परिभाषय SANE_समुच्चयः 1              /* Set in 'sane' mode                  */
#परिभाषय SANE_UNSET 2            /* Unset in 'sane' mode                */
#परिभाषय REV 4                   /* Can be turned off by prepending '-' */
#परिभाषय OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
संरचना बहुलकम्_info {
	ध्रुव uint8_t type;           /* Which structure element to change    */
	ध्रुव uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#यदि   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	ध्रुव uint8_t आवरणम्;
#नोचेद्यदि (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	ध्रुव uint16_t आवरणम्;
#अन्यथा
	ध्रुव tcflag_t आवरणम्;          /* Other bits to turn off for this mode */
#यदि_अन्तः
	/* was using short here, but ppc32 was unhappy */
	ध्रुव tcflag_t द्वयङ्काः;          /* Bits to set for this mode            */
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
#यदि_अन्तः
#यदि TABDLY || OXTABS
	IDX_tabs,
#यदि_अन्तः
#यदि XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#यदि_अन्तः
};

#परिभाषय MI_प्रविष्टिः(N,T,F,B,M) N "\0"

/* Mode names given on command line */
स्थिर ध्रुव वर्ण बहुलकम्_name[] ALIGN1 =
	MI_प्रविष्टिः("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("parity",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("nl",       combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("ek",       combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("sane",     combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("raw",      combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("litout",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("crt",      combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("dec",      combination, OMIT,              0,          0 )
#यदि IXANY
	MI_प्रविष्टिः("decctlq",  combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
#यदि TABDLY || OXTABS
	MI_प्रविष्टिः("tabs",     combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
#यदि XCASE && IUCLC && OLCUC
	MI_प्रविष्टिः("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("LCASE",    combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
	MI_प्रविष्टिः("parenb",   control,     REV,               PARENB,     0 )
	MI_प्रविष्टिः("parodd",   control,     REV,               PARODD,     0 )
#यदि CMSPAR
	MI_प्रविष्टिः("cmspar",   control,     REV,               CMSPAR,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("cs5",      control,     0,                 CS5,     CSIZE)
	MI_प्रविष्टिः("cs6",      control,     0,                 CS6,     CSIZE)
	MI_प्रविष्टिः("cs7",      control,     0,                 CS7,     CSIZE)
	MI_प्रविष्टिः("cs8",      control,     0,                 CS8,     CSIZE)
	MI_प्रविष्टिः("hupcl",    control,     REV,               HUPCL,      0 )
	MI_प्रविष्टिः("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_प्रविष्टिः("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_प्रविष्टिः("cread",    control,     SANE_समुच्चयः   | REV,  CREAD,      0 )
	MI_प्रविष्टिः("clocal",   control,     REV,               CLOCAL,     0 )
#यदि CRTSCTS
	MI_प्रविष्टिः("crtscts",  control,     REV,               CRTSCTS,    0 )
#यदि_अन्तः
	MI_प्रविष्टिः("ignbrk",   निवेशः,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_प्रविष्टिः("brkint",   निवेशः,       SANE_समुच्चयः   | REV,  BRKINT,     0 )
	MI_प्रविष्टिः("ignpar",   निवेशः,       REV,               IGNPAR,     0 )
	MI_प्रविष्टिः("parmrk",   निवेशः,       REV,               PARMRK,     0 )
	MI_प्रविष्टिः("inpck",    निवेशः,       REV,               INPCK,      0 )
	MI_प्रविष्टिः("istrip",   निवेशः,       REV,               ISTRIP,     0 )
	MI_प्रविष्टिः("inlcr",    निवेशः,       SANE_UNSET | REV,  INLCR,      0 )
	MI_प्रविष्टिः("igncr",    निवेशः,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_प्रविष्टिः("icrnl",    निवेशः,       SANE_समुच्चयः   | REV,  ICRNL,      0 )
	MI_प्रविष्टिः("ixon",     निवेशः,       REV,               IXON,       0 )
	MI_प्रविष्टिः("ixoff",    निवेशः,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_प्रविष्टिः("tandem",   निवेशः,       OMIT       | REV,  IXOFF,      0 )
#यदि IUCLC
	MI_प्रविष्टिः("iuclc",    निवेशः,       SANE_UNSET | REV,  IUCLC,      0 )
#यदि_अन्तः
#यदि IXANY
	MI_प्रविष्टिः("ixany",    निवेशः,       SANE_UNSET | REV,  IXANY,      0 )
#यदि_अन्तः
#यदि IMAXBEL
	MI_प्रविष्टिः("imaxbel",  निवेशः,       SANE_समुच्चयः   | REV,  IMAXBEL,    0 )
#यदि_अन्तः
#यदि IUTF8
	MI_प्रविष्टिः("iutf8",    निवेशः,       SANE_UNSET | REV,  IUTF8,      0 )
#यदि_अन्तः
	MI_प्रविष्टिः("opost",    output,      SANE_समुच्चयः   | REV,  OPOST,      0 )
#यदि OLCUC
	MI_प्रविष्टिः("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#यदि_अन्तः
#यदि OCRNL
	MI_प्रविष्टिः("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#यदि_अन्तः
#यदि ONLCR
	MI_प्रविष्टिः("onlcr",    output,      SANE_समुच्चयः   | REV,  ONLCR,      0 )
#यदि_अन्तः
#यदि ONOCR
	MI_प्रविष्टिः("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#यदि_अन्तः
#यदि ONLRET
	MI_प्रविष्टिः("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#यदि_अन्तः
#यदि OFILL
	MI_प्रविष्टिः("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#यदि_अन्तः
#यदि OFDEL
	MI_प्रविष्टिः("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#यदि_अन्तः
#यदि NLDLY
	MI_प्रविष्टिः("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_प्रविष्टिः("nl0",      output,      SANE_समुच्चयः,          NL0,     NLDLY)
#यदि_अन्तः
#यदि CRDLY
	MI_प्रविष्टिः("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_प्रविष्टिः("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_प्रविष्टिः("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_प्रविष्टिः("cr0",      output,      SANE_समुच्चयः,          CR0,     CRDLY)
#यदि_अन्तः

#यदि TABDLY
	MI_प्रविष्टिः("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# यदि TAB2
	MI_प्रविष्टिः("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# यदि TAB1
	MI_प्रविष्टिः("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_प्रविष्टिः("tab0",     output,      SANE_समुच्चयः,          TAB0,   TABDLY)
#अन्यथा
# यदि OXTABS
	MI_प्रविष्टिः("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#यदि_अन्तः

#यदि BSDLY
	MI_प्रविष्टिः("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_प्रविष्टिः("bs0",      output,      SANE_समुच्चयः,          BS0,     BSDLY)
#यदि_अन्तः
#यदि VTDLY
	MI_प्रविष्टिः("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_प्रविष्टिः("vt0",      output,      SANE_समुच्चयः,          VT0,     VTDLY)
#यदि_अन्तः
#यदि FFDLY
	MI_प्रविष्टिः("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_प्रविष्टिः("ff0",      output,      SANE_समुच्चयः,          FF0,     FFDLY)
#यदि_अन्तः
	MI_प्रविष्टिः("isig",     local,       SANE_समुच्चयः   | REV,  ISIG,       0 )
	MI_प्रविष्टिः("icanon",   local,       SANE_समुच्चयः   | REV,  ICANON,     0 )
#यदि IEXTEN
	MI_प्रविष्टिः("iexten",   local,       SANE_समुच्चयः   | REV,  IEXTEN,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("echo",     local,       SANE_समुच्चयः   | REV,  ECHO,       0 )
	MI_प्रविष्टिः("echoe",    local,       SANE_समुच्चयः   | REV,  ECHOE,      0 )
	MI_प्रविष्टिः("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_प्रविष्टिः("echok",    local,       SANE_समुच्चयः   | REV,  ECHOK,      0 )
	MI_प्रविष्टिः("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_प्रविष्टिः("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#यदि XCASE
	MI_प्रविष्टिः("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#यदि_अन्तः
#यदि TOSTOP
	MI_प्रविष्टिः("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#यदि_अन्तः
#यदि ECHOPRT
	MI_प्रविष्टिः("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_प्रविष्टिः("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#यदि_अन्तः
#यदि ECHOCTL
	MI_प्रविष्टिः("echoctl",  local,       SANE_समुच्चयः   | REV,  ECHOCTL,    0 )
	MI_प्रविष्टिः("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#यदि_अन्तः
#यदि ECHOKE
	MI_प्रविष्टिः("echoke",   local,       SANE_समुच्चयः   | REV,  ECHOKE,     0 )
	MI_प्रविष्टिः("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#यदि_परिभाषितम् EXTPROC
	MI_प्रविष्टिः("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#यदि_अन्तः
	;

#अपरिभाषय MI_प्रविष्टिः
#परिभाषय MI_प्रविष्टिः(N,T,F,B,M) { T, F, M, B },

स्थिर ध्रुव संरचना बहुलकम्_info बहुलकम्_info[] ALIGN4 = {
	/* This should be verbatim cut-n-paste copy of the above MI_ENTRYs */
	MI_प्रविष्टिः("evenp",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("parity",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("oddp",     combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("nl",       combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("ek",       combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("sane",     combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("cooked",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("raw",      combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("pass8",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("litout",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("cbreak",   combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("crt",      combination, OMIT,              0,          0 )
	MI_प्रविष्टिः("dec",      combination, OMIT,              0,          0 )
#यदि IXANY
	MI_प्रविष्टिः("decctlq",  combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
#यदि TABDLY || OXTABS
	MI_प्रविष्टिः("tabs",     combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
#यदि XCASE && IUCLC && OLCUC
	MI_प्रविष्टिः("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_प्रविष्टिः("LCASE",    combination, REV        | OMIT, 0,          0 )
#यदि_अन्तः
	MI_प्रविष्टिः("parenb",   control,     REV,               PARENB,     0 )
	MI_प्रविष्टिः("parodd",   control,     REV,               PARODD,     0 )
#यदि CMSPAR
	MI_प्रविष्टिः("cmspar",   control,     REV,               CMSPAR,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("cs5",      control,     0,                 CS5,     CSIZE)
	MI_प्रविष्टिः("cs6",      control,     0,                 CS6,     CSIZE)
	MI_प्रविष्टिः("cs7",      control,     0,                 CS7,     CSIZE)
	MI_प्रविष्टिः("cs8",      control,     0,                 CS8,     CSIZE)
	MI_प्रविष्टिः("hupcl",    control,     REV,               HUPCL,      0 )
	MI_प्रविष्टिः("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_प्रविष्टिः("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_प्रविष्टिः("cread",    control,     SANE_समुच्चयः   | REV,  CREAD,      0 )
	MI_प्रविष्टिः("clocal",   control,     REV,               CLOCAL,     0 )
#यदि CRTSCTS
	MI_प्रविष्टिः("crtscts",  control,     REV,               CRTSCTS,    0 )
#यदि_अन्तः
	MI_प्रविष्टिः("ignbrk",   निवेशः,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_प्रविष्टिः("brkint",   निवेशः,       SANE_समुच्चयः   | REV,  BRKINT,     0 )
	MI_प्रविष्टिः("ignpar",   निवेशः,       REV,               IGNPAR,     0 )
	MI_प्रविष्टिः("parmrk",   निवेशः,       REV,               PARMRK,     0 )
	MI_प्रविष्टिः("inpck",    निवेशः,       REV,               INPCK,      0 )
	MI_प्रविष्टिः("istrip",   निवेशः,       REV,               ISTRIP,     0 )
	MI_प्रविष्टिः("inlcr",    निवेशः,       SANE_UNSET | REV,  INLCR,      0 )
	MI_प्रविष्टिः("igncr",    निवेशः,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_प्रविष्टिः("icrnl",    निवेशः,       SANE_समुच्चयः   | REV,  ICRNL,      0 )
	MI_प्रविष्टिः("ixon",     निवेशः,       REV,               IXON,       0 )
	MI_प्रविष्टिः("ixoff",    निवेशः,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_प्रविष्टिः("tandem",   निवेशः,       OMIT       | REV,  IXOFF,      0 )
#यदि IUCLC
	MI_प्रविष्टिः("iuclc",    निवेशः,       SANE_UNSET | REV,  IUCLC,      0 )
#यदि_अन्तः
#यदि IXANY
	MI_प्रविष्टिः("ixany",    निवेशः,       SANE_UNSET | REV,  IXANY,      0 )
#यदि_अन्तः
#यदि IMAXBEL
	MI_प्रविष्टिः("imaxbel",  निवेशः,       SANE_समुच्चयः   | REV,  IMAXBEL,    0 )
#यदि_अन्तः
#यदि IUTF8
	MI_प्रविष्टिः("iutf8",    निवेशः,       SANE_UNSET | REV,  IUTF8,      0 )
#यदि_अन्तः
	MI_प्रविष्टिः("opost",    output,      SANE_समुच्चयः   | REV,  OPOST,      0 )
#यदि OLCUC
	MI_प्रविष्टिः("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#यदि_अन्तः
#यदि OCRNL
	MI_प्रविष्टिः("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#यदि_अन्तः
#यदि ONLCR
	MI_प्रविष्टिः("onlcr",    output,      SANE_समुच्चयः   | REV,  ONLCR,      0 )
#यदि_अन्तः
#यदि ONOCR
	MI_प्रविष्टिः("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#यदि_अन्तः
#यदि ONLRET
	MI_प्रविष्टिः("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#यदि_अन्तः
#यदि OFILL
	MI_प्रविष्टिः("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#यदि_अन्तः
#यदि OFDEL
	MI_प्रविष्टिः("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#यदि_अन्तः
#यदि NLDLY
	MI_प्रविष्टिः("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_प्रविष्टिः("nl0",      output,      SANE_समुच्चयः,          NL0,     NLDLY)
#यदि_अन्तः
#यदि CRDLY
	MI_प्रविष्टिः("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_प्रविष्टिः("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_प्रविष्टिः("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_प्रविष्टिः("cr0",      output,      SANE_समुच्चयः,          CR0,     CRDLY)
#यदि_अन्तः

#यदि TABDLY
	MI_प्रविष्टिः("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# यदि TAB2
	MI_प्रविष्टिः("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# यदि TAB1
	MI_प्रविष्टिः("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_प्रविष्टिः("tab0",     output,      SANE_समुच्चयः,          TAB0,   TABDLY)
#अन्यथा
# यदि OXTABS
	MI_प्रविष्टिः("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#यदि_अन्तः

#यदि BSDLY
	MI_प्रविष्टिः("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_प्रविष्टिः("bs0",      output,      SANE_समुच्चयः,          BS0,     BSDLY)
#यदि_अन्तः
#यदि VTDLY
	MI_प्रविष्टिः("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_प्रविष्टिः("vt0",      output,      SANE_समुच्चयः,          VT0,     VTDLY)
#यदि_अन्तः
#यदि FFDLY
	MI_प्रविष्टिः("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_प्रविष्टिः("ff0",      output,      SANE_समुच्चयः,          FF0,     FFDLY)
#यदि_अन्तः
	MI_प्रविष्टिः("isig",     local,       SANE_समुच्चयः   | REV,  ISIG,       0 )
	MI_प्रविष्टिः("icanon",   local,       SANE_समुच्चयः   | REV,  ICANON,     0 )
#यदि IEXTEN
	MI_प्रविष्टिः("iexten",   local,       SANE_समुच्चयः   | REV,  IEXTEN,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("echo",     local,       SANE_समुच्चयः   | REV,  ECHO,       0 )
	MI_प्रविष्टिः("echoe",    local,       SANE_समुच्चयः   | REV,  ECHOE,      0 )
	MI_प्रविष्टिः("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_प्रविष्टिः("echok",    local,       SANE_समुच्चयः   | REV,  ECHOK,      0 )
	MI_प्रविष्टिः("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_प्रविष्टिः("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#यदि XCASE
	MI_प्रविष्टिः("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#यदि_अन्तः
#यदि TOSTOP
	MI_प्रविष्टिः("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#यदि_अन्तः
#यदि ECHOPRT
	MI_प्रविष्टिः("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_प्रविष्टिः("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#यदि_अन्तः
#यदि ECHOCTL
	MI_प्रविष्टिः("echoctl",  local,       SANE_समुच्चयः   | REV,  ECHOCTL,    0 )
	MI_प्रविष्टिः("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#यदि_अन्तः
#यदि ECHOKE
	MI_प्रविष्टिः("echoke",   local,       SANE_समुच्चयः   | REV,  ECHOKE,     0 )
	MI_प्रविष्टिः("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#यदि_अन्तः
	MI_प्रविष्टिः("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#यदि_परिभाषितम् EXTPROC
	MI_प्रविष्टिः("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#यदि_अन्तः
};

गणना {
	अङ्कः_बहुलकम्_info = ARRAY_आकार(बहुलकम्_info)
};


/* Control characters */
संरचना control_info {
	ध्रुव uint8_t saneval;  /* Value to set for 'stty sane' */
	ध्रुव uint8_t विस्थापनम्;   /* Offset in c_cc */
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
#यदि_अन्तः
#यदि VSWTCH
	CIDX_swtch,
#यदि_अन्तः
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#यदि VDSUSP
	CIDX_dsusp,
#यदि_अन्तः
#यदि VREPRINT
	CIDX_rprnt,
#यदि_अन्तः
#यदि VWERASE
	CIDX_werase,
#यदि_अन्तः
#यदि VLNEXT
	CIDX_lnext,
#यदि_अन्तः
#यदि VFLUSHO
	CIDX_flush,
#यदि_अन्तः
#यदि VSTATUS
	CIDX_स्थितिः,
#यदि_अन्तः
	CIDX_min,
	CIDX_time,
};

#परिभाषय CI_प्रविष्टिः(n,s,o) n "\0"

/* Name given on command line */
स्थिर ध्रुव वर्ण control_name[] ALIGN1 =
	CI_प्रविष्टिः("intr",     CINTR,   VINTR   )
	CI_प्रविष्टिः("quit",     CQUIT,   VQUIT   )
	CI_प्रविष्टिः("erase",    CERASE,  VERASE  )
	CI_प्रविष्टिः("kill",     CKILL,   VKILL   )
	CI_प्रविष्टिः("eof",      CEOF,    VEOF    )
	CI_प्रविष्टिः("eol",      CEOL,    VEOL    )
#यदि VEOL2
	CI_प्रविष्टिः("eol2",     CEOL2,   VEOL2   )
#यदि_अन्तः
#यदि VSWTCH
	CI_प्रविष्टिः("swtch",    CSWTCH,  VSWTCH  )
#यदि_अन्तः
	CI_प्रविष्टिः("start",    CSTART,  VSTART  )
	CI_प्रविष्टिः("stop",     CSTOP,   VSTOP   )
	CI_प्रविष्टिः("susp",     CSUSP,   VSUSP   )
#यदि VDSUSP
	CI_प्रविष्टिः("dsusp",    CDSUSP,  VDSUSP  )
#यदि_अन्तः
#यदि VREPRINT
	CI_प्रविष्टिः("rprnt",    CRPRNT,  VREPRINT)
#यदि_अन्तः
#यदि VWERASE
	CI_प्रविष्टिः("werase",   CWERASE, VWERASE )
#यदि_अन्तः
#यदि VLNEXT
	CI_प्रविष्टिः("lnext",    CLNEXT,  VLNEXT  )
#यदि_अन्तः
#यदि VFLUSHO
	CI_प्रविष्टिः("flush",    CFLUSHO, VFLUSHO )
#यदि_अन्तः
#यदि VSTATUS
	CI_प्रविष्टिः("status",   CSTATUS, VSTATUS )
#यदि_अन्तः
	/* These must be last because of the display routines */
	CI_प्रविष्टिः("min",      1,       VMIN    )
	CI_प्रविष्टिः("time",     0,       VTIME   )
	;

#अपरिभाषय CI_प्रविष्टिः
#परिभाषय CI_प्रविष्टिः(n,s,o) { s, o },

स्थिर ध्रुव संरचना control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_प्रविष्टिः("intr",     CINTR,   VINTR   )
	CI_प्रविष्टिः("quit",     CQUIT,   VQUIT   )
	CI_प्रविष्टिः("erase",    CERASE,  VERASE  )
	CI_प्रविष्टिः("kill",     CKILL,   VKILL   )
	CI_प्रविष्टिः("eof",      CEOF,    VEOF    )
	CI_प्रविष्टिः("eol",      CEOL,    VEOL    )
#यदि VEOL2
	CI_प्रविष्टिः("eol2",     CEOL2,   VEOL2   )
#यदि_अन्तः
#यदि VSWTCH
	CI_प्रविष्टिः("swtch",    CSWTCH,  VSWTCH  )
#यदि_अन्तः
	CI_प्रविष्टिः("start",    CSTART,  VSTART  )
	CI_प्रविष्टिः("stop",     CSTOP,   VSTOP   )
	CI_प्रविष्टिः("susp",     CSUSP,   VSUSP   )
#यदि VDSUSP
	CI_प्रविष्टिः("dsusp",    CDSUSP,  VDSUSP  )
#यदि_अन्तः
#यदि VREPRINT
	CI_प्रविष्टिः("rprnt",    CRPRNT,  VREPRINT)
#यदि_अन्तः
#यदि VWERASE
	CI_प्रविष्टिः("werase",   CWERASE, VWERASE )
#यदि_अन्तः
#यदि VLNEXT
	CI_प्रविष्टिः("lnext",    CLNEXT,  VLNEXT  )
#यदि_अन्तः
#यदि VFLUSHO
	CI_प्रविष्टिः("flush",    CFLUSHO, VFLUSHO )
#यदि_अन्तः
#यदि VSTATUS
	CI_प्रविष्टिः("status",   CSTATUS, VSTATUS )
#यदि_अन्तः
	/* These must be last because of the display routines */
	CI_प्रविष्टिः("min",      1,       VMIN    )
	CI_प्रविष्टिः("time",     0,       VTIME   )
};

गणना {
	अङ्कः_control_info = ARRAY_आकार(control_info)
};


संरचना globals {
	ध्रुव वर्ण *device_name;
	/* The width of the screen, for output wrapping */
	अचिह्नित max_col;
	/* Current position, to know when to wrap */
	अचिह्नित current_col;
} FIX_ALIASING;
#परिभाषय G (*(संरचना globals*)bb_common_bufsiz1)
#परिभाषय INIT_G() कुरु { \
	सज्जता_common_bufsiz(); \
	G.device_name = bb_सन्देशः_standard_निवेशः; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} यावत् (0)

स्थिर शून्य समुच्चयः_जवः_or_die(गणना जवः_setting type, ध्रुव वर्ण *arg,
					संरचना termios *बहुलकम्)
{
	जवः_t baud;

	baud = tty_मानम्_गन्तव्यम्_baud(xatou(arg));

	यदि (type != output_जवः) {     /* either input or both */
		cfsetispeed(बहुलकम्, baud);
	}
	यदि (type != निवेशः_जवः) {      /* either output or both */
		cfsetospeed(बहुलकम्, baud);
	}
}

स्थिर NORETURN शून्य perror_on_device_and_die(ध्रुव वर्ण *fmt)
{
	bb_perror_सन्देशः_and_die(fmt, G.device_name);
}

स्थिर शून्य perror_on_device(ध्रुव वर्ण *fmt)
{
	bb_perror_सन्देशः(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
स्थिर शून्य wrapf(ध्रुव वर्ण *सन्देशः_2, ...)
{
	वर्ण buf[128];
	va_list तर्काः;
	अचिह्नित buflen;

	va_start(तर्काः, सन्देशः_2);
	buflen = vsnprintf(buf, परिमाण(buf), सन्देशः_2, तर्काः);
	va_अन्तः(तर्काः);
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

#यदि_परिभाषितम् TIOCGWINSZ
स्थिर शून्य समुच्चयः_गवाक्षः_आकार(पूर्णाङ्क पङ्क्तिकाः, पूर्णाङ्क स्तम्भसङ्ख्या)
{
	संरचना winsize win = { 0, 0, 0, 0 };

	यदि (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		यदि (errno != EINVAL) {
			गच्छ bail;
		}
		स्मृतिं_पूरय(&win, 0, परिमाण(win));
	}

	यदि (पङ्क्तिकाः >= 0)
		win.ws_row = पङ्क्तिकाः;
	यदि (स्तम्भसङ्ख्या >= 0)
		win.ws_col = स्तम्भसङ्ख्या;

	यदि (ioctl(STDIN_FILENO, TIOCSWINSZ, (वर्ण *) &win))
bail:
		perror_on_device("%s");
}
#यदि_अन्तः

स्थिर शून्य display_गवाक्षः_आकार(पूर्णाङ्क fancy)
{
	ध्रुव वर्ण *fmt_str = "%s\0%s: no size information for this device";
	अचिह्नित विस्तारः, height;

	यदि (get_terminal_विस्तारः_height(STDIN_FILENO, &विस्तारः, &height)) {
		यदि ((errno != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} अन्यथा {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, विस्तारः);
	}
}

स्थिर ध्रुव संरचना suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

स्थिर ध्रुव संरचना बहुलकम्_info *find_बहुलकम्(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i = सूचकांक_अन्तर्_वाक्यानि(बहुलकम्_name, name);
	प्रत्यागम i >= 0 ? &बहुलकम्_info[i] : रिक्त;
}

स्थिर ध्रुव संरचना control_info *find_control(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i = सूचकांक_अन्तर्_वाक्यानि(control_name, name);
	प्रत्यागम i >= 0 ? &control_info[i] : रिक्त;
}

गणना {
	param_need_arg = 0x80,
	param_पङ्क्तिः    = 1 | 0x80,
	param_पङ्क्तिकाः    = 2 | 0x80,
	param_स्तम्भसङ्ख्या    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_आकार    = 5,
	param_जवः   = 6,
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
	पूर्णाङ्क i = सूचकांक_अन्तर्_वाक्यानि(params, name) + 1;
	यदि (i == 0)
		प्रत्यागम 0;
	यदि (i != 5 && i != 6)
		i |= 0x80;
	प्रत्यागम i;
}

स्थिर पूर्णाङ्क recover_बहुलकम्(ध्रुव वर्ण *arg, संरचना termios *बहुलकम्)
{
	पूर्णाङ्क i, n;
	अचिह्नित chr;
	अचिह्नित दीर्घ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	यदि (वाक्यात्_पठ(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		प्रत्यागम 0;
	बहुलकम्->c_iflag = iflag;
	बहुलकम्->c_oflag = oflag;
	बहुलकम्->c_cflag = cflag;
	बहुलकम्->c_lflag = lflag;
	arg += n;
	प्रति (i = 0; i < NCCS; ++i) {
		यदि (वाक्यात्_पठ(arg, ":%x%n", &chr, &n) != 1)
			प्रत्यागम 0;
		बहुलकम्->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	यदि (*arg != '\0')
		प्रत्यागम 0;

	प्रत्यागम 1;
}

स्थिर शून्य display_recoverable(ध्रुव संरचना termios *बहुलकम्,
				पूर्णाङ्क अप्रयुक्तम्_PARAM dummy)
{
	पूर्णाङ्क i;
	प्रारूप_लिख("%lx:%lx:%lx:%lx",
		   (अचिह्नित दीर्घ) बहुलकम्->c_iflag, (अचिह्नित दीर्घ) बहुलकम्->c_oflag,
		   (अचिह्नित दीर्घ) बहुलकम्->c_cflag, (अचिह्नित दीर्घ) बहुलकम्->c_lflag);
	प्रति (i = 0; i < NCCS; ++i)
		प्रारूप_लिख(":%x", (अचिह्नित पूर्णाङ्क) बहुलकम्->c_cc[i]);
	bb_putchar('\n');
}

स्थिर शून्य display_जवः(ध्रुव संरचना termios *बहुलकम्, पूर्णाङ्क fancy)
{
	//____________________ 01234567 8 9
	ध्रुव वर्ण *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	अचिह्नित दीर्घ ispeed, ospeed;

	ispeed = cfgetispeed(बहुलकम्);
	ospeed = cfgetospeed(बहुलकम्);
	यदि (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	यदि (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_गन्तव्यम्_मानम्(ispeed), tty_baud_गन्तव्यम्_मानम्(ospeed));
}

स्थिर शून्य do_display(ध्रुव संरचना termios *बहुलकम्, पूर्णाङ्क all)
{
	पूर्णाङ्क i;
	tcflag_t *bitsp;
	अचिह्नित दीर्घ आवरणम्;
	पूर्णाङ्क पूर्व_type = control;

	display_जवः(बहुलकम्, 1);
	यदि (all)
		display_गवाक्षः_आकार(1);
#यदि_परिभाषितम् __linux__
	wrapf("line = %u;\n", बहुलकम्->c_पङ्क्तिः);
#अन्यथा
	newline();
#यदि_अन्तः

	प्रति (i = 0; i != CIDX_min; ++i) {
		वर्ण ch;
		वर्ण buf10[10];

		/* If swtch is the same as susp, don't print both */
#यदि VSWTCH == VSUSP
		यदि (i == CIDX_swtch)
			अनुवर्त;
#यदि_अन्तः
		/* If eof uses the same slot as min, only print whichever applies */
#यदि VEOF == VMIN
		यदि (!(बहुलकम्->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			अनुवर्त;
		}
#यदि_अन्तः
		ch = बहुलकम्->c_cc[control_info[i].विस्थापनम्];
		यदि (ch == _POSIX_VDISABLE)
			वाक्यं_प्रतिलिख(buf10, "<undef>");
		अन्यथा
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#यदि VEOF == VMIN
	यदि ((बहुलकम्->c_lflag & ICANON) == 0)
#यदि_अन्तः
		wrapf("min = %u; time = %u;", बहुलकम्->c_cc[VMIN], बहुलकम्->c_cc[VTIME]);
	newline();

	प्रति (i = 0; i < अङ्कः_बहुलकम्_info; ++i) {
		यदि (बहुलकम्_info[i].flags & OMIT)
			अनुवर्त;
		यदि (बहुलकम्_info[i].type != पूर्व_type) {
			newline();
			पूर्व_type = बहुलकम्_info[i].type;
		}

		bitsp = get_ptr_गन्तव्यम्_tcflag(बहुलकम्_info[i].type, बहुलकम्);
		आवरणम् = बहुलकम्_info[i].आवरणम् ? बहुलकम्_info[i].आवरणम् : बहुलकम्_info[i].द्वयङ्काः;
		यदि ((*bitsp & आवरणम्) == बहुलकम्_info[i].द्वयङ्काः) {
			यदि (all || (बहुलकम्_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(बहुलकम्_name, i));
		} अन्यथा {
			यदि ((all && बहुलकम्_info[i].flags & REV)
			 || (!all && (बहुलकम्_info[i].flags & (SANE_समुच्चयः | REV)) == (SANE_समुच्चयः | REV))
			) {
				wrapf("-%s", nth_string(बहुलकम्_name, i));
			}
		}
	}
	newline();
}

स्थिर शून्य sane_बहुलकम्(संरचना termios *बहुलकम्)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < अङ्कः_control_info; ++i) {
#यदि VMIN == VEOF
		यदि (i == CIDX_min)
			भङ्ग;
#यदि_अन्तः
		बहुलकम्->c_cc[control_info[i].विस्थापनम्] = control_info[i].saneval;
	}

	प्रति (i = 0; i < अङ्कः_बहुलकम्_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_गन्तव्यम्_tcflag(बहुलकम्_info[i].type, बहुलकम्);

		यदि (!bitsp)
			अनुवर्त;
		val = *bitsp & ~((अचिह्नित दीर्घ)बहुलकम्_info[i].आवरणम्);
		यदि (बहुलकम्_info[i].flags & SANE_समुच्चयः) {
			*bitsp = val | बहुलकम्_info[i].द्वयङ्काः;
		} अन्यथा
		यदि (बहुलकम्_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~बहुलकम्_info[i].द्वयङ्काः;
		}
	}
}

स्थिर शून्य समुच्चयः_बहुलकम्(ध्रुव संरचना बहुलकम्_info *info, पूर्णाङ्क reversed,
					संरचना termios *बहुलकम्)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_गन्तव्यम्_tcflag(info->type, बहुलकम्);

	यदि (bitsp) {
		tcflag_t val = *bitsp & ~info->आवरणम्;
		यदि (reversed)
			*bitsp = val & ~info->द्वयङ्काः;
		अन्यथा
			*bitsp = val | info->द्वयङ्काः;
		प्रत्यागम;
	}

	/* !bitsp - it's a "combination" mode */
	यदि (info == &बहुलकम्_info[IDX_evenp] || info == &बहुलकम्_info[IDX_parity]) {
		यदि (reversed)
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_oddp]) {
		यदि (reversed)
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_nl]) {
		यदि (reversed) {
			बहुलकम्->c_iflag = (बहुलकम्->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			बहुलकम्->c_oflag = (बहुलकम्->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} अन्यथा {
			बहुलकम्->c_iflag = बहुलकम्->c_iflag & ~ICRNL;
			यदि (ONLCR) बहुलकम्->c_oflag = बहुलकम्->c_oflag & ~ONLCR;
		}
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_ek]) {
		बहुलकम्->c_cc[VERASE] = CERASE;
		बहुलकम्->c_cc[VKILL] = CKILL;
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_sane]) {
		sane_बहुलकम्(बहुलकम्);
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_cbreak]) {
		यदि (reversed)
			बहुलकम्->c_lflag |= ICANON;
		अन्यथा
			बहुलकम्->c_lflag &= ~ICANON;
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_pass8]) {
		यदि (reversed) {
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~CSIZE) | CS7 | PARENB;
			बहुलकम्->c_iflag |= ISTRIP;
		} अन्यथा {
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~PARENB & ~CSIZE) | CS8;
			बहुलकम्->c_iflag &= ~ISTRIP;
		}
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_litout]) {
		यदि (reversed) {
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~CSIZE) | CS7 | PARENB;
			बहुलकम्->c_iflag |= ISTRIP;
			बहुलकम्->c_oflag |= OPOST;
		} अन्यथा {
			बहुलकम्->c_cflag = (बहुलकम्->c_cflag & ~PARENB & ~CSIZE) | CS8;
			बहुलकम्->c_iflag &= ~ISTRIP;
			बहुलकम्->c_oflag &= ~OPOST;
		}
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_raw] || info == &बहुलकम्_info[IDX_cooked]) {
		यदि ((info == &बहुलकम्_info[IDX_raw] && reversed)
		 || (info == &बहुलकम्_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			बहुलकम्->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			बहुलकम्->c_oflag |= OPOST;
			बहुलकम्->c_lflag |= ISIG | ICANON;
#यदि VMIN == VEOF
			बहुलकम्->c_cc[VEOF] = CEOF;
#यदि_अन्तः
#यदि VTIME == VEOL
			बहुलकम्->c_cc[VEOL] = CEOL;
#यदि_अन्तः
		} अन्यथा {
			/* Raw mode */
			बहुलकम्->c_iflag = 0;
			बहुलकम्->c_oflag &= ~OPOST;
			बहुलकम्->c_lflag &= ~(ISIG | ICANON | XCASE);
			बहुलकम्->c_cc[VMIN] = 1;
			बहुलकम्->c_cc[VTIME] = 0;
		}
	}
#यदि IXANY
	अन्यथा यदि (info == &बहुलकम्_info[IDX_decctlq]) {
		यदि (reversed)
			बहुलकम्->c_iflag |= IXANY;
		अन्यथा
			बहुलकम्->c_iflag &= ~IXANY;
	}
#यदि_अन्तः
#यदि TABDLY
	अन्यथा यदि (info == &बहुलकम्_info[IDX_tabs]) {
		यदि (reversed)
			बहुलकम्->c_oflag = (बहुलकम्->c_oflag & ~TABDLY) | TAB3;
		अन्यथा
			बहुलकम्->c_oflag = (बहुलकम्->c_oflag & ~TABDLY) | TAB0;
	}
#यदि_अन्तः
#यदि OXTABS
	अन्यथा यदि (info == &बहुलकम्_info[IDX_tabs]) {
		यदि (reversed)
			बहुलकम्->c_oflag |= OXTABS;
		अन्यथा
			बहुलकम्->c_oflag &= ~OXTABS;
	}
#यदि_अन्तः
#यदि XCASE && IUCLC && OLCUC
	अन्यथा यदि (info==&बहुलकम्_info[IDX_lcase] || info==&बहुलकम्_info[IDX_LCASE]) {
		यदि (reversed) {
			बहुलकम्->c_lflag &= ~XCASE;
			बहुलकम्->c_iflag &= ~IUCLC;
			बहुलकम्->c_oflag &= ~OLCUC;
		} अन्यथा {
			बहुलकम्->c_lflag |= XCASE;
			बहुलकम्->c_iflag |= IUCLC;
			बहुलकम्->c_oflag |= OLCUC;
		}
	}
#यदि_अन्तः
	अन्यथा यदि (info == &बहुलकम्_info[IDX_crt]) {
		बहुलकम्->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} अन्यथा यदि (info == &बहुलकम्_info[IDX_dec]) {
		बहुलकम्->c_cc[VINTR] = 3; /* ^C */
		बहुलकम्->c_cc[VERASE] = 127; /* DEL */
		बहुलकम्->c_cc[VKILL] = 21; /* ^U */
		बहुलकम्->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		यदि (IXANY) बहुलकम्->c_iflag &= ~IXANY;
	}
}

स्थिर शून्य समुच्चयः_control_char_or_die(ध्रुव संरचना control_info *info,
			ध्रुव वर्ण *arg, संरचना termios *बहुलकम्)
{
	अचिह्नित वर्ण मानम्_2;

	यदि (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		मानम्_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	अन्यथा यदि (arg[0] == '\0' || arg[1] == '\0')
		मानम्_2 = arg[0];
	अन्यथा यदि (वाक्यं_तुलय(arg, "^-") == 0 || वाक्यं_तुलय(arg, "undef") == 0)
		मानम्_2 = _POSIX_VDISABLE;
	अन्यथा यदि (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		मानम्_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		यदि (arg[1] == '?')
			मानम्_2 = 127;
	} अन्यथा
		मानम्_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	बहुलकम्->c_cc[info->विस्थापनम्] = मानम्_2;
}

#परिभाषय STTY_require_समुच्चयः_attr   (1 << 0)
#परिभाषय STTY_जवः_was_समुच्चयः      (1 << 1)
#परिभाषय STTY_verbose_output     (1 << 2)
#परिभाषय STTY_recoverable_output (1 << 3)
#परिभाषय STTY_noargs             (1 << 4)

पूर्णाङ्क stty_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क stty_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	संरचना termios बहुलकम्;
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
		ध्रुव संरचना बहुलकम्_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		यदि (arg[0] == '-') {
			पूर्णाङ्क i;
			mp = find_बहुलकम्(arg+1);
			यदि (mp) {
				यदि (!(mp->flags & REV))
					गच्छ invalid_तर्कः;
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
						bb_simple_error_सन्देशः_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					यदि (!file_name[0]) { /* nope, "-F device" */
						पूर्णाङ्क p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						यदि (!file_name)
							bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						यावत् (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					गच्छ अन्तः_option;
				स्वभाव:
					गच्छ invalid_तर्कः;
				}
			}
 अन्तः_option:
			अनुवर्त;
		}

		mp = find_बहुलकम्(arg);
		यदि (mp) {
			stty_state &= ~STTY_noargs;
			अनुवर्त;
		}

		cp = find_control(arg);
		यदि (cp) {
			यदि (!argnext)
				bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			समुच्चयः_control_char_or_die(cp, argnext, &बहुलकम्);
			stty_state &= ~STTY_noargs;
			++k;
			अनुवर्त;
		}

		param = find_param(arg);
		यदि (param & param_need_arg) {
			यदि (!argnext)
				bb_error_सन्देशः_and_die(bb_सन्देशः_requires_arg, arg);
			++k;
		}

		विकल्प (param) {
#यदि_परिभाषितम् __linux__
		पक्ष param_पङ्क्तिः:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, अधिकतम_पूर्णाङ्कः, stty_suffixes);
			भङ्ग;
# endif /* else fall-through */
#यदि_अन्तः
#यदि_परिभाषितम् TIOCGWINSZ
		पक्ष param_पङ्क्तिकाः:
		पक्ष param_स्तम्भसङ्ख्या:
		पक्ष param_columns:
			xatoul_range_sfx(argnext, 1, अधिकतम_पूर्णाङ्कः, stty_suffixes);
			भङ्ग;
		पक्ष param_आकार:
#यदि_अन्तः
		पक्ष param_जवः:
			भङ्ग;
		पक्ष param_ispeed:
			/* called for the side effect of xfunc death only */
			समुच्चयः_जवः_or_die(निवेशः_जवः, argnext, &बहुलकम्);
			भङ्ग;
		पक्ष param_ospeed:
			/* called for the side effect of xfunc death only */
			समुच्चयः_जवः_or_die(output_जवः, argnext, &बहुलकम्);
			भङ्ग;
		स्वभाव:
			यदि (recover_बहुलकम्(arg, &बहुलकम्) == 1) भङ्ग;
			यदि (tty_मानम्_गन्तव्यम्_baud(xatou(arg)) != (जवः_t) -1) भङ्ग;
 invalid_तर्कः:
			bb_error_सन्देशः_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	यदि ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_सन्देशः_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	यदि ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_सन्देशः_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	यदि (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	स्मृतिं_पूरय(&बहुलकम्, 0, परिमाण(बहुलकम्));
	यदि (tcgetattr(STDIN_FILENO, &बहुलकम्))
		perror_on_device_and_die("%s");

	यदि (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_विस्तारः(STDOUT_FILENO);
		output_func(&बहुलकम्, display_all);
		प्रत्यागम EXIT_SUCCESS;
	}

	/* Second pass: perform actions */
	k = 0;
	यावत् (argv[++k]) {
		ध्रुव संरचना बहुलकम्_info *mp;
		ध्रुव संरचना control_info *cp;
		ध्रुव वर्ण *arg = argv[k];
		ध्रुव वर्ण *argnext = argv[k+1];
		पूर्णाङ्क param;

		यदि (arg[0] == '-') {
			mp = find_बहुलकम्(arg+1);
			यदि (mp) {
				समुच्चयः_बहुलकम्(mp, 1 /* reversed */, &बहुलकम्);
				stty_state |= STTY_require_समुच्चयः_attr;
			}
			/* It is an option - already parsed. Skip it */
			अनुवर्त;
		}

		mp = find_बहुलकम्(arg);
		यदि (mp) {
			समुच्चयः_बहुलकम्(mp, 0 /* non-reversed */, &बहुलकम्);
			stty_state |= STTY_require_समुच्चयः_attr;
			अनुवर्त;
		}

		cp = find_control(arg);
		यदि (cp) {
			++k;
			समुच्चयः_control_char_or_die(cp, argnext, &बहुलकम्);
			stty_state |= STTY_require_समुच्चयः_attr;
			अनुवर्त;
		}

		param = find_param(arg);
		यदि (param & param_need_arg) {
			++k;
		}

		विकल्प (param) {
#यदि_परिभाषितम् __linux__
		पक्ष param_पङ्क्तिः:
			बहुलकम्.c_पङ्क्तिः = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_समुच्चयः_attr;
			भङ्ग;
#यदि_अन्तः
#यदि_परिभाषितम् TIOCGWINSZ
		पक्ष param_स्तम्भसङ्ख्या:
		पक्ष param_columns:
			समुच्चयः_गवाक्षः_आकार(-1, xatoul_sfx(argnext, stty_suffixes));
			भङ्ग;
		पक्ष param_आकार:
			display_गवाक्षः_आकार(0);
			भङ्ग;
		पक्ष param_पङ्क्तिकाः:
			समुच्चयः_गवाक्षः_आकार(xatoul_sfx(argnext, stty_suffixes), -1);
			भङ्ग;
#यदि_अन्तः
		पक्ष param_जवः:
			display_जवः(&बहुलकम्, 0);
			भङ्ग;
		पक्ष param_ispeed:
			समुच्चयः_जवः_or_die(निवेशः_जवः, argnext, &बहुलकम्);
			stty_state |= (STTY_require_समुच्चयः_attr | STTY_जवः_was_समुच्चयः);
			भङ्ग;
		पक्ष param_ospeed:
			समुच्चयः_जवः_or_die(output_जवः, argnext, &बहुलकम्);
			stty_state |= (STTY_require_समुच्चयः_attr | STTY_जवः_was_समुच्चयः);
			भङ्ग;
		स्वभाव:
			यदि (recover_बहुलकम्(arg, &बहुलकम्) == 1)
				stty_state |= STTY_require_समुच्चयः_attr;
			अन्यथा /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				समुच्चयः_जवः_or_die(both_speeds, arg, &बहुलकम्);
				stty_state |= (STTY_require_समुच्चयः_attr | STTY_जवः_was_समुच्चयः);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	यदि (stty_state & STTY_require_समुच्चयः_attr) {
		संरचना termios new_बहुलकम्;

		यदि (tcsetattr(STDIN_FILENO, TCSADRAIN, &बहुलकम्))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		स्मृतिं_पूरय(&new_बहुलकम्, 0, परिमाण(new_बहुलकम्));
		यदि (tcgetattr(STDIN_FILENO, &new_बहुलकम्))
			perror_on_device_and_die("%s");

		यदि (memcmp(&बहुलकम्, &new_बहुलकम्, परिमाण(बहुलकम्)) != 0) {
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

			new_बहुलकम्.c_cflag &= (~CIBAUD);
			यदि ((stty_state & STTY_जवः_was_समुच्चयः)
			 || memcmp(&बहुलकम्, &new_बहुलकम्, परिमाण(बहुलकम्)) != 0)
#यदि_अन्तः
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	प्रत्यागम EXIT_SUCCESS;
}
