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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

#अगर_अघोषित _POSIX_VDISABLE
# define _POSIX_VDISABLE ((अचिन्हित अक्षर) 0)
#पूर्ण_अगर

#घोषणा Control(c) ((c) & 0x1f)
/* Canonical values for control characters */
#अगर_अघोषित CINTR
# define CINTR Control('c')
#पूर्ण_अगर
#अगर_अघोषित CQUIT
# define CQUIT 28
#पूर्ण_अगर
#अगर_अघोषित CERASE
# define CERASE 127
#पूर्ण_अगर
#अगर_अघोषित CKILL
# define CKILL Control('u')
#पूर्ण_अगर
#अगर_अघोषित CEOF
# define CEOF Control('d')
#पूर्ण_अगर
#अगर_अघोषित CEOL
# define CEOL _POSIX_VDISABLE
#पूर्ण_अगर
#अगर_अघोषित CSTART
# define CSTART Control('q')
#पूर्ण_अगर
#अगर_अघोषित CSTOP
# define CSTOP Control('s')
#पूर्ण_अगर
#अगर_अघोषित CSUSP
# define CSUSP Control('z')
#पूर्ण_अगर
#अगर defined(VEOL2) && !defined(CEOL2)
# define CEOL2 _POSIX_VDISABLE
#पूर्ण_अगर
/* glibc-2.12.1 uses only VSWTC name */
#अगर defined(VSWTC) && !defined(VSWTCH)
# define VSWTCH VSWTC
#पूर्ण_अगर
/* ISC renamed swtch to susp for termios, but we'll accept either name */
#अगर defined(VSUSP) && !defined(VSWTCH)
# define VSWTCH VSUSP
# define CSWTCH CSUSP
#पूर्ण_अगर
#अगर defined(VSWTCH) && !defined(CSWTCH)
# define CSWTCH _POSIX_VDISABLE
#पूर्ण_अगर

/* SunOS 5.3 loses (^Z doesn't work) if 'swtch' is the same as 'susp'.
   So the default is to disable 'swtch.'  */
#अगर defined(__sparc__) && defined(__svr4__)
# undef CSWTCH
# define CSWTCH _POSIX_VDISABLE
#पूर्ण_अगर

#अगर defined(VWERSE) && !defined(VWERASE)       /* AIX-3.2.5 */
# define VWERASE VWERSE
#पूर्ण_अगर
#अगर defined(VDSUSP) && !defined(CDSUSP)
# define CDSUSP Control('y')
#पूर्ण_अगर
#अगर !defined(VREPRINT) && defined(VRPRNT)       /* Irix 4.0.5 */
# define VREPRINT VRPRNT
#पूर्ण_अगर
#अगर defined(VREPRINT) && !defined(CRPRNT)
# define CRPRNT Control('r')
#पूर्ण_अगर
#अगर defined(VWERASE) && !defined(CWERASE)
# define CWERASE Control('w')
#पूर्ण_अगर
#अगर defined(VLNEXT) && !defined(CLNEXT)
# define CLNEXT Control('v')
#पूर्ण_अगर
#अगर defined(VDISCARD) && !defined(VFLUSHO)
# define VFLUSHO VDISCARD
#पूर्ण_अगर
#अगर defined(VFLUSH) && !defined(VFLUSHO)        /* Ultrix 4.2 */
# define VFLUSHO VFLUSH
#पूर्ण_अगर
#अगर defined(CTLECH) && !defined(ECHOCTL)        /* Ultrix 4.3 */
# define ECHOCTL CTLECH
#पूर्ण_अगर
#अगर defined(TCTLECH) && !defined(ECHOCTL)       /* Ultrix 4.2 */
# define ECHOCTL TCTLECH
#पूर्ण_अगर
#अगर defined(CRTKIL) && !defined(ECHOKE)         /* Ultrix 4.2 and 4.3 */
# define ECHOKE CRTKIL
#पूर्ण_अगर
#अगर defined(VFLUSHO) && !defined(CFLUSHO)
# define CFLUSHO Control('o')
#पूर्ण_अगर
#अगर defined(VSTATUS) && !defined(CSTATUS)
# define CSTATUS Control('t')
#पूर्ण_अगर

/* Save us from #ifdef forest plague */
#अगर_अघोषित BSDLY
# define BSDLY 0
#पूर्ण_अगर
#अगर_अघोषित CIBAUD
# define CIBAUD 0
#पूर्ण_अगर
#अगर_अघोषित CRDLY
# define CRDLY 0
#पूर्ण_अगर
#अगर_अघोषित CMSPAR
# define CMSPAR 0
#पूर्ण_अगर
#अगर_अघोषित CRTSCTS
# define CRTSCTS 0
#पूर्ण_अगर
#अगर_अघोषित ECHOCTL
# define ECHOCTL 0
#पूर्ण_अगर
#अगर_अघोषित ECHOKE
# define ECHOKE 0
#पूर्ण_अगर
#अगर_अघोषित ECHOPRT
# define ECHOPRT 0
#पूर्ण_अगर
#अगर_अघोषित FFDLY
# define FFDLY 0
#पूर्ण_अगर
#अगर_अघोषित IEXTEN
# define IEXTEN 0
#पूर्ण_अगर
#अगर_अघोषित IMAXBEL
# define IMAXBEL 0
#पूर्ण_अगर
#अगर_अघोषित IUCLC
# define IUCLC 0
#पूर्ण_अगर
#अगर_अघोषित IXANY
# define IXANY 0
#पूर्ण_अगर
#अगर_अघोषित NLDLY
# define NLDLY 0
#पूर्ण_अगर
#अगर_अघोषित OCRNL
# define OCRNL 0
#पूर्ण_अगर
#अगर_अघोषित OFDEL
# define OFDEL 0
#पूर्ण_अगर
#अगर_अघोषित OFILL
# define OFILL 0
#पूर्ण_अगर
#अगर_अघोषित OLCUC
# define OLCUC 0
#पूर्ण_अगर
#अगर_अघोषित ONLCR
# define ONLCR 0
#पूर्ण_अगर
#अगर_अघोषित ONLRET
# define ONLRET 0
#पूर्ण_अगर
#अगर_अघोषित ONOCR
# define ONOCR 0
#पूर्ण_अगर
#अगर_अघोषित OXTABS
# define OXTABS 0
#पूर्ण_अगर
#अगर_अघोषित TABDLY
# define TABDLY 0
#पूर्ण_अगर
#अगर_अघोषित TAB1
# define TAB1 0
#पूर्ण_अगर
#अगर_अघोषित TAB2
# define TAB2 0
#पूर्ण_अगर
#अगर_अघोषित TOSTOP
# define TOSTOP 0
#पूर्ण_अगर
#अगर_अघोषित VDSUSP
# define VDSUSP 0
#पूर्ण_अगर
#अगर_अघोषित VEOL2
# define VEOL2 0
#पूर्ण_अगर
#अगर_अघोषित VFLUSHO
# define VFLUSHO 0
#पूर्ण_अगर
#अगर_अघोषित VLNEXT
# define VLNEXT 0
#पूर्ण_अगर
#अगर_अघोषित VREPRINT
# define VREPRINT 0
#पूर्ण_अगर
#अगर_अघोषित VSTATUS
# define VSTATUS 0
#पूर्ण_अगर
#अगर_अघोषित VSWTCH
# define VSWTCH 0
#पूर्ण_अगर
#अगर_अघोषित VTDLY
# define VTDLY 0
#पूर्ण_अगर
#अगर_अघोषित VWERASE
# define VWERASE 0
#पूर्ण_अगर
#अगर_अघोषित XCASE
# define XCASE 0
#पूर्ण_अगर
#अगर_अघोषित IUTF8
# define IUTF8 0
#पूर्ण_अगर

/* Which speeds to set */
क्रमागत speed_setting {
	निवेश_speed, output_speed, both_speeds
};

/* Which member(s) of 'struct termios' a mode uses */
क्रमागत {
	control, निवेश, output, local, combination
};
अटल tcflag_t *get_ptr_to_tcflag(अचिन्हित type, स्थिर काष्ठा termios *mode)
{
	अटल स्थिर uint8_t tcflag_offsets[] ALIGN1 = {
		दुरत्व(काष्ठा termios, c_cflag), /* control */
		दुरत्व(काष्ठा termios, c_iflag), /*निवेश*/
		दुरत्व(काष्ठा termios, c_oflag), /* output */
		दुरत्व(काष्ठा termios, c_lflag)  /* local */
	};
	अगर (type <= local) {
		वापस (tcflag_t*) (((अक्षर*)mode) + tcflag_offsets[type]);
	}
	वापस शून्य;
}

/* Flags for 'struct mode_info' */
#घोषणा SANE_SET 1              /* Set in 'sane' mode                  */
#घोषणा SANE_UNSET 2            /* Unset in 'sane' mode                */
#घोषणा REV 4                   /* Can be turned off by prepending '-' */
#घोषणा OMIT 8                  /* Don't display value                 */


/* Each mode.
 * This structure should be kept as small as humanly possible.
 */
काष्ठा mode_info {
	स्थिर uint8_t type;           /* Which structure element to change    */
	स्थिर uint8_t flags;          /* Setting and display options          */
	/* only these values are ever used, so... */
#अगर   (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x100
	स्थिर uint8_t mask;
#या_अगर (CSIZE | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY) < 0x10000
	स्थिर uint16_t mask;
#अन्यथा
	स्थिर tcflag_t mask;          /* Other bits to turn off for this mode */
#पूर्ण_अगर
	/* was using short here, but ppc32 was unhappy */
	स्थिर tcflag_t bits;          /* Bits to set for this mode            */
};

क्रमागत {
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
#अगर IXANY
	IDX_decctlq,
#पूर्ण_अगर
#अगर TABDLY || OXTABS
	IDX_tabs,
#पूर्ण_अगर
#अगर XCASE && IUCLC && OLCUC
	IDX_lcase,
	IDX_LCASE,
#पूर्ण_अगर
};

#घोषणा MI_ENTRY(N,T,F,B,M) N "\0"

/* Mode names given on command line */
अटल स्थिर अक्षर mode_name[] ALIGN1 =
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
#अगर IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
#अगर TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
#अगर XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#अगर CMSPAR
	MI_ENTRY("cmspar",   control,     REV,               CMSPAR,     0 )
#पूर्ण_अगर
	MI_ENTRY("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ENTRY("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ENTRY("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ENTRY("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ENTRY("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ENTRY("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ENTRY("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ENTRY("cread",    control,     SANE_SET   | REV,  CREAD,      0 )
	MI_ENTRY("clocal",   control,     REV,               CLOCAL,     0 )
#अगर CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#पूर्ण_अगर
	MI_ENTRY("ignbrk",   निवेश,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेश,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेश,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेश,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेश,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेश,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेश,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेश,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेश,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेश,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेश,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेश,       OMIT       | REV,  IXOFF,      0 )
#अगर IUCLC
	MI_ENTRY("iuclc",    निवेश,       SANE_UNSET | REV,  IUCLC,      0 )
#पूर्ण_अगर
#अगर IXANY
	MI_ENTRY("ixany",    निवेश,       SANE_UNSET | REV,  IXANY,      0 )
#पूर्ण_अगर
#अगर IMAXBEL
	MI_ENTRY("imaxbel",  निवेश,       SANE_SET   | REV,  IMAXBEL,    0 )
#पूर्ण_अगर
#अगर IUTF8
	MI_ENTRY("iutf8",    निवेश,       SANE_UNSET | REV,  IUTF8,      0 )
#पूर्ण_अगर
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#अगर OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#पूर्ण_अगर
#अगर OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#पूर्ण_अगर
#अगर ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#पूर्ण_अगर
#अगर ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#पूर्ण_अगर
#अगर ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#पूर्ण_अगर
#अगर OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#पूर्ण_अगर
#अगर OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#पूर्ण_अगर
#अगर NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#पूर्ण_अगर
#अगर CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#पूर्ण_अगर

#अगर TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# अगर TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# अगर TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# अगर OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#पूर्ण_अगर

#अगर BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#पूर्ण_अगर
#अगर VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#पूर्ण_अगर
#अगर FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#पूर्ण_अगर
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#अगर IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#पूर्ण_अगर
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#अगर XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#पूर्ण_अगर
#अगर TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#पूर्ण_अगर
#अगर ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#पूर्ण_अगर
#अगर ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#पूर्ण_अगर
#अगर ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#पूर्ण_अगर
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#अगर_घोषित EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#पूर्ण_अगर
	;

#अघोषित MI_ENTRY
#घोषणा MI_ENTRY(N,T,F,B,M) { T, F, M, B },

अटल स्थिर काष्ठा mode_info mode_info[] ALIGN4 = {
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
#अगर IXANY
	MI_ENTRY("decctlq",  combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
#अगर TABDLY || OXTABS
	MI_ENTRY("tabs",     combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
#अगर XCASE && IUCLC && OLCUC
	MI_ENTRY("lcase",    combination, REV        | OMIT, 0,          0 )
	MI_ENTRY("LCASE",    combination, REV        | OMIT, 0,          0 )
#पूर्ण_अगर
	MI_ENTRY("parenb",   control,     REV,               PARENB,     0 )
	MI_ENTRY("parodd",   control,     REV,               PARODD,     0 )
#अगर CMSPAR
	MI_ENTRY("cmspar",   control,     REV,               CMSPAR,     0 )
#पूर्ण_अगर
	MI_ENTRY("cs5",      control,     0,                 CS5,     CSIZE)
	MI_ENTRY("cs6",      control,     0,                 CS6,     CSIZE)
	MI_ENTRY("cs7",      control,     0,                 CS7,     CSIZE)
	MI_ENTRY("cs8",      control,     0,                 CS8,     CSIZE)
	MI_ENTRY("hupcl",    control,     REV,               HUPCL,      0 )
	MI_ENTRY("hup",      control,     REV        | OMIT, HUPCL,      0 )
	MI_ENTRY("cstopb",   control,     REV,               CSTOPB,     0 )
	MI_ENTRY("cread",    control,     SANE_SET   | REV,  CREAD,      0 )
	MI_ENTRY("clocal",   control,     REV,               CLOCAL,     0 )
#अगर CRTSCTS
	MI_ENTRY("crtscts",  control,     REV,               CRTSCTS,    0 )
#पूर्ण_अगर
	MI_ENTRY("ignbrk",   निवेश,       SANE_UNSET | REV,  IGNBRK,     0 )
	MI_ENTRY("brkint",   निवेश,       SANE_SET   | REV,  BRKINT,     0 )
	MI_ENTRY("ignpar",   निवेश,       REV,               IGNPAR,     0 )
	MI_ENTRY("parmrk",   निवेश,       REV,               PARMRK,     0 )
	MI_ENTRY("inpck",    निवेश,       REV,               INPCK,      0 )
	MI_ENTRY("istrip",   निवेश,       REV,               ISTRIP,     0 )
	MI_ENTRY("inlcr",    निवेश,       SANE_UNSET | REV,  INLCR,      0 )
	MI_ENTRY("igncr",    निवेश,       SANE_UNSET | REV,  IGNCR,      0 )
	MI_ENTRY("icrnl",    निवेश,       SANE_SET   | REV,  ICRNL,      0 )
	MI_ENTRY("ixon",     निवेश,       REV,               IXON,       0 )
	MI_ENTRY("ixoff",    निवेश,       SANE_UNSET | REV,  IXOFF,      0 )
	MI_ENTRY("tandem",   निवेश,       OMIT       | REV,  IXOFF,      0 )
#अगर IUCLC
	MI_ENTRY("iuclc",    निवेश,       SANE_UNSET | REV,  IUCLC,      0 )
#पूर्ण_अगर
#अगर IXANY
	MI_ENTRY("ixany",    निवेश,       SANE_UNSET | REV,  IXANY,      0 )
#पूर्ण_अगर
#अगर IMAXBEL
	MI_ENTRY("imaxbel",  निवेश,       SANE_SET   | REV,  IMAXBEL,    0 )
#पूर्ण_अगर
#अगर IUTF8
	MI_ENTRY("iutf8",    निवेश,       SANE_UNSET | REV,  IUTF8,      0 )
#पूर्ण_अगर
	MI_ENTRY("opost",    output,      SANE_SET   | REV,  OPOST,      0 )
#अगर OLCUC
	MI_ENTRY("olcuc",    output,      SANE_UNSET | REV,  OLCUC,      0 )
#पूर्ण_अगर
#अगर OCRNL
	MI_ENTRY("ocrnl",    output,      SANE_UNSET | REV,  OCRNL,      0 )
#पूर्ण_अगर
#अगर ONLCR
	MI_ENTRY("onlcr",    output,      SANE_SET   | REV,  ONLCR,      0 )
#पूर्ण_अगर
#अगर ONOCR
	MI_ENTRY("onocr",    output,      SANE_UNSET | REV,  ONOCR,      0 )
#पूर्ण_अगर
#अगर ONLRET
	MI_ENTRY("onlret",   output,      SANE_UNSET | REV,  ONLRET,     0 )
#पूर्ण_अगर
#अगर OFILL
	MI_ENTRY("ofill",    output,      SANE_UNSET | REV,  OFILL,      0 )
#पूर्ण_अगर
#अगर OFDEL
	MI_ENTRY("ofdel",    output,      SANE_UNSET | REV,  OFDEL,      0 )
#पूर्ण_अगर
#अगर NLDLY
	MI_ENTRY("nl1",      output,      SANE_UNSET,        NL1,     NLDLY)
	MI_ENTRY("nl0",      output,      SANE_SET,          NL0,     NLDLY)
#पूर्ण_अगर
#अगर CRDLY
	MI_ENTRY("cr3",      output,      SANE_UNSET,        CR3,     CRDLY)
	MI_ENTRY("cr2",      output,      SANE_UNSET,        CR2,     CRDLY)
	MI_ENTRY("cr1",      output,      SANE_UNSET,        CR1,     CRDLY)
	MI_ENTRY("cr0",      output,      SANE_SET,          CR0,     CRDLY)
#पूर्ण_अगर

#अगर TABDLY
	MI_ENTRY("tab3",     output,      SANE_UNSET,        TAB3,   TABDLY)
# अगर TAB2
	MI_ENTRY("tab2",     output,      SANE_UNSET,        TAB2,   TABDLY)
# endif
# अगर TAB1
	MI_ENTRY("tab1",     output,      SANE_UNSET,        TAB1,   TABDLY)
# endif
	MI_ENTRY("tab0",     output,      SANE_SET,          TAB0,   TABDLY)
#अन्यथा
# अगर OXTABS
	MI_ENTRY("tab3",     output,      SANE_UNSET,        OXTABS,     0 )
# endif
#पूर्ण_अगर

#अगर BSDLY
	MI_ENTRY("bs1",      output,      SANE_UNSET,        BS1,     BSDLY)
	MI_ENTRY("bs0",      output,      SANE_SET,          BS0,     BSDLY)
#पूर्ण_अगर
#अगर VTDLY
	MI_ENTRY("vt1",      output,      SANE_UNSET,        VT1,     VTDLY)
	MI_ENTRY("vt0",      output,      SANE_SET,          VT0,     VTDLY)
#पूर्ण_अगर
#अगर FFDLY
	MI_ENTRY("ff1",      output,      SANE_UNSET,        FF1,     FFDLY)
	MI_ENTRY("ff0",      output,      SANE_SET,          FF0,     FFDLY)
#पूर्ण_अगर
	MI_ENTRY("isig",     local,       SANE_SET   | REV,  ISIG,       0 )
	MI_ENTRY("icanon",   local,       SANE_SET   | REV,  ICANON,     0 )
#अगर IEXTEN
	MI_ENTRY("iexten",   local,       SANE_SET   | REV,  IEXTEN,     0 )
#पूर्ण_अगर
	MI_ENTRY("echo",     local,       SANE_SET   | REV,  ECHO,       0 )
	MI_ENTRY("echoe",    local,       SANE_SET   | REV,  ECHOE,      0 )
	MI_ENTRY("crterase", local,       OMIT       | REV,  ECHOE,      0 )
	MI_ENTRY("echok",    local,       SANE_SET   | REV,  ECHOK,      0 )
	MI_ENTRY("echonl",   local,       SANE_UNSET | REV,  ECHONL,     0 )
	MI_ENTRY("noflsh",   local,       SANE_UNSET | REV,  NOFLSH,     0 )
#अगर XCASE
	MI_ENTRY("xcase",    local,       SANE_UNSET | REV,  XCASE,      0 )
#पूर्ण_अगर
#अगर TOSTOP
	MI_ENTRY("tostop",   local,       SANE_UNSET | REV,  TOSTOP,     0 )
#पूर्ण_अगर
#अगर ECHOPRT
	MI_ENTRY("echoprt",  local,       SANE_UNSET | REV,  ECHOPRT,    0 )
	MI_ENTRY("prterase", local,       OMIT       | REV,  ECHOPRT,    0 )
#पूर्ण_अगर
#अगर ECHOCTL
	MI_ENTRY("echoctl",  local,       SANE_SET   | REV,  ECHOCTL,    0 )
	MI_ENTRY("ctlecho",  local,       OMIT       | REV,  ECHOCTL,    0 )
#पूर्ण_अगर
#अगर ECHOKE
	MI_ENTRY("echoke",   local,       SANE_SET   | REV,  ECHOKE,     0 )
	MI_ENTRY("crtkill",  local,       OMIT       | REV,  ECHOKE,     0 )
#पूर्ण_अगर
	MI_ENTRY("flusho",   local,       SANE_UNSET | REV,  FLUSHO,     0 )
#अगर_घोषित EXTPROC
	MI_ENTRY("extproc",  local,       SANE_UNSET | REV,  EXTPROC,    0 )
#पूर्ण_अगर
};

क्रमागत {
	अंक_mode_info = ARRAY_आकार(mode_info)
};


/* Control characters */
काष्ठा control_info {
	स्थिर uint8_t saneval;  /* Value to set for 'stty sane' */
	स्थिर uint8_t offset;   /* Offset in c_cc */
};

क्रमागत {
	/* Must match control_name[] and control_info[] order! */
	CIDX_intr = 0,
	CIDX_quit,
	CIDX_erase,
	CIDX_kill,
	CIDX_eof,
	CIDX_eol,
#अगर VEOL2
	CIDX_eol2,
#पूर्ण_अगर
#अगर VSWTCH
	CIDX_swtch,
#पूर्ण_अगर
	CIDX_start,
	CIDX_stop,
	CIDX_susp,
#अगर VDSUSP
	CIDX_dsusp,
#पूर्ण_अगर
#अगर VREPRINT
	CIDX_rprnt,
#पूर्ण_अगर
#अगर VWERASE
	CIDX_werase,
#पूर्ण_अगर
#अगर VLNEXT
	CIDX_lnext,
#पूर्ण_अगर
#अगर VFLUSHO
	CIDX_flush,
#पूर्ण_अगर
#अगर VSTATUS
	CIDX_status,
#पूर्ण_अगर
	CIDX_min,
	CIDX_time,
};

#घोषणा CI_ENTRY(n,s,o) n "\0"

/* Name given on command line */
अटल स्थिर अक्षर control_name[] ALIGN1 =
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#अगर VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#पूर्ण_अगर
#अगर VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#पूर्ण_अगर
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#अगर VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#पूर्ण_अगर
#अगर VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#पूर्ण_अगर
#अगर VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#पूर्ण_अगर
#अगर VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#पूर्ण_अगर
#अगर VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#पूर्ण_अगर
#अगर VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#पूर्ण_अगर
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
	;

#अघोषित CI_ENTRY
#घोषणा CI_ENTRY(n,s,o) { s, o },

अटल स्थिर काष्ठा control_info control_info[] ALIGN2 = {
	/* This should be verbatim cut-n-paste copy of the above CI_ENTRYs */
	CI_ENTRY("intr",     CINTR,   VINTR   )
	CI_ENTRY("quit",     CQUIT,   VQUIT   )
	CI_ENTRY("erase",    CERASE,  VERASE  )
	CI_ENTRY("kill",     CKILL,   VKILL   )
	CI_ENTRY("eof",      CEOF,    VEOF    )
	CI_ENTRY("eol",      CEOL,    VEOL    )
#अगर VEOL2
	CI_ENTRY("eol2",     CEOL2,   VEOL2   )
#पूर्ण_अगर
#अगर VSWTCH
	CI_ENTRY("swtch",    CSWTCH,  VSWTCH  )
#पूर्ण_अगर
	CI_ENTRY("start",    CSTART,  VSTART  )
	CI_ENTRY("stop",     CSTOP,   VSTOP   )
	CI_ENTRY("susp",     CSUSP,   VSUSP   )
#अगर VDSUSP
	CI_ENTRY("dsusp",    CDSUSP,  VDSUSP  )
#पूर्ण_अगर
#अगर VREPRINT
	CI_ENTRY("rprnt",    CRPRNT,  VREPRINT)
#पूर्ण_अगर
#अगर VWERASE
	CI_ENTRY("werase",   CWERASE, VWERASE )
#पूर्ण_अगर
#अगर VLNEXT
	CI_ENTRY("lnext",    CLNEXT,  VLNEXT  )
#पूर्ण_अगर
#अगर VFLUSHO
	CI_ENTRY("flush",    CFLUSHO, VFLUSHO )
#पूर्ण_अगर
#अगर VSTATUS
	CI_ENTRY("status",   CSTATUS, VSTATUS )
#पूर्ण_अगर
	/* These must be last because of the display routines */
	CI_ENTRY("min",      1,       VMIN    )
	CI_ENTRY("time",     0,       VTIME   )
};

क्रमागत {
	अंक_control_info = ARRAY_आकार(control_info)
};


काष्ठा globals {
	स्थिर अक्षर *device_name;
	/* The width of the screen, for output wrapping */
	अचिन्हित max_col;
	/* Current position, to know when to wrap */
	अचिन्हित current_col;
} FIX_ALIASING;
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { \
	setup_common_bufsiz(); \
	G.device_name = bb_संदेश_standard_निवेश; \
	G.max_col = 80; \
	G.current_col = 0; /* we are noexec, must clear */ \
} जबतक (0)

अटल व्योम set_speed_or_die(क्रमागत speed_setting type, स्थिर अक्षर *arg,
					काष्ठा termios *mode)
{
	speed_t baud;

	baud = tty_मूल्य_to_baud(xatou(arg));

	अगर (type != output_speed) {     /* either input or both */
		cfsetispeed(mode, baud);
	}
	अगर (type != निवेश_speed) {      /* either output or both */
		cfsetospeed(mode, baud);
	}
}

अटल NORETURN व्योम perror_on_device_and_die(स्थिर अक्षर *fmt)
{
	bb_perror_संदेश_and_die(fmt, G.device_name);
}

अटल व्योम perror_on_device(स्थिर अक्षर *fmt)
{
	bb_perror_संदेश(fmt, G.device_name);
}

/* Print format string MESSAGE and optional args.
   Wrap to next line first if it won't fit.
   Print a space first unless MESSAGE will start a new line */
अटल व्योम wrapf(स्थिर अक्षर *message, ...)
{
	अक्षर buf[128];
	बहु_सूची args;
	अचिन्हित buflen;

	बहु_शुरू(args, message);
	buflen = vsnprintf(buf, माप(buf), message, args);
	बहु_पूर्ण(args);
	/* We seem to be called only with suitable lengths, but check if
	   somebody failed to adhere to this assumption just to be sure.  */
	अगर (!buflen || buflen >= माप(buf)) वापस;

	अगर (G.current_col > 0) {
		G.current_col++;
		अगर (buf[0] != '\n') {
			अगर (G.current_col + buflen >= G.max_col) {
				G.current_col = 0;
				bb_putchar('\n');
			} अन्यथा {
				bb_putchar(' ');
			}
		}
	}
	fputs_stdout(buf);
	G.current_col += buflen;
	अगर (buf[buflen-1] == '\n')
		G.current_col = 0;
}

अटल व्योम newline(व्योम)
{
	अगर (G.current_col != 0)
		wrapf("\n");
}

#अगर_घोषित TIOCGWINSZ
अटल व्योम set_window_आकार(पूर्णांक rows, पूर्णांक cols)
{
	काष्ठा winsize win = { 0, 0, 0, 0 };

	अगर (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		अगर (त्रुटि_सं != EINVAL) {
			जाओ bail;
		}
		स_रखो(&win, 0, माप(win));
	}

	अगर (rows >= 0)
		win.ws_row = rows;
	अगर (cols >= 0)
		win.ws_col = cols;

	अगर (ioctl(STDIN_FILENO, TIOCSWINSZ, (अक्षर *) &win))
bail:
		perror_on_device("%s");
}
#पूर्ण_अगर

अटल व्योम display_window_आकार(पूर्णांक fancy)
{
	स्थिर अक्षर *fmt_str = "%s\0%s: no size information for this device";
	अचिन्हित width, height;

	अगर (get_terminal_width_height(STDIN_FILENO, &width, &height)) {
		अगर ((त्रुटि_सं != EINVAL) || ((fmt_str += 2), !fancy)) {
			perror_on_device(fmt_str);
		}
	} अन्यथा {
		wrapf(fancy ? "rows %u; columns %u;" : "%u %u\n",
				height, width);
	}
}

अटल स्थिर काष्ठा suffix_mult stty_suffixes[] ALIGN_SUFFIX = {
	{ "b",  512 },
	{ "k", 1024 },
	{ "B", 1024 },
	{ "", 0 }
};

अटल स्थिर काष्ठा mode_info *find_mode(स्थिर अक्षर *name)
{
	पूर्णांक i = सूचकांक_in_strings(mode_name, name);
	वापस i >= 0 ? &mode_info[i] : शून्य;
}

अटल स्थिर काष्ठा control_info *find_control(स्थिर अक्षर *name)
{
	पूर्णांक i = सूचकांक_in_strings(control_name, name);
	वापस i >= 0 ? &control_info[i] : शून्य;
}

क्रमागत {
	param_need_arg = 0x80,
	param_पंक्ति    = 1 | 0x80,
	param_rows    = 2 | 0x80,
	param_cols    = 3 | 0x80,
	param_columns = 4 | 0x80,
	param_आकार    = 5,
	param_speed   = 6,
	param_ispeed  = 7 | 0x80,
	param_ospeed  = 8 | 0x80,
};

अटल पूर्णांक find_param(स्थिर अक्षर *name)
{
	अटल स्थिर अक्षर params[] ALIGN1 =
		"line\0"    /* 1 */
		"rows\0"    /* 2 */
		"cols\0"    /* 3 */
		"columns\0" /* 4 */
		"size\0"    /* 5 */
		"speed\0"   /* 6 */
		"ispeed\0"
		"ospeed\0";
	पूर्णांक i = सूचकांक_in_strings(params, name) + 1;
	अगर (i == 0)
		वापस 0;
	अगर (i != 5 && i != 6)
		i |= 0x80;
	वापस i;
}

अटल पूर्णांक recover_mode(स्थिर अक्षर *arg, काष्ठा termios *mode)
{
	पूर्णांक i, n;
	अचिन्हित chr;
	अचिन्हित दीर्घ iflag, oflag, cflag, lflag;

	/* Scan into temporaries since it is too much trouble to figure out
	   the right format for 'tcflag_t' */
	अगर (माला_पूछो(arg, "%lx:%lx:%lx:%lx%n",
			   &iflag, &oflag, &cflag, &lflag, &n) != 4)
		वापस 0;
	mode->c_iflag = iflag;
	mode->c_oflag = oflag;
	mode->c_cflag = cflag;
	mode->c_lflag = lflag;
	arg += n;
	क्रम (i = 0; i < NCCS; ++i) {
		अगर (माला_पूछो(arg, ":%x%n", &chr, &n) != 1)
			वापस 0;
		mode->c_cc[i] = chr;
		arg += n;
	}

	/* Fail if there are too many fields */
	अगर (*arg != '\0')
		वापस 0;

	वापस 1;
}

अटल व्योम display_recoverable(स्थिर काष्ठा termios *mode,
				पूर्णांक UNUSED_PARAM dummy)
{
	पूर्णांक i;
	म_लिखो("%lx:%lx:%lx:%lx",
		   (अचिन्हित दीर्घ) mode->c_iflag, (अचिन्हित दीर्घ) mode->c_oflag,
		   (अचिन्हित दीर्घ) mode->c_cflag, (अचिन्हित दीर्घ) mode->c_lflag);
	क्रम (i = 0; i < NCCS; ++i)
		म_लिखो(":%x", (अचिन्हित पूर्णांक) mode->c_cc[i]);
	bb_putchar('\n');
}

अटल व्योम display_speed(स्थिर काष्ठा termios *mode, पूर्णांक fancy)
{
	//____________________ 01234567 8 9
	स्थिर अक्षर *fmt_str = "%lu %lu\n\0ispeed %lu baud; ospeed %lu baud;";
	अचिन्हित दीर्घ ispeed, ospeed;

	ispeed = cfgetispeed(mode);
	ospeed = cfgetospeed(mode);
	अगर (ispeed == 0 || ispeed == ospeed) {
		ispeed = ospeed;                /* in case ispeed was 0 */
		//________ 0123 4 5 6 7 8 9
		fmt_str = "%lu\n\0\0\0\0\0speed %lu baud;";
	}
	अगर (fancy) fmt_str += 9;
	wrapf(fmt_str, tty_baud_to_मूल्य(ispeed), tty_baud_to_मूल्य(ospeed));
}

अटल व्योम do_display(स्थिर काष्ठा termios *mode, पूर्णांक all)
{
	पूर्णांक i;
	tcflag_t *bitsp;
	अचिन्हित दीर्घ mask;
	पूर्णांक पूर्व_type = control;

	display_speed(mode, 1);
	अगर (all)
		display_window_आकार(1);
#अगर_घोषित __linux__
	wrapf("line = %u;\n", mode->c_पंक्ति);
#अन्यथा
	newline();
#पूर्ण_अगर

	क्रम (i = 0; i != CIDX_min; ++i) {
		अक्षर ch;
		अक्षर buf10[10];

		/* If swtch is the same as susp, don't print both */
#अगर VSWTCH == VSUSP
		अगर (i == CIDX_swtch)
			जारी;
#पूर्ण_अगर
		/* If eof uses the same slot as min, only print whichever applies */
#अगर VEOF == VMIN
		अगर (!(mode->c_lflag & ICANON)
		 && (i == CIDX_eof || i == CIDX_eol)
		) {
			जारी;
		}
#पूर्ण_अगर
		ch = mode->c_cc[control_info[i].offset];
		अगर (ch == _POSIX_VDISABLE)
			म_नकल(buf10, "<undef>");
		अन्यथा
			visible(ch, buf10, 0);
		wrapf("%s = %s;", nth_string(control_name, i), buf10);
	}
#अगर VEOF == VMIN
	अगर ((mode->c_lflag & ICANON) == 0)
#पूर्ण_अगर
		wrapf("min = %u; time = %u;", mode->c_cc[VMIN], mode->c_cc[VTIME]);
	newline();

	क्रम (i = 0; i < अंक_mode_info; ++i) {
		अगर (mode_info[i].flags & OMIT)
			जारी;
		अगर (mode_info[i].type != पूर्व_type) {
			newline();
			पूर्व_type = mode_info[i].type;
		}

		bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);
		mask = mode_info[i].mask ? mode_info[i].mask : mode_info[i].bits;
		अगर ((*bitsp & mask) == mode_info[i].bits) {
			अगर (all || (mode_info[i].flags & SANE_UNSET))
				wrapf("-%s"+1, nth_string(mode_name, i));
		} अन्यथा {
			अगर ((all && mode_info[i].flags & REV)
			 || (!all && (mode_info[i].flags & (SANE_SET | REV)) == (SANE_SET | REV))
			) {
				wrapf("-%s", nth_string(mode_name, i));
			}
		}
	}
	newline();
}

अटल व्योम sane_mode(काष्ठा termios *mode)
{
	पूर्णांक i;

	क्रम (i = 0; i < अंक_control_info; ++i) {
#अगर VMIN == VEOF
		अगर (i == CIDX_min)
			अवरोध;
#पूर्ण_अगर
		mode->c_cc[control_info[i].offset] = control_info[i].saneval;
	}

	क्रम (i = 0; i < अंक_mode_info; ++i) {
		tcflag_t val;
		tcflag_t *bitsp = get_ptr_to_tcflag(mode_info[i].type, mode);

		अगर (!bitsp)
			जारी;
		val = *bitsp & ~((अचिन्हित दीर्घ)mode_info[i].mask);
		अगर (mode_info[i].flags & SANE_SET) {
			*bitsp = val | mode_info[i].bits;
		} अन्यथा
		अगर (mode_info[i].flags & SANE_UNSET) {
			*bitsp = val & ~mode_info[i].bits;
		}
	}
}

अटल व्योम set_mode(स्थिर काष्ठा mode_info *info, पूर्णांक reversed,
					काष्ठा termios *mode)
{
	tcflag_t *bitsp;

	bitsp = get_ptr_to_tcflag(info->type, mode);

	अगर (bitsp) {
		tcflag_t val = *bitsp & ~info->mask;
		अगर (reversed)
			*bitsp = val & ~info->bits;
		अन्यथा
			*bitsp = val | info->bits;
		वापस;
	}

	/* !bitsp - it's a "combination" mode */
	अगर (info == &mode_info[IDX_evenp] || info == &mode_info[IDX_parity]) {
		अगर (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~PARODD & ~CSIZE) | PARENB | CS7;
	} अन्यथा अगर (info == &mode_info[IDX_oddp]) {
		अगर (reversed)
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
		अन्यथा
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARODD | PARENB;
	} अन्यथा अगर (info == &mode_info[IDX_nl]) {
		अगर (reversed) {
			mode->c_iflag = (mode->c_iflag | ICRNL) & ~INLCR & ~IGNCR;
			mode->c_oflag = (mode->c_oflag | ONLCR) & ~OCRNL & ~ONLRET;
		} अन्यथा {
			mode->c_iflag = mode->c_iflag & ~ICRNL;
			अगर (ONLCR) mode->c_oflag = mode->c_oflag & ~ONLCR;
		}
	} अन्यथा अगर (info == &mode_info[IDX_ek]) {
		mode->c_cc[VERASE] = CERASE;
		mode->c_cc[VKILL] = CKILL;
	} अन्यथा अगर (info == &mode_info[IDX_sane]) {
		sane_mode(mode);
	} अन्यथा अगर (info == &mode_info[IDX_cbreak]) {
		अगर (reversed)
			mode->c_lflag |= ICANON;
		अन्यथा
			mode->c_lflag &= ~ICANON;
	} अन्यथा अगर (info == &mode_info[IDX_pass8]) {
		अगर (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
		}
	} अन्यथा अगर (info == &mode_info[IDX_litout]) {
		अगर (reversed) {
			mode->c_cflag = (mode->c_cflag & ~CSIZE) | CS7 | PARENB;
			mode->c_iflag |= ISTRIP;
			mode->c_oflag |= OPOST;
		} अन्यथा {
			mode->c_cflag = (mode->c_cflag & ~PARENB & ~CSIZE) | CS8;
			mode->c_iflag &= ~ISTRIP;
			mode->c_oflag &= ~OPOST;
		}
	} अन्यथा अगर (info == &mode_info[IDX_raw] || info == &mode_info[IDX_cooked]) {
		अगर ((info == &mode_info[IDX_raw] && reversed)
		 || (info == &mode_info[IDX_cooked] && !reversed)
		) {
			/* Cooked mode */
			mode->c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
			mode->c_oflag |= OPOST;
			mode->c_lflag |= ISIG | ICANON;
#अगर VMIN == VEOF
			mode->c_cc[VEOF] = CEOF;
#पूर्ण_अगर
#अगर VTIME == VEOL
			mode->c_cc[VEOL] = CEOL;
#पूर्ण_अगर
		} अन्यथा {
			/* Raw mode */
			mode->c_iflag = 0;
			mode->c_oflag &= ~OPOST;
			mode->c_lflag &= ~(ISIG | ICANON | XCASE);
			mode->c_cc[VMIN] = 1;
			mode->c_cc[VTIME] = 0;
		}
	}
#अगर IXANY
	अन्यथा अगर (info == &mode_info[IDX_decctlq]) {
		अगर (reversed)
			mode->c_iflag |= IXANY;
		अन्यथा
			mode->c_iflag &= ~IXANY;
	}
#पूर्ण_अगर
#अगर TABDLY
	अन्यथा अगर (info == &mode_info[IDX_tabs]) {
		अगर (reversed)
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB3;
		अन्यथा
			mode->c_oflag = (mode->c_oflag & ~TABDLY) | TAB0;
	}
#पूर्ण_अगर
#अगर OXTABS
	अन्यथा अगर (info == &mode_info[IDX_tabs]) {
		अगर (reversed)
			mode->c_oflag |= OXTABS;
		अन्यथा
			mode->c_oflag &= ~OXTABS;
	}
#पूर्ण_अगर
#अगर XCASE && IUCLC && OLCUC
	अन्यथा अगर (info==&mode_info[IDX_lcase] || info==&mode_info[IDX_LCASE]) {
		अगर (reversed) {
			mode->c_lflag &= ~XCASE;
			mode->c_iflag &= ~IUCLC;
			mode->c_oflag &= ~OLCUC;
		} अन्यथा {
			mode->c_lflag |= XCASE;
			mode->c_iflag |= IUCLC;
			mode->c_oflag |= OLCUC;
		}
	}
#पूर्ण_अगर
	अन्यथा अगर (info == &mode_info[IDX_crt]) {
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
	} अन्यथा अगर (info == &mode_info[IDX_dec]) {
		mode->c_cc[VINTR] = 3; /* ^C */
		mode->c_cc[VERASE] = 127; /* DEL */
		mode->c_cc[VKILL] = 21; /* ^U */
		mode->c_lflag |= ECHOE | ECHOCTL | ECHOKE;
		अगर (IXANY) mode->c_iflag &= ~IXANY;
	}
}

अटल व्योम set_control_char_or_die(स्थिर काष्ठा control_info *info,
			स्थिर अक्षर *arg, काष्ठा termios *mode)
{
	अचिन्हित अक्षर मूल्य_2;

	अगर (info == &control_info[CIDX_min] || info == &control_info[CIDX_time])
		मूल्य_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	अन्यथा अगर (arg[0] == '\0' || arg[1] == '\0')
		मूल्य_2 = arg[0];
	अन्यथा अगर (म_भेद(arg, "^-") == 0 || म_भेद(arg, "undef") == 0)
		मूल्य_2 = _POSIX_VDISABLE;
	अन्यथा अगर (arg[0] == '^') { /* Ignore any trailing junk (^Cjunk) */
		मूल्य_2 = arg[1] & 0x1f; /* Non-letters get weird results */
		अगर (arg[1] == '?')
			मूल्य_2 = 127;
	} अन्यथा
		मूल्य_2 = xatoul_range_sfx(arg, 0, 0xff, stty_suffixes);
	mode->c_cc[info->offset] = मूल्य_2;
}

#घोषणा STTY_require_set_attr   (1 << 0)
#घोषणा STTY_speed_was_set      (1 << 1)
#घोषणा STTY_verbose_output     (1 << 2)
#घोषणा STTY_recoverable_output (1 << 3)
#घोषणा STTY_noargs             (1 << 4)

पूर्णांक stty_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक stty_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	काष्ठा termios mode;
	व्योम (*output_func)(स्थिर काष्ठा termios *, पूर्णांक);
	स्थिर अक्षर *file_name = शून्य;
	पूर्णांक display_all = 0;
	पूर्णांक stty_state;
	पूर्णांक k;

	INIT_G();

	stty_state = STTY_noargs;
	output_func = do_display;

	/* First pass: only parse/verify command line params */
	k = 0;
	जबतक (argv[++k]) {
		स्थिर काष्ठा mode_info *mp;
		स्थिर काष्ठा control_info *cp;
		स्थिर अक्षर *arg = argv[k];
		स्थिर अक्षर *argnext = argv[k+1];
		पूर्णांक param;

		अगर (arg[0] == '-') {
			पूर्णांक i;
			mp = find_mode(arg+1);
			अगर (mp) {
				अगर (!(mp->flags & REV))
					जाओ invalid_argument;
				stty_state &= ~STTY_noargs;
				जारी;
			}
			/* It is an option - parse it */
			i = 0;
			जबतक (arg[++i]) {
				चयन (arg[i]) {
				हाल 'a':
					stty_state |= STTY_verbose_output;
					output_func = do_display;
					display_all = 1;
					अवरोध;
				हाल 'g':
					stty_state |= STTY_recoverable_output;
					output_func = display_recoverable;
					अवरोध;
				हाल 'F':
					अगर (file_name)
						bb_simple_error_संदेश_and_die("only one device may be specified");
					file_name = &arg[i+1]; /* "-Fdevice" ? */
					अगर (!file_name[0]) { /* nope, "-F device" */
						पूर्णांक p = k+1; /* argv[p] is argnext */
						file_name = argnext;
						अगर (!file_name)
							bb_error_संदेश_and_die(bb_संदेश_requires_arg, "-F");
						/* remove -F param from arg[vc] */
						जबतक (argv[p]) {
							argv[p] = argv[p+1];
							++p;
						}
					}
					जाओ end_option;
				शेष:
					जाओ invalid_argument;
				}
			}
 end_option:
			जारी;
		}

		mp = find_mode(arg);
		अगर (mp) {
			stty_state &= ~STTY_noargs;
			जारी;
		}

		cp = find_control(arg);
		अगर (cp) {
			अगर (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			/* called for the side effect of xfunc death only */
			set_control_char_or_die(cp, argnext, &mode);
			stty_state &= ~STTY_noargs;
			++k;
			जारी;
		}

		param = find_param(arg);
		अगर (param & param_need_arg) {
			अगर (!argnext)
				bb_error_संदेश_and_die(bb_संदेश_requires_arg, arg);
			++k;
		}

		चयन (param) {
#अगर_घोषित __linux__
		हाल param_पंक्ति:
# ifndef TIOCGWINSZ
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			अवरोध;
# endif /* else fall-through */
#पूर्ण_अगर
#अगर_घोषित TIOCGWINSZ
		हाल param_rows:
		हाल param_cols:
		हाल param_columns:
			xatoul_range_sfx(argnext, 1, पूर्णांक_उच्च, stty_suffixes);
			अवरोध;
		हाल param_आकार:
#पूर्ण_अगर
		हाल param_speed:
			अवरोध;
		हाल param_ispeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(निवेश_speed, argnext, &mode);
			अवरोध;
		हाल param_ospeed:
			/* called for the side effect of xfunc death only */
			set_speed_or_die(output_speed, argnext, &mode);
			अवरोध;
		शेष:
			अगर (recover_mode(arg, &mode) == 1) अवरोध;
			अगर (tty_मूल्य_to_baud(xatou(arg)) != (speed_t) -1) अवरोध;
 invalid_argument:
			bb_error_संदेश_and_die("invalid argument '%s'", arg);
		}
		stty_state &= ~STTY_noargs;
	}

	/* Specifying both -a and -g is an error */
	अगर ((stty_state & (STTY_verbose_output | STTY_recoverable_output)) ==
		(STTY_verbose_output | STTY_recoverable_output)
	) {
		bb_simple_error_संदेश_and_die("-a and -g are mutually exclusive");
	}
	/* Specifying -a or -g with non-options is an error */
	अगर ((stty_state & (STTY_verbose_output | STTY_recoverable_output))
	 && !(stty_state & STTY_noargs)
	) {
		bb_simple_error_संदेश_and_die("modes may not be set when -a or -g is used");
	}

	/* Now it is safe to start doing things */
	अगर (file_name) {
		G.device_name = file_name;
		xmove_fd(xopen_nonblocking(G.device_name), STDIN_FILENO);
		ndelay_off(STDIN_FILENO);
	}

	/* Initialize to all zeroes so there is no risk memcmp will report a
	   spurious difference in an uninitialized portion of the structure */
	स_रखो(&mode, 0, माप(mode));
	अगर (tcgetattr(STDIN_FILENO, &mode))
		perror_on_device_and_die("%s");

	अगर (stty_state & (STTY_verbose_output | STTY_recoverable_output | STTY_noargs)) {
		G.max_col = get_terminal_width(STDOUT_FILENO);
		output_func(&mode, display_all);
		वापस निकास_सफल;
	}

	/* Second pass: perform actions */
	k = 0;
	जबतक (argv[++k]) {
		स्थिर काष्ठा mode_info *mp;
		स्थिर काष्ठा control_info *cp;
		स्थिर अक्षर *arg = argv[k];
		स्थिर अक्षर *argnext = argv[k+1];
		पूर्णांक param;

		अगर (arg[0] == '-') {
			mp = find_mode(arg+1);
			अगर (mp) {
				set_mode(mp, 1 /* reversed */, &mode);
				stty_state |= STTY_require_set_attr;
			}
			/* It is an option - already parsed. Skip it */
			जारी;
		}

		mp = find_mode(arg);
		अगर (mp) {
			set_mode(mp, 0 /* non-reversed */, &mode);
			stty_state |= STTY_require_set_attr;
			जारी;
		}

		cp = find_control(arg);
		अगर (cp) {
			++k;
			set_control_char_or_die(cp, argnext, &mode);
			stty_state |= STTY_require_set_attr;
			जारी;
		}

		param = find_param(arg);
		अगर (param & param_need_arg) {
			++k;
		}

		चयन (param) {
#अगर_घोषित __linux__
		हाल param_पंक्ति:
			mode.c_पंक्ति = xatoul_sfx(argnext, stty_suffixes);
			stty_state |= STTY_require_set_attr;
			अवरोध;
#पूर्ण_अगर
#अगर_घोषित TIOCGWINSZ
		हाल param_cols:
		हाल param_columns:
			set_window_आकार(-1, xatoul_sfx(argnext, stty_suffixes));
			अवरोध;
		हाल param_आकार:
			display_window_आकार(0);
			अवरोध;
		हाल param_rows:
			set_window_आकार(xatoul_sfx(argnext, stty_suffixes), -1);
			अवरोध;
#पूर्ण_अगर
		हाल param_speed:
			display_speed(&mode, 0);
			अवरोध;
		हाल param_ispeed:
			set_speed_or_die(निवेश_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			अवरोध;
		हाल param_ospeed:
			set_speed_or_die(output_speed, argnext, &mode);
			stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			अवरोध;
		शेष:
			अगर (recover_mode(arg, &mode) == 1)
				stty_state |= STTY_require_set_attr;
			अन्यथा /* true: if (tty_value_to_baud(xatou(arg)) != (speed_t) -1) */{
				set_speed_or_die(both_speeds, arg, &mode);
				stty_state |= (STTY_require_set_attr | STTY_speed_was_set);
			} /* else - impossible (caught in the first pass):
				bb_error_msg_and_die("invalid argument '%s'", arg); */
		}
	}

	अगर (stty_state & STTY_require_set_attr) {
		काष्ठा termios new_mode;

		अगर (tcsetattr(STDIN_FILENO, TCSADRAIN, &mode))
			perror_on_device_and_die("%s");

		/* POSIX (according to Zlotnick's book) tcsetattr returns zero if
		   it performs *any* of the requested operations.  This means it
		   can report 'success' when it has actually failed to perform
		   some proper subset of the requested operations.  To detect
		   this partial failure, get the current terminal attributes and
		   compare them to the requested ones */

		/* Initialize to all zeroes so there is no risk memcmp will report a
		   spurious difference in an uninitialized portion of the structure */
		स_रखो(&new_mode, 0, माप(new_mode));
		अगर (tcgetattr(STDIN_FILENO, &new_mode))
			perror_on_device_and_die("%s");

		अगर (स_भेद(&mode, &new_mode, माप(mode)) != 0) {
/*
 * I think the below chunk is not necessary on Linux.
 * If you are deleting it, also delete STTY_speed_was_set bit -
 * it is only ever checked here.
 */
#अगर 0 /* was "if CIBAUD" */
			/* SunOS 4.1.3 (at least) has the problem that after this sequence,
			   tcgetattr (&m1); tcsetattr (&m1); tcgetattr (&m2);
			   sometimes (m1 != m2).  The only difference is in the four bits
			   of the c_cflag field corresponding to the baud rate.  To save
			   Sun users a little confusion, don't report an error if this
			   happens.  But suppress the error only if we haven't tried to
			   set the baud rate explicitly -- otherwise we'd never give an
			   error for a true failure to set the baud rate */

			new_mode.c_cflag &= (~CIBAUD);
			अगर ((stty_state & STTY_speed_was_set)
			 || स_भेद(&mode, &new_mode, माप(mode)) != 0)
#पूर्ण_अगर
				perror_on_device_and_die("%s: cannot perform all requested operations");
		}
	}

	वापस निकास_सफल;
}
