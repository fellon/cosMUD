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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <time.h>

#include "mud.h"

/*
 
 Locker System, (c) 1997-2001 Jay Roman, jay@directlink.net
 =====================================================================

*/ 

LOCKER_DATA *load_locker( CHAR_DATA *ch );
void fread_locker( CHAR_DATA *ch, LOCKER_DATA *locker, FILE *fp );
void fwrite_locker( CHAR_DATA *ch );
ROOM_INDEX_DATA *generate_virtual_room( ROOM_INDEX_DATA *in_room );
void delete_locker( CHAR_DATA *ch );

/* Declared in act_move.c */
extern ROOM_INDEX_DATA * vroom_hash [64];

#define LOCKER_PRICE    500000
#define LOCKER_CAPACITY 500

LOCKER_DATA *load_locker( CHAR_DATA *ch )
{
    LOCKER_DATA *locker;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;
    char buf[MAX_INPUT_LENGTH];
        
    locker = NULL;
    
    /* 
     * Find the locker file 
     */
        
    sprintf( strsave, "%s%s", LOCKER_DIR, capitalize( ch->name ) );
    
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {   
        CREATE( locker, LOCKER_DATA, 1 );
        locker->capacity     = 0;
        locker->holding      = 0;
        locker->flags        = 0;
	locker->room         = 0;
	
	found = TRUE;

	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_locker: # not found.", 0 );
		bug( ch->name, 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "LOCKER" ) )
	    {
		fread_locker ( ch, locker, fp );

                room = generate_virtual_room( ch->in_room );
          
                ch->pcdata->locker_vnum = ch->in_room->vnum;
                ch->pcdata->locker_room = room;

                /* Move the character to the locker room */
                char_from_room( ch );
                char_to_room( ch, room );
	    }
	    else if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
		fread_obj( ch, fp, OS_LOCKER );
	    else if ( !str_cmp( word, "END"    ) )	/* Done		*/
		break;
	    else
	    {
		bug( "Load_locker: bad section.", 0 );
		sprintf( buf, "[%s]", word );
		bug( buf, 0 );
		bug( ch->name, 0 );
		break;
	    }
	}

	fclose( fp );

        /* Total the weight of the contents */
        for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        {
            locker->holding = locker->holding + ( obj->weight * obj->count );
        }
    }
    
    return locker;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_locker( CHAR_DATA *ch, LOCKER_DATA *locker, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	    case '*':
	       fMatch = TRUE;
	       fread_to_eol( fp );
	       break;
 
	    case 'C':
	        KEY( "Capacity", locker->capacity, fread_number( fp ) );
	        break;

	    case 'F':
	        KEY( "Flags",    locker->flags, fread_number( fp ) );
	        break;
	        
	    case 'E':
                if( ! str_cmp( word, "End" ) )
	    	    return;
	    	break;
	    	
	    case 'R':
	        KEY( "Room",     locker->room, fread_number( fp ) );
	        break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_locker: no match: %s", word );
	    bug( buf, 0 );
	}
    }
    
}

void fwrite_locker( CHAR_DATA *ch )
{
    /* Variables */
    FILE *fp = NULL;
    OBJ_DATA *obj;
    char strsave[MAX_INPUT_LENGTH];

    if( ! ch->pcdata->locker )
    {
	bug( "Fwrite_locker: NULL object.", 0 );
	bug( ch->name, 0 );
        return;
    }

    sprintf( strsave, "%s%s", LOCKER_DIR, capitalize( ch->name ) );
    
    if ( ( fp = fopen( strsave, "w" ) ) != NULL )
    {
        /* Save the locker details */
        fprintf( fp, "#LOCKER\n" );
        fprintf( fp, "Capacity     %d\n", ch->pcdata->locker->capacity );
        fprintf( fp, "Flags        %d\n", ch->pcdata->locker->flags );
        fprintf( fp, "End\n\n" );
    
        /* Save the objects in the locker room */
        for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        {
            fwrite_obj( ch, obj, fp, 0, OS_LOCKER );
        }

        fprintf( fp, "#END\n" );
        fclose( fp );
    }
    return;
}

void do_locker( CHAR_DATA *ch, char *argument )
{ 
    /* Variables */   
    char buf [MAX_INPUT_LENGTH];
    LOCKER_DATA *locker;
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
          
    if ( IS_NPC(ch) )
        return;
        
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: locker <open | close>\r\n", ch );
	return;
    }
    
    if( ! str_cmp( arg, "open" ) )
    {
        if( ch->pcdata->locker ) 
        {
            send_to_char( "Your locker is already open.\r\n", ch );
            return;
        }

        if(!IS_SET(ch->in_room->room_flags, ROOM_LOCKER))
        {
            send_to_char( "Does this smell like a locker room to you?\r\n", ch );
            return;
        }

        locker = load_locker( ch );

        if( ! locker )
        {
    	    send_to_char( "You do not have a locker.\r\n", ch );
	    return;
        }

	ch->pcdata->locker = locker;

        do_look( ch, "auto" );
        send_to_char( "You are now in your personal locker.\r\n", ch );
    }
    else if( ! str_cmp( arg, "close" ) ) 
    {
        if( ! ch->pcdata->locker )
        {
            send_to_char( "You are not currently in the locker room.\r\n", ch );
            return;
        }
        
    	locker = ch->pcdata->locker;

        /* Total the weight of the contents */
        locker->holding = 0;
        
        for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        {
            locker->holding = locker->holding + ( obj->weight * obj->count );

            if( obj->item_type == ITEM_CONTAINER )
            {
                send_to_char( "You may not leave containers in the locker room.\r\n", ch );
                return;
            } 
        }

        if( locker->holding > locker->capacity )
        {
            sprintf( buf, "The weight limit of your locker has beed exceeded.  "
                          "You must carry\r\nsome items out when you leave.\r\n" );
            send_to_char( buf, ch );
            return;
        } 
        
        /* Save the locker */
        fwrite_locker( ch );

        /* Return the player to the real world. */ 
        char_from_room( ch );
        char_to_room( ch, get_room_index( ch->pcdata->locker_vnum ) );

        delete_locker( ch );

        do_look( ch, "auto" );
        send_to_char( "Your locker has been closed and saved.\r\n", ch );
       }    
    else
    {
        send_to_char( "Syntax: locker <open | close>\r\n", ch );
    }
       
    return;    
}

ROOM_INDEX_DATA *generate_virtual_room( ROOM_INDEX_DATA *in_room )
{
    ROOM_INDEX_DATA *room = NULL;
    int    serial;
    sh_int hash;
    bool   found;
    int vnum = in_room->vnum;

    do
    {
        found  = FALSE;
        serial = (vnum << 16) | vnum;
    
        hash = serial % 64;
    
        for ( room = vroom_hash[hash]; room; room = room->next )
        {
	    if ( room->vnum == serial )
   	    {
	        found = TRUE;
	        break;
	    }
	}
        vnum++;
    } while( found );
    
    if ( !found )
    {
	CREATE( room, ROOM_INDEX_DATA, 1 );
	room->area	  = in_room->area;
	room->vnum	  = serial;
	room->tele_vnum	  = 1;
	room->sector_type = SECT_INSIDE;
	room->room_flags  = ROOM_INDOORS | ROOM_PRIVATE | ROOM_NO_RECALL | ROOM_NO_SUMMON | ROOM_NO_ASTRAL | ROOM_SOLITARY;
	room->next	  = vroom_hash[hash];
	vroom_hash[hash]  = room;
	++top_vroom;
    }
    
    if( room )
    {
        if ( room->name )
            STRFREE( room->name );
        if ( room->description )
            STRFREE( room->description );
    
        room->name = STRALLOC( "Locker Room" );
        room->description = STRALLOC( "Use LOCKER CLOSE to leave your locker and save its contents.\r\nYou may not leave containers in the locker.\r\n" );
    }
    return room;
}

void delete_locker( CHAR_DATA *ch )
{
    /* Clean-up the locker room -- Remove the loaded objects */
    while ( ch->pcdata->locker_room->first_content )
    {
        extract_obj( ch->pcdata->locker_room->first_content );
    }
    
    /* Kill the locker object */
    DISPOSE( ch->pcdata->locker );
    ch->pcdata->locker      = NULL;

    ch->pcdata->locker_vnum = 0;
    ch->pcdata->locker_room = NULL;

    /* Need to delete the virtual room */
    clear_vrooms();
}

void do_makelocker( CHAR_DATA *ch, char *argument )
{
    FILE *fp;
    char filename[256];
    LOCKER_DATA *locker;
    char buf [MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makelocker <player name>\n\r", ch );
	return;
    }

    /* Is this a valid player? */
    strcpy( name, capitalize(argument) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), name );
    if ( stat( buf, &fst ) == -1 )
    {
        sprintf( buf, "Player file for %s does not exist!\r\n", name );
        send_to_char( buf, ch );
        return;
    }

    sprintf( filename, "%s%s", LOCKER_DIR, name );
    
    if ( stat( filename, &fst ) != -1 )
    {
        sprintf( buf, "Locker file for %s already exists!\r\n", name );
        send_to_char( buf, ch );
        return;
    }

    CREATE( locker, LOCKER_DATA, 1 );
    locker->capacity     = LOCKER_CAPACITY;
    locker->flags        = 0;

    if ( ( fp = fopen( filename, "w" ) ) != NULL )
    {
        /* Save the locker details */
        fprintf( fp, "#LOCKER\n" );
        fprintf( fp, "Capacity     %d\n", locker->capacity );
        fprintf( fp, "Flags        %d\n", locker->flags );
        fprintf( fp, "End\n\n" );
    
        fprintf( fp, "#END\n" );
        fclose( fp );

        sprintf( buf, "Locker [%s] created for %s\r\n", filename, argument );
        send_to_char( buf, ch );
    }

    return;
}

void do_buylocker( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  struct stat fst;

  if( !IS_SET( ch->in_room->room_flags, ROOM_LOCKER ) )
  {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
  }
  
  if( ch->gold >= LOCKER_PRICE )
  {
     sprintf( buf, "%s%s", LOCKER_DIR, ch->name );
    
     if( stat( buf, &fst ) != -1 )
     {
        sprintf( buf, "You already have a locker!\r\n" );
        send_to_char( buf, ch );
     }
     else
     {  
        ch->gold -= LOCKER_PRICE;

        do_makelocker( ch, ch->name );
        
        send_to_char( "Your locker has now been created.\r\n", ch );
     }
  }
  else
  {
     send_to_char( "You do not have the gold to buy a locker!\r\n", ch );
  }
  return;
}
