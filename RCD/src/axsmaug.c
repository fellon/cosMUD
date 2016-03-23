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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mud.h"
#include "axsmaug.h"

extern MOB_INDEX_DATA *mob_index_hash[];
extern OBJ_INDEX_DATA *obj_index_hash[];
extern ROOM_INDEX_DATA *room_index_hash[];
EXIT_DATA *pExit;
extern char * const ex_flags[];
extern char * const wear_locs[];
extern bool fBootDb;
extern int top_mob_index;
extern int top_obj_index;
extern int top_room;
extern int top_sn;
extern int top_herb;
extern int top_ed;
extern int top_affect;
extern int top_shop;
extern int top_repair;
extern int top_area;

int extmob[500],extobj[500],top_extmob,top_extobj,addmobrange,addobjrange;
int objnew[500],mobnew[500],newrange;
 
extern int 		mprog_name_to_type	args ( ( char* name ) );

struct sub_skill
{
	struct sub_skill *next;
	struct sub_skill *prev;
	char *from;
	char *to;
} *subskill,*firstsubskill,*lastsubskill;

struct sub_spell
{
	struct sub_spell *next;
	struct sub_spell *prev;
	char *from;
	char *to;
} *subspell,*firstsubspell,*lastsubspell;

struct sub_objflag
{
	struct sub_objflag *next;
	struct sub_objflag *prev;
	char *from;
	char *to;
} *subobjflag,*firstsubobjflag,*lastsubobjflag;

struct sub_mflag
{
	struct sub_mflag *next;
	struct sub_mflag *prev;
	char *from;
	char *to;
} *submobflag,*firstsubmobflag,*lastsubmobflag;

struct sub_aflag
{
	struct sub_aflag *next;
	struct sub_aflag *prev;
	char *from;
	char *to;
} *subareaflag,*firstsubareaflag,*lastsubareaflag;

struct sub_affected
{
	struct sub_affected *next;
	struct sub_affected *prev;
	char *from;
	char *to;
} *subaffected,*firstsubaffectedflag,*lastsubaffectedflag;

struct sub_mobrace
{
	struct sub_mobrace *next;
	struct sub_mobrace *prev;
	char *from;
	char *to;
} *submobrace,*firstsubmobrace,*lastsubmobrace;

struct sub_mobclass
{
	struct sub_mobclass *next;
	struct sub_mobclass *prev;
	char *from;
	char *to;
} *submobclass,*firstsubmobclass,*lastsubmobclass;

struct sub_moblang
{
	struct sub_moblang *next;
	struct sub_moblang *prev;
	char *from;
	char *to;
} *submoblang,*firstsubmoblang,*lastsubmoblang;

struct sub_mobpos
{
	struct sub_mobpos *next;
	struct sub_mobpos *prev;
	char *from;
	char *to;
} *submobpos,*firstsubmobpos,*lastsubmobpos;

struct sub_sector
{
	struct sub_sector *next;
	struct sub_sector *prev;
	char *from;
	char *to;
} *subsector,*firstsubsector,*lastsubsector;

struct sub_bodypart
{
	struct sub_bodypart *next;
	struct sub_bodypart *prev;
	char *from;
	char *to;
} *subbodypart,*firstsubbodypart,*lastsubbodypart;

struct sub_risflag
{
	struct sub_risflag *next;
	struct sub_risflag *prev;
	char *from;
	char *to;
} *subrisflag,*firstsubrisflag,*lastsubrisflag;

struct sub_attack
{
	struct sub_attack *next;
	struct sub_attack *prev;
	char *from;
	char *to;
} *subattack,*firstsubattack,*lastsubattack;

struct sub_defense
{
	struct sub_defense *next;
	struct sub_defense *prev;
	char *from;
	char *to;
} *subdefense,*firstsubdefense,*lastsubdefense;

struct sub_objtype
{
	struct sub_objtype *next;
	struct sub_objtype *prev;
	char *from;
	char *to;
} *subobjtype,*firstsubobjtype,*lastsubobjtype;

struct sub_wearflag
{
	struct sub_wearflag *next;
	struct sub_wearflag *prev;
	char *from;
	char *to;
} *subwearflag,*firstsubwearflag,*lastsubwearflag;
struct sub_contflag
{
	struct sub_contflag *next;
	struct sub_contflag *prev;
	char *from;
	char *to;
} *subcontflag,*firstsubcontflag,*lastsubcontflag;

struct sub_wearlayerflag
{
	struct sub_wearlayerflag *next;
	struct sub_wearlayerflag *prev;
	char *from;
	char *to;
} *subwearlayerflag,*firstsubwearlayerflag,*lastsubwearlayerflag;

struct sub_liquid
{
	struct sub_liquid *next;
	struct sub_liquid *prev;
	char *from;
	char *to;
} *subliquid,*firstsubliquid,*lastsubliquid;

struct sub_herb
{
	struct sub_herb *next;
	struct sub_herb *prev;
	char *from;
	char *to;
} *subherb,*firstsubherb,*lastsubherb;

struct sub_trigger
{
	struct sub_trigger *next;
	struct sub_trigger *prev;
	char *from;
	char *to;
} *subtrigger,*firstsubtrigger,*lastsubtrigger;

struct sub_traptype
{
	struct sub_traptype *next;
	struct sub_traptype *prev;
	char *from;
	char *to;
} *subtraptype,*firstsubtraptype,*lastsubtraptype;

struct sub_trapflag
{
	struct sub_trapflag *next;
	struct sub_trapflag *prev;
	char *from;
	char *to;
} *subtrapflag,*firstsubtrapflag,*lastsubtrapflag;

struct sub_weapontype
{
	struct sub_weapontype *next;
	struct sub_weapontype *prev;
	char *from;
	char *to;
} *subweapontype,*firstsubweapontype,*lastsubweapontype;

struct sub_objaffects
{
	struct sub_objaffects *next;
	struct sub_objaffects *prev;
	char *from;
	char *to;
} *subobjaffects,*firstsubobjaffects,*lastsubobjaffects;

struct sub_roomflag
{
	struct sub_roomflag *next;
	struct sub_roomflag *prev;
	char *from;
	char *to;
} *subroomflag,*firstsubroomflag,*lastsubroomflag;

struct sub_exitdir
{
	struct sub_exitdir *next;
	struct sub_exitdir *prev;
	char *from;
	char *to;
} *subexitdir,*firstsubexitdir,*lastsubexitdir;

struct sub_exitflag
{
	struct sub_exitflag *next;
	struct sub_exitflag *prev;
	char *from;
	char *to;
} *subexitflag,*firstsubexitflag,*lastsubexitflag;

struct sub_spec
{
	struct sub_spec *next;
	struct sub_spec *prev;
	char *from;
	char *to;
} *subspec,*firstsubspec,*lastsubspec;

struct sub_wearloc
{
	struct sub_wearloc *next;
	struct sub_wearloc *prev;
	char *from;
	char *to;
} *subwearloc,*firstsubwearloc,*lastsubwearloc;

struct sub_progtrig
{
	struct sub_progtrig *next;
	struct sub_progtrig *prev;
	char *from;
	char *to;
} *subprogtrig,*firstsubprogtrig,*lastsubprogtrig;
struct sub_progcomm
{
	struct sub_progcomm *next;
	struct sub_progcomm *prev;
	char *from;
	char *to;
} *subprogcomm,*firstsubprogcomm,*lastsubprogcomm;

/* externals ... or to be mades */
int	get_areaflag	args( ( char *flag ) );
int	get_actflag	args( ( char *flag ) );
int	get_npc_race	args( ( char *flag ) );
int	get_npc_class	args( ( char *flag ) );
int	get_risflag	args( ( char *flag ) );
int	get_langflag	args( ( char *flag ) );
int	get_partflag	args( ( char *flag ) );
int	get_attackflag	args( ( char *flag ) );
int	get_attackflag2	args( ( char *flag ) );
int	get_defenseflag	args( ( char *flag ) );
int	get_wflag	args( ( char *flag ) );
int	get_mflag	args( ( char *flag ) );
int	get_trigflag	args( ( char *flag ) );
int	get_trapflag	args( ( char *flag ) );
int	get_rflag	args( ( char *flag ) );
int	get_rflag2	args( ( char *flag ) );
int	get_exflag	args( ( char *flag ) );
int	get_wearloc	args( ( char *flag ) );
void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex);
void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex);
void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex);
void fix_exits(void);

/* the good stuff */


/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes. do not mess with case.
 * as opposed to native one_argument which ignores case.
 */
char *One_Argument( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = (*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char * find_sub(void *sub, char *s)
{
	if(sub == subskill)
	{
		struct sub_skill *subit;
		for(subit = firstsubskill; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subspell)
	{
		struct sub_spell *subit;
		for(subit = firstsubspell; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subobjflag)
	{
		struct sub_objflag *subit;
		for(subit = firstsubobjflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == submobflag)
	{
		struct sub_mflag *subit;
		for(subit = firstsubmobflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subareaflag)
	{
		struct sub_aflag *subit;
		for(subit = firstsubareaflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subaffected)
	{
		struct sub_affected *subit;
		for(subit = firstsubaffectedflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == submobrace)
	{
		struct sub_mobrace *subit;
		for(subit = firstsubmobrace; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == submobclass)
	{
		struct sub_mobclass *subit;
		for(subit = firstsubmobclass; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == submoblang)
	{
		struct sub_moblang *subit;
		for(subit = firstsubmoblang; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == submobpos)
	{
		struct sub_mobpos *subit;
		for(subit = firstsubmobpos; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subsector)
	{
		struct sub_sector *subit;
		for(subit = firstsubsector; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subbodypart)
	{
		struct sub_bodypart *subit;
		for(subit = firstsubbodypart; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subrisflag)
	{
		struct sub_risflag *subit;
		for(subit = firstsubrisflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subattack)
	{
		struct sub_attack *subit;
		for(subit = firstsubattack; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subdefense)
	{
		struct sub_defense *subit;
		for(subit = firstsubdefense; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subobjtype)
	{
		struct sub_objtype *subit;
		for(subit = firstsubobjtype; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subwearflag)
	{
		struct sub_wearflag *subit;
		for(subit = firstsubwearflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subcontflag)
	{
		struct sub_contflag *subit;
		for(subit = firstsubcontflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subwearlayerflag)
	{
		struct sub_wearlayerflag *subit;
		for(subit = firstsubwearlayerflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subliquid)
	{
		struct sub_liquid *subit;
		for(subit = firstsubliquid; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subherb)
	{
		struct sub_herb *subit;
		for(subit = firstsubherb; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subtrigger)
	{
		struct sub_trigger *subit;
		for(subit = firstsubtrigger; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subtraptype)
	{
		struct sub_traptype *subit;
		for(subit = firstsubtraptype; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subtrapflag)
	{
		struct sub_trapflag *subit;
		for(subit = firstsubtrapflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subweapontype)
	{
		struct sub_weapontype *subit;
		for(subit = firstsubweapontype; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subobjaffects)
	{
		struct sub_objaffects *subit;
		for(subit = firstsubobjaffects; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subroomflag)
	{
		struct sub_roomflag *subit;
		for(subit = firstsubroomflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subexitdir)
	{
		struct sub_exitdir *subit;
		for(subit = firstsubexitdir; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subexitflag)
	{
		struct sub_exitflag *subit;
		for(subit = firstsubexitflag; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subspec)
	{
		struct sub_spec *subit;
		for(subit = firstsubspec; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subwearloc)
	{
		struct sub_wearloc *subit;
		for(subit = firstsubwearloc; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subprogtrig)
	{
		struct sub_progtrig *subit;
		for(subit = firstsubprogtrig; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	if(sub == subprogcomm)
	{
		struct sub_progcomm *subit;
		for(subit = firstsubprogcomm; subit; subit=subit->next)
		{
			if(!strcmp(subit->from,s))
			{
				return subit->to;
			}
		}
		return NULL;
	}
	bug("unknown sub array",0);
	return NULL;
}
void clean_subs(void)
{
	struct sub_skill *sub1;
	struct sub_spell *sub2;
	struct sub_objflag *sub3;
	struct sub_mflag *sub4;
	struct sub_aflag *sub5;
	struct sub_affected *sub6;
	struct sub_mobrace *sub7;
	struct sub_mobclass *sub8;
	struct sub_moblang *sub9;
	struct sub_mobpos *sub10;
	struct sub_sector *sub11;
	struct sub_bodypart *sub12;
	struct sub_risflag *sub13;
	struct sub_attack *sub14;
	struct sub_defense *sub15;
	struct sub_objtype *sub16;
	struct sub_wearflag *sub17;
	struct sub_contflag *sub18;
	struct sub_wearlayerflag *sub19;
	struct sub_liquid *sub20;
	struct sub_herb *sub21;
	struct sub_trigger *sub22;
	struct sub_traptype *sub23;
	struct sub_trapflag *sub24;
	struct sub_weapontype *sub25;
	struct sub_objaffects *sub26;
	struct sub_roomflag *sub27;
	struct sub_exitdir *sub28;
	struct sub_exitflag *sub29;
	struct sub_spec *sub30;
	struct sub_wearloc *sub31;
	struct sub_progtrig *sub32;
	struct sub_progcomm *sub33;

	for(sub1=firstsubskill;sub1;sub1=sub1->next)
	{
		STRFREE(sub1->to);
		STRFREE(sub1->from);
	}
	while((sub1=firstsubskill))
	{	
		UNLINK(sub1,firstsubskill,lastsubskill,next,prev);
		DISPOSE(sub1);
	}
	for(sub2=firstsubspell;sub2;sub2=sub2->next)
	{
		STRFREE(sub2->to);
		STRFREE(sub2->from);
	}
	while((sub2=firstsubspell))
	{	
		UNLINK(sub2,firstsubspell,lastsubspell,next,prev);
		DISPOSE(sub2);
	}
	for(sub3=firstsubobjflag;sub3;sub3=sub3->next)
	{
		STRFREE(sub3->to);
		STRFREE(sub3->from);
	}
	while((sub3=firstsubobjflag))
	{	
		UNLINK(sub3,firstsubobjflag,lastsubobjflag,next,prev);
		DISPOSE(sub3);
	}
	for(sub4=firstsubmobflag;sub4;sub4=sub4->next)
	{
		STRFREE(sub4->to);
		STRFREE(sub4->from);
	}
	while((sub4=firstsubmobflag))
	{	
		UNLINK(sub4,firstsubmobflag,lastsubmobflag,next,prev);
		DISPOSE(sub4);
	}
	for(sub5=firstsubareaflag;sub5;sub5=sub5->next)
	{
		STRFREE(sub5->to);
		STRFREE(sub5->from);
	}
	while((sub5=firstsubareaflag))
	{	
		UNLINK(sub5,firstsubareaflag,lastsubareaflag,next,prev);
		DISPOSE(sub5);
	}
	for(sub6=firstsubaffectedflag;sub6;sub6=sub6->next)
	{
		STRFREE(sub6->to);
		STRFREE(sub6->from);
	}
	while((sub6=firstsubaffectedflag))
	{	
		UNLINK(sub6,firstsubaffectedflag,lastsubaffectedflag,next,prev);
		DISPOSE(sub6);
	}
	for(sub7=firstsubmobrace;sub7;sub7=sub7->next)
	{
		STRFREE(sub7->to);
		STRFREE(sub7->from);
	}
	while((sub7=firstsubmobrace))
	{	
		UNLINK(sub7,firstsubmobrace,lastsubmobrace,next,prev);
		DISPOSE(sub7);
	}
	for(sub8=firstsubmobclass;sub8;sub8=sub8->next)
	{
		STRFREE(sub8->to);
		STRFREE(sub8->from);
	}
	while((sub8=firstsubmobclass))
	{	
		UNLINK(sub8,firstsubmobclass,lastsubmobclass,next,prev);
		DISPOSE(sub8);
	}
	for(sub9=firstsubmoblang;sub9;sub9=sub9->next)
	{
		STRFREE(sub9->to);
		STRFREE(sub9->from);
	}
	while((sub9=firstsubmoblang))
	{	
		UNLINK(sub9,firstsubmoblang,lastsubmoblang,next,prev);
		DISPOSE(sub9);
	}
	for(sub10=firstsubmobpos;sub10;sub10=sub10->next)
	{
		STRFREE(sub10->to);
		STRFREE(sub10->from);
	}
	while((sub10=firstsubmobpos))
	{	
		UNLINK(sub10,firstsubmobpos,lastsubmobpos,next,prev);
		DISPOSE(sub10);
	}
	for(sub11=firstsubsector;sub11;sub11=sub11->next)
	{
		STRFREE(sub11->to);
		STRFREE(sub11->from);
	}
	while((sub11=firstsubsector))
	{	
		UNLINK(sub11,firstsubsector,lastsubsector,next,prev);
		DISPOSE(sub11);
	}
	for(sub12=firstsubbodypart;sub12;sub12=sub12->next)
	{
		STRFREE(sub12->to);
		STRFREE(sub12->from);
	}
	while((sub12=firstsubbodypart))
	{	
		UNLINK(sub12,firstsubbodypart,lastsubbodypart,next,prev);
		DISPOSE(sub12);
	}
	for(sub13=firstsubrisflag;sub13;sub13=sub13->next)
	{
		STRFREE(sub13->to);
		STRFREE(sub13->from);
	}
	while((sub13=firstsubrisflag))
	{	
		UNLINK(sub13,firstsubrisflag,lastsubrisflag,next,prev);
		DISPOSE(sub13);
	}
	for(sub14=firstsubattack;sub14;sub14=sub14->next)
	{
		STRFREE(sub14->to);
		STRFREE(sub14->from);
	}
	while((sub14=firstsubattack))
	{	
		UNLINK(sub14,firstsubattack,lastsubattack,next,prev);
		DISPOSE(sub14);
	}
	for(sub15=firstsubdefense;sub15;sub15=sub15->next)
	{
		STRFREE(sub15->to);
		STRFREE(sub15->from);
	}
	while((sub15=firstsubdefense))
	{	
		UNLINK(sub15,firstsubdefense,lastsubdefense,next,prev);
		DISPOSE(sub15);
	}
	for(sub16=firstsubobjtype;sub16;sub16=sub16->next)
	{
		STRFREE(sub16->to);
		STRFREE(sub16->from);
	}
	while((sub16=firstsubobjtype))
	{	
		UNLINK(sub16,firstsubobjtype,lastsubobjtype,next,prev);
		DISPOSE(sub16);
	}
	for(sub17=firstsubwearflag;sub17;sub17=sub17->next)
	{
		STRFREE(sub17->to);
		STRFREE(sub17->from);
	}
	while((sub17=firstsubwearflag))
	{	
		UNLINK(sub17,firstsubwearflag,lastsubwearflag,next,prev);
		DISPOSE(sub17);
	}
	for(sub18=firstsubcontflag;sub18;sub18=sub18->next)
	{
		STRFREE(sub18->to);
		STRFREE(sub18->from);
	}
	while((sub18=firstsubcontflag))
	{	
		UNLINK(sub18,firstsubcontflag,lastsubcontflag,next,prev);
		DISPOSE(sub18);
	}
	for(sub19=firstsubwearlayerflag;sub19;sub19=sub19->next)
	{
		STRFREE(sub19->to);
		STRFREE(sub19->from);
	}
	while((sub19=firstsubwearlayerflag))
	{	
		UNLINK(sub19,firstsubwearlayerflag,lastsubwearlayerflag,next,prev);
		DISPOSE(sub19);
	}
	for(sub20=firstsubliquid;sub20;sub20=sub20->next)
	{
		STRFREE(sub20->to);
		STRFREE(sub20->from);
	}
	while((sub20=firstsubliquid))
	{	
		UNLINK(sub20,firstsubliquid,lastsubliquid,next,prev);
		DISPOSE(sub20);
	}
	for(sub21=firstsubherb;sub21;sub21=sub21->next)
	{
		STRFREE(sub21->to);
		STRFREE(sub21->from);
	}
	while((sub21=firstsubherb))
	{	
		UNLINK(sub21,firstsubherb,lastsubherb,next,prev);
		DISPOSE(sub21);
	}
	for(sub22=firstsubtrigger;sub22;sub22=sub22->next)
	{
		STRFREE(sub22->to);
		STRFREE(sub22->from);
	}
	while((sub22=firstsubtrigger))
	{	
		UNLINK(sub22,firstsubtrigger,lastsubtrigger,next,prev);
		DISPOSE(sub22);
	}
	for(sub23=firstsubtraptype;sub23;sub23=sub23->next)
	{
		STRFREE(sub23->to);
		STRFREE(sub23->from);
	}
	while((sub23=firstsubtraptype))
	{	
		UNLINK(sub23,firstsubtraptype,lastsubtraptype,next,prev);
		DISPOSE(sub23);
	}
	for(sub24=firstsubtrapflag;sub24;sub24=sub24->next)
	{
		STRFREE(sub24->to);
		STRFREE(sub24->from);
	}
	while((sub24=firstsubtrapflag))
	{	
		UNLINK(sub24,firstsubtrapflag,lastsubtrapflag,next,prev);
		DISPOSE(sub24);
	}
	for(sub25=firstsubweapontype;sub25;sub25=sub25->next)
	{
		STRFREE(sub25->to);
		STRFREE(sub25->from);
	}
	while((sub25=firstsubweapontype))
	{	
		UNLINK(sub25,firstsubweapontype,lastsubweapontype,next,prev);
		DISPOSE(sub25);
	}
	for(sub26=firstsubobjaffects;sub26;sub26=sub26->next)
	{
		STRFREE(sub26->to);
		STRFREE(sub26->from);
	}
	while((sub26=firstsubobjaffects))
	{	
		UNLINK(sub26,firstsubobjaffects,lastsubobjaffects,next,prev);
		DISPOSE(sub26);
	}
	for(sub27=firstsubroomflag;sub27;sub27=sub27->next)
	{
		STRFREE(sub27->to);
		STRFREE(sub27->from);
	}
	while((sub27=firstsubroomflag))
	{	
		UNLINK(sub27,firstsubroomflag,lastsubroomflag,next,prev);
		DISPOSE(sub27);
	}
	for(sub28=firstsubexitdir;sub28;sub28=sub28->next)
	{
		STRFREE(sub28->to);
		STRFREE(sub28->from);
	}
	while((sub28=firstsubexitdir))
	{	
		UNLINK(sub28,firstsubexitdir,lastsubexitdir,next,prev);
		DISPOSE(sub28);
	}
	for(sub29=firstsubexitflag;sub29;sub29=sub29->next)
	{
		STRFREE(sub29->to);
		STRFREE(sub29->from);
	}
	while((sub29=firstsubexitflag))
	{	
		UNLINK(sub29,firstsubexitflag,lastsubexitflag,next,prev);
		DISPOSE(sub29);
	}
	for(sub30=firstsubspec;sub30;sub30=sub30->next)
	{
		STRFREE(sub30->to);
		STRFREE(sub30->from);
	}
	while((sub30=firstsubspec))
	{	
		UNLINK(sub30,firstsubspec,lastsubspec,next,prev);
		DISPOSE(sub30);
	}
	for(sub31=firstsubwearloc;sub31;sub31=sub31->next)
	{
		STRFREE(sub31->to);
		STRFREE(sub31->from);
	}
	while((sub31=firstsubwearloc))
	{	
		UNLINK(sub31,firstsubwearloc,lastsubwearloc,next,prev);
		DISPOSE(sub31);
	}
	for(sub32=firstsubprogtrig;sub32;sub32=sub32->next)
	{
		STRFREE(sub32->to);
		STRFREE(sub32->from);
	}
	while((sub32=firstsubprogtrig))
	{	
		UNLINK(sub32,firstsubprogtrig,lastsubprogtrig,next,prev);
		DISPOSE(sub32);
	}
	for(sub33=firstsubprogcomm;sub33;sub33=sub33->next)
	{
		STRFREE(sub33->to);
		STRFREE(sub33->from);
	}
	while((sub33=firstsubprogcomm))
	{	
		UNLINK(sub33,firstsubprogcomm,lastsubprogcomm,next,prev);
		DISPOSE(sub33);
	}
}
void add_sub(void *sub, char *buf)
{
	char arg1[1024];
	char arg2[1024];

	buf=buf=One_Argument(buf,arg1);
	buf=buf=One_Argument(buf,arg2);
	if(!*arg1 || !*arg2) /* probably a blank line */
		return;
	if(sub == subskill)
	{
		struct sub_skill *subit;
		CREATE(subit,struct sub_skill,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubskill,lastsubskill,next,prev);
		return;
	}
	if(sub == subspell)
	{
		struct sub_spell *subit;
		CREATE(subit,struct sub_spell,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubspell,lastsubspell,next,prev);
		return;
	}
	if(sub == subobjflag)
	{
		struct sub_objflag *subit;
		CREATE(subit,struct sub_objflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubobjflag,lastsubobjflag,next,prev);
		return;
	}
	if(sub == submobflag)
	{
		struct sub_mflag *subit;
		CREATE(subit,struct sub_mflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubmobflag,lastsubmobflag,next,prev);
		return;
	}
	if(sub == subareaflag)
	{
		struct sub_aflag *subit;
		CREATE(subit,struct sub_aflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubareaflag,lastsubareaflag,next,prev);
		return;
	}
	if(sub == subaffected)
	{
		struct sub_affected *subit;
		CREATE(subit,struct sub_affected,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubaffectedflag,lastsubaffectedflag,next,prev);
		return;
	}
	if(sub == submobrace)
	{
		struct sub_mobrace *subit;
		CREATE(subit,struct sub_mobrace,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubmobrace,lastsubmobrace,next,prev);
		return;
	}
	if(sub == submobclass)
	{
		struct sub_mobclass *subit;
		CREATE(subit,struct sub_mobclass,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubmobclass,lastsubmobclass,next,prev);
		return;
	}
	if(sub == submobclass)
	{
		struct sub_moblang *subit;
		CREATE(subit,struct sub_moblang,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubmoblang,lastsubmoblang,next,prev);
		return;
	}
	if(sub == submobpos)
	{
		struct sub_mobpos *subit;
		CREATE(subit,struct sub_mobpos,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubmobpos,lastsubmobpos,next,prev);
		return;
	}
	if(sub == subsector)
	{
		struct sub_sector *subit;
		CREATE(subit,struct sub_sector,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubsector,lastsubsector,next,prev);
		return;
	}
	if(sub == subbodypart)
	{
		struct sub_bodypart *subit;
		CREATE(subit,struct sub_bodypart,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubbodypart,lastsubbodypart,next,prev);
		return;
	}
	if(sub == subrisflag)
	{
		struct sub_risflag *subit;
		CREATE(subit,struct sub_risflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubrisflag,lastsubrisflag,next,prev);
		return;
	}
	if(sub == subattack)
	{
		struct sub_attack *subit;
		CREATE(subit,struct sub_attack,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubattack,lastsubattack,next,prev);
		return;
	}
	if(sub == subdefense)
	{
		struct sub_defense *subit;
		CREATE(subit,struct sub_defense,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubdefense,lastsubdefense,next,prev);
		return;
	}
	if(sub == subobjtype)
	{
		struct sub_objtype *subit;
		CREATE(subit,struct sub_objtype,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubobjtype,lastsubobjtype,next,prev);
		return;
	}
	if(sub == subwearflag)
	{
		struct sub_wearflag *subit;
		CREATE(subit,struct sub_wearflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubwearflag,lastsubwearflag,next,prev);
		return;
	}
	if(sub == subcontflag)
	{
		struct sub_contflag *subit;
		CREATE(subit,struct sub_contflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubcontflag,lastsubcontflag,next,prev);
		return;
	}
	if(sub == subwearlayerflag)
	{
		struct sub_wearlayerflag *subit;
		CREATE(subit,struct sub_wearlayerflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubwearlayerflag,lastsubwearlayerflag,next,prev);
		return;
	}
	if(sub == subliquid)
	{
		struct sub_liquid *subit;
		CREATE(subit,struct sub_liquid,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubliquid,lastsubliquid,next,prev);
		return;
	}
	if(sub == subherb)
	{
		struct sub_herb *subit;
		CREATE(subit,struct sub_herb,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubherb,lastsubherb,next,prev);
		return;
	}
	if(sub == subtrigger)
	{
		struct sub_trigger *subit;
		CREATE(subit,struct sub_trigger,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubtrigger,lastsubtrigger,next,prev);
		return;
	}
	if(sub == subtraptype)
	{
		struct sub_traptype *subit;
		CREATE(subit,struct sub_traptype,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubtraptype,lastsubtraptype,next,prev);
		return;
	}
	if(sub == subtrapflag)
	{
		struct sub_trapflag *subit;
		CREATE(subit,struct sub_trapflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubtrapflag,lastsubtrapflag,next,prev);
		return;
	}
	if(sub == subweapontype)
	{
		struct sub_weapontype *subit;
		CREATE(subit,struct sub_weapontype,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubweapontype,lastsubweapontype,next,prev);
		return;
	}
	if(sub == subobjaffects)
	{
		struct sub_objaffects *subit;
		CREATE(subit,struct sub_objaffects,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubobjaffects,lastsubobjaffects,next,prev);
		return;
	}
	if(sub == subroomflag)
	{
		struct sub_roomflag *subit;
		CREATE(subit,struct sub_roomflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubroomflag,lastsubroomflag,next,prev);
		return;
	}
	if(sub == subexitdir)
	{
		struct sub_exitdir *subit;
		CREATE(subit,struct sub_exitdir,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubexitdir,lastsubexitdir,next,prev);
		return;
	}
	if(sub == subexitflag)
	{
		struct sub_exitflag *subit;
		CREATE(subit,struct sub_exitflag,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubexitflag,lastsubexitflag,next,prev);
		return;
	}
	if(sub == subspec)
	{
		struct sub_spec *subit;
		CREATE(subit,struct sub_spec,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubspec,lastsubspec,next,prev);
		return;
	}
	if(sub == subwearloc)
	{
		struct sub_wearloc *subit;
		CREATE(subit,struct sub_wearloc,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubwearloc,lastsubwearloc,next,prev);
		return;
	}
	if(sub == subprogtrig)
	{
		struct sub_progtrig *subit;
		CREATE(subit,struct sub_progtrig,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubprogtrig,lastsubprogtrig,next,prev);
		return;
	}
	if(sub == subprogcomm)
	{
		struct sub_progcomm *subit;
		CREATE(subit,struct sub_progcomm,1);
		subit->from=STRALLOC(arg1);
		subit->to=STRALLOC(arg2);
		LINK(subit,firstsubprogcomm,lastsubprogcomm,next,prev);
		return;
	}
	bug("unknown sub array",0);

}
int getmobext(AREA_DATA *tarea,int vnum)
{
	int i;

	for(i=0;i<top_extmob;i++)
	{
		if(extmob[i]==vnum)
		{
			if(mobnew[i])
				return mobnew[i];
			if(vnum < tarea->low_m_vnum
			|| vnum > tarea->hi_m_vnum)
			{
				mobnew[i]=newrange;
				return newrange++;	
			}
			else
			{
				mobnew[i]=extmob[i];
				return vnum;
			}
		}
	}
	bug("mob %d not found",vnum);
	return 0;
}
int getobjext(AREA_DATA *tarea,int vnum)
{
	int i;

	for(i=0;i<top_extobj;i++)
	{
		if(extobj[i]==vnum)
		{
			if(objnew[i])
				return objnew[i];
			if(vnum < tarea->low_o_vnum
			|| vnum > tarea->hi_o_vnum)
			{
				objnew[i]=newrange;
				return newrange++;	
			}
			else
			{
				objnew[i]=vnum;
				return vnum;
			}
		}
	}
	bug("mob %d not found",vnum);
	return 0;
}
void find_externals(AREA_DATA *tarea)
{
	RESET_DATA *treset;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	top_extmob=0;
	top_extobj=0;
        addmobrange=0;
        addobjrange=0;

        for(vnum=tarea->low_m_vnum;vnum<= tarea->hi_m_vnum;vnum++)
	{
		if(!(pMobIndex=get_mob_index(vnum)))
		{
			continue;
		}
#ifdef DRAGONBANE
        	if (IS_SET(pMobIndex->act,ACT_DELETED))
	  		continue;
#endif
		mobnew[top_extmob]=0;
		extmob[top_extmob++]=vnum;
	}
        for(vnum=tarea->low_o_vnum;vnum<= tarea->hi_o_vnum;vnum++)
	{
		if(!(pObjIndex=get_obj_index(vnum)))
		{
			continue;
		}
#ifdef DRAGONBANE
        	if (pObjIndex->item_type == ITEM_DELETED)
	  		continue;
#endif
		objnew[top_extmob]=0;
		extobj[top_extobj++]=vnum;
	}

    	for ( treset = tarea->first_reset; treset; treset = treset->next )
    	{
		switch( treset->command ) /* extra arg1 arg2 arg3 */
		{
	  		default: break;
			case 'o': case 'O':
			case 'e': case 'E':
			case 'g': case 'G':
			case 'p': case 'P':
				vnum=treset->arg1;
				if(vnum >= tarea->low_o_vnum
                                && vnum <= tarea->hi_o_vnum)
					break;
				objnew[top_extmob]=0;
				extobj[top_extobj++]=vnum;
				addobjrange++;	
				break;
			case 'm': case 'M':
				vnum=treset->arg1;
				if(vnum >= tarea->low_m_vnum
                                && vnum <= tarea->hi_m_vnum)
					break;
				mobnew[top_extmob]=0;
				extmob[top_extmob++]=vnum;
				addmobrange++;	
				break;
		}
	}
}
static char mudprog[MAX_STRING_LENGTH];
	
char * make_out_prog(char *s)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
        char *bptr;

        bptr=buf;
	mudprog[0]=0;

	while(*s)
        {
		if(*s == '\n')	
		{
			s++;
			*bptr=0;
			bptr=buf;
			bptr=one_argument(bptr,arg);
			while(*arg)
			{
				if(!str_prefix(arg,"mpoload"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					bptr=one_argument(bptr,arg);
					if(*arg)
					{
						sprintf(mudprog + strlen(mudprog),
						"%f",OUT_LEVEL(atoi(arg)));
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpmload"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpat"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
                                        if(isdigit(arg[strlen(arg)-1]))
                                        {
					  sprintf(mudprog + strlen(mudprog),
						"%d %s",OUT_VNUM(atoi(arg)),bptr);
					}
					else
					{
					  sprintf(mudprog + strlen(mudprog),
						"%s %s",arg,bptr);
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpgoto"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					if(isdigit(arg[strlen(arg)-1]))
					{
						sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					}
					else
					{
						sprintf(mudprog + strlen(mudprog),
						"%s ",bptr);
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mptransfer"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%s ",arg);
					bptr=one_argument(bptr,arg);
                                        if(*arg)
                                        {
						sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpopenpassage"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					sprintf(mudprog + strlen(mudprog),
						"%s",bptr);
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpclosepassage"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%d ",OUT_VNUM(atoi(arg)));
					sprintf(mudprog + strlen(mudprog),
						"%s",bptr);
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else
				{
					sprintf(mudprog + strlen(mudprog),
					"%s %s\n",arg,bptr);
					*arg=0;
				}
			}
			bptr=buf;
			*bptr=0;
				
		}
		*bptr++ = *s++;
	}
        return mudprog; 
}

char * make_in_prog(char *s,char *name,int its_vnum, int lowvnum, int hivnum,FILE *axlog)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
        char *bptr,*word;
        int vnum;

        bptr=buf;
	mudprog[0]=0;

	while(*s)
        {
		if(*s == '\n')	
		{
			s++;
			*bptr=0;
			bptr=buf;
			bptr=one_argument(bptr,arg);
			if((word=find_sub(subprogcomm,arg)))
			{
				strcpy(arg,word);
			}
			if (*arg
			&& strcmp(arg,"if")
			&& strcmp(arg,"endif")
			&& strcmp(arg,"else")
			&& strcmp(arg,"~")
			&& strcmp(arg,"|")
			&& ! find_command(arg)
			&& ! find_social(arg))
			{
				fprintf(axlog,"Uknown Command,%s, in mud program for %s (%d)\n",
					arg,name,its_vnum);		
                        }
			while(*arg)
			{
				if(!str_prefix(arg,"mpoload"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					vnum=IN_VNUM(atoi(arg));
					if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpoload references external object for %s (%d)\n",
							name,its_vnum);		
					sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					bptr=one_argument(bptr,arg);
					if(*arg)
					{
						sprintf(mudprog + strlen(mudprog),
						"%d",IN_LEVEL(atof(arg)));
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpmload"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					vnum=IN_VNUM(atoi(arg));
					if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpmload references external mobile for %s (%d)\n",
							name,its_vnum);		
					sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpat"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					if(isdigit(arg[strlen(arg)-1]))
                                        {
					  vnum=IN_VNUM(atoi(arg));
					  if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpat references external room for %s (%d)\n",
							name,its_vnum);		
					  sprintf(mudprog + strlen(mudprog),
						"%d %s",vnum,bptr);
					}
					else
					{
					  sprintf(mudprog + strlen(mudprog),
						"%s %s ",arg,bptr);
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpgoto"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					if(isdigit(arg[strlen(arg)-1]))
					{
					  vnum=IN_VNUM(atoi(arg));
					  if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpgoto references external room for %s (%d)\n",
							name,its_vnum);		
						sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					}
					else
					{
						sprintf(mudprog + strlen(mudprog),
						"%s %s ",arg,bptr);
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mptransfer"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					sprintf(mudprog + strlen(mudprog),
						"%s ",arg);
					bptr=one_argument(bptr,arg);
                                        if(*arg)
                                        {
					  vnum=IN_VNUM(atoi(arg));
					  if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mptransfer references external room for %s (%d)\n",
							name,its_vnum);		
					  sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					}
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpopenpassage"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					vnum=IN_VNUM(atoi(arg));
					if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpopenpassage references external room for %s (%d)\n",
							name,its_vnum);		
					sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					bptr=one_argument(bptr,arg);
					vnum=IN_VNUM(atoi(arg));
					if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpopenpassage references external room for %s (%d)\n",
							name,its_vnum);		
					sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					sprintf(mudprog + strlen(mudprog),
						"%s",bptr);
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else if(!str_prefix(arg,"mpclosepassage"))
				{
					strcat(mudprog,arg);
					strcat(mudprog," ");
					bptr=one_argument(bptr,arg);
					vnum=IN_VNUM(atoi(arg));
					if(vnum < lowvnum || vnum > hivnum)
						fprintf(axlog,"mpclosepassage references external room for %s (%d)\n",
							name,its_vnum);		
					sprintf(mudprog + strlen(mudprog),
						"%d ",vnum);
					sprintf(mudprog + strlen(mudprog),
						"%s",bptr);
					strcat(mudprog,"\n");
					bptr=one_argument(bptr,arg);
					continue;
				}
				else
				{
					sprintf(mudprog + strlen(mudprog),
					"%s %s\n",arg,bptr);
					*arg=0;
				}
			}
			bptr=buf;
			*bptr=0;
				
		}
		*bptr++ = *s++;
	}
        return mudprog; 
}
void do_ax_import(CHAR_DATA *ch, char *argument)
{
	FILE *fp,*axlog;
	char buff[2048];
	char *buf=buff;
	char command[80];
	char word[80];
	char filename[80];
	AREA_DATA *area=NULL,*tarea=NULL;
	char *s;
	int subtype=0,i=0,iHash=0,letter=0,extra=0,arg1=0,arg2=0,arg3=0;
	int buyslot=0;
    	MOB_INDEX_DATA	*pMobIndex=NULL;
    	OBJ_INDEX_DATA	*pObjIndex=NULL;
    	ROOM_INDEX_DATA	*pRoomIndex=NULL;
    	SHOP_DATA	*pShop;
    	REPAIR_DATA	*rShop;
	int 		noextlink=0;

	if(! *argument)
	{
		display(version);
		display("aximport <filename.aex> [newstartvnum] [NOEXTLINK]\n\r");
		return;
	}
	argument=One_Argument(argument,filename);
	argument=One_Argument(argument,word);
        fBootDb=TRUE;
	if(isdigit(*word))
	{ 
		renumber=atoi(word);
		argument=One_Argument(argument,word);
	}
	if(!str_cmp(word,"noextlink"))
	{
		noextlink=1;
	}	
	
	if(strchr(filename,'/'))
	{
		sprintf(buf,"../exchange/%s",filename);
		strcpy(strrchr(buf,'/'),"/axsub.dat");	
	}
	else
	{
		sprintf(buf,"../exchange/axsub.dat");
	}
	if((fp=fopen(buf,"r"))==NULL)
	{
		display("Can't open ");
		display(buf);
		display("\n\r");
		return;	
        }
	while(fgets(buff,1000,fp))
	{
		buf=strip_cr(buff);
		if(strrchr(buf,'\n'))
			*(strrchr(buf,'\n'))=0;
		buf=One_Argument(buf,command);	
		if(!strcmp(command,"#GOLDADJUSTPERCENT"))
                {
			goldadjust=atof(buf);
		}
		if(!strcmp(command,"#SPELLS"))
		{
			subtype=1;
			continue;
		}
		if(!strcmp(command,"#SKILLS"))
		{
			subtype=2;
			continue;
		}
		if(!strcmp(command,"#OBJFLAGS"))
		{
			subtype=3;
			continue;
		}
		if(!strcmp(command,"#MOBFLAGS"))
		{
			subtype=4;
			continue;
		}
		if(!strcmp(command,"#AREAFLAGS"))
		{
			subtype=5;
			continue;
		}
		if(!strcmp(command,"#AFFECTED"))
		{
			subtype=6;
			continue;
		}
		if(!strcmp(command,"#MOBRACE"))
		{
			subtype=7;
			continue;
		}
		if(!strcmp(command,"#MOBCLASS"))
		{
			subtype=8;
			continue;
		}
		if(!strcmp(command,"#MOBLANGUAGES"))
		{
			subtype=9;
			continue;
		}
		if(!strcmp(command,"#MOBPOSITIONS"))
		{
			subtype=10;
			continue;
		}
		if(!strcmp(command,"#SECTORTYPES"))
		{
			subtype=11;
			continue;
		}
		if(!strcmp(command,"#BODYPARTS"))
		{
			subtype=12;
			continue;
		}
		if(!strcmp(command,"#RISFLAGS"))
		{
			subtype=13;
			continue;
		}
		if(!strcmp(command,"#MOBATTACKS"))
		{
			subtype=14;
			continue;
		}
		if(!strcmp(command,"#MOBDEFENSES"))
		{
			subtype=15;
			continue;
		}
		if(!strcmp(command,"#OBJTYPES"))
		{
			subtype=16;
			continue;
		}
		if(!strcmp(command,"#WEARFLAGS"))
		{
			subtype=17;
			continue;
		}
		if(!strcmp(command,"#CONTAINERFLAGS"))
		{
			subtype=18;
			continue;
		}
		if(!strcmp(command,"#OBJWEARLAYERS"))
		{
			subtype=19;
			continue;
		}
		if(!strcmp(command,"#LIQUIDTYPES"))
		{
			subtype=20;
			continue;
		}
		if(!strcmp(command,"#HERBS"))
		{
			subtype=21;
			continue;
		}
		if(!strcmp(command,"#TRAPTRIGGERS"))
		{
			subtype=22;
			continue;
		}
		if(!strcmp(command,"#TRAPTYPES"))
		{
			subtype=23;
			continue;
		}
		if(!strcmp(command,"#TRAPFLAGS"))
		{
			subtype=24;
			continue;
		}
		if(!strcmp(command,"#WEAPONTYPES"))
		{
			subtype=25;
			continue;
		}
		if(!strcmp(command,"#OBJAPPLYTYPES"))
		{
			subtype=26;
			continue;
		}
		if(!strcmp(command,"#ROOMFLAGS"))
		{
			subtype=27;
			continue;
		}
		if(!strcmp(command,"#ROOMEXITDIRS"))
		{
			subtype=28;
			continue;
		}
		if(!strcmp(command,"#ROOMEXITFLAGS"))
		{
			subtype=29;
			continue;
		}
		if(!strcmp(command,"#SPECIALFUNCTIONS"))
		{
			subtype=30;
			continue;
		}
		if(!strcmp(command,"#WEARLOCATIONS"))
		{
			subtype=31;
			continue;
		}
		if(!strcmp(command,"#PROGTRIGGERS"))
		{
			subtype=32;
			continue;
		}
		if(!strcmp(command,"#PROGCOMMANDS"))
		{
			subtype=33;
			continue;
		}
		if(!strcmp(command,"#END"))
		{
			subtype=0;
			break;
		}
		buf=strip_cr(buff);
		if(strrchr(buf,'\n'))
			*(strrchr(buf,'\n'))=0;
		switch(subtype)
		{
			case 0:
				break;
			case 1:
				add_sub(subskill,buf);
				break;
			case 2:
				add_sub(subspell,buf);
				break;
			case 3:
				add_sub(subobjflag,buf);
				break;
			case 4:
				add_sub(submobflag,buf);
				break;
			case 5:
				add_sub(subareaflag,buf);
				break;
			case 6:
				add_sub(subaffected,buf);
				break;
			case 7:
				add_sub(submobrace,buf);
				break;
			case 8:
				add_sub(submobclass,buf);
				break;
			case 9:
				add_sub(submoblang,buf);
				break;
			case 10:
				add_sub(submobpos,buf);
				break;
			case 11:
				add_sub(subsector,buf);
				break;
			case 12:
				add_sub(subbodypart,buf);
				break;
			case 13:
				add_sub(subrisflag,buf);
				break;
			case 14:
				add_sub(subattack,buf);
				break;
			case 15:
				add_sub(subdefense,buf);
				break;
			case 16:
				add_sub(subobjtype,buf);
				break;
			case 17:
				add_sub(subwearflag,buf);
				break;
			case 18: 
				add_sub(subcontflag,buf);
				break;
			case 19:
				add_sub(subwearlayerflag,buf);
				break;
			case 20:
				add_sub(subliquid,buf);
				break;
			case 21:
				add_sub(subherb,buf);
				break;
			case 22:
				add_sub(subtrigger,buf);
				break;
			case 23:
				add_sub(subtraptype,buf);
				break;
			case 24:
				add_sub(subtrapflag,buf);
				break;
			case 25:
				add_sub(subweapontype,buf);
				break;
			case 26:
				add_sub(subobjaffects,buf);
				break;
			case 27:
				add_sub(subroomflag,buf);
				break;
			case 28:
				add_sub(subexitdir,buf);
				break;
			case 29:
				add_sub(subexitflag,buf);
				break;
			case 30:
				add_sub(subspec,buf);
				break;
			case 31:
				add_sub(subwearloc,buf);
				break;
			case 32:
				add_sub(subprogtrig,buf);
				break;
			case 33:
				add_sub(subprogcomm,buf);
				break;
			default:
				sprintf(buf,"Unknow SubType %d\n",subtype);
				bug(buf,0);
				break;
		}
	
	}
	fclose(fp);
	sprintf(buf,"../exchange/%s",filename);
	if((fp=fopen(buf,"r"))==NULL)
	{
		display("Can't open ");
		display(buf);
		display("\n\r");
		return;	
	}
	strcat(buf,".log");
	if((axlog=fopen(buf,"w"))==NULL)
	{
		display("Can't open ");
		display(buf);
		display("\n\r");
		return;	
	}
	while(fgets(buff,2040,fp))
	{
		buf=strip_cr(buff);
		if(strrchr(buf,'\n'))
			*(strrchr(buf,'\n'))=0;
		buf=One_Argument(buf,command);
		if(!strcmp(command,"#EXPORTEDBY"))
                {
			/* documentary */
			continue;
		}
		if(!strcmp(command,"#AREANAME"))
		{
			CREATE(area,AREA_DATA,1);
			LINK( area, first_build, last_build, next, prev );
			top_area++;
			area=last_build;
			area->name=STRALLOC(buf);
			if(strchr(filename,'/'))
			{
				strcpy(filename,strrchr(filename,'/')+1);
			}
			area->filename=STRALLOC(filename);
			continue;	
		}
		if(!strcmp(command,"#STARTVNUM"))
		{
			startvnum = atoi(buf);
			if(renumber < 1)
			{
				renumber=startvnum;
			}
			if(IN_VNUM(0) > MAX_POSSIBLE_VNUM)
			{
				display("Out of range vnum detected, try aiming a little lower\n\r");
				STRFREE(area->filename);
				STRFREE(area->name);
				UNLINK( area, first_build, last_build, next, prev );
				DISPOSE(area);
				top_area--;
				return;
			}
			continue;
		}
		if(!strcmp(command,"#AUTHOR"))
		{
			area->author=STRALLOC(buf);
			continue;	
		}
		if(!strcmp(command,"#SOFTRANGE"))
		{
			buf=One_Argument(buf,word);
			area->low_soft_range=IN_LEVEL(atof(word));
			buf=One_Argument(buf,word);
			area->hi_soft_range=IN_LEVEL(atof(word));
			continue;
		}
		if(!strcmp(command,"#HARDRANGE"))
		{
			buf=One_Argument(buf,word);
			area->low_hard_range=IN_LEVEL(atof(word));
			buf=One_Argument(buf,word);
			area->hi_hard_range=IN_LEVEL(atof(word));
			continue;
		}
		if(!strcmp(command,"#ROOMRANGE"))
		{
			buf=One_Argument(buf,word);
			area->low_r_vnum=IN_VNUM(atoi(word));
			buf=One_Argument(buf,word);
			area->hi_r_vnum=IN_VNUM(atoi(word));
			if(IN_VNUM(atoi(word)) > MAX_POSSIBLE_VNUM)
			{
				display("Out of range vnum detected, try aiming a little lower\n\r");
				STRFREE(area->filename);
				STRFREE(area->name);
				STRFREE(area->author);
				UNLINK( area, first_build, last_build, next, prev );
				DISPOSE(area);
				return;
			}
			for(tarea=first_area;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_r_vnum >=tarea->low_r_vnum
					&& area->low_r_vnum <= tarea->hi_r_vnum)
				|| (area->hi_r_vnum >=tarea->low_r_vnum
					&& area->hi_r_vnum <= tarea->hi_r_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) rooms\n\r",
						area->name,
						area->low_r_vnum,
						area->hi_r_vnum,
						tarea->name,
						tarea->low_r_vnum,
						tarea->hi_r_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			for(tarea=first_build;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_r_vnum >=tarea->low_r_vnum
					&& area->low_r_vnum <= tarea->hi_r_vnum)
				|| (area->hi_r_vnum >=tarea->low_r_vnum
					&& area->hi_r_vnum <= tarea->hi_r_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) rooms\n\r",
						area->name,
						area->low_r_vnum,
						area->hi_r_vnum,
						tarea->name,
						tarea->low_r_vnum,
						tarea->hi_r_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			continue;
		}
		if(!strcmp(command,"#MOBRANGE"))
		{
			buf=One_Argument(buf,word);
			area->low_m_vnum=IN_VNUM(atoi(word));
			buf=One_Argument(buf,word);
			area->hi_m_vnum=IN_VNUM(atoi(word));
			if(IN_VNUM(atoi(word)) > MAX_POSSIBLE_VNUM)
			{
				display("Out of range vnum detected, try aiming a little lower\n\r");
				STRFREE(area->filename);
				STRFREE(area->name);
				STRFREE(area->author);
				UNLINK( area, first_build, last_build, next, prev );
				DISPOSE(area);
				return;
			}
			for(tarea=first_area;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_m_vnum >=tarea->low_m_vnum
					&& area->low_m_vnum <= tarea->hi_m_vnum)
				|| (area->hi_m_vnum >=tarea->low_m_vnum
					&& area->hi_m_vnum <= tarea->hi_m_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) mobs\n\r",
						area->name,
						area->low_m_vnum,
						area->hi_m_vnum,
						tarea->name,
						tarea->low_m_vnum,
						tarea->hi_m_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			for(tarea=first_build;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_m_vnum >=tarea->low_m_vnum
					&& area->low_m_vnum <= tarea->hi_m_vnum)
				|| (area->hi_m_vnum >=tarea->low_m_vnum
					&& area->hi_m_vnum <= tarea->hi_m_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) mobs\n\r",
						area->name,
						area->low_m_vnum,
						area->hi_m_vnum,
						tarea->name,
						tarea->low_m_vnum,
						tarea->hi_m_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			continue;
		}
		if(!strcmp(command,"#OBJRANGE"))
		{
			buf=One_Argument(buf,word);
			area->low_o_vnum=IN_VNUM(atoi(word));
			buf=One_Argument(buf,word);
			area->hi_o_vnum=IN_VNUM(atoi(word));
			if(IN_VNUM(atoi(word)) > MAX_POSSIBLE_VNUM)
			{
				display("Out of range vnum detected, try aiming a little lower\n\r");
				STRFREE(area->filename);
				STRFREE(area->name);
				STRFREE(area->author);
				UNLINK( area, first_build, last_build, next, prev );
				DISPOSE(area);
				return;
			}
			for(tarea=first_area;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_o_vnum >=tarea->low_o_vnum
					&& area->low_o_vnum <= tarea->hi_o_vnum)
				|| (area->hi_o_vnum >=tarea->low_o_vnum
					&& area->hi_o_vnum <= tarea->hi_o_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) objects\n\r",
						area->name,
						area->low_o_vnum,
						area->hi_o_vnum,
						tarea->name,
						tarea->low_o_vnum,
						tarea->hi_o_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			for(tarea=first_build;tarea;tarea=tarea->next)
			{
				if(area==tarea)
				{
					continue;
				}
				if((area->low_o_vnum >=tarea->low_o_vnum
					&& area->low_o_vnum <= tarea->hi_o_vnum)
				|| (area->hi_o_vnum >=tarea->low_o_vnum
					&& area->hi_o_vnum <= tarea->hi_o_vnum))
				{
					sprintf(buff,"%s (%d - %d) would overlap %s (%d - %d) objects\n\r",
						area->name,
						area->low_o_vnum,
						area->hi_o_vnum,
						tarea->name,
						tarea->low_o_vnum,
						tarea->hi_o_vnum);
					display(buff);
					STRFREE(area->filename);
					STRFREE(area->name);
					STRFREE(area->author);
					UNLINK( area, first_build, last_build, next, prev );
					DISPOSE(area);
					return;
				}
			}
			continue;
		}
		if(!strcmp(command,"#RESETMSG"))
		{
			area->resetmsg=STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"#RESETFREQUENCY"))
		{
			area->reset_frequency = atoi(buf);
			continue;
		}
		if(!strcmp(command,"#RECALL"))
		{
			continue;
		}
		if(!strcmp(command,"#AREAFLAGS"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subareaflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_areaflag(word)) > -1)
				{
					SET_BIT(area->flags,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown area flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"#ECONOMY"))
		{
			buf=One_Argument(buf,word);
			area->high_economy=atoi(word);
			buf=One_Argument(buf,word);
			area->low_economy=atoi(word);
			continue;
		}
		if(!strcmp(command,"#MOBILES"))
		{
			continue;
		}
		if(!strcmp(command,"!mobvnum"))
		{
			CREATE(pMobIndex,MOB_INDEX_DATA,1);
			pMobIndex->vnum=IN_VNUM(atoi(buf));
			iHash			= pMobIndex->vnum % MAX_KEY_HASH;
			pMobIndex->next		= mob_index_hash[iHash];
			mob_index_hash[iHash]	= pMobIndex;
#ifdef DRAGONBANE
			pMobIndex->talk=STRALLOC("");
#endif
			top_mob_index++;
			continue;
		}
		if(!strcmp(command,"@mobkeywordname"))
		{
			pMobIndex->player_name = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@mobactionname"))
		{
			pMobIndex->short_descr = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@mobinroomname"))
		{
			strcat(buf,"\n\r");
			pMobIndex->long_descr = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@moblookdescription"))
		{
			pMobIndex->description=fread_string(fp);
			continue;
		}
		if(!strcmp(command,"@mobtalkreply"))
		{
#ifdef DRAGONBANE
			pMobIndex->talk=fread_string(fp);
#endif
			continue;
		}
		if(!strcmp(command,"@mobactflags"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(submobflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_actflag(word)) > -1)
				{
#ifdef SMAUG102
					SET_BIT(pMobIndex->act,1 << i);
#endif
#ifdef SMAUG14
					xSET_BIT(pMobIndex->act,i);
#endif
				}
				else
			        {
					fprintf(axlog,"Unknown mobactflag flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobaffectedby"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subaffected,word)))
				{
					strcpy(word,s);
				}
				if((i=get_aflag(word)) > -1)
				{
#ifdef SMAUG102
					SET_BIT(pMobIndex->affected_by,1 << i);
#endif
#ifdef SMAUG14
					xSET_BIT(pMobIndex->affected_by,i);
#endif
				}
				else
			        {
					fprintf(axlog,"Unknown mobaffectedby flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobalignment"))
                {
			pMobIndex->alignment=(int)(atof(buf) * MAX_ALIGNMENT);
			continue;
		}
		if(!strcmp(command,"@moblevel"))
		{
			pMobIndex->level=IN_LEVEL(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobthac0"))
		{
			pMobIndex->mobthac0=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobarmorclass"))
		{
			pMobIndex->ac=(int)(atof(buf) * MAX_MOBARMORCLASS);
			continue;
		}
		if(!strcmp(command,"@mobnumberofhitdice"))
		{
			pMobIndex->hitnodice=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobsizeofhitdice"))
		{
			pMobIndex->hitsizedice=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobplustohitdice"))
		{
			pMobIndex->hitplus=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobnumberofdamagedice"))
		{
			pMobIndex->damnodice=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobsizeofdamagedice"))
		{
			pMobIndex->damsizedice=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobplustodamagedice"))
		{
			pMobIndex->damplus=IN_DICE(atof(buf));
			continue;
		}
		if(!strcmp(command,"@mobgold"))
		{
			pMobIndex->gold= GOLDADJ(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@mobexperience"))
		{
			pMobIndex->exp=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobposition"))
		{
			i=0;
			if((s=find_sub(submobpos,word)))
			{
				strcpy(buf,s);
			}
			while(ax_position[i])
			{
				if(!strcmp(ax_position[i],buf))
				{
					pMobIndex->position=i; 
					break;
				}
				i++;
			}
			if(!ax_position[i])
			{
				fprintf(axlog,"Unknown mobposition %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@mobdefaultposition"))
		{
			i=0;
			if((s=find_sub(submobpos,word)))
			{
				strcpy(buf,s);
			}
			while(ax_position[i])
			{
				if(!strcmp(ax_position[i],buf))
				{
					pMobIndex->defposition=i; 
					break;
				}
				i++;
			}
			if(!ax_position[i])
			{
				fprintf(axlog,"Unknown mobdefaultposition %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@mobsex"))
		{
			i=0;
			while(ax_sex[i])
			{
				if(!strcmp(ax_sex[i],buf))
				{
					pMobIndex->sex=i; 
					break;
				}
				i++;
			}
			if(!ax_sex[i])
			{
				fprintf(axlog,"Unknown mobsex %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@mobstrength"))
		{
			pMobIndex->perm_str = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobintelligence"))
		{
			pMobIndex->perm_int = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobwisdom"))
		{
			pMobIndex->perm_wis = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobdexterity"))
		{
			pMobIndex->perm_dex = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobconstitution"))
		{
			pMobIndex->perm_con = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobcharisma"))
		{
			pMobIndex->perm_cha = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobluck"))
		{
			pMobIndex->perm_lck = (int) (atof(buf) * MAX_PRIME_ATTRIB);
			continue;
		}
		if(!strcmp(command,"@mobsavevspoisondeath"))
		{
			pMobIndex->saving_poison_death=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobsavevswand"))
		{
			pMobIndex->saving_wand=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobsavevsparapetri"))
		{
			pMobIndex->saving_para_petri=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobsavevsbreath"))
		{
			pMobIndex->saving_breath=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobsavevsspellstaff"))
		{
			pMobIndex->saving_spell_staff=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobsavevspsionic"))
		{
#ifdef DRAGONBANE
			pMobIndex->saving_psionics=atoi(buf);
#endif
			continue;
		}
		if(!strcmp(command,"@mobrace"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(submobrace,word)))
				{
					strcpy(word,s);
				}
				if((i=get_npc_race(word)) > -1)
				{
					pMobIndex->race=i;
				}
				else
			        {
					fprintf(axlog,"Unknown mobrace: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobclass"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(submobclass,word)))
				{
					strcpy(word,s);
				}
				if((i=get_npc_class(word)) > -1)
				{
					pMobIndex->class=i;
				}
				else
			        {
					fprintf(axlog,"Unknown mobclass: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobheight"))
		{
			pMobIndex->height=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobweight"))
		{
			pMobIndex->weight=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobspeaks"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(submoblang,word)))
				{
					strcpy(word,s);
				}
				if((i=get_langflag(word)) > -1)
				{
					SET_BIT(pMobIndex->speaks,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobspeaks: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobspeaking"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(submoblang,word)))
				{
					strcpy(word,s);
				}
				if((i=get_langflag(word)) > -1)
				{
					SET_BIT(pMobIndex->speaking,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobspeaking: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobnumberofattacks"))
		{
			pMobIndex->numattacks=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobhoardroom"))
		{
#ifdef DRAGONBANE
			pMobIndex->hoard=IN_VNUM(atoi(buf));
#endif 
			continue;
		}
		if(!strcmp(command,"@mobstayterraintype"))
		{
#ifdef DRAGONBANE
			i=0;
			if((s=find_sub(subsector,word)))
			{
				strcpy(buf,s);
			}
			while(ax_sector_names[i])
			{
				if(!strcmp(ax_sector_names[i],buf))
				{
					pMobIndex->terrain=i; 
					break;
				}
				i++;
			}
			if(!ax_sector_names[i])
			{
				fprintf(axlog,"Unknown mobstayterraintype %s\n",buf);
			}
#endif
			continue;
		}
		if(!strcmp(command,"@mobhitroll"))
		{
			pMobIndex->hitroll=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobdamageroll"))
		{
			pMobIndex->damroll=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@mobbodyparts"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subbodypart,word)))
				{
					strcpy(word,s);
				}
				if((i=get_partflag(word)) > -1)
				{
					SET_BIT(pMobIndex->xflags,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobbodyparts: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobisresistant"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subrisflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_risflag(word)) > -1)
				{
					SET_BIT(pMobIndex->resistant,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobisresistant: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobisimmune"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subrisflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_risflag(word)) > -1)
				{
					SET_BIT(pMobIndex->immune,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobisimmune: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobissusceptible"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subrisflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_risflag(word)) > -1)
				{
					SET_BIT(pMobIndex->susceptible,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown mobissusceptible: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobattacks"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subattack,word)))
				{
					strcpy(word,s);
				}
				if((i=get_attackflag(word)) > -1)
				{
#ifdef SMAUG102
					SET_BIT(pMobIndex->attacks,1 << i);
#endif
#ifdef SMAUG14
					xSET_BIT(pMobIndex->attacks, i);
#endif
				}/* I have 2 attack bit vectors .. soo */
#ifdef DRAGONBANE
				else if((i=get_attackflag2(word)) > -1) 
				{
					SET_BIT(pMobIndex->attack2,1 << i);
				}
#endif
				else
			        {
					fprintf(axlog,"Unknown mobattacks: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobdefenses"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subdefense,word)))
				{
					strcpy(word,s);
				}
				if((i=get_defenseflag(word)) > -1)
				{
#ifdef SMAUG102
					SET_BIT(pMobIndex->defenses,1 << i);
#endif
#ifdef SMAUG14
					xSET_BIT(pMobIndex->defenses, i);
#endif
				}
				else
			        {
					fprintf(axlog,"Unknown mobdefenses: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@mobspecial"))
		{
			if((s=find_sub(subspec,buf)))
			{
				strcpy(buf,s);
			}
			if(spec_lookup(buf))
			{
				pMobIndex->spec_fun=spec_lookup(buf);
			}
			else
			{
				fprintf(axlog,"Unknown mobspecial: %s\n",buf);
			}
			continue;
			
		}
		if(!strcmp(command,"@mobprograms"))
		{
    			MPROG_DATA	*mprog = NULL;
			char *prog;
			int err=0;
			char progbuf[MAX_STRING_LENGTH];
			
			while(fgets(buff,1000,fp))
			{

				if(strrchr(buff,'\n'))
					*(strrchr(buff,'\n'))=0;
				if(strrchr(buff,'~'))
					*(strrchr(buff,'~'))=0;
				buf=strip_cr(buff);
				buf=One_Argument(buf,word);
				if(!strcmp(word,">"))
				{
					err=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subprogtrig,word)))
					{
						strcpy(word,s);
					}
					i = mprog_name_to_type(word);
					if(i==ERROR_PROG)
					{
						err=1;
						fprintf(axlog,"Unknown mob prog trigger %s\n",word);
						continue;
					}
					if(pMobIndex->mudprogs)
					{
						mprog->comlist=STRALLOC(progbuf);
					   	CREATE( mprog->next, MPROG_DATA, 1 );
					   	mprog = mprog->next;
					}
					else
					{
						CREATE( mprog, MPROG_DATA, 1 ); 
						pMobIndex->mudprogs=mprog;
					}
					progbuf[0]=0;
					mprog->type=i;
#ifdef SMAUG14
					xSET_BIT(pMobIndex->progtypes,mprog->type);
#else
					pMobIndex->progtypes = pMobIndex->progtypes | mprog->type;
#endif
					mprog->arglist=STRALLOC(buf);

				}
				else if(!strcmp(word,"|"))
				{
					mprog->comlist=STRALLOC(progbuf);
					mprog->next = NULL;
					break;
				}
				else
				{
					if(err)
					{
						continue;
					}
					sprintf(progbuf + strlen(progbuf),"%s\n",buff);
				}
			}
	  		for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
			{
				prog=make_in_prog(mprog->comlist,
					pMobIndex->player_name,
					pMobIndex->vnum,
					area->low_m_vnum,
					area->hi_m_vnum,
					axlog);
				STRFREE(mprog->comlist);
				mprog->comlist=STRALLOC(prog);
			}
			continue;
		}
		if(!strcmp(command,"#OBJECTS"))
		{
			continue;
		}
		if(!strcmp(command,"!objvnum"))
		{
			CREATE(pObjIndex,OBJ_INDEX_DATA,1);
			pObjIndex->vnum=IN_VNUM(atoi(buf));
			iHash		= pObjIndex->vnum % MAX_KEY_HASH;
			pObjIndex->next	= obj_index_hash[iHash];
			obj_index_hash[iHash]	= pObjIndex;
			top_obj_index++;
			pObjIndex->action_desc = STRALLOC("");
			continue;
		}
		if(!strcmp(command,"@objlevel"))
                {
			pObjIndex->level=IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@objkeywordname"))
		{
			pObjIndex->name = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@objinventoryname"))
		{
			pObjIndex->short_descr = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@objinroomname"))
		{
			strcat(buf,"\n\r");
			pObjIndex->description = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@objactiondescription"))
		{
			STRFREE(pObjIndex->action_desc);
			pObjIndex->action_desc = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@objitemtype"))
		{
			buf=One_Argument(buf,word);
			if((s=find_sub(subobjtype,word)))
			{
				strcpy(word,s);
			}
			if((i=get_otype(word)) > -1)
			{
				pObjIndex->item_type=i;
			}
			else
		        {
				fprintf(axlog,"Unknown objtype: %s\n",word);
			}	
			continue;
		}
		if(!strcmp(command,"@objflags"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subobjflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_oflag(word)) > -1)
				{
#ifdef SMAUG102
					SET_BIT(pObjIndex->extra_flags,1 << i);
#endif
#ifdef SMAUG14
					xSET_BIT(pObjIndex->extra_flags, i);
#endif
				}
				else if((i=get_mflag(word)) > -1)
				{
					SET_BIT(pObjIndex->magic_flags,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown objflag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@objwearflags"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subwearflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_wflag(word)) > -1)
				{
					SET_BIT(pObjIndex->wear_flags,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown wearflag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@objwearlayers"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subwearlayerflag,word)))
				{
					strcpy(word,s);
				}
				i=0;
				while(ax_layers[i])
				{
					if(!strcmp(word,ax_layers[i]))
					{
						SET_BIT(pObjIndex->layers,1 << i);
						break;
					}
					i++;
				}
				if(!ax_layers[i])
				{
					fprintf(axlog,"Unknown objwearlayer: %s\n",word);
				}
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if (!strcmp(command,"@objcurrac")
		|| !strcmp(command,"@objorgac")
		|| !strcmp(command,"@objcapacity")
		|| !strcmp(command,"@objcontainerflags")
		|| !strcmp(command,"@objkeyvnum")
		|| !strcmp(command,"@objcontainercondtn")
		|| !strcmp(command,"@objdrinkcapacity")
		|| !strcmp(command,"@objdrinkquantity")
		|| !strcmp(command,"@objliquidtype")
		|| !strcmp(command,"@objliquidpoisonflag")
		|| !strcmp(command,"@objfoodvalue")
		|| !strcmp(command,"@objfoodcondition")
		|| !strcmp(command,"@objfoodpoisonflag")
		|| !strcmp(command,"@objherbdoses")
		|| !strcmp(command,"@objherbname")
		|| !strcmp(command,"@objkeylockvnum")
		|| !strcmp(command,"@objtriggerflags")
		|| !strcmp(command,"@objtrigtoroom")
		|| !strcmp(command,"@objrandomizeroom")
		|| !strcmp(command,"@objtrigfromroom")
		|| !strcmp(command,"@objlighthrsleft")
		|| !strcmp(command,"@objnumbercoins")
		|| !strcmp(command,"@objcointype")
		|| !strcmp(command,"@objspelllevel")
		|| !strcmp(command,"@objspell1")
		|| !strcmp(command,"@objspell2")
		|| !strcmp(command,"@objspell3")
		|| !strcmp(command,"@objmaxcharge")
		|| !strcmp(command,"@objnumcharge")
		|| !strcmp(command,"@objchargedelay")
		|| !strcmp(command,"@objtrapcharges")
		|| !strcmp(command,"@objtraptype")
		|| !strcmp(command,"@objtraplevel")
		|| !strcmp(command,"@objtrapflag")
		|| !strcmp(command,"@objtreasuretype")
		|| !strcmp(command,"@objtreasurecond")
		|| !strcmp(command,"@objweaponcond")
		|| !strcmp(command,"@objweaponnumdice")
		|| !strcmp(command,"@objweaponsizedice")
		|| !strcmp(command,"@objweapontype")
		|| !strcmp(command,"@objpenusesleft"))
		{
		switch ( pObjIndex->item_type )
		{
			case ITEM_ARMOR:
				if(!strcmp(command,"@objcurrac"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objorgac"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				break;
			case ITEM_CONTAINER:
				if(!strcmp(command,"@objcapacity"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objcontainerflags"))
				{
					buf=One_Argument(buf,word);
					while(*word)
					{
						if((s=find_sub(subcontflag,word)))
						{
							strcpy(word,s);
						}

						if((i=get_cont_flag(word)) > -1)
							SET_BIT(pObjIndex->value[1], 1 << i);
						buf=One_Argument(buf,word);
					}
				}
				if(!strcmp(command,"@objkeyvnum"))
				{
					if(!strncmp(buf,"none",4))
					{
						pObjIndex->value[2]= -1;
						continue;
					}
					pObjIndex->value[2]=IN_VNUM(atoi(buf));
					if(pObjIndex->value[2] < area->low_o_vnum 
					|| pObjIndex->value[2] > area->hi_o_vnum)
					{
						if(pObjIndex->value[2] > 0)
							fprintf(axlog,"Key from outside the area for %d\n",
							pObjIndex->vnum);
					}
				}
				if(!strcmp(command,"@objcontainercondtn"))
				{
					pObjIndex->value[3]=atoi(buf);
				}
				break;
			case ITEM_DRINK_CON:
			case ITEM_FOUNTAIN:
				if(!strcmp(command,"@objdrinkcapacity"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objdrinkquantity"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				if(!strcmp(command,"@objliquidtype"))
				{
					i=0;
					
					buf=One_Argument(buf,word);
					if((s=find_sub(subliquid,word)))
					{
						strcpy(word,s);
					}
					while(i < LIQ_MAX)
					{
						if(!strcmp(word,liq_table[i].liq_name))
						{
							pObjIndex->value[2]=i;
							break;
						}
						i++;
					}
					if(i == LIQ_MAX)
					{
						fprintf(axlog,"Unknown liquid %s\n",word);
					}
				}
				if(!strcmp(command,"@objliquidpoisonflag"))
				{
					pObjIndex->value[3]=atoi(buf);
				}
				break;
			case ITEM_BLOOD:
				if(!strcmp(command,"@objdrinkcapacity"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objdrinkquantity"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				if(!strcmp(command,"@objliquidpoisonflag"))
				{ /* yep its three, not a boo boo */
					pObjIndex->value[3]=atoi(buf);
				}
				break;
			case ITEM_FOOD:
				if(!strcmp(command,"@objfoodvalue"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objfoodcondition"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				if(!strcmp(command,"@objfoodpoisonflag"))
				{
					pObjIndex->value[3]=atoi(buf);
				}
				break;
			case ITEM_HERB:
				if(!strcmp(command,"@objherbdoses"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objherbname"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subherb,word)))
					{
						strcpy(word,s);
					}
					while(i < top_herb)
					{
						if(!strcmp(word,herb_table[i]->name))
						{
							pObjIndex->value[2]=i;
							break;
						}
						i++;
					}
					if(i == top_herb)
					{
						fprintf(axlog,"Unknown herb %s\n",buf);
					}
				}
				break;
			case ITEM_KEY:
				if(!strcmp(command,"@objkeylockvnum"))
				{
					pObjIndex->value[0]=IN_VNUM(atoi(buf));
				}
				break;
			case ITEM_LEVER:
			case ITEM_SWITCH:
			case ITEM_BUTTON:
				if(!strcmp(command,"@objtriggerflags"))
				{
					buf=One_Argument(buf,word);
					while(*word)
					{
						if((s=find_sub(subtrigger,word)))
						{
							strcpy(word,s);
						}
						if((i=get_trigflag(word)) > -1)
						{
							SET_BIT(pObjIndex->value[0],1 << i);
						}
						else
			        		{
							fprintf(axlog,"Unknown triggerflag: %s\n",word);
						}	
						buf=One_Argument(buf,word);
					}
					continue;
				}
				if ( IS_SET( pObjIndex->value[0], TRIG_TELEPORT )
				||   IS_SET( pObjIndex->value[0], TRIG_TELEPORTALL )
				||   IS_SET( pObjIndex->value[0], TRIG_TELEPORTPLUS ) )
				{
					if(!strcmp(command,"@objtrigtoroom"))
					{
						pObjIndex->value[1]=IN_VNUM(atoi(buf));
					}
					break;
				}
				if ( IS_SET( pObjIndex->value[0], TRIG_RAND4 )
				    ||	 IS_SET( pObjIndex->value[0], TRIG_RAND6 ) )
				{
					if(!strcmp(command,"@objrandomizeroom"))
					{
						pObjIndex->value[1]=IN_VNUM(atoi(buf));
					}
					break;
				}
				if ( IS_SET( pObjIndex->value[0], TRIG_DOOR ) )
				{
					if(!strcmp(command,"@objtrigfromroom"))
					{
						pObjIndex->value[1]=IN_VNUM(atoi(buf));
					}
					if(!strcmp(command,"@objtrigtoroom"))
					{
						pObjIndex->value[2]=IN_VNUM(atoi(buf));
					}
				}

				break;
			case ITEM_LIGHT:
				if(!strcmp(command,"@objlighthrsleft"))
				{
					pObjIndex->value[2]=IN_VNUM(atoi(buf));
				}
				break;
			case ITEM_MONEY:
				if(!strcmp(command,"@objnumbercoins"))
				{
					pObjIndex->value[1]=GOLDADJ(atoi(buf));
				}
				if(!strcmp(command,"@objcointype"))
				{
					pObjIndex->value[2]=atoi(buf);
				}
				break;
			case ITEM_PILL:
			case ITEM_POTION:
			case ITEM_SCROLL:
				if(!strcmp(command,"@objspelllevel"))
				{
					pObjIndex->value[0]=IN_LEVEL(atof(buf));
				}
				if(!strcmp(command,"@objspell1"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[1]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
				if(!strcmp(command,"@objspell2"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[2]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
				if(!strcmp(command,"@objspell3"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[3]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
	    			break;
			case ITEM_STAFF:
			case ITEM_WAND:
				if(!strcmp(command,"@objspelllevel"))
				{
					pObjIndex->value[0]=IN_LEVEL(atof(buf));
				}
				if(!strcmp(command,"@objmaxcharge"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				if(!strcmp(command,"@objnumcharge"))
				{
					pObjIndex->value[2]=atoi(buf);
				}
				if(!strcmp(command,"@objspell1"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[3]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
	    			break;
			case ITEM_SALVE:
				if(!strcmp(command,"@objspelllevel"))
				{
					pObjIndex->value[0]=IN_LEVEL(atof(buf));
				}
				if(!strcmp(command,"@objmaxcharge"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				if(!strcmp(command,"@objnumcharge"))
				{
					pObjIndex->value[2]=atoi(buf);
				}
				if(!strcmp(command,"@objchargedelay"))
				{
					pObjIndex->value[3]=atoi(buf);
				}
				if(!strcmp(command,"@objspell1"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[4]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
				if(!strcmp(command,"@objspell2"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subspell,word)))
					{
						strcpy(word,s);
					}
					while(i < top_sn)
					{
						if(!strcmp(word,skill_table[i]->name))
						{
							pObjIndex->value[5]=i;
							break;
						}
						i++;
					}
					if(i == top_sn)
					{
						fprintf(axlog,"Unknown spell %s\n",buf);
					}
				}
	    			break;
			case ITEM_TRAP:
				if(!strcmp(command,"@objtrapcharges"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objtraptype"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subtraptype,word)))
					{
						strcpy(word,s);
					}
					while(ax_traps[i])
					{
						if(!strcmp(word,ax_traps[i]))
						{
							pObjIndex->value[1]=i;
							break;
						}
						i++;
					}
					if(!ax_traps[i])
					{
						fprintf(axlog,"Unknown traptype %s\n",buf);
					}
				}
				if(!strcmp(command,"@objtraplevel"))
				{
					pObjIndex->value[2]=IN_LEVEL(atof(buf));
				}
				if(!strcmp(command,"@objtrapflag"))
				{
					buf=One_Argument(buf,word);
					while(*word)
					{
						if((s=find_sub(subtrapflag,word)))
						{
							strcpy(word,s);
						}
						if((i=get_trapflag(word)) > -1)
						{
							SET_BIT(pObjIndex->value[3],1 << i);
						}
						else
			        		{
							fprintf(axlog,"Unknown trapflag: %s\n",word);
						}	
						buf=One_Argument(buf,word);
					}
					continue;
				}
				break;
			case ITEM_TREASURE:
				if(!strcmp(command,"@objtreasuretype"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objtreasurecond"))
				{
					pObjIndex->value[1]=atoi(buf);
				}
				break;
			case ITEM_WEAPON:
				if(!strcmp(command,"@objweaponcond"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				if(!strcmp(command,"@objweaponnumdice"))
				{
					pObjIndex->value[1]=IN_DICE(atof(buf));
				}
				if(!strcmp(command,"@objweaponsizedice"))
				{
					pObjIndex->value[2]=IN_DICE(atof(buf));
				}
				if(!strcmp(command,"@objweapontype"))
				{
					i=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subweapontype,word)))
					{
						strcpy(word,s);
					}
					while(i < MAX_WEAPONTYPE)
					{
						if(!strcmp(word,attack_table[i]))
						{
							pObjIndex->value[3]=i;
							break;
						}
						i++;
					}
					if(i == MAX_WEAPONTYPE)
					{
						fprintf(axlog,"Unknown weapontype %s\n",word);
					}
				}
				break;
			case ITEM_PEN:
				if(!strcmp(command,"@objpenusesleft"))
				{
					pObjIndex->value[0]=atoi(buf);
				}
				break;
			case ITEM_FURNITURE:
			case ITEM_TRASH:
			case ITEM_PAPER:
			case ITEM_BOAT:
			case ITEM_FIRE:
			case ITEM_WORN:
			case ITEM_BOOK:
				break;
			default:
                                display(o_types[pObjIndex->item_type]);
				display("-Unknown itemtype for values\n\r");
				fprintf(axlog,"Unknown itemtype in switch\n");
				break;

		}
		continue;
		}

		if(!strcmp(command,"@objweight"))
		{
			pObjIndex->weight=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@objcost"))
		{
			pObjIndex->cost=GOLDADJ(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@objrent"))
		{
			pObjIndex->rent=GOLDADJ(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@objextradesc"))
		{
			EXTRA_DESCR_DATA *ed;

			CREATE( ed, EXTRA_DESCR_DATA, 1 );
			ed->keyword		= STRALLOC(buf);
			ed->description		= fread_string( fp );
			LINK( ed, pObjIndex->first_extradesc, 
				pObjIndex->last_extradesc, next, prev );
			top_ed++;
			continue;
		}

		if(!strcmp(command,"@objaffects"))
		{
			AFFECT_DATA *paf;

			buf=One_Argument(buf,word);
			if((s=find_sub(subobjaffects,word)))
			{
				strcpy(word,s);
			}
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= -1;
			paf->duration		= -1;
			i=0;
			while(i < MAX_APPLY_TYPE)
			{
				if(!strcmp(word,a_types[i]))
				{
					paf->location=i;
					break;
				}
				i++;
			}
			if(i == MAX_APPLY_TYPE)
			{
				fprintf(axlog,"Unknown objaffects %s\n",word);
				DISPOSE(paf);
				continue;
			}
#ifdef SMAUG102
			paf->bitvector		= 0;
#endif
#ifdef SMAUG14
			xCLEAR_BITS(paf->bitvector);
#endif
			paf->modifier		= atoi(buf);
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
			top_affect++;
			continue;
		}
		if(!strcmp(command,"@objaffectris"))
		{
			AFFECT_DATA *paf;

			buf=One_Argument(buf,word);
			if((s=find_sub(subobjaffects,word)))
			{
				strcpy(word,s);
			}
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= -1;
			paf->duration		= -1;
			i=0;

			while(i < MAX_APPLY_TYPE)
			{
				if(!strcmp(word,a_types[i]))
				{
					paf->location=i;
					break;
				}
				i++;
			}
			if(i == MAX_APPLY_TYPE)
			{
				fprintf(axlog,"Unknown objapplytype %s\n",word);
				DISPOSE(paf);
				continue;
			}
#ifdef SMAUG102
			paf->bitvector		= 0;
#endif
#ifdef SMAUG14
			xCLEAR_BITS(paf->bitvector);
#endif
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subrisflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_risflag(word)) < 0)
				{
					fprintf(axlog,"Unknown objapplyris %s\n",word);
					DISPOSE(paf);
					buf=One_Argument(buf,word);
					continue;
				}
				SET_BIT(paf->modifier,1 << i);
				LINK( paf, pObjIndex->first_affect, 
					pObjIndex->last_affect,
					   next, prev );
				top_affect++;
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@objapplyaffect"))
		{
			AFFECT_DATA *paf;

			buf=One_Argument(buf,word);
			if((s=find_sub(subobjaffects,word)))
			{
				strcpy(word,s);
			}
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= -1;
			paf->duration		= -1;
			i=0;

			while(i < MAX_APPLY_TYPE)
			{
				if(!strcmp(word,a_types[i]))
				{
					paf->location=i;
					break;
				}
				i++;
			}
			if(i == MAX_APPLY_TYPE)
			{
				fprintf(axlog,"Unknown objapplytype %s\n",word);
				DISPOSE(paf);
				continue;
			}
#ifdef SMAUG102
			paf->bitvector		= 0;
#endif
#ifdef SMAUG14
			xCLEAR_BITS(paf->bitvector);
#endif
			if((s=find_sub(subaffected,buf)))
			{
				strcpy(buf,s);
			}
			if((i=get_aflag(buf)) < 0)
			{
				fprintf(axlog,"Unknown objapplyaffect %s\n",buf);
				DISPOSE(paf);
				continue;
			}
			SET_BIT(paf->modifier,1 << i);
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
			top_affect++;
			continue;
		}
		if(!strcmp(command,"@objcastspell"))
		{
			AFFECT_DATA *paf;

			buf=One_Argument(buf,word);
			if((s=find_sub(subobjaffects,word)))
			{
				strcpy(word,s);
			}
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= -1;
			paf->duration		= -1;
			i=0;

			while(i < MAX_APPLY_TYPE)
			{
				if(!strcmp(word,a_types[i]))
				{
					paf->location=i;
					break;
				}
				i++;
			}
			if(i == MAX_APPLY_TYPE)
			{
				fprintf(axlog,"Unknown objapplytype %s\n",word);
				DISPOSE(paf);
				continue;
			}
#ifdef SMAUG102
			paf->bitvector		= 0;
#endif
#ifdef SMAUG14
			xCLEAR_BITS(paf->bitvector);
#endif
			buf=One_Argument(buf,word);
			if((s=find_sub(subspell,word)))
			{
				strcpy(word,s);
			}
			i=0;
			while(i < top_sn)
			{
				if(!strcmp(word,skill_table[i]->name))
				{
					break;
				}
				i++;
			}
			if(i==top_sn)
			{
				fprintf(axlog,"Unknown objcastspell %s\n",word);
				DISPOSE(paf);
				continue;
			}
			paf->modifier=i;
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
			top_affect++;
			continue;
		}
		if(!strcmp(command,"@objprograms"))
		{
    			MPROG_DATA	*mprog = NULL;
			char *prog;
			int err=0;
			char progbuf[MAX_STRING_LENGTH];
			
			while(fgets(buff,1000,fp))
			{

				if(strrchr(buff,'\n'))
					*(strrchr(buff,'\n'))=0;
				if(strrchr(buff,'~'))
					*(strrchr(buff,'~'))=' ';
				buf=strip_cr(buff);
				buf=One_Argument(buf,word);
				if(!strcmp(word,">"))
				{
					err=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subprogtrig,word)))
					{
						strcpy(word,s);
					}
					i = mprog_name_to_type(word);
					if(i==ERROR_PROG)
					{
						err=1;
						fprintf(axlog,"Unknown obj prog trigger %s\n",word);
						continue;
					}
					if(pObjIndex->mudprogs)
					{
						mprog->comlist=STRALLOC(progbuf);
					   	CREATE( mprog->next, MPROG_DATA, 1 );
					   	mprog = mprog->next;
					}
					else
					{
						CREATE( mprog, MPROG_DATA, 1 ); 
						pObjIndex->mudprogs=mprog;
					}
					progbuf[0]=0;
					mprog->type=i;
#ifdef SMAUG14
					xSET_BIT(pObjIndex->progtypes,mprog->type);
#else
					pObjIndex->progtypes = pObjIndex->progtypes | mprog->type;
#endif
					mprog->arglist=STRALLOC(buf);

				}
				else if(!strcmp(word,"|"))
				{
					mprog->comlist=STRALLOC(progbuf);
					mprog->next = NULL;
					break;
				}
				else
				{
					if(err)
					{
						continue;
					}
					sprintf(progbuf + strlen(progbuf),"%s\n",buff);
				}
			}
	  		for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
			{
				prog=make_in_prog(mprog->comlist,
					pObjIndex->name,
					pObjIndex->vnum,
					area->low_o_vnum,
					area->hi_o_vnum,
					axlog);
				STRFREE(mprog->comlist);
				mprog->comlist=STRALLOC(prog);
			}
			continue;
		}
		if(!strcmp(command,"#ROOMS"))
		{
			continue;
		}
		if(!strcmp(command,"!roomvnum"))
		{
			CREATE(pRoomIndex,ROOM_INDEX_DATA,1);
			pRoomIndex->vnum=IN_VNUM(atoi(buf));
			pRoomIndex->area=area;
			iHash			= pRoomIndex->vnum % MAX_KEY_HASH;
			pRoomIndex->next	= room_index_hash[iHash];
			room_index_hash[iHash]	= pRoomIndex;
			top_room++;
			continue;
		}
		if(!strcmp(command,"@roomname"))
		{
			pRoomIndex->name = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@roomdescription"))
		{
			pRoomIndex->description=fread_string(fp);
			continue;
		}
		if(!strcmp(command,"@roomflags"))
		{
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subroomflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_rflag(word)) > -1)
				{
					SET_BIT(pRoomIndex->room_flags,1 << i);
				}
#ifdef DRAGONBANE
				else if((i=get_rflag2(word)) > -1)
				{
					SET_BIT(pRoomIndex->room_flag2,1 << i);
				}
#endif
				else
			        {
					fprintf(axlog,"Unknown room flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@roomsectortype"))
		{
			i=0;
			if((s=find_sub(subsector,buf)))
			{
				strcpy(buf,s);
			}
			while(ax_sector_names[i])
			{
				if(!strcmp(ax_sector_names[i],buf))
				{
					pRoomIndex->sector_type=i; 
					break;
				}
				i++;
			}
			if(!ax_sector_names[i])
			{
				fprintf(axlog,"Unknown roomsectortype %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@roomteleportdelay"))
		{
			pRoomIndex->tele_delay = atoi(buf);
			continue;
		}
		if(!strcmp(command,"@roomteleportvnum"))
		{
			pRoomIndex->tele_vnum = IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@roomtunnelcapacity"))
		{
			pRoomIndex->tunnel = atoi(buf);
			continue;
		}
#ifdef USED
		if(!strcmp(command,"@roommanatype"))
		{
			continue;
		}
		if(!strcmp(command,"@roommanaamount"))
		{
			continue;
		}
#endif
		if(!strcmp(command,"^roomexit"))
		{
			if((s=find_sub(subexitdir,buf)))
			{
				strcpy(buf,s);
			}
			if((door=get_dir(buf)) < 0 )
			{
				fprintf(axlog,"Unknown roomexit direction %s\n",buf);
				continue;
			}
			pExit = make_exit( pRoomIndex, NULL, door );
		  	pExit->vdir		= door;
		  	pExit->key		= -1;
			pExit->description = STRALLOC("");
			pExit->keyword = STRALLOC("");
		}
		if(!strcmp(command,"@roomexitdesc"))
		{
			if(door < 0) 
			{
				continue;
			}
			STRFREE(pExit->description);
			pExit->description = fread_string(fp);
			continue;
		}

		if(!strcmp(command,"@roomexitkeyword"))
		{
			if(door < 0) 
			{
				continue;
			}
			STRFREE(pExit->keyword);
			pExit->keyword = STRALLOC(buf);
			continue;
		}
		if(!strcmp(command,"@roomexitexitinfo"))
		{
			if(door < 0) 
			{
				continue;
			}
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subexitflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_exflag(word)) > -1)
				{
					SET_BIT(pExit->exit_info,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown room exitinfo flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@roomexitkey"))
		{
			if(door < 0) 
			{
				continue;
			}
			if(!strncmp(buf,"none",4))
                        {
				pExit->key = -1;
				continue;
			}
			pExit->key=IN_VNUM(atoi(buf));
			if(pExit->key < area->low_o_vnum 
			|| pExit->key > area->hi_o_vnum)
			{
				if(pExit->key > 0)
				fprintf(axlog,"Key from outside the area in room %d - exit %s",
					pRoomIndex->vnum,dir_name[door]);
			}
			continue;
		}
		if(!strcmp(command,"@roomexittovnum"))
		{
			if(door < 0) 
			{
				continue;
			}
			pExit->vnum=IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@roomexitdistance"))
		{
			if(door < 0) 
			{
				continue;
			}
			pExit->distance = atoi(buf);
			continue;
		}
		if(!strcmp(command,"@roomislinktoexternalarea"))
		{
			buf=One_Argument(buf,word);
			fprintf(axlog,"%s ",word);
			buf=One_Argument(buf,word);
			fprintf(axlog,"%d ",IN_VNUM(atoi(word)));
			fprintf(axlog,"%s\n",buf);
			if(noextlink)
			{
				extract_exit(pRoomIndex,pExit);
			}
			continue;
		}
		if(!strcmp(command,"@roomhaslinkfromexternalarea"))
		{
			buf=One_Argument(buf,word);
			fprintf(axlog,"%s ",word);
			buf=One_Argument(buf,word);
			fprintf(axlog,"%d ",IN_VNUM(atoi(word)));
			fprintf(axlog,"%s\n",buf);
			continue;
		}
		if(!strcmp(command,"@roomextradesckeywords"))
		{
			EXTRA_DESCR_DATA *ed;

			CREATE( ed, EXTRA_DESCR_DATA, 1 );
			ed->keyword		= STRALLOC(buf);
			ed->description		= fread_string( fp );
			LINK( ed, pRoomIndex->first_extradesc, 
				pRoomIndex->last_extradesc, next, prev );
			top_ed++;
			continue;
		}
		if(!strcmp(command,"@roomprograms"))
		{
    			MPROG_DATA	*mprog = NULL;
			char *prog;
			int err=0;
			char progbuf[MAX_STRING_LENGTH];
			
			while(fgets(buff,1000,fp))
			{

				if(strrchr(buff,'\n'))
					*(strrchr(buff,'\n'))=0;
				if(strrchr(buff,'~'))
					*(strrchr(buff,'~'))=' ';
				buf=strip_cr(buff);
				buf=One_Argument(buf,word);
				if(!strcmp(word,">"))
				{
					err=0;
					buf=One_Argument(buf,word);
					if((s=find_sub(subprogtrig,word)))
					{
						strcpy(word,s);
					}
					i = mprog_name_to_type(word);
					if(i==ERROR_PROG)
					{
						err=1;
						fprintf(axlog,"Unknown room prog trigger %s\n",word);
						continue;
					}
					if(pRoomIndex->mudprogs)
					{
						mprog->comlist=STRALLOC(progbuf);
					   	CREATE( mprog->next, MPROG_DATA, 1 );
					   	mprog = mprog->next;
					}
					else
					{
						CREATE( mprog, MPROG_DATA, 1 ); 
						pRoomIndex->mudprogs=mprog;
					}
					progbuf[0]=0;
					mprog->type=i;
#ifdef SMAUG14
					xSET_BIT(pRoomIndex->progtypes,mprog->type);
#else
					pRoomIndex->progtypes = pRoomIndex->progtypes | mprog->type;
#endif
					mprog->arglist=STRALLOC(buf);

				}
				else if(!strcmp(word,"|"))
				{
					mprog->comlist=STRALLOC(progbuf);
					mprog->next = NULL;
					break;
				}
				else
				{
					if(err)
					{
						continue;
					}
					sprintf(progbuf + strlen(progbuf),"%s\n",buff);
				}
			}
	  		for ( mprog = pRoomIndex->mudprogs; mprog; mprog = mprog->next )
			{
				prog=make_in_prog(mprog->comlist,
					pRoomIndex->name,
					pRoomIndex->vnum,
					area->low_r_vnum,
					area->hi_r_vnum,
					axlog);
				STRFREE(mprog->comlist);
				mprog->comlist=STRALLOC(prog);
			}
			continue;
		}
		if(!strcmp(command,"@roomexitwordcheck"))
		{
			char *d;
			int dir,found;
			EXIT_DATA *xit;
			d=pRoomIndex->description;
			d=one_argument(d,word); /* yeppers, the native one */
			while(*word)
			{
				if(!str_cmp(word,"n")
				|| !str_cmp(word,"e")
				|| !str_cmp(word,"w")
				|| !str_cmp(word,"s")
				|| !str_cmp(word,"u")
				|| !str_cmp(word,"d")
				|| !str_cmp(word,"ne")
				|| !str_cmp(word,"nw")
				|| !str_cmp(word,"se")
				|| !str_cmp(word,"sw")
				|| !str_cmp(word,"north")
				|| !str_cmp(word,"east")
				|| !str_cmp(word,"west")
				|| !str_cmp(word,"south")
				|| !str_cmp(word,"up")
				|| !str_cmp(word,"down")
				|| !str_cmp(word,"northeast")
				|| !str_cmp(word,"northwest")
				|| !str_cmp(word,"southeast")
				|| !str_cmp(word,"southwest"))
				{
					if(!str_cmp(word,"n"))
						strcpy(word,"north");
					if(!str_cmp(word,"e"))
						strcpy(word,"east");
					if(!str_cmp(word,"w"))
						strcpy(word,"west");
					if(!str_cmp(word,"s"))
						strcpy(word,"south");
					if(!str_cmp(word,"u"))
						strcpy(word,"up");
					if(!str_cmp(word,"d"))
						strcpy(word,"down");
					if(!str_cmp(word,"ne"))
						strcpy(word,"northeast");
					if(!str_cmp(word,"nw"))
						strcpy(word,"northwest");
					if(!str_cmp(word,"se"))
						strcpy(word,"southeast");
					if(!str_cmp(word,"sw"))
						strcpy(word,"southwest");
					dir=get_dir(word);
					found=0;
					for ( xit = pRoomIndex->first_exit; xit; xit = xit->next )
					{
						if(xit->vdir == dir)
						{
							found=1;
							break;
						}
					}
					if(!found)
					{
						fprintf(axlog,"******** Warning room %d has a reference to %s but has no exit that way\n",
							pRoomIndex->vnum,word);
						fprintf(axlog,"%s\n",strip_cr(pRoomIndex->description));
					}
				}
				d=one_argument(d,word);
			}
			continue;	
		}
		if(!strcmp(command,"#RESETS"))
		{
			continue;
		}
		if(!strcmp(command,"!mobresetvnum"))
		{
			letter='M';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			pMobIndex=get_mob_index(arg1);
			continue;
		}
		if(!strcmp(command,"@mobresetinroom"))
		{
			arg3=IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@mobresetlimit"))
		{
			arg2=atoi(buf);
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"!objresetvnum"))
		{
			letter='O';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			pObjIndex=get_obj_index(arg1);
			continue;
		}
		if(!strcmp(command,"@objresetinroom"))
		{
			arg3=IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@objresetlimit"))
		{
			arg2=atoi(buf);
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		
		if(!strcmp(command,"!putobjvnum"))
		{
			letter='P';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			pObjIndex=get_obj_index(arg1);
			continue;
		}
		if(!strcmp(command,"@putobjinobjvnum"))
		{
			arg3=IN_VNUM(atoi(buf));
			continue;
		}
		if(!strcmp(command,"@putobjunderobjvnum"))
		{
			arg3=0;
			continue;
		}
		if(!strcmp(command,"@putobjresetlimit"))
		{
			arg2=atoi(buf);
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"!equipobjvnum"))
		{
			letter='E';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			continue;
		}
		if(!strcmp(command,"@equipwearlocation"))
		{
			if((s=find_sub(subwearloc,buf)))
			{
				strcpy(buf,s);
			}
			if((i=get_wearloc(buf)) > -1)
			{
				arg3=i;
				add_reset(area,letter,extra,arg1,arg2,arg3);
			}
			else
			{
				fprintf(axlog,"Unknown equipwearlocation %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"!doorinroomvnum"))
		{
			letter='D';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			continue;
		}
		if(!strcmp(command,"@doordirection"))
		{
			if((s=find_sub(subexitdir,buf)))
			{
				strcpy(buf,s);
			}
			if((i=get_dir(buf)) > -1 )
			{
				arg2=i;
			}
			else
			{
				fprintf(axlog,"Unknown reset doordirection %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@doorstate"))
		{
			if(i < 0) /* check if doordirection failed */
			{
				continue;
			}
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subexitflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_exflag(word)) > -1)
				{
					SET_BIT(arg3,1 << i);
				}
				else
			        {
					fprintf(axlog,"Unknown room exitinfo flag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"!trapflag"))
		{
			letter='T';
			extra=0;
			arg2=0;
			arg3=0;
			buf=One_Argument(buf,word);
			while(*word)
			{
				if((s=find_sub(subtrapflag,word)))
				{
					strcpy(word,s);
				}
				if((i=get_trapflag(word)) > -1)
				{
					SET_BIT(extra,1 << i);
				}
				else
	        		{
					fprintf(axlog,"Unknown reset trapflag: %s\n",word);
				}	
				buf=One_Argument(buf,word);
			}
			continue;
		}
		if(!strcmp(command,"@traptype"))
		{
			i=0;
			buf=One_Argument(buf,word);
			if((s=find_sub(subtraptype,word)))
			{
				strcpy(word,s);
			}
			while(ax_traps[i])
			{
				if(!strcmp(word,ax_traps[i]))
				{
					arg1=i;
					break;
				}
				i++;
			}
			if(!ax_traps[i])
			{
				fprintf(axlog,"Unknown reset traptype %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@trapcharges"))
		{
			arg2=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@trapvnum"))
		{
			arg3=IN_VNUM(atoi(buf));
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"!giveobj"))
		{
			letter='G';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			continue;
		}
		if(!strcmp(command,"@giveobjresetlimit"))
		{
			arg2=atoi(buf);
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"!randomexitsforroom"))
		{
			letter='R';
			extra=0;
			arg1=IN_VNUM(atoi(buf));
			arg2=0;
			arg3=0;
			continue;
		}
		if(!strcmp(command,"@randomexitlastdoor"))
		{
			arg2=atoi(buf);
			add_reset(area,letter,extra,arg1,arg2,arg3);
			continue;
		}
		if(!strcmp(command,"#SHOPS"))
		{
			continue;
		}
		if(!strcmp(command,"!shopkeepervnum"))
		{
			CREATE(pShop,SHOP_DATA,1);
			pShop->keeper=IN_VNUM(atoi(buf));
			if ( (pMobIndex = get_mob_index(pShop->keeper)) != NULL )
			{
				pMobIndex->pShop=pShop;
			}
			else
			{
				DISPOSE(pShop);
			}
			buyslot=0;
			continue;
		}
		if(!strcmp(command,"@shoptradewith"))
		{
			continue;
		}
		if(!strcmp(command,"@shopbuys"))
		{
			if ( (pShop = pMobIndex->pShop) == NULL )
			{
	  			continue;
			}
			if(buyslot >= MAX_SHOP_BUY_SLOT)
			{
				continue;
			}
			buf=One_Argument(buf,word);
			if((s=find_sub(subobjtype,word)))
			{
				strcpy(word,s);
			}
			if((i=get_otype(word)) > -1)
			{
				pShop->buy_type[buyslot++]=i;
			}
			else
		        {
				fprintf(axlog,"Unknown shop objtype: %s\n",word);
			}	
			continue;
		}
		if(!strcmp(command,"@shoptype"))
		{
			/* this is purely documentary for the .aex */
			continue;
		}
		if(!strcmp(command,"@shopprofitbuy"))
		{
			if ( (pShop = pMobIndex->pShop) == NULL )
			{
	  			continue;
			}
			pShop->profit_buy=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@shopprofitsell"))
		{
			if ( (pShop = pMobIndex->pShop) == NULL )
			{
	  			continue;
			}
			pShop->profit_sell=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@shopopenhour"))
		{
			if ( (pShop = pMobIndex->pShop) == NULL )
			{
	  			continue;
			}
			pShop->open_hour=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@shopclosehour"))
		{
			if ( (pShop = pMobIndex->pShop) == NULL )
			{
	  			continue;
			}
			pShop->close_hour=atoi(buf);
			if ( !first_shop )
			{
	    			first_shop		= pShop;
			}
			else
			{
	    			last_shop->next	= pShop;
			}
			pShop->next = NULL;
			pShop->prev = last_shop;
			last_shop = pShop;
			top_shop++;
			continue;
		}
		if(!strcmp(command,"#REPAIRS"))
		{
			continue;
		}
		if(!strcmp(command,"!shoprepairvnum"))
		{
			CREATE(rShop,REPAIR_DATA,1);
			rShop->keeper=IN_VNUM(atoi(buf));
			if ( (pMobIndex = get_mob_index(rShop->keeper)) != NULL )
			{
				pMobIndex->rShop=rShop;
			}
			else
			{
				DISPOSE(rShop);
			}
			buyslot=0;
			continue;
		}
		if(!strcmp(command,"@shoprepairs"))
		{
			if ( (rShop = pMobIndex->rShop) == NULL )
			{
	  			continue;
			}
			if(buyslot >= MAX_SHOP_FIX_SLOT)
			{
				continue;
			}
			buf=One_Argument(buf,word);
			if((s=find_sub(subobjtype,word)))
			{
				strcpy(word,s);
			}
			if((i=get_otype(word)) > -1)
			{
				rShop->fix_type[buyslot++]=i;
			}
			else
		        {
				fprintf(axlog,"Unknown repair objtype: %s\n",word);
			}	
			continue;
		}
		if(!strcmp(command,"@shopprofitfix"))
		{
			if ( (rShop = pMobIndex->rShop) == NULL )
			{
	  			continue;
			}
			rShop->profit_fix=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@shoprepairtype"))
		{
			if ( (rShop = pMobIndex->rShop) == NULL )
			{
	  			continue;
			}
			i=0;
			while(ax_shoptypes[i])
			{
				if(!strcmp(buf,ax_shoptypes[i]))
				{
					break;
				}
				i++;
			}
			if(ax_shoptypes[i])
			{
				rShop->shop_type=atoi(buf);
			}
			else
			{
				fprintf(axlog,"Unknown shoprepairtype %s\n",buf);
			}
			continue;
		}
		if(!strcmp(command,"@shoprepairopenhour"))
		{
			if ( (rShop = pMobIndex->rShop) == NULL )
			{
	  			continue;
			}
			rShop->open_hour=atoi(buf);
			continue;
		}
		if(!strcmp(command,"@shoprepairclosehour"))
		{
			if ( (rShop = pMobIndex->rShop) == NULL )
			{
	  			continue;
			}
			rShop->close_hour=atoi(buf);
			if ( !first_repair )
			{
	    			first_repair = rShop;
			}
			else
			{
	    			last_repair->next = rShop;
			}
			rShop->next = NULL;
			rShop->prev = last_repair;
			last_repair = rShop;
			top_repair++;
			continue;
		}
		if(!strcmp(command,"#END"))
		{
			continue;
		}
		if(command[0]=='#' 
		|| command[0]=='!' 
		|| command[0]=='@') 
		{
			display("Unknown field ");
			display(command);
			display("\n\r");
			fprintf(axlog,"Unknown field: %s\n",command);
		}
        }
#ifdef SMAUG14
    /* initialize weather data - FB */
	CREATE(area->weather, WEATHER_DATA, 1);
    	area->weather->temp = 0;
    	area->weather->precip = 0;
    	area->weather->wind = 0;
    	area->weather->temp_vector = 0;
    	area->weather->precip_vector = 0;
    	area->weather->wind_vector = 0;
    	area->weather->climate_temp = 2;
    	area->weather->climate_precip = 2;
    	area->weather->climate_wind = 2;
    	area->weather->first_neighbor = NULL;
    	area->weather->last_neighbor = NULL;
    	area->weather->echo = NULL;
    	area->weather->echo_color = AT_GREY;
#endif
	sort_area(area,TRUE);
	reset_area(area);
	SET_BIT(area->status,AREA_LOADED);
	fix_exits();
	area_update();
        fBootDb=FALSE;
	fclose(axlog);
	fclose(fp);
	clean_subs();
	display("done\n\r");
}
void do_ax_export(CHAR_DATA *ch, char *argument)
{
    	RESET_DATA	*treset;
    	ROOM_INDEX_DATA	*room;
    	MOB_INDEX_DATA	*pMobIndex;
    	OBJ_INDEX_DATA	*pObjIndex;
    	MPROG_DATA	*mprog;
	OBJ_DATA        *obj;
    	EXIT_DATA	*xit;
    	EXTRA_DESCR_DATA *ed;
    	AFFECT_DATA	*paf;
    	SHOP_DATA	*pShop;
    	REPAIR_DATA	*pRepair;
    	AREA_DATA	*tarea;
    	char		 buf[MAX_STRING_LENGTH];
    	FILE		*fpout;
    	int		 vnum,i,currext;
    	int		 val0, val1, val2, val3, val4, val5;
	int 		lastobj=0;
	char 		filename[80];

	if(! *argument)
	{
		display(version);
		display("axexport <filename.aex>\n\r");
		return;
	}
        argument=one_argument(argument,filename);
	sprintf(buf,"../exchange/%s",filename);
	if((fpout=fopen(buf,"w"))==NULL)
	{
		display("Can't open ");
		display(buf);
		display("\n\r");
		return;	
	}
    	tarea=ch->in_room->area;	
    	sprintf( buf, "exporting %s...", tarea->filename );
	find_externals(tarea);
    	log_string_plus( buf, LOG_NORMAL, LEVEL_GREATER );
	startvnum=tarea->low_r_vnum;
        fprintf( fpout, "#EXPORTEDBY %s\n",strip_cr(version));
    	fprintf( fpout, "#AREANAME   %s\n", tarea->name );
    	fprintf( fpout, "#STARTVNUM   %d\n", startvnum );
	
    	fprintf( fpout, "#AUTHOR %s\n", tarea->author );
    	fprintf( fpout, "#SOFTRANGE %f %f\n", OUT_LEVEL(tarea->low_soft_range),
				     OUT_LEVEL(tarea->hi_soft_range));
    	fprintf( fpout, "#HARDRANGE %f %f\n",
		 		     OUT_LEVEL(tarea->low_hard_range),
			   	     OUT_LEVEL(tarea->hi_hard_range));
	fprintf(fpout, "#ROOMRANGE %d %d\n",
		OUT_VNUM(tarea->low_r_vnum),OUT_VNUM(tarea->hi_r_vnum));
	fprintf(fpout, "#MOBRANGE %d %d\n",
		OUT_VNUM(tarea->low_m_vnum),OUT_VNUM(tarea->hi_m_vnum)+addmobrange);
	fprintf(fpout, "#OBJRANGE %d %d\n",
		OUT_VNUM(tarea->low_o_vnum),OUT_VNUM(tarea->hi_o_vnum)+addobjrange);
    	if ( tarea->resetmsg )	/* Rennard */
		fprintf( fpout, "#RESETMSG %s\n", tarea->resetmsg );
    	if ( tarea->reset_frequency )
		fprintf( fpout, "#RESETFREQUENCY %d\n",
			tarea->reset_frequency );
#ifdef USED
	fprintf(fpout, "#RECALL %d\n",OUT_VNUM(tarea->recall));
#endif
	fprintf( fpout, "#AREAFLAGS %s\n", flag_string(tarea->flags,area_flags) );
	
    	fprintf( fpout, "#ECONOMY %d %d\n\n", tarea->high_economy, tarea->low_economy );
	
    	/* save mobiles */
    	fprintf( fpout, "#MOBILES\n" );
	newrange=tarea->hi_m_vnum+1;
	currext=0;
	for (vnum=extmob[currext];currext < top_extmob; vnum=extmob[currext])
//    	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    	{
		currext++;
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  		continue;
#ifdef DRAGONBANE
        	if (IS_SET(pMobIndex->act,ACT_DELETED))
	  		continue;
#endif
		vnum=getmobext(tarea,vnum);
		fprintf( fpout, "!mobvnum %d\n",OUT_VNUM(vnum));
		fprintf( fpout, "@mobkeywordname %s\n",pMobIndex->player_name		);
		fprintf( fpout,	"@mobactionname %s\n",	pMobIndex->short_descr		);
		fprintf( fpout,	"@mobinroomname %s\n",	strip_cr(pMobIndex->long_descr)	);
		fprintf( fpout,	"@moblookdescription\n");
		fprintf( fpout, "%s\n",	strip_cr(pMobIndex->description));
        	fprintf( fpout,"~\n");
#ifdef DRAGONBANE
        	if(pMobIndex->talk)
		{
			fprintf( fpout,	"@mobtalkreply\n");
			fprintf(fpout,"%s\n",strip_cr(pMobIndex->talk));
       		 	fprintf( fpout,"~\n");
		}
#endif
#ifdef SMAUG102
		fprintf( fpout, "@mobactflags %s\n",flag_string(pMobIndex->act,act_flags));
#endif
#ifdef SMAUG14
		fprintf( fpout, "@mobactflags %s\n",ext_flag_string(&pMobIndex->act,act_flags));
#endif
#ifdef SMAUG102
		fprintf( fpout, "@mobaffectedby %s\n",flag_string(pMobIndex->affected_by,a_flags));
#endif
#ifdef SMAUG14
		fprintf( fpout, "@mobaffectedby %s\n",ext_flag_string(&pMobIndex->affected_by,a_flags));
#endif
		fprintf( fpout, "@mobalignment %f\n",(float) pMobIndex->alignment/MAX_ALIGNMENT);
		fprintf( fpout, "@moblevel %f\n",OUT_LEVEL(pMobIndex->level));
		fprintf( fpout, "@mobthac0 %d\n",pMobIndex->mobthac0);
		fprintf( fpout, "@mobarmorclass %f\n",(float)pMobIndex->ac/MAX_MOBARMORCLASS);
		fprintf( fpout, "@mobnumberofhitdice %f\n",OUT_DICE(pMobIndex->hitnodice));
		fprintf( fpout, "@mobsizeofhitdice %f\n",OUT_DICE(pMobIndex->hitsizedice));
		fprintf( fpout, "@mobplustohitdice %f\n",OUT_DICE(pMobIndex->hitplus));
		fprintf( fpout, "@mobnumberofdamagedice %f\n",OUT_DICE(pMobIndex->damnodice));
		fprintf( fpout, "@mobsizeofdamagedice %f\n",OUT_DICE(pMobIndex->damsizedice));
		fprintf( fpout, "@mobplustodamagedice %f\n",OUT_DICE(pMobIndex->damplus));
		fprintf( fpout, "@mobgold %d\n",pMobIndex->gold);
		fprintf( fpout, "@mobexperience %d\n",pMobIndex->exp);
		fprintf( fpout, "@mobposition %s\n",ax_position[pMobIndex->position]);
		fprintf( fpout, "@mobdefaultposition %s\n",ax_position[pMobIndex->defposition]);
		fprintf( fpout, "@mobsex %s\n",ax_sex[pMobIndex->sex]);
		fprintf( fpout, "@mobstrength %f\n",(float)pMobIndex->perm_str/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobintelligence %f\n",(float) pMobIndex->perm_int/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobwisdom %f\n",(float) pMobIndex->perm_wis/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobdexterity %f\n",(float) pMobIndex->perm_dex/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobconstitution %f\n",(float) pMobIndex->perm_con/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobcharisma %f\n",(float) pMobIndex->perm_cha/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobluck %f\n",(float) pMobIndex->perm_lck/MAX_PRIME_ATTRIB); 
		fprintf( fpout, "@mobsavevspoisondeath %d\n",
					pMobIndex->saving_poison_death);
		fprintf( fpout, "@mobsavevswand %d\n",
					pMobIndex->saving_wand);
		fprintf( fpout, "@mobsavevsparapetri %d\n",
					pMobIndex->saving_para_petri);
		fprintf( fpout, "@mobsavevsbreath %d\n",
					pMobIndex->saving_breath);
		fprintf( fpout, "@mobsavevsspellstaff %d\n",
					pMobIndex->saving_spell_staff);
#ifdef DRAGONBANE
		fprintf( fpout, "@mobsavevspsionic %d\n",
					pMobIndex->saving_psionics);
#endif
		fprintf( fpout, "@mobrace %s\n", npc_race[pMobIndex->race]);
		fprintf( fpout, "@mobclass %s\n", npc_class[pMobIndex->class]);
		if(pMobIndex->height)
			fprintf( fpout, "@mobheight %d\n", pMobIndex->height);
		if(pMobIndex->weight)
			fprintf( fpout, "@mobweight %d\n", pMobIndex->weight);
                fprintf( fpout, "@mobspeaks");
	    	for ( i = 0; lang_array[i] != LANG_UNKNOWN; i++ )
		{
	      	   	if (IS_SET(pMobIndex->speaks, lang_array[i]) )
	      		{
				fprintf(fpout, " %s", lang_names[i]);
			}
	      	}
		fprintf( fpout,"\n");
                fprintf( fpout, "@mobspeaking");
	    	for ( i = 0; lang_array[i] != LANG_UNKNOWN; i++ )
		{
	      	   	if (IS_SET(pMobIndex->speaking, lang_array[i]) )
	      		{
				fprintf(fpout, " %s", lang_names[i]);
			}
	      	}
		fprintf( fpout,"\n");
		fprintf( fpout, "@mobnumberofattacks %d\n", pMobIndex->numattacks);
#ifdef DRAGONBANE
                if(pMobIndex->hoard)
			fprintf( fpout, "@mobhoardroom %d\n", OUT_VNUM(pMobIndex->hoard));
		if(IS_SET(pMobIndex->act,ACT_STAYTERRAIN))
		{
			fprintf( fpout, "@mobstayterraintype %s\n", 
				ax_sector_names[pMobIndex->terrain]);
		}
#endif
		fprintf( fpout, "@mobhitroll %d\n", pMobIndex->hitroll);
		fprintf( fpout, "@mobdamageroll %d\n", pMobIndex->damroll);
		fprintf( fpout, "@mobbodyparts %s\n", flag_string(pMobIndex->xflags,part_flags) );
		fprintf( fpout, "@mobisresistant %s\n", flag_string(pMobIndex->resistant,ris_flags) );
		fprintf( fpout, "@mobisimmune %s\n", flag_string(pMobIndex->immune,ris_flags) );
		fprintf( fpout, "@mobissusceptible %s\n", flag_string(pMobIndex->susceptible,ris_flags) );
#ifdef SMAUG102
		fprintf( fpout, "@mobattacks %s\n", flag_string(pMobIndex->attacks,attack_flags) );
#endif
#ifdef SMAUG14
		fprintf( fpout, "@mobattacks %s\n", ext_flag_string(&pMobIndex->attacks,attack_flags) );
#endif
#ifdef DRAGONBANE
		fprintf( fpout, "@mobattacks %s\n", flag_string(pMobIndex->attack2,attack2_flags) );
#endif
#ifdef SMAUG102
		fprintf( fpout, "@mobdefenses %s\n", flag_string(pMobIndex->defenses,defense_flags) );
#endif
#ifdef SMAUG14
		fprintf( fpout, "@mobdefenses %s\n", ext_flag_string(&pMobIndex->defenses,defense_flags) );
#endif
		if(pMobIndex->spec_fun)
			fprintf( fpout, "@mobspecial %s\n",lookup_spec(pMobIndex->spec_fun));
		if ( pMobIndex->mudprogs )
		{
			fprintf( fpout, "@mobprograms\n");
	  		for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ),
					mprog->arglist, 
					strip_cr(make_out_prog(mprog->comlist)) );
	  		fprintf( fpout, "|\n" );	  
		}
    	}
	
    	/* save objects */
    	fprintf( fpout, "#OBJECTS\n" );
	newrange=tarea->hi_o_vnum + 1;
	currext=0;
	for (vnum=extobj[currext];currext < top_extobj; vnum=extobj[currext])
// 	for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
    	{
		currext++;
		if ( (pObjIndex = get_obj_index( vnum )) == NULL )
	  		continue;
#ifdef DRAGONBANE
        	if (pObjIndex->item_type == ITEM_DELETED)
	  		continue;
#endif
		vnum=getobjext(tarea,vnum);
		if (!pObjIndex->item_type) /* no support for boards/mail at this time */
			continue;
		fprintf( fpout, "!objvnum %d\n",OUT_VNUM(vnum));
		for ( obj = first_object; obj; obj = obj->next )
		{
			if(obj->pIndexData->vnum == pObjIndex->vnum)
			{
				pObjIndex->level=obj->level;
				break;
			}
		}

		fprintf( fpout, "@objlevel %f\n",OUT_LEVEL(pObjIndex->level));
		fprintf( fpout, "@objkeywordname %s\n",	pObjIndex->name);
		fprintf( fpout, "@objinventoryname %s\n",pObjIndex->short_descr);
		fprintf( fpout, "@objinroomname %s\n",pObjIndex->description);
		fprintf( fpout, "@objactiondescription %s\n",pObjIndex->action_desc);
		fprintf( fpout, "@objitemtype %s\n",o_types[pObjIndex->item_type]);
#ifdef SMAUG102
		fprintf( fpout, "@objflags %s\n", flag_string(pObjIndex->extra_flags,o_flags) );
#endif
#ifdef SMAUG14
		fprintf( fpout, "@objflags %s\n", ext_flag_string(&pObjIndex->extra_flags,o_flags) );
#endif
		fprintf( fpout, "@objflags %s\n", flag_string(pObjIndex->magic_flags,mag_flags) );
		fprintf( fpout, "@objwearflags %s\n", flag_string(pObjIndex->wear_flags,w_flags) );
		fprintf( fpout, "@objwearlayers %s\n",flag_string(pObjIndex->layers,ax_layers));


                val0=pObjIndex->value[0];
                val1=pObjIndex->value[1];
                val2=pObjIndex->value[2];
                val3=pObjIndex->value[3];
                val4=pObjIndex->value[4];
                val5=pObjIndex->value[5];

		switch ( pObjIndex->item_type )
		{
			case ITEM_ARMOR:
				fprintf( fpout, "@objcurrac %d\n",val0);
				fprintf( fpout, "@objorgac %d\n",val1);
				break;
			case ITEM_CONTAINER:
				fprintf( fpout, "@objcapacity %d\n",val0);
				if(val1 > 0)
				fprintf( fpout, "@objcontainerflags %s\n",flag_string(val1,axcontainer_flags));
                                if(val2 > 0)
				  fprintf( fpout, "@objkeyvnum %d\n",OUT_VNUM(val2));
				else
				  fprintf( fpout, "@objkeyvnum none\n");
                           
				fprintf( fpout, "@objcontainercondtn %d\n",val3);
				break;
			case ITEM_DRINK_CON:
			case ITEM_FOUNTAIN:
				fprintf( fpout, "@objdrinkcapacity %d\n",val0);
				fprintf( fpout, "@objdrinkquantity %d\n",val1);
				fprintf( fpout, "@objliquidtype '%s'\n",liq_table[val2].liq_name);
				fprintf( fpout, "@objliquidpoisonflag %d\n",val3);
			case ITEM_BLOOD:
				fprintf( fpout, "@objdrinkcapacity %d\n",val0);
				fprintf( fpout, "@objdrinkquantity %d\n",val1);
				fprintf( fpout, "@objliquidpoisonflag %d\n",val3);
				break;
			case ITEM_FOOD:
				fprintf( fpout, "@objfoodvalue %d\n",val0);
				fprintf( fpout, "@objfoodcondition %d\n",val1);
				fprintf( fpout, "@objfoodpoisonflag %d\n",val3);
				break;
			case ITEM_HERB:
				fprintf( fpout, "@objherbdoses %d\n",val1);
				fprintf( fpout, "@objherbname %s\n",herb_table[val2]->name);
				break;
			case ITEM_KEY:
				fprintf( fpout, "@objkeylockvnum %d\n",OUT_VNUM(val0));
				break;
			case ITEM_LEVER:
			case ITEM_SWITCH:
			case ITEM_BUTTON:
				fprintf( fpout, "@objtriggerflags %s\n", flag_string(val0,trig_flags) );
				if ( IS_SET( val0, TRIG_TELEPORT )
				||   IS_SET( val0, TRIG_TELEPORTALL )
				||   IS_SET( val0, TRIG_TELEPORTPLUS ) )
				{
					fprintf( fpout, "@objtrigtoroom %d\n", OUT_VNUM(val1));
					break;
				}
				if ( IS_SET( val0, TRIG_RAND4 )
				    ||	 IS_SET( val0, TRIG_RAND6 ) )
				{
					fprintf( fpout, "@objrandomizeroom %d\n", OUT_VNUM(val1));
					break;
				}
				if ( IS_SET( val0, TRIG_DOOR ) )
				{
					fprintf( fpout, "@objtrigfromroom %d\n", OUT_VNUM(val1));
					fprintf( fpout, "@objtrigtoroom %d\n", OUT_VNUM(val2));
				}

				break;
			case ITEM_LIGHT:
				fprintf( fpout, "@objlighthrsleft %d\n",val2);
				break;
			case ITEM_MONEY:
				fprintf( fpout, "@objnumbercoins %d\n",val1);
				fprintf( fpout, "@objcointype %d\n",val2);
				break;
			case ITEM_PILL:
			case ITEM_POTION:
			case ITEM_SCROLL:
				fprintf( fpout, "@objspelllevel %f\n",OUT_LEVEL(pObjIndex->value[0]));
	    			if ( IS_VALID_SN(val1) ) 
					fprintf( fpout, "@objspell1 '%s'\n",skill_table[val1]->name);
	    			if ( IS_VALID_SN(val2) ) 
					fprintf( fpout, "@objspell2 '%s'\n",skill_table[val2]->name);
	    			if ( IS_VALID_SN(val3) ) 
					fprintf( fpout, "@objspell3 '%s'\n",skill_table[val3]->name);
	    			break;
			case ITEM_STAFF:
			case ITEM_WAND:
				fprintf( fpout, "@objspelllevel %f\n",OUT_LEVEL(pObjIndex->value[0]));
				fprintf( fpout, "@objmaxcharge %d\n",val1);
				fprintf( fpout, "@objnumcharge %d\n",val2);
	    			if ( IS_VALID_SN(val3) ) 
					fprintf( fpout, "@objspell1 '%s'\n",skill_table[val3]->name);
	    			break;
			case ITEM_SALVE:
				fprintf( fpout, "@objspelllevel %f\n",OUT_LEVEL(pObjIndex->value[0]));
				fprintf( fpout, "@objmaxcharge %d\n",val1);
				fprintf( fpout, "@objnumcharge %d\n",val2);
				fprintf( fpout, "@objchargedelay %d\n",val3);
	    			if ( IS_VALID_SN(val4) ) 
					fprintf( fpout, "@objspell1 '%s'\n",skill_table[val4]->name);
	    			if ( IS_VALID_SN(val5) ) 
					fprintf( fpout, "@objspell2 '%s'\n",skill_table[val5]->name);
	    			break;
			case ITEM_TRAP:
				fprintf( fpout, "@objtrapcharges %d\n",val0);
				fprintf( fpout, "@objtraptype %s\n",ax_traps[val1]);
				fprintf( fpout, "@objtraplevel %f\n",OUT_LEVEL(val2));
				fprintf( fpout, "@objtrapflag %s\n",flag_string(val3,trap_flags));
				break;
			case ITEM_TREASURE:
				fprintf( fpout, "@objtreasuretype %d\n",val0);
				fprintf( fpout, "@objtreasurecond %d\n",val1);
				break;
			case ITEM_WEAPON:
				fprintf( fpout, "@objweaponcond %d\n",val0);
				fprintf( fpout, "@objweaponnumdice %f\n",OUT_DICE(val1));
				fprintf( fpout, "@objweaponsizedice %f\n",OUT_DICE(val2));
				fprintf( fpout, "@objweapontype %s\n",attack_table[val3]);
				break;
			case ITEM_PEN:
				fprintf( fpout, "@objpenusesleft %d\n",val0);
				break;
			case ITEM_FURNITURE:
			case ITEM_TRASH:
			case ITEM_PAPER:
			case ITEM_BOAT:
			case ITEM_FIRE:
			case ITEM_WORN:
			case ITEM_BOOK:
				break;
			default:
                                display(o_types[pObjIndex->item_type]);
				display("-Unknown itemtype for values\n\r");
				fprintf( fpout, "@objvalue0 %d\n",pObjIndex->value[0]);
				fprintf( fpout, "@objvalue1 %d\n",pObjIndex->value[1]);
				fprintf( fpout, "@objvalue2 %d\n",pObjIndex->value[2]);
				fprintf( fpout, "@objvalue3 %d\n",pObjIndex->value[3]);
				fprintf( fpout, "@objvalue4 %d\n",pObjIndex->value[4]);
				fprintf( fpout, "@objvalue5 %d\n",pObjIndex->value[5]);
				break;

		}
		fprintf( fpout, "@objweight %d\n",pObjIndex->weight);
		fprintf( fpout, "@objcost %d\n",pObjIndex->cost);
		fprintf( fpout, "@objrent %d\n",pObjIndex->rent);

		for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
		{
	   		fprintf( fpout, "@objextradesc %s\n",
				ed->keyword);
	   		fprintf( fpout, "%s\n",
				strip_cr( ed->description ));
			fprintf( fpout, "~\n");
		}

		for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
		{
	  		if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	  		{
				switch(paf->location)
				{
		    			default:
		        			fprintf( fpout, "@objaffects %s %d \n",
	    	        			a_types[paf->location],
		        			paf->modifier);
						break;
						break;
		    			case APPLY_RESISTANT:
		    			case APPLY_SUSCEPTIBLE:
		    			case APPLY_IMMUNE:
						fprintf( fpout, "@objaffectris %s %s\n",
	    					a_types[paf->location],
						flag_string(paf->modifier, ris_flags) );
						break;
                    			case APPLY_AFFECT:
#ifdef SMAUG102
						fprintf( fpout, "@objapplyaffect %s %s\n",
	    					a_types[paf->location],
						affect_bit_name(paf->modifier) );
#endif
#ifdef SMAUG14
						fprintf( fpout, "@objapplyaffect %s",
	    					a_types[paf->location]);
						for ( i = 0; i < 32 ; i++ )
						if ( IS_SET( paf->modifier, 1 << i ) )
						{
						  fprintf(fpout, " %s",a_flags[i]);
						}
						fprintf(fpout, "\n" );
#endif
						break;
		    			case APPLY_WEAPONSPELL:
		    			case APPLY_WEARSPELL:
		    			case APPLY_REMOVESPELL:
		        			fprintf( fpout, "@objcastspell %s '%s'\n",
	    	        			a_types[paf->location],
						IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name :
						"'unknown'");
						if(!IS_VALID_SN(paf->modifier))
							pager_printf(ch,"      &R unknown sn %d&w\n\r",
								paf->modifier);
						break;
                		}
			}
		}
		if ( pObjIndex->mudprogs )
		{
			fprintf( fpout, "@objprograms\n");
	  		for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ),
					mprog->arglist,
					strip_cr(make_out_prog(mprog->comlist)) );
	  		fprintf( fpout, "|\n" );	  
		}
    	}

    /* save rooms   */
    	fprintf( fpout, "#ROOMS\n" );
        ax_exit(fpout,tarea,tarea->low_r_vnum,tarea->hi_r_vnum);
        ax_entrance(fpout,tarea,tarea->low_r_vnum,tarea->hi_r_vnum);
    	for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
    	{
		if ( (room = get_room_index( vnum )) == NULL )
	  	continue;
#ifdef DRAGONBANE
        	if (room->sector_type == SECT_DELETED)
        	{
			continue;
		}
#endif
		fprintf( fpout, "!roomvnum %d\n",OUT_VNUM(vnum));
		fprintf( fpout, "@roomname %s\n",room->name);
		fprintf( fpout, "@roomdescription\n%s\n",strip_cr(room->description)	);
        	fprintf( fpout,"~\n");
		fprintf( fpout, "@roomflags %s\n",flag_string(room->room_flags,r_flags));
#ifdef DRAGONBANE
		fprintf( fpout, "@roomflags %s\n",flag_string(room->room_flag2,r_flag2));
#endif
		fprintf( fpout, "@roomsectortype %s\n",ax_sector_names[room->sector_type]);
        	fprintf( fpout, "@roomteleportdelay %d\n",room->tele_delay);
                if(room->tele_vnum)
        	  fprintf( fpout, "@roomteleportvnum %d\n",OUT_VNUM(room->tele_vnum));
        	fprintf( fpout, "@roomtunnelcapacity %d\n",room->tunnel);
		for ( xit = room->first_exit; xit; xit = xit->next )
		{
	   		if ( IS_SET(xit->exit_info, EX_PORTAL) ) /* don't fold portals */
				continue;
	   		fprintf( fpout, "^roomexit %s\n",dir_name[xit->vdir] );
	   		fprintf( fpout, "@roomexitdesc \n%s\n",strip_cr( xit->description ) );
        		fprintf( fpout,"~\n");
	   		fprintf( fpout, "@roomexitkeyword %s\n",strip_cr( xit->keyword ) );
	   		fprintf( fpout, "@roomexitexitinfo %s\n",flag_string(xit->exit_info & ~EX_BASHED,ex_flags));
                        if(xit->key > 0)
			  fprintf( fpout, "@roomexitkey %d\n",OUT_VNUM(xit->key));
                        else
			  fprintf( fpout, "@roomexitkey none\n");
			fprintf( fpout, "@roomexittovnum %d\n",OUT_VNUM(xit->vnum));
			fprintf( fpout, "@roomexitdistance %d\n",xit->distance);
/*
			if(xit->vnum < tarea->low_r_vnum || xit->vnum > tarea->hi_r_vnum)
				fprintf(fpout, "@roomislinktoexternalarea\n");
*/
		}	
		for ( ed = room->first_extradesc; ed; ed = ed->next )
		{
	   		fprintf( fpout, "@roomextradesckeywords %s\n", ed->keyword);
	   		fprintf( fpout, "%s\n",strip_cr( ed->description ));
	   		fprintf( fpout, "~\n");
		}
#ifdef SAVEMAPS
		if ( room->map )   /* maps */
		{
#ifdef OLDMAPS
	   	fprintf( fpout, "M\n" );
	   	fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#endif
	   	fprintf( fpout, "M %d %d %d %c\n",	room->map->vnum
					      , room->map->x
					      , room->map->y
					      , room->map->entry );
		}
#endif
		if ( room->mudprogs )
		{
			fprintf( fpout, "@roomprograms\n");
	  		for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ),
					mprog->arglist, 
					strip_cr(make_out_prog(mprog->comlist)) );
	  		fprintf( fpout, "|\n" );	  
		}
		fprintf( fpout, "@roomexitwordcheck\n");
	}

    /* save resets   */
    	fprintf( fpout, "#RESETS\n" );
    	for ( treset = tarea->first_reset; treset; treset = treset->next )
    	{
		switch( treset->command ) /* extra arg1 arg2 arg3 */
		{
	  		default:  case '*': break;
	  		case 'm': case 'M':
                        	fprintf(fpout, "!mobresetvnum %d\n",OUT_VNUM(getmobext(tarea,treset->arg1)));
                        	fprintf(fpout, "@mobresetinroom %d\n",OUT_VNUM(treset->arg3));
                        	fprintf(fpout, "@mobresetlimit %d\n",treset->arg2);
				break;
	  		case 'o': case 'O':
                        	fprintf(fpout, "!objresetvnum %d\n",OUT_VNUM(getobjext(tarea,treset->arg1)));
				lastobj=getobjext(tarea,treset->arg1);
                        	fprintf(fpout, "@objresetinroom %d\n",OUT_VNUM(treset->arg3));
                        	fprintf(fpout, "@objresetlimit %d\n",treset->arg2);
				break;
	  		case 'p': case 'P':
                        	fprintf(fpout, "!putobjvnum %d\n",OUT_VNUM(getobjext(tarea,treset->arg1)));
				if(treset->arg3)
                        		fprintf(fpout, "@putobjinobjvnum %d\n",OUT_VNUM(getobjext(tarea,treset->arg3)));
				else
                        		fprintf(fpout, "@putobjunderobjvnum %d\n",OUT_VNUM(lastobj));
                        	fprintf(fpout, "@putobjresetlimit %d\n",treset->arg2);

				break;
	  		case 'e': case 'E':
                        	fprintf( fpout, "!equipobjvnum %d\n",OUT_VNUM(getobjext(tarea,treset->arg1)));
                        	fprintf( fpout,"@equipwearlocation %s\n",wear_locs[treset->arg3]);
				break;
	  		case 'd': case 'D':
                        	fprintf( fpout, "!doorinroomvnum %d\n",OUT_VNUM(treset->arg1));
                        	fprintf( fpout, "@doordirection %s\n",dir_name[treset->arg2]);
				fprintf( fpout, "@doorstate %s\n",flag_string(treset->arg3,ex_flags));
				break;
	  		case 't': case 'T':
				fprintf( fpout, "!trapflag %s\n",flag_string(treset->extra,trap_flags));
				fprintf( fpout, "@traptype %s\n",ax_traps[treset->arg1]);
				fprintf( fpout, "@trapcharges %d\n",treset->arg2);
				fprintf( fpout, "@trapvnum %d\n",OUT_VNUM(treset->arg3));
				break;
	  		case 'g': case 'G':
				fprintf(fpout ,"!giveobj %d\n",OUT_VNUM(getobjext(tarea,treset->arg1)));
                        	fprintf(fpout, "@giveobjresetlimit %d\n",treset->arg2);
				break;
	  		case 'r': case 'R':
				fprintf(fpout, "!randomexitsforroom %d\n",OUT_VNUM(treset->arg1));
				fprintf(fpout, "@randomexitlastdoor %d\n",treset->arg2);

				break;
		}

	}
    	/* save shops */
    	fprintf( fpout, "#SHOPS\n" );
    	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    	{
		int keeper;

		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
		  	continue;
		if ( (pShop = pMobIndex->pShop) == NULL )
	  		continue;
		keeper=getmobext(tarea,pShop->keeper);
		fprintf( fpout, "!shopkeepervnum %d\n",OUT_VNUM(keeper));
		fprintf( fpout, "@shoptradewith all\n");
		if(pShop->buy_type[0])
			fprintf( fpout, "@shopbuys %s\n",o_types[pShop->buy_type[0]]);
		if(pShop->buy_type[1])
			fprintf( fpout, "@shopbuys %s\n",o_types[pShop->buy_type[1]]);
		if(pShop->buy_type[2])
			fprintf( fpout, "@shopbuys %s\n",o_types[pShop->buy_type[2]]);
		if(pShop->buy_type[3])
			fprintf( fpout, "@shopbuys %s\n",o_types[pShop->buy_type[3]]);
		if(pShop->buy_type[4])
			fprintf( fpout, "@shopbuys %s\n",o_types[pShop->buy_type[4]]);
		
		fprintf( fpout, "@shoptype %s\n",ax_shoptypes[0]);
		fprintf( fpout, "@shopprofitbuy %d\n",pShop->profit_buy);
		fprintf( fpout, "@shopprofitsell %d\n",pShop->profit_sell);
		fprintf( fpout, "@shopopenhour %d\n",pShop->open_hour);
		fprintf( fpout, "@shopclosehour %d\n",pShop->close_hour);
    	}

    /* save repair shops */
    	fprintf( fpout, "#REPAIRS\n" );
    	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    	{
		int keeper;

		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  		continue;
		if ( (pRepair = pMobIndex->rShop) == NULL )
	  		continue;
		keeper=getmobext(tarea,pRepair->keeper);
		fprintf( fpout, "!shoprepairvnum %d\n",OUT_VNUM(keeper));
		if(pRepair->fix_type[0])
			fprintf( fpout, "@shoprepairs %s\n",o_types[pRepair->fix_type[0]]);
		if(pRepair->fix_type[1])
			fprintf( fpout, "@shoprepairs %s\n",o_types[pRepair->fix_type[1]]);
		if(pRepair->fix_type[2])
			fprintf( fpout, "@shoprepairs %s\n",o_types[pRepair->fix_type[2]]);
		fprintf( fpout, "@shopprofitfix %d\n",pRepair->profit_fix);
		fprintf( fpout, "@shoprepairtype %s\n",ax_shoptypes[pRepair->shop_type]);
		fprintf( fpout, "@shoprepairopenhour %d\n",pRepair->open_hour);
		fprintf( fpout, "@shoprepairclosehour %d\n",pRepair->close_hour);
    	}
#ifdef DONT_THINK_THIS_IS_NEEDED
    /* save specials */
    	fprintf( fpout, "#SPECIALS\n" );
    	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  		continue;
		if ( !pMobIndex->spec_fun )
	  		continue;
		fprintf( fpout, "!mobhasspecvnum %d\n",OUT_VNUM(vnum));
		fprintf( fpout, "@mobspecial %s\n",lookup_spec(pMobIndex->spec_fun));
    	}
#endif

    /* END */
    	fprintf( fpout, "#END\n" );
	fclose(fpout);
	display("done\n\r");
    	return;
}
