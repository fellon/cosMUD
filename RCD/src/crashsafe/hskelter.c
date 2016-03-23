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

/*
 *  All Pkill Mode code (Helter Skelter day)
 *  Author: Cronel (cronel_kal@hotmail.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "mud.h"

#define HSKELTER_BACKUP ("HelterSkelter.tgz")
#define HSKELTER_AFTER ("HelterPlayer.tgz")

bool exists_file( char *filen );
void save_sysdata( SYSTEM_DATA sys );
void free_hpk_list( void );

#ifdef HISCORE_CODE
typedef struct hskelter_pkill HSKELTER_PKILL;
struct hskelter_pkill
{
	char *name;
	int pkills;

	HSKELTER_PKILL *next;
	HSKELTER_PKILL *prev;
};

HSKELTER_PKILL *first_hpk, *last_hpk;
#endif

void do_hskelter( CHAR_DATA *ch, char *argument )
{
	char cmd[ MAX_INPUT_LENGTH ];
	char buf[ MAX_STRING_LENGTH ];
	bool override;
	CHAR_DATA *victim;

	argument = one_argument( argument, cmd );
	override = !str_cmp( argument, "!" );

	set_char_color( AT_IMMORT, ch );

	if( cmd[0] == '\0' )
	{
		send_to_char( "Syntax: hskelter [start, end]\n\r", ch );
		return;
	}
	else if( !str_cmp( cmd, "start" ) )
	{
		if( sysdata.all_pkill_mode )
		{
			send_to_char( "Helter Skelter mode is already on.\n\r", ch );
			return;
		}

		if( exists_file( HSKELTER_BACKUP ) )
		{
			send_to_char( "An old Helter Skelter backup file exists.\n\r", ch );
			if( !override )
			{
				send_to_char( "Remove it, or use '!'.\n\r", ch );
				return;
			}
			else
			{
				send_to_char( "Removing...\n\r", ch );
				unlink( HSKELTER_BACKUP );
			}
		}
		send_to_char( "Creating new player base backup...\n\r", ch );
		sprintf( buf, "tar -zcf %s %s*", HSKELTER_BACKUP, PLAYER_DIR );
		system( buf );
		/* check for the file */
		if( !exists_file( HSKELTER_BACKUP ) )
		{
			send_to_char( "Cannot create player base backup!\n\r", ch );
			return;
		}

		sysdata.all_pkill_mode = TRUE;
		save_sysdata( sysdata );

		echo_to_all( AT_YELLOW | AT_BLINK, "Helter Skelter day has officialy STARTED!", ECHOTAR_PC );

#ifdef HISCORE_CODE
		first_hpk = last_hpk = NULL;
#endif
		set_char_color( AT_IMMORT, ch );
		send_to_char( "Ready.\n\r", ch );
	}
	else if ( !str_cmp( cmd, "end" ) )
	{
		if( !sysdata.all_pkill_mode )
		{
			send_to_char( "Helter Skelter mode is not on right now.\n\r", ch );
			return;
		}

		if( !exists_file(HSKELTER_BACKUP) )
		{
			send_to_char( "There's no player base backup!\n\r", ch );
			return;
		}

		echo_to_all( AT_YELLOW | AT_BLINK, "Helter Skelter day is officialy CLOSED.", ECHOTAR_PC );
		echo_to_all( AT_YELLOW | AT_BLINK, "You will all be forced to quit to restore the player base.", ECHOTAR_PC );
		echo_to_all( AT_YELLOW | AT_BLINK, "You should then wait a minute and reconnect.", ECHOTAR_PC );

		set_char_color( AT_IMMORT, ch );
		send_to_char( "Backing up post-helter skelter player base\n\r", ch );
		sprintf( buf, "tar -zcf %s %s*", HSKELTER_AFTER, PLAYER_DIR );
		system( buf );

		send_to_char( "Forcing everyone to quit.\n\r", ch );
		for( victim = first_char; victim ; victim = victim->next )
		{
			if( IS_NPC(victim) )
				continue;
			/* remove recent fight timer */
			if( get_timer( victim, TIMER_RECENTFIGHT ) )
				remove_timer( victim, TIMER_RECENTFIGHT);

			/* make everyone STANDING (for all peace, etc) */
			stop_fighting( victim, TRUE );
			victim->position = POS_SLEEPING;
		}
		do_force( ch, "all quit" );

		send_to_char( "Restoring pre-helter skelter player base backup.\n\r", ch );
		sprintf( buf, "tar -zvxf %s", HSKELTER_BACKUP );
		system( buf );

		sysdata.all_pkill_mode = FALSE;
		save_sysdata( sysdata );

		free_hpk_list( );

		send_to_char( "Ready.\n\r", ch );
	}
	else
	{
		send_to_char( "You can either manualy START or END helter skelter mode.\n\r", ch );
		return;
	}
}

bool exists_file( char *filen )
{
	struct stat fst;

	if( stat(filen, &fst) == -1 )
		return FALSE;
	else
		return TRUE;
}

void register_hskelter_pkill( CHAR_DATA *ch )
{
#ifdef HISCORE_CODE
	HSKELTER_PKILL *hpk;

	if( !sysdata.all_pkill_mode )
		return;

	for( hpk = first_hpk ; hpk ; hpk = hpk->next )
	{
		if( !str_cmp( hpk->name, ch->name ) )
			break;
	}

	if( hpk != NULL )
		hpk->pkills++;
	else
	{
		CREATE( hpk, HSKELTER_PKILL, 1 );
		hpk->name = STRALLOC( ch->name );
		hpk->pkills = 1;
		LINK( hpk, first_hpk, last_hpk, next, prev );
	}

	adjust_hiscore( "hskelter_pkill", ch, hpk->pkills );
#endif
}

void free_hpk_list( void )
{
#ifdef HISCORE_CODE
	HSKELTER_PKILL *hpk, *n_hpk;

	for( hpk = first_hpk ; hpk ; hpk = n_hpk )
	{
		n_hpk = hpk->next;
		UNLINK( hpk, first_hpk, last_hpk, next, prev );
		STRFREE( hpk->name );
		DISPOSE( hpk );
	}

	first_hpk = last_hpk = NULL;
#endif
}


