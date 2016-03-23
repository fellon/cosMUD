/*
 * Copyright (c) 2000 Fatal Dimensions
 * 
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * 
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code Copyright (c)2001 Alsherok.
 *
 * I condensed the 14 or so Fatal Dimensions source code files into this
 * one file, because I for one find it far easier to maintain when all of
 * the functions are right here in one file. I've also renamed everything
 * using I3 in order to further distinguish this from the IMC
 * code standard in stock Smaug and ACK muds.
 */
#ifdef I3

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#ifdef I3SMAUG
   #include "mud.h"
#endif
#ifdef I3ROM
   #include "merc.h"
   #include "tables.h"
#endif
#ifdef I3ACK
   #include "ack.h"
#endif
#ifdef I3MERC
   #include "merc.h"
#endif
#ifdef I3UENVY
   #include "merc.h"
#endif
#ifdef I3CIRCLE
   #include "conf.h"
   #include "sysdep.h"
   #include "structs.h"
   #include "utils.h"
   #include "db.h"
   #include "comm.h"
   #include "interpreter.h"
   #ifdef HAVE_ARPA_TELNET_H
      #include <arpa/telnet.h>
   #else
      #include "telnet.h"
   #endif
#endif

/* If you don't have my custom color code, this will change the color for I3 channels to your normal chat color 
 * Of course, if your using the Smaug 1.4a colorize code, you can add the AT_INTERMUD define in the approrpiate
 * places for that and set the color to your preference and then remove the #ifndef here.
 */
#ifndef SAMSONCOLOR
   #define AT_INTERMUD AT_GOSSIP
#endif

#ifdef I3CIRCLE
ACMD( do_i3 );
ACMD( do_i3who );
ACMD( do_i3finger );
ACMD( do_i3locate );
ACMD( do_i3tell );
ACMD( do_i3reply );
ACMD( do_i3emote );
ACMD( do_i3beep );
extern DESCRIPTOR_DATA *descriptor_list; /* Bleh, Circle doesn't just put these in an H file somewhere?? */
extern CHAR_DATA *character_list;
extern const char *pc_class_types[];
extern char		*class_abbrevs	[];
char			log_buf		[MSL];
int find_action( int cmd );
#endif

/* Global variables for I3 */
char cbuf[MSL];		  /* General purpose global buffer - used mostly in color conversions */
char debugstring[MSL];    /* Sole purpose of this is to gather up pieces of a packet for debugging */
bool packetdebug = FALSE; /* Packet debugging toggle, can be turned on to check outgoing packets */
int i3wait; 		  /* Number of game loops to ait before attempting to reconnect when a socket dies */
int reconattempts; 	  /* Number of attempts to reconnect that have been made */
time_t ucache_clock;	  /* Timer for pruning the ucache */

I3_MUD *this_mud;
I3_MUD *first_mud;
I3_MUD *last_mud;

I3_CHANNEL *first_I3chan;
I3_CHANNEL *last_I3chan;

I3_LISTENER *first_listener;
I3_LISTENER *last_listener;

UCACHE_DATA *first_ucache;
UCACHE_DATA *last_ucache;

char I3_input_buffer[256*256];
char I3_output_buffer[256*256];
long I3_input_pointer = 0;
long I3_output_pointer = 4;
char I3_currentpacket[256*256];
char I3_THISMUD[256];
char I3_ROUTER_NAME[256];
int  I3_socket = 0;
I3_STATS I3_stats;

#ifdef I3CIRCLE
/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	log( "I3 Strn_cmp: null astr." );
	return TRUE;
    }

    if ( !bstr )
    {
	log( "I3 Strn_cmp: null bstr." );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}
#endif

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes. do not mess with case.
 * as opposed to native one_argument which ignores case.
 */
char *i3one_argument( char *argument, char *arg_first )
{
   char cEnd;
   int count;

   count = 0;

   while ( isspace(*argument) )
	argument++;

   cEnd = ' ';
   if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

   while ( *argument != '\0' || ++count >= 255 )
   {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = (*argument);
	arg_first++;
	argument++;
   }
   *arg_first = '\0';

   while ( isspace(*argument) )
	argument++;

   return argument;
}

/* Generic log function which will route the log messages to the appropriate system logging function */
void i3log( char *string )
{
#ifdef I3CIRCLE
   log( string );
#else
   log_string( string );
#endif
   return;
}

/* Generic bug logging function which will route the message to the appropriate function that handles bug logs */
void i3bug( char *string )
{
#ifdef I3CIRCLE
   sprintf( log_buf, "SYSERR: %s", string );
   log( log_buf );
#else
   bug( string, 0 );
#endif
   return;
}

void escape_smaug_color( char *buffer, const char *txt )
{
    const char *point;

    for( point = txt ; *point ; point++ )
    {
        *buffer = *point;
        if( *point == '&' )
            *++buffer = '&';
        *++buffer = '\0';
    }
    *buffer = '\0';
    return;
}

/* You need to change the &, } and { tokens in the table below, and in the if statement
 * in i3_tagtofish to match what your mud uses to identify a color token with.
 *
 * & is the foreground text token.
 * } is the blink text token.
 * { is the background text token.
 */
#define I3MAX_ANSI 47
const char *i3ansi_conversion[I3MAX_ANSI][3] =
{
	/* Foreground Standard Colors */
	{ "&x", "%^BLACK%^",   "\e[0;0;30m" }, // Black
	{ "&r", "%^RED%^",     "\e[0;0;31m" }, // Dark Red
	{ "&g", "%^GREEN%^",   "\e[0;0;32m" }, // Dark Green
	{ "&O", "%^ORANGE%^",  "\e[0;0;33m" }, // Orange/Brown
	{ "&b", "%^BLUE%^",    "\e[0;0;34m" }, // Dark Blue
	{ "&p", "%^MAGENTA%^", "\e[0;0;35m" }, // Purple/Magenta
	{ "&c", "%^CYAN%^",    "\e[0;0;36m" }, // Cyan
	{ "&w", "%^WHITE%^",   "\e[0;0;37m" }, // Grey

	/* Background colors */
	{ "{x", "%^B_BLACK%^",   "\e[40m" }, // Black
	{ "{r", "%^B_RED%^",     "\e[41m" }, // Red
	{ "{g", "%^B_GREEN%^",   "\e[42m" }, // Green
	{ "{O", "%^B_ORANGE%^",  "\e[43m" }, // Orange
	{ "{B", "%^B_BLUE%^",    "\e[44m" }, // Blue
	{ "{p", "%^B_MAGENTA%^", "\e[45m" }, // Purple/Magenta
	{ "{c", "%^B_CYAN%^",    "\e[46m" }, // Cyan
	{ "{w", "%^B_WHITE%^",   "\e[47m" }, // White

	/* Text Affects */
	{ "&d", "%^RESET%^",     "\e[0m" }, // Reset Text
        { "&D", "%^RESET%^",     "\e[0m" }, // Reset Text
	{ "&L", "%^BOLD%^",      "\e[1m" }, // Bolden Text(Brightens it)
	{ "&u", "%^UNDERLINE%^", "\e[4m" }, // Underline Text
	{ "&$", "%^FLASH%^",     "\e[5m" }, // Blink Text
	{ "&i", "%^ITALIC%^",    "\e[6m" }, // Italic Text
	{ "&v", "%^REVERSE%^",   "\e[7m" }, // Reverse Background and Foreground Colors

	/* Foreground extended colors */
	{ "&z", "%^BLACK%^%^BOLD%^",   "\e[0;1;30m" }, // Dark Grey
	{ "&R", "%^RED%^%^BOLD%^",     "\e[0;1;31m" }, // Red
	{ "&G", "%^GREEN%^%^BOLD%^",   "\e[0;1;32m" }, // Green
	{ "&Y", "%^YELLOW%^",          "\e[0;1;33m" }, // Yellow
	{ "&B", "%^BLUE%^%^BOLD%^",    "\e[0;1;34m" }, // Blue
	{ "&C", "%^MAGENTA%^%^BOLD%^", "\e[0;1;35m" }, // Pink
	{ "&P", "%^CYAN%^%^BOLD%^",    "\e[0;1;36m" }, // Light Blue
	{ "&W", "%^WHITE%^%^BOLD%^",   "\e[0;1;37m" }, // White

	/* Blinking foreground standard color */
	{ "}x", "%^BLACK%^%^FLASH%^",           "\e[0;5;30m" }, // Black
	{ "}r", "%^RED%^%^FLASH%^",             "\e[0;5;31m" }, // Dark Red
	{ "}g", "%^GREEN%^%^FLASH%^",           "\e[0;5;32m" }, // Dark Green
	{ "}O", "%^ORANGE%^%^FLASH%^",          "\e[0;5;33m" }, // Orange/Brown
	{ "}b", "%^BLUE%^%^FLASH%^",            "\e[0;5;34m" }, // Dark Blue
	{ "}p", "%^MAGENTA%^%^FLASH%^",         "\e[0;5;35m" }, // Magenta/Purple
	{ "}c", "%^CYAN%^%^FLASH%^",            "\e[0;5;36m" }, // Cyan
	{ "}w", "%^WHITE%^%^FLASH%^",           "\e[0;5;37m" }, // Grey
	{ "}z", "%^BLACK%^%^BOLD%^%^FLASH%^",   "\e[1;5;30m" }, // Dark Grey
	{ "}R", "%^RED%^%^BOLD%^%^FLASH%^",     "\e[1;5;31m" }, // Red
	{ "}G", "%^GREEN%^%^BOLD%^%^FLASH%^",   "\e[1;5;32m" }, // Green
	{ "}Y", "%^YELLOW%^%^FLASH%^",          "\e[1;5;33m" }, // Yellow
	{ "}B", "%^BLUE%^%^BOLD%^%^FLASH%^",    "\e[1;5;34m" }, // Blue
	{ "}C", "%^MAGENTA%^%^BOLD%^%^FLASH%^", "\e[1;5;35m" }, // Pink
	{ "}P", "%^CYAN%^%^BOLD%^%^FLASH%^",    "\e[1;5;36m" }, // Light Blue
	{ "}W", "%^WHITE%^%^BOLD%^%^FLASH%^",   "\e[1;5;37m" }  // White
};

char *I3_tagtofish( const char *txt )
{
   int c, x, count = 0;
   static char tbuf[MSL];
   char code[3];

   if( !txt || *txt == '\0' )
	return "";

   tbuf[0] = '\0';

   for( count = 0; count < MSL; count++ )
   {	
	if( *txt == '\0' )
	   break;

	if( *txt != '&' && *txt != '{' && *txt != '}' )
	{
	   tbuf[count] = *txt;
	   txt++;
	   continue;
	}

      code[0] = *txt;
	txt++;
		
	if( *txt == code[0] )
	{
	   tbuf[count] = *txt;
	   txt++;
	   continue;
	}

	code[1] = *txt;
      code[2] = '\0';

	for( c = 0; c < I3MAX_ANSI; c++ )
	{
	   if( i3ansi_conversion[c][0][0] == code[0] && i3ansi_conversion[c][0][1] == code[1] )
	   {
	      for( x = 0; i3ansi_conversion[c][1][x]; x++ )
		{
		   tbuf[count] = i3ansi_conversion[c][1][x];
		   count++;
		}
		break;
	   }
	}
	count--;
	txt++;
   }

   tbuf[count] = '\0';

   return tbuf;
}

/* Takes the string you pass it and converts its Pinkfish color tags into ANSI codes */
char *I3_fishtoansi( const char *inbuf )
{
   char *cp, *cp2;
   char col[30];
   static char abuf[MSL];
   int len;
   bool found = FALSE;

   /* catch the trivial case first (for speed) */
   cp = strstr( inbuf, "%^" );
   if( !cp )
   {
      strcpy( abuf, inbuf );
	return abuf;
   }

   abuf[0] = '\0';
   col[0] = '\0';

   do
   {
      cp2 = strstr( cp+2, "%^" );
      if (!cp2) break; /* unmatched single %^ */

      /* ok, we have 2 delimiters now.
       * get the converted color and its length */

      len = cp2 - cp + 2;
    
      if( len == 4 )
      { /* means "%^%^" which is the escape */
         len = 2;
         strcpy( col, "%^" );
      }
	else
	{
	   int c;

         for( c = 0; i3ansi_conversion[c][1]; c++ )
         {
	      if( !strncmp( cp, i3ansi_conversion[c][1], len ) )
	      {
	         strcpy( col, i3ansi_conversion[c][2] );
	         len = strlen( col );
		   found = TRUE;
		   break;
	      }
         }
	}

	if( !found )
	   strcpy( col, cp );

      /* copy the first part into the buffer and add the converted color code */

      strncat( abuf, inbuf, cp-inbuf );
      strncat( abuf, col, len );
      inbuf = cp2+2;

   } while( (cp = strstr( inbuf, "%^" )) );

   /* copy the rest */
   strcat( abuf, inbuf );

   return abuf;
}

/* Generic substitute for write_to_buffer since not all codebases seem to have it */
void to_char_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
#ifdef I3CIRCLE
   SEND_TO_Q( txt, d );
#else
   write_to_buffer( d, txt, 0 );
#endif
   return;
}

/* Modified version of Smaug's send_to_char_color function */
void i3_to_char( const char *txt, CHAR_DATA *ch )
{
   char buf[MSL];

   if( !ch )
   {
	i3bug( "i3_to_char: NULL ch!" );
	return;
   }

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
	sprintf( log_buf, "i3_to_char: NULL descriptor for %s!", CH_NAME(ch) );
	i3bug( log_buf );
	return;
   }

   sprintf( buf, "%s", I3_tagtofish( txt ) );
   sprintf( cbuf, "%s", I3_fishtoansi( buf ) );
   to_char_buffer( ch->desc, cbuf, 0 );
   to_char_buffer( ch->desc, "\e[0m", 0 ); /* Reset color to stop bleeding */
   return;
}

/* Modified version of Smaug's ch_printf_color function */
void i3_printf( CHAR_DATA *ch, char *fmt, ... )
{
     char buf[MSL*2];
     va_list args;
 
     va_start( args, fmt );
     vsprintf( buf, fmt, args );
     va_end( args );
 
     i3_to_char( buf, ch );
}

/* Generic send_to_pager type function to send to the proper code for each codebase */
void i3send_to_pager( const char *txt, CHAR_DATA *ch )
{
#ifdef I3SMAUG
   send_to_pager_color( txt, ch );
#elif defined(I3ROM)
   page_to_char( txt, ch );
#else
   i3_to_char( txt, ch );
#endif
   return;
}

/* Generic pager_printf type function */
void i3pager_printf( CHAR_DATA *ch, char *fmt, ... )
{
   char buf[MSL*2];
   va_list args;
 
   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   i3send_to_pager( buf, ch );
   return;
}

/* Some more code borrowed from the IMC2 code to handle the channel list */

/*  I3_getarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
const char *I3_getarg( const char *argument, char *arg, int length )
{
  int len = 0;

  while( *argument && isspace( *argument ) )
    argument++;

  if( arg )
    while( *argument && !isspace( *argument ) && len < length-1 )
      *arg++= *argument++, len++;
  else
    while( *argument && !isspace( *argument ) )
      argument++;

  while( *argument && !isspace( *argument ) )
    argument++;

  while( *argument && isspace( *argument ) )
    argument++;

  if( arg )
    *arg = 0;

  return argument;
}

/* Check for a name in a list */
int I3_hasname( const char *list, const char *name )
{
    const char *p;
    char arg[MIL];

    if( !list )
	return(0);

    p = I3_getarg( list, arg, MIL );
    while( arg[0] )
    {
      if( !strcasecmp( name, arg ) )
        return 1;
      p = I3_getarg( p, arg, MIL );
    }

    return 0;
}

/* Add a name to a list */
void I3_flagchan( char **list, const char *name )
{
  char buf[MSL];

  if( I3_hasname( *list, name ) )
    return;

  if( *list && *list[0] != '\0' )
    sprintf( buf, "%s %s", *list, name );
  else
    strcpy( buf, name );
  
  if( *list )
    I3DISPOSE( *list );
  *list = str_dup( buf );
}

/* Remove a name from a list */
void I3_unflagchan( char **list, const char *name )
{
  char buf[1000];
  char arg[MIL];
  const char *p;
  
  buf[0] = 0;
  p = I3_getarg( *list, arg, MIL );
  while( arg[0] )
  {
    if( strcasecmp( arg, name ) )
    {
      if( buf[0] )
	strcat( buf, " " );
      strcat( buf, arg );
    }
    p = I3_getarg( p, arg, MIL );
  }

  I3DISPOSE( *list );
  *list = str_dup( buf );
}

/* Modified form of the strlen_color command that Rustry made and Xerves cleaned up,
 * and IMC2 used as well. Figured it would do the job nicely here too.
 * You need to change the & and { tokens to match what your mud uses to identify color tags.
 * & is the forgound color, { is the background color.
 */
int I3_strlen_color( const char *argument )
{
    const char  *str;
    int    i, length;

    str = argument;
    if( argument[0] == '\0' )
        return 0;

    for( length = i = 0 ; i < strlen ( argument ) ; i++ )
    {
        if( str[i] != '&' && str[i] != '{' )
            ++length;
        if( str[i] == '&' || str[i] == '{' )
        {
            if( (str[i] == '&' && str[i+1] == '{' )
                || (str[i] == '&' && str[i+1] == '{' ))
                length += 2;
            else
                --length;
        }
    }
    return length;
}

/*
 * Returns an initial-capped string.
 */
char *i3capitalize( const char *str )
{
    static char strcap[MSL];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = tolower( str[i] );
    strcap[i] = '\0';
    strcap[0] = toupper( strcap[0] );
    return strcap;
}

/* Borrowed from Samson's new_auth snippet - checks to see if a particular player exists in the mud.
 * This is called from i3locate and i3finger to report on offline characters.
 */
bool i3exists_player( char *name )
{
   struct stat fst;
   char buf[MSL];

   /* Stands to reason that if there ain't a name to look at, they damn well don't exist! */
   if( !name || !str_cmp( name, "" ) )
      return FALSE;

   sprintf( log_buf, "%s%c/%s", PLAYER_DIR, tolower( buf[0] ), i3capitalize( buf ) );

   if( stat( buf, &fst ) != -1 )
	return TRUE;
   else
	return FALSE;
}

/* Thanks to the IMC2 folks - this function is a modified form of verify_format from the IMC2 code.
 * It makes sure that your channel layouts have the proper number of %s tags in them.
 */
bool verify_i3layout( const char *fmt, int number )
{
  const char *c;
  int i = 0;

  c = fmt;
  while( ( c = strchr(c, '%') ) != NULL )
  {
    if( *( c+1 ) == '%' )  /* %% */
    {
      c += 2;
      continue;
    }
    
    if( *(c+1) != 's' )  /* not %s */
      return FALSE;

    c++;
    i++;
  }

  if( i != number )
    return FALSE;

  return TRUE;
}

/*
 * Easy way to go through a list of options.
 */
int which_keyword( char *keyword, ... ) 
{
    va_list ap;
    int i = 1;
    char *arg;
    char k[MAX_STRING_LENGTH];

    if( !keyword || !keyword[0] )
        return -1;

    va_start( ap, keyword );
    keyword = i3one_argument( keyword, k );
    do 
    {
	arg = va_arg( ap, char * );
	if( arg ) 
	{
	    if( !str_prefix( k, arg ) ) 
	    {
		va_end( ap );
		return i;
	    }
	}
      i++;
    } while( arg );

    va_end( ap );
    return 0;
}

char *rankbuffer( CHAR_DATA *ch )
{
   static char rbuf[MSL];

   if( IS_IMMORTAL(ch) )
   {
      strcpy( rbuf, "&YImmortal" );

      if( CH_RANK(ch) && CH_RANK(ch)[0] != '\0' )
         sprintf( rbuf, "&Y%s", CH_RANK(ch) );
   }
   else
   {
      sprintf( rbuf, "&B%s", CH_CLASSNAME(ch) );

   	if( CH_RANK(ch) && CH_RANK(ch)[0] != '\0' )
	   sprintf( rbuf, "&B%s", CH_RANK(ch) );

	if( CH_CLAN(ch) && !str_cmp( CH_NAME(ch), CH_CLANLEADNAME(ch) ) && CH_CLANLEADRANK(ch)[0] != '\0' )
         sprintf( rbuf, "&B%s", CH_CLANLEADRANK(ch) );

      if( CH_CLAN(ch) && !str_cmp( CH_NAME(ch), CH_CLANONENAME(ch) ) && CH_CLANONERANK(ch)[0] != '\0' )
         sprintf( rbuf, "&B%s", CH_CLANONERANK(ch) );

 	if( CH_CLAN(ch) && !str_cmp( CH_NAME(ch), CH_CLANTWONAME(ch) ) && CH_CLANTWORANK(ch)[0] != '\0' )
         sprintf( rbuf, "&B%s", CH_CLANTWORANK(ch) );
   }
   return rbuf;
}

bool i3ignoring( CHAR_DATA *ch, char *ignore )
{
   I3_IGNORE *temp;

   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
   {
	if( !str_cmp( temp->name, ignore ) )
	   return TRUE;
   }
   return FALSE;
}

/* Fixed this function yet again. If the socket is negative or 0, then it will return
 * a FALSE. Used to just check to see if the socket was positive, and that just wasn't
 * working for the way some places checked for this. Any negative value is an indication
 * that the socket never existed.
 */
bool I3_is_connected( void )
{
    if( I3_socket < 1 )
	return FALSE;

    return TRUE;
}

/*
 * Add backslashes in front of the " and \'s
 */
char *I3_escape( char *ps ) 
{
    static char new[MSL];
    char *pnew = new;

    while( ps[0] ) 
    {
	if( ps[0] == '"' ) 
	{
	    pnew[0]= '\\';
	    pnew++;
	}
	if( ps[0] == '\\' ) 
	{
	    pnew[0] = '\\';
	    pnew++;
	}
	pnew[0] = ps[0];
	pnew++;
	ps++;
    }
    pnew[0] = 0;
    return new;
}

/*
 * Remove "'s at begin/end of string
 * If a character is prefixed by \'s it also will be unescaped
 */
void I3_remove_quotes( char **ps ) 
{
    char *ps1, *ps2;

    if( *ps[0] == '"' )
	(*ps)++;
    if( (*ps)[strlen(*ps)-1] == '"' )
	(*ps)[strlen(*ps)-1] = 0;

    ps1 = ps2 = *ps;
    while( ps2[0] ) 
    {
	if( ps2[0] == '\\' ) 
	{
	    ps2++;
	}
	ps1[0] = ps2[0];
	ps1++;
	ps2++;
    }
    ps1[0] = 0;
}

/* Searches through the channel list to see if one exists with the localname supplied to it. */
I3_CHANNEL *find_I3_channel_by_localname( char *name )
{
    I3_CHANNEL *this;

    for( this = first_I3chan; this; this = this->next )
    {
	if( !str_cmp( this->local_name, name ) )
	   return this;
    }

    return NULL;
}

/* Searches through the channel list to see if one exists with the I3 channel name supplied to it.*/
I3_CHANNEL *find_I3_channel_by_name( char *name ) 
{
    I3_CHANNEL *this;

    for( this = first_I3chan; this; this = this->next )
    {
	if( !str_cmp( this->I3_name, name ) )
	   return this;
    }

    return NULL;
}

/* Sets up a channel on the mud for the first time, configuring its default layout.
 * If you don't like the default layout of channels, this is where you should edit it to your liking.
 */
I3_CHANNEL *new_I3_channel( void ) 
{
    I3_CHANNEL *new;

    CREATE( new, I3_CHANNEL, 1 );
    LINK( new, first_I3chan, last_I3chan, next, prev );
    strcpy( new->layout_m, "&R[&W%s&R] &Y%s@%s: &c%s" );
    strcpy( new->layout_e, "&R[&W%s&R] &Y%s" );
    return new;
}

/* Removes a listener's pointer from the channel's list of listeners. */
void destroy_I3_listener( I3_CHANNEL *channel, I3_LISTENER *listener ) 
{
    if( channel == NULL ) 
    {
	i3bug( "destroy_I3_listener: Null parameter for channel" );
	return;
    }

    if( listener == NULL )
    {
	i3bug( "destroy_I3_listener: NULL parameter for listener" );
	return;
    }

    if( channel->first_listener == NULL )
    {
	sprintf( log_buf, "destroy_I3_listener: NULL first_listener for channel %s!", channel->local_name );
	i3bug( log_buf );
	return;
    }

    UNLINK( listener, channel->first_listener, channel->last_listener, next, prev );
    DISPOSE( listener );
}

/* Deletes a channel's information from the mud. */
void destroy_I3_channel( I3_CHANNEL *channel )
{
    I3_LISTENER *ckill, *kill_next;

    if ( channel == NULL )
    {
	i3bug( "destroy_I3_channel: Null parameter" );
	return;
    }

    UNLINK( channel, first_I3chan, last_I3chan, next, prev );

    for( ckill = channel->first_listener; ckill; ckill = kill_next )
    {
	kill_next = ckill->next;
	destroy_I3_listener( channel, ckill );
    }
    channel->first_listener = channel->last_listener = NULL;
    DISPOSE( channel );
}

/* Finds a mud with the name supplied on the mudlist */
I3_MUD *find_I3_mud_by_name( char *name ) 
{
    I3_MUD *this;

    for( this = first_mud; this; this = this->next )
    {
	if( !str_cmp( this->name, name ) )
	    return this;
    }

    return NULL;
}

I3_MUD *new_I3_mud( char *name )
{
   I3_MUD *new, *mud_prev;

   CREATE( new, I3_MUD, 1 );
   strcpy( new->name, name );

   for( mud_prev = first_mud; mud_prev; mud_prev = mud_prev->next )
      if( strcasecmp( mud_prev->name, name ) >= 0 )
         break;

   if( !mud_prev )
      LINK( new, first_mud, last_mud, next, prev );
   else
      INSERT( new, mud_prev, first_mud, next, prev );

   return new;
}

void destroy_I3_mud( I3_MUD *mud )
{
    if( mud == NULL ) 
    {
	i3bug( "destroy_I3_mud: Null parameter" );
	return;
    }
    
    UNLINK( mud, first_mud, last_mud, next, prev );
    DISPOSE( mud );
}

I3_LISTENER *new_I3_listener( I3_CHANNEL *channel, DESCRIPTOR_DATA *d ) 
{
    I3_LISTENER *new;

    if( !channel )
    {
	i3bug( "new_I3_listener: NULL channel!" );
	return NULL;
    }

    CREATE( new, I3_LISTENER, 1 );
    new->desc = d;
    LINK( new, channel->first_listener, channel->last_listener, next, prev );
    return new;
}

I3_LISTENER *find_I3_listener_by_char( I3_CHANNEL *channel, CHAR_DATA *ch ) 
{
    I3_LISTENER *this;

    if( !channel )
    {
	i3bug( "find_I3_listener_by_char: NULL channel!" );
	return NULL;
    }

    if( !ch )
    {
	i3bug( "find_I3_listener_by_char: NULL ch!" );
	return NULL;
    }

    for( this = channel->first_listener; this; this = this->next )
    {
	if( this->desc == ch->desc )
	   return this;
    }

    return NULL;
}

I3_LISTENER *find_I3_listener_by_descriptor( I3_CHANNEL *channel, DESCRIPTOR_DATA *d ) 
{
    I3_LISTENER *this;

    if( !channel )
    {
	i3bug( "find_I3_listener_by_descriptor: NULL channel" );
	return NULL;
    }

    if( !d )
    {
	i3bug( "find_I3_listener_by_descriptor: NULL descriptor!" );
	return NULL;
    }

    if( !channel->first_listener )
       return NULL;
    
    for( this = channel->first_listener; this; this = this->next )
    {
	if( this->desc == d )
	   return this;
    }
    return NULL;
}

/*
 * Close the socket to the router.
 */
void I3_connection_close( bool reconnect ) 
{
   i3log( "Closing connection to Intermud-3 router." );
   close( I3_socket );
   I3_socket = -1;
   if( reconnect )
   {
      if( reconattempts <= 5 )
	{
	   i3wait = 100; /* Wait for 100 game loops */
	   i3log( "I3: Will attempt to reconnect in approximately 5 seconds." );
      }
	else if( reconattempts <= 20 )
	{
	   i3wait = 5000; /* Wait for 5000 game loops */
         i3log( "I3: Will attempt to reconnect in approximately 5 minutes due to extended failures." );
	}
	else
	{
         i3wait = -2; /* Abandon attempts - probably an ISP failure anyway if this happens :) */
	   i3log( "Abandoning attempts to reconnect to Intermud-3 router. Too many failures." );
	}
   }
   return;
}

/*
 * Write a string into the send-buffer. Does not yet send it.
 */
void I3_write_buffer( char *msg )
{
   long newsize = I3_output_pointer+strlen( msg );

   if( newsize > 256*256-1 )
   {
	sprintf( log_buf, "I3: I3_write_buffer: buffer too large (would become %ld)", newsize );
	i3bug( log_buf );
	return;
   }
   if( packetdebug )
	strcat( debugstring, msg );
   strcpy( I3_output_buffer + I3_output_pointer, msg );
   I3_output_pointer = newsize;
}

/* Use this function in place of I3_write_buffer ONLY if the text to be sent could 
 * contain color tags to parse into ANSI codes. Otherwise it will mess up the packet.
 */
void send_to_i3( char *text )
{
   sprintf( cbuf, "%s", I3_tagtofish( text ) );
   I3_write_buffer( cbuf );
}

/*
 * Writes the string into the socket, prefixed by the size.
 */
bool I3_write_packet( char *msg ) 
{
   int oldsize, size, check;
   char *s = I3_output_buffer;

   oldsize = size = strlen( msg+4 );
   s[3] = size%256;
   size >>= 8;
   s[2] = size%256;
   size >>= 8;
   s[1] = size%256;
   size >>= 8;
   s[0] = size%256;
   check = write( I3_socket, msg, oldsize + 4 );
   if( !check || ( check < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
   {
	if( check < 0 )
	   i3log( "I3: Write error on socket." );
	else
	   i3log( "I3: EOF encountered on socket write." );
  	I3_connection_close( TRUE );
	return FALSE;
   }

   if( check < 0 ) /* EAGAIN */
	return TRUE;

   if( packetdebug )
   {
	sprintf( log_buf, "Packet sent: %s", debugstring );
	i3log( log_buf );
	debugstring[0] = '\0';
   }
   I3_output_pointer = 4;
   return TRUE;
}

void I3_send_packet( void )
{
   I3_write_packet( I3_output_buffer );
   return;
}

/*
 * Put a I3-header in the send-buffer. If a field is NULL it will
 * be replaced by a 0 (zero).
 */
void I3_write_header( char *identifier, char *originator_mudname,
		       char *originator_username, char *target_mudname, char *target_username ) 
{
    I3_write_buffer( "({\"" );
    I3_write_buffer( identifier );
    I3_write_buffer( "\",5," );
    if( originator_mudname ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( originator_mudname );
	I3_write_buffer( "\"," );
    }
    else I3_write_buffer( "0," );

    if( originator_username )  
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( originator_username );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );

    if( target_mudname ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( target_mudname );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );

    if( target_username ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( target_username );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );
}

/*
 * Gets the next I3 field, that is when the amount of {[("'s and
 * ")]}'s match each other when a , is read. It's not foolproof, it
 * should honestly be some kind of statemachine, which does error-
 * checking. Right now I trust the I3-router to send proper packets
 * only. How naive :-) [Indeed Edwin, but I suppose we have little choice :P - Samson]
 *
 * ps will point to the beginning of the next field.
 *
 */
char *I3_get_field( char *packet, char **ps )
{
    int count[256];
    char has_apostrophe = 0, has_backslash = 0;
    char foundit = 0;

    bzero( count, sizeof(count) );

    *ps = packet;
    while( 1 ) 
    {
	switch( *ps[0] ) 
      {
	    case '{': if( !has_apostrophe ) count['{']++; break;
	    case '}': if( !has_apostrophe ) count['}']++; break;
	    case '[': if( !has_apostrophe ) count['[']++; break;
	    case ']': if( !has_apostrophe ) count[']']++; break;
	    case '(': if( !has_apostrophe ) count['(']++; break;
	    case ')': if( !has_apostrophe ) count[')']++; break;
	    case '\\':
		if( has_backslash )
		    has_backslash = 0;
		else
		    has_backslash = 1;
		break;
	    case '"':
		if( has_backslash ) 
		{
		    has_backslash = 0;
		} 
		else 
		{
		    if( has_apostrophe )
			has_apostrophe = 0;
		    else
			has_apostrophe = 1;
		}
		break;
	    case ',':
	    case ':':
		if( has_apostrophe )
		    break;
		if( has_backslash )
		    break;
		if( count['{'] != count['}'] )
		    break;
		if( count['['] != count[']'] )
		    break;
		if( count['('] != count[')'] )
		    break;
		foundit = 1;
		break;
	}
	if( foundit )
	    break;
	(*ps)++;
    }
    *ps[0] = 0;
    (*ps)++;
    return *ps;
}

/*
 * Read the header of an I3 packet. pps will point to the next field
 * of the packet.
 */
void I3_get_header( char **pps, I3_HEADER *header ) 
{
    char *ps = *pps, *next_ps;

    header->originator_mudname[0] = 0;	header->originator_mudname[255] = 0;
    header->originator_username[0] = 0;	header->originator_username[255] = 0;
    header->target_mudname[0] = 0;		header->target_mudname[255] = 0;
    header->target_username[0] = 0;		header->target_username[255] = 0;

    I3_get_field( ps, &next_ps );
    ps = next_ps;
    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strncpy( header->originator_mudname, ps, 254 );
    ps = next_ps;
    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strncpy( header->originator_username, ps, 254 );
    ps = next_ps;
    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strncpy( header->target_mudname, ps, 254 );
    ps = next_ps;
    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strncpy( header->target_username, ps, 254 );
    *pps = next_ps;
}

/*
 * Returns a CHAR_DATA structure which matches the string
 *
 */
CHAR_DATA *I3_find_user( char *name ) 
{
    DESCRIPTOR_DATA *d;

    for ( d = first_descriptor; d ; d = d->next ) 
    {
	if( d->character && !str_cmp( CH_NAME(d->character), name ) ) 
	   return d->character;
    }
    return NULL;
}

/* The all important startup packet. This is what will be initially sent upon trying to connect
 * to the I3 router. It is therefore quite important that the information here be exactly correct.
 * If anything is wrong, your packet will be dropped by the router as invalid and your mud simply
 * won't connect to I3. DO NOT USE COLOR TAGS FOR ANY OF THIS INFORMATION!!!
 */
void I3_startup_packet( void ) 
{
   char s[MIL];

   if( !I3_is_connected() )
	return;

   I3_stats.count_startup_req_3++;

   I3_write_header( "startup-req-3", this_mud->name, NULL, this_mud->routerName, NULL );
   
   sprintf( s, "%d", this_mud->password );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   sprintf( s, "%d", this_mud->mudlist_id );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   sprintf( s, "%d", this_mud->chanlist_id );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   sprintf( s, "%d", this_mud->player_port );
   I3_write_buffer( s );
   I3_write_buffer( ",0,0,\"" );

   I3_write_buffer( this_mud->mudlib );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->base_mudlib );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->driver );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->mud_type );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->open_status );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->admin_email );
   I3_write_buffer( "\"," );

   I3_write_buffer( "([\"emoteto\":" );
   I3_write_buffer( this_mud->emoteto ? "1" : "0" );
   I3_write_buffer( ",\"news\":" );
   I3_write_buffer( this_mud->news ? "1" : "0" );
   I3_write_buffer( ",\"ucache\":" );
   I3_write_buffer( this_mud->ucache ? "1" : "0" );
   I3_write_buffer( ",\"auth\":" );
   I3_write_buffer( this_mud->auth ? "1" : "0" );
   I3_write_buffer( ",\"ftp\":" );
   sprintf( s, "%d", this_mud->ftp );
   I3_write_buffer( s );
   I3_write_buffer( ",\"nntp\":" );
   sprintf( s, "%d", this_mud->nntp );
   I3_write_buffer( s );
   I3_write_buffer( ",\"rcp\":" );
   sprintf( s, "%d", this_mud->rcp );
   I3_write_buffer( s );
   I3_write_buffer( ",\"amrcp\":" );
   sprintf( s, "%d", this_mud->amrcp );
   I3_write_buffer( s );
   I3_write_buffer( ",\"tell\":" );
   I3_write_buffer( this_mud->tell ? "1" : "0" );
   I3_write_buffer( ",\"beep\":" );
   I3_write_buffer( this_mud->beep ? "1" : "0" );
   I3_write_buffer( ",\"mail\":" );
   I3_write_buffer( this_mud->mail ? "1" : "0" );
   I3_write_buffer( ",\"file\":" );
   I3_write_buffer( this_mud->file ? "1" : "0" );
   I3_write_buffer( ",\"url\":" );
   sprintf( s, "\"http://%s\"", this_mud->web );
   I3_write_buffer( s );
   I3_write_buffer( ",\"http\":" );
   sprintf( s, "%d", this_mud->http );
   I3_write_buffer( s );
   I3_write_buffer( ",\"smtp\":" );
   sprintf( s, "%d", this_mud->smtp );
   I3_write_buffer( s );
   I3_write_buffer( ",\"pop3\":" );
   sprintf( s, "%d", this_mud->pop3 );
   I3_write_buffer( s );
   I3_write_buffer( ",\"locate\":" );
   I3_write_buffer( this_mud->locate ? "1" : "0" );
   I3_write_buffer( ",\"finger\":" );
   I3_write_buffer( this_mud->finger ? "1" : "0" );
   I3_write_buffer( ",\"channel\":" );
   I3_write_buffer( this_mud->channel ? "1" : "0" );
   I3_write_buffer( ",\"who\":" );
   I3_write_buffer( this_mud->who ? "1" : "0" );
   I3_write_buffer( ",]),0,})\r" );

   I3_send_packet( );

   sprintf( log_buf, "I3: Sending startup_packet to %s", this_mud->routerName );
   i3log( log_buf );
}

/*
startup-reply
   
   This packet will be delivered to a mud for three conditions: in
   response to a startup-req packet, when the router wishes the mud to
   connect to a different router, or when the set of routers change for
   some reason.                      
    ({
        (string)   "startup-reply",
        (int)      5, 
        (string)   originator_mudname,     // the router
        (string)   0,
        (string)   target_mudname,
        (string)   0,
        (string *) router_list,
        (int)      password
    })                        

   The router_list is an array representing an ordered list of routers to
   use. The first element should be the router that the mud should use.
   Typically, this will be the router that the mud initially connected
   to. If not, however, then the mud should close the connection and
   reopen to the designated router. The list should be saved and used in
   case of failure to connect to a router. Each element in the list is an
   array of two elements; the first element is the router name, the   
   second element is the router's address in the following format:
   "ip.ad.re.ss portnum". Note that this address can be passed to MudOS's
   socket_connect() function.
   For example: ({ "*nightmare", "199.199.122.10 9000" }).                                       
   
   The first router specified in the list will be the mud's preferred   
   router. Future initial connections and startup-req packets will go to
   that router until told otherwise.

*/

/* This function saves the password, mudlist ID, and chanlist ID that are used by the mud.
 * The password value is returned from the I3 router upon your initial connection.
 * The mudlist and chanlist ID values are updated as needed while your mud is connected.
 * Do not modify the file it generates because doing so may prevent your mud from reconnecting
 * to the router in the future. This file will be rewritten each time the I3_shutdown function
 * is called, or any of the id values change.
 */
void I3_save_id( void )
{
   FILE *fp;

   if( ( fp = fopen( I3_PASSWORD_FILE, "w" ) ) == NULL ) 
   {
	i3log( "Couldn't write to I3 password file." );
	return;
   }

   fprintf( fp, "#PASSWORD\n" );
   fprintf( fp, "%d %d %d\n", this_mud->password, this_mud->mudlist_id, this_mud->chanlist_id );
   FCLOSE( fp );
}

/* The second most important packet your mud will deal with. If you never get this
 * coming back from the I3 router, something was wrong with your startup packet
 * or the router may be jammed up. Whatever the case, if you don't get a reply back
 * your mud won't be acknowledged as connected.
 */
int I3_process_startup_reply( char *s ) 
{
   char *ps = s, *next_ps;
   I3_HEADER header;

   I3_get_header( &ps, &header );

   /* Recevies the router list. Nothing much to do here until there's more than 1 router. */
   I3_get_field( ps, &next_ps );
   i3log( ps ); /* Just checking for now */
   ps = next_ps;

   /* Receives your mud's updated password, which may or may not be the same as what it sent out before */
   I3_get_field( ps, &next_ps );
   this_mud->password = atoi( ps );
   ps = next_ps;

   sprintf( log_buf, "I3: Received startup_reply from %s", header.originator_mudname );
   i3log( log_buf );
   I3_save_id( );
   reconattempts = 0;
   i3wait = 0;
   return 0;
}

void I3_send_error( char *mud, char *user, char *code, char *message ) 
{
    if( !I3_is_connected() )
	return;

    I3_write_header( "error", I3_THISMUD, 0, mud, user );
    I3_write_buffer( "\"" );
    I3_write_buffer( code );
    I3_write_buffer( "\",\"" );
    I3_write_buffer( message );
    I3_write_buffer( "\",0,})\r" );

    I3_send_packet( );
}

void I3_process_error( char *s ) 
{
    CHAR_DATA *ch;
    I3_HEADER header;
    char *next_ps, *ps = s;
    char type[MSL], message[MSL], error[MSL];

    I3_get_header( &ps, &header );

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strcpy( type, ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    strcpy( message, ps );

    sprintf( error, "I3: Error: from %s to %s@%s\n\r%s: %s",
	header.originator_mudname, header.target_username, header.target_mudname, type, message );

    if( ( ch = I3_find_user( header.target_username ) ) == NULL )
      i3log( error );
    else
	i3_printf( ch, "%s\n\r", error );
}

int i3todikugender( int gender )
{
   int sex = 0;

   if( gender == 0 )
      sex = SEX_MALE;

   if( gender == 1 )
	sex = SEX_FEMALE;

   if( gender > 1 )
	sex = SEX_NEUTRAL;

   return sex;
}

int dikutoi3gender( int gender )
{
   int sex = 0;

   if( gender > 2 || gender < 0 )
      sex = 2; /* I3 neuter */
   
   if( gender == SEX_MALE )
	sex = 0; /* I3 Male */

   if( gender == SEX_FEMALE )
	sex = 1; /* I3 Female */

   return sex;
}

/* This is very possibly going to be spammy as hell */
void I3_show_ucache_contents( CHAR_DATA *ch )
{
   UCACHE_DATA *user;
   int users = 0;

   if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR )
   {
	i3_to_char( "This function is restricted to administrators only.\n\r", ch );
	return;
   }

   i3send_to_pager( "Cached user information\n\r", ch );
   i3send_to_pager( "User                          | Gender ( 0 = Male, 1 = Female, 2 = Neuter )\n\r", ch );
   i3send_to_pager( "---------------------------------------------------------------------------\n\r", ch );
   for( user = first_ucache; user; user = user->next )
   {
	i3pager_printf( ch, "%-30s %d\n\r", user->name, user->gender );
	users++;
   }
   i3pager_printf( ch, "%d users being cached.\n\r", users );
   return;
}

int I3_get_ucache_gender( char *name )
{
   UCACHE_DATA *user;

   for( user = first_ucache; user; user = user->next )
   {
	if( !str_cmp( user->name, name ) )
	   return user->gender;
   }

   /* -1 means you aren't in the list and need to be put there. */
   return -1;
}

/* Saves the ucache info to disk because it would just be spamcity otherwise */
void I3_save_ucache( void )
{
   FILE *fp;
   UCACHE_DATA *user;

   if( ( fp = fopen( I3_UCACHE_FILE, "w" ) ) == NULL ) 
   {
	i3log( "Couldn't write to I3 ucache file." );
	return;
   }

   for( user = first_ucache; user; user = user->next )
   {
	fprintf( fp, "#UCACHE\n" );
	fprintf( fp, "Name %s~\n", user->name );
	fprintf( fp, "Sex  %d\n", user->gender );
	fprintf( fp, "Time %ld\n", user->time );
	fprintf( fp, "End\n\n" );
   }
   fprintf( fp, "#END\n" );
   FCLOSE( fp );
   return;
}

void I3_prune_ucache( void )
{
   UCACHE_DATA *ucache, *next_ucache;

   for( ucache = first_ucache; ucache; ucache = next_ucache )
   {
	next_ucache = ucache->next;

	/* Info older than 30 days is removed since this person likely hasn't logged in at all */
	if( current_time - ucache->time >= 2592000 )
	{
	   UNLINK( ucache, first_ucache, last_ucache, next, prev );
	   DISPOSE( ucache );
	}
   }
   I3_save_ucache( );
   return;
}

/* Updates user info if they exist, adds them if they don't. */
void I3_ucache_update( char *name, int gender )
{
   UCACHE_DATA *user;

   for( user = first_ucache; user; user = user->next )
   {
      if( !str_cmp( user->name, name ) )
	{
	   user->gender = gender;
	   return;
	}
   }
   CREATE( user, UCACHE_DATA, 1 );
   strcpy( user->name, name );
   user->gender = gender;
   user->time = current_time;
   LINK( user, first_ucache, last_ucache, next, prev );

   I3_save_ucache( );
   return;
}

void I3_send_ucache_update( char *visname, int gender )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "ucache-update", I3_THISMUD, NULL, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( visname );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( visname );
   I3_write_buffer( "\"," );
   sprintf( log_buf, "%d", gender );
   I3_write_buffer( log_buf );
   I3_write_buffer( ",})\r" );

   I3_send_packet( );

   return;
}

void I3_process_ucache_update( char *s )
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char username[MSL], visname[MSL];
   int sex, gender;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( username, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( visname, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   gender = atoi( ps );

   sprintf( log_buf, "%s@%s", visname, header.originator_mudname );
   sex = I3_get_ucache_gender( log_buf );

   if( sex == gender )
      return;

   I3_ucache_update( log_buf, gender );
   return;
}

/*
    For targetted emotes, it is necessary to get information on the target. 
    There are two pieces of information required: the visname and the gender of the target. 
    This operation is performed with the following packets: 

                         ({
                             (string) "chan-user-req",
                             (int)    5,
                             (string) originator_mudname,
                             (string) 0,
                             (string) target_mudname,
                             (string) 0,
                             (string) username
                         })

    Note that the username is separate since the packet is not targetted to the user. 
*/
int I3_send_chan_user_req( char *targetmud, char *targetuser )
{
   if( !I3_is_connected() )
	return 0;

   I3_write_header( "chan-user-req", I3_THISMUD, NULL, targetmud, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( targetuser );
   I3_write_buffer( "\",})\r" );

   I3_send_packet( );

   return 0;
}

int I3_process_chan_user_req( char *s )
{
   char *ps = s, *next_ps;
   CHAR_DATA *ch;
   I3_HEADER header;
   int sex, gender;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( ( ch = I3_find_user( ps ) ) == NULL ) 
   {
	if( !i3exists_player( ps ) )
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	return 0;
   }

   if( I3ISINVIS(ch) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
   if( i3ignoring( ch, cbuf ) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   /* Since this is a gender thing, we need to gather that info. It's assumed anything above 2
    * is generally referred to as an "it" anyway, so send them a neuter if it's above 2.
    * And I3 genders are in a different order from standard Diku genders, so this complicates matters some.
    */
   gender = CH_SEX(ch);

   sex = dikutoi3gender( gender );

   I3_write_header( "chan-user-reply", I3_THISMUD, NULL, header.originator_mudname, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_NAME(ch) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( CH_NAME(ch) );
   I3_write_buffer( "\"," );
   sprintf( log_buf, "%d", sex );
   I3_write_buffer( log_buf );
   I3_write_buffer( ",})\r" );

   I3_send_packet( );

   return 0;
}

/*
    The reply for the user info request takes the following format: 

                         ({
                             (string) "chan-user-reply",
                             (int)    5,
                             (string) originator_mudname,
                             (string) 0,
                             (string) target_mudname,
                             (string) 0,
                             (string) username,
                             (string) visname,
                             (int)    gender
                         })

                         The gender takes one of the following values: 

                             0  male
                             1  female
                             2  neuter

     Note that a mud may have more variants on gender, but most human languages only have
     three forms at most. These are used to select the appropriate pronouns, possessives, and reflexive words.
     This information can then be saved for later retreival on muds using the ucache service.
*/
int I3_process_chan_user_reply( char *s )
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char username[MSL], visname[MSL];
   int sex, gender;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( username, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( visname, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   gender = atoi( ps );

   sprintf( log_buf, "%s@%s", visname, header.originator_mudname );
   sex = I3_get_ucache_gender( log_buf );

   if( sex == gender )
      return 0;

   I3_ucache_update( log_buf, gender );
   return 0;
}

/*
   The router will send this to a mud whenever the mud's list needs to be
   updated. Typically, this will happen once right after login (based on 
   the old_mudlist_id that a mud provided in the startup-req-3 packet),
   and then as changes occur within the intermud network. A mud should 
   remember the mudlist and its associated mudlist_id across reconnects
   to the router.                                                       
    ({                                                                   
        (string)  "mudlist",                                          
        (int)     5,
        (string)  originator_mudname,     // the router             
        (string)  0,                                                  
        (string)  target_mudname,
        (string)  0,       
        (int)     mudlist_id  
        (mapping) info_mapping                                      
    })  

   The info_mapping contains mud names as keys and information about each
   mud as the value. This information is specified as an array with the
   following format:                                                 
    ({                                                                
        (int)     state,                                               
        (string)  ip_addr,                                              
        (int)     player_port,                                           
        (int)     imud_tcp_port,                                         
        (int)     imud_udp_port,                                      
        (string)  mudlib,
        (string)  base_mudlib,
        (string)  driver,                                               
        (string)  mud_type,                      
        (string)  open_status,
        (string)  admin_email,                                        
        (mapping) services                                              
        (mapping) other_data                                           
    })           
     
   Each record of information should replace any prior record for a    
   particular mud. If the mapping's value is zero, then the mud has been
   deleted (it went down and has not come back for a week) from the    
   Intermud.           

   state is an integer with the following values:                   
        -1  mud is up                                                    
         0  mud is down                                                
         n  mud will be down for n seconds                        
*/

int I3_process_mudlist( char *s ) 
{
   char *ps = s, *next_ps;
   I3_MUD *mud;
   I3_HEADER header;

   I3_get_header( &ps, &header );
   I3_get_field( ps, &next_ps );
   this_mud->mudlist_id = atoi( ps );
   I3_save_id( );

   ps = next_ps;
   ps += 2;

   while( 1 ) 
   {
	char *next_ps2;
	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
	mud = find_I3_mud_by_name( ps );
	if( mud == NULL ) 
	   mud = new_I3_mud( ps );

	ps = next_ps;
	I3_get_field( ps, &next_ps2 );

	if( ps[0] != '0' ) 
	{
	   ps += 2;

	   I3_get_field( ps, &next_ps );
	   mud->status = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->ipaddress, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->player_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->imud_tcp_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->imud_udp_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->mudlib, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->base_mudlib, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->driver, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->mud_type, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->open_status, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( mud->admin_email, ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );

	   ps += 2;
	   while( 1 ) 
	   {
		char *next_ps3;
		char key[MIL];

		if( ps[0] == ']' )
		    break;

		I3_get_field( ps, &next_ps3 );
		I3_remove_quotes( &ps );
		strcpy( key, ps );
		ps = next_ps3;
		I3_get_field( ps, &next_ps3 );

		switch( key[0] ) 
		{
		case 'a': 
		    if( !str_cmp( key, "auth" ) )
		    {
			mud->auth = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "amrcp" ) )
		    {
			mud->amrcp = atoi( ps );
			break;
		    }
		    break;
		case 'b':
		    if( !str_cmp( key, "beep" ) )
		    {
			mud->beep = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'c': 
		    if( !str_cmp( key, "channel" ) )
		    {
			mud->channel = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		case 'e': 
		    if( !str_cmp( key, "emoteto" ) )
		    {
			mud->emoteto = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'f': 
		    if( !str_cmp( key, "file" ) )
		    {
			mud->file = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "finger" ) )
		    {
			mud->finger = ps[0] == '0' ? 0 : 1;
			break;
                }
		    if( !str_cmp( key, "ftp" ) ) 
		    {
			mud->ftp = atoi( ps );
			break;
		    }
		    break;
		case 'h': 
		    if( !str_cmp( key, "http" ) )
		    {
			mud->http = atoi( ps );
			break;
                }
		    break;
		case 'l': 
		    if( !str_cmp( key, "locate" ) )
		    {
			mud->locate = ps[0] == '0' ? 0 : 1;
			break;
 		    }
		    break;
		case 'm':
		    if( !str_cmp( key, "mail" ) )
		    {
			mud->mail = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'n': 
		    if( !str_cmp( key, "news" ) )
		    {
			mud->news = ps[0] == '0' ? 0 : 1; 
			break;
		    }
		    if( !str_cmp( key, "nntp" ) )
		    {
			mud->nntp = atoi( ps );
			break;
                }
		    break;
		case 'p': 
		    if( !str_cmp( key, "pop3" ) )
		    {
			mud->pop3 = atoi( ps );
			break;
		    }
		    break;
		case 'r': 
		    if( !str_cmp( key, "rcp" ) )
		    {
			mud->rcp = atoi( ps );
			break;
		    }
		    break;
		case 's': 
		    if( !str_cmp( key, "smtp" ) )
		    {
			mud->smtp = atoi( ps );
			break;
		    }
		    break;
		case 't': 
		    if( !str_cmp( key, "tell" ) )
		    {
			mud->tell = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		case 'u': 
		    if( !str_cmp( key, "ucache" ) )
		    {
			mud->ucache = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "url" ) )
		    {
			I3_remove_quotes( &ps );
			strcpy( mud->web, ps );
			break;
		    }
		    break;
		case 'w':
		    if( !str_cmp( key, "who" ) )
		    {
			mud->who = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		default:
		    break;
		}

		ps = next_ps3;
		if( ps[0] == ']' )
		    break;
	   }
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   ps = next_ps;

	}
	ps = next_ps2;
	if( ps[0] == ']' )
	   break;
   }
   return 0;
}

/* When a mud sends a startup-req-3 packet, it includes its chanlist-id in the packet. 
   The router will potentially respond with a chanlist-reply message to update the
   mud's channel list. 

   The router will respond to channel list changes with the chanlist-reply packet. 

                         ({
                             (string)  "chanlist-reply",
                             (int)     5,
                             (string)  originator_mudname,     // the router
                             (string)  0,
                             (string)  target_mudname,
                             (string)  0,
                             (int)     chanlist_id,
                             (mapping) channel_list
                         })

   channel_list is mapping with channel names as keys, and an array of two elements as the values. 
   If the value is 0, then the channel has been deleted. The array contains the host mud, 
   and the type of the channel: 

                             0  selectively banned
                             1  selectively admitted
                             2  filtered (selectively admitted)

   All channel messages are delivered to the router. It will then pass the message to the 
   appropriate set of muds. If the channel is filtered, then the packet will be delivered to 
   the host mud for filtering; it will then return to the router network for distribution. It is
   assumed that a channel packet for a filtered channel that comes from the channel host has been filtered. 
*/

int I3_process_chanlist_reply( char *s ) 
{
   char *ps = s, *next_ps;
   I3_CHANNEL *channel;
   I3_HEADER header;

   I3_get_header( &ps, &header );
   I3_get_field( ps, &next_ps );
   this_mud->chanlist_id = atoi( ps );
   I3_save_id( );

   ps = next_ps;
   ps += 2;

   while( 1 ) 
   {
	char *next_ps2;

	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );

	if( ( channel = find_I3_channel_by_name( ps ) ) == NULL ) 
      {
	   channel = new_I3_channel();
	   strcpy( channel->I3_name, ps );
	}

	ps = next_ps;
	I3_get_field( ps, &next_ps2 );
	if( ps[0] != '0' ) 
	{
	   ps += 2;
	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   strcpy( channel->host_mud, ps );
	   ps = next_ps;
	   I3_get_field( ps, &next_ps );
	   channel->status = atoi( ps );
	}
	ps = next_ps2;
	if( ps[0] == ']' )
	   break;
   }
   return 0;
}

/*
   Channel messages come in three flavors: standard messages, emotes, and
   targetted emotes. These use packets channel-m, channel-e, and
   channel-t, respectively. They are:                                   
    ({                                                                  
        (string) "channel-m",                                                   
        (int)    5,
        (string) originator_mudname,
        (string) originator_username,
        (string) 0,                                                     
        (string) 0,                                                  
        (string) channel_name,                                   
        (string) visname,                                                       
        (string) message
    })                                                                
*/
int I3_send_channel_message( I3_CHANNEL *channel, char *name, char *message ) 
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_channel_m_commands++;

    I3_write_header( "channel-m", I3_THISMUD, name, NULL, NULL );
    I3_write_buffer( "\"" );
    I3_write_buffer( channel->I3_name );
    I3_write_buffer( "\",\"" );
    I3_write_buffer( name );
    I3_write_buffer( "\",\"" );
    send_to_i3( I3_escape( message ) );
    I3_write_buffer( "\",})\r" );

    I3_send_packet( );

    return 0;
}

/*
   Channel messages come in three flavors: standard messages, emotes, and
   targetted emotes. These use packets channel-m, channel-e, and
   channel-t, respectively. They are:                                   
    ({                                                                  
        (string) "channel-e",                                                   
        (int)    5,
        (string) originator_mudname,
        (string) originator_username,
        (string) 0,                                                     
        (string) 0,                                                  
        (string) channel_name,                                   
        (string) visname,                                                       
        (string) message
    })                                                                
*/
int I3_send_channel_emote( I3_CHANNEL *channel, char *name, char *message ) 
{
   if( !I3_is_connected() )
	return 0;

   I3_stats.count_channel_e_commands++;

   I3_write_header( "channel-e", I3_THISMUD, name, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( name );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( message ) );
   I3_write_buffer( "\",})\r" );

   I3_send_packet( );

   return 0;
}

/*
When a mud receives a channel-t packet, it should deliver the message locally to all listeners. 
The actual message delivered should be a composition of the channel_name and one of message_others 
or message_target. The messages will include $N and $O for the name of the originator and the 
object/target of their emote. The appropriate message is selected based on the listener - if the
listener matches the targetted mud/user, then the message_target should be used. 
A suggested format is: 

[gwiz] With a flying leap, John@Doe Mud falls into the pit.
[gwiz] Jane@BlandMud waves to Goober@PutzMud.
[gwiz] Jane@BlandMud waves to you.

These were composed with: 

sprintf("[%s] %s", channel_name, one_of_the_messages); 

### need a bit more on channel-t ### 

All messages should not be terminated with a newline - that will be applied on the receiving mud if necessary.
The target_username should be in lower-case if provided. 

    ({
                                  (string) "channel-t",
                                  (int)    5,
                                  (string) originator_mudname,
                                  (string) originator_username,
                                  (string) 0,
                                  (string) 0,
                                  (string) channel_name,
                                  (string) targetted_mudname,
                                  (string) targetted_username,
                                  (string) message_others,
                                  (string) message_target,
                                  (string) originator_visname,
                                  (string) target_visname
                              })
*/
int I3_send_channel_t( I3_CHANNEL *channel, char *name, char *tmud, char *tuser, char *msg_o, char *msg_t, char *tvis )
{
   if( !I3_is_connected() )
	return 0;

   I3_write_header( "channel-t", I3_THISMUD, name, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tmud );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tuser );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( msg_o ) );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( msg_t ) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tvis );
   I3_write_buffer( "\",})\r" );

   I3_send_packet( );

   return 0;
}

int I3_token( char type, char *string, char *oname, char *tname )
{
    char code[ 25 ];
    char *p = '\0';

    switch( type )
    {
        default:
            code[0] = type;
		code[1] = '\0';
            return 1;
        case ' ':
            sprintf( code, " " );
            break;
        case 'N': /* Originator's name */
            sprintf( code, oname );
            break;
        case 'O': /* Target's name */
            sprintf( code, tname );
            break;
    }

    p = code;
    while( *p != '\0' )
    {
        *string = *p++;
        *++string = '\0';
    }

    return( strlen( code ) );
}

void I3_message_convert( char *buffer, const char *txt, char *oname, char *tname )
{
    const char *point;
    int skip = 0;

    for( point = txt ; *point ; point++ )
    {
        if( *point == '$' )
        {
            point++;
            if( *point == '\0' )
                point--;
            else
              skip = I3_token( *point, buffer, oname, tname );
            while( skip-- > 0 )
                ++buffer;
            continue;
        }
        *buffer = *point;
        *++buffer = '\0';
    }                   
    *buffer = '\0';
    return;
}

char *I3_convert_channel_message( const char *message, char *sname, char *tname )
{
   cbuf[0] = '\0';

   /* Sanity checks - if any of these are NULL, bad things will happen - Samson 6-29-01 */
   if( !message )
   {
	i3bug( "I3_convert_channel_message: NULL message!" );
	return "";
   }

   if( !sname )
   {
	i3bug( "I3_convert_channel_message: NULL sname!" );
	return "";
   }

   if( !tname )
   {
	i3bug( "I3_convert_channel_message: NULL tname!" );
	return "";
   }

   I3_message_convert( cbuf, message, sname, tname );
   return cbuf;
}

int I3_process_channel_t( char *s )
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char targetmud[MIL], targetuser[MIL], message_o[MSL], message_t[MSL];
   char visname_o[MIL], visname_t[MIL], sname[MIL], tname[MIL], lname[MIL], tmsg[MSL], omsg[MSL];
   char mudname[24];
   I3_CHANNEL *channel = NULL;
   I3_LISTENER *listener = NULL;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( ( channel = find_I3_channel_by_name( ps ) ) == NULL ) 
   {
	sprintf( log_buf, "I3_process_channel_t: received unknown channel (%s)", ps );
	i3log( log_buf );
	return 0;
   }

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( targetmud, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( targetuser, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( message_o, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( message_t, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( visname_o, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( visname_t, ps );

   /* Need to truncate to no more than 24 chars in the name, cause it crashes like a whore otherwise */
   strncpy( mudname, header.originator_mudname, 23 );
   mudname[24] = '\0';

   sprintf( sname, "%s@%s", visname_o, mudname );
   sprintf( tname, "%s@%s", visname_t, targetmud );

   sprintf( omsg, "%s", I3_convert_channel_message( message_o, sname, tname ) );
   sprintf( tmsg, "%s", I3_convert_channel_message( message_t, sname, tname ) );

   for( listener = channel->first_listener; listener; listener = listener->next ) 
   {
	sprintf( lname, "%s@%s", CH_NAME(listener->desc->character), I3_THISMUD );
 
	if( listener->desc->connected == CON_PLAYING )
	{
	   if( !str_cmp( lname, tname ) )
	   {
            sprintf( log_buf, channel->layout_e, channel->local_name, tmsg );
		i3_printf( listener->desc->character, "%s\n\r", log_buf );
	   }
	   else
	   {
            sprintf( log_buf, channel->layout_e, channel->local_name, omsg );
		i3_printf( listener->desc->character, "%s\n\r", log_buf );
	   }
	}
   }
   return 0;
}

int I3_process_channel_m( char *s ) 
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char visname[MSL], message[MSL];
   I3_CHANNEL *channel;
   I3_LISTENER *listener;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( ( channel = find_I3_channel_by_name( ps ) ) == NULL ) 
   {
	sprintf( log_buf, "I3: channel_m: received unknown channel (%s)", ps );
	i3log( log_buf );
	return 0;
   }

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( visname, ps );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( message, ps );

   sprintf( log_buf, channel->layout_m, channel->local_name, visname, header.originator_mudname, message );

   for( listener = channel->first_listener; listener; listener = listener->next )
   {
	if( listener->desc->connected == CON_PLAYING )
	   i3_printf( listener->desc->character, "%s\n\r", log_buf );
   }
   return 0;
}

int I3_process_channel_e( char *s ) 
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char visname[MSL], message[MSL], msg[MSL];
   char mudname[24];
   I3_CHANNEL *channel;
   I3_LISTENER *listener;

   I3_get_header( &ps, &header );

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( ( channel = find_I3_channel_by_name( ps ) ) == NULL ) 
   {
	sprintf( log_buf, "I3: channel_e: received unknown channel (%s)", ps );
	i3log( log_buf );
	return 0;
   }

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   /* Need to truncate to no more than 24 chars in the name, cause it crashes like a whore otherwise */
   strncpy( mudname, header.originator_mudname, 23 );
   mudname[24] = '\0';

   sprintf( visname, "%s@%s", ps, mudname );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( message, ps );

   sprintf( msg, "%s", I3_convert_channel_message( message, visname, visname ) );
   sprintf( log_buf, channel->layout_e, channel->local_name, msg );

   for( listener = channel->first_listener; listener; listener = listener->next ) 
   {
	if( listener->desc->connected == CON_PLAYING )
	   i3_printf( listener->desc->character, "%s\n\r", log_buf );
   }
   return 0;
}

/*
   A list of who is listening to a channel on a remote mud may be
   requested with the following packet:            
    ({                                                                   
        (string) "chan-who-req",                      
        (int)    5,
        (string) originator_mudname,
        (string) originator_username,
        (string) target_mudname,
        (string) 0,
        (string) channel_name
    })                                                              
*/
int I3_process_chan_who_req( char *s )
{
    I3_HEADER header;
    char *ps = s, *next_ps;
    char buf[MSL];
    I3_CHANNEL *channel;
    I3_LISTENER *listener;

    I3_get_header( &ps, &header );
    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );

    if( ( channel = find_I3_channel_by_name( ps ) ) == NULL )
    {
	sprintf( buf, "The channel you specified (%s) is unknown at %s", ps, I3_THISMUD );
	I3_escape( buf );
	I3_send_error( header.originator_mudname, header.originator_username, "unk-channel", buf );
	sprintf( log_buf, "I3: chan_who_req: received unknown channel (%s)", ps );
	i3log( log_buf );
	return 0;
    }
    if( channel->local_name[0] == 0 ) 
    {
	sprintf( buf, "The channel you specified (%s) is not registered at %s", ps, I3_THISMUD );
	I3_escape( buf );
	I3_send_error( header.originator_mudname, header.originator_username, "unk-channel", buf );
	return 0;
    }

    I3_write_header( "chan-who-reply", I3_THISMUD, NULL, header.originator_mudname, header.originator_username );
    I3_write_buffer( "\"" );
    I3_write_buffer( channel->I3_name );
    I3_write_buffer( "\",({" );

    for( listener = channel->first_listener; listener; listener = listener->next )
    {
      if( I3ISINVIS(listener->desc->character) )
	   continue;
	I3_write_buffer( "\"" );
	I3_write_buffer( CH_NAME(listener->desc->character) );
	I3_write_buffer( "\"," );
    }
    I3_write_buffer( "}),})\r" );

    I3_send_packet( );

    return 0;
}

/*
   The reply for the who request takes the following format:
    ({                                                               
        (string)   "chan-who-reply",
        (int)      5,
        (string)   originator_mudname,
        (string)   0,
        (string)   target_mudname,
        (string)   target_username, 
        (string)   channel_name,                      
        (string *) user_list                   
    })                       
   
   The user_list should be an array of strings, representing the users'
   "visual" names.                                                 
*/
int I3_process_chan_who_reply( char *s ) 
{
    char *ps = s, *next_ps;
    I3_HEADER header;
    CHAR_DATA *ch;

    I3_get_header( &ps, &header );
    if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
    {
	sprintf( log_buf, "I3: I3_process_chan_who_reply(): user %s not found.", header.target_username );
	i3bug( log_buf );
	return 0;
    }

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    i3_printf( ch, "I3 reply from %s for %s\n\r", header.originator_mudname, ps );

    ps = next_ps;
    I3_get_field( ps, &next_ps );
    ps += 2;
    while( 1 ) 
    {
	if( ps[0] == '}' ) 
      {
	    i3_to_char( "No information returned or no people listening.\n\r", ch );
	    return 0;
	}

	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
	i3_printf( ch,"- %s\n\r", ps );

	ps = next_ps;
	if( ps[0]== '}' )
	    break;
    }

    return 0;
}

int I3_send_chan_who( CHAR_DATA *ch, I3_CHANNEL *channel, I3_MUD *mud ) 
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_channel_who_commands++;

    I3_write_header( "chan-who-req", I3_THISMUD, CH_NAME(ch), mud->name, NULL );
    I3_write_buffer( "\"" );
    I3_write_buffer( channel->I3_name );
    I3_write_buffer( "\",})\r" );

    I3_send_packet( );

    return 0;
}

/* beep packet:

The originator will deliver the following packet to the target mud over the in-band network (to its router): 

                         ({
                             (string) "beep",
                             (int)     5,
                             (string) originator_mudname,
                             (string) originator_username,
                             (string) target_mudname,
                             (string) target_username,
				     (string) orig_visname
                         })

At the target mud, the target_user will be paged with a message and a short sound tone if the mud
supports it.

*/
int I3_send_beep( CHAR_DATA *ch, char *to, I3_MUD *mud )
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_beep_commands++;

    I3_escape( to );
    I3_write_header( "beep", I3_THISMUD, CH_NAME(ch), mud->name, to );
    I3_write_buffer( "\"" );
    I3_write_buffer( CH_NAME(ch) );
    I3_write_buffer( "\",})\r" );

    I3_send_packet( );

    return 0;
}

int I3_process_beep( char *s ) 
{
   char *ps = s, *next_ps;
   CHAR_DATA *ch;
   I3_HEADER header;

   I3_get_header( &ps, &header );
   if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
   {
	if( !i3exists_player( header.target_username ) )
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	return 0;
   }

   if( I3ISINVIS(ch) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
   if( i3ignoring( ch, cbuf ) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

#ifdef ALSHEROKCODE
   if( IS_PC_FLAG( ch, PCFLAG_NOBEEP ) )
   {
	sprintf( log_buf, "%s is not accepting beeps.", CH_NAME(ch) );
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", log_buf );
	return 0;
   }
#endif

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   i3_printf( ch, "&Y\a%s@%s beeps you.\n\r", ps, header.originator_mudname );
   return 0;
}

I3DOFUN( do_i3beep )
{
   char *ps;
   char mud[MIL];
   I3_MUD *pmud;

   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }
    
   ps = strchr( argument, '@' );

   if( argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   ps++;
   strcpy( mud, ps );

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->beep == 0 )
	i3_printf( ch, "%s does not support the 'beep' command. Sending anyway.\n\r", pmud->name );

   i3_printf( ch, "&YYou beep %s@%s.\n\r", argument, pmud->name );
   I3_send_beep( ch, argument, pmud );
}

int I3_send_tell( CHAR_DATA *ch, char *to, I3_MUD *mud, char *message )
{
   if( !I3_is_connected() )
	return 0;

   I3_stats.count_tell_commands++;

   I3_escape( to );
   I3_write_header( "tell", I3_THISMUD, CH_NAME(ch), mud->name, to );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_NAME(ch) );
   I3_write_buffer( "\",\"" );
   send_to_i3( message );
   I3_write_buffer( "\",})\r" );

   I3_send_packet( );

   return 0;
}

int I3_process_tell( char *s ) 
{
   char *ps = s, *next_ps;
   CHAR_DATA *ch;
   I3_HEADER header;

   I3_get_header( &ps, &header );
   if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
   {
	if( !i3exists_player( header.target_username ) )
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	return 0;
   }

   if( I3ISINVIS(ch) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
   if( i3ignoring( ch, cbuf ) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( I3NOTELL(ch) )
   {
	sprintf( log_buf, "%s is not accepting tells.", header.target_username );
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", log_buf );
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3_printf( ch, "&Y%s@%s tells you: ", ps, header.originator_mudname );

   sprintf( log_buf, "'%s@%s'", ps, header.originator_mudname );
   if( strlen( log_buf ) < 256 )
	strcpy( I3REPLY(ch), log_buf );
   else
	i3bug( "I3_process_tell: Reply name is too long!" );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3_printf( ch, "&c%s\n\r", ps );
   return 0;
}

void I3_tell( CHAR_DATA *ch, char *argument )
{
   char to[MIL], *ps;
   char mud[MIL];
   I3_MUD *pmud;

   if( I3NOTELL(ch) )
   {
	i3_to_char( "You are not allowed to use i3tell, or your tells are turned off.\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, to );
   ps = strchr( to, '@' );

   if( to[0] == '\0' || argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   ps++;
   strcpy( mud, ps );

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->tell == 0 )
	i3_printf( ch, "%s does not support the 'tell' command. Sending anyway.\n\r", pmud->name );

   i3_printf( ch, "&YYou tell %s@%s: &c%s\n\r", to, pmud->name, argument );
   I3_send_tell( ch, to, pmud, argument );
}

I3DOFUN( do_i3tell )
{
   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   I3_tell( ch, argument );
   return;
}

I3DOFUN( do_i3reply )
{
   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   if( I3NOTELL(ch) )
   {
	i3_to_char( "You are not allowed to use i3reply, or your tells are turned off.\n\r", ch );
	return;
   }

   if( !I3REPLY(ch) )
   {
	i3_to_char( "You have not yet received an i3tell?!?\n\r", ch );
	return;
   }

   sprintf( log_buf, "%s %s", I3REPLY(ch), argument );
   I3_tell( ch, log_buf );
   return;
}

#define COUNTER 3
void I3_printstats( int i ) 
{
    I3_MUD *mud;
    I3_CHANNEL *channel;
    static int counter = -1;

    counter++;

    if( counter%COUNTER == 0 ) 
    {
    printf( "messages	: %d (%d unknown)\n",
					I3_stats.count_total,
					I3_stats.count_unknown );
    printf( "private		: %d tells/%d emotes/%d beeps\n",
					I3_stats.count_tell,
					I3_stats.count_emoteto,
					I3_stats.count_beep );
    printf( "who		: %d/%d\n",	I3_stats.count_who_req,
					I3_stats.count_who_reply );
    printf( "finger		: %d/%d\n",
					I3_stats.count_finger_req,
					I3_stats.count_finger_reply );
    printf( "locate		: %d/%d\n",
					I3_stats.count_locate_req,
					I3_stats.count_locate_reply );
    printf( "channels	: %d m/%d e/%d t\n",
					I3_stats.count_channel_m,
					I3_stats.count_channel_e,
					I3_stats.count_channel_t );
    printf( "		  %d list-reply/%d add/%d remove\n",
					I3_stats.count_chanlist_reply,
					I3_stats.count_channel_add,
					I3_stats.count_channel_remove );
    printf( "		  filter %d/%d\n",
					I3_stats.count_channel_filter_req,
					I3_stats.count_channel_filter_reply );
    printf( "		  who %d/%d\n", I3_stats.count_channel_who_req,
					I3_stats.count_channel_who_reply );
    printf( "		  user %d/%d\n",I3_stats.count_chan_user_req,
					I3_stats.count_chan_user_reply );
    printf( "news		: %d read/%d post/%d grplist_req\n",
					I3_stats.count_news_read_req,
					I3_stats.count_news_post_req,
					I3_stats.count_news_grplist_req );
    printf( "mail		: %d/%d\n",
					I3_stats.count_mail,
					I3_stats.count_mail_ack );
    printf( "filelist	: %d/%d\n",	I3_stats.count_file_list_req,
					I3_stats.count_file_list_reply );
    printf( "file		: %d put/%d getreq/%d getreply\n",
					I3_stats.count_file_put,
					I3_stats.count_file_get_req,
					I3_stats.count_file_get_reply );
    printf( "auth		: %d/%d\n",
					I3_stats.count_auth_mud_req,
					I3_stats.count_auth_mud_reply );
    printf( "startup		: %d/%d\n",
					I3_stats.count_startup_req_3,
					I3_stats.count_startup_reply );
    printf( "oob		: %d req/%d begin/%d end\n",
					I3_stats.count_oob_req,
					I3_stats.count_oob_begin,
					I3_stats.count_oob_end );
    printf( "errors		: %d\n",I3_stats.count_error );
    printf( "mudlist		: %d\n",I3_stats.count_mudlist );
    printf( "shutdown	: %d\n",	I3_stats.count_shutdown );
    printf( "ucache		: %d\n",I3_stats.count_ucache_update );
    printf( "\n" );
    }

    if( counter%COUNTER == 1 ) 
    {
      for( mud = first_mud; mud; mud = mud->next )
	{
	    printf( "%s\n", mud->name );
	}
	printf( "\n" );
	return;
    }

    if( counter%COUNTER == 2 ) 
    {
	for( channel = first_I3chan; channel; channel = channel->next )
	{
	    printf( "%s @ %s\n", channel->I3_name, channel->host_mud );
	}
	printf( "\n" );
	return;
    }
}

int I3_send_who( CHAR_DATA *ch, char *mud ) 
{
    if( !I3_is_connected() )
	return 0;
    I3_stats.count_who_commands++;

    I3_escape( mud );
    I3_write_header( "who-req", I3_THISMUD, CH_NAME(ch), mud, NULL );
    I3_write_buffer( "})\r" );

    I3_send_packet( );

    return 0;
}

/*
   The router will route the packet to another router or to the target
   mud. The target mud returns:
    ({
        (string)  "who-reply",                                         
        (int)     5,                                                   
        (string)  originator_mudname,                                 
        (string)  0,                                  
        (string)  target_mudname,
        (string)  target_username,                                  
        (mixed *) who_data
    })                                                               
   
   where who_data is an array containing an array of the following format
   for each user on the mud:
    ({
        (string)  user_visname,                                      
        (int)     idle_time,                
        (string)  xtra_info          
    })                     

   Each user_visname should specify the user's visual name. idle_time
   should be measured in seconds and xtra_info should be a string.
 */

/* This is where the incoming results of a who-reply packet are processed.
 * Note that rather than just spit the names out, I've copied the packet fields into
 * buffers to be output later. Also note that if it receives an idle value of 9999
 * the normal 30 space output will be bypassed. This is so that muds who want to
 * customize the listing headers in their who-reply packets can do so and the results
 * won't get chopped off after the 30th character. If for some reason a person on
 * the target mud just happens to have been idling for 9999 cycles, their data may
 * be displayed strangely compared to the rest. But I don't expect that 9999 is a very
 * common length of time to be idle either :P
 * Receving an idle value of 19998 may also cause odd results since this is used
 * to indicate receipt of the last line of a who, which is typically the number of
 * visible players found.
 */
int I3_process_who_reply( char *s ) 
{
    char *ps =s, *next_ps, *next_ps2; 
    CHAR_DATA *ch;
    I3_HEADER header;
    char person[MSL], title[MIL];
    int idle;

    I3_get_header( &ps, &header );

    if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
    {
	return 0;
    }

    ps+=2;

    while( 1 ) 
    {
	if( ps[0] == '}' ) 
	{
	    i3_to_char( "No information returned.\n\r", ch );
	    return 0;
	}

	I3_get_field( ps, &next_ps );

	ps += 2;
	I3_get_field( ps, &next_ps2 );
	I3_remove_quotes( &ps );
	sprintf( person, "%s", ps );
	ps = next_ps2;
	I3_get_field( ps, &next_ps2 );
	idle = atoi( ps );
	ps = next_ps2;
	I3_get_field( ps, &next_ps2 );
	I3_remove_quotes( &ps );
	sprintf( title, "%s", ps );
	ps = next_ps2;

      if( idle == 9999 )
         i3_printf( ch, "%s %s\n\r\n\r", person, title );
	else if( idle == 19998 )
	   i3_printf( ch, "\n\r%s %s\n\r", person, title );
      else if( idle == 29997 )
	   i3_printf( ch, "\n\r%s %s\n\r\n\r", person, title );
	else
	   i3_printf( ch, "%s %s\n\r", person, title );

	ps = next_ps;
	if( ps[0] == '}' )
	    break;
    }

    return 0;
}

int i3const_color_str_len( const char *argument )
{
    int  str, count = 0;
    bool IS_COLOR = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
        if ( argument[str] == '&' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else if ( argument[str] == '^' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else
        {
            if ( IS_COLOR == FALSE ) count++;
            else IS_COLOR = FALSE;
        }
    }

    return count;
}

int i3const_color_strnlen( const char *argument, int maxlength )
{
    int str, count = 0;
    bool IS_COLOR = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
        if ( argument[str] == '&' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else if ( argument[str] == '^' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else
        {
            if ( IS_COLOR == FALSE ) count++;
            else IS_COLOR = FALSE;
        }

	if ( count >= maxlength ) break;
    }
    if ( count < maxlength ) return ((str - count) + maxlength);

    str++;
    return str;
}

const char *i3const_color_align( const char *argument, int size, int align )
{
    int space = ( size - i3const_color_str_len( argument ) );
    static char buf[MSL];

    if ( align == ALIGN_RIGHT || i3const_color_str_len( argument ) >= size )
        sprintf( buf, "%*.*s", i3const_color_strnlen( argument, size ),
            i3const_color_strnlen( argument, size ), argument );
    else if ( align == ALIGN_CENTER )
        sprintf( buf, "%*s%s%*s", ( space/2 ),"",argument,
            ((space/2)*2)==space ? (space/2) : ((space/2)+1),"" );
    else
        sprintf( buf, "%s%*s", argument, space, "" );

    return buf;
}

/* You can customize the output of this - to a point. Because of how the I3 packets are formatted
 * you need to send even the listing header as a person+info packet. It should be fairly obvious
 * how to change this around if you really want to. Use the bogusidle variable for the idle time
 * on a divider, like what I've done here for headerbuf. If you wish to subvert this with your
 * own custom who list, add a #define I3CUSTOMWHO to one of your H files, and make your own
 * I3_process_who_req function somewhere else in your code.
 */
#ifndef I3CUSTOMWHO
int I3_process_who_req( char *s ) 
{
   char *ps = s;
   I3_HEADER header;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *person;
   char personbuf[MSL], headerbuf[MSL], tailbuf[MSL], smallbuf[MSL], rank[MSL], clan_name[MSL];
   char buf[300], outbuf[400], stats[20];
   int pcount = 0, amount, xx, yy;
   long int bogusidle = 9999;
       
   I3_get_header( &ps, &header );

   I3_write_header( "who-reply", I3_THISMUD, NULL, header.originator_mudname, header.originator_username );
   I3_write_buffer( "({" );

   I3_write_buffer( "({\"" );
   {
      outbuf[0] = '\0';

      sprintf( buf, "&R-=[ &WPlayers on %s &R]=-", I3_THISMUD );           
      amount = 78 - I3_strlen_color( buf ); /* Determine amount to put in front of line */

      if( amount < 1 )
         amount = 1;

      amount = amount / 2;

      for( xx = 0 ; xx < amount ; xx++ )
         strcat( outbuf, " " );

      strcat( outbuf, buf );
      send_to_i3( I3_escape( outbuf ) );
   }

   I3_write_buffer( "\"," );
   sprintf( smallbuf, "%ld", -1l );
   I3_write_buffer( smallbuf );

   I3_write_buffer( ",\"" );
   send_to_i3( "&x-" );

   I3_write_buffer( "\",}),({\"" );
   {
      outbuf[0] = '\0';

      sprintf( buf, "&Y-=[ &Wtelnet://%s:%d &Y]=-", this_mud->telnet, this_mud->player_port );           
      amount = 78 - I3_strlen_color( buf ); /* Determine amount to put in front of line */

      if( amount < 1 )
         amount = 1;

      amount = amount / 2;

      for( xx = 0 ; xx < amount ; xx++ )
         strcat( outbuf, " " );

      strcat( outbuf, buf );
      send_to_i3( I3_escape( outbuf ) );
   }

   I3_write_buffer( "\"," );
   sprintf( smallbuf, "%ld", bogusidle );
   I3_write_buffer( smallbuf );

   I3_write_buffer( ",\"" );
   send_to_i3( "&x-" );

   I3_write_buffer( "\",})," );

   xx = 0;
   for( d = first_descriptor; d; d = d->next )
   {
	if( d->character && d->connected == CON_PLAYING )
	{
	   if( CH_LEVEL(d->character) >= LEVEL_IMMORTAL )
	      continue;

         if( CH_LEVEL(d->character) < this_mud->minlevel )
	      continue;

	   if( I3ISINVIS(d->character) )
	      continue;

    	   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
    	   if( i3ignoring( d->character, cbuf ) )
            continue;

         xx++;
	}
   }

   if( xx > 0 )
   {
      I3_write_buffer( "({\"" );
      strcpy( headerbuf, "&B--------------------------------=[ &WPlayers &B]=---------------------------------" );
      send_to_i3( I3_escape( headerbuf ) );
      I3_write_buffer( "\"," );
      sprintf( smallbuf, "%ld", bogusidle );
      I3_write_buffer( smallbuf );
      I3_write_buffer( ",\"" );
      send_to_i3( "&x-" );
      I3_write_buffer( "\",})," );

      /* This section is displaying only players - not imms */
      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
	      if( CH_LEVEL(d->character) >= LEVEL_IMMORTAL )
		   continue;

            if( CH_LEVEL(d->character) < this_mud->minlevel )
		   continue;

	      if( I3ISINVIS(d->character) )
		   continue;

    	      sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
    	      if( i3ignoring( d->character, cbuf ) )
		   continue;

            person = d->character;
            pcount++;

	      I3_write_buffer( "({\"" );

	      sprintf( rank, "%s", rankbuffer( person ) );
		sprintf( outbuf, "%s", i3const_color_align( rank, 20, ALIGN_CENTER ) );
            send_to_i3( I3_escape( outbuf ) );

	      I3_write_buffer( "\"," );
	      sprintf( smallbuf, "%ld", -1l );
	      I3_write_buffer( smallbuf );
	      I3_write_buffer( ",\"" );
	    
            strcpy( stats, "&z[" );
            if( CH_AFK(person) )
               strcat( stats, "AFK" );
            else
               strcat( stats, "---" );
            if( CH_PK(person) )
	         strcat( stats, "PK" );
	      else
		   strcat( stats, "--" );
            strcat( stats, "]&G" );

	      if( CH_CLAN(person) )
	      {
		   strcpy( clan_name, " &c[" );

	  	   strcat( clan_name, CH_CLANNAME(person) );
        	   strcat( clan_name, "&c]" );
	      }
	      else
	         clan_name[0] = '\0';

	      sprintf( personbuf, "%s %s%s%s", stats, CH_NAME(person), CH_TITLE(person), clan_name );
	      send_to_i3( I3_escape( personbuf ) );
    	      I3_write_buffer( "\",})," );
	   }
      }
   }

   yy = 0;
   for( d = first_descriptor; d; d = d->next )
   {
	if( d->character && d->connected == CON_PLAYING )
	{
	   if( CH_LEVEL(d->character) < LEVEL_IMMORTAL )
	      continue;

	   if( I3ISINVIS(d->character) )
	      continue;

    	   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
    	   if( i3ignoring( d->character, cbuf ) )
            continue;

         yy++;
	}
   }

   if( yy > 0 )
   {
      I3_write_buffer( "({\"" );
      strcpy( headerbuf, "&R-------------------------------=[ &WImmortals &R]=--------------------------------" );
      send_to_i3( I3_escape( headerbuf ) );
      I3_write_buffer( "\"," );
	if( xx > 0 )
	   sprintf( smallbuf, "%ld", bogusidle * 3 );
	else
         sprintf( smallbuf, "%ld", bogusidle );
      I3_write_buffer( smallbuf );
      I3_write_buffer( ",\"" );
      send_to_i3( "&x-" );
      I3_write_buffer( "\",})," );

      /* This section is displaying only immortals, not players */
      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
	      if( CH_LEVEL(d->character) < LEVEL_IMMORTAL )
		   continue;

	      if( I3ISINVIS(d->character) )
		   continue;

    	      sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
    	      if( i3ignoring( d->character, cbuf ) )
		   continue;

            person = d->character;
            pcount++;

	      I3_write_buffer( "({\"" );

	      sprintf( rank, "%s", rankbuffer( person ) );
		sprintf( outbuf, "%s", i3const_color_align( rank, 20, ALIGN_CENTER ) );
            send_to_i3( I3_escape( outbuf ) );

	      I3_write_buffer( "\"," );
	      sprintf( smallbuf, "%ld", -1l );
	      I3_write_buffer( smallbuf );
	      I3_write_buffer( ",\"" );
	    
            strcpy( stats, "&z[" );
            if( CH_AFK(person) )
               strcat( stats, "AFK" );
            else
               strcat( stats, "---" );
            if( CH_PK(person) )
	         strcat( stats, "PK" );
	      else
		   strcat( stats, "--" );
            strcat( stats, "]&G" );

	      if( CH_CLAN(person) )
	      {
		   strcpy( clan_name, " &c[" );

	  	   strcat( clan_name, CH_CLANNAME(person) );
        	   strcat( clan_name, "&c]" );
	      }
	      else
	         clan_name[0] = '\0';

	      sprintf( personbuf, "%s %s%s%s", stats, CH_NAME(person), CH_TITLE(person), clan_name );
	      send_to_i3( I3_escape( personbuf ) );
    	      I3_write_buffer( "\",})," );
	   }
      }
   }

   I3_write_buffer( "({\"" );
   sprintf( tailbuf, "&Y[&W%d Player%s&Y]", pcount, pcount == 1 ? "" : "s" );
   send_to_i3( I3_escape( tailbuf ) );
   I3_write_buffer( "\"," );
   sprintf( smallbuf, "%ld", bogusidle * 2 );
   I3_write_buffer( smallbuf );
   I3_write_buffer( ",\"" );
   sprintf( tailbuf, "&Y[&WHomepage: http://%s&Y] [&W%3d Max Since Reboot&Y]", this_mud->web, I3MAXPLAYERS );
   send_to_i3( I3_escape( tailbuf ) );
   I3_write_buffer( "\",}),}),})\r" );

   I3_send_packet( );

   return 0;
}
#endif

int I3_send_emoteto( CHAR_DATA *ch, char *to, I3_MUD *mud, char *message ) 
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_emoteto_commands++;

    I3_escape( to );
    I3_write_header( "emoteto", I3_THISMUD, CH_NAME(ch), mud->name, to );
    I3_write_buffer( "\"" );
    I3_write_buffer( CH_NAME(ch) );
    I3_write_buffer( "\",\"" );
    send_to_i3( message );
    I3_write_buffer( "\",})\r" );

    I3_send_packet( );

    return 0;
}

int I3_process_emoteto( char *s ) 
{
   CHAR_DATA *ch;
   I3_HEADER header;
   char *ps = s, *next_ps;
   char visname[MIL];
   char message[MSL], msg[MSL];

   I3_get_header( &ps, &header );

   if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
   {
	if( !i3exists_player( header.target_username ) )
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	else 
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	return 0;
   }

   if( I3ISINVIS(ch) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
   if( i3ignoring( ch, cbuf ) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   sprintf( visname, "%s@%s", ps, header.originator_mudname );

   ps = next_ps;  
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   strcpy( message, ps );

   if( strstr( message, "$N" ) == NULL )
	strcat( message, " (from $N)" );

   sprintf( msg, "%s", I3_convert_channel_message( message, visname, visname ) );

   i3_to_char( msg, ch );

   return 0;
}

int I3_send_finger( CHAR_DATA *ch, char *user, char *mud ) 
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_finger_commands++;

    I3_escape( mud );
    I3_escape( user );

    I3_write_header( "finger-req", I3_THISMUD, CH_NAME(ch), mud, NULL );
    I3_write_buffer( "\"" );
    I3_write_buffer( user );
    I3_write_buffer( "\",})\r" );

    I3_send_packet( );

    return 0;
}

/*
   The target mud will return:
    ({
        (string) "finger-reply",     
        (int)    5,
        (string) originator_mudname, 
        (string) 0,
        (string) target_mudname,
        (string) target_username,
        (string) visname,
        (string) title,
        (string) real_name,
        (string) e_mail,
        (string) loginout_time,
        (int)    idle_time,
        (string) ip_name,     
        (string) level,             
        (string) extra
    })

   A mud may return 0 for any item if they wish to keep the information
   private. In particular, it is suggested that information about players
   (as opposed to wizards) be kept confidential.

   The returned visname should contain the user's visual name.
   loginout_time specifies the (local) time the user logged in (if they
   are currently on) or the time the user logged out. The value should be
   expressed as a string. It should be 0 to indicate no information. The
   idle_time is expressed as an integer number of seconds of idle time.  
   If this value is -1, then the user is not logged onto the mud at the
   moment.

   If extra is given, then it should be terminated with a carriage
   return.                                                           
*/

/* The output of this was slightly modified to resemble the Finger snippet */
int I3_process_finger_reply( char *s ) 
{
    I3_HEADER header;
    CHAR_DATA *ch;
    char *ps = s, *next_ps;
    char title[MSL], homepage[MSL], email[MSL], last[MSL], level[MSL];

    I3_get_header( &ps, &header );
    if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
    {
	return 0;
    }

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    i3_printf( ch, "&wI3FINGER information for &G%s@%s\n\r", ps, header.originator_mudname );
    i3_to_char( "&w-------------------------------------------------\n\r", ch );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    sprintf( title, "%s", ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    sprintf( email, "%s", ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    sprintf( last, "%s", ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    sprintf( level, "%s", ps );
    ps = next_ps;

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
    sprintf( homepage, "%s", ps );

    i3_printf( ch, "&wTitle: &G%s\n\r", title );
    i3_printf( ch, "&wLevel: &G%s\n\r", level );
    i3_printf( ch, "&wEmail: &G%s\n\r", email );
    i3_printf( ch, "&wHTTP : &G%s\n\r", homepage );
    i3_printf( ch, "&wLast on: &G%s\n\r", last );

    return 0;
}

/*
    ({
        (string) "finger-req",       
        (int)    5,
        (string) originator_mudname, 
        (string) originator_username,
        (string) target_mudname,
        (string) 0,   
        (string) username
    })

*/
int I3_process_finger_req( char *s ) 
{
   I3_HEADER header;
   CHAR_DATA *ch;
   char *ps = s, *next_ps;
   char smallbuf[MSL];

   I3_get_header( &ps, &header );
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   if( ( ch = I3_find_user( ps ) ) == NULL ) 
   {
	if( !i3exists_player( ps ) )
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "No such player." );
	return 0;
   }

   if( I3ISINVIS(ch) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
   if( i3ignoring( ch, cbuf ) )
   {
      I3_send_error( header.originator_mudname, header.originator_username, "unk-user", "That player is offline." );
	return 0;
   }

   i3_printf( ch, "%s@%s has requested your i3finger information.\n\r",
	header.originator_username, header.originator_mudname );

   I3_write_header( "finger-reply", I3_THISMUD, NULL, header.originator_mudname, header.originator_username );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_escape( CH_NAME(ch) ) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( I3_escape( CH_NAME(ch) ) );
   send_to_i3( I3_escape( CH_TITLE(ch) ) );
   I3_write_buffer( "\",\"\",\"" );			// real name
#ifdef FINGERCODE
   if( ch->pcdata->email ) 
	I3_write_buffer( ch->pcdata->email );			// email address
   else 
	I3_write_buffer( "Not provided" );			// email address
#else
   I3_write_buffer( "Not supported" );
#endif
   I3_write_buffer( "\",\"" );
#ifdef I3CIRCLE
   I3_write_buffer( "-1" );	// online since
#else
   strcpy( smallbuf, ctime( &ch->logon ) ); // online since
   I3_write_buffer( smallbuf );
#endif
   I3_write_buffer( "\"," );
   sprintf( smallbuf, "%ld", -1l );
   I3_write_buffer( smallbuf );			// idle since
   I3_write_buffer( ",\"" );
   I3_write_buffer( "[PRIVATE]" );			// IP address
   I3_write_buffer( "\",\"" );
   sprintf( log_buf, "%s", rankbuffer( ch ) );
   send_to_i3( log_buf );
#ifdef FINGERCODE
   I3_write_buffer( "\",\"" );
   if( ch->pcdata->homepage )
      I3_write_buffer( I3_escape( ch->pcdata->homepage ) );
   else
	I3_write_buffer( "Not Provided" );
   I3_write_buffer( "\",})\r" );
#else
   I3_write_buffer( "\",\"Not Suported\",})\r" );		// No extra info
#endif

   I3_send_packet( );

   return 0;
}

int I3_send_locate( CHAR_DATA *ch, char *user )
{
   if( !I3_is_connected() )
	return 0;
   I3_stats.count_locate_commands++;

   I3_escape( user );
   I3_write_header( "locate-req", I3_THISMUD, CH_NAME(ch), NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( user );
   I3_write_buffer( "\",})" );

   I3_send_packet( );

   return 0;
}

/*
   If the requested user is logged into the receiving mud, then the
   following reply is returned:
    ({
        (string) "locate-reply",     
        (int)    5,
        (string) originator_mudname, 
        (string) 0,
        (string) target_mudname,
        (string) target_username,
        (string) located_mudname,
        (string) located_visname,
        (int)    idle_time,
        (string) status,
    })   

   located_visname should contain the correct visual name of the user.
   idle_time will have the idle time (in seconds) of the located user.

   status specifies any special status of the user. This will typically
   be zero to indicate that the user has no special status. The values
   for this string are arbitrary, but certain statuses have predefined
   values that can be used:                                      
     * "link-dead"                                      
     * "editing"                                                      
     * "inactive"
     * "invisible"                                                     
     * "hidden"                                                 
*/

int I3_process_locate_reply( char *s ) 
{
   char mud_name[MSL], user_name[MSL], status[MSL];
   char *ps = s, *next_ps;
   CHAR_DATA *ch;
   I3_HEADER header;

   I3_get_header( &ps, &header );

   if( ( ch = I3_find_user( header.target_username ) ) == NULL ) 
   {
	return 0;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   sprintf( mud_name, ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   sprintf( user_name, ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   sprintf( status, ps );
    
   if( !str_cmp( status, "active" ) )
	sprintf( status, "Online." );

   if( !str_cmp( status, "exists, but not logged on" ) )
	sprintf( status, "Offline." );

   i3_printf( ch, "&RI3 Locate: &Y%s@%s: &c%s\n\r", user_name, mud_name, status );
   return 0;
}

int I3_process_locate_req( char *s ) 
{
   char *ps = s, *next_ps;
   I3_HEADER header;
   char smallbuf[MSL];
   CHAR_DATA *ch;
   bool choffline = FALSE;

   I3_get_header( &ps, &header );
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( ( ch = I3_find_user( ps ) ) == NULL ) 
   {
	if( i3exists_player( ps ) )
	   choffline = TRUE;
	else
	   return 0;
   }

   if( ch )
   {
      if( CH_LEVEL(ch) < this_mud->minlevel )
         return 0;

      if( I3ISINVIS(ch) )
         choffline = TRUE;

      sprintf( cbuf, "%s@%s", header.originator_username, header.originator_mudname );
      if( i3ignoring( ch, cbuf ) )
	   choffline = TRUE;
   }

   sprintf( log_buf, "I3 Locate: Responding to %s@%s with status of %s.", header.originator_username, header.originator_mudname, ps );
   i3log( log_buf );

   I3_write_header( "locate-reply", I3_THISMUD, NULL, header.originator_mudname, header.originator_username );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_THISMUD );
   I3_write_buffer( "\",\"" );
   if( !choffline )
      I3_write_buffer( CH_NAME(ch) );
   else
	I3_write_buffer( ps );
   I3_write_buffer( "\"," );
   sprintf( smallbuf, "%ld", -1l );
   I3_write_buffer( smallbuf );
   if( !choffline )
      I3_write_buffer( ",\"active\",})" );
   else
	I3_write_buffer( ",\"exists, but not logged on\",})" );

   I3_send_packet( );

   return 0;
}

/*
   A mud may decide whether or not it is listening to any given channel
   by sending a channel-listen packet. This packet is also used to tune
   out a channel, which should be done whenever no one on the mud is
   listening to the channel. The format of this packet is:
    ({
        (string) "channel-listen",
        (int)    5,                                                    
        (string) originator_mudname,                            
        (string) 0,
        (string) target_mudname,         // the router             
        (string) 0,                                                    
        (string) channel_name,
        (int)    on_or_off
    })                                                                   
     
   The on_or_off will contain one of the following values:               
        0 The mud does not wish to receive this channel.       
        1 The mud wishes to receive this channel.
*/
int I3_send_channel_listen( I3_CHANNEL *channel, bool lconnect ) 
{
    if( !I3_is_connected() )
	return 0;

    I3_stats.count_channel_listen++;

    I3_write_header( "channel-listen", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL );
    I3_write_buffer( "\"" );
    I3_write_buffer( channel->I3_name );
    I3_write_buffer( "\"," );
    if( lconnect )
	I3_write_buffer( "1,})\r" );
    else
	I3_write_buffer( "0,})\r" );

    I3_send_packet( );

    return 0;
}

void I3_check_channel( I3_CHANNEL *channel, bool reconnect ) 
{
   /* The reconnect condition should only be called after a successful reconnection */
   if( reconnect )
   {
	I3_send_channel_listen( channel, FALSE );
	channel->connected = FALSE;
	if( channel->first_listener != NULL )
	{
	   I3_send_channel_listen( channel, TRUE );
	   channel->connected = TRUE;
	   sprintf( log_buf, "I3_check_channel: Resetting channel connection for %s.", channel->local_name );
	   i3log( log_buf );
	}
	return;
   }
   else
   {
      if( channel->first_listener == NULL && channel->connected == TRUE ) 
      {
	   I3_send_channel_listen( channel, FALSE );
	   sprintf( log_buf, "I3: check_channel: unsubscribing from %s (%s@%s)",
	      channel->local_name, channel->I3_name, channel->host_mud );
	   i3log( log_buf );
	   channel->connected = FALSE;
	   return;
      }
      if( channel->first_listener != NULL && channel->connected == FALSE )
      {
	   I3_send_channel_listen( channel, TRUE );
	   sprintf( log_buf, "I3: check_channel: subscribing to %s (%s@%s)",
	      channel->local_name, channel->I3_name, channel->host_mud );
	   i3log( log_buf );
	   channel->connected = TRUE;
	   return;
      }
      return;
   }
}

#if defined(KEY)
#undef KEY
#endif
#if defined(SKEY)
#undef SKEY
#endif

#define KEY( literal, field, value )				\
				if ( !str_cmp( word, literal ) )	\
				{						\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

#define SKEY( literal, field, value )  		\
	if ( !str_cmp( word, literal ) ) 		\
	{ 							\
	   strcpy( field, value );			\
	   fMatch = TRUE; 				\
	   break; 						\
	}

#ifdef I3ACK
#define ACKKEY( literal, field, value )		\
	if( !str_cmp( word, literal ) ) 		\
	{ 							\
	   if (field!=NULL) free_string(field); 	\
	   field  = value; 				\
	   fMatch = TRUE; 				\
	   break; 						\
	}
#endif
  
/*
 * Read a string from file fp using str_dup [Taken from Smaug's fread_string_nohash]
 */
char *i3fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    i3log( "i3fread_string: EOF encountered on read.\n\r" );
	    return str_dup("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return str_dup( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   i3log( "i3fread_string: string too long" );
	   *plast = '\0';
	   return str_dup( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    i3log( "i3fread_string: EOF" );
	    *plast = '\0';
	    return str_dup(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return str_dup( buf );
	}
    }
}

/*
 * Read a number from a file. [Taken from Smaug's fread_number]
 */
int i3fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          i3log( "fread_number: EOF encountered on read." );
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	sprintf( log_buf, "i3fread_number: bad format. (%c)", c );
	i3log( log_buf );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          i3log( "fread_number: EOF encountered on read." );
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += i3fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * Read to end of line into static buffer [Taken from Smaug's fread_line]
 */
char *i3fread_line( FILE *fp )
{
    static char line[MSL];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    i3bug( "i3fread_line: EOF encountered on read.\n\r" );
	    strcpy(line, "");
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    i3bug( "i3fread_line: EOF encountered on read.\n\r" );
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (MSL - 1) )
	{
	    i3bug( "i3fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}

/*
 * Read one word (into static buffer). [Taken from Smaug's fread_word]
 */
char *i3fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    i3log( "i3fread_word: EOF encountered on read." );
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	if ( feof(fp) )
	{
	    i3log( "i3fread_word: EOF encountered on read." );
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    i3log( "i3fread_word: word too long" );
    return NULL;
}

/*
 * Read a letter from a file. [Taken from Smaug's fread_letter]
 */
char i3fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          i3log( "i3fread_letter: EOF encountered on read." );
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}

/*
 * Read to end of line (for comments). [Taken from Smaug's fread_to_eol]
 */
void i3fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    i3log( "i3fread_to_eol: EOF encountered on read." );
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

bool i3load_char( CHAR_DATA *ch, FILE *fp, char *word )
{
   bool fMatch = FALSE;

   switch( UPPER(word[0]) )
   {
	case 'I':
#ifdef I3ACK
	   ACKKEY( "i3listen",		I3LISTEN(ch),		i3fread_string( fp ) );
#endif
         KEY( "i3listen",		I3LISTEN(ch),		i3fread_string( fp ) );
         KEY( "i3invis",		I3INVIS(ch),		i3fread_number( fp ) );
         if( !str_cmp( word, "i3ignore" ) )
         {
            I3_IGNORE *temp;

	      CREATE( temp, I3_IGNORE, 1 );
	      strcpy( temp->name, i3fread_string( fp ) );
	      LINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );

	      fMatch = TRUE;
	      break;
         }
	break;
   }
   return fMatch;
}

#ifdef I3CIRCLE
void i3save_char( CHAR_DATA *ch, FBFILE *fp )
#else
void i3save_char( CHAR_DATA *ch, FILE *fp )
#endif
{
   I3_IGNORE *temp;

#ifndef I3CIRCLE
   fprintf( fp, "i3invis	%d\n", I3INVIS(ch) );
   if( I3LISTEN(ch) && str_cmp( I3LISTEN(ch), "" ) )
      fprintf( fp, "i3listen	%s~\n", I3LISTEN(ch) );
   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
      fprintf( fp, "i3ignore	%s~\n", temp->name );
#else
   fbprintf( fp, "I3iv:	%d\n", I3INVIS(ch) );
   if( I3LISTEN(ch) && str_cmp( I3LISTEN(ch), "" ) )
	fbprintf( fp, "I3li:	%s~\n", I3LISTEN(ch) );
   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
	fbprintf( fp, "I3ig:	%s~\n", temp->name );
#endif
   return;
}

void I3_readucache( UCACHE_DATA *user, FILE *fp )
{
   char *word;
   bool fMatch;

   for ( ; ; )
   {
	word   = feof( fp ) ? "End" : i3fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fp );
	      break;

	   case 'N':
		SKEY( "Name",		user->name,		i3fread_string( fp ) );
		break;

         case 'S':
		KEY( "Sex",			user->gender,	i3fread_number( fp ) );
		break;

	   case 'T':
		KEY( "Time",		user->time,		i3fread_number( fp ) );
		break;

	   case 'E':
	      if ( !str_cmp( word, "End" ) )
	      {
		   return;
	      }
	      break;
	}

	if ( !fMatch )
	{
	    sprintf( log_buf, "I3_readucache: no match: %s", word );
	    i3bug( log_buf );
	}
   }
}

void I3_load_ucache( void )
{
    FILE *fp;
    UCACHE_DATA *user;

    i3log( "Loading I3 ucache data" );

    if( ( fp = fopen( I3_UCACHE_FILE, "r" ) ) == NULL ) 
    {
	i3log( "No I3 ucache data found." );
	return;
    }

    for ( ; ; )
    {
	char letter;
	char *word;

	letter = i3fread_letter( fp );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fp );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "I3_load_ucahe: # not found." );
	   break;
	}

      word = i3fread_word( fp );
	if ( !str_cmp( word, "UCACHE" ) )
	{
	   CREATE( user, UCACHE_DATA, 1 );
	   I3_readucache( user, fp );
	   LINK( user, first_ucache, last_ucache, next, prev );
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   sprintf( log_buf, "I3_load_ucache: bad section: %s.", word );
	   i3bug( log_buf );
	   continue;
	}
    }
    FCLOSE( fp );
    return;
}

void I3_fread_config_file( FILE *fin )
{
   char *word;
   bool fMatch;

   for( ;; )
   {
	word   = feof( fin ) ? "end" : i3fread_word( fin );
	fMatch = FALSE;
	
	switch( word[0] ) 
	{
	   case '#':
		fMatch = TRUE;
		i3fread_to_eol( fin );
		break;
	   case 'a':
		SKEY( "adminemail",	this_mud->admin_email,	i3fread_string( fin ) );
		KEY( "amrcp",		this_mud->amrcp,		i3fread_number( fin ) );
		KEY( "auth",		this_mud->auth,		i3fread_number( fin ) );
		KEY( "autoconnect",	this_mud->autoconnect,	i3fread_number( fin ) );
		break;
	   case 'b':
		SKEY( "basemudlib",	this_mud->base_mudlib,	i3fread_string( fin ) );
		KEY( "beep",		this_mud->beep,		i3fread_number( fin ) );
		break;
	   case 'c':
		KEY( "channel",		this_mud->channel,	i3fread_number( fin ) );
		break;
	   case 'd':
		SKEY( "driver",		this_mud->driver,		i3fread_string( fin ) );
		break;
	   case 'e':
		KEY( "emoteto",		this_mud->emoteto,	i3fread_number( fin ) );
		if( !str_cmp( word, "end" ) )
		{
		   return;
		}
		break;
	   case 'f':
		KEY( "file",		this_mud->file,		i3fread_number( fin ) );
		KEY( "finger",		this_mud->finger,		i3fread_number( fin ) );
		KEY( "ftp",			this_mud->ftp,		i3fread_number( fin ) );
		break;
	   case 'h':
		KEY( "http",		this_mud->http,		i3fread_number( fin ) );
		break;
	   case 'l':
		KEY( "locate",		this_mud->locate,		i3fread_number( fin ) );
		break;
	   case 'm':
		KEY( "mail",		this_mud->mail,		i3fread_number( fin ) );
		KEY( "minlevel",		this_mud->minlevel,	i3fread_number( fin ) );
		SKEY( "mudlib",		this_mud->mudlib,		i3fread_string( fin ) );
		SKEY( "mudtype",		this_mud->mud_type,	i3fread_string( fin ) );
		break;
	   case 'n':
		KEY( "news",		this_mud->news,		i3fread_number( fin ) );
		KEY( "nntp",		this_mud->nntp,		i3fread_number( fin ) );
		break;
	   case 'o':
		SKEY( "openstatus",	this_mud->open_status,	i3fread_string( fin ) );
		break;
	   case 'p':
		KEY( "port",		this_mud->routerPort,	i3fread_number( fin ) );
		KEY( "pop3",		this_mud->pop3,		i3fread_number( fin ) );
		break;
	   case 'r':
		KEY( "rcp",			this_mud->rcp,		i3fread_number( fin ) );
		SKEY( "router",		this_mud->routerIP,	i3fread_string( fin ) );
		SKEY( "routername",	this_mud->routerName,	i3fread_string( fin ) );
		break;
	   case 's':
		KEY( "smtp",		this_mud->smtp,		i3fread_number( fin ) );
		break;
	   case 't':
		KEY( "tell",		this_mud->tell,		i3fread_number( fin ) );
		SKEY( "telnet",		this_mud->telnet,		i3fread_string( fin ) );
		SKEY( "thismud",		this_mud->name,		i3fread_string( fin ) );
		break;
	   case 'u':
		KEY( "ucache",		this_mud->ucache,		i3fread_number( fin ) );
		break;
	   case 'w':
		SKEY( "web",		this_mud->web,		i3fread_string( fin ) );
		KEY( "who",			this_mud->who,		i3fread_number( fin ) );
		break;
	}

	if( !fMatch ) 
	{
	   sprintf( log_buf, "I3_fread_config_file: Bad keyword: %s\n\r", word );
	   i3bug( log_buf );
	}
   }
}

bool I3_read_config( int mudport ) 
{
   FILE *fin, *fp;

   i3log( "Loading Intermud-3 network data..." );

   if( ( fin = fopen( I3_CONFIG_FILE, "r" ) ) == NULL ) 
   {
	i3log( "Can't open configuration file: i3.config" );
	i3log( "Network configuration aborted." );
	return FALSE;
   }

   CREATE( this_mud, I3_MUD, 1 );

   strcpy( this_mud->ipaddress, "127.0.0.1" );
   this_mud->status 	 = -1;
   this_mud->autoconnect = 0;
   this_mud->player_port = mudport; /* Passed in from the mud's startup script */
   this_mud->password 	 = 0;
   this_mud->mudlist_id  = 0;
   this_mud->chanlist_id = 0;
   this_mud->minlevel 	 = 10; /* Minimum default level before I3 will acknowledge you exist */

   if( ( fp = fopen( I3_PASSWORD_FILE, "r" ) ) != NULL )
   {
 	char *word;

      word = i3fread_word( fp );

      if( !str_cmp( word, "#PASSWORD" ) )
	{
	   char *ln = i3fread_line( fp );
	   int pass, mud, chan;

	   pass = mud = chan = 0;
	   sscanf( ln, "%d %d %d", &pass, &mud, &chan );
	   this_mud->password 	 = pass;
	   this_mud->mudlist_id  = mud;
	   this_mud->chanlist_id = chan;
	}
      FCLOSE( fp );
   }

   for( ; ; )
   {
    	char letter;
 	char *word;

   	letter = i3fread_letter( fin );

	if( letter == '#' )
	{
	   i3fread_to_eol( fin );
	   continue;
      }

	if( letter != '$' )
	{
	   i3bug( "I3_read_config: $ not found" );
	   break;
	}

	word = i3fread_word( fin );
	if( !str_cmp( word, "I3CONFIG" ) )
	{
	   I3_fread_config_file( fin );
	   continue;
	}
      else if( !str_cmp( word, "END" ) )
	   break;
	else
	{
	   sprintf( log_buf, "I3_read_config: Bad section in config file: %s", word );
	   i3bug( log_buf );
	   continue;
      }
   }
   FCLOSE( fin );

   if( !this_mud->name || this_mud->name[0] == '\0' )
   {
	i3bug( "I3_read_config: Mud name not loaded in configuration file." );
	i3log( "Network configuration aborted." );
	destroy_I3_mud( this_mud );
	return FALSE;
   }

   if( !this_mud->routerName || this_mud->routerName[0] == '\0' )
   {
	i3bug( "I3_read_config: No router name loaded in config file." );
	i3log( "Network configuration aborted." );
	destroy_I3_mud( this_mud );
	return FALSE;
   }

   if( !this_mud->telnet || this_mud->telnet[0] == '\0' )
	strcpy( this_mud->telnet, "Address not configured" );

   if( !this_mud->web || this_mud->web[0] == '\0' )
	strcpy( this_mud->web, "Address not configured" );

   strcpy( I3_THISMUD, this_mud->name );
   strcpy( I3_ROUTER_NAME, this_mud->routerName );
   return TRUE;
}

void I3_readchannel( I3_CHANNEL *channel, FILE *fin ) 
{
     char *word;
     bool fMatch;

     for ( ; ; )
     {
	word   = feof( fin ) ? "End" : i3fread_word( fin );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	  case '*':
	    fMatch = TRUE;
	    i3fread_to_eol( fin );
	    break;

	  case 'C':
		SKEY( "ChanName",		channel->I3_name,		i3fread_string( fin ) );
		SKEY( "ChanMud",		channel->host_mud,	i3fread_string( fin ) );
		SKEY( "ChanLocal",	channel->local_name,	i3fread_string( fin ) );
		SKEY( "ChanLayM",		channel->layout_m,	i3fread_string( fin ) );
		SKEY( "ChanLayE",		channel->layout_e,	i3fread_string( fin ) );
		KEY( "ChanLevel",		channel->local_level,	i3fread_number( fin ) );
		KEY( "ChanStatus",		channel->status,	i3fread_number( fin ) );
		break;

	  case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		return;
	    }
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( log_buf, "I3_readchannel: no match: %s", word );
	    i3bug( log_buf );
	}
     }
}

void I3_loadchannels( void )
{
    FILE *fin;
    I3_CHANNEL *channel;

    first_I3chan = NULL;
    last_I3chan = NULL;

    i3log( "Loading I3 channels" );

    if( ( fin = fopen( I3_CHANNEL_FILE, "r" ) ) == NULL ) 
    {
	i3log( "No I3 channel config file found." );
	return;
    }

    for ( ; ; )
    {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "I3_loadchannels: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if ( !str_cmp( word, "I3CHAN" ) )
	{
	   CREATE( channel, I3_CHANNEL, 1 );
	   I3_readchannel( channel, fin );

	   channel->first_listener = NULL;
	   channel->last_listener = NULL;
	   channel->connected = FALSE;
	   LINK( channel, first_I3chan, last_I3chan, next, prev );

	   sprintf( log_buf, "I3: Auto-subscribing to %s (%s@%s)", channel->local_name, channel->I3_name, channel->host_mud );
	   i3log( log_buf );
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   sprintf( log_buf, "I3_loadchannels: bad section: %s.", word );
	   i3bug( log_buf );
	   continue;
	}
    }
    FCLOSE( fin );
    return;
}

void I3_write_channel_config( void ) 
{
    FILE *fout;
    I3_CHANNEL *channel;

    if( ( fout = fopen( I3_CHANNEL_FILE, "w" ) ) == NULL ) 
    {
	i3log( "Couldn't write to I3 channel config file." );
	return;
    }

    for( channel = first_I3chan; channel; channel = channel->next ) 
    {
	if( channel->local_name && channel->local_name[0] ) 
      {
	    fprintf( fout, "#I3CHAN\n" );
	    fprintf( fout, "ChanName   %s~\n", channel->I3_name );
    	    fprintf( fout, "ChanMud    %s~\n", channel->host_mud );
	    fprintf( fout, "ChanLocal  %s~\n", channel->local_name );
	    fprintf( fout, "ChanLayM   %s~\n", channel->layout_m );
	    fprintf( fout, "ChanLayE   %s~\n", channel->layout_e );
	    fprintf( fout, "ChanLevel  %d\n", channel->local_level );
	    fprintf( fout, "ChanStatus %d\n", channel->status );
	    fprintf( fout, "End\n\n" );
	}
    }
    fprintf( fout, "#END\n" );
    FCLOSE( fout );
}

void I3_mudlist( CHAR_DATA *ch, char *argument ) 
{
    I3_MUD *mud;
    char buf[MSL], outbuf[MSL];
    char filter[MIL];
    int mudcount = 0;
    bool all=FALSE;

    if( IS_NPC(ch) ) 
	return;

    argument = i3one_argument( argument, filter );

    if( !str_cmp( filter, "all" ) ) 
    {
	all = TRUE;
	argument = i3one_argument( argument, filter );
    }

    if( first_mud == NULL )
    {
	i3_to_char( "There are no muds to list!?\n\r", ch );
	return;
    }

    i3pager_printf( ch, "%-30s%-7.7s%-22.22s%-15.15s %s\n\r", "Name", "Type", "Mudlib", "Address", "Port" );
    for( mud = first_mud; mud; mud = mud->next ) 
    {
	if( mud == NULL )
	{
	   i3bug( "I3_mudlist: NULL mud found in listing!" );
	   continue;
	}

	if( mud->name == NULL )
	{
	   i3bug( "I3_mudlist: NULL mud name found in listing!" );
         continue;
	}

      if( filter[0] &&
        str_prefix( filter, mud->name ) &&
        ( mud->mud_type && str_prefix( filter, mud->mud_type ) ) &&
        ( mud->mudlib && str_prefix( filter, mud->mudlib ) ) )
	   continue;

	if( !all && mud->status == 0 )
	    continue;

	mudcount++;

	switch( mud->status )
	{
	case -1:
          sprintf( buf, "%-30s%-7.7s%-22.22s%-15.15s %d\n\r",
             mud->name, mud->mud_type, mud->mudlib, mud->ipaddress, mud->player_port );
	    break;
	case 0:
	    sprintf( buf, "%-26s(down)\n\r", mud->name );
	    break;
	default:
	    sprintf( buf, "%-26s(rebooting, back in %d seconds)\n\r", mud->name, mud->status );
	    break;
	}
      escape_smaug_color( outbuf, buf );
	i3send_to_pager( outbuf, ch );
    }
    i3pager_printf( ch, "%d total muds listed.\n\r", mudcount );
    return;
}

void I3_chanlist( CHAR_DATA *ch, char *argument ) 
{
    I3_CHANNEL *channel;
    I3_LISTENER *listener;
    char filter[MSL];
    char buf[MSL];
    bool all = FALSE, found = FALSE;

    if( IS_NPC(ch) )
      return;

    argument = i3one_argument( argument, filter );

    if( !str_cmp( filter, "all" ) )
	all = TRUE;

    i3send_to_pager( "Local name          Lvl I3 Name             Hosted at           Status\n\r", ch );
    i3send_to_pager( "--------------------------------------------------------------------------\n\r", ch );
    for( channel = first_I3chan; channel; channel = channel->next ) 
    {
	if( !all && channel->local_name[0] == 0 )
	    continue;

	if( ( listener = find_I3_listener_by_char( channel, ch ) ) )
	    found = TRUE;

	sprintf( buf, "&C%c &W%-18s&Y%-4d&B%-20s&P%-20s%-8s\n\r",
	    listener ? '*' : ' ',
	    channel->local_name ? channel->local_name : "Not configured",
          channel->local_level, channel->I3_name, channel->host_mud,
	    channel->status == 0 ? "&GPublic" : "&RPrivate" );

      i3send_to_pager( buf, ch );
    }

    if( found )
    {
	strcpy( buf, "&C*: You are subscribed to this channel.\n\r" );
      i3send_to_pager( buf, ch );
    }

    return;
}

void IMUD3_stats( CHAR_DATA *ch, char *argument ) 
{
    if( IS_NPC(ch) )
	return;

    if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR )
    {
	i3_to_char( "That option is restricted to mud administrators only.\n\r", ch );
	return;
    }

    i3_printf( ch, "messages	: %d (%d unknown)\n", I3_stats.count_total, I3_stats.count_unknown );

    i3_printf( ch, "private	: %d/%d tells/%d/%d emotes\n", I3_stats.count_tell_commands, I3_stats.count_tell,
	I3_stats.count_emoteto_commands, I3_stats.count_emoteto );

    i3_printf( ch, "who		: %d commands/%d req/%d reply\n", I3_stats.count_who_commands, 
	I3_stats.count_who_req, I3_stats.count_who_reply );

    i3_printf( ch, "finger	: %d commands/%d req/%d reply\n", I3_stats.count_finger_commands,
	I3_stats.count_finger_req, I3_stats.count_finger_reply );

    i3_printf( ch, "locate	: %d commands/%d req/%d reply\n", I3_stats.count_locate_commands,
	I3_stats.count_locate_req, I3_stats.count_locate_reply );

    i3_printf( ch, "channels	: %d m/%d e/%d t sent\n", I3_stats.count_channel_m_commands,
	I3_stats.count_channel_e_commands, I3_stats.count_channel_t_commands );

    i3_printf( ch, "		: %d m/%d e/%d t\n", I3_stats.count_channel_m, I3_stats.count_channel_e,
	I3_stats.count_channel_t ); 

    i3_printf( ch, "		: %d list-reply/%d add/%d remove\n", I3_stats.count_chanlist_reply,
	I3_stats.count_channel_add, I3_stats.count_channel_remove );

    i3_printf( ch, "		: filter %d req/%d reply\n", I3_stats.count_channel_filter_req, 
	I3_stats.count_channel_filter_reply );

    i3_printf( ch, "		: who %d commands/%d req/%d reply\n", I3_stats.count_channel_who_commands,
	I3_stats.count_channel_who_req, I3_stats.count_channel_who_reply );

    i3_printf( ch, "		: user %d req/%d reply\n", I3_stats.count_chan_user_req, I3_stats.count_chan_user_reply );

    i3_printf( ch, "news	: %d read/%d post/%d grplist_req\n", I3_stats.count_news_read_req,
	I3_stats.count_news_post_req, I3_stats.count_news_grplist_req );

    i3_printf( ch, "mail 	: %d/%d ack\n", I3_stats.count_mail, I3_stats.count_mail_ack );

    i3_printf( ch, "filelist	: %d req/%d reply\n", I3_stats.count_file_list_req, I3_stats.count_file_list_reply );

    i3_printf( ch, "file	: %d put/%d getreq/%d getreply\n", I3_stats.count_file_put, I3_stats.count_file_get_req,
	I3_stats.count_file_get_reply ); 

    i3_printf( ch, "auth	: %d req/%d reply\n", I3_stats.count_auth_mud_req, I3_stats.count_auth_mud_reply );

    i3_printf( ch, "startup	: %d req/%d reply\n", I3_stats.count_startup_req_3, I3_stats.count_startup_reply );

    i3_printf( ch, "oob		: %d req/%d begin/%d end\n", I3_stats.count_oob_req, I3_stats.count_oob_begin,
	I3_stats.count_oob_end );

    i3_printf( ch, "errors	: %d\n", I3_stats.count_error );

    i3_printf( ch, "mudlist	: %d\n", I3_stats.count_mudlist );

    i3_printf( ch, "shutdown	: %d\n", I3_stats.count_shutdown );

    i3_printf( ch, "ucache	: %d\n", I3_stats.count_ucache_update );
}

void I3_setup_channel( CHAR_DATA *ch, char *argument ) 
{
    char localname[MIL];
    char I3_name[MIL];
    char level[MIL];
    I3_CHANNEL *channel, *channel2;
    I3_LISTENER *ikill;
    int ilevel = 0;

    if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR ) 
    {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
    }

    argument = i3one_argument( argument, I3_name );
    argument = i3one_argument( argument, localname );
    argument = i3one_argument( argument, level );

    ilevel = atoi( level );

    if( ( channel = find_I3_channel_by_name( I3_name ) ) == NULL )
    {
	i3_to_char("&YUnknown channel\n\r"
	    "(use &Wi3 chanlist&Y to get an overview of the channels available)\n\r", ch );
	return;
    }

    if( localname[0] == 0 )
    {
	if( channel->local_name[0] == 0 ) 
	{
	    i3_printf( ch, "Channel %s@%s isn't configured.\n\r", channel->I3_name, channel->host_mud );
	    return;
	}
	while( ( ikill = channel->first_listener ) != NULL ) 
	{
	    i3_printf( ikill->desc->character, "Channel %s (%s@%s) has been removed by %s.\n\r",
		channel->local_name, channel->I3_name, channel->host_mud, CH_NAME(ch) );
	    destroy_I3_listener( channel, ikill );
	}
	sprintf( log_buf, "I3 - setup_channel: removing %s as %s@%s", channel->local_name, channel->I3_name, channel->host_mud );
      i3log( log_buf );
	I3_check_channel( channel, FALSE );
	strcpy( channel->local_name, "" );
	channel->local_name[0] = '\0';
    }
    else 
    {
	if( channel->local_name[0] ) 
      {
	    i3_printf( ch, "Channel %s@%s is already known as %s.\n\r",
		channel->I3_name, channel->host_mud, channel->local_name );
	    return;
	}
	if( ( channel2 = find_I3_channel_by_localname( localname ) ) ) 
 	{
	    i3_printf( ch, "Channel %s@%s is already known as %s.\n\r",
		channel2->I3_name, channel2->host_mud, channel2->local_name );
	    return;
	}
	strcpy( channel->local_name, localname );
	channel->local_level = ilevel;
	i3_printf( ch, "%s@%s is now locally known as %s\n\r",
	    channel->I3_name, channel->host_mud, channel->local_name );
	sprintf( log_buf, "I3 - setup_channel: setting up %s@%s as %s",
	    channel->I3_name, channel->host_mud, channel->local_name );
      i3log( log_buf );
    }
}

void I3_chan_who( CHAR_DATA *ch, char *argument ) 
{
    char channel_name[MIL];
    char mud_name[MIL];
    I3_CHANNEL *channel;
    I3_MUD *mud;

    argument = i3one_argument( argument, channel_name );
    argument = i3one_argument( argument, mud_name );

    if( ( channel = find_I3_channel_by_localname( channel_name ) ) == NULL ) 
    {
	i3_to_char( "&YUnknown channel.\n\r"
	    "(use &Wi3 chanlist&Y to get an overview of the channels available)\n\r", ch );
	return;
    }
    if( ( mud = find_I3_mud_by_name( mud_name ) ) == NULL ) 
    {
	i3_to_char( "&YUnknown mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
    }

    if( mud->status >= 0 ) 
    {
	i3_printf( ch, "%s is marked as down.\n\r", mud->name );
	return;
    }

    I3_send_chan_who( ch, channel, mud );
}

void I3_listen_channel( DESCRIPTOR_DATA *d, char *argument, bool silent, bool addflag ) 
{
   char channel_name[MIL];
   I3_CHANNEL *channel;
   I3_LISTENER *listener;

   if( !d )
   {
	i3bug( "I3_listen_channel: NULL descriptor!" );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3 listen <local channel name>\n\r", d->character );
	return;
   }

   if( !I3_is_connected() )
   {
	i3_to_char( "The mud is not connected to I3.\n\r", d->character );
	return;
   }

   argument = i3one_argument( argument, channel_name );

   if( ( channel = find_I3_channel_by_localname( channel_name ) ) == NULL ) 
   {
	if ( !silent )
	    i3_to_char( "&YUnknown channel.\n\r"
		"(use &Wi3 chanlist&Y to get an overview of the channels available)\n\r", d->character );
	return;
   }

   if( ( listener = find_I3_listener_by_descriptor( channel, d ) ) != NULL ) 
   {
	if( channel->first_listener != NULL )
	   destroy_I3_listener( channel, listener );
	else
	{
	   sprintf( log_buf, "I3: I3_listen_channel: NULL channel->first_listener for %s!", channel->local_name );
	   i3bug( log_buf );
	}
	if( !silent )
	    i3_printf( d->character, "You are now removed from %s (%s@%s)\n",
		channel->local_name, channel->I3_name, channel->host_mud );

	if( addflag )
	   I3_unflagchan( &I3LISTEN(d->character), channel->local_name );

	I3_check_channel( channel, FALSE );
   }
   else
   {
      if( CH_LEVEL(d->character) < channel->local_level )
      {
	   i3_printf( d->character, "That channel is reserved for level %d and higher.\n\r", channel->local_level );
	   return;
      }

	listener = new_I3_listener( channel, d );
	if( !listener )
      {
	   i3bug( "I3_listen_channel: Received NULL listener - NULL channel!" );
	   if( !silent )
	      i3_to_char( "An error occured trying to add you to the channel.\n\r", d->character );
	   return;
	}
	if( !silent )
	    i3_printf( d->character, "You are now subscribed to %s (%s@%s)\n", 
		channel->local_name, channel->I3_name, channel->host_mud );

	if( addflag )
	   I3_flagchan( &I3LISTEN(d->character), channel->local_name );

	I3_check_channel( channel, FALSE );
   }
   return;
}

void free_i3chardata( CHAR_DATA *ch )
{
   I3_IGNORE *temp, *next;

   if( I3LISTEN(ch) )
      I3DISPOSE( I3LISTEN(ch) );

   for( temp = FIRST_I3IGNORE(ch); temp; temp = next )
   {
      next = temp->next;
    	UNLINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
    	DISPOSE( temp );
   }
   return;
}

void I3_close_char( DESCRIPTOR_DATA *dclose )
{
   I3_CHANNEL *channel;
   I3_LISTENER *listener;

   for( channel = first_I3chan; channel; channel = channel->next ) 
   {
      if( ( listener = find_I3_listener_by_descriptor( channel, dclose ) ) )
	   I3_listen_channel( dclose, channel->local_name, TRUE, FALSE );
   }
   return;
}

void I3_mudinfo( CHAR_DATA *ch, char *argument ) 
{
   I3_MUD *mud;
   char mudname[MSL];

   argument = i3one_argument( argument, mudname );

   if( mudname[0] == '\0' ) 
   {
	i3_to_char( "&YWhich mud do you want information about?\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( ( mud = find_I3_mud_by_name( mudname ) ) == NULL ) 
   {
	i3_to_char( "&YUnknown mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   i3_printf( ch, "Information about %s\n\r\n\r", mud->name );
   if( mud->status == 0 )
	i3_to_char( "Status     : Currently down\n\r", ch );
   else if ( mud->status > 0 )
	i3_printf(ch, "Status     : Currently rebooting, back in %d seconds\n\r", mud->status );
   i3_printf( ch, "MUD port   : %s %d\n\r", mud->ipaddress, mud->player_port );
   i3_printf( ch, "Base mudlib: %s\n\r", mud->base_mudlib );
   i3_printf( ch, "Mudlib     : %s\n\r", mud->mudlib );
   i3_printf( ch, "Driver     : %s\n\r", mud->driver );
   i3_printf( ch, "Type       : %s\n\r", mud->mud_type );
   i3_printf( ch, "Open status: %s\n\r", mud->open_status );
   i3_printf( ch, "Admin      : %s\n\r", mud->admin_email );
   if( mud->web[0] )
      i3_printf( ch, "URL        : %s\n\r", mud->web );

   i3_to_char( "Supports   : ", ch );
   if (mud->tell)	i3_to_char( "tell, ", ch );
   if (mud->beep)	i3_to_char( "beep, ", ch );
   if (mud->emoteto)	i3_to_char( "emoteto, ", ch );
   if (mud->who)		i3_to_char( "who, ", ch );
   if (mud->finger)	i3_to_char( "finger, ", ch );
   if (mud->locate)	i3_to_char( "locate, ", ch );
   if (mud->channel)	i3_to_char( "channel, ", ch );
   if (mud->news)	i3_to_char( "news, ", ch );
   if (mud->mail)	i3_to_char( "mail, ", ch );
   if (mud->file)	i3_to_char( "file, ", ch );
   if (mud->auth)	i3_to_char( "auth, ", ch );
   if (mud->ucache)	i3_to_char( "ucache, ", ch );
   i3_to_char( "\n\r", ch );

   i3_to_char( "Supports   : ", ch );
   if (mud->smtp)	i3_printf( ch, "smtp (port %d), ", mud->smtp );
   if (mud->http)	i3_printf( ch, "http (port %d), ", mud->http );
   if (mud->ftp)		i3_printf( ch, "ftp  (port %d), ", mud->ftp );
   if (mud->pop3)	i3_printf( ch, "pop3 (port %d), ", mud->pop3 );
   if (mud->nntp)	i3_printf( ch, "nntp (port %d), ", mud->nntp );
   if (mud->rcp)		i3_printf( ch, "rcp  (port %d), ", mud->rcp );
   if (mud->amrcp)	i3_printf( ch, "amrcp (port %d), ",mud->amrcp );
   i3_to_char( "\n\r", ch );
}

void I3_chanlayout( CHAR_DATA *ch, char *argument ) 
{
    I3_CHANNEL *channel = NULL;
    char arg1[MIL];
    char arg2[MIL];

    if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR )
    {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
    }

    if( !argument || argument[0] == '\0' )
    {
	i3_to_char( "Usage: i3 chanlayout <localchannel> <layout> <format...>\n\r", ch );
	i3_to_char( "Layout can be one of these: layout_e layout_m\n\r", ch );
	i3_to_char( "Format can be any way you want it to look, provided you have the proper number of %s tags in it.\n\r", ch );
	return;
    }

    argument = i3one_argument( argument, arg1 );
    argument = i3one_argument( argument, arg2 );

    if( !arg1 || arg1[0] == '\0' )
    {
	I3_chanlayout( ch, "chanlayout" );
	return;
    }
    if( !arg2 || arg2[0] == '\0' )
    {
	I3_chanlayout( ch, "chanlayout" );
	return;
    }
    if( !argument || argument[0] == '\0' )
    {
	I3_chanlayout( ch, "chanlayout" );
	return;
    }

    if( ( channel = find_I3_channel_by_localname( arg1 ) ) == NULL ) 
    {
	    i3_to_char( "&YUnknown channel.\n\r"
		"(use &Wi3 chanlist&Y to get an overview of the channels available)\n\r", ch );
	return;
    }
    
    if( !str_cmp( arg2, "layout_e" ) )
    {
	if( !verify_i3layout( argument, 2 ) )
	{
	   i3_to_char( "Incorrect format for layout_e. You need exactly 2 %s's.\n\r", ch );
	   return;
	}
	strcpy( channel->layout_e, argument );
	i3_to_char( "Channel layout_e changed.\n\r", ch );
	return;
    }

    if( !str_cmp( arg2, "layout_m" ) )
    {
	if( !verify_i3layout( argument, 4 ) )
	{
	   i3_to_char( "Incorrect format for layout_m. You need exactly 4 %s's.\n\r", ch );
	   return;
	}
	strcpy( channel->layout_m, argument );
	i3_to_char( "Channel layout_m changed.\n\r", ch );
	return;
    }

    I3_chanlayout( ch, "chanlayout" );
    return;
}

void I3_connect( CHAR_DATA *ch, char *argument ) 
{
    if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR ) 
    {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
    }

    if( I3_is_connected() ) 
    {
	i3_to_char( "The MUD is already connected to the Intermud-3 router.\n\r", ch );
	return;
    }

    i3_to_char( "Connecting to Intermud-3 router\n\r", ch );
    I3_main( TRUE, this_mud->player_port, FALSE );
}

void free_i3data( void )
{
   I3_MUD *mud, *next_mud;
   I3_CHANNEL *channel, *next_chan;
   UCACHE_DATA *ucache, *next_ucache;

   if( first_I3chan )
   {
	for( channel = first_I3chan; channel; channel = next_chan )
	{
	   next_chan = channel->next;
	   destroy_I3_channel( channel );
	}
   }

   if( first_mud )
   {
	for( mud = first_mud; mud; mud = next_mud )
	{
	   next_mud = mud->next;
	   destroy_I3_mud( mud );
	}
   }

   if( first_ucache )
   {
	for( ucache = first_ucache; ucache; ucache = next_ucache )
	{
	   next_ucache = ucache->next;
	   UNLINK( ucache, first_ucache, last_ucache, next, prev );
	   DISPOSE( ucache );
	}
   }
   return;
}

void I3_disconnect( CHAR_DATA *ch, char *argument ) 
{
   CHAR_DATA *vch;

   if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR ) 
   {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
   }

   if( !I3_is_connected() ) 
   {
	i3_to_char( "The MUD isn't connected to the Intermud-3 router.\n\r", ch );
	return;
   }

   for( vch = first_char; vch; vch = vch->next )
   {
	if( !IS_NPC( vch ) && vch->desc )
	   I3_close_char( vch->desc );
   }

   i3_to_char( "Disconnecting from Intermud-3 router.\n\r", ch );
   I3_shutdown( 0 );
   return;
}

void I3_savechannels( CHAR_DATA *ch, char *argument )
{
   if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR )
   {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
   }

   i3_to_char( "Saving current channels...\n\r", ch );
   I3_write_channel_config();
   return;
}

#ifndef I3ADMIN
void I3_addchan( CHAR_DATA *ch, char *argument )
{
   i3_to_char( "This function is disabled. Have your coders refer to the Intermud-3 homepage to set this up.\n\r", ch );
   return;
}

void I3_removechan( CHAR_DATA *ch, char *argument )
{
   i3_to_char( "This function is disabled. Have your coders refer to the Intermud-3 homepage to set this up.\n\r", ch );
   return;
}

void I3_admin_channel( CHAR_DATA *ch, char *argument )
{
   i3_to_char( "This function is disabled. Have your coders refer to the Intermud-3 homepage to set this up.\n\r", ch );
   return;
}
#endif

void I3_ignore( CHAR_DATA *ch, char *argument )
{
   I3_IGNORE *temp;
   I3_MUD *pmud;
   char *ps;
   char mud[MSL];

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "You are currently ignoring the following people:\n\r\n\r", ch );

	if( !FIRST_I3IGNORE(ch) )
	{
	   i3_to_char( "Nobody\n\r", ch );
	   return;
	}
      for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
	   i3_printf( ch, "\t  - %s\n\r", temp->name );
      return;
   }

   sprintf( cbuf, "%s@%s", CH_NAME(ch), I3_THISMUD );
   if( !str_cmp( cbuf, argument ) )
   {
	i3_to_char( "You don't really want to do that....\n\r", ch );
	return;
   }

   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
   {
	if( !str_cmp( temp->name, argument ) )
	{
	   UNLINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
	   i3_printf( ch, "You are no longer ignoring %s.\n\r", temp->name );
	   DISPOSE( temp );
	   return;
	}
   }

   strcpy( cbuf, argument );

   ps = strchr( argument, '@' );

   if( argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   ps++;
   strcpy( mud, ps );

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   CREATE( temp, I3_IGNORE, 1 );
   LINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
   strcpy( temp->name, cbuf );
   i3_printf( ch, "You now ignore %s.\n\r", temp->name );
}

void I3_invis( CHAR_DATA *ch )
{
   I3INVIS(ch) = !I3INVIS(ch);
#ifdef I3CIRCLE
   save_char( ch, NOWHERE );
#else
   save_char_obj( ch );
#endif

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are now invisible to I3.\n\r", ch );
	return;
   }

   i3_to_char( "You are no longer invisible to I3.\n\r", ch );
   return;
}

void I3_debug( CHAR_DATA *ch )
{
   if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR )
   {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
   }

   packetdebug = !packetdebug;

   if( packetdebug )
      i3_to_char( "Packet debugging enabled.\n\r", ch );
   else
	i3_to_char( "Packet debugging disabled.\n\r", ch );

   return;
}

void I3_send_user_req( CHAR_DATA *ch, char *argument )
{
   char user[MSL], mud[MSL];
   char *ps;
   I3_MUD *pmud;

   if( IS_NPC(ch) ) 
	return;

   if( argument[0] == '\0' ) 
   {
	i3_to_char( "&YQuery who at which mud?\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }
   if( ( ps = strchr( argument, '@' ) ) == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   strcpy( user, argument );
   strcpy( mud, ps+1 );

   if( user[0] == 0 || mud[0] == 0 ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   I3_send_chan_user_req( pmud->name, user );
   return;
}

void I3_saveconfig( void )
{
   FILE *fp;

   if( ( fp = fopen( I3_CONFIG_FILE, "w" ) ) == NULL ) 
   {
	i3log( "Couldn't write to I3 config file." );
	return;
   }

   fprintf( fp, "$I3CONFIG\n" );
   fprintf( fp, "#\n" );
   fprintf( fp, "# When changing this information, be sure you don't remove the tildes!\n" );
   fprintf( fp, "# Set autoconnect to 1 to automatically connect at bootup.\n" );
   fprintf( fp, "# This information can be edited online using 'i3 config'\n" );
   fprintf( fp, "thismud      %s~\n", this_mud->name );
   fprintf( fp, "autoconnect  %d\n",  this_mud->autoconnect );
   fprintf( fp, "telnet       %s~\n", this_mud->telnet );
   fprintf( fp, "web          %s~\n", this_mud->web );
   fprintf( fp, "adminemail   %s~\n", this_mud->admin_email );
   fprintf( fp, "openstatus   %s~\n", this_mud->open_status );
   fprintf( fp, "mudtype      %s~\n", this_mud->mud_type );
   fprintf( fp, "basemudlib   %s~\n", this_mud->base_mudlib );
   fprintf( fp, "mudlib       %s~\n", this_mud->mudlib );
   fprintf( fp, "driver       %s~\n", this_mud->driver );
   fprintf( fp, "minlevel     %d\n",  this_mud->minlevel );
   fprintf( fp, "#\n" );
   fprintf( fp, "# The router information. Currently only one.\n" );
   fprintf( fp, "# Information below this point cannot be edited online.\n" );
   fprintf( fp, "router       %s~\n", this_mud->routerIP );
   fprintf( fp, "port         %d\n", this_mud->routerPort );
   fprintf( fp, "routername   %s~\n", this_mud->routerName );
   fprintf( fp, "#\n" );
   fprintf( fp, "# The services provided by your mud.\n" );
   fprintf( fp, "# Do not turn things on unless you KNOW your mud properly supports them!\n" );
   fprintf( fp, "# Refer to http://www.intermud.org for public packet specifications.\n" );
   fprintf( fp, "tell         %d\n", this_mud->tell );
   fprintf( fp, "beep         %d\n", this_mud->beep );
   fprintf( fp, "emoteto      %d\n", this_mud->emoteto );
   fprintf( fp, "who          %d\n", this_mud->who );
   fprintf( fp, "finger       %d\n", this_mud->finger );
   fprintf( fp, "locate       %d\n", this_mud->locate );
   fprintf( fp, "channel      %d\n", this_mud->channel );
   fprintf( fp, "news         %d\n", this_mud->news );
   fprintf( fp, "mail         %d\n", this_mud->mail );
   fprintf( fp, "file         %d\n", this_mud->file );
   fprintf( fp, "auth         %d\n", this_mud->auth );
   fprintf( fp, "ucache       %d\n", this_mud->ucache );
   fprintf( fp, "#\n" );
   fprintf( fp, "# Port numbers for OOB services. Leave as 0 if your mud does not support these.\n" );
   fprintf( fp, "smtp         %d\n", this_mud->smtp );
   fprintf( fp, "ftp          %d\n", this_mud->ftp );
   fprintf( fp, "nntp         %d\n", this_mud->nntp );
   fprintf( fp, "http         %d\n", this_mud->http );
   fprintf( fp, "pop3         %d\n", this_mud->pop3 );
   fprintf( fp, "rcp          %d\n", this_mud->rcp );
   fprintf( fp, "amrcp        %d\n", this_mud->amrcp );
   fprintf( fp, "end\n" );
   fprintf( fp, "$END\n" );
   FCLOSE( fp );
   return;
}

void I3_setconfig( CHAR_DATA *ch, char *argument )
{
   char arg[MIL];

   if( CH_LEVEL(ch) < LEVEL_IMPLEMENTOR ) 
   {
	i3_to_char( "&RThat option is restricted to mud administrators only.\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, arg );

   if( !arg || arg[0] == '\0' )
   {
	i3_to_char( "&GConfiguration info for your mud. Changes save when edited.\n\r", ch );
	i3_to_char( "&GYou can set the following:\n\r\n\r", ch );
	i3_to_char( "&wShow       : &GDisplays your current congfiguration.\n\r", ch );
	i3_to_char( "&wAutoconnect: &GA toggle. Either on or off. Your mud will connect automatically with it on.\n\r", ch );
	i3_to_char( "&wMudname    : &GThe name you want displayed on I3 for your mud.\n\r", ch );
	i3_to_char( "&wTelnet     : &GThe telnet address for your mud. Do not include the port number.\n\r", ch );
	i3_to_char( "&wWeb        : &GThe website address for your mud. In the form of: www.address.com\n\r", ch );
	i3_to_char( "&wAdmin      : &GThe email address of your mud's administrator. Needs to be valid!!\n\r", ch );
	i3_to_char( "&wStatus     : &GThe open status of your mud. IE: Public, Development, etc.\n\r", ch );
	i3_to_char( "&wMudtype    : &GWhat type of mud you have. Diku, Rom, Smaug, Circle, etc.\n\r", ch );
	i3_to_char( "&wBaselib    : &GThe base version of the codebase you have.\n\r", ch );
	i3_to_char( "&wMudlib     : &GWhatever you call your mud's current codebase.\n\r", ch );
	i3_to_char( "&wMinlevel   : &GMinimum level at which I3 will recognize your players.\n\r", ch );
	return;
   }

   if( !strcmp( arg, "show" ) )
   {
	i3_printf( ch, "&wMudname       : &G%s\n\r", this_mud->name );
	i3_printf( ch, "&wAutoconnect   : &G%s\n\r", this_mud->autoconnect == TRUE ? "Enabled" : "Disabled" );
	i3_printf( ch, "&wTelnet address: &G%s:%d\n\r", this_mud->telnet, this_mud->player_port );
	i3_printf( ch, "&wWebsite       : &Ghttp://%s\n\r", this_mud->web );
	i3_printf( ch, "&wAdmin Email   : &G%s\n\r", this_mud->admin_email );
	i3_printf( ch, "&wStatus        : &G%s\n\r", this_mud->open_status );
	i3_printf( ch, "&wMudtype       : &G%s\n\r", this_mud->mud_type );
	i3_printf( ch, "&wBase Mudlib   : &G%s\n\r", this_mud->base_mudlib );
	i3_printf( ch, "&wMudlib        : &G%s\n\r", this_mud->mudlib );
	i3_printf( ch, "&wMinlevel      : &G%d\n\r", this_mud->minlevel );
	return;
   }

   if( !str_cmp( arg, "autoconnect" ) )
   {
	this_mud->autoconnect = !this_mud->autoconnect;

	if( this_mud->autoconnect )
	   i3_to_char( "Autoconnect enabled.\n\r", ch );
	else
	   i3_to_char( "Autoconnect disabled.\n\r", ch );
	I3_saveconfig( );
	return;
   }

   if( I3_is_connected() )
   {
	i3_to_char( "Configuration may not be changed while the mud is connected.\n\r", ch );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	I3_setconfig( ch, "" );
	return;
   }

   if( strlen( argument ) > 255 )
   {
	i3_to_char( "Configuration value exceeds 255 characters.\n\r", ch );
	return;
   }

   if( !str_cmp( arg, "minlevel" ) )
   {
	int value = atoi( argument );

	this_mud->minlevel = value;
	I3_saveconfig( );
	i3_printf( ch, "Minimum level changed to %d\n\r", value );
	return;
   }

   if( !strcmp( arg, "mudname" ) )
   {
	strcpy( this_mud->name, argument );
	strcpy( I3_THISMUD, argument );
	unlink( I3_PASSWORD_FILE );
	I3_saveconfig( );
	i3_printf( ch, "Mud name changed to %s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "telnet" ) )
   {
	strcpy( this_mud->telnet, argument );
	I3_saveconfig( );
	i3_printf( ch, "Telnet address changed to %s:%d\n\r", argument, this_mud->player_port );
	return;
   }

   if( !strcmp( arg, "web" ) )
   {
	strcpy( this_mud->web, argument );
	I3_saveconfig( );
	i3_printf( ch, "Website changed to http://%s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "admin" ) )
   {
	strcpy( this_mud->admin_email, argument );
	I3_saveconfig( );
	i3_printf( ch, "Admin email changed to %s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "status" ) )
   {
	strcpy( this_mud->open_status, argument );
	I3_saveconfig( );
	i3_printf( ch, "Status changed to %s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "mudtype" ) )
   {
	strcpy( this_mud->mud_type, argument );
	I3_saveconfig( );
	i3_printf( ch, "Mud type changed to %s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "baselib" ) )
   {
	strcpy( this_mud->base_mudlib, argument );
	I3_saveconfig( );
	i3_printf( ch, "Base mudlib changed to %s\n\r", argument );
	return;
   }

   if( !strcmp( arg, "mudlib" ) )
   {
	strcpy( this_mud->mudlib, argument );
	I3_saveconfig( );
	i3_printf( ch, "Mudlib changed to %s\n\r", argument );
	return;
   }

   I3_setconfig( ch, "" );
   return;
}

I3DOFUN( do_i3 )
{
   char arg[MIL];

   if( IS_NPC(ch) )
	return;

   if( !ch->desc )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, arg );
   switch( which_keyword( arg, "stats", "setup", "chanlist", "mud", "mudlist", "listen", "chanwho",
	"mudinfo", "chanlayout", "connect", "disconnect", "savechan", "addchan", "removechan", "chanadmin",
	"ignore", "invis", "debug", "user", "config", "ucache", NULL ) ) 
   {
	case  1: IMUD3_stats( ch, argument ); break;
	case  2: I3_setup_channel( ch, argument ); break;
	case  3: I3_chanlist( ch, argument ); break;
	case  4: 
	case  5: I3_mudlist( ch, argument ); break;
	case  6: I3_listen_channel( ch->desc, argument, FALSE, TRUE ); break;
	case  7: I3_chan_who( ch, argument ); break;
	case  8: I3_mudinfo( ch, argument ); break;
	case  9: I3_chanlayout( ch, argument ); break;
	case 10: I3_connect( ch, argument ); break;
	case 11: I3_disconnect( ch, argument ); break;
	case 12: I3_savechannels( ch, argument ); break;
      case 13: I3_addchan( ch, argument ); break;
      case 14: I3_removechan( ch, argument ); break;
	case 15: I3_admin_channel( ch, argument ); break;
      case 16: I3_ignore( ch, argument ); break;
      case 17: I3_invis( ch ); break;
      case 18: I3_debug( ch ); break;
      case 19: I3_send_user_req( ch, argument ); break;
	case 20: I3_setconfig( ch, argument ); break;
	case 21: I3_show_ucache_contents( ch ); break;
	default:
	   i3_to_char( "Usage:\n\r", ch );
	   i3_to_char( "i3 chanlist [all] [filter]\n\r", ch );
	   i3_to_char( "i3 mudlist [filter]\n\r", ch );
	   i3_to_char( "i3 listen <localchannel>\n\r", ch );
	   i3_to_char( "i3 chanwho <channel> <mud>\n\r", ch );
	   i3_to_char( "i3 mudinfo <mud>\n\r", ch );
	   i3_to_char( "i3 ignore <person@mud>\n\r", ch );
	   i3_to_char( "i3 invis\n\r", ch );
         if( CH_LEVEL(ch) >= LEVEL_IMPLEMENTOR )
         {
	      i3_to_char( "i3 stats\n\r", ch );
	      i3_to_char( "i3 chanlayout <channel> <message> <emote>\n\r", ch );
	      i3_to_char( "i3 setup <channel> <localname> [level]\n\r", ch );
	      i3_to_char( "i3 addchan <channel> <type>\n\r", ch );
	      i3_to_char( "i3 removechan <channel>\n\r", ch );
		i3_to_char( "i3 chanadmin <localchannel> <add|remove> <mudname>\n\r", ch );
	      i3_to_char( "i3 connect\n\r", ch );
	      i3_to_char( "i3 disconnect\n\r", ch );
	      i3_to_char( "i3 savechan\n\r", ch );
		i3_to_char( "i3 debug\n\r", ch );
		i3_to_char( "i3 config <setting>\n\r", ch );
		i3_to_char( "i3 ucache\n\r", ch );
	   }
   }
   return;
}

I3DOFUN( do_i3who )
{
   I3_MUD *mud;
   char mudname[MSL];

   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, mudname );
   if( mudname[0] == '\0' ) 
   {
	i3_to_char( "&YGet an overview of which mud?\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( ( mud = find_I3_mud_by_name( mudname ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( mud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", mud->name );
	return;
   }

   if( mud->who == 0 )
	i3_printf( ch, "%s does not support the 'who' command. Sending anyway.\n\r", mud->name );

   I3_send_who( ch, mud->name );
}

I3DOFUN( do_i3locate )
{
   char person[MSL];

   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, person );
   if( person[0] == '\0' )
   {
	i3_to_char( "Locate who?\n\r", ch );
	return;
   }
   I3_send_locate( ch, person );
}

I3DOFUN( do_i3finger )
{
   char targetname[MSL];
   char user[MSL], mud[MSL];
   char *ps;
   I3_MUD *pmud;

   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, targetname );

   if( targetname[0] == '\0' ) 
   {
	i3_to_char( "&YFinger who at which mud?\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }
   if( ( ps = strchr( targetname, '@' ) ) == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   strcpy( user, targetname );
   strcpy( mud, ps+1 );

   if( user[0] == 0 || mud[0] == 0 ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->finger == 0 )
	i3_printf( ch, "%s does not support the 'finger' command. Sending anyway.\n\r", pmud->name );

   I3_send_finger( ch, user, pmud->name );
}

I3DOFUN( do_i3emote )
{
   char to[MIL], *ps;
   char mud[MIL];
   I3_MUD *pmud;

   if( IS_NPC(ch) )
	return;

   if( CH_LEVEL(ch) < this_mud->minlevel )
   {
	i3_to_char( "Huh?\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, to );
   ps = strchr( to, '@' );

   if( to[0] == '\0' || argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "&YYou should specify a person and a mud.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = 0;
   ps++;
   strcpy( mud, ps );

   if( ( pmud = find_I3_mud_by_name( mud ) ) == NULL ) 
   {
	i3_to_char( "&YNo such mud known.\n\r"
	    "(use &Wi3 mudlist&Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   if( pmud->status >= 0 )  
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->emoteto == 0 )
	i3_printf( ch, "%s does not support the 'emoteto' command. Sending anyway.\n\r", pmud->name );

   I3_send_emoteto( ch, to, pmud, argument );
}

void I3_char_login( CHAR_DATA *ch )
{
   int gender, sex;

   if( !I3_is_connected() )
   {
	if( CH_LEVEL(ch) >= LEVEL_IMPLEMENTOR && i3wait == -2 )
	   i3_to_char( "&RThe Intermud-3 connection is down. Attempts to reconnect were abandoned due to excessive failures.\n\r", ch );
	return;
   }

   if( CH_LEVEL(ch) < this_mud->minlevel )
	return;

   if( I3LISTEN(ch) != NULL )
   {
	I3_CHANNEL *channel = NULL;
	I3_LISTENER *listener;
	char *channels = I3LISTEN(ch);
	char arg[MIL];

	while( channels[0] != '\0' )
      {
	   channels = i3one_argument( channels, arg );

	   if( ( channel = find_I3_channel_by_localname( arg ) ) != NULL )
	   {
		if( ( listener = find_I3_listener_by_descriptor( channel, ch->desc ) ) == NULL )
		{
		   listener = new_I3_listener( channel, ch->desc );
		   if( !listener )
		   {
			sprintf( log_buf, "i3_char_login: Error occurred adding %s to I3 channel %s", CH_NAME(ch), channel->local_name );
		   	i3bug( log_buf );
		   }
		   else
			I3_check_channel( channel, FALSE );
		}
	   }
	   else
		I3_unflagchan( &I3LISTEN(ch), arg );
	}
   }

   if( this_mud->ucache == TRUE )
   {
      sprintf( log_buf, "%s@%s", CH_NAME(ch), I3_THISMUD );
      gender = I3_get_ucache_gender( log_buf );
      sex = dikutoi3gender( CH_SEX(ch) );

      if( gender == sex )
         return;

      I3_ucache_update( log_buf, sex );
      I3_send_ucache_update( CH_NAME(ch), sex );
   }
   return;
}

/*
 * Setup a TCP session to the router. Returns socket or <0 if failed.
 *
 */
int I3_connection_open( char *host, int rport ) 
{
   struct sockaddr_in sa;
   struct hostent *hostp;
   u_long ip_addr;
   int x = 1;

   sprintf( log_buf, "Attempting connect to %s on port %d", host, rport );
   i3log( log_buf );

   I3_socket = socket( AF_INET, SOCK_STREAM, 0 );
   if( I3_socket < 0 )
   {
	i3log( "Cannot create I3 socket!" );
      I3_connection_close( TRUE );
	return -1;
   }

   if( setsockopt( I3_socket, SOL_SOCKET, SO_KEEPALIVE, (void *) &x, sizeof(x) ) < 0 )
   {
	perror( "I3_connection_open: SO_KEEPALIVE" );
      I3_connection_close( TRUE );
	return -1;
   }

   if( ( x = fcntl( I3_socket, F_GETFL, 0 ) ) < 0 )
   {
      i3log( "I3_connection_open: fcntl(F_GETFL)" );
      I3_connection_close( TRUE );
      return -1;
   }

   if( fcntl( I3_socket, F_SETFL, x | O_NONBLOCK ) < 0 )
   {
      i3log( "I3_connection_open: fcntl(F_SETFL)" );
      I3_connection_close( TRUE );
      return -1;
   }

   memset( &sa, 0, sizeof( sa ) );
   sa.sin_family = AF_INET;

   if( ( ip_addr = inet_addr( host ) ) != INADDR_NONE ) 
	memcpy( &sa.sin_addr, &ip_addr, sizeof( ip_addr ) );
   else 
   {
	hostp = gethostbyname( host );
	if( !hostp )
      {
	   i3log( "I3_connection_open: Cannot resolve router hostname." );
         I3_connection_close( TRUE );
	   return -1;
	}
	memcpy( &sa.sin_addr, hostp->h_addr, hostp->h_length );
   }

   sa.sin_port = htons( rport );

   if( connect( I3_socket, (struct sockaddr *)&sa, sizeof(sa) ) < 0 )
   {
	if( errno != EINPROGRESS )
	{
	   i3log( "I3_connection_open: Unable to connect to router." );
         I3_connection_close( TRUE );
	   return -1;
	}
   }
   i3log( "Connected to Intermud-3 router." );
   return I3_socket;
}

/*
 * Read one I3 packet into the I3_input_buffer
 */
void I3_read_packet( void ) 
{
   long size;

   memcpy( &size, I3_input_buffer, 4 );
   size = ntohl( size );

   memcpy( I3_currentpacket, I3_input_buffer + 4, size );
   I3_currentpacket[size+1] = 0;

   memcpy( I3_input_buffer, I3_input_buffer + size + 4, I3_input_pointer - size - 4 );
   I3_input_pointer -= size + 4;
   return;
}

/*
 * Read the first field of an I3 packet and call the proper function to
 * process it. Afterwards the original I3 packet is completly messed up.
 */
void I3_parse_packet( void ) 
{
   char *ps, *next_ps;

   ps = I3_currentpacket;
   if( ps[0] != '(' || ps[1] != '{' ) 
   	return;
   ps += 2;
   I3_get_field( ps, &next_ps );

   I3_stats.count_total++;

   if( !str_cmp( ps, "\"tell\"" ) ) 
   {
	I3_stats.count_tell++;
	I3_process_tell( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"beep\"" ) )
   {
	I3_stats.count_beep++;
	I3_process_beep( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"emoteto\"" ) ) 
   {
	I3_stats.count_emoteto++;
	I3_process_emoteto( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"channel-m\"" ) ) 
   {
	I3_stats.count_channel_m++;
	I3_process_channel_m( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"channel-e\"" ) ) 
   {
	I3_stats.count_channel_e++;
	I3_process_channel_e( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"finger-req\"" ) ) 
   {
	I3_stats.count_finger_req++;
	I3_process_finger_req( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"finger-reply\"" ) ) 
   {
	I3_stats.count_finger_reply++;
	I3_process_finger_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"locate-req\"" ) ) 
   {
	I3_stats.count_locate_req++;
	I3_process_locate_req( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"locate-reply\"" ) ) 
   {
	I3_stats.count_locate_reply++;
	I3_process_locate_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"chan-who-req\"" ) ) 
   {
	I3_stats.count_channel_who_req++;
	I3_process_chan_who_req( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"chan-who-reply\"" ) ) 
   {
	I3_stats.count_channel_who_reply++;
	I3_process_chan_who_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"ucache-update\"" ) )
   {
	I3_stats.count_ucache_update++;
	I3_process_ucache_update( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"who-req\"" ) ) 
   {
	I3_stats.count_who_req++;
	I3_process_who_req( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"who-reply\"" ) ) 
   {
	I3_stats.count_who_reply++;
	I3_process_who_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"chanlist-reply\"" ) ) 
   {
	I3_stats.count_chanlist_reply++;
	I3_process_chanlist_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"startup-reply\"" ) ) 
   {
	I3_stats.count_startup_reply++;
	I3_process_startup_reply( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"mudlist\"" ) ) 
   {
	I3_stats.count_mudlist++;
	I3_process_mudlist( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"error\"" ) ) 
   {
	I3_stats.count_error++;
	I3_process_error( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"channel-t\"" ) )
   {
	I3_process_channel_t( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"chan-user-req\"" ) )
   {
	I3_process_chan_user_req( next_ps );
	return;
   }
   if( !str_cmp( ps, "\"chan-user-reply\"" ) )
   {
	I3_process_chan_user_reply( next_ps );
	return;
   }

   I3_stats.count_unknown++;
   sprintf( log_buf, "I3: Unknown packet: %s\n", I3_currentpacket );
   i3log( log_buf );
   sprintf( log_buf, "I3: Found unknown field: .%s.\n", ps );
   i3log( log_buf );
   return;
}

/* Used only during copyovers */
void fread_mudlist( FILE *fin, I3_MUD *mud )
{
   char *word;
   char *ln;
   bool fMatch;
   int x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12;

   for ( ; ; )
   {
	word   = feof( fin ) ? "End" : i3fread_word( fin );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fin );
	      break;

	   case 'B':
		SKEY( "Baselib",		mud->base_mudlib,		i3fread_string( fin ) );
		break;

	   case 'D':
		SKEY( "Driver",		mud->driver,		i3fread_string( fin ) );
		break;

	   case 'E':
		SKEY( "Email",		mud->admin_email,		i3fread_string( fin ) );
	      if ( !str_cmp( word, "End" ) )
		   return;

	   case 'I':
		SKEY( "IP",			mud->ipaddress,		i3fread_string( fin ) );
		break;

	   case 'M':
		SKEY( "Mudlib",		mud->mudlib,		i3fread_string( fin ) );
	      break;

	   case 'O':
		SKEY( "Openstatus",	mud->open_status,		i3fread_string( fin ) );
		if( !str_cmp( word, "OOBPorts" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=x4=x5=x6=x7=0;

	         sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		   mud->smtp  = x1;
		   mud->ftp   = x2;
		   mud->nntp  = x3;
		   mud->http  = x4;
		   mud->pop3  = x5;
		   mud->rcp   = x6;
		   mud->amrcp = x7;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'P':
		if( !str_cmp( word, "Ports" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=0;

	         sscanf( ln, "%d %d %d ", &x1, &x2, &x3 );
		   mud->player_port   = x1;
		   mud->imud_tcp_port = x2;
		   mud->imud_udp_port = x3;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'S':
		KEY( "Status",		mud->status,		i3fread_number( fin ) );
		if( !str_cmp( word, "Services" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=0;

	         sscanf( ln, "%d %d %d %d %d %d %d %d %d %d %d %d", 
			&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12 );
		   mud->tell    = x1;
		   mud->beep    = x2;
		   mud->emoteto = x3;
		   mud->who     = x4;
		   mud->finger  = x5;
		   mud->locate  = x6;
		   mud->channel = x7;
		   mud->news    = x8;
		   mud->mail    = x9;
		   mud->file    = x10;
		   mud->auth    = x11;
		   mud->ucache  = x12;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'T':
		SKEY( "Telnet",		mud->telnet,		i3fread_string( fin ) );
		SKEY( "Type",		mud->mud_type,		i3fread_string( fin ) );
		break;

	   case 'W':
		SKEY( "Web",			mud->web,			i3fread_string( fin ) );
		break;		
	}

	if ( !fMatch )
	{
	    sprintf( log_buf, "I3_readmudlist: no match: %s", word );
	    i3bug( log_buf );
	}
   }
}

/* Called only during copyovers */
void I3_loadmudlist( void )
{
    FILE *fin;
    I3_MUD *mud;

    if( ( fin = fopen( I3_MUDLIST_FILE, "r" ) ) == NULL ) 
	return;

    for ( ; ; )
    {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "I3_loadmudlist: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if ( !str_cmp( word, "MUDLIST" ) )
	{
	   word = i3fread_word( fin );
	   if( !str_cmp( word, "Name" ) )
	   {
		char *tmpname;

		tmpname = i3fread_string( fin );
		mud = new_I3_mud( tmpname );
		fread_mudlist( fin, mud );
	   }
	   else
	   {
	      i3bug( "fread_mudlist: No mudname saved, skipping entry." );
		i3fread_to_eol( fin );
		for( ; ; )
		{
               word   = feof( fin ) ? "End" : i3fread_word( fin );
		   if( str_cmp( word, "End" ) )
			i3fread_to_eol( fin );
		   else
			break;
		}
	   }  
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   sprintf( log_buf, "I3_loadmudlist: bad section: %s.", word );
	   i3bug( log_buf );
	   continue;
	}
    }
    FCLOSE( fin );
    unlink( I3_MUDLIST_FILE );
    return;
}

/* Called only during copyovers */
void I3_loadchanlist( void )
{
    FILE *fin;
    I3_CHANNEL *channel;

    if( ( fin = fopen( I3_CHANLIST_FILE, "r" ) ) == NULL ) 
	return;

    for ( ; ; )
    {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "I3_loadchanlist: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if ( !str_cmp( word, "I3CHAN" ) )
	{
	   CREATE( channel, I3_CHANNEL, 1 );
	   I3_readchannel( channel, fin );

	   channel->first_listener = NULL;
	   channel->last_listener = NULL;
	   channel->connected = FALSE;
	   LINK( channel, first_I3chan, last_I3chan, next, prev );
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   sprintf( log_buf, "I3_loadchanlist: bad section: %s.", word );
	   i3bug( log_buf );
	   continue;
	}
    }
    FCLOSE( fin );
    unlink( I3_CHANLIST_FILE );
    return;
}

/* Called only during copyovers */
void I3_savemudlist( void )
{
   FILE *fp;
   I3_MUD *mud;

   if( ( fp = fopen( I3_MUDLIST_FILE, "w" ) ) == NULL )
   {
	i3bug( "I3_savemudlist: Unable to write to mudlist file." );
	return;
   }

   for( mud = first_mud; mud; mud = mud->next )
   {
	/* Don't store muds that are down, who cares? They'll update themselves anyway */
      if( mud->status == 0 )
	   continue;

	fprintf( fp, "#MUDLIST\n" );
	fprintf( fp, "Name		%s~\n", mud->name );
	fprintf( fp, "Status		%d\n", mud->status );
	fprintf( fp, "IP			%s~\n", mud->ipaddress );
	fprintf( fp, "Mudlib		%s~\n", mud->mudlib );
	fprintf( fp, "Baselib		%s~\n", mud->base_mudlib );
	fprintf( fp, "Driver		%s~\n", mud->driver );
	fprintf( fp, "Type		%s~\n", mud->mud_type );
	fprintf( fp, "Openstatus	%s~\n", mud->open_status );
	fprintf( fp, "Email		%s~\n", mud->admin_email );
      if( mud->telnet )
	   fprintf( fp, "Telnet		%s~\n", mud->telnet );
	if( mud->web )
	   fprintf( fp, "Web		%s~\n", mud->web );
	fprintf( fp, "Ports %d %d %d\n", mud->player_port, mud->imud_tcp_port, mud->imud_udp_port );
	fprintf( fp, "Services %d %d %d %d %d %d %d %d %d %d %d %d\n",
	   mud->tell, mud->beep, mud->emoteto, mud->who, mud->finger, mud->locate, mud->channel, mud->news, mud->mail,
	   mud->file, mud->auth, mud->ucache );
	fprintf( fp, "OOBports %d %d %d %d %d %d %d\n", mud->smtp, mud->ftp, mud->nntp, mud->http, mud->pop3, mud->rcp, mud->amrcp );
	fprintf( fp, "End\n\n" );
   }
   fprintf( fp, "#END\n" );
   FCLOSE( fp );
   return;
}

/* Called only during copyovers */
void I3_savechanlist( void )
{
   FILE *fp;
   I3_CHANNEL *channel;

   if( ( fp = fopen( I3_CHANLIST_FILE, "w" ) ) == NULL )
   {
	i3bug( "I3_savechanlist: Unable to write to chanlist file." );
	return;
   }

   for( channel = first_I3chan; channel; channel = channel->next )
   {
	/* Don't save local channels, they are stored elsewhere */
	if( channel->local_name && channel->local_name[0] != '\0' )
	   continue;

	fprintf( fp, "#I3CHAN\n" );
	fprintf( fp, "ChanMud		%s~\n", channel->host_mud );
	fprintf( fp, "ChanName		%s~\n", channel->I3_name );
	fprintf( fp, "ChanStatus	%d\n", channel->status );
	fprintf( fp, "End\n\n" );
   }
   fprintf( fp, "#END\n" );
   FCLOSE( fp );
   return;
}

/*
 * Connect to the router and send the startup-packet.
 * Mud port is passed in from main() so that the information passed along to the I3
 * network regarding the mud's operational port is now determined by the mud's own
 * startup script instead of the I3 config file.
 */
void I3_main( bool forced, int mudport, bool isconnected )
{
   I3_CHANNEL *channel;
   CHAR_DATA *vch;

   i3wait = 0;
   reconattempts = 0;
   this_mud = NULL;
   first_mud = NULL;
   last_mud = NULL;
   first_ucache = NULL;
   last_ucache = NULL;
   debugstring[0] = '\0';

   if( !I3_read_config( mudport ) )
   {
	this_mud = NULL;
	I3_socket = -1;
	return;
   }

   if( ( !this_mud->autoconnect && !forced && !isconnected ) || ( isconnected && I3_socket < 1 ) )
   {
	i3log( "Intermud-3 network data loaded. Autoconnect not set. Will need to connect manually." );
	return;
   }
   else
	i3log( "Intermud-3 network data loaded. Initialiazing network connection..." );

   I3_loadchannels( );

   if( this_mud->ucache == TRUE )
      I3_load_ucache( );

   if( this_mud->ucache == TRUE )
   {
	I3_prune_ucache( );
	ucache_clock = current_time + 86400;
   }

   if( I3_socket < 1 )
      I3_socket = I3_connection_open( this_mud->routerIP, this_mud->routerPort );

   if( I3_socket < 1 )
   {
	i3wait = 100;
      return;
   }

   sleep( 1 );

   i3log( "Intermud-3 Network initialized." );

   if( !isconnected )
      I3_startup_packet( );
   else
   {
      I3_loadmudlist();
	I3_loadchanlist();
   }

   for( channel = first_I3chan; channel; channel = channel->next )
      I3_check_channel( channel, FALSE );

   for( vch = first_char; vch; vch = vch->next )
   {
	if( !IS_NPC(vch) && vch->desc )
	   I3_char_login( vch );
   }
}

int I3_send_shutdown( int delay ) 
{
   char s[MIL];

   if( !I3_is_connected() )
	return 0;

   I3_stats.count_shutdown++;

   I3_write_header( "shutdown", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL );
   sprintf( s, "%d", delay );
   I3_write_buffer( s );
   I3_write_buffer( ",})\r" );

   if( !I3_write_packet( I3_output_buffer ) )
	I3_connection_close( FALSE );

   return 0;
}

/*
 * Check for a packet and if one available read it and parse it.
 * Also checks to see if the mud should attempt to reconnect to the router.
 * This is an input only loop. Attempting to use it to send buffered output
 * just wasn't working out, so output has gone back to sending packets to the
 * router as soon as they're assembled.
 */
void I3_loop( void ) 
{
   I3_CHANNEL *channel;
   int ret;
   long size;
   fd_set in_set, out_set, exc_set;
   static struct timeval null_time;

   FD_ZERO( &in_set  );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );

   if( i3wait > 0 )
      i3wait--;

   /* This condition can only occur if you were previously connected and the socket was closed.
    * Will increase the interval between attempts if it takes more than 20 initial tries,
    * and will abandon attempts to reconnect if it takes more than 100 tries.
    */
   if( i3wait == 1 )
   {
      I3_socket = I3_connection_open( this_mud->routerIP, this_mud->routerPort ); 
	reconattempts++;
	if( I3_socket < 1 )
      {
         if( reconattempts <= 5 )
	      i3wait = 100; /* Wait for 100 game loops */
	   else if( reconattempts <= 20 )
	      i3wait = 5000; /* Wait for 5000 game loops */
	   else
	   {
            i3wait = -2; /* Abandon attempts - probably an ISP failure anyway if this happens :) */
		i3log( "Abandoning attempts to reconnect to Intermud-3 router. Too many failures." );
	   }
	   return;
	}

      sleep( 1 );

      i3log( "Connection to Intermud-3 router reestablished." );
      I3_startup_packet();
      for( channel = first_I3chan; channel; channel = channel->next )
         I3_check_channel( channel, TRUE );
	return;
   }

   if( !I3_is_connected() )
	return;

   /* Will prune the cache once every 24hrs after bootup time */
   if( ucache_clock <= current_time )
   {
	ucache_clock = current_time + 86400;
	I3_prune_ucache( );
   }

   FD_SET( I3_socket, &in_set );
   FD_SET( I3_socket, &out_set );
   FD_SET( I3_socket, &exc_set );

   if( select( I3_socket+1, &in_set, &out_set, &exc_set, &null_time ) < 0 ) 
   {
	perror( "I3_loop: select: Unable to poll I3_socket!" );
	I3_connection_close( TRUE );
	return;
   }

   if( FD_ISSET( I3_socket, &exc_set ) )
   {
	FD_CLR( I3_socket, &in_set );
	FD_CLR( I3_socket, &out_set );
	i3log( "I3: Exception raised on socket." );
	I3_connection_close( TRUE );
	return;
   }

   if( FD_ISSET( I3_socket, &in_set ) )
   {
	ret = read( I3_socket, I3_input_buffer + I3_input_pointer, MSL );
	if( !ret || ( ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
	{
	   FD_CLR( I3_socket, &out_set );
	   if( ret < 0 )
		i3log( "I3: Read error on socket." );
	   else
		i3log( "I3: EOF encountered on socket read." );
  	   I3_connection_close( TRUE );
	   return;
	}
	if( ret < 0 ) /* EAGAIN */
	   return;

	I3_input_pointer += ret;
   }

   memcpy( &size, I3_input_buffer, 4 );
   size = ntohl( size );

   if( size <= I3_input_pointer - 4 )
   { 
      I3_read_packet();
	I3_parse_packet();
   }
   return;
}

/*
 * Shutdown the connection towards the router.
 */
void I3_shutdown( int delay )
{
   free_i3data( );

   if( I3_socket < 1 )
   {
	i3log( "I3: I3_shutdown was called but no socket existed" );
	return;
   }

   /* Flush the outgoing buffer */
   if( I3_output_pointer != 4 )
	I3_write_packet( I3_output_buffer );
 
   I3_send_shutdown( delay );
   I3_connection_close( FALSE );
   I3_input_pointer = 0;
   I3_output_pointer = 4;
   I3_save_id( );
   sleep( 2 ); /* Short delay to allow the socket to close */
}

char *I3_find_social( CHAR_DATA *ch, char *sname, char *person, char *mud, bool victim )
{
   static char socname[MSL];
#ifdef SMAUGSOCIAL
   SOCIAL_DATA *social;
#elif defined(I3CIRCLE)
   int act_nr;
   struct social_messg *social;
#else
   int cmd;
   bool found;
#endif

   socname[0] = '\0';

#ifdef SMAUGSOCIAL
   if( ( social = find_social( sname ) ) == NULL )
   {
	i3_printf( ch, "&YSocial &W%s&Y does not exist on this mud.\n\r", sname );
	return socname;
   }
   if( person && person[0] != '\0' && mud && mud[0] != '\0' )
   {
	if( person && person[0] != '\0' && !str_cmp( person, CH_NAME(ch) ) 
	 && mud && mud[0] != '\0' && !str_cmp( mud, I3_THISMUD ) )
	   strcpy( socname, social->others_auto );
	else
	{
	   if( !victim )
	      strcpy( socname, social->others_found );
	   else
		strcpy( socname, social->vict_found );
	}
   }
   else
	strcpy( socname, social->others_no_arg );
#elif defined(I3CIRCLE)
   if( ( act_nr = find_action( sname ) ) < 0 )
   {
      i3_printf( ch, "&YSocial &W%s&Y does not exist on this mud.\n\r", sname );
      return socname;
   }
   social = &soc_mess_list[act_nr];

   if( person && person[0] != '\0' && mud && mud[0] != '\0' )
   {
	if( person && person[0] != '\0' && !str_cmp( person, CH_NAME(ch) ) 
	 && mud && mud[0] != '\0' && !str_cmp( mud, I3_THISMUD ) )
	   strcpy( socname, social->others_auto );
	else
	{
	   if( !victim )
	      strcpy( socname, social->others_found );
	   else
		strcpy( socname, social->vict_found );
	}
   }
   else
	strcpy( socname, social->others_no_arg );
#else
   found  = FALSE;
   for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
   {
	if( sname[0] == social_table[cmd].name[0] && !str_prefix( sname, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
   }

   if ( !found )
   {
	i3_printf( ch, "&YSocial &W%s&Y does not exist on this mud.\n\r", sname );
	return socname;
   }
   if( person && person[0] != '\0' && mud && mud[0] != '\0' )
   {
	if( person && person[0] != '\0' && !str_cmp( person, CH_NAME(ch) ) 
	 && mud && mud[0] != '\0' && !str_cmp( mud, I3_THISMUD ) )
	   strcpy( socname, social_table[cmd].others_auto );
	else
	{
	   if( !victim )
	      strcpy( socname, social_table[cmd].others_found );
	   else
		strcpy( socname, social_table[cmd].vict_found );
	}
   }
   else
	strcpy( socname, social_table[cmd].others_no_arg );
#endif
   return socname;
}

/* Modified form of Smaug's act_string */
char *i3act_string( const char *format, CHAR_DATA *to, CHAR_DATA *ch, const void *arg1 )
{
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
  static char buf[MSL];
  char *point = buf;
  const char *str = format;
  const char *i;
  CHAR_DATA *vch = (CHAR_DATA *) arg1;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    ++str;
    if ( !arg1 && *str >= 'A' && *str <= 'Z' )
    {
	sprintf( log_buf, "i3act_string: missing arg1 for code %c:", *str );
      i3bug( log_buf );
      i3bug( (char *)format );
      i = " <@@@> ";
    }
    else
    {
      switch ( *str )
      {
      default:  
		sprintf( log_buf, "i3act_string: bad code %c.", *str );
		i3bug( log_buf );
		i = " <@@@> ";						break;

	case '$': 
	    i = "$";
	    break;
      case 't': i = (char *) arg1;					break;
      case 'T': i = (char *) arg1;					break;
      case 'n':
		  i = "$N";
              break;
      case 'N':
		  i = "$O";
		  break;
      case 'e': 
		if( CH_SEX(ch) > 2 || CH_SEX(ch) < 0 )
		  i = "it";
		else
		  i = he_she [URANGE(0,  CH_SEX(ch), 2)];
		break;
      case 'E':
		if( CH_SEX(vch) > 2 || CH_SEX(vch) < 0 )
		  i = "it";
		else
		  i = he_she [URANGE(0, CH_SEX(vch), 2)];
		break;
      case 'm': 
		if( CH_SEX(ch) > 2 || CH_SEX(ch) < 0 )
		  i = "it";
		else
		  i = him_her[URANGE(0,  CH_SEX(ch), 2)];
		break;
      case 'M': 
		if( CH_SEX(vch) > 2 || CH_SEX(vch) < 0 )
		  i = "it";
		else
		  i = him_her[URANGE(0, CH_SEX(vch), 2)];
		break;
      case 's':
		if( CH_SEX(ch) > 2 || CH_SEX(ch) < 0 )
		  i = "its";
		else
		  i = his_her[URANGE(0,  CH_SEX(ch), 2)];
		break;
      case 'S': 
		if( CH_SEX(vch) > 2 || CH_SEX(vch) < 0 )
		  i = "its";
		else
		  i = his_her[URANGE(0, CH_SEX(vch), 2)];
		break;
      }
    }
    ++str;
    while ( (*point = *i) != '\0' )
      ++point, ++i;
  }
  *point = '\0';
  return buf;
}

CHAR_DATA *I3_make_skeleton( char *name )
{
   CHAR_DATA *skeleton;

   CREATE( skeleton, CHAR_DATA, 1 );

#ifdef I3CIRCLE
   skeleton->player.name = I3STRALLOC( name );
   skeleton->player.short_descr = I3STRALLOC( name );
   skeleton->in_room = 1;
#else
   skeleton->name = I3STRALLOC( name );
   skeleton->short_descr = I3STRALLOC( name );
   skeleton->in_room = get_room_index( ROOM_VNUM_LIMBO );
#endif

   return skeleton;
}

void I3_purge_skeleton( CHAR_DATA *skeleton )
{
   if( !skeleton )
      return;

#ifdef I3CIRCLE
   I3STRFREE( skeleton->player.name );
   I3STRFREE( skeleton->player.short_descr );
#else
   I3STRFREE( skeleton->name );
   I3STRFREE( skeleton->short_descr );
#endif
   DISPOSE( skeleton );

   return;
}

void I3_send_social( I3_CHANNEL *channel, CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *skeleton = NULL;
   char *ps;
   char socbuf_o[MSL];
   char socbuf_t[MSL];
   char msg_o[MSL], msg_t[MSL];
   char arg1[MIL];
   char arg2[MIL];
   char person[MSL];
   char mud[MSL];
   int x;

   person[0] = '\0';
   mud[0] = '\0';

   /* Name of social */
   argument = i3one_argument( argument, arg1 );

   /* Target of social, if given */
   argument = i3one_argument( argument, arg2 );

   if( arg2 && arg2[0] != '\0' )
   {
      if( ( ps = strchr( arg2, '@' ) ) == NULL )
      {
	   i3_to_char( "You need to specify a person@mud for a target.\n\r", ch );
	   return;
      }
      else
      {
         for( x = 0; x < strlen( arg2 ); x++ )
         {
	      person[x] = arg2[x];
	      if( person[x] == '@' )
	         break;
         }
         person[x] = '\0';

         ps[0] = 0;
	   strcpy( mud, ps+1 );
      }
   }

   sprintf( socbuf_o, "%s", I3_find_social( ch, arg1, person, mud, FALSE ) );
   sprintf( socbuf_t, "%s", I3_find_social( ch, arg1, person, mud, TRUE ) );

   if( (socbuf_o && str_cmp( socbuf_o, "" ))
    && (socbuf_t && str_cmp( socbuf_t, "" )) )
   {
	if( arg2 && arg2[0] != '\0' )
	{
	   int sex;

	   sprintf( log_buf, "%s@%s", person, mud );
	   sex = I3_get_ucache_gender( log_buf );
	   if( sex == -1 )
	   {
	      /* Greg said to "just punt and call them all males".
		 * I decided to meet him halfway and at least request data before punting :)
		 */
            I3_send_chan_user_req( mud, person );
		sex = SEX_MALE;
	   }
	   else
		sex = i3todikugender( sex );

	   skeleton = I3_make_skeleton( log_buf );
	   CH_SEX(skeleton) = sex;
	}

	strcpy( msg_o, (char *)i3act_string( socbuf_o, NULL, ch, skeleton ) );
	strcpy( msg_t, (char *)i3act_string( socbuf_t, NULL, ch, skeleton ) );

	if( !skeleton )
	   I3_send_channel_emote( channel, CH_NAME(ch), msg_o );
	else
	{
	   sprintf( log_buf, "%s", person );
	   tolower( log_buf[0] );
	   I3_send_channel_t( channel, CH_NAME(ch), mud, log_buf, msg_o, msg_t, person );
	}
	if( skeleton )
	   I3_purge_skeleton( skeleton );
   }
   else
	i3_printf( ch, "&YSocial &W%s&Y is incomplete.\n\r", arg1 );
   return;
}

/*
 * This is how channels are interpreted. If they are not commands
 * or socials, this function will go through the list of channels
 * and send it to it if the name matches the local channel name.
 */
bool I3_command_hook( CHAR_DATA *ch, char *command, char *argument ) 
{
   I3_CHANNEL *channel;

   if( ( channel = find_I3_channel_by_localname( command ) ) == NULL )
	return FALSE;

   if( !find_I3_listener_by_char( channel, ch ) )
   {
	i3_printf( ch, "&YYou were trying to send something to an I3 "
	   "channel but you're not subcribed to it. Please use the command "
	   "'&WI3 listen %s&Y' to subscribe to it.\n\r", channel->local_name );
	return TRUE;
   }

   switch( argument[0] )
   {
	case ',':
	   I3_send_channel_emote( channel, CH_NAME(ch), argument+1 );
	   break;
	case '@':
	   I3_send_social( channel, ch, argument+1 );
	   break;
	default:
	   I3_send_channel_message( channel, CH_NAME(ch), argument );
	   break;
   }
   return 1;
}
#endif
