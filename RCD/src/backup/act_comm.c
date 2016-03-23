/****************************************************************************
 * ResortMUD 4.0 Beta by Ntanel, Garinan, Badastaz, Josh, Digifuzz, Senir,  *
 * Kratas, Scion, Shogar and Tagith.  Special thanks to Thoric, Nivek,      *
 * Altrag, Arlorn, Justice, Samson, Dace, HyperEye and Yakkov.              *
 ****************************************************************************
 * Copyright (C) 1996 - 2001 Haslage Net Electronics: MudWorld              *
 * of Lorain, Ohio - ALL RIGHTS RESERVED                                    *
 * The text and pictures of this publication, or any part thereof, may not  *
 * be reproduced or transmitted in any form or by any means, electronic or  *
 * mechanical, includes photocopying, recording, storage in a information   *
 * retrieval system, or otherwise, without the prior written or e-mail      *
 * consent from the publisher.                                              *
 ****************************************************************************
 * GREETING must mention ResortMUD programmers and the help file named      *
 * CREDITS must remain completely intact as listed in the SMAUG license.    *
 ****************************************************************************/

/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			   Player communication module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
/* #include "rx.h" Removed by fellon */

#ifndef WIN32
  /* #include <regex.h> */
  /* #include "rx.h" */
#endif

/*
 *  Externals
 */
void send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void send_control_page_to_char(CHAR_DATA * ch, char page);
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		 const void *arg1, const void *arg2, int flags);
/* mobchat.c -- Scion */
/* void do_mobchat(CHAR_DATA *vict, CHAR_DATA *ch, char *message); */

/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, const char *verb ) );

char *  scramble        args( ( const char *argument, int modifier ) );			    
char *  drunk_speech    args( ( const char *argument, CHAR_DATA *ch ) );

void drop_artifacts(CHAR_DATA *ch, OBJ_DATA *obj); /* Scion */ 

/*
 *  Profanity handler stuff  (forgive me for what i must do)
 */
void add_profane_word( char * word);
int is_profane (char *what);
char *bigregex = NULL;
char * preg;


bool room_is_silent( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *room = ch->in_room;

    if ( IS_SET( room->room_flags, ROOM_SILENCE ) )
        return TRUE;

    /* if ( xIS_SET( room->affected_by, ROOM_AFF_SILENCE ) )
        return TRUE; */

    return FALSE;
}

/* Text scrambler -- Altrag */
char *scramble( const char *argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;
    
    modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */
    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
    	if ( argument[position] == '\0' )
    	{
    		arg[position] = '\0';
    		return arg;
    	}
    	else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'A';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'A';
	    }
	    else if ( argument[position] >= 'a' && argument[position] <= 'z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'a';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'a';
	    }
	    else if ( argument[position] >= '0' && argument[position] <= '9' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - '0';
	    	conversion = number_range( conversion - 2, conversion + 2 );
	    	while ( conversion > 9 )
	    		conversion -= 10;
	    	while ( conversion < 0 )
	    		conversion += 10;
	    	arg[position] = conversion + '0';
	    }
	    else
	    	arg[position] = argument[position];
	}
	arg[position] = '\0';
	return arg;	     
}

/* I'll rewrite this later if its still needed.. -- Altrag
char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}
*/

LANG_DATA *get_lang(const char *name)
{
    LANG_DATA *lng;

    for (lng = first_lang; lng; lng = lng->next)
	if (!str_cmp(lng->name, name))
	    return lng;
    return NULL;
}

/* percent = percent knowing the language. */
char *translate(int percent, const char *in, const char *name)
{
    LCNV_DATA *cnv;
    static char buf[256];
    char buf2[256];
    const char *pbuf;
    char *pbuf2 = buf2;
    LANG_DATA *lng;

    if ( percent > 99 || !str_cmp(name, "common") )
	return (char *) in;

    /* If we don't know this language... use "default" */
    if ( !(lng=get_lang(name)) )
	if ( !(lng = get_lang("default")) )
	    return (char *) in;

    for (pbuf = in; *pbuf;)
    {
	for (cnv = lng->first_precnv; cnv; cnv = cnv->next)
	{
	    if (!str_prefix(cnv->old, pbuf))
	    {
		if ( percent && (rand() % 100) < percent )
		{
		    strncpy(pbuf2, pbuf, cnv->olen);
		    pbuf2[cnv->olen] = '\0';
		    pbuf2 += cnv->olen;
		}
		else
		{
		    strcpy(pbuf2, cnv->new);
		    pbuf2 += cnv->nlen;
		}
		pbuf += cnv->olen;
		break;
	    }
	}
	if (!cnv)
	{
	    if (isalpha(*pbuf) && (!percent || (rand() % 100) > percent) )
	    {
		*pbuf2 = lng->alphabet[LOWER(*pbuf) - 'a'];
		if ( isupper(*pbuf) )
		    *pbuf2 = UPPER(*pbuf2);
	    }
	    else
		*pbuf2 = *pbuf;
	    pbuf++;
	    pbuf2++;
	}
    }
    *pbuf2 = '\0';
    for (pbuf = buf2, pbuf2 = buf; *pbuf;)
    {
	for (cnv = lng->first_cnv; cnv; cnv = cnv->next)
	    if (!str_prefix(cnv->old, pbuf))
	    {
		strcpy(pbuf2, cnv->new);
		pbuf += cnv->olen;
		pbuf2 += cnv->nlen;
		break;
	    }
	if (!cnv)
	    *(pbuf2++) = *(pbuf++);
    }
    *pbuf2 = '\0';
#if 0
    for (pbuf = in, pbuf2 = buf; *pbuf && *pbuf2; pbuf++, pbuf2++)
	if (isupper(*pbuf))
	    *pbuf2 = UPPER(*pbuf2);
    /* Attempt to align spacing.. */
	else if (isspace(*pbuf))
	    while (*pbuf2 && !isspace(*pbuf2))
		pbuf2++;
#endif
    return buf;
}


char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
  const char *arg = argument;
  static char buf[MAX_INPUT_LENGTH*2];
  char buf1[MAX_INPUT_LENGTH*2];
  sh_int drunk;
  char *txt;
  char *txt1;  

  if ( IS_NPC( ch ) || !ch->pcdata ) return (char *) argument;

  drunk = ch->pcdata->condition[COND_DRUNK];

  if ( drunk <= 0 )
    return (char *) argument;

  buf[0] = '\0';
  buf1[0] = '\0';

  if ( !argument )
  {
     bug( "Drunk_speech: NULL argument", 0 );
     return "";
  }

  /*
  if ( *arg == '\0' )
    return (char *) argument;
  */

  txt = buf;
  txt1 = buf1;

  while ( *arg != '\0' )
  {
    if ( toupper(*arg) == 'T' )
    {
	if ( number_percent() < ( drunk * 2 ) )		/* add 'h' after an 'T' */
	{
	   *txt++ = *arg;
	   *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( toupper(*arg) == 'X' )
    {
	if ( number_percent() < ( drunk * 2 / 2 ) )
	{
	  *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( number_percent() < ( drunk * 2 / 5 ) )  /* slurred letters */
    {
      sh_int slurn = number_range( 1, 2 );
      sh_int currslur = 0;	

      while ( currslur < slurn )
	*txt++ = *arg, currslur++;
    }
   else
    *txt++ = *arg;

    arg++;
  };

  *txt = '\0';

  txt = buf;

  while ( *txt != '\0' )   /* Let's mess with the string's caps */
  {
    if ( number_percent() < ( 2 * drunk / 2.5 ) )
    {
      if ( isupper(*txt) )
        *txt1 = tolower( *txt );
      else
      if ( islower(*txt) )
        *txt1 = toupper( *txt );
      else
        *txt1 = *txt;
    }
    else
      *txt1 = *txt;

    txt1++, txt++;
  };

  *txt1 = '\0';
  txt1 = buf1;
  txt = buf;

  while ( *txt1 != '\0' )   /* Let's make them stutter */
  {
    if ( *txt1 == ' ' )  /* If there's a space, then there's gotta be a */
    {			 /* along there somewhere soon */

      while ( *txt1 == ' ' )  /* Don't stutter on spaces */
        *txt++ = *txt1++;

      if ( ( number_percent() < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
      {
	sh_int offset = number_range( 0, 2 );
	sh_int pos = 0;

	while ( *txt1 != '\0' && pos < offset )
	  *txt++ = *txt1++, pos++;

	if ( *txt1 == ' ' )  /* Make sure not to stutter a space after */
	{		     /* the initial offset into the word */
	  *txt++ = *txt1++;
	  continue;
	}

	pos = 0;
	offset = number_range( 2, 4 );	
	while (	*txt1 != '\0' && pos < offset )
	{
	  *txt++ = *txt1;
	  pos++;
	  if ( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */ 
	  {		               /* A hyphen right before a space	*/
	    txt1--;
	    break;
	  }
	  *txt++ = '-';
	}
	if ( *txt1 != '\0' )
	  txt1++;
      }     
    }
   else
    *txt++ = *txt1++;
  }

  *txt = '\0';

  return buf;
}

/*
 * An attempt to reduce redundancy                 - Tagith
 */
bool can_see_chan( CHAR_DATA *ch, DESCRIPTOR_DATA *d, int channel )
{
    CHAR_DATA *och;
    CHAR_DATA *vch;
    
    if(!d) return FALSE; /* catch the link dead - shogar */
    och = CH(d);
    vch = d->character;
    
    if ( IS_SET( och->deaf, channel ) )
	return FALSE;

    if ( channel != CHANNEL_NEWBIE  && NOT_AUTHED(och) )
        return FALSE;
    if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och) )
       	return FALSE;
    if ( channel == CHANNEL_WARTALK && NOT_AUTHED( och ) )
        return FALSE;
    if ( channel == CHANNEL_AVTALK  && !IS_HERO(och) )
        return FALSE;
    if ( channel == CHANNEL_HIGHGOD && get_trust( och ) < sysdata.staff_level )
	return FALSE;
    if ( channel == CHANNEL_HIGH    && get_trust( och ) < sysdata.think_level )
	return FALSE;

    /* Fix by Narn to let newbie council members see the newbie channel. */
    if ( channel == CHANNEL_NEWBIE  &&
      ( !IS_IMMORTAL(och) && !NOT_AUTHED(och)
      && (!str_cmp( och->pcdata->council_name, "Newbie Council" )
      ||  !str_cmp( och->pcdata->council2_name, "Newbie Council" ) ) ) )
        return FALSE;
    
    if ( room_is_silent(vch) )
       	return FALSE;

    if ( channel == CHANNEL_YELL
    &&   vch->in_room->area != ch->in_room->area )
      	return FALSE;

    if ( channel == CHANNEL_CLAN || channel == CHANNEL_ORDER || channel == CHANNEL_GUILD )
    {
       	if ( IS_NPC( vch ) )
            return FALSE;
       	if ( vch->pcdata->clan != ch->pcdata->clan )
            return FALSE;
    }

    /* if ( channel == CHANNEL_GUILD )
    {
     	if ( IS_NPC( vch ) )
            return FALSE;
       	if (vch->pcdata->guild != ch->pcdata->guild )
            return FALSE;
    } */

    if ( channel == CHANNEL_COUNCIL )
    {
        if ( IS_NPC( vch ) )
            return FALSE;
        if (( vch->pcdata->council != ch->pcdata->council 
	 && vch->pcdata->council2 != ch->pcdata->council ))
            return FALSE;
    }
    if ( channel == CHANNEL_COUNCIL2)
    {
	if ( IS_NPC( vch ) )
	   return FALSE;
	if (( vch->pcdata->council != ch->pcdata->council2
	&& vch->pcdata->council2 != ch->pcdata->council2 ))
           return FALSE;
    }
return TRUE;
}

/*
 * Allows people to do socials, even targeted, over channels	   - Tagith
 */
void chan_social( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char otherbuf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    SOCIALTYPE *social;
    DESCRIPTOR_DATA *d;
    sh_int AType;
    int position;
    char *txt;

    *argument = (int) argument++;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
   
    if ( ( social = find_social( arg1 ) ) == NULL )
    {
	send_to_char( "No such social.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return;
    }
   
    switch( channel )
    {
    default:
        AType = AT_GOSSIP;
        break;
    case CHANNEL_IMMTALK:
    case CHANNEL_AVTALK:
        AType = AT_IMMORT;
        break;
    case CHANNEL_WARTALK:
        AType = AT_WARTALK;
        break;
    }

    otherbuf[0] = '\0';
    if ( arg2[0] == '\0' )
    {
	position = ch->position;
	ch->position = POS_STANDING;
	sprintf( buf, "[%s] %s", verb, social->char_no_arg );
        act( AType, buf,   ch, NULL, victim, TO_CHAR    );
	ch->position = position;

	if( social->others_no_arg && social->others_no_arg[0] != '\0' )
	    sprintf( otherbuf, "[%s] %s", verb, social->others_no_arg );
    }
    else if ( ( victim = get_char_world( ch, arg2 ) ) == NULL || IS_NPC(victim) )
	send_to_char( "They aren't here.\n\r", ch );
    else if ( victim == ch )
    {
	sprintf( buf, "[%s] %s", verb, social->char_auto );
	position = ch->position;
	ch->position = POS_STANDING;
        act( AType, buf, ch, NULL, victim, TO_CHAR );
	ch->position = position;
	
	if ( social->others_auto && social->others_auto[0] != '\0' )
	    sprintf( otherbuf, "[%s] %s", verb, social->others_auto );
    }
    else
    {
	sprintf( buf, "[%s] %s", verb, social->char_found );
	position = ch->position;
	ch->position = POS_STANDING;
	act( AType, buf, ch, NULL, victim, TO_CHAR );
	ch->position = position;
	if ( social->vict_found
	&&   social->vict_found[0] != '\0'
	&&   can_see_chan( ch, victim->desc, channel ) )
	{ 
	    sprintf( buf, "[%s] %s", verb, social->vict_found );
	    position = victim->position;
	    victim->position = POS_STANDING;
	    act( AType, buf, ch, NULL, victim, TO_VICT );
	    victim->position = position;
	}

	if ( social->others_found && social->others_found[0] != '\0' )
	    sprintf( otherbuf, "[%s] %s", verb, social->others_found );
    }

    /* An attempt at reducing redundancy */
    if ( !otherbuf || otherbuf[0] == '\0' )
	return;

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *vch = d->character;

	if ( d->connected == CON_PLAYING
	&&   vch != ch
	&&   vch != victim
	&&   can_see_chan( ch, d, channel ) )
	{

	    if ( IS_IMMORTAL(vch) )
		txt = act_string( otherbuf, vch, ch, NULL, victim, STRING_IMM );
	    else
		txt = act_string( otherbuf, vch, ch, NULL, victim, STRING_NONE );

	    set_char_color( AType, vch );
	    send_to_char_color( txt, vch );
	}
	else
	    continue;
    }
    return;
}

/*
 * Generic channel function.  Revamped by Justice of AaernMUD, based on SMAUG 1.02a.
 * August 3, 1998.  Removal of redundant code, and implementation of global socials.
 * August 25, 1998.  Added global emotes and help comments.
 * New talk_channel code originally written for GOTA
 * act_string prototype necessary for global social code.
 * October 7, 1998  Aaern came up yesterday, and I am now converting my talk_channel
 * code to SMAUG 1.4
 * March 19, 1999   Adding AaernMUD Talk_Channel code to UCMM --Justice
 */
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
                 const void *arg1, const void *arg2, int flags);
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    char             buf[MAX_STRING_LENGTH];
    char             buf2[MAX_STRING_LENGTH];
    char             arg[MAX_STRING_LENGTH];
    char             soc_char[MAX_STRING_LENGTH];
    char             soc_vict[MAX_STRING_LENGTH];
    char             soc_others[MAX_STRING_LENGTH];
    char             emotetxt[MAX_STRING_LENGTH];
    int              position, i;
    CHAR_DATA       *victim = NULL;
    DESCRIPTOR_DATA *d;
    SOCIALTYPE      *social = NULL;

    struct channel_data
    {
        int channel;
        char *prefix;
        char *act_text;
        char *buf_text;
        int color;
        int type;
    } fields[] = {
    { CHANNEL_CHAT,     "&W[&G%s&W] &G%s&w", "&W[&G%s&W] &w$n&W: &G$t&w", "&W[&G%s&W] &wYou&W: &G%s&w\n\r", AT_GOSSIP,   1 },
    { CHANNEL_WARTALK,  "&W[&r%s&W] &r%s&w", "&W[&r%s&W] &w$n&W: &r$t&w", "&W[&r%s&W] &wYou&W: &r%s&w\n\r", AT_WARTALK,  1 },
    { CHANNEL_IMMTALK,  "&W[&Y%s&W] &Y%s&w", "&W[&Y%s&W] &w$n&W: &Y$t&w", "&W[&Y%s&W] &wYou&W: &Y%s&w\n\r", AT_IMMORT,   2 },
    { CHANNEL_AVTALK,   "&W[&O%s&W] &O%s&w", "&W[&O%s&W] &w$n&W: &O$t&w", "&W[&O%s&W] &wYou&W: &O%s&w\n\r", AT_AVTALK,   2 },
    { CHANNEL_RACETALK, "&W[&B%s&W] &B%s&w", "&W[&B%s&W] &w$n&W: &B$t&w", "&W[&B%s&W] &wYou&W: &B%s&w\n\r", AT_RACETALK, 1 },
	{ CHANNEL_MUSIC,"&W[&P%s&W] &P%s&w", "&W[&P%s&W] &w$n&W: &P$t&w", "&W[&P%s&W] &wYou&W: &P%s&w\n\r", AT_MUSIC, 1 },
	{ CHANNEL_CLAN, "&W[&R%s&W] &R%s&w", "&W[&R%s&W] &w$n&W: &R$t&w", "&W[&R%s&W] &wYou&W: &R%s&w\n\r", AT_CLANTALK,  1 },
	{ CHANNEL_GRATZ,"&C[&P%s&C] &C%s&w", "&C[&P%s&C] &w$n&W: &C$t&w", "&C[&P%s&C] &wYou&W: &C%s&w\n\r", AT_GRATZ, 1 },
	{ CHANNEL_THINK,"&W[&C%s&W] &C%s&w", "&W[&C%s&W] &w$n&W: &C$t&w", "&W[&C%s&W] &wYou&W: &C%s&w\n\r", AT_CYAN, 1 },
    { -1,               "&W[&c%s&W] &c%s&w", "&W[&c%s&W] &w$n&W: &c$t&w", "&W[&c%s&W] &wYou&W: &c%s&w\n\r", AT_CHAN_DEFAULT,   1 }
    };

#ifndef SCRAMBLE
    int speaking = -1, lang;
 
    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        if ( ch->speaking & lang_array[lang] )
        {
            speaking = lang;
            break;
        }
#endif

/*******************************************************************************************
 * Adding new channels:  Add another field, as demonstrated above.  Each channel must have *
 * 6 variables.  The first, is the channel flag, next you specify a prefix, which will be  *
 * used for socials and emotes.  The next variable is the act statement text which will be *
 * sent to the channel.  Afterward you define the text that is sent to the character, then *
 * you give it a color.  The last variable is to specify channel type.  Type 1 is normal,  *
 * type 2 is setup for imm channels (imm/avatar talk).                                     *
 * NOTE: The line containing -1 as the channel flag should always be the last channel, any *
 *       channels behind it will be ignored.                                               *
 *******************************************************************************************/

    if ( IS_NPC( ch )
		 && (( 
		        channel == CHANNEL_CLAN 
		     || channel == CHANNEL_ORDER 
		     || channel == CHANNEL_COUNCIL
		     || channel == CHANNEL_COUNCIL2 
		     || channel == CHANNEL_GUILD
	        )  
		 ||   ((IS_AFFECTED(ch, AFF_CHARM)) && (!xIS_SET(ch->act, ACT_PET)))
   	     ||   ((xIS_SET(ch->act, PLR_SILENCE)) && (!IS_NPC(ch))) 
         ||   (IS_SET(ch->in_room->room_flags, ROOM_SILENCE)) 
	   ))
    {
        ch_printf( ch, "For some odd reason, you lose your voice and can't %s.\n\r", capitalize( verb ) );
        if ( ch->master ) 
            ch_printf( ch->master, "A mystical force prevents %s from speaking.\n\r", IS_NPC( ch ) ? 
                ch->short_descr : capitalize( ch->name ) );
        return;
    }

    if ( argument[0] == '\0' )
    {
        ch_printf( ch, "%s what?", capitalize( verb ) );
        return;
    }

    REMOVE_BIT( ch->deaf, channel );

    for ( i = 0; fields[i].channel != -1; i++ )
        if ( fields[i].channel == channel )
            break;

    set_char_color( fields[i].color, ch );
    if ( argument[0] == '@' ) /* was '@@', not sure whether it's bad or not -- Scion */
    {
        argument = one_argument( argument, arg );
        if ( ( social = find_social( arg+1 ) ) == NULL )
        {
            ch_printf( ch, "%s is not a social!\n\r", capitalize( arg+1 ) );
            return;
        }
        else
        {
            if ( argument[0] != '\0' && ( victim = get_char_world( ch, argument ) ) != NULL )
            {
                if ( victim == ch )
                {
                    sprintf( soc_char, fields[i].prefix, capitalize( verb ), social->char_auto );
                    sprintf( soc_others, fields[i].prefix, capitalize( verb ), social->others_auto );
                    act( fields[i].color, soc_char, ch, NULL, victim, TO_CHAR );
                }
                else
                {
                    sprintf( soc_char, fields[i].prefix, capitalize( verb ), social->char_found );
                    sprintf( soc_vict, fields[i].prefix, capitalize( verb ), social->vict_found );
                    sprintf( soc_others, fields[i].prefix, capitalize( verb ), social->others_found );
                    act( fields[i].color, soc_char, ch, NULL, victim, TO_CHAR );
                }
            }
            else
            {
                if ( argument[0] != '\0' ) send_to_char( "You can't find that person.\n\r", ch );
                sprintf( soc_char, fields[i].prefix, capitalize( verb ), social->char_no_arg );
                sprintf( soc_others, fields[i].prefix, capitalize( verb ), social->others_no_arg );
                act( fields[i].color, soc_char, ch, NULL, victim, TO_CHAR );
            }
        }
    }
    else if ( argument[0] == ',' )
    {
        sprintf( emotetxt, "%s %s", capitalize( ch->name ), argument+1 );
        ch_printf( ch, fields[i].prefix, capitalize( verb ), emotetxt );
        send_to_char( "\n\r", ch );
        sprintf( buf, fields[i].prefix, capitalize( verb ), "$n $t" );
        argument = argument+1;
    }
    else if ( fields[i].type == 1 )
    {
        ch_printf( ch, fields[i].buf_text, capitalize( verb ), argument );
        sprintf( buf, fields[i].act_text, capitalize( verb ));
    }
    else if ( fields[i].type == 2 )
    {
        ch_printf( ch, fields[i].buf_text, capitalize( verb ), argument );
        sprintf( buf, fields[i].act_text, capitalize( verb ));
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf2, "%s: %s (%s)", IS_NPC( ch ) ? ch->short_descr : capitalize( ch->name ),
            argument, verb );
        append_to_file( LOG_FILE, buf2 );
    }

#ifdef HMM
    if ( is_profane(argument) )
    {
        sprintf( buf2, "%s Profanity warning: %s: %s (%s)",
                        "say",
                        IS_NPC( ch ) ? ch->short_descr : ch->name,
                        argument,
                        verb );
        /* force Puff mpat 6 mpforce imp mpat 1 say hi */
    
        puff = get_char_world( ch, "Puff" );
        if(puff!=NULL)
        {
          if ( ( location = get_room_index( 1 ) ) != NULL ){
              original = puff->in_room;
              char_from_room( puff );
              char_to_room( puff, location );
              interpret( puff, buf2 );
              char_to_room( puff, original );
          }
        }
    }
#endif

    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

        och = d->original ? d->original : d->character;
        vch = d->character;

        if ( d->connected == CON_PLAYING && vch != ch && !IS_SET( och->deaf, channel ) )
        {
            char *sbuf = argument;
            char lbuf[MAX_INPUT_LENGTH + 4]; /* invis level string + buf */
 
            if ( ( NOT_AUTHED( och ) && ( channel != CHANNEL_NEWBIE || channel == CHANNEL_WARTALK ) )
            ||   ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) )
            ||   ( channel == CHANNEL_AVTALK  && !IS_HERO( och ) )
            ||   ( channel == CHANNEL_HIGHGOD && get_trust( och ) < sysdata.staff_level )
            ||   ( channel == CHANNEL_HIGH    && get_trust( och ) < sysdata.think_level )
            ||   ( channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area )
            ||   ( channel == CHANNEL_NEWBIE && ( !IS_IMMORTAL( och ) && !NOT_AUTHED( och )
            &&   ( !( och->pcdata->council && !str_cmp( och->pcdata->council->name, "Newbie Council" ) )
		|| !( och->pcdata->council2 && !str_cmp( och->pcdata->council2_name, "Newbie Council") ) ) ) )
            ||   ( ( channel == CHANNEL_CLAN || channel == CHANNEL_GUILD || channel == CHANNEL_ORDER )
            &&     ( IS_NPC( vch ) || vch->pcdata->clan != ch->pcdata->clan ) )
            ||   ( ( channel == CHANNEL_RACETALK ) && ( vch->race != ch->race ) )
            ||   ( channel == CHANNEL_COUNCIL &&  ( IS_NPC( ch ) 
            ||    !( vch->pcdata->council != ch->pcdata->council
	    &&	   vch->pcdata->council != ch->pcdata->council2 ) ) )
	    ||   ( channel == CHANNEL_COUNCIL2 && (IS_NPC( ch )
	    ||	   !( vch->pcdata->council2 != ch->pcdata->council
	    &&     vch->pcdata->council2 != ch->pcdata->council2 ) ) )
            ||   IS_WRITING( vch ) )
                continue;

            if ( xIS_SET(ch->act, PLR_WIZINVIS) &&
                can_see(vch, ch) && IS_IMMORTAL(vch))
            {
                sprintf(lbuf, "(%d) ", (!IS_NPC(ch))?ch->pcdata->wizinvis
                        :ch->mobinvis);
            }
            else
            {
                lbuf[0] = '\0';
            }
             
            position            = vch->position;
            if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
                vch->position   = POS_STANDING;
#ifndef SCRAMBLE
            if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
            {     
                int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
                                      knows_language(ch, ch->speaking, vch));
             
                if ( speakswell < 85 )
                    sbuf = translate(speakswell, argument, lang_names[speaking]);
            }
#else
            if ( !knows_language(vch, ch->speaking, ch)
            &&  (!IS_NPC(ch) || ch->speaking != 0) )
                sbuf = scramble(argument, ch->speaking);
#endif       
           /*  Scramble speech if vch or ch has nuisance flag */
            
            if ( !IS_NPC(ch) && ch->pcdata->nuisance
            &&   ch->pcdata->nuisance->flags > 7
            &&  (number_percent()<((ch->pcdata->nuisance->flags-7)*10*
                ch->pcdata->nuisance->power)))
                sbuf = scramble(argument,number_range(1,10));
                                      
            if ( !IS_NPC(vch) && vch->pcdata->nuisance &&
                vch->pcdata->nuisance->flags > 7
                 &&(number_percent()<((vch->pcdata->nuisance->flags-7)*10*
                 vch->pcdata->nuisance->power)))
                        sbuf = scramble(argument, number_range(1,10));

            MOBtrigger = FALSE;

            if ( social )
            {
                set_char_color( fields[i].color, vch );
                if ( victim && vch == victim )
                    send_to_char( act_string( soc_vict, victim, ch, NULL, victim, STRING_NONE ), vch );
                else
                    send_to_char( act_string( soc_others, vch, ch, NULL, victim, STRING_NONE ), vch );
            }
            else
                act( fields[i].color, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT );
            vch->position = position;
        }
    }
    return;
}

void to_channel( const char *argument, int channel, const char *verb, sh_int level )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( !first_descriptor || argument[0] == '\0' )
      return;

    sprintf(buf, "%s: %s\r\n", verb, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( !och || !vch )
	  continue;
	if ( !IS_IMMORTAL(vch)
	|| ( get_trust(vch) < sysdata.build_level && channel == CHANNEL_BUILD )
	|| ( get_trust(vch) < sysdata.log_level
	&& ( channel == CHANNEL_LOG || channel == CHANNEL_HIGH || 
	channel == CHANNEL_WARN ||channel == CHANNEL_COMM ) ) )
	  continue;

	if ( d->connected == CON_PLAYING
	&&  !IS_SET(och->deaf, channel)
	&&   get_trust( vch ) >= level )
	{
	  set_char_color( AT_LOG, vch );
	  send_to_char_color( buf, vch );
	}
    }

    return;
}


void do_chat( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_CHAT, "chat" );
    return;
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan 
    ||   ch->pcdata->clan->clan_type == CLAN_ORDER
    ||   ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_CLAN, "clantalk" );
    return;
}

void do_newbiechat( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch )
       || ( !NOT_AUTHED( ch ) && !IS_IMMORTAL(ch) 
       && !( ch->pcdata->council && 
          !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
    return;
}

void do_ot( CHAR_DATA *ch, char *argument )
{
  do_ordertalk( ch, argument );
}

void do_ordertalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan 
         || ch->pcdata->clan->clan_type != CLAN_ORDER )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_ORDER, "ordertalk" );
    return;
}

void do_counciltalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_COUNCIL, "counciltalk" );
    return;
}

void do_counciltalk2( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->council2 )
    {            
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_COUNCIL2, "counciltalk2" );
    return;
}   

void do_guildtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan || ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_GUILD, "guildtalk" );
    return;
}

void do_music( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}

void do_gratz( CHAR_DATA *ch, char *argument )
{
	if (NOT_AUTHED(ch))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	talk_channel( ch, argument, CHANNEL_GRATZ, "gratz" );
	return;
}


void do_quest( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_QUEST, "quest" );
    return;
}

void do_ask( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "ask" );
    return;
}



void do_answer( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "answer" );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_SHOUT, "shout" );
  WAIT_STATE( ch, 12 );
  return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_YELL, "yell" );
  return;
}



void do_immtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
    return;
}


void do_staff( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_HIGHGOD, "staff" );
    return;
}


void do_think( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_HIGH, "think" );
    return;
}


void do_avtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_AVTALK, "avtalk" );
    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    EXT_BV actflags;
#ifndef SCRAMBLE
    int speaking = -1, lang;

	MOBtrigger = TRUE;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC(ch) )
	xREMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
	char *sbuf = argument;

	if ( vch == ch )
		continue;
			
	/* Check to see if character is ignoring speaker */
	if (is_ignoring(vch, ch))
	{
		/* continue unless speaker is an immortal */
		if(!IS_IMMORTAL(ch) || get_trust(vch) > get_trust(ch))
			continue;
		else
		{
			set_char_color(AT_IGNORE, vch);
			ch_printf(vch,"You attempt to ignore %s, but"
				" are unable to do so.\n\r", ch->name);
		}
	}
				
#ifndef SCRAMBLE
	if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
	{
	    int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
				      knows_language(ch, ch->speaking, vch));

		if ( speakswell < 75 ) 
		sbuf = translate(speakswell, argument, lang_names[speaking]);
	}
#else
	if ( !knows_language(vch, ch->speaking, ch)
		&&  (!IS_NPC(ch) || ch->speaking != 0) )
		sbuf = scramble(argument, ch->speaking);
#endif
	sbuf = drunk_speech( sbuf, ch );

	MOBtrigger = FALSE;
	act( AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT );
    }
/*    MOBtrigger = FALSE;
    act( AT_SAY, "$n says '$T'", ch, NULL, argument, TO_ROOM );*/
    ch->act = actflags;
    MOBtrigger = FALSE;
    act( AT_SAY, "You say '$T'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR ); 
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument );
	append_to_file( LOG_FILE, buf );
    }
	/* Yakkov's amplify request - shogar */
    if ( ch->in_room && IS_SET( ch->in_room->room_flags, ROOM_AMPLIFY ) )
    {
	ROOM_INDEX_DATA *was_in_room;
	EXIT_DATA       *pexit;

	sprintf(buf,"You hear %s",argument);
	actflags = ch->act;
	xREMOVE_BIT(ch->act, ACT_SECRETIVE);
	was_in_room = ch->in_room;
	for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room
		    &&   pexit->to_room != was_in_room )
		{
			ch->in_room = pexit->to_room;
			MOBtrigger  = FALSE; 
			act( AT_CYAN, buf, ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->act = actflags;
	ch->in_room = was_in_room;
    }
	if (MOBtrigger==TRUE) {
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
	return;
    oprog_speech_trigger( argument, ch ); 
    if ( char_died(ch) )
	return;
    rprog_speech_trigger( argument, ch ); 
    return;
	}
}


void do_whisper( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;
    int speaking = -1, lang;

	MOBtrigger = TRUE;
#ifndef SCRAMBLE

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

    REMOVE_BIT( ch->deaf, CHANNEL_WHISPER );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Whisper to whom what?\n\r", ch );
	return;
    }


    if ( (victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You have a nice little chat with yourself.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched ) 
	&& !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
      send_to_char( "That player is switched.\n\r", ch );
      return;
    }
   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
    }
    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK) )
      {
      send_to_char( "That player is away.\n\r", ch );
      return;
      }
    if ( IS_SET( victim->deaf, CHANNEL_WHISPER ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S whispers turned off.", ch, NULL, victim,
		TO_CHAR );
      return;
    }
    if ( !IS_NPC(victim) &&  xIS_SET(victim->act, PLR_SILENCE) )
	send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   get_trust(ch) < LEVEL_GOD )
    {
	act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    /* Check to see if target of tell is ignoring the sender */
    if(is_ignoring(victim, ch))	
    {
	/* If the sender is an imm then they cannot be ignored */
	if(!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
	{
		set_char_color(AT_IGNORE, ch);
		ch_printf(ch,"%s is ignoring you.\n\r",
			victim->name);
		return;
	}
	else
	{
		set_char_color(AT_IGNORE, victim);
		ch_printf(victim, "You attempt to ignore %s, but ",
			"are unable to do so.\n\r", ch->name);
	}
    }

    /* Bug fix by guppy@wavecomputers.net */
    MOBtrigger = FALSE;
    act( AT_WHISPER, "You whisper to $N '$t'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
#ifndef SCRAMBLE
    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 ) {
	    act( AT_WHISPER, "$n whispers to you '$t'", ch,
		translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
#else
    if ( !knows_language(vch, ch->speaking, ch ) &&
		( !IS_NPC(ch) || ch->speaking != 0) ) {
            act( AT_WHISPER, "$n whispers to you '$t'", ch,
	    	translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
#endif
	MOBtrigger=FALSE;
	} else
	    act( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );
    }
    else
	act( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );

        MOBtrigger = TRUE;
    if ( !IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	act( AT_WHISPER, "$n whispers something to $N.", ch, argument, victim, TO_NOTVICT );

    victim->position	= position;
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s (whisper to) %s.",
		 IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument,
		 IS_NPC( victim ) ? victim->short_descr : victim->name );
	append_to_file( LOG_FILE, buf );
    }
    
	
    mprog_speech_trigger( argument, ch );
    return;
}
    /* beep snippet installed by Scion 3/30/99 */
    void do_beep(CHAR_DATA *ch, char *argument) 
    {
        char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
     
        argument = one_argument(argument, arg);
    
        if (!*arg || !(victim = get_char_world(ch, arg)))
        {
          send_to_char( "Beep who?\n\r", ch );
          return;
        }
    
        /* NPC check added by Samson 2-15-98 */
        if ( IS_NPC( victim ))
        {
            send_to_char( "Beep who?\n\r", ch );
            return;
        }
    
        /* PCFLAG_NOBEEP check added by Samson 2-15-98 */
        if ( IS_SET(victim->pcdata->flags, PCFLAG_NOBEEP) )
        {
            ch_printf( ch, "%s is not accepting beeps at this time.\n\r",
    victim->name );
            return;
        }
    
		if (argument[0]!='\0') {
			ch_printf(victim, "&W^z%s beeps: %s^x\r\n", PERS(ch, victim), argument);
			ch_printf(ch, "&WYou beep %s: %s\r\n", PERS(victim, ch), argument);
		} else {
	        ch_printf(victim, "&W^z%s beeps you from '%s'^x&w\a\n\r", PERS(ch, victim), ch->in_room->name);
		    ch_printf(ch, "&WYou beep %s.&w\n\r", PERS(victim, ch));
		}
        return;
}

void do_tell(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;
    CHAR_DATA *switched_victim = NULL;

#ifndef SCRAMBLE
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

	MOBtrigger=TRUE;

    if (!IS_IMMORTAL(ch))
		REMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if (!IS_NPC(ch)
    && ( xIS_SET(ch->act, PLR_SILENCE)
    ||   xIS_SET(ch->act, PLR_NO_TELL) ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL 
       || ( IS_NPC(victim) && victim->in_room != ch->in_room ) 
       || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You have a nice little chat with yourself.\n\r", ch );
	return;
    }

    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
	send_to_char( "They can't hear you because you are not authorized.\n\r", ch);
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched ) 
	&& ( get_trust( ch ) > LEVEL_AVATAR ) 
	&& !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
      send_to_char( "That player is switched.\n\r", ch );
      return;
    }

   else if ( !IS_NPC( victim ) && ( victim->switched ) 
 	&&  IS_AFFECTED(victim->switched, AFF_POSSESS) ) 
     switched_victim = victim->switched;

   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
    }

    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK) )
    {
	send_to_char( "That player is away and may not have received your tell.\n\r", ch );
    }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) <= get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
		TO_CHAR );
      return;
    }

    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_SILENCE) )
	send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );

    if ( (!IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    || (!IS_NPC(victim)&&IS_SET(victim->in_room->room_flags, ROOM_SILENCE ) ) )
    {
    act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   get_trust(ch) < LEVEL_GOD )
    {
	act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    /* Check to see if target of tell is ignoring the sender */
    if(is_ignoring(victim, ch))	
    {
	/* If the sender is an imm then they cannot be ignored */
	if(!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
	{
		set_char_color(AT_IGNORE, ch);
		ch_printf(ch,"%s is ignoring you.\n\r",
			victim->name);
		return;
	}
	else
	{
		set_char_color(AT_IGNORE, victim);
		ch_printf(victim, "You attempt to ignore %s, but ",
			"are unable to do so.\n\r", ch->name);
	}
    }

    ch->retell = victim;
    
    if(!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history &&
    	isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
    {
	sprintf(buf, "%s told you '%s'\n\r",
		capitalize(IS_NPC(ch) ? ch->short_descr : ch->name),
		argument);
	
	/* get lasttell index... assumes names begin with characters */
	victim->pcdata->lt_index =
		tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]) - 'a';
	
	/* get rid of old messages */
	if(victim->pcdata->tell_history[victim->pcdata->lt_index])
		STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);
	
	/* store the new message */
	victim->pcdata->tell_history[victim->pcdata->lt_index] =
		STRALLOC(buf);
    }   	

    if(switched_victim)
      victim = switched_victim;
     /* Bug fix by guppy@wavecomputers.net */
    MOBtrigger = FALSE;
    act( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 ) {
	    act( AT_TELL, "$n tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
		MOBtrigger=FALSE;
	}
	else
	    act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
    }
    else
	act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );

     MOBtrigger = TRUE;

    /* mobs can talk! -- Scion */
    /* if (!IS_NPC(victim))  */
    /* Mobs should never talk to one another: endless loop! */
    /* do_mobchat(ch,victim,argument); */

    victim->position	= position;
    victim->reply	= ch;
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s (tell to) %s.",
		 IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument,
		 IS_NPC( victim ) ? victim->short_descr : victim->name );
	append_to_file( LOG_FILE, buf );
    }
    
	
    mprog_speech_trigger( argument, ch );
    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int position;
#ifndef SCRAMBLE
    int speaking = -1, lang;

	MOBtrigger=TRUE;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

	if (!IS_IMMORTAL(ch))
	    REMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_SILENCE) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
	&& can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
    {
      send_to_char( "That player is switched.\n\r", ch );
      return;
    }
   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
    }

    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK) )
    {
	send_to_char( "That player is away.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
	TO_CHAR );
      return;
    }

    if ( ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    || ( !IS_NPC(victim) && IS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
    {
    act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   get_trust(ch) < LEVEL_GOD )
    {
	act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    /* Check to see if the receiver is ignoring the sender */
    if(is_ignoring(victim, ch))
    {
    	/* If the sender is an imm they cannot be ignored */
    	if(!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
    	{
    		set_char_color(AT_IGNORE, ch);
    		ch_printf(ch,"%s is ignoring you.\n\r",
    			victim->name);
    		return;
    	}
    	else
    	{
    		set_char_color(AT_IGNORE, victim);
    		ch_printf(victim, "You attempt to ignore %s, but "
    			"are unable to do so.\n\r", ch->name);
    	}
    }
    /* Bug fix by guppy@wavecomputers.net */
    MOBtrigger = FALSE;
    act( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
#ifndef SCRAMBLE
    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 ) {
		MOBtrigger=FALSE;
	    act( AT_TELL, "$n tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
	}else
	    act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
    }
    else
	act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
#else
    if ( knows_language( victim, ch->speaking, ch ) ||
    	 (IS_NPC(ch) && !ch->speaking) )
	    act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	else
		act( AT_TELL, "$n tells you '$t'", ch, scramble(argument, ch->speaking), victim, TO_VICT );
#endif

    MOBtrigger = TRUE;
    mprog_speech_trigger( argument, ch );

    victim->position	= position;
    victim->reply	= ch;
    ch->retell		= victim;
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s (reply to) %s.",
		 IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument,
		 IS_NPC( victim ) ? victim->short_descr : victim->name );
	append_to_file( LOG_FILE, buf );
    }

    if(!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history &&
    	isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
    {
    	sprintf(buf, "%s told you '%s'\n\r",
    		capitalize(IS_NPC(ch) ? ch->short_descr : ch->name),
    		argument);
    	
    	/* get lasttell index... assumes names begin with characters */
    	victim->pcdata->lt_index =
    		tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]) - 'a';
    	
    	/* get rid of old messages */
    	if(victim->pcdata->tell_history[victim->pcdata->lt_index])
    		STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);
    	
    	/* store the new message */
    	victim->pcdata->tell_history[victim->pcdata->lt_index] =
    		STRALLOC(buf);
    }

    return;
}

void do_retell(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	CHAR_DATA *switched_victim = NULL;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	MOBtrigger=TRUE;
	
	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	    if ( ch->speaking & lang_array[lang] )
	    {
		speaking = lang;
		break;
	    }
#endif

		if (!IS_IMMORTAL(ch))
			REMOVE_BIT(ch->deaf, CHANNEL_TELLS);
	if(IS_SET(ch->in_room->room_flags, ROOM_SILENCE))
	{
		send_to_char("You can't do that here.\n\r", ch);
		return;
	}
	
	if ( !IS_NPC(ch) && (xIS_SET(ch->act, PLR_SILENCE)
	||   xIS_SET(ch->act, PLR_NO_TELL)) )
	{
		send_to_char("You can't do that.\n\r", ch);
		return;
	}
	
	if(argument[0] == '\0')
	{
		ch_printf(ch, "What message do you wish to send?\n\r");
		return;
	}
	
	victim = ch->retell;
	
	if(!victim)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}
	
	if(!IS_NPC(victim) && (victim->switched) &&
		(get_trust(ch) > LEVEL_AVATAR) &&
		!IS_AFFECTED(victim->switched, AFF_POSSESS))
	{
		send_to_char("That player is switched.\n\r", ch);
		return;
	}
	else if(!IS_NPC(victim) && (victim->switched) &&
		IS_AFFECTED(victim->switched, AFF_POSSESS))
	{
		switched_victim = victim->switched;
	}
	else if(!IS_NPC(victim) &&(!victim->desc))
	{
		send_to_char("That player is link-dead.\n\r", ch);
		return;
	}
	
	if(!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK) )
	{
		send_to_char("That player is away.\n\r", ch);
		return;
	}
	
	if(IS_SET(victim->deaf, CHANNEL_TELLS) &&
		(!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
	{
		act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	
	if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_SILENCE) )
		send_to_char("That player is silenced. They will receive your message, but can not respond.\n\r", ch);
	
	if((!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) ||
		(!IS_NPC(victim) &&
		IS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
	{
		act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
		return;
	}
	
	if(victim->desc && victim->desc->connected == CON_EDITING &&
		get_trust(ch) < LEVEL_GOD)
	{
		act(AT_PLAIN, "$E is currently in a writing buffer. Please "
			"try again in a few minutes.", ch, 0, victim, TO_CHAR);
		return;
	}
	
	/* check to see if the target is ignoring the sender */
	if(is_ignoring(victim, ch))
	{
		/* if the sender is an imm then they cannot be ignored */
		if(!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
		{
			set_char_color(AT_IGNORE, ch);
			ch_printf(ch, "%s is ignoring you.\n\r",
				victim->name);
			return;
		}
		else
		{
			set_char_color(AT_IGNORE,victim);
			ch_printf(victim, "You attempy to ignore %s, but "
				"are unable to do so.\n\r", ch->name);
		}
	}

	/* store tell history for victim */
	if(!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history &&
		isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
	{
		sprintf(buf, "%s told you '%s'\n\r",
			capitalize(IS_NPC(ch) ? ch->short_descr : ch->name),
			argument);
		
		/* get lasttel index... assumes names begin with chars */
		victim->pcdata->lt_index =
			tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0])
			 - 'a';
		
		/* get rid of old messages */
		if(victim->pcdata->tell_history[victim->pcdata->lt_index])
			STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);
		
		/* store the new messagec */
		victim->pcdata->tell_history[victim->pcdata->lt_index] =
			STRALLOC(buf);
	}
	
	if(switched_victim)
		victim = switched_victim;
	/* Bug fix by guppy@wavecomputers.net */
        MOBtrigger = FALSE;
	act(AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR);
	position = victim->position;
	victim->position = POS_STANDING;
#ifndef SCRAMBLE
	if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
	{
	    int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

		if ( speakswell < 85 ){
			MOBtrigger=FALSE;
		act( AT_TELL, "$n tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
	}else
		act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	}
	else
	    act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
#else
	if(knows_language(victim, ch->speaking, ch) ||
		(IS_NPC(ch) && !ch->speaking))
	{
		act(AT_TELL, "$n tells you '$t'", ch, argument, victim,
			TO_VICT);
	}
	else
	{
		act(AT_TELL, "$n tells you '$t'", ch,
			scramble(argument, ch->speaking), victim, TO_VICT);
	}
#endif
        MOBtrigger = TRUE;
	victim->position = position;
	victim->reply = ch;
	if(IS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
	{
		sprintf(buf, "%s: %s (retell to) %s.",
			IS_NPC(ch) ? ch->short_descr : ch->name,
			argument,
			IS_NPC(victim) ? victim->short_descr : victim->name);
		append_to_file(LOG_FILE, buf);
	}

	mprog_speech_trigger(argument, ch);
	return;
}

void do_repeat(CHAR_DATA *ch, char *argument)
{
	int index;
	
	if(IS_NPC(ch) || !IS_IMMORTAL(ch) || !ch->pcdata->tell_history)
	{
		ch_printf(ch, "Huh?\n\r");
		return;
	}
	
	if(argument[0] == '\0')
	{
		index = ch->pcdata->lt_index;
	}
	else if(isalpha(argument[0]) && argument[1] == '\0')
	{
		index = tolower(argument[0]) - 'a';
	}
	else
	{
		ch_printf(ch, "You may only index your tell history using "
			"a single letter.\n\r");
		return;
	}
	
	if(ch->pcdata->tell_history[index])
	{
		set_char_color(AT_TELL, ch);
		ch_printf(ch, ch->pcdata->tell_history[index]);
	}
	else
	{
		ch_printf(ch, "No one like that has sent you a tell.\n\r");
	}
	
	return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    CHAR_DATA *vch;
    EXT_BV actflags;
#ifndef SCRAMBLE
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

    if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC(ch) )
	xREMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( plast = argument; *plast != '\0'; plast++ )
	;

    strcpy( buf, argument );
    if ( isalpha(plast[-1]) )
	strcat( buf, "." );
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = buf;
		
		/* Check to see if character is ignoring emoter */
		if(is_ignoring(vch, ch))
		{
			/* continue unless emoter is an immortal */
			if(!IS_IMMORTAL(ch) || get_trust(vch) > get_trust(ch))
				continue;
			else
			{
				set_char_color(AT_IGNORE, vch);
				ch_printf(vch,"You attempt to ignore %s, but"
					" are unable to do so.\n\r", ch->name);
			}
		}
#ifndef SCRAMBLE
		if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
		{
		    int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
					  knows_language(ch, ch->speaking, vch));

		    if ( speakswell < 85 )
			sbuf = translate(speakswell, argument, lang_names[speaking]);
		}
#else
		if ( !knows_language( vch, ch->speaking, ch ) &&
			 (!IS_NPC(ch) && ch->speaking != 0) )
			sbuf = scramble(buf, ch->speaking);
#endif
		MOBtrigger = FALSE;
		act( AT_SOCIAL, "$n $t", ch, sbuf, vch, (vch == ch ? TO_CHAR : TO_VICT) );
	}
 /*  MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );
*/
    ch->act = actflags;
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s %s (emote)", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument );
	append_to_file( LOG_FILE, buf );
    }
    return;
}


void do_bug( CHAR_DATA *ch, char *argument )
{
    char    buf[MAX_STRING_LENGTH];
    struct  tm *t = localtime(&current_time);

    set_char_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' ) {
        send_to_char( "\n\rUsage:  'bug <message>'  (your location is automatically recorded)\n\r", ch );
        return;
    }
    sprintf( buf, "(%-2.2d/%-2.2d):  %s",
	t->tm_mon+1, t->tm_mday, argument );
    append_file( ch, PBUG_FILE, buf );
    send_to_char( "Thanks, your bug notice has been recorded.\n\r", ch );
    return;
}

void do_ide( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_PLAIN, ch );
    send_to_char("\n\rIf you want to send an idea, type 'idea <message>'.\n\r", ch);
    send_to_char("If you want to identify an object, use the identify spell.\n\r", ch);
    return;
}

void do_idea( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' ) {
        send_to_char( "\n\rUsage:  'idea <message>'\n\r", ch );
        return;
    }
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Thanks, your idea has been recorded.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' ) {
        send_to_char( "\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r", ch );
	if ( get_trust( ch ) >= LEVEL_ASCENDANT )
	  send_to_char( "Usage:  'typo list' or 'typo clear now'\n\r", ch );
        return;
    }
    if ( !str_cmp( argument, "clear now" )
    &&    get_trust( ch ) >= LEVEL_ASCENDANT ) {
        FILE *fp = fopen( TYPO_FILE, "w" );
        if ( fp )
          fclose( fp );
        send_to_char( "Typo file cleared.\n\r", ch);
        return;
    }
    if ( !str_cmp( argument, "list" ) ) {
	send_to_char( "\n\r VNUM \n\r.......\n\r", ch );
        show_file( ch, TYPO_FILE );
     } else {
	append_file( ch, TYPO_FILE, argument );
	send_to_char( "Thanks, your typo notice has been recorded.\n\r", ch );
    }
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_WHITE, ch );
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    /*OBJ_DATA *obj;   unused */
    CHAR_DATA *vch;
    int x, y;
    int level;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING
       || ch->position ==  POS_EVASIVE
       || ch->position ==  POS_DEFENSIVE
       || ch->position ==  POS_AGGRESSIVE
       || ch->position ==  POS_BERSERK
    )
    { 
	set_char_color( AT_RED, ch );
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	set_char_color( AT_BLOOD, ch );
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( get_timer(ch, TIMER_RECENTFIGHT) > 0
    &&  !IS_IMMORTAL(ch) )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "Your adrenaline is pumping too hard to quit now!\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
    {
	send_to_char("Wait until you have bought/sold the item on auction.\n\r", ch);
	return;

    }

    if ( IS_PKILL( ch ) && ch->wimpy > (int) ch->max_hit / 2.25 )
    {
        send_to_char( "Your wimpy has been adjusted to the maximum level for deadlies.\n\r", ch );
        do_wimpy( ch, "max" );
    }

	if (in_arena( ch )) {  /* Scion arenacode */
		send_to_char( "Get out of the arena first!\r\n",ch);
		return;
	}

	/* Make them lose artifact items. Can't leave the game with those! -- Scion */
	drop_artifacts(ch, ch->last_carrying);
    
	/* Get 'em dismounted until we finish mount saving -- Blodkai, 4/97 */
    if ( ch->position == POS_MOUNTED )
	do_dismount( ch, "" );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You are wracked with uncontrollable pain as you are extracted from the matrix.\n\r\n\r", ch );
    act( AT_SAY, "A strange voice says, 'We await your return, $n...'", ch, NULL, NULL, TO_CHAR );
    act( AT_BYE, "$n has escaped the realm.", ch, NULL, NULL, TO_CANSEE );
    set_char_color( AT_GREY, ch);

    sprintf( log_buf, "%s has quit (Room %d).", ch->name, 
	( ch->in_room ? ch->in_room->vnum : -1 ) );
    quitting_char = ch;
    save_char_obj( ch );

    if ( sysdata.save_pets && ch->pcdata->pet )
    {
       act( AT_BYE, "$N follows $S master into the Void.", ch, NULL, 
		ch->pcdata->pet, TO_ROOM );
       extract_char( ch->pcdata->pet, TRUE );
    }

    /* Synch clandata up only when clan member quits now. --Shaddai
     */
    if ( ch->pcdata->clan )
        save_clan( ch->pcdata->clan );     

    if ( ch->level >= 1 )
	for ( vch = first_char; vch; vch = vch->next )
	{
	    if ( IS_NPC( vch ) || vch == ch )
		continue;
	    act( AT_GREEN, "&R[&WINFO&R]&c $n has escaped the realm!", ch, NULL, vch, TO_VICT );
/*	Don't really need this.. just annoys me :)   -- Scion */
  }

    saving_char = NULL;

    level = get_trust(ch);
    /*
     * After extract_char the ch is no longer valid!
     */
    extract_char( ch, TRUE );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;

    /* don't show who's logging off to leaving player */
/*
    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", level ); 
*/
    log_string_plus( log_buf, LOG_COMM, level );
    return;
}

/* Made this a function so it could work recursively on containers.
   Make them drop artifact items. Can't leave the game with those! -- Scion */
void drop_artifacts(CHAR_DATA *ch, OBJ_DATA *obj) {
	/* Expecting a ch->last_carrying or an obj->last_content
	( We go BACKWARDS through the lists!) */
	
	if (!obj)
		return;
	
	

	while (obj) {
		if (obj->last_content)
			drop_artifacts(ch, obj->last_content);
		
		if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)) {
			OBJ_DATA *tobj;

			tobj=obj;
			obj=obj->prev_content;

			if (tobj->in_obj)
				obj_from_obj(tobj);
			if (tobj->carried_by)
				obj_from_char(tobj);

			obj_to_room(tobj, ch->in_room);
			act(AT_MAGIC, "$p falls to the ground, drawn by the magic of these lands!", ch, tobj, NULL, TO_CHAR);
			act(AT_MAGIC, "$p falls from $n's hands, drawn by the magic of these lands!", ch, tobj, NULL, TO_CANSEE);
		} else
			obj = obj->prev_content;
	}
}

void send_rip_screen( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPSCREEN_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void send_rip_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPTITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void send_ansi_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(ANSITITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void send_ascii_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH];

    if ((rpfile = fopen(ASCTITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void do_omenu( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    /* looks like the menus keep crashing, lets log them - shogar */
    sprintf(arg1,"%s: omenu %s",ch->name,argument);
    log_string(arg1);
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
	 
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: omenu <object> [page]  \n\r",     ch );
        send_to_char( "      Where:    <object> is a prototype object  \n\r",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\n\r",     ch );
        return;
    }
	 
    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* can redit or something */

    ch->inter_type = OBJ_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(obj->pIndexData->name);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  obj->pIndexData->vnum;
    send_obj_page_to_char(ch, obj->pIndexData, arg2[0]);   
}


void do_rmenu( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *idx;
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    /* looks like the menus keep crashing, lets log them - shogar */
    sprintf(arg1,"%s: rmenu %s",ch->name,argument);
    log_string(arg1);
    argument = one_argument( argument, arg1 );
	 
    idx = ch->in_room;
    /* can redit or something */

    ch->inter_type = ROOM_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(idx->name);  
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  idx->vnum;
    send_room_page_to_char(ch, idx, arg1[0]);   
}

void do_cmenu( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    /* looks like the menus keep crashing, lets log them - shogar */
    sprintf(arg1,"%s: cmenu %s",ch->name,argument);
    log_string(arg1);
    argument = one_argument( argument, arg1 );
	 
    ch->inter_type = CONTROL_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup("Control Panel");  
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    send_control_page_to_char(ch, arg1[0]);   
}


void do_mmenu( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    /* looks like the menus keep crashing, lets log them - shogar */
    sprintf(arg1,"%s: mmenu %s",ch->name,argument);
    log_string(arg1);
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
	 
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: mmenu <victim> [page]  \n\r",     ch );
        send_to_char( "      Where:    <victim> is a prototype mob  \n\r",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\n\r",     ch );
        return;
    }
      

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on players.\n\r", ch );
	return;
    }

    if ( get_trust( ch ) < victim->level )
    {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "Their godly glow prevents you from getting a good look .\n\r", ch );
	return;
    }
    ch->inter_type = MOB_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(arg1);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  victim->pIndexData->vnum;
    send_page_to_char(ch, victim->pIndexData, arg2[0]);   
}


void do_rip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rip ON or OFF?\n\r", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	send_rip_screen(ch);
	xSET_BIT(ch->act,PLR_RIP);
	xSET_BIT(ch->act,PLR_ANSI);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	xREMOVE_BIT(ch->act,PLR_RIP);
	send_to_char( "!|*\n\rRIP now off...\n\r", ch );
	return;
    }
}

void do_ansi( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "ANSI ON or OFF?\n\r", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	xSET_BIT(ch->act,PLR_ANSI);
	set_char_color( AT_WHITE + AT_BLINK, ch);
	send_to_char( "Ansi On, Way Cool!!!\n\r", ch);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	xREMOVE_BIT(ch->act,PLR_ANSI);
	send_to_char( "Okay... ANSI support is now off.\n\r", ch );
	return;
    }
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;
    if ( ch->level < 2 ) {
	send_to_char_color( "&BYou must be at least second level to save.\n\r", ch );
	return;
    }
    WAIT_STATE( ch, 2 ); /* For big muds with save-happy players, like RoD */
    update_aris(ch);     /* update char affects and RIS */
    save_char_obj( ch );
    saving_char = NULL;
    send_to_char( "Saved...\n\r", ch );
    return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *tmp;

    for ( tmp = victim; tmp; tmp = tmp->master )
	if ( tmp == ch )
	  return TRUE;
    return FALSE;
}


void do_dismiss( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );     

    if ( arg[0] == '\0' )
    {
	send_to_char( "Dismiss whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ( IS_AFFECTED( victim, AFF_CHARM ) )
    && ( IS_NPC( victim ) )
    && ( victim->master == ch ) )
    {
	stop_follower( victim );
        stop_hating( victim );
        stop_hunting( victim );
        stop_fearing( victim );
        act( AT_ACTION, "$n dismisses $N.", ch, NULL, victim, TO_NOTVICT );
 	act( AT_ACTION, "You dismiss $N.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
	send_to_char( "You cannot dismiss them.\n\r", ch );
    }

return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master )
    {
	act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( !ch->master )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }
/*
    if ( ( ch->level - victim->level < -10 || ch->level - victim->level >  10 )
    &&   !IS_HERO(ch) && !(ch->level < 15 && !IS_NPC(victim) 
    && victim->pcdata->council 
    && !str_cmp(victim->pcdata->council->name,"Newbie Council")))
    {
	send_to_char( "You are not of the right caliber to follow.\n\r", ch );
	return;
    }
-- Screw this, level restrictions on following are stupid. -- Scion */
    if ( circle_follow( ch, victim ) )
    {
		stop_follower( victim );
		return;
    }

    if ( ch->master )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    /* Support for saving pets --Shaddai */
    if ( IS_NPC(ch) && xIS_SET(ch->act, ACT_PET) && !IS_NPC(master) )
	master->pcdata->pet = ch;

    if ( can_see( master, ch ) )
    act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

    act( AT_ACTION, "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( !ch->master )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_NPC(ch) && !IS_NPC(ch->master) && ch->master->pcdata->pet == ch )
	ch->master->pcdata->pet = NULL;

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) )
	if (!(!IS_NPC(ch->master) && IS_IMMORTAL(ch) && !IS_IMMORTAL(ch->master)))
	    act( AT_ACTION, "$n stops following you.",     ch, NULL, ch->master, TO_VICT  );
    act( AT_ACTION, "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }
    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    strcpy( argbuf, argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->first_person; och; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM) && och->master == ch 
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
        sprintf( log_buf, "%s: order %s.", ch->name, argbuf );
        log_string_plus( log_buf, LOG_NORMAL, ch->level );
 	send_to_char( "Ok.\n\r", ch );
        WAIT_STATE( ch, 12 );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

/* Overhauled 2/97 -- Blodkai */
void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = ch->leader ? ch->leader : ch;
        set_char_color( AT_DGREEN, ch );
        ch_printf( ch, "\n\rFollowing %-12.12s     [hitpnts]   [ magic ] [mst] [mvs] [race]%s\n\r",
		PERS(leader, ch),
		ch->level < LEVEL_AVATAR ? " [to lvl]" : "" );
	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		set_char_color( AT_DGREEN, ch );
		if (IS_AFFECTED(gch, AFF_POSSESS) || IS_SET( ch->pcdata->flags, PCFLAG_ANONYMOUS)) /*reveal no information*/
		  ch_printf( ch,
			"[%2d %s] %-16s %4s/%4s hp %4s/%4s %s %4s/%4s mv %5s xp\n\r",
		    	gch->level,
		    	IS_NPC(gch) ? "Mob" : class_table[gch->class]->who_name,
		    	capitalize( PERS(gch, ch) ),
		    	"????",   
		    	"????",
		    	"????",
		    	"????",
		    	IS_VAMPIRE(gch) ? "bp" : "mana",
		    	"????",  
		    	"????",  
		    	"?????"    );
		else
              	    if      ( gch->alignment >  750 ) sprintf(buf, " A");
                    else if ( gch->alignment >  350 ) sprintf(buf, "-A");
                    else if ( gch->alignment >  150 ) sprintf(buf, "+N");
                    else if ( gch->alignment > -150 ) sprintf(buf, " N");
                    else if ( gch->alignment > -350 ) sprintf(buf, "-N");
                    else if ( gch->alignment > -750 ) sprintf(buf, "+S");
                    else                              sprintf(buf, " S");
		    set_char_color( AT_DGREEN, ch );
		    send_to_char( "[", ch );
		    set_char_color( AT_GREEN, ch );
		    ch_printf( ch, "%-2d %2.2s %3.3s",
			gch->level,
			buf,
			IS_NPC(gch) ? "Mob" : class_table[gch->class]->who_name );
		    set_char_color( AT_DGREEN, ch );
		    send_to_char( "]  ", ch );
		    set_char_color( AT_GREEN, ch );
                    ch_printf( ch, "%-12.12s ",
                        capitalize( PERS(gch, ch) ) );
                    if ( gch->hit < gch->max_hit/4 )
                      set_char_color( AT_DANGER, ch );
                    else if ( gch->hit < gch->max_hit/2.5 )
                      set_char_color( AT_YELLOW, ch );
                    else set_char_color( AT_GREY, ch );
                    ch_printf( ch, "%5d", gch->hit );
                    set_char_color( AT_GREY, ch );
                    ch_printf( ch, "/%-5d ", gch->max_hit );
                    if ( IS_VAMPIRE(gch) )
                      set_char_color( AT_BLOOD, ch );
                    else
                      set_char_color( AT_LBLUE, ch );
		    if ( gch->class != CLASS_WARRIOR )
                      ch_printf( ch, "%5d/%-5d ",
			IS_VAMPIRE(gch) ? gch->pcdata->condition[COND_BLOODTHIRST] : gch->mana,
                        IS_VAMPIRE(gch) ? 10 + gch->level : gch->max_mana );
		    else
		      send_to_char( "            ", ch );
		    if ( gch->mental_state < -25 || gch->mental_state > 25 )
		      set_char_color( AT_YELLOW, ch );
		    else
		      set_char_color( AT_GREEN, ch );
                    ch_printf( ch, "%3.3s  ",
                        gch->mental_state > 75  ? "+++" :
                        gch->mental_state > 50  ? "=++" :
                        gch->mental_state > 25  ? "==+" :
                        gch->mental_state > -25 ? "===" :
                        gch->mental_state > -50 ? "-==" :
                        gch->mental_state > -75 ? "--=" :
                                                  "---" );
		    set_char_color( AT_DGREEN, ch );
                    ch_printf( ch, "%5d ",
                        gch->move );
		    ch_printf( ch, "%6s ", npc_race[gch->race]);
		    set_char_color( AT_GREEN, ch );
		    if ( gch->level < LEVEL_AVATAR )
		      ch_printf( ch, "%8d ",
			exp_level( gch, gch->level+1) - gch->exp );
		    send_to_char( "\n\r", ch);
	    }
	}
	return;
    }

    if ( !strcmp( arg, "disband" ))
    {
	CHAR_DATA *gch;
	int count = 0;

	if ( ch->leader || ch->master )
	{
	    send_to_char( "You cannot disband a group if you're following someone.\n\r", ch );
	    return;
	}
	
	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( ch, gch )
	    && ( ch != gch ) )
	    {
		gch->leader = NULL;
		gch->master = NULL;
		count++;
		send_to_char( "Your group is disbanded.\n\r", gch );
	    }
	}

	if ( count == 0 )
	   send_to_char( "You have no group members to disband.\n\r", ch );
	else
	   send_to_char( "You disband your group.\n\r", ch );
	
	return;
    }

    if ( !strcmp( arg, "all" ) )
    {
	CHAR_DATA *rch;
	int count = 0;

        for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
	{
           if ( ch != rch
           &&   !IS_NPC( rch )
	   &&   can_see( ch, rch )
	   &&   rch->master == ch
	   &&   !ch->master
	   &&   !ch->leader
/*    	   &&   abs( ch->level - rch->level ) < 8  Level restrict on groups is stupid -- Scion */
    	   &&   !is_same_group( rch, ch )
   	   &&   IS_PKILL( ch ) == IS_PKILL( rch )
	      )
	   {
		rch->leader = ch;
		count++;
	   }
	}
	
	if ( count == 0 )
	  send_to_char( "You have no eligible group members.\n\r", ch );
	else
	{
     	   act( AT_ACTION, "$n groups $s followers.", ch, NULL, NULL, TO_ROOM );
	   send_to_char( "You group your followers.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
        act( AT_PLAIN, "You can't group yourself.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( AT_ACTION, "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    if ( ch->level - victim->level < -8
    ||   ch->level - victim->level >  8 
    ||   ( IS_PKILL( ch ) != IS_PKILL( victim ) ) )
    {
	act( AT_PLAIN, "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( AT_PLAIN, "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( AT_PLAIN, "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
    act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }

    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    
    if ( xIS_SET(ch->act, PLR_AUTOGOLD) && members < 2 )
	return;

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    set_char_color( AT_GOLD, ch );
    ch_printf( ch,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }
    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
#ifndef SCRAMBLE
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}
#endif

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( xIS_SET(ch->act, PLR_NO_TELL) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
/*    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );*/
    for ( gch = first_char; gch; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	{
	    set_char_color( AT_GTELL, gch );
	    /* Groups unscrambled regardless of clan language.  Other languages
		   still garble though. -- Altrag */
#ifndef SCRAMBLE
	    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
	    {
		int speakswell = UMIN(knows_language(gch, ch->speaking, ch),
				      knows_language(ch, ch->speaking, gch));

		if ( speakswell < 85 )
		    ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, translate(speakswell, argument, lang_names[speaking]) );
		else
		    ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
	    }
	    else
		ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
#else
	    if ( knows_language( gch, ch->speaking, gch )
	    ||  (IS_NPC(ch) && !ch->speaking) )
		ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
	    else
		ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, scramble(argument, ch->speaking) );
#endif
	}
    }

    return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"Auction: %s", argument); /* last %s to reset color */

    for (d = first_descriptor; d; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf,CHANNEL_AUCTION) 
        && !IS_SET(original->in_room->room_flags, ROOM_SILENCE) && !NOT_AUTHED(original))
            act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
    }
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 *
 * Modified to return how well the language is known 04/04/98 - Thoric
 * Currently returns 100% for known languages... but should really return
 * a number based on player's wisdom (maybe 50+((25-wisdom)*2) ?)
 */
int knows_language( CHAR_DATA *ch, int language, CHAR_DATA *cch )
{
	sh_int sn;

	if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
		return 100;
	if ( IS_NPC(ch) && !ch->speaks ) /* No langs = knows nothing for npcs */
		return 0;
	if ( IS_NPC(ch) && IS_SET(ch->speaks, (language & ~LANG_CLAN)) )
		return (50+(25 - ch->mod_int)*2);
	/* everyone KNOWS common tongue
	if ( IS_SET(language, LANG_COMMON) )
		return (50+(25 - ch->int)*2);

	if ( language & LANG_CLAN )
	{
	 Clan = common for mobs.. snicker.. -- Altrag
		if ( IS_NPC(ch) || IS_NPC(cch) )
			return (50+(25 - ch->int)*2);
		if ( ch->pcdata->clan == cch->pcdata->clan &&
			 ch->pcdata->clan != NULL )
			return (50+(25 - ch->int)*2);
	} */
	if ( !IS_NPC( ch ) )
	{
	    int lang;
	    
		/* Racial languages for PCs */
	    if ( IS_SET(race_table[ch->race]->language, language) )
			return ((65+(25 - ch->mod_int)*2)<=100 ? (65+(25 - ch->mod_int)*2) : 100);

	    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	      if ( IS_SET(language, lang_array[lang]) &&
	      	   IS_SET(ch->speaks, lang_array[lang]) )
	      {
		  if ( (sn = skill_lookup(lang_names[lang])) != -1 )
		    return ch->pcdata->learned[sn];
	      }
	}
	return 0;
}

bool can_learn_lang( CHAR_DATA *ch, int language )
{
	if ( language & LANG_CLAN )
		return FALSE;
	if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
		return FALSE;
	if ( race_table[ch->race]->language & language )
		return FALSE;
	if ( ch->speaks & language )
	{
		int lang;
		
		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
			if ( language & lang_array[lang] )
			{
				int sn;
				
				if ( !(VALID_LANGS & lang_array[lang]) )
					return FALSE;
				if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
				{
					bug( "Can_learn_lang: valid language without sn: %d", lang );
					continue;
				}
				if ( ch->pcdata->learned[sn] >= 99 )
					return FALSE;
			}
	}
	if ( VALID_LANGS & language )
		return TRUE;
	return FALSE;
}

int const lang_array[] = {
 LANG_COMMON, LANG_ELVEN, LANG_DWARVEN, LANG_PIXIE,
 LANG_OGRE, LANG_ORCISH, LANG_TROLLISH, LANG_RODENT,
 LANG_INSECTOID, LANG_MAMMAL, LANG_REPTILE,
 LANG_DRAGON, LANG_SPIRITUAL, LANG_MAGICAL,
 LANG_GOBLIN, LANG_GOD, LANG_ANCIENT, LANG_HALFLING,
 LANG_CLAN, LANG_GITH, LANG_GNOME, LANG_UNKNOWN };

char * const lang_names[] = {
 "common", "elvish", "dwarven", "pixie", "ogre",
 "orcish", "trollese", "rodent", "insectoid",
 "mammal", "reptile", "dragon", "spiritual",
 "magical", "goblin", "god", "ancient",
 "halfling", "clan", "gith", "gnome", "" };


/* Note: does not count racial language.  This is intentional (for now). */
int countlangs( int languages )
{
	int numlangs = 0;
	int looper;

	for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
	{
		if ( lang_array[looper] == LANG_CLAN )
			continue;
		if ( languages & lang_array[looper] )
			numlangs++;
	}
	return numlangs;
}

void do_speak( CHAR_DATA *ch, char *argument )
{
	int langs;
	char arg[MAX_INPUT_LENGTH];
	
	argument = one_argument(argument, arg );
	
	if ( !str_cmp( arg, "all" ) && IS_IMMORTAL( ch ) )
	{
		set_char_color( AT_SAY, ch );
		ch->speaking = ~LANG_CLAN;
		send_to_char( "Now speaking all languages.\n\r", ch );
		return;
	}
	for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
		if ( !str_prefix( arg, lang_names[langs] ) )
			if ( knows_language( ch, lang_array[langs], ch ) )
			{
				if ( lang_array[langs] == LANG_CLAN &&
					(IS_NPC(ch) || !ch->pcdata->clan) )
					continue;
				ch->speaking = lang_array[langs];
				set_char_color( AT_SAY, ch );
				ch_printf( ch, "You now speak %s.\n\r", lang_names[langs] );
				return;
			}
	set_char_color( AT_SAY, ch );
	send_to_char( "You do not know that language.\n\r", ch );
}

void do_languages( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int lang;
	
	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' && !str_prefix( arg, "learn" ) &&
		!IS_IMMORTAL(ch) && !IS_NPC(ch) )
	{
		CHAR_DATA *sch;
		char arg2[MAX_INPUT_LENGTH];
		int sn;
		int prct;
		int prac;
		
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Learn which language?\n\r", ch );
			return;
		}
		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		{
			if ( lang_array[lang] == LANG_CLAN )
				continue;
			if ( !str_prefix( arg2, lang_names[lang] ) )
				break;
		}
		if ( lang_array[lang] == LANG_UNKNOWN )
		{
			send_to_char( "That is not a language.\n\r", ch );
			return;
		}
		if ( !(VALID_LANGS & lang_array[lang]) )
		{
			send_to_char( "You may not learn that language.\n\r", ch );
			return;
		}
		if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
		{
			send_to_char( "That is not a language.\n\r", ch );
			return;
		}
		if ( race_table[ch->race]->language & lang_array[lang] ||
			 lang_array[lang] == LANG_COMMON ||
			 ch->pcdata->learned[sn] >= 99 )
		{
			act( AT_PLAIN, "You are already fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
			return;
		}
		for ( sch = ch->in_room->first_person; sch; sch = sch->next_in_room )
			if ( IS_NPC(sch) && xIS_SET(sch->act, ACT_SCHOLAR)
			&&   knows_language( sch, ch->speaking, ch )
			&&   knows_language( sch, lang_array[lang], sch )
			&& (!sch->speaking || knows_language( ch, sch->speaking, sch )) )
				break;
		if ( !sch )
		{
			send_to_char( "There is no one who can teach that language here.\n\r", ch );
			return;
		}
		if ( countlangs(ch->speaks) >= (ch->level / 10) &&
			 ch->pcdata->learned[sn] <= 0 )
		{
			act( AT_TELL, "$n tells you 'You may not learn a new language yet.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		/* 0..16 cha = 2 pracs, 17..25 = 1 prac. -- Altrag */
		prac = 2 - (get_curr_cha(ch) / 17);
		if ( ch->practice < prac )
		{
			act( AT_TELL, "$n tells you 'You do not have enough practices.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		ch->practice -= prac;
		/* Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag */
		prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);
		ch->pcdata->learned[sn] += prct;
		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 99);
		SET_BIT( ch->speaks, lang_array[lang] );
		if ( ch->pcdata->learned[sn] == prct )
			act( AT_PLAIN, "You begin lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 )
			act( AT_PLAIN, "You continue lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 + prct )
			act( AT_PLAIN, "You feel you can start communicating in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 99 )
			act( AT_PLAIN, "You become more fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else
			act( AT_PLAIN, "You now speak perfect $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		return;
	}
	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( knows_language( ch, lang_array[lang], ch ) )
		{
			if ( ch->speaking & lang_array[lang] ||
				(IS_NPC(ch) && !ch->speaking) )
				set_char_color( AT_SAY, ch );
			else
				set_char_color( AT_PLAIN, ch );
			send_to_char( lang_names[lang], ch );
			send_to_char( "\n\r", ch );
		}
	send_to_char( "\n\r", ch );
	return;
}

void do_wartalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_WARTALK, "war" );
    return;
}

void do_racetalk( CHAR_DATA *ch, char *argument )
{
  if (NOT_AUTHED(ch))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  talk_channel( ch, argument, CHANNEL_RACETALK, "racetalk" );
  return;
}


void
init_profanity_checker()
{
/* #ifndef WIN32
*  bigregex = (char *) malloc (4096);
*  add_profane_word("fuck");
*  skip over first 2 slashes and bar
*  bigregex+=3;
*  
*  add_profane_word("shit");
*  add_profane_word("cunt");
*  Removed by Fellon
*  preg = re_comp( bigregex );
* #endif */
}

void add_profane_word( char * word)
{
#ifndef WIN32
   char _word[4096];
   int i,j;
   
   j=0;
   _word[j] = '\\'; j++;
   _word[j] = '|'; j++;
   _word[j] = '\\'; j++;
   _word[j] = '('; j++;
   for(i=0;i<strlen(word);i++){
      _word[j]='['; j++;
      _word[j]=tolower(word[i]); j++;
      _word[j]=toupper(word[i]); j++;
      _word[j]=']'; j++;
      _word[j]='+'; j++;
      _word[j]='['; j++;
   
   
   
      _word[j]='-'; j++;
      _word[j]=' '; j++;
      _word[j]='\t'; j++;
   
      _word[j]='`'; j++;
      _word[j]='~'; j++;
      _word[j]='1'; j++;
      _word[j]='!'; j++;
      _word[j]='2'; j++;
      _word[j]='@'; j++; /* Was @@ -- Scion */
      _word[j]='3'; j++;
      _word[j]='#'; j++;
      _word[j]='4'; j++;
      _word[j]='5'; j++;
      _word[j]='%'; j++;
      _word[j]='6'; j++;
      _word[j]='7'; j++;
      _word[j]='&'; j++;
      _word[j]='8'; j++;
      _word[j]='9'; j++;
      _word[j]='0'; j++;
      _word[j]='_'; j++;
      _word[j]=';'; j++;
      _word[j]=':'; j++;
      _word[j]=','; j++;
      _word[j]='<'; j++;
      /* These need to be escaped  for C */
   
   
      _word[j]='\''; j++;
      _word[j]='\\'; j++;
      _word[j]='\"'; j++;
   
      /* These need to be escaped  for regex*/
      _word[j]='\\'; j++;
      _word[j]='$'; j++;
   
      _word[j]='>'; j++;
      _word[j]='/'; j++;
      _word[j]='\\'; j++;
      _word[j]='^'; j++;
      _word[j]='\\'; j++;
      _word[j]='.'; j++;
      _word[j]='\\'; j++;
      _word[j]=')'; j++;
      _word[j]='\\'; j++;
      _word[j]='?'; j++;
      _word[j]='\\'; j++;
      _word[j]='*'; j++;
   
      _word[j]='\\'; j++;
      _word[j]='('; j++;
      _word[j]='\\'; j++;
      _word[j]='['; j++;
   
      _word[j]='\\'; j++;
      _word[j]='{'; j++;
      _word[j]='\\'; j++;
      _word[j]='+'; j++;
   
#ifdef BIG
   /* i don't get what the deal is with this guy, it seems unescapable,
      so to speak. */
      _word[j]='\\'; j++;
      _word[j]=']'; j++;
#endif
      _word[j]='\\'; j++;
      _word[j]='}'; j++;
      _word[j]='\\'; j++;
      _word[j]='|'; j++;
      _word[j]='\\'; j++;
      _word[j]='='; j++;
   
      /* close up funny characters */
      _word[j]=']'; j++;
      _word[j]='*'; j++;
   }
   _word[j] = '\\'; j++;
   _word[j] = ')'; j++;
   _word[j]='\0';
   
   strcat(bigregex, _word);
#endif
}

int is_profane (char *what)
{
/* #ifndef WIN32
*  int ret;
* Removed by Fellon
*  ret = re_exec(what);
*  if (ret==1)
*     return(1);
* #endif */
  return(0); 
}

