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

//#define SMAUG102
//#define DRAGONBANE
#define SMAUG14

#define BILLIONVNUM
#ifdef DRAGONBANE
#define BILLIONVNUM
#endif
#ifdef BILLIONVNUM
#define MAX_POSSIBLE_VNUM 2000000000
#else
#define MAX_POSSIBLE_VNUM 32765
#endif


int renumber,startvnum,door;
float goldadjust;

#define display(s) send_to_char(s,ch)
#define MAX_MOB_LEVEL ((float)(LEVEL_HERO))
#define MAX_PRIME_ATTRIB 25.0
#define MAX_ALIGNMENT 1000.0
#define MAX_MOBARMORCLASS 300.0
#define MAX_WEAPONTYPE 13
#define MAX_SHOP_BUY_SLOT 5
#define MAX_SHOP_FIX_SLOT 3
/* make my life easiers */
#define IN_VNUM(v) (((v) + startvnum) + (renumber - startvnum)) 
#define OUT_VNUM(v) ((v) - startvnum) 
#define IN_LEVEL(v) ((int)( v * MAX_MOB_LEVEL + .5))
#define OUT_LEVEL(v) ((float) v / MAX_MOB_LEVEL)
#define IN_PRIME(v) ((int)( v * MAX_PRIME_ATTRIB + .5))
#define OUT_PRIME(v) ((float) v / MAX_PRIME_ATTRIB)
#define IN_DICE(v) ((int)(v * (MAX_MOB_LEVEL * 100.0) + .5))
#define OUT_DICE(v) ((float) v / (MAX_MOB_LEVEL * 100.0))
#define GOLDADJ(v) ((int)((float)(v)*goldadjust))

#ifdef DRAGONBANE
extern char * const mag_flags[];
#else
extern char * const mag_flags[7]; /*change this to be the right number */
				  /* of elements - anyone know a better
				     way without changing build.c? */
#endif
#ifdef SMAUG14
char *ax_position[]={
"dead","mortallywounded","incapacitated","stunned","sleeping",
"berserk","resting","aggressive","sitting","fighting","defensive",
"evasive","standing","mounted","shove","dragged",
NULL};
#else
char *ax_position[]={
"dead","mortallywounded","incapacitated","stunned","sleeping",
"resting","sitting","fighting","standing","mounted",
"shove","dragged",NULL};
#endif
char *ax_sex[]={"neutral","male","female",NULL};

char *ax_traps[]={"none",
            "poisongas","poisondart","poisonneedle",
            "poisondagger","poisonarrow","blindnessgas",
            "sleepinggas","flame","explosion",
            "acidspray","eletricshock","blade"
            "sexchange",NULL};

char *ax_shoptypes[]={"sell","fix","charge",NULL};

char *ax_layers[]={
"none","inbody","onskin_tatoo_scars","against_skin","clothing",
"overclothes","normal_armor","worn_on_armor","magical_effects",NULL};
char *ax_sector_names[]=
{
	"inside",
	"city",
	"field",
	"forest",
	"hills",
	"mountain",
	"water_swim",
	"water_no_swim",
	"underwater",
	"air",
	"desert",
	"somewhere",
	"oceanfloor",
	"underground",
        "fire",
        "deleted",
        "swamp",
        NULL
};

char * const axcontainer_flags[] =
{
"closeable","pickproof","closed","locked",NULL
};
