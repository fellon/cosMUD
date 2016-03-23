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

void do_saveall( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *vch;
    AREA_DATA   *tarea;
    char         filename[256];
   
    for ( vch = first_char; vch; vch = vch->next )
    {
        if ( !IS_NPC( vch ) )
        {
           save_char_obj( vch );
           set_pager_color( AT_PLAIN, ch );
           pager_printf( ch, "Saving %-s...\n\r", vch->name);
        }
    }
    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
       if ( !IS_SET(tarea->status, AREA_LOADED ) )
       {
          continue;
       }
       sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
       fold_area( tarea, filename, FALSE );
    }

}

