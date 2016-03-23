/* $Id: imc-mercs.h,v 1.6 2000/02/23 01:36:52 deaz Exp $ */

#ifndef IMC_MERCS_H
#define IMC_MERCS_H

/* Stick your mud ad in this #define. */
#define IMC_MUD_INFO "No MUD information has been defined.\n\r"

/* mud type configuration.
 * Either uncomment one of these #defines, or add an appropriate
 * -DROM / -DMERC / etc flag to your Makefile
 */

//#define ROM
//#define ROT
//#define MERC
//#define SMAUG /* 102 users */
//#define SMAUG14
//#define ENVY  
//#define ACK   
//#define CIRCLE
//#define HUB
//#define CRBS
#define RESORTMUD

// Define a MUD_TITLE based off what they chose above.
#if defined(ROM)
#define MUD_TITLE "rom"
#elif defined(ROT)
#define MUD_TITLE "rot"
#elif defined(MERC)
#define MUD_TITLE "merc"
#elif defined(SMAUG)
#define MUD_TITLE "RM4 Beta"
#elif defined(SMAUG14)
#define MUD_TITLE "RM4 Beta"
#elif defined(ENVY)
#define MUD_TITLE "envy"
#elif defined(ACK)
#define MUD_TITLE "ack"
#elif defined(CIRCLE)
#define MUD_TITLE "circle"
#elif defined(CRBS)
#define MUD_TITLE "CRBS"
#elif defined(RESORTMUD)
#define MUD_TITLE "RM4.0 Beta"
#else
#define MUD_TITLE "hub"
//#define MUD_TITLE "webcgi"
#endif

#ifdef CRBS
#ifndef SMAUG14
#define SMAUG14
#endif
#endif

#ifdef RESORTMUD
#ifndef SMAUG14
#define SMAUG14
#endif
#endif

#ifdef SMAUG14
#ifndef SMAUG
#define SMAUG
#endif
#endif

#ifdef ROT
#ifndef ROM
#define ROM 
#endif
#endif

#endif
