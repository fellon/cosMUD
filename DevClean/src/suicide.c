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

/*This code is to let a person commit suicide. Here's how to install:
 *COPY THIS WHOLE THING OVER TO THE FILE THAT IS EXPLAINED LATER
 *1: Create a file in /dist/src named suicide.c copy this and paste
 *   it there. (create the file by typing pico suicide.c)
 *   then type control x, y, and hit enter to save.
 *2: Then go into tables.c and type control w and type 'm'
 *   It'll bring you down to this line:
 *   case 'm':
 *   underneath it, add this line:
 *   if ( !str_cmp( name, "do_suicide" ))       return do_suicide;
 *   then type control w again, and type skill
 *   go down further into the file by entering control v a few times
 *   go till you see a line that looks similar to this:
 *   if ( skill =3D=3D do_save )        return "do_save";
 *   (it doesn't matter if it exactly here..just a line that looks
 *   similar to the beginning of that line, in there add:
 *   if ( skill =3D=3D do_suicide )     return "do_suicide";
 *   then type control x, (it'll ask you if you want to save, type
 *   y, then enter.
 *
 *3: Goto a file called Makefile (pico Makefile)
 *   Under the O_FILES you'll see a list of things...
 *   go to the end and enter suicide.o (make sure it's before the \
 *   and leave a space before suicide.o and \
 *   go down furthur..you'll see C_FILES do the same thing as above
 *   only now..type suicide.c
 *   exit, and save. (by typin control-x y, and enter)
 *
 *4: Go to mud.h
 *   search for act_*.c (by typing control w, then entering it)
 *   you should then see a line that looks like
 *   DECLARE_DO_FUN( do_etc ); (just an example)
 *   add this anywhere in there
 *   DECLARE_DO_FUN( do_suicide );
 *   exit, save.
 *
 *5: To add color to the two messages..
     first message shown below this (below as in this code)
     if you want to commit suicide, the syntax is: etc..
     just do color as you would in the mud with &c &W ect (help pcolors)
     the 3 messages are:
 
     If you want to commit suicide..
     Wrong password
     and $s has committed suicide
     **when puttin in color make sure it's inside the "  "
     for example "&z&Wwrong password" (I do &z&<color of choice> to =
prevent bleeding)
 
 *6: Here's how to compile the mud after
 *   Type make clean, then type make..and wait for it to finish
 *   Then reboot or copyover in the mud..                                =
      =20
 *                                      Callidyrr
 * Also, do not worry about this here, you can leave it, and it will
 * not affect the code.
 *
 * Do this after..go into player.c and right at the first page..
 * you should see the exact same thing as above..if not..tell me
 * cause ucmm is modified..could be a little difference.
 */
 
/*#include <type.h> */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
 
 
void do_suicide (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  argument =one_argument (argument, arg);
 
  if (arg[0] == '\0')
    {
      send_to_char ("&WIf you want to commit suicide, the syntax is: suicide <password>.&w\n\r",ch);
      return;
    }
 
  if (strcmp (crypt (arg, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      send_to_char ("^z&WWrong password.&w\n\r", ch);
      return;
    }
  else
    {
      act(AT_PLAIN, "$n writhes and screams as $e is extracted from the matrix.",ch, NULL, NULL, TO_ROOM);
      act(AT_PLAIN, "You are wracked with uncontrollable pain as you are extracted from the matrix.", ch, NULL, NULL, TO_CHAR);
      do_destroy (ch, ch->name);
      sprintf (buf, "%s has committed suicide.", ch->name);
      log_string(buf);
      do_info(ch, buf);
    }
return;
}
