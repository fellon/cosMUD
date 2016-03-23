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
 *  ResortMUD Arena Code
 *  Author: Scion (Scion@usa.net)
 *  of ResortMUD (ucmm.dhs.org:4000)
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

void arena_add(int number, bool money);
int arena_death(CHAR_DATA *ch, CHAR_DATA *victim);
void arena_update(void); /* called by comm.c */

extern void save_sysdata  args( ( SYSTEM_DATA sys ) ); /* act_wiz.c in ShogTag */

const int MAX_ARENA_FEE=3000000; /* Set this to something reasonable */
const int ARENA_DEFAULT_PRIZE = 10000;

int arena_players=0; /* This keeps track of the number of players left in the arena */
int arena_total_players=0;
int arena_prize; /* This is always 500 coins, set it to be balanced on your MUD */
int arena_fee=0;     /* This is the entrance fee to get in, which can be set with arena start */
int low_level=0;	 /* Lowest level allowed in arena */
int hi_level=0;		 /* Highest level allowes in arena */
int arena_update_mode=0; /* Update is called once per minute, but we don't want to do something
							every minute. */

void do_arena( CHAR_DATA *ch, char *argument ) {
	char cmd[ MAX_INPUT_LENGTH ];
	char cmd2[ MAX_INPUT_LENGTH ];
	char cmd3[ MAX_INPUT_LENGTH ];
	char buf[ MAX_STRING_LENGTH ];
	CHAR_DATA *victim;
	AREA_DATA *arena;
	int hi_vnum = 0;
	int lo_vnum = 0;
	int real_vnum;
	int i;

	argument = one_argument( argument, cmd );

	set_char_color( AT_IMMORT, ch );

	if (IS_IMMORTAL(ch)) {
		if( !str_cmp( cmd, "start" ) ) {
			if (sysdata.arena >= 1) {
				send_to_char("There is already a battle in the arena!\r\n",ch);
				return;
			}
			argument = one_argument(argument, cmd); /* entrance fee */
			argument = one_argument(argument, cmd2); /* low level */
			argument = one_argument(argument, cmd3); /* high level */
			
			arena_prize=ARENA_DEFAULT_PRIZE;

			if (cmd[0] == '\0')
				arena_fee=0;
			else
				arena_fee=atoi(cmd);

			if (arena_fee < 0) /* Free matches are, of course, allowed and encouraged! */
				arena_fee = 0;
			if (arena_fee >= MAX_ARENA_FEE)
				arena_fee = MAX_ARENA_FEE;

			if (cmd2[0] == '\0')
				low_level=-1; /* Ignore level settings if not specified */
			else {
				low_level=atoi(cmd2);
				if (low_level>MAX_LEVEL)
					low_level=MAX_LEVEL;
				if (low_level<=0)
					low_level=-1;
			}

			if (cmd3[0] == '\0')
				hi_level=-1; /* Ignore level settings if not specified */
			else {
				hi_level=atoi(cmd3);
				if (hi_level>MAX_LEVEL)
					hi_level=MAX_LEVEL;
				if (hi_level<=0)
					hi_level=-1;
			}
			
			if (hi_level < low_level) { /* If they're backward, reverse them */
				i=hi_level;
				hi_level=low_level;
				low_level=i;
			}


                        sysdata.arena=2;			
			save_sysdata( sysdata );

			arena_players=0;
			arena_total_players=0;

			/* Display the appropriate start messages */
			if (hi_level>0 && low_level==-1)
				sprintf(buf, "%s has ^zOPENED^x the arena with a MAX level of %d!",ch->name, hi_level);
			else if (low_level>0 && hi_level==-1)
				sprintf(buf, "%s has ^zOPENED^x the arena with a MIN level of %d!",ch->name, low_level);
			else if (low_level == -1 && hi_level == -1)
				sprintf(buf, "%s has ^zOPENED^x the arena!", ch->name);
			else
				sprintf(buf,"%s has ^zOPENED^x the arena for levels %d through %d!", ch->name, low_level, hi_level);
			do_info(ch, buf);
			if (arena_fee) {
				sprintf(buf,"The entrance fee is %d coins!", arena_fee);
				do_info(ch, buf);
			}
			do_info(ch, "Type \'arena join\' to join!");
                         
			set_char_color( AT_IMMORT, ch );
			send_to_char( "Ready.\n\r", ch );
			return;
		} else if ( !str_cmp( cmd, "end" ) ) {
			if( sysdata.arena == 0) {
				send_to_char( "There is not currently an arena battle to end.\n\r", ch );
				return;
			}

			sprintf(buf, "%s has canceled the arena match!",ch->name);
			do_info(ch, buf);
			do_info(ch,  "The arena is ^zCLOSED^x.");

			for( victim = first_char; victim ; victim = victim->next ) {
				if( IS_NPC(victim) )
					continue;
				if( victim->arena!=2 )
					continue;
				/* remove recent fight timer */
				if( get_timer( victim, TIMER_RECENTFIGHT ) )
					remove_timer( victim, TIMER_RECENTFIGHT);

				/* make everyone STANDING (for all peace, etc) */
				stop_fighting( victim, TRUE );
				victim->position = POS_STANDING;

				if (arena_fee) {
					send_to_char("Your entrance fee has been returned to you.\r\n",victim);
					victim->gold+=arena_fee;
				}
				act( AT_MAGIC, "$n disappears in blinding flash of light.", victim, NULL, NULL, TO_ROOM );
				char_from_room( victim );
				char_to_room( victim, get_room_index(victim->retran) );
				act( AT_MAGIC, "$n arrives in a blinding flash of light.", victim, NULL, NULL, TO_ROOM );
				do_look( victim, "auto" );
				victim->arena=0;
			}

			sysdata.arena = 0;
			save_sysdata( sysdata );

			set_char_color( AT_IMMORT, ch );
			send_to_char( "Done.\n\r", ch );
			return;
		} else if (!str_cmp(cmd, "who")) {
			send_to_char("\r\nWho is in the arena:\r\n",ch);
			for( victim = first_char; victim ; victim = victim->next ) {
				if( IS_NPC(victim) )
					continue;
				if( victim->arena==0 )
					continue;
				switch (victim->arena) {
				case 2:
					sprintf(cmd, "Fighting");
					break;
				case 3:
					sprintf(cmd, "Defeated");
					break;
				case 4:
					sprintf(cmd, "Spectator");
					break;
				default:
					sprintf(cmd, "Unknown");
					break;
				}
				sprintf(buf, "%15s: %s\r\n", victim->name, cmd);
				send_to_char(buf, ch);
			}
			return;
		}
	} /* if IS_IMMORTAL(ch) */
	
	if ((!IS_IMMORTAL(ch)) || (sysdata.imm_arena==TRUE)) {
		/******* JOIN *******/
		if ( !str_cmp( cmd, "join" ) ) {
			if (ch->arena==2) {
				send_to_char( "You are already in the arena!\r\n", ch);
				return;
			}
			if (sysdata.arena==0) {
				send_to_char( "The arena is currently closed. Ask an immortal to open it!\r\n", ch);
				return;
			}
			if ((ch->gold < arena_fee) && !IS_IMMORTAL(ch)) {
				sprintf(buf, "You do not have the %d coins to join this arena match!\r\n",arena_fee);
				send_to_char(buf,ch);
				return;
			}
			if (ch->arena==3) {
				send_to_char("You were already defeated in this round!\r\n",ch);
				return;
			}
			if (ch->arena==4) {
				send_to_char("You are watching a player in this round, you cannot participate.\r\n",ch);
				return;
			}
			if (ch->level<low_level && low_level != -1) {
				send_to_char("You are not of a high enough level to compete in this match!\r\n",ch);
				return;
			}
			if (ch->level>hi_level && hi_level != -1) {
				send_to_char("Leave those low level players to bicker amongst themselves.\r\n",ch);
				return;
			}
			if (ch->fighting) {
				send_to_char("You had better stop fighting before entering the arena!\r\n",ch);
				return;
			}
			/* Trespin's anti-cheat fix. -Marten */
			if (((current_time-ch->logon) / 60) <= 5) {
				send_to_char("You have to wait 5 minutes after logon before you can join the arena.\r\n",ch);
				return;
			}

			/* Put them in a random room inside arena.are */
			for ( arena = first_area; arena; arena = arena->next ) {
				if ( !str_cmp( arena->filename, "arena.are" ) ) {
					lo_vnum=arena->low_r_vnum;
					hi_vnum=arena->hi_r_vnum;
					break;
				} else {
					lo_vnum=2;
					hi_vnum=2;
				}
			}	
	
			if ((lo_vnum==2) && (hi_vnum==2)) {
				bug("Could not find arena.are!",0);
				send_to_char("An error ocurred while moving you to the arena.\r\nThe administrators have been notified of the error.\r\n",ch);
				return;
			}
			
			i=0;
			while (!get_room_index(real_vnum=number_range(lo_vnum, hi_vnum - 1))) {
				if (i++ >= 50) {
					bug("Cannot find a room in the arena for %s!", ch->name);
					send_to_char("An error ocurred while moving you to the arena.\r\nThe administrators have been notified of the error.\r\n",ch);
					return;
				}
			}	

			stop_fighting( ch, TRUE );
			ch->position = POS_STANDING;
			act( AT_MAGIC, "$n disappears in a blinding flash of light.", ch, NULL, NULL, TO_ROOM );
		    ch->retran = ch->in_room->vnum;
		    char_from_room( ch );

			char_to_room( ch, get_room_index(real_vnum));
			
		    /* Take their money and put it in the pot */
			if (arena_fee) {
				if (IS_IMMORTAL(ch)) {
					sprintf(buf, "Bill Gates donates %d coins for %s\'s entrance fee!",arena_fee, ch->name);
					do_info(ch, buf);
				} else
					ch->gold-=arena_fee;
				arena_prize+=arena_fee;
			}

			act( AT_MAGIC, "$n arrives in a blinding flash of light.", ch, NULL, NULL, TO_ROOM );
			do_look( ch, "auto" );
			send_to_char("You have entered the arena!\r\n",ch);
			sprintf(buf, "%s has joined the arena match!", ch->name);
			do_info(ch, buf);
			ch->arena=2;
			arena_add(1,FALSE);
			return;
		}
		/******** WIMPOUT ********/
		else if ( !str_cmp( cmd, "wimpout" ) ) {
			if (ch->arena!=2) {
				send_to_char( "You are not in the arena!\r\n", ch);
				return;
			}

			if (ch->fighting) {
				send_to_char("You had better stop fighting before you do that!\r\n", ch);
				return;
			}
			if (arena_fee>0)
				send_to_char("By wimping out, you forfeit your entrance fee!\r\n",ch);
			stop_fighting(ch, TRUE);	
			ch->position = POS_STANDING;
			act( AT_MAGIC, "$n disappears in a blinding flash of light.", ch, NULL, NULL, TO_ROOM );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index(ch->retran) );
			act( AT_MAGIC, "$n arrives in a blinding flash of light.", ch, NULL, NULL, TO_ROOM );
			do_look( ch, "auto" );
			sprintf(buf, "%s has wimped out of the arena match!", ch->name);
			do_info(ch, buf);
			ch->arena=0;
			arena_add(-1,FALSE);
			return;
		}
		/****** NO VALID COMMAND ******/
		else {
			/* If we got no valid arguments */
			send_to_char( "Syntax:\r\n", ch);
			if (!IS_IMMORTAL(ch) || (sysdata.imm_arena==TRUE)) {
				if (sysdata.arena > 0)
					send_to_char( "arena join\r\n",ch);
				if (ch->arena==2)
					send_to_char( "arena wimpout\r\n",ch);
			}
			if (IS_IMMORTAL(ch)) {
				if (sysdata.arena==0) {
					send_to_char( "\r\narena start [fee] [low] [high]\r\n", ch);
				}
				if (sysdata.arena > 0)
					send_to_char( "arena end\r\n", ch);
				send_to_char( "arena who\r\n",ch);
			}
			switch (sysdata.arena) {
			case 0:
				send_to_char("\r\nThe arena is closed.\r\n",ch);
				break;
			case 2:
				sprintf(buf, "\r\nThere are %d combatants in the arena.\r\n", arena_players);
				send_to_char(buf,ch);
				break;
			} /* switch */
		} /* else */
	} /* if !IS_IMMORTAL(ch) */
} /* function */

void arena_update(void) {
	char buf[MAX_STRING_LENGTH];
	
	switch(sysdata.arena) {
	case 0: /* arena is off */
		arena_update_mode=0;
		return;
		break;
	case 2:	/* arena is in free for all mode */	
		if (arena_fee > 0)
			sprintf(buf, "Now in the arena: levels %d to %d, entry fee only %d coins!",((low_level>0) ? low_level : 1), ((hi_level>0) ? hi_level : MAX_LEVEL), arena_fee);
		else
			sprintf(buf, "Now in the arena: Free for All, levels %d to %d!", ((low_level>0) ? low_level : 1), ((hi_level>0) ? hi_level : MAX_LEVEL));
		do_info(first_char, buf);
		break;
	default:
		sysdata.arena=0;
		bug("Default case reached in arena_update!",0);
		break;
	}
}

void arena_add(int number, bool money) {
 /* This adds "number" players to arena_players and checks win conditions. */

	char buf[ MAX_STRING_LENGTH ];
	CHAR_DATA *ch;

	arena_players+=number;
	if (arena_players>arena_total_players)
		arena_total_players=arena_players;

	if ((sysdata.arena == 2) && (arena_players <= 1) && (number <= 0)) {  /* someone won */
		for( ch = first_char; ch ; ch = ch->next ) {	/* if number is positive, we've just added */         
				if( IS_NPC(ch) )						/* a player. How could they have just won? */
					continue;
				switch( ch->arena ) {
				case 0: /* Character's not in, and hasn't been in the arena */
					continue;
					break;
				case 2: /* Character has won, being the last one left alive in arena */
					if (money==TRUE) {
						sprintf(buf, "%s has ^zWON^x the arena match, and ^z%d^x coins!", ch->name, arena_prize);
						do_info(ch, buf);
						ch->gold+=arena_prize;
						ch->arena_wins++;
						adjust_hiscore("arena_pkill", ch, ch->arena_wins);
					} else
						do_info(ch, "The arena match has been closed without a winner.");
					stop_fighting(ch, TRUE);
					ch->position=POS_STANDING;
					act( AT_MAGIC, "$n disappears in a golden beam of light.", ch, NULL, NULL, TO_ROOM);	
	
					ch->hit = ch->max_hit;
					ch->mana = ch->max_mana;
					ch->move = ch->max_move;
					ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level);
					update_pos(ch);
					send_to_char("You have been fully restored!\r\n",ch);
	
					char_from_room( ch );
					char_to_room( ch, get_room_index(ch->retran));
					act( AT_MAGIC, "$n appears in a golden beam of light.", ch, NULL, NULL, TO_ROOM);
					do_look( ch, "auto" );
					ch->arena=0;				
					break;
				case 3: /* Character died in arena */
					ch->arena=0;
					break;
				case 4: /* Character is watching a participant */
					ch->arena=0;
					break;
				}
		}
		arena_prize=ARENA_DEFAULT_PRIZE;
		sysdata.arena=0;
		save_sysdata( sysdata );
		arena_players=0;
	}
}

int arena_death(CHAR_DATA *ch, CHAR_DATA *victim) {
	char buf[MAX_STRING_LENGTH];
	
	/* If they were in the arena, override death! */
	if (victim->arena==2) {
		act(AT_DEAD, "$n has been DEFEATED!!", victim, 0, 0, TO_ROOM);
		act(AT_DEAD, "You have been DEFEATED!", victim, 0, 0, TO_CHAR);
		sprintf( buf, "%s has been defeated by %s in the arena!", victim->name, ch->name);
		do_info( victim, buf );
		sprintf( buf, "%s (%d) killed by %s in arena at %d",
			victim->name,
			victim->level,
			(IS_NPC(ch) ? ch->short_descr : ch->name),
			victim->in_room->vnum );
		log_string( buf );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
		ch->arena_kills++;		
		victim->arena_deaths++;

		/* Now that that fiasco is over, retran and restore the victim */
		stop_fighting(victim, TRUE);
		victim->position=POS_STANDING;
		act( AT_MAGIC, "$n disappears in a beam of holy light.", victim, NULL, NULL, TO_ROOM);
		victim->hit = victim->max_hit;
		victim->mana = victim->max_mana;
		victim->move = victim->max_move;
		victim->pcdata->condition[COND_BLOODTHIRST] = (10 + victim->level);
		update_pos(victim);
		send_to_char("You have been fully restored!\r\n",victim);

		char_from_room( victim );
		char_to_room( victim, get_room_index(victim->retran));
		act( AT_MAGIC, "$n falls delicately out of the sky.", victim, NULL, NULL, TO_ROOM);
		do_look( victim, "auto" );
		victim->arena=3; /* 0==no arena, 1==betting, 2==fighting, 3==dead, 4==watching */
		arena_add(-1,TRUE); /* Reduce the player count by one */
		return 0;
	}
	return 1;
}
