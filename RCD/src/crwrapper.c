#include <string.h>
#include "mud.h"
#include "imc-mercbase.h"

/* 
These are wrapper functions for using the chat/buddylist server 
link them to your mud's interpreter.

As you can see , they all rely on rtell(), so you might want to 
check what level you let your players use rtell at.

At the bottom of this file is a function crbs_command_hook which works
like the icec_command_hook function, call the crbs hook after the icec
hook.
*/

/* comment out this define if running less than 2.00g */
#define ATLEAST200G

#ifdef ATLEAST200G
extern int silent_rtell;
#endif

void do_chelp(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs help %s",argument);
	do_rtell(ch,buf);
	return;
}
void do_ctran(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs transcript %s",argument);
	do_rtell(ch,buf);
	return;
}
void do_cbudson(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs budson");
}
void do_clock(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs lock");
}
void do_cunlock(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs unlock");
}
void do_clogon(CHAR_DATA *ch, char *argument)
{
#ifdef ATLEAST200G
	silent_rtell=1;
#endif
	do_rtell(ch,"chat@crbs login");
#ifdef ATLEAST200G
	silent_rtell=0;
#endif
	return;
}
void do_clogoff(CHAR_DATA *ch, char *argument)
{
#ifdef ATLEAST200G
	silent_rtell=1;
#endif
	do_rtell(ch,"chat@crbs logout");
#ifdef ATLEAST200G
	silent_rtell=0;
#endif
	return;
}
void do_clist(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs list");
	return;
}
void do_cgoto(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs goto %s",argument);
	do_rtell(ch,buf);
}
void do_cwho(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs who");
	return;
}
void do_cwhereami(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs whereami");
	return;
}
void do_csay(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs say %s",argument);
	do_rtell(ch,buf);
}
void do_csocial(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs socials");
	return;
}
void do_cafk(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
#ifdef ATLEAST200G
	silent_rtell=1;
#endif
	sprintf(buf,"chat@crbs afk %s",argument);
	do_rtell(ch,buf);
#ifdef ATLEAST200G
	silent_rtell=0;
#endif
	return;
}
void do_cnote(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs note %s",argument);
	do_rtell(ch,buf);
}
void do_cseebud(CHAR_DATA *ch, char *argument)
{
	do_rtell(ch,"chat@crbs seebuddies");
	return;
}
void do_caddbud(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs addbuddy %s",argument);
	do_rtell(ch,buf);
}
void do_cdropbud(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs dropbuddy %s",argument);
	do_rtell(ch,buf);
}
void do_croll(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs rolldie %s",argument);
	do_rtell(ch,buf);
}
void do_cchess(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"chat@crbs chess %s",argument);
	do_rtell(ch,buf);
}
/* 
   search your files for icec_command_hook, then add this function in 
   the same manner as icec_command_hook where it appears
*/
bool crbs_command_hook(CHAR_DATA *ch, const char *command, char *argument)
{
#ifdef ATLEAST200G
	silent_rtell=1;
#endif
	if(!strcasecmp(command,"chelp"))
	{
		do_chelp(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"ctran"))
	{
		do_ctran(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cbudson"))
	{
		do_cbudson(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"clock"))
	{
		do_clock(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cunlock"))
	{
		do_cunlock(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"clogin"))
	{
		do_clogon(ch,argument);
		return 1;
	}
	else if(!strcasecmp(command,"clogout"))
	{
		do_clogoff(ch,argument);
		return 1;
	}
	else if(!strcasecmp(command,"clogon"))
	{
		do_clogon(ch,argument);
		return 1;
	}
	else if(!strcasecmp(command,"clogoff"))
	{
		do_clogoff(ch,argument);
		return 1;
	}
	else if(!strcasecmp(command,"clist"))
	{
		do_clist(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cgoto"))
	{
		do_cgoto(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cwho"))
	{
		do_cwho(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cwhereami"))
	{
		do_cwhereami(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"csay"))
	{
		do_csay(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"csocial"))
	{
		do_csocial(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cafk"))
	{
		do_cafk(ch,argument);
		return 1;
	}
	else if(!strcasecmp(command,"cnote"))
	{
		do_cnote(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cseebud"))
	{
		do_cseebud(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"caddbud"))
	{
		do_caddbud(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cdropbud"))
	{
		do_cdropbud(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"croll"))
	{
		do_croll(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else if(!strcasecmp(command,"cchess"))
	{
		do_cchess(ch,argument);
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 1;
	}
	else
 	{
#ifdef ATLEAST200G
		silent_rtell=0;
#endif
		return 0;
	}
}
