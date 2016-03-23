/* $Id: imc-color.c,v 1.2 2000/02/03 13:56:23 gmoody Exp $ */
/*
 * Will handle color code crap. Can be moved inside a file if needed.
 * Will keep it here for now. Note: This is not a useless file.
 * Just because it has 1 function in it, it will contain more.
 * (Loading/Saving of the color configuration) This could also
 * be placed in imc-config.c if people wanted it. Up to you all.
 * See imc-color.h for more info.
 *
 * Kratas (moon@deathmoon.com/Kratas@eternal)
 */

#if !defined(CIRCLE)
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#endif

#define IN_IMC
#include "imc-mercdefs.h"
#include "imc-mercbase.h"

DEFINE_DO_FUN(do_imccolor)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char ccode; /* Color Code Character */
    
    if( IS_NPC( ch ) )
    {
        imc_to_char( "NPC cannot use IMC.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    one_argument( argument, arg2 );
    
    if( arg[0] == '\0' )
    {
        ccode = IMC_COLORCHAR;
        imc_to_char( "Syntax:\n\r"
                     "\tIMCColor <imc_color> <your_color_code>\n\r\n\r", ch );

        /*
         * I _COULD_ color this. But, we'd be stuffing so much damn shit inside
         * here. Do it in rev2.
         */
        sprintf( buf, "Current Color Settings\n\r"
                 "\tGrey     : %c%-2s\tRed       : %c%-2s\tGreen       : %c%-2s\n\r"
                 "\tBrown    : %c%-2s\tBlue      : %c%-2s\tMagenta     : %c%-2s\n\r"
                 "\tCyan     : %c%-2s\tBlack     : %c%-2s\tYellow      : %c%-2s\n\r"
                 "\tWhite    : %c%-2s\tNormal    : %c%-2s\tDark Grey   : %c%-2s\n\r"
                 "\tBold Blue: %c%-2s\tBold Green: %c%-2s\tBold Magenta: %c%-2s\n\r"
                 "\tBold Cyan: %c%-2s\tBold Red  : %c%-2s\tBold        : %c%-2s\n\r",

                 /* Now put in all the color codes */
                 ccode, IMC_GREY, ccode, IMC_RED, ccode, IMC_GREEN,
                 ccode, IMC_BROWN, ccode, IMC_BLUE, ccode, IMC_MAGENTA,
                 ccode, IMC_CYAN, ccode, IMC_BLACK, ccode, IMC_YELLOW,
                 ccode, IMC_WHITE, ccode, IMC_NORMAL, ccode, IMC_DARK_GREY,
                 ccode, IMC_BOLD_BLUE, ccode, IMC_BOLD_GREEN, ccode, IMC_BOLD_MAGENTA,
                 ccode, IMC_BOLD_CYAN, ccode, IMC_BOLD_RED, ccode, IMC_BOLD );

        imc_to_char( buf, ch );
    }
}

