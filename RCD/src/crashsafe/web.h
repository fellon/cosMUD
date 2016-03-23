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
 *	Web.h was made because some webmasters don't know the first thing
 *	about code.  So I saw it as a chance to dip my quill into a little
 *	more coding.
 *	
 *	What this is for is use with 'show_web_file' in websvr.c, it will
 *	do about the exact same as 'show_file' but for the WWW.  What this
 *	will do is show HTML files from a directory rather from code.
 *	
 *	The file types go like this:
 *
 *	*.ti	These are files that are just one solid page with no code
 *		before or after it.  Such as an index page or error page.
 *
 *	*.tih	This will be a header (Telnet Interface Header), that will
 *	start out a HTML file with tags or what not, before the code.
 *	Placing graphics for headers is not uncommon and leave a nice touch.
 *	
 *	*.tif	This will be a footer (Telnet Interface Footer), it will clean
 *	up the mess made by the code, and finish the HTML, providing links
 *	and so on and so forth.  This is your interface, be creative :)
 *
 *	This should make those rudy poo webmasters happy, somewhat :)
 *	Just explain that they will have to account for the middle of the html
 *	to be code generated and to format accordingly before and after.
 *
 *	-- Christopher Aaron Haslage (Yakkov) 6/3/99 (No Help)
 */	

#define PUBLIC_WEB		"../web/public_html/"
#define STAFF_WEB		"../web/staff_html/"

#define PUB_INDEX		PUBLIC_WEB "index.ti"
#define PUB_ERROR		PUBLIC_WEB "error.ti"
#define PUB_ABOUT		PUBLIC_WEB "about.ti"
#define PUB_WIZLIST_H		PUBLIC_WEB "wizlist.tih"
#define PUB_WIZLIST_F		PUBLIC_WEB "wizlist.tif"
#define PUB_WHOLIST_H		PUBLIC_WEB "wholist.tih"
#define PUB_WHOLIST_F		PUBLIC_WEB "wholist.tif"
#define PUB_WWWLIST_H		PUBLIC_WEB "wwwlist.tih"
#define PUB_WWWLIST_F		PUBLIC_WEB "wwwlist.tif"
#define PUB_HELP_H		PUBLIC_WEB "help.tih"
#define PUB_HELP_F		PUBLIC_WEB "help.tif"

#define STA_ERROR		STAFF_WEB "error.ti"
