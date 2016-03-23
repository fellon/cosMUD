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
 *			   Smaug banking support module                           *
 ****************************************************************************/
/***************************************************************************  
 *                          SMAUG Banking Support Code                     *
 ***************************************************************************
 *                                                                         *
 * This code may be used freely, as long as credit is given in the help    *
 * file. Thanks.                                                           *
 *								                           *
 *                                        -= Minas Ravenblood =-           *
 *                                 Implementor of The Apocalypse Theatre   *
 *                                      (email: krisco7@hotmail.com)       *
 *									                     *
 ***************************************************************************/

/* Modifications to original source by Samson */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* You can add this or just put it in the do_bank code. I don't really know
   why I made a seperate function for this, but I did. If you do add it,
   don't forget to declare it - Minas */
/* Finds banker mobs in a room. Installed by Samson on unknown date */
/* NOTE: Smaug 1.02a Users - Your compiler probably died on this
   function - if so, remove the x in front of IS_SET and recompile */
CHAR_DATA *find_banker( CHAR_DATA *ch )
{
  CHAR_DATA *banker;
  
  for ( banker = ch->in_room->first_person; banker; banker = banker->next_in_room )
    if ( IS_NPC( banker ) && xIS_SET( banker->act, ACT_BANKER ) )
      break;

  return banker;
}

/* SMAUG Bank Support
 * Coded by Minas Ravenblood for The Apocalypse Theatre
 * (email: krisco7@hotmail.com)
 */
/* Installed by Samson on unknown date */
/* Deposit, withdraw, balance and transfer commands */
void do_deposit( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char arg1[MAX_INPUT_LENGTH];
  char buf [MAX_STRING_LENGTH];
  int amount;
  
  if ( !( banker = find_banker( ch ) ) )
  {
    send_to_char( "&WYou're not in a bank!&w\n\r", ch );
    return;
  }
  
  if ( IS_NPC( ch ) )
  {
    sprintf( buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr );
    do_say( banker, buf );
    return;
  }
  
  if ( argument[0] == '\0' )
  {
    do_say( banker, "If you need help, see HELP BANK." );
    return;
  }
  
  argument = one_argument( argument, arg1 );
          
  if ( arg1 == '\0' )
  {
    sprintf( buf, "%s How much gold do you wish to deposit?", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( str_cmp( arg1, "all" ) && !is_number( arg1 ) )
  {
    sprintf( buf, "%s How much gold do you wish to deposit?", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( !str_cmp( arg1, "all" ) )
    amount = ch->gold;
  else
    amount = atoi( arg1 );
  
  if ( amount > ch->gold )
  {
      sprintf( buf, "%s Sorry, but you don't have that much gold to deposit.", ch->name );
      do_tell( banker, buf );
      return;
  }
    
  if ( amount <= 0 )
  {
      sprintf( buf, "%s Oh, I see.. I didn't know i was doing business with a comedian.", ch->name );
      do_tell( banker, buf );
      return;
  }

  if( !IS_IMMORTAL( ch ) )
  {
      if( ch->pcdata->balance == 5000000 )
      {
          do_tell( banker, "You can't deposit more than 5,000,000 coins!" );
          return;
      }

      if( ch->pcdata->balance + amount > 5000000 )
          amount = 5000000 - ch->pcdata->balance;
  }
  ch->gold		-= amount;
  ch->pcdata->balance	+= amount;
  sprintf( buf, "You deposit %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "" );
  set_char_color( AT_WHITE, ch );
  send_to_char( buf, ch );
  sprintf( buf, "$n deposits %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "" );
  act( AT_WHITE, buf, ch, NULL, NULL, TO_ROOM );
  return;
}

void do_withdraw( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char arg1[MAX_INPUT_LENGTH];
  char buf [MAX_STRING_LENGTH];
  int amount;
  
  if ( !( banker = find_banker( ch ) ) )
  {
    send_to_char( "&WYou're not in a bank!&w\n\r", ch );
    return;
  }
  
  if ( IS_NPC( ch ) )
  {
    sprintf( buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr );
    do_say( banker, buf );
    return;
  }
  
  if ( argument[0] == '\0' )
  {
    do_say( banker, "If you need help, see HELP BANK." );
    return;
  }
  
  argument = one_argument( argument, arg1 );
    
  if ( arg1 == '\0' )
  {
    sprintf( buf, "%s How much gold do you wish to withdraw?", ch->name );
    do_tell( banker, buf );
    return;
  }
  if ( str_cmp( arg1, "all" ) && !is_number( arg1 ) )
  { 
    sprintf( buf, "%s How much gold do you wish to withdraw?", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( !str_cmp( arg1, "all" ) )
    amount = ch->pcdata->balance;    
  else
    amount = atoi( arg1 );
  
  if ( amount > ch->pcdata->balance )
  {
    sprintf( buf, "%s But you do not have that much gold in your account!", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( amount <= 0 )
  {
    sprintf( buf, "%s Oh I see.. I didn't know i was doing business with a comedian.", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  ch->pcdata->balance	-= amount;
  ch->gold		+= amount;
  sprintf( buf, "You withdraw %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "" );
  set_char_color( AT_WHITE, ch );
  send_to_char( buf, ch );
  sprintf( buf, "$n withdraws %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "" );
  act( AT_WHITE, buf, ch, NULL, NULL, TO_ROOM );
  return;
}

void do_balance( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char buf [MAX_STRING_LENGTH];
    
    if ( !( banker = find_banker( ch ) ) )
    {
      send_to_char( "&WYou're not in a bank!&w\n\r", ch );
      return;
    }
    
    if ( IS_NPC( ch ) )
    {
      sprintf( buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr );
      do_say( banker, buf );
      return;
    }
  
    set_char_color( AT_WHITE, ch );
    sprintf( buf, "You have %d gold coin%s in the bank.\n\r",
    ch->pcdata->balance, (ch->pcdata->balance == 1) ? "" : "s" );
    send_to_char( buf, ch );
    return;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  char buf [MAX_STRING_LENGTH];
  int amount;
  
  if ( !( banker = find_banker( ch ) ) )
  {
    send_to_char( "&WYou're not in a bank!&w\n\r", ch );
    return;
  }
  
  if ( IS_NPC( ch ) )
  {
    sprintf( buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr );
    do_say( banker, buf );
    return;
  }
  
  if ( argument[0] == '\0' )
  {
    do_say( banker, "If you need help, see HELP BANK." );
    return;
  }
  
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
    
  if ( arg1 == '\0' || arg2 == '\0' )
  {
    sprintf( buf, "%s How much gold do you wish to send to who?", ch->name );
    do_tell( banker, buf );
    return;
  }
  if ( str_cmp( arg1, "all" ) && !is_number( arg1 ) )
  {
    sprintf( buf, "%s How much gold do you wish to send to who?", ch->name );
    do_tell( banker, buf );
    return;
  }
  
  if ( !( victim = get_char_world( ch, arg2 ) ) )
  {
    sprintf( buf, "%s %s could not be located.", ch->name, capitalize(arg2) );
    do_tell( banker, buf );
    return;
  }
    
  if ( IS_NPC( victim ) )
  {
    sprintf( buf, "%s We do not do business with mobiles...", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( !str_cmp( arg1, "all" ) )
    amount = ch->pcdata->balance;
  else
    amount = atoi( arg1 );
  
  if ( amount > ch->pcdata->balance )
  {
    sprintf( buf, "%s You are very generous, but you don't have that much gold!", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  if ( amount <= 0 )
  {
    sprintf( buf, "%s Oh I see.. I didn't know I was doing business with a comedian.", ch->name );
    do_tell( banker, buf );
    return;
  }
    
  ch->pcdata->balance     -= amount;
  victim->pcdata->balance += amount;
  sprintf( buf, "You transfer %d gold coin%s to %s's bank account.\n\r",
           amount, (amount != 1) ? "s" : "", victim->name );
  set_char_color( AT_WHITE, ch );
  send_to_char( buf, ch );
  sprintf( buf, "%s just transferred %d gold coin%s to your bank account.\n\r",
           ch->name, amount, (amount != 1) ? "s" : "" );
  set_char_color( AT_WHITE, victim );
  send_to_char( buf, victim );
  return;
}

/* End of new bank support */
