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
/*-relgar added vars-------- */
char lastfilename[ MAX_INPUT_LENGTH ];
char thisfilename[ MAX_INPUT_LENGTH ];
int arena_hi_vnum = 0;    /* made these 2 vars global and added "arena_" in front */
int arena_low_vnum = 0;   /* and optimized order analysis on the code segment that fills them */

/* i did global search and replace "lo_vnum" --> "arena_low_vnum", and "hi_vnum" --> "arena_hi_vnum" 
     (no point in documenting EVERY occurance, when search and replace works)
*/

int arena_place_prize=0; 
int arena_place_percent=0; 

int arena_show_prize=0; 
int arena_show_percent=0; 

int arena_kill_percent=0;     
int arena_fee_level_exponent=0;

void do_arena( CHAR_DATA *ch, char *argument ) {
	char cmd[ MAX_INPUT_LENGTH ];
	char cmd2[ MAX_INPUT_LENGTH ];
	char cmd3[ MAX_INPUT_LENGTH ];
	char buf[ MAX_STRING_LENGTH ];
	CHAR_DATA *victim;
	AREA_DATA *arena;

/* relgar moved 2 local vars "lo and hi vnum" up and added "arena_" in front */

	int real_vnum;
	int i;

	argument = one_argument( argument, cmd );

/* BEGIN pot add (ie. arena add coins $$ $$ $$ ) adds to win, place, show pots*/	
	if ( !str_cmp( cmd, "add" ) ) {
/*pot add*/	if (sysdata.arena==0) {
			send_to_char( "The arena is currently closed. Ask an immortal to open it!\r\n", ch);
			return;
		}
		argument = one_argument( argument, cmd );
/*pot add  FUTURE -- non money prize gets added here (arena_add will pay it out)*/		

/*pot add*/
		if ( !str_cmp( cmd, "coins" ) || !str_cmp( cmd, "cash" ) ) { int x,y,z;
			argument = one_argument( argument, cmd );
			argument = one_argument( argument, cmd2);
/*pot add*/		argument = one_argument( argument, cmd3);
			x = atoi(cmd);
			y = atoi(cmd2);
/*pot add*/		z = atoi(cmd3);
			if (x < 0) x=0;
			if (y < 0) y=0;
/*pot add*/		if (z < 0) z=0;

			if (!IS_IMMORTAL(ch))
				if (ch->gold < (x+y+z)) 
/*pot add*/				return;
				else
					ch->gold-=(x+y+z);
/*pot add*/
			arena_show_prize+=z; 
			arena_place_prize+=y;
			arena_prize+=x;
/*pot add*/
			sprintf(buf, "%s just donated %d coins to the next arena winner, %d%s,%d%s!\r\n",TEMP, ch->name, x,y, ((y>0) ?" To 2nd place" :"") ,z, ((z>0)?" To 3rd place":""));
			do_info(ch, buf);
		}
		return;
/* END pot add  (the whole code segment)*/
	}


/* relgar alt arena */
	if( !str_cmp( cmd, "list" ) ) {
	
		AREA_DATA *def;
		int x;
		char st_code[ MAX_INPUT_LENGTH ];
/* list alt arenas */
		if (IS_IMMORTAL(ch)) {
			for ( arena = first_area; arena; arena = arena->next ) {
				if ( !str_N_cmp( arena->filename, "arena", 5 ) ) {
					if (strlen(arena->filename) == 9)
/* list alt arenas */			def = arena;
					else {
						strcpy(st_code,&arena->filename[4]);
						x = strlen(st_code);
						st_code[0] = st_code[x-4] = '\"';			
						st_code[x-3] = '\0';			
/* list alt arenas */
						ch_printf(ch, "%-10s |%-7d |%-20s |%-40s\r\n", 													"st.code","r.count","filename","arena_name");
						ch_printf(ch, "%-10s |%-7d |%-20s |%-40s\r\n", 
							st_code, (arena->hi_r_vnum - arena->low_r_vnum), arena->filename, arena->name);
/* list alt arenas */						
					}
				} 
			}	
		ch_printf(ch, "%-10s |%-7s |%-20s |%-40s\r\n", 	"---------","------","---------","---------");
		ch_printf(ch, "%-10s |%-7d |%-20s |%-40s\r\n", " default:", 
			(arena->hi_r_vnum - arena->low_r_vnum), arena->filename, arena->name);
/* list alt arenas */
		}
		else 
			for ( arena = first_area; arena; arena = arena->next ) 
				if ( !str_N_cmp( arena->filename, "arena" ) ) {
						ch_printf(ch, "%s\r\n", "arena_name");
						ch_printf(ch, "%s\r\n", arena->name);
/* list alt arenas */	} 
		return;
	}
/*  END list alt arenas */

/* BEGIN alt arena */
if (arena_hi_vnum == 0)
	lastfilename[0] = '\0';
/* alt arena */

if (cmd[0] == '"'){
	int x;
	if (cmd[x= (strlen(cmd) - 1)] == '"')
		cmd[x] = '\0';
	sprintf(thisfilename, "arena%s.are", &cmd[1]);
	argument = one_argument( argument, cmd );
	}
	else 	strcpy(thisfilename, "arena.are");
/* end alt arena code chunk */

/* relgar alt arena  -- above makes filename, then refills the cmd for further processing*/

	set_char_color( AT_IMMORT, ch );

	if (IS_IMMORTAL(ch)) {
		if( !str_cmp( cmd, "start" ) ) {
			if (sysdata.arena >= 1) {
				send_to_char("There is already a battle in the arena!\r\n",ch);
				return;
			}

/* relgar alt arena */
		if (strcmp(thisfilename, lastfilename) !=0 ){
			lastfilename[0] = '\0';
			arena_low_vnum=2;
			arena_hi_vnum=2;
/* alt arena */
			for ( arena = first_area; arena; arena = arena->next ) {
				if ( !str_cmp( arena->filename, thisfilename ) ) {
					arena_low_vnum=arena->low_r_vnum;
					arena_hi_vnum=arena->hi_r_vnum -1;        /* i subtracted one from hi here */
					break;						/* keep mortals out of control rooms */
/* alt arena */		} 
			}	
	
			if ((arena_low_vnum==2) && (arena_hi_vnum==2)) {
				sprintf(buf,"Could not find %s!\r\n", thisfilename);
				send_to_char(buf,ch);
				return;
/* alt arena */	}
			strcpy(lastfilename, thisfilename);
		}
/* relgar alt arena */

			argument = one_argument(argument, cmd); /* entrance fee */
/* kill percent */if (cmd[0] == 'k') {
				arena_kill_percent=atoi(&cmd[1]);
				argument = one_argument(argument, cmd); 
			}
			else arena_kill_percent=0;     
/* place percent */if (cmd[0] == 'p') {
				arena_place_percent=atoi(&cmd[1]);
				argument = one_argument(argument, cmd); 
			}
			else arena_place_percent=0;     
/* show percent */if (cmd[0] == 's') {
				arena_show_percent=atoi(&cmd[1]);
				argument = one_argument(argument, cmd); 
			}
			else arena_show_percent=0; 


			argument = one_argument(argument, cmd2); /* low level */
			argument = one_argument(argument, cmd3); /* high level */
			
			arena_prize=ARENA_DEFAULT_PRIZE;

/* relgar fee NOW supports "l"'s at begining (for each "l" multiply fee number by player's 
level to get how much it costs to enter. ie. ll500 = level^2 * 500  (50k for level 10, 1250k for level 50)*/
/* l fee */		arena_fee_level_exponent=0;
			if (cmd[0] == '\0')
				arena_fee=0;
			else
				{
/* l fee */			for ( ; (cmd[arena_fee_level_exponent]=='l' || cmd[arena_fee_level_exponent]=='L') ; arena_fee_level_exponent++);
/* l fee */			arena_fee=atoi(&cmd[arena_fee_level_exponent]);
				}
/* relgar end l fee edits in start  (see join)*/

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
/* l fee */		int TEMP;
			if (ch->arena==2) {
				send_to_char( "You are already in the arena!\r\n", ch);
				return;
			}
			if (sysdata.arena==0) {
				send_to_char( "The arena is currently closed. Ask an immortal to open it!\r\n", ch);
				return;
			}
/* l fee */		TEMP = (ch->gold^arena_fee_level_exponent) * arena_fee;
/* l fee */		if (TEMP >= MAX_ARENA_FEE)  TEMP = MAX_ARENA_FEE;

/* l fee */		if ((ch->gold < TEMP) && !IS_IMMORTAL(ch)) {
/* l fee */			sprintf(buf, "You do not have the %d coins to join this arena match!\r\n",TEMP);
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
			
			/* Put them in a random room inside arena.are */

/* reglar comment out this section 
1. it should be known before a player joins is the arena exists
2. this code should happen LESS frequently than every player joining
3. this is inefficent code (setting vars to a constant in an else condition inside a for loop)
4. changed this code to support multiple arenas

			for ( arena = first_area; arena; arena = arena->next ) {
				if ( !str_cmp( arena->filename, "arena.are" ) ) {
					arena_low_vnum=arena->low_r_vnum;
					arena_hi_vnum=arena->hi_r_vnum;
					break;
				} else {
					arena_low_vnum=2;
					arena_hi_vnum=2;
				}
			}	
	
			if ((arena_low_vnum==2) && (arena_hi_vnum==2)) {
				bug("Could not find arena.are!",0);
				send_to_char("An error ocurred while moving you to the arena.\r\nThe administrators have been notified of the error.\r\n",ch);
				return;
			}
relgar -- comment out this section code moved to arena start  */
			
			i=0;
			while (!get_room_index(real_vnum=number_range(arena_low_vnum, arena_hi_vnum))) {
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
/* l fee */				sprintf(buf, "Bill Gates donates %d coins for %s\'s entrance fee!",TEMP, ch->name);
					do_info(ch, buf);
				} else {
/* l fee */				ch->gold-=TEMP;
/* l & place fee */		arena_show_prize+= (TEMP*arena_place_percent)/100;
/* l & show fee */		arena_place_prize+=(TEMP*arena_show_percent)/100;
/* l & main fee */		arena_prize+=TEMP -(((TEMP*arena_kill_percent)+(TEMP*arena_place_percent)+(TEMP*arena_show_percent))/100);
/* N players == n-1 kills */	if (arena_players == 0) arena_prize+=(TEMP*arena_kill_percent)/100;
				}
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
			int TEMP;
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

/* kill fee won't go to a player, so add it to pot */
/* l fee */		TEMP = (ch->level^arena_fee_level_exponent) * arena_fee;
/* l fee */		if (TEMP >= MAX_ARENA_FEE)  TEMP = MAX_ARENA_FEE;
/* l fee */		arena_prize+=(TEMP*arena_kill_percent)/100;
		
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
/* l fee */		{if (arena_fee_level_exponent < 2)
/* l fee */			sprintf(buf, "Now in the arena: levels %d to %d, entry fee only%s %d coins!",((low_level>0) ? low_level : 1), ((hi_level>0) ? hi_level : MAX_LEVEL), ((arena_fee_level_exponent==1) ? " (player level) *" : "") , arena_fee);
/* l fee */		else
/* l fee */			sprintf(buf, "Now in the arena: levels %d to %d, entry fee (player level^%d)* %d coins!",((low_level>0) ? low_level : 1), ((hi_level>0) ? hi_level : MAX_LEVEL), arena_fee_level_exponent, arena_fee);
/* l fee */		}
		
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
/* if only 2 entered winner gets */
/* 3rd place prize also */		ch->gold+=arena_prize+arena_show_prize;
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
	
/* relgar money per kill (it's an imm selected percent of the entry fee)*/
if (arena_kill_percent > 0)
{int TEMP;
/* l fee */	TEMP = (victim->level^arena_fee_level_exponent) * arena_fee;
/* l fee */	if (TEMP >= MAX_ARENA_FEE)  TEMP = MAX_ARENA_FEE;
		TEMP=(TEMP*arena_kill_percent)/100;
/* l fee */	ch->gold+=TEMP;
sprintf( buf, "%s has earned %d for an arena kill", ch->name, TEMP);
do_info( victim, buf );
}

/* relgar 3rd place pot (it's an imm selected percent of the entry fee)*/
if ((arena_show_prize > 0) && (arena_players == 3))
{
victim->gold+=arena_show_prize;
sprintf( buf, "%s has earned %d for A 3rd place in the arena", victim->name, arena_show_prize);
do_info( victim, buf );
arena_show_prize = 0;
}

/* relgar award 2nd place pot (it's an imm selected percent of the entry fee)*/
if ((arena_place_prize > 0) && (arena_players == 2))
{
victim->gold+=arena_place_prize;
sprintf( buf, "%s has earned %d for THE 2nd place in the arena", victim->name, arena_place_prize);
do_info( victim, buf );
arena_place_prize = 0;
}
/* relgar */

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
