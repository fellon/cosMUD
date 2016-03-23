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
 *	Makes a wizlist for showing on the Telnet Interface WWW Site -- KCAH
 */

void make_webwiz( )
{
  DIR *dp;
  struct dirent *dentry;
  FILE *gfp;
  char *word;
  int ilevel, iflags;
  WIZENT *wiz, *wiznext;
  char buf[MAX_STRING_LENGTH];

  first_wiz = NULL;
  last_wiz  = NULL;

  dp = opendir( GOD_DIR );

  ilevel = 0;
  dentry = readdir( dp );
  while ( dentry )
  {
      if ( dentry->d_name[0] != '.' )
      {
	sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
	gfp = fopen( buf, "r" );
	if ( gfp )
	{
	  word = feof( gfp ) ? "End" : fread_word( gfp );
	  ilevel = fread_number( gfp );
          fread_to_eol( gfp );
	  word = feof( gfp ) ? "End" : fread_word( gfp );
          if ( !str_cmp( word, "Pcflags" ) )
	    iflags = fread_number( gfp );
          else
	    iflags = 0;
	  fclose( gfp );
          if ( IS_SET( iflags, PCFLAG_RETIRED ) )
            ilevel = MAX_LEVEL - 15;
          if ( IS_SET( iflags, PCFLAG_GUEST ) )
            ilevel = MAX_LEVEL - 16;
	  add_to_wizlist( dentry->d_name, ilevel );
	}
      }
      dentry = readdir( dp );
  }
  closedir( dp );

  unlink( WEBWIZ_FILE );

  sprintf(buf, "<BR>&wof %s!", sysdata.mud_name);
  towebwiz(buf);
  buf[0] = '\0';
  ilevel = 65535;
  for ( wiz = first_wiz; wiz; wiz = wiz->next )
  {
    if ( wiz->level < ilevel )
    {
      if ( buf[0] )
      {
	towebwiz( buf );
	buf[0] = '\0';
      }
      towebwiz( "" );
      ilevel = wiz->level;
      switch(ilevel)
      {
case MAX_LEVEL -  0: towebwiz( "<P>&rOwner<BR>&w" );				break;
case MAX_LEVEL -  1: towebwiz( "<P>&RAdministration<BR>&w" );			break;
case MAX_LEVEL -  2: towebwiz( "<P>&wHead Coders / Head Builders<BR>&w" );	break;
case MAX_LEVEL -  3: towebwiz( "<P>&WCoders / Builders<BR>&w" );		break;
case MAX_LEVEL -  4: towebwiz( "<P>&bNew Coders / New Builders<BR>&w" );	break;
case MAX_LEVEL -  5: towebwiz( "<P>&BGuest / Citizen Administration<BR>&w" );	break;
case MAX_LEVEL -  6: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL -  7: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL -  8: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL -  9: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL - 10: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL - 11: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL - 12: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL - 13: towebwiz( "<P>---<BR>" );	break;
case MAX_LEVEL - 14: towebwiz( "<P>&BPeople<BR>&w" );	break;
case MAX_LEVEL - 15: towebwiz( "<P>&BRetired<BR>&w" );	break;
case MAX_LEVEL - 16: towebwiz( "<P>&BGuests<BR>&w" );	break;
default:	     towebwiz( "<P>&BServants<BR>&w" );	break;
      }
    }

    if ( strlen( buf ) + strlen( wiz->name ) > 999 )
    {
	towebwiz( buf );
	buf[0] = '\0';
    }

    strcat( buf, " " );
    strcat( buf, wiz->name );

    if ( strlen( buf ) > 999 )
    {
      towebwiz( buf );
      buf[0] = '\0';
    }

  }

  if ( buf[0] )
    towebwiz( buf );

  for ( wiz = first_wiz; wiz; wiz = wiznext )
  {
    wiznext = wiz->next;
    DISPOSE(wiz->name);
    DISPOSE(wiz);
  }
  first_wiz = NULL;
  last_wiz = NULL;
}
