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
 Here is what my logf looks like: 
 I like those variable argument functions, and have printf_to_char,
 bugf, etc. as well :) 
 
 Oh, and MSL = MAX_STRING_LENGTH (MIL = MAX_INPUT_LENGTH).
 
*/

void logf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	log_string (buf);
}


/* This is the handy CH() macro. I think that it was Tom Adriansen (sp?) */

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)



/*

You need to define:

COPYOVER_FILE - temporary data file used
EXE_FILE      - file to be exec'ed (i.e. the MUD)


Note that I advance level 1 chars to level 2 - this is necessary in MERC and
Envy, but I think that ROM saves level 1 characters too.

Note that you might want to change your close_socket() a bit. I have changed
the connected state so that negative states represent logging-in, while as
positive ones represent states where the character is already inside the game.
close_socket() frees that chararacters with negative state, but just loses
link on those with a positive state. I believe that idea comes from Elwyn
originally.

Things to note: This corresponds to a reboot, followed by the characters
logging in again. This means that stuff like corpses, disarmed weapons etc.
are lost, unless you save those to the pfile. You should probably give the
players some warning before doing a copyover.

The command was inspired by the discussion on merc-l about how Fusion's MUD++
could reboot without players having to re-login :)

*/


extern int port,control; /* db.c */

void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		logf ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}
	
	/* Consider changing all saved areas here, if you use OLC */
	
	/* do_asave (NULL, ""); - autosave changed areas */
	
	
	sprintf (buf, "\n\r&R[&WINFO&R] &cCOPYOVER initiated by %s, sorry for the inconvenience!\n\r", ch->name);
	
	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
			if (och->level == 1)
			{
				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++; /* Advance_level doesn't do that */
			}
			save_char_obj (och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
#ifdef I3
   if( I3_is_connected() )
   {
      /* Be sure the amount of numbers and strings match with the fprintf
       * statement in the loop above or you'll have serious problems
       */
      fprintf( fp, "%d Intermud-3 I3\n", I3_socket );
	I3_savemudlist();
	I3_savechanlist();
   }
#endif
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXE_FILE, "EnvyMUD", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);
	
	/* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	int desc;
	bool fOld;
	
	logf ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		logf ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
        if (desc == -1 || feof(fp) )
            break;

#ifdef I3
	if( !str_cmp( name, "Intermud-3" ) )
	{
	   I3_socket = desc;
	   continue;
	}
#endif

		/* Write something, and check if it goes error-free */		
		if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r",0))
		{
			close (desc); /* nope */
			continue;
		}
		
		d = alloc_perm (sizeof(DESCRIPTOR_DATA));
		init_descriptor (d,desc); /* set up various stuff */
		
		d->host = str_dup (host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
		
	
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name);
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
			write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0);
			write_to_descriptor (desc, "\n\rLoading mass quantities of areas... \n\rPerhaps a good time to go wash the car.\n\r",0);
	
			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room (d->character, d->character->in_room);
			do_look (d->character, "");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;
		}
		
	}
	
	fclose (fp);
}

