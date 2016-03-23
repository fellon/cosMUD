/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code Copyright (c)2001 Alsherok.
 */

#include "i3cfg.h"

/* You should not need to edit anything below this line if I've done this all correctly. */

#ifndef I3CIRCLE /* You know, this whole disorganized Circle code mess is irritating! */
DECLARE_DO_FUN( do_i3 );
DECLARE_DO_FUN( do_i3who );
DECLARE_DO_FUN( do_i3finger );
DECLARE_DO_FUN( do_i3locate );
DECLARE_DO_FUN( do_i3tell );
DECLARE_DO_FUN( do_i3reply );
DECLARE_DO_FUN( do_i3emote );
DECLARE_DO_FUN( do_i3beep );
#endif

// internal structures
typedef struct I3_channel	I3_CHANNEL;
typedef struct I3_listener	I3_LISTENER;
typedef struct I3_mud		I3_MUD;
typedef struct I3_stats		I3_STATS;
typedef struct I3_header	I3_HEADER;
typedef struct I3_ignore	I3_IGNORE;
typedef struct ucache_data	UCACHE_DATA;

extern char I3_THISMUD[256];
extern char I3_ROUTER_NAME[256];
extern char cbuf[MSL];
extern int I3_socket;

extern I3_CHANNEL *first_I3chan;
extern I3_CHANNEL *last_I3chan;
extern I3_MUD *first_mud;
extern I3_MUD *last_mud;
extern I3_MUD *this_mud;
extern UCACHE_DATA *first_ucache;
extern UCACHE_DATA *last_ucache;
extern I3_STATS I3_stats;

struct ucache_data
{
#ifdef I3ACK
   bool is_free;
#endif
   UCACHE_DATA *next;
   UCACHE_DATA *prev;
   char name[256];
   int gender;
   time_t time;
};

struct I3_ignore
{
#ifdef I3ACK
   bool is_free;
#endif
   I3_IGNORE *next;
   I3_IGNORE *prev;
   char name[256];
};

struct I3_listener 
{
#ifdef I3ACK
   bool is_free;
#endif
   I3_LISTENER *next;
   I3_LISTENER *prev;
   DESCRIPTOR_DATA *desc;
};

struct I3_header 
{
    char originator_mudname[256];
    char originator_username[256];
    char target_mudname[256];
    char target_username[256];
};

struct I3_channel 
{
#ifdef I3ACK
   bool is_free;
#endif
   I3_CHANNEL *next;
   I3_CHANNEL *prev;
   I3_LISTENER *first_listener;
   I3_LISTENER *last_listener;
   char local_name[256];
   char host_mud[256];
   char I3_name[256];
   char layout_m[MIL];
   char layout_e[MIL];
   int status;
   int local_level;
   bool connected;
};

struct I3_mud 
{
#ifdef I3ACK
   bool is_free;
#endif
   I3_MUD *next;
   I3_MUD *prev;
   int status;
   char name[256];
   char ipaddress[256];
   char mudlib[256];
   char base_mudlib[256];
   char driver[256];
   char mud_type[256];
   char open_status[256];
   char admin_email[256];
   char telnet[256];
   char web[256];
   int  player_port;
   int  imud_tcp_port;
   int  imud_udp_port;
 
   bool tell:1;
   bool beep:1;
   bool emoteto:1;
   bool who:1;
   bool finger:1;
   bool locate:1;
   bool channel:1;
   bool news:1;
   bool mail:1;
   bool file:1;
   bool auth:1;
   bool ucache:1;

   int smtp;
   int ftp;
   int nntp;
   int http;
   int pop3;
   int rcp;
   int amrcp;

   // only used for this mud
   char routerIP[256];
   char routerName[256];
   int  routerPort;
   bool autoconnect;
   int password;
   int mudlist_id;
   int chanlist_id;
   int minlevel;
};

struct I3_stats 
{
    int count_tell_commands;
    int count_tell;
    int count_beep_commands;
    int count_beep;
    int count_emoteto_commands;
    int count_emoteto;
    int count_who_commands;
    int count_who_req;
    int count_who_reply;
    int count_finger_commands;
    int count_finger_req;
    int count_finger_reply;
    int count_locate_commands;
    int count_locate_req;
    int count_locate_reply;
    int count_chanlist_reply;
    int count_channel_m_commands;
    int count_channel_m;
    int count_channel_e_commands;
    int count_channel_e;
    int count_channel_t_commands;
    int count_channel_t;
    int count_channel_add;
    int count_channel_remove;
    int count_channel_admin;
    int count_channel_filter_req;
    int count_channel_filter_reply;
    int count_channel_who_commands;
    int count_channel_who_req;
    int count_channel_who_reply;
    int count_channel_listen;
    int count_chan_user_req;
    int count_chan_user_reply;
    int count_news_read_req;
    int count_news_post_req;
    int count_news_grplist_req;
    int count_mail;
    int count_mail_ack;
    int count_file_list_req;
    int count_file_list_reply;
    int count_file_put;
    int count_file_get_req;
    int count_file_get_reply;
    int count_auth_mud_req;
    int count_auth_mud_reply;
    int count_ucache_update;
    int count_error;
    int count_startup_req_3;
    int count_startup_reply;
    int count_shutdown;
    int count_mudlist;
    int count_oob_req;
    int count_oob_begin;
    int count_oob_end;

    int count_unknown;
    int count_total;
};

void I3_main( bool forced, int mudport, bool isconnected );
bool I3_is_connected( void );
void I3_loop( void );
void I3_shutdown( int delay );
void I3_listen_channel( DESCRIPTOR_DATA *desc, char *argument, bool silent, bool addflag );
I3_LISTENER *find_I3_listener_by_descriptor( I3_CHANNEL *channel, DESCRIPTOR_DATA *desc );
I3_LISTENER *find_I3_listener_by_char( I3_CHANNEL *channel, CHAR_DATA *ch );
bool I3_command_hook( CHAR_DATA *ch, char *command, char *argument );
I3_CHANNEL *find_I3_channel_by_name( char *name );
I3_CHANNEL *find_I3_channel_by_localname( char *name );
I3_LISTENER *new_I3_listener( I3_CHANNEL *channel, DESCRIPTOR_DATA *d );
void I3_addchan( CHAR_DATA *ch, char *argument );
void I3_removechan( CHAR_DATA *ch, char *argument );
void I3_admin_channel( CHAR_DATA *ch, char *argument );
void I3_char_login( CHAR_DATA *ch );
int which_keyword( char *keyword, ... );
#ifndef I3CIRCLE
void i3save_char( CHAR_DATA *ch, FILE *fp );
#endif
bool i3load_char( CHAR_DATA *ch, FILE *fp, char *word );
void I3_close_char( DESCRIPTOR_DATA *dclose );
void free_i3chardata( CHAR_DATA *ch );
int I3_process_who_req( char *s );
void I3_get_header( char **pps, I3_HEADER *header );
void I3_write_header( char *identifier, char *originator_mudname, char *originator_username, char *target_mudname, char *target_username );
void I3_write_buffer( char *msg );
void send_to_i3( char *text );
char *I3_escape( char *ps );
bool i3ignoring( CHAR_DATA *ch, char *ignore );
int I3_strlen_color( const char *argument );
void I3_send_packet( void );
void I3_savemudlist( void );
void I3_savechanlist( void );
