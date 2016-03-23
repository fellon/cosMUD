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

#include <stdio.h>
#include "mud.h"
extern bool wizlock;

void do_iscore(CHAR_DATA * ch, char *argument)
{
    char buf [MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
    {
        do_oldscore(ch, argument);
        return;
    }

    set_pager_color(AT_SCORE, ch);
    pager_printf_color(ch, "\n\r&cImmortal Data: &W%s&c.\n\r", ch->name );

    if ( get_trust( ch ) != ch->level )
        pager_printf( ch, "&cYou are trusted at level &W%d&c.\n\r", get_trust( ch ) );

    send_to_pager_color("&B----------------------------------------------------------------------------\n\r", ch);

    if (IS_IMMORTAL(ch))
    {
        pager_printf_color(ch, "&cLevel : &W%-6d      &cClass : &W%-10.10s      &cPlayed: &W%d hours\n\r",
                           ch->level, capitalize(get_class(ch)), (get_age(ch) - 17) * 2);
        pager_printf_color(ch, "&cYears : &W%-6d      &cRace  : &W%-10.10s      &cLog In: &W%s\n\r",
                           get_age(ch), capitalize(get_race(ch)), ctime(&(ch->logon)) );

        send_to_pager_color("&B----------------------------------------------------------------------------\n\r", ch);

        pager_printf_color(ch, "&RBamfin : &p%s\n\r", (ch->pcdata->bamfin[0] != '\0')
                           ? ch->pcdata->bamfin : "Not changed/Switched.");
        pager_printf(ch, "&RBamfout: &p%s\n\r", (ch->pcdata->bamfout[0] != '\0')
                     ? ch->pcdata->bamfout : "Not changed/Switched.");

        if (ch->pcdata->area)
        {
            send_to_pager_color("&B----------------------------------------------------------------------------\n\r", ch);
            pager_printf(ch, "&RVnums  : &pRoom &W(&R%-5.5d &Y- &R%-5.5d&W)   &pObject &W(&R%-5.5d &Y- &R%-5.5d&W)   &pMob &W(&R%-5.5d &Y- &R%-5.5d&W)\n\r",
                         ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum, ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
            pager_printf(ch, "&p         Area Loaded &W[%s&W]\n\r",
                         (IS_SET(ch->pcdata->area->status, AREA_LOADED))
                         ? "&GYES" : "&RNO");
        }
        send_to_pager_color("&B----------------------------------------------------------------------------\n\r",ch);

        sprintf( buf, "&cHolylight: %3s    &cInvis: %3s &c[&W%2d&c]    &cIncog: %3s &c[&W%2d&c]    &cGhost: %3s &c[&W%2d&c]\n\r",
                 xIS_SET( ch->act, PLR_HOLYLIGHT ) ? "&GON&c "  : "&ROFF&c",
                 xIS_SET( ch->act, PLR_WIZINVIS  ) ? "&GON&c "  : "&ROFF&c",
                 ch->pcdata->wizinvis,
                 ch->incog_level > 0 ? "&GON&c " : "&ROFF&c",
                 ch->incog_level,
                 ch->ghost_level > 0 ? "&GON&c " : "&ROFF&c",
                 ch->ghost_level );
	pager_printf( ch, buf );

        sprintf( buf, "&cWizlock  : %s\n\r",
                 wizlock == TRUE     ? "&GON&c " : "&ROFF&c");
	pager_printf( ch, buf );

    }
    send_to_pager("\n\r", ch);
    return;
}

/*
 * Incognito is a level based command. Once cloaked,
 * all players up to the level set will not be able
 * to see you, unless they are in the same room.
 * -Froboz
 */
void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    /* Defaults to Players max_level */
    if ( arg[0] == '\0' )
        if ( ch->incog_level)
        {
            ch->incog_level = 0;
            act( AT_BLUE, "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You are no longer cloaked.\n\r", ch );
        }
        else
        {
            ch->incog_level = get_trust(ch);
            ch->ghost_level = 0;
            act( AT_BLUE, "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You cloak your presence.\n\r", ch );
        }
    /* do the level thing */
    else
    {
        level = atoi(arg);
        if (level < 2 || level > get_trust(ch))
        {
            send_to_char("Incog level must be between 2 and your level.\n\r",ch);
            return;
        }
        else
        {
            ch->reply = NULL;
            ch->incog_level = level;
            ch->ghost_level = 0;
            act( AT_BLUE, "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You cloak your presence.\n\r", ch );
        }
    }

    return;
}

/*
 * Ghost hides the immortal in the room they are
 * standing.  They still show up on who.
 * -Froboz
 */
void do_ghost( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
        /* take the default path */
        if ( ch->ghost_level)
        {
            ch->ghost_level = 0;
            act( AT_WHITE, "$n steps out from the mist.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You step out from the mist.\n\r", ch );
        }
        else
        {
            ch->ghost_level = get_trust(ch);
            ch->incog_level = 0;
            act( AT_WHITE, "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You vanish into a mist.\n\r", ch );
        }
    else
        /* do the level thing */
    {
        level = atoi(arg);
        if (level < 2 || level > get_trust(ch))
        {
            send_to_char("Ghost level must be between 2 and your level.\n\r",ch);
            return;
        }
        else
        {
            ch->reply = NULL;
            ch->ghost_level = level;
            ch->incog_level = 0;
            act( AT_WHITE, "$n vanishes into a mist.", ch, NULL, NULL,
                 TO_ROOM );
            send_to_char( "You vanish into a mist.\n\r", ch );
        }
    }

    return;
}
