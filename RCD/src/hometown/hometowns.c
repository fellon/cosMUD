/*****************************************************
**     _________       __			    **
**     \_   ___ \_____|__| _____  ________  ___     **
**	/    \	\/_  __ \ |/	 \/  ___/_ \/	\   **
**	\     \___|  | \/ |  | |  \___ \  / ) |  \  **
**	 \______  /__| |__|__|_|  /____ \__/__|  /  **
**	   ____\/____ _        \/ ___ \/      \/    **
**	   \______   \ |_____  __| _/___	    **
**	    |	 |  _/ |\__  \/ __ | __ \	    **
**	    |	 |   \ |_/ __ \  / | ___/_	    **
**	    |_____  /__/____  /_  /___	/	    **
**		 \/Antipode\/  \/    \/ 	    **
******************************************************
**	   Crimson Blade Codebase (CbC) 	    **
**     (c) 2000-2002 John Bellone (Noplex)	    **
**	     Coders: Noplex, Krowe		    **
**	  http://www.crimsonblade.org		    **
******************************************************
** Based on SMAUG 1.4a, by; Thoric, Altrag, Blodkai **
**  Narn, Haus, Scryn, Rennard, Swordbearer, Gorog  **
**    Grishnakh, Nivek, Tricops, and Fireblade	    **
******************************************************
** Merc 2.1 by; Michael Chastain, Michael Quan, and **
**		    Mitchell Tse		    **
******************************************************
**   DikuMUD by; Sebastian Hammer, Michael Seifert, **
**     Hans Staerfeldt, Tom Madsen and Katja Nyobe  **
*****************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )		\
	if ( !str_cmp( word, literal ))		\
	{					\
	   field  = value;			\
	   fMatch = TRUE;			\
	   break;				\
	}

/*
 * Save Hometowns		-Nopey
 * 6/16/01
 */
/* void save_hometowns( HOMETOWN_DATA *htown )
{
	FILE *fp;
	char filename[256];

	if( !htown )
	{
		bug( "save_hometowns: null pointer. Who fucked up this time?" );
		return;
	}

	if( !htown->filename || htown->filename[0] == '\0' )
	{
		bug( "save_hometowns: no filename. What the fuck?" );
		return;
	}

	sprintf( filename, "%s%s", HTOWN_DIR, htown->filename );
	fclose( fpReserve );
	if((fp = fopen( filename, "w" )) == NULL )
	{
		bug( "save_hometowns: fopen" );
		perror( filename );
	}
	else
	{
		fprintf( fp, "\n#HOMETOWN\n" );
		fprintf( fp, "Filename		%s~\n", htown->filename );
		fprintf( fp, "Name		%s~\n", htown->name );
		if( htown->recall != 0 ) 
		fprintf( fp, "Recall		%d\n", htown->recall );
		fprintf( fp, "End\n" );
	}
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
}
*/
/*
 * Load Hometowns			-Nopey
 * Rewritten 6/17/01
 */
void load_hometowns( )
{
	FILE *fpList;
	char *filename;
	char htownlist[256];
	char buf[MAX_INPUT_LENGTH];
	
	first_hometown = NULL;
	last_hometown = NULL;

	log_string( "Loading Hometown list..." );
	sprintf( htownlist, "%shtown.lst", HTOWN_DIR );
	fclose( fpReserve );
	if( ( fpList = fopen( htownlist, "r" ) ) == NULL )
	{
		bug( "No Hometown list found." );
		bug( "Fix the Damn thing already." );
		perror( htownlist );
		exit( 1 );
	}
	
    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;
	
	if ( !load_htownfile( filename ) ) {
	  sprintf( buf, "Cannot load htown file: %s", filename );
	  bug( buf, 0 );
	}

	}
	fclose( fpList );
	log_string( "Done hometowns." );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
}

/*
 * Write Hometown LIst			-Nopey
 * 6/17/01
 */
void save_hometown_list( )
{
	FILE *fp;
	HOMETOWN_DATA *htown;
	char filename[256];

	sprintf( filename, "%shtown.lst", HTOWN_DIR );
	if( (fp = fopen( filename, "w" ) ) == NULL )
	{
		bug( "FATAL: cannot open hometown.lst to write!!" );
		return;
	}

	for( htown = first_hometown; htown; htown = htown->next )
		fprintf( fp, "%s\n", htown->filename );

	fprintf( fp, "$\n" );
	fclose( fp );
}

/*
 * Load a single Hometown file		-Nopey
 * 6/17/01
 */
bool load_htownfile( char *htownfile ) 
{
	HOMETOWN_DATA *htown;
	FILE *fp;
	bool found = FALSE;
	char filename[256];

	CREATE( htown, HOMETOWN_DATA, 1 );
	htown->name	= NULL;
	htown->recall	= 0;

	found = FALSE;
	sprintf( filename, "%s%s", HTOWN_DIR, htownfile );
	if( (fp = fopen( filename, "r" ) ) != NULL)
	{
		found = TRUE;
		for( ; ; ) {
		char letter;
		char *word;
		
		letter = fread_letter(fp);
		if( letter == '*' )
		{
			fread_to_eol(fp);
			continue;
		}
		if( letter != '#' )
		{
			bug( "Bad hometown file, no #" );
			break;
		}
		word = fread_word(fp);
		if( !str_cmp( word, "HOMETOWN" ) )
		{
			fread_hometown( htown, fp );
			break;
		}
		else
		if( !str_cmp( word, "END" ) )
		break;
		else
		{
			char buf[MAX_INPUT_LENGTH];
			sprintf( buf, "Bad Section %s in hometown file %s", word, htownfile );
			bug( buf, 0 );
			break;
		}
	    }
	}
	fclose( fp );

	if( found )
		LINK( htown, first_hometown, last_hometown, next, prev );
	if( !found )
		free_hometown( htown );

	return found;
}

/*
 * Fread Hometown		-Nopey
 * 6/17/01
 */
void fread_hometown( HOMETOWN_DATA *htown, FILE *fp )
{
	char buf[MAX_INPUT_LENGTH];
	char *word;
	bool fMatch;

	for( ; ; )
	{
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;
		switch( UPPER(word[0]) )
		{
			case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;
	
			case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !htown->name)
				htown->name	= STRALLOC( htown->filename );
				
				if( !htown->recall)
				htown->recall	= ROOM_VNUM_TEMPLE;
				
				return;
			}
			break;
		
			case 'F':
			KEY( "Filename", htown->filename, fread_string_nohash(fp) );
			break;

			case 'N':
			KEY( "Name", htown->name, fread_string(fp) );
			break;

			case 'R':
			KEY( "Recall", htown->recall, fread_number(fp) );
			break;
		}

		if( !fMatch )
		{
			sprintf( buf, "Fread_Hometown: no match for %s", word );
			bug( buf, 0 );
		}
	}
}
/*
 * Lookup Hometown		-Nopey
 * 6/16/01
 * Rewritten 6/20/01 (I knew I wrote it wrong)
 */
HOMETOWN_DATA *hometown_lookup( char *name )
{
	HOMETOWN_DATA *htown;

	for( htown = first_hometown; htown; htown = htown->next )
	{
		if( !str_cmp( name, htown->name ) )
			return htown;
		if( !str_cmp( name, htown->filename ) ) /* for some oddball reason... */
			return htown;
	}
	return NULL;
}

/*
 * Hometown Set 		-Nopey
 * 6/16/01
 */
void do_sethtown( CHAR_DATA *ch, char *argument )
{
	HOMETOWN_DATA *htown;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	/* Wouldn't that be nice? */
	if( IS_NPC(ch) )
	return;

	if( !IS_IMMORTAL(ch) )	
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	
	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: sethtown <hometown> <field> <value>\n\r", ch );
		send_to_char( "Syntax: sethtown create <hometown>\n\r", ch );
		send_to_char( "Syntax: sethtown save\n\r", ch );
		send_to_char( "Fields being one of the following:\n\r"
			      " name recall filename\n\r", ch );
		return;
	}

	/* this should be fine */
	if( !str_cmp( arg, "save" ) )
	{
		for( htown = first_hometown; htown; htown = htown->next )
		/*	save_hometowns( htown ); */

		save_hometown_list( ); /* write the damn list also */
		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* create one baby! */
	if( !str_cmp( arg, "create" ) )
	{
		if( arg2[0] == '\0' )
		{
			send_to_char( "Syntax: hometownset create <name>\n\r", ch );
			return;
		}
		htown = hometown_lookup( arg2 );
		if( htown )
		{
			send_to_char( "There is already a hometown with that name!\n\r", ch );
			return; 
		}

		CREATE( htown, HOMETOWN_DATA, 1 );
		LINK( htown, first_hometown, last_hometown, next, prev );
		htown->name	= STRALLOC( arg2 );
		htown->filename = str_dup( "" );
		htown->recall	= 0;
		send_to_char( "Done.\n\r", ch );
		send_to_char( "Remember to save.\n\r", ch );
		return;
	}
		
	if(!(htown = hometown_lookup( arg )))
	{
		send_to_char( "That is not a hometown!\n\r", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		send_to_char( "Syntax: sethtown <hometown> <field> <value>\n\r", ch );
		send_to_char( "Syntax: sethtown create <hometown>\n\r", ch );
		send_to_char( "Syntax: sethtown save\n\r", ch );
		send_to_char( "Fields being one of the following:\n\r"
			      " name recall filename\n\r", ch );
		return;
	}

	/* change filename */
	if( !str_cmp( arg2, "filename" ) )
	{
		if( !is_number( argument ) )
		{
			STRFREE( htown->filename );
			htown->filename = str_dup( argument );
			send_to_char( "Hometown filename renamed.\n\r", ch );
			return;
		} else {
			send_to_char( "Characters only, no numbers.\n\r", ch );
			return;
		}
	}		

	/* change the name */
	if( !str_cmp( arg2, "name" ) )
	{
		if( !is_number( argument ) )
		{
			htown->name = argument;
			send_to_char( "Hometown renamed.\n\r", ch );
			return;
		} else {
			send_to_char( "Characters only, no numbers.\n\r", ch );
			return;
		}
	}

	/* set/change recall */
	if( !str_cmp( arg2, "recall" ) )
	{
		if( !is_number( argument ) )
		{
			send_to_char( "Recall VNUM must be numbers.\n\r", ch );
			return;
		} else {
			htown->recall = atoi( argument );
			send_to_char( "Recall changed.\n\r", ch );
			return;
		}	
	}
	return; 	/* just in case missed it somewhere... */
}

/*
 * Listhometowns		-Nopey
 * 6/21/01
 */
void do_hometowns( CHAR_DATA *ch, char *argument )
{
	HOMETOWN_DATA *htown;
	
	/* I suggest you give this to immortals only */
	if( !IS_IMMORTAL(ch) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	pager_printf_color( ch, "&g---------------------------------------------------------\n\r" );
	pager_printf_color( ch, "&G&W			 Loaded Hometowns\n\r", ch );
	pager_printf_color( ch, "Name		Recall		Filename	  Race\n\r" );
	pager_printf_color( ch, "&g---------------------------------------------------------\n\r" );
	for( htown = first_hometown; htown; htown = htown->next )
	{
		pager_printf_color( ch, "&G%-14s %-15d %s\n\r", capitalize( htown->name ), htown->recall, htown->filename );
	}
	return;
}

/*
 * MPHTOWNSET		-Nopey
 */
void do_mphtownset( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	HOMETOWN_DATA *htown;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );

	if( !IS_NPC(ch) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}
	
	if( argument[0] == '\0' || arg[0] == '\0' )
	{
		send_to_char( "What?\n\r", ch );
		return;
	}

	if( (victim = get_char_room( ch, arg ) ) == NULL )
	{
		bug( "mphtownset: no victim" );
		return;
	}

	htown = hometown_lookup( argument );

	if( !htown )
	{
		bug( "mphtownset: invalid hometown" );
		return;
	}

	if( IS_NPC(victim) )
	return;

	victim->pcdata->hometown	= htown;
	victim->pcdata->htown_name	= STRALLOC( htown->name );
	return;
}

void sethtown( CHAR_DATA *ch, char *argument )
{
	HOMETOWN_DATA *htown;

	if( !IS_NPC(ch) )
	{
		bug( "sethtown: not on npc" );
		return;
	}
	
	if( argument[0] == '\0' )
	{
		bug( "sethtown: no argument" );
		return;
	}

	htown = hometown_lookup( argument );

	if( !htown )
	{
		bug( "mphtownset: invalid hometown" );
		return;
	}

	ch->pcdata->hometown	= htown;
	ch->pcdata->htown_name	= STRALLOC( htown->name );
	return;
}

/*
 * free_hometown		-Nopey
 */
void free_hometown( HOMETOWN_DATA *htown )
{
	STRFREE( htown->name );
	STRFREE( htown->filename );  /* Fellon */
	UNLINK( htown, first_hometown, last_hometown, next, prev );
	DISPOSE( htown );
	return;
}

/*
=======================
Write the Hometown list 	       -Nopey
=======================
*/
void write_hometown_list( DESCRIPTOR_DATA *d )
{
  HOMETOWN_DATA *htown = NULL;
  char buf[MAX_INPUT_LENGTH];

  for( htown = first_hometown; htown; htown = htown->next )
  {
    sprintf( buf, "[%s]\n\r", htown->name );
    write_to_buffer( d, buf, 0 );
  }
  return;
}
