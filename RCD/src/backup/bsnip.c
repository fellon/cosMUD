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

void do_aexit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*room;
    int			 i,vnum;
    AREA_DATA		*tarea;
    EXIT_DATA *pexit;
    int lrange;
    int trange;

    tarea = ch->in_room->area;
    trange = tarea->hi_r_vnum;
    lrange = tarea->low_r_vnum;
    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
	if ( (room = get_room_index( vnum )) == NULL )
	  continue;
        for (i=0;i<MAX_DIR+1;i++)
        {
	    if ( (pexit = get_exit(room, i)) == NULL)
			continue;
		if(pexit->to_room->area != tarea)
                {
			pager_printf(ch, "To: %-20.20s %s\n\r",pexit->to_room->area->filename,pexit->to_room->area->name);
 			pager_printf( ch,"%15d %-30.30s -> %15d (%s)\n\r", vnum, room->name,pexit->vnum,dir_name[i] );
		}
	}
    }
}

void do_aentrance( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*room;
    int			 i,vnum;
    AREA_DATA		*tarea;
    AREA_DATA		*otherarea;

    EXIT_DATA *pexit;
    int lrange;
    int trange;

    tarea = ch->in_room->area;
    for ( otherarea=first_area; otherarea; otherarea=otherarea->next)
    {
	if (tarea == otherarea)
		continue;
    	trange = otherarea->hi_r_vnum;
    	lrange = otherarea->low_r_vnum;
    	for ( vnum = lrange; vnum <= trange; vnum++ )
    	{

		if ( (room = get_room_index( vnum )) == NULL )
	  		continue;
        	for (i=0;i<MAX_DIR+1;i++)
        	{
			if ( (pexit = get_exit(room, i)) == NULL)
				continue;
			if(pexit->to_room->area == tarea)
			{
			        pager_printf(ch, "From: %-20.20s %s\n\r",otherarea->filename,otherarea->name);
 				pager_printf(ch, "%15d %-30.30s -> %15d (%s)\n\r", vnum, room->name,pexit->vnum,dir_name[i] );
			}
		}
	}
    }
}
