/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code Copyright (c)2001 Alsherok.
 *
 * Contains codebase specific defines to make the rest of it all work - hopefully.
 * Anything your codebase needs to alter is more than likely going to be stored in here.
 * This should be the only file you need to edit to solve unforseen compiling problems
 * if I've done this properly. And remember, this is all based on what these defines mean
 * in your STOCK code. If you've made adjustments to any of it, then you'll need to adjust
 * them here too.
 */

/* Locations of the configuration files */
#define I3_CONFIG_FILE   "../i3/i3.config"
#define I3_CHANNEL_FILE  "../i3/i3.channels"
#define I3_PASSWORD_FILE "../i3/i3.password"
#define I3_UCACHE_FILE   "../i3/i3.ucache"
#define I3_MUDLIST_FILE  "../i3/i3.mudlist"
#define I3_CHANLIST_FILE "../i3/i3.chanlist"

/* Well, you never know. Someone may have decided to utterly redo this for some reason. */
#ifndef MAX_STRING_LENGTH
   #define MAX_STRING_LENGTH 4096
#endif

#ifndef MAX_INPUT_LENGTH
   #define MAX_INPUT_LENGTH 1024
#endif

#ifndef MSL 
   #define MSL MAX_STRING_LENGTH
#endif

#ifndef MIL
   #define MIL MAX_INPUT_LENGTH
#endif

#ifndef FCLOSE
   /* Macro taken from DOTD codebase. Fcloses a file, then nulls its pointer for safety. */
   #define FCLOSE(fp)  fclose(fp); fp=NULL;
#endif

#ifdef I3SMAUG
   #define I3DOFUN(x) void x( CHAR_DATA *ch, char *argument )
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOCIALTYPE
   #define I3DISPOSE DISPOSE
   #define I3STRALLOC STRALLOC
   #define I3STRFREE STRFREE
   #define I3MAXPLAYERS sysdata.maxplayers
   #define I3MAX_LEVEL MAX_LEVEL
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_RANK(ch)		((ch)->pcdata->rank)
   #define CH_SEX(ch)		((ch)->sex)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class]->who_name)
   #define CH_CLAN(ch)		((ch)->pcdata->clan)
   #define CH_CLANNAME(ch)	((ch)->pcdata->clan->name)
   #define CH_CLANLEADNAME(ch) ((ch)->pcdata->clan->leader)
   #define CH_CLANONENAME(ch)  ((ch)->pcdata->clan->number1)
   #define CH_CLANTWONAME(ch)  ((ch)->pcdata->clan->number2)
   #define CH_CLANLEADRANK(ch) ( "Leader" )
   #define CH_CLANONERANK(ch)  ( "First Officer" )
   #define CH_CLANTWORANK(ch)  ( "Second Officer" )
   #define I3INVIS(ch)		((ch)->pcdata->i3invis)
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3_listen)
   #define I3REPLY(ch)  	((ch)->pcdata->i3_replyname)
   #define CH_AFK(ch)		(xIS_SET((ch)->act, PLR_AFK))
   #define CH_PK(ch)		IS_PKILL(ch)
   #define I3ISINVIS(ch)	((IS_IMMORTAL(ch) && xIS_SET((ch)->act, PLR_WIZINVIS)) || I3INVIS(ch) == TRUE)
   #define I3NOTELL(ch) 	(xIS_SET((ch)->act, PLR_NO_TELL) || IS_SET((ch)->deaf, CHANNEL_TELLS))
#endif

#ifdef I3ROM
   #define I3DOFUN(x) void x( CHAR_DATA *ch, char *argument )
   #define first_descriptor descriptor_list
   #define first_char char_list
   #define I3DISPOSE free_string
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define LEVEL_IMPLEMENTOR SUPREME
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_SEX(ch)		((ch)->sex)
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3_listen)
   #define I3REPLY(ch)  	((ch)->pcdata->i3_replyname)
   #define CH_CLAN(ch)		((ch)->clan)
   #define CH_CLANNAME(ch)	(clan_table[(ch)->clan].who_name)
   #define I3INVIS(ch)		((ch)->pcdata->i3invis)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_AFK(ch)		(IS_SET((ch)->comm, COMM_AFK ))
   #define I3NOTELL(ch) 	(IS_SET((ch)->comm, COMM_NOTELL ))
   #define CH_PK(ch)		( 0 == 1 )
   #define I3ISINVIS(ch)  	( (IS_IMMORTAL(ch) && (ch)->invis_level > 0) || I3INVIS(ch) == TRUE )


/*
 * Memory allocation macros. Taken from Smaug code.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
      sprintf( log_buf, "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	i3bug( log_buf ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */

#define LINK(link, first, last, next, prev)                     	\
do                                                              	\
{                                                               	\
    if ( !(first) ) {                                           	\
      (first)                   = (link);                       	\
      (last)					= (link);		    	\
    } else                                                      	\
      (last)->next              = (link);                       	\
    (link)->next                = NULL;                         	\
    if (first == link)								\
      (link)->prev 				= NULL;			\
    else										\
      (link)->prev              = (last);                       	\
    (last)                      = (link);                       	\
} while(0)

#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                = (insert)->prev;               \
    if ( !(insert)->prev )                                      \
      (first)                   = (link);                       \
    else                                                        \
      (insert)->prev->next      = (link);                       \
    (insert)->prev              = (link);                       \
    (link)->next                = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)                   	\
do                                                              	\
{                                                               	\
	if ( !(link)->prev ) {                                    	\
      (first)                   = (link)->next;                 	\
	  if ((first))							 	\
	  (first)->prev				= NULL;			\
	} else {                                                    \
      (link)->prev->next        = (link)->next;                 	\
	}										\
	if ( !(link)->next ) {                                      \
      (last)                    = (link)->prev;                 	\
	  if ((last))								\
	  (last)->next				= NULL;			\
	} else {                                                    \
      (link)->next->prev        = (link)->prev;                 	\
	}										\
} while(0)
#endif

#ifdef I3MERC
   #define I3DOFUN(x) void x( CHAR_DATA *ch, char *argument )
   #define first_descriptor descriptor_list
   #define first_char char_list
   #define I3DISPOSE free_string
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define LEVEL_IMPLEMENTOR MAX_LEVEL
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_SEX(ch)		((ch)->sex)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3_listen)
   #define I3INVIS(ch)		((ch)->pcdata->i3invis)
   #define I3REPLY(ch)  	((ch)->pcdata->i3_replyname)
   #define I3ISINVIS(ch)  	((IS_IMMORTAL(ch) && IS_SET((ch)->act, PLR_WIZINVIS)) || I3INVIS(ch) == TRUE)
   #define I3NOTELL(ch) 	(IS_SET((ch)->act, PLR_NO_TELL))
   #define CH_PK(ch)		( 0 == 1 )
   #define CH_AFK(ch)		( 0 == 1 )
   #define CH_CLAN(ch)		( 0 == 1 )
   #define CH_CLANNAME(ch)	( 0 == 1 )

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
      sprintf( log_buf, "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	i3bug( log_buf ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */

#define LINK(link, first, last, next, prev)                     	\
do                                                              	\
{                                                               	\
    if ( !(first) ) {                                           	\
      (first)                   = (link);                       	\
      (last)					= (link);		    	\
    } else                                                      	\
      (last)->next              = (link);                       	\
    (link)->next                = NULL;                         	\
    if (first == link)								\
      (link)->prev 				= NULL;			\
    else										\
      (link)->prev              = (last);                       	\
    (last)                      = (link);                       	\
} while(0)

#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                = (insert)->prev;               \
    if ( !(insert)->prev )                                      \
      (first)                   = (link);                       \
    else                                                        \
      (insert)->prev->next      = (link);                       \
    (insert)->prev              = (link);                       \
    (link)->next                = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)                   	\
do                                                              	\
{                                                               	\
	if ( !(link)->prev ) {                                    	\
      (first)                   = (link)->next;                 	\
	  if ((first))							 	\
	  (first)->prev				= NULL;			\
	} else {                                                    \
      (link)->prev->next        = (link)->next;                 	\
	}										\
	if ( !(link)->next ) {                                      \
      (last)                    = (link)->prev;                 	\
	  if ((last))								\
	  (last)->next				= NULL;			\
	} else {                                                    \
      (link)->next->prev        = (link)->prev;                 	\
	}										\
} while(0)
#endif

#ifdef I3UENVY
   #define I3DOFUN(x) void x( CHAR_DATA *ch, char *argument )
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOC_INDEX_DATA
   #define first_descriptor descriptor_list
   #define first_char char_list 
   #define I3DISPOSE free_string
   #define I3STRALLOC str_dup   
   #define I3STRFREE free_string
   #define LEVEL_IMPLEMENTOR MAX_LEVEL
   #define CH_LEVEL(ch)         ((ch)->level)
   #define CH_NAME(ch)          ((ch)->name)
   #define CH_TITLE(ch)         ((ch)->pcdata->title)
   #define CH_SEX(ch)		  ((ch)->sex)
   #define CH_CLASSNAME(ch)     (class_table[(ch)->class]->name)
   #define CH_RANK(ch)		  (title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_CLAN(ch)          ((ch)->pcdata->clan)
   #define CH_CLANNAME(ch)      ((ch)->pcdata->clan->who_name)
   #define FIRST_I3IGNORE(ch)   ((ch)->pcdata->i3first_ignore) 
   #define LAST_I3IGNORE(ch)    ((ch)->pcdata->i3last_ignore)
   #define I3LISTEN(ch)         ((ch)->pcdata->i3_listen)
   #define I3INVIS(ch)          ((ch)->pcdata->i3invis)
   #define I3REPLY(ch)          ((ch)->pcdata->i3_replyname)
   #define I3ISINVIS(ch)        ((IS_IMMORTAL(ch) && IS_SET((ch)->act, PLR_WIZINVIS)) || I3INVIS(ch) == TRUE)
   #define I3NOTELL(ch)         (IS_SET((ch)->act, PLR_NO_TELL))
   #define CH_PK(ch)            (IS_SET((ch)->act, PLR_REGISTER))
   #define CH_AFK(ch)           (IS_SET((ch)->act, PLR_AFK))
   SOC_INDEX_DATA *find_social( char *command );

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
      sprintf( log_buf, "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	i3bug( log_buf ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */

#define LINK(link, first, last, next, prev)                     	\
do                                                              	\
{                                                               	\
    if ( !(first) ) {                                           	\
      (first)                   = (link);                       	\
      (last)					= (link);		    	\
    } else                                                      	\
      (last)->next              = (link);                       	\
    (link)->next                = NULL;                         	\
    if (first == link)								\
      (link)->prev 				= NULL;			\
    else										\
      (link)->prev              = (last);                       	\
    (last)                      = (link);                       	\
} while(0)

#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                = (insert)->prev;               \
    if ( !(insert)->prev )                                      \
      (first)                   = (link);                       \
    else                                                        \
      (insert)->prev->next      = (link);                       \
    (insert)->prev              = (link);                       \
    (link)->next                = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)                   	\
do                                                              	\
{                                                               	\
	if ( !(link)->prev ) {                                    	\
      (first)                   = (link)->next;                 	\
	  if ((first))							 	\
	  (first)->prev				= NULL;			\
	} else {                                                    \
      (link)->prev->next        = (link)->next;                 	\
	}										\
	if ( !(link)->next ) {                                      \
      (last)                    = (link)->prev;                 	\
	  if ((last))								\
	  (last)->next				= NULL;			\
	} else {                                                    \
      (link)->next->prev        = (link)->prev;                 	\
	}										\
} while(0)
#endif

#ifdef I3ACK
   #define I3DOFUN(x) void x( CHAR_DATA *ch, char *argument )
   #define first_descriptor first_desc
   #define I3DISPOSE free_string
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define LEVEL_IMPLEMENTOR MAX_LEVEL
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_SEX(ch)		((ch)->sex)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(class_table[(ch)->class].who_name)
   #define I3INVIS(ch)		((ch)->pcdata->i3invis)
   #define I3REPLY(ch)  	((ch)->pcdata->i3_replyname)
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3_listen)
   #define I3ISINVIS(ch)	((IS_IMMORTAL(ch) && (ch)->invis > 0) || I3INVIS(ch) == TRUE)
   #define CH_AFK(ch)		(IS_SET((ch)->pcdata->pflags, PFLAG_AFK))
   #define CH_PK(ch)		(IS_SET((ch)->pcdata->pflags, PFLAG_PKOK))
   #define CH_CLAN(ch)		((ch)->clan)
   #define CH_CLANNAME(ch)	(clan_table[(ch)->clan].clan_name)
   #define I3NOTELL(ch) 	(IS_SET((ch)->act, PLR_NO_TELL ))

/*
 * Memory allocation macros. Taken from Smaug code.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
      sprintf( log_buf, "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	i3bug( log_buf ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)
#endif

#ifdef I3CIRCLE
   typedef struct	char_data		CHAR_DATA;
   typedef struct	descriptor_data	DESCRIPTOR_DATA;
   extern		char			log_buf		[];
   #include "diskio.h" /* Ugly ugly ugly, this I know. Circle's H files are rather fragmented though */
   #define I3DOFUN ACMD
   #define I3DISPOSE DISPOSE
   #define I3STRALLOC str_dup
   #define I3STRFREE DISPOSE
   #define first_descriptor descriptor_list
   #define first_char character_list
   #define PLAYER_DIR "../lib/pfiles/" /* ASCII Pfile snippet */
   #define URANGE(a, b, c)	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
   #define CH_LEVEL(ch)		(GET_LEVEL(ch))
   #define CH_NAME(ch)		(GET_NAME(ch))
   #define CH_TITLE(ch)		(GET_TITLE(ch))
   #define CH_CLASSNAME(ch)	(CLASS_ABBR(ch))
   #define CH_RANK(ch)		(CLASS_ABBR(ch))
   #define CH_SEX(ch)		(GET_SEX(ch))
   #define IS_IMMORTAL(ch)	(GET_LEVEL(ch) >= LVL_IMMORT)
   #define LEVEL_IMPLEMENTOR 	LVL_IMPL
   #define I3INVIS(ch)		((ch)->player_specials->i3invis)
   #define FIRST_I3IGNORE(ch) ((ch)->player_specials->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->player_specials->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->player_specials->i3_listen)
   #define I3REPLY(ch)		((ch)->player_specials->i3_replyname)
   #define I3ISINVIS(ch) 	((IS_IMMORTAL(ch) && (ch)->player_specials->saved.invis_level > 0) || I3INVIS(ch) == TRUE)
   #define I3NOTELL(ch)		(PRF_FLAGGED((ch), PRF_NOTELL))
   #define CH_PK(ch)		(PLR_FLAGGED((ch), PLR_KILLER))
   #define CH_AFK(ch)		( 0 == 1 )
   #define CH_CLAN(ch)		( 0 == 1 )
   #define CH_CLANNAME(ch)	( 0 == 1 )
   void i3save_char( CHAR_DATA *ch, FBFILE *fp );
   void save_char( CHAR_DATA *ch, room_rnum load_room );

/*
 * Memory allocation macros. Taken from Smaug code.
 */

#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
      log( "SYSERR: Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */

#define LINK(link, first, last, next, prev)                     	\
do                                                              	\
{                                                               	\
    if ( !(first) ) {                                           	\
      (first)                   = (link);                       	\
      (last)					= (link);		    	\
    } else                                                      	\
      (last)->next              = (link);                       	\
    (link)->next                = NULL;                         	\
    if (first == link)								\
      (link)->prev 				= NULL;			\
    else										\
      (link)->prev              = (last);                       	\
    (last)                      = (link);                       	\
} while(0)

#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                = (insert)->prev;               \
    if ( !(insert)->prev )                                      \
      (first)                   = (link);                       \
    else                                                        \
      (insert)->prev->next      = (link);                       \
    (insert)->prev              = (link);                       \
    (link)->next                = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)                   	\
do                                                              	\
{                                                               	\
	if ( !(link)->prev ) {                                    	\
      (first)                   = (link)->next;                 	\
	  if ((first))							 	\
	  (first)->prev				= NULL;			\
	} else {                                                    \
      (link)->prev->next        = (link)->next;                 	\
	}										\
	if ( !(link)->next ) {                                      \
      (last)                    = (link)->prev;                 	\
	  if ((last))								\
	  (last)->next				= NULL;			\
	} else {                                                    \
      (link)->next->prev        = (link)->prev;                 	\
	}										\
} while(0)
#endif
