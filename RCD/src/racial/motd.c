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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* MOTD Displayer -- Chris/James/Bill -- 4/99 */

void do_motd ( CHAR_DATA *ch, char *argument )
{		
    if ( IS_NPC(ch) )
        return;

    if ( ch->level >= 103 )
    {
        if ( argument[0] == '\0' )
        {
            send_to_char( "Syntax:  motd <imotd / gmotd / amotd / motd / nmotd>\n\r", ch );
            return;
        }
        else
        {
            if ( !str_cmp( argument, "imotd" ) )
            {
		do_help( ch, "imotd" );
            }
            else if ( !str_cmp( argument, "gmotd" ) )
            {
		do_help( ch, "gmotd" );
            }
            else if ( !str_cmp( argument, "amotd" ) )
            {
		do_help( ch, "amotd" );
            }
            else if ( !str_cmp( argument, "motd" ) )
            {
                do_help( ch, "motd" );
            }
            else if ( !str_cmp( argument, "nmotd" ) )
            {
                do_help( ch, "nmotd" );
            }
	    else
            {
            send_to_char( "Syntax:  motd <imotd / gmotd / amotd / motd / nmotd>\n\r", ch );
                return;
            }

        }
    }
    else if ( ch->level >= 101 )
    {
        do_help( ch, "gmotd" );
    }
    else if ( ch->level >= 50 )
    {
        do_help( ch, "amotd" );
    }
    else if ( ch->level >= 2 )
    {
        do_help( ch, "motd" );
    }
    else if ( ch->level == 1 )
    {
        do_help( ch, "nmotd" );
    }

}

/* MOTD File Edit -- Chris -- 4/99 */

/*
 *	Do no add do_medit, it is not done and does not work well -- KCAH
 */

void do_medit ( CHAR_DATA *ch, char *argument )
{		
    if ( IS_NPC(ch) )
        return;
    
/*      if ( argument[0] == '\0' )
        {
            send_to_char( "&wSyntax: medit <amotd/smotd/cmotd/nmotd/save>\n\r",ch );
            return;
        }
        else
        { */

/*	    interpret (ch, "clear"); */
/*	    set_char_color ( AT_BLUE, ch ); */
/*          send_to_char( "                           &w^x&wMessage of the Day Editor\n\r",ch ); */

            if ( !str_cmp( argument, "amotd" ) )
            {
/*              send_to_char( "&CAdministration MOTD\n\r\n\r",ch ); */
		do_hedit ( ch, "_amotd_");
/*		interpret (ch, "hedit _amotd_"); */
/*		return; */
            }
            else if ( !str_cmp( argument, "smotd" ) )
            {
                send_to_char( "&CStaff MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_smotd_");
		return;
            }
            else if ( !str_cmp( argument, "cmotd" ) )
            {
                send_to_char( "&CCitizen MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_cmotd_");
		return;
            }
            else if ( !str_cmp( argument, "nmotd" ) )
            {
                send_to_char( "&CNewbie Citizen MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_nmotd_");
		return;
            }
/*            else if ( !str_cmp( argument, "/" ) )
            {
		do_hedit ( ch, "");
		return;
            } */
            else if ( !str_cmp( argument, "save" ) )
            {
		do_hset( ch, "level -1 _amotd_");
		do_hset( ch, "level -1 _smotd_");
		do_hset( ch, "level -1 _cmotd_");
		do_hset( ch, "level -1 _nmotd_");
		do_hset( ch, "save");
                send_to_char( "&w^x&wAll MOTD files saved.\n\r",ch );
		return;
            }
/*            else
            {
		set_char_color ( AT_BLUE, ch );
                send_to_char( "&w^x&wSyntax: medit <amotd/smotd/cmotd/nmotd/save>\n\r",ch );
                return;
            } */

        }
/* } */
