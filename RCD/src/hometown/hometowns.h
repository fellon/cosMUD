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

/* directories */
#define HTOWN_DIR	"../hometowns/"

/* hometowns */
DECLARE_DO_FUN( do_sethtown );
DECLARE_DO_FUN( do_hometowns );
DECLARE_DO_FUN( do_mphtownset );
DECLARE_DO_FUN( do_mphtownchck );

typedef struct	hometown_data		HOMETOWN_DATA;
struct	hometown_data
{
	HOMETOWN_DATA *next;
	HOMETOWN_DATA *prev;
	char	*name;
	char	*filename;
	int	recall;
};

HOMETOWN_DATA	  *	first_hometown;
HOMETOWN_DATA	  *	last_hometown;

/* hometowns */
void do_hometownset	args( ( CHAR_DATA *ch, char *argument ));
void load_hometowns	args( ( void ) );
bool load_htownfile	args( ( char *htownfile ) );
void save_hometown_list args( ( void ) );
void fread_hometown	args( ( HOMETOWN_DATA *htown, FILE *fp ) );
void set_homeown	args( ( CHAR_DATA *ch, char *argument ) );
void free_hometown	args( ( HOMETOWN_DATA *htown ) );
void write_hometown_list args( ( DESCRIPTOR_DATA *d ));

/* lookup functions */
HOMETOWN_DATA *hometown_lookup args( ( char *name ) );

/* memory */
void free_hometown args( ( HOMETOWN_DATA *htown ) );


