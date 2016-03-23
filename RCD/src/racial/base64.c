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
 * Base64 Decoder, for the webserver basic authentication
 * stuff, not mine found it kicking about at 
 * http://local.15seconds.com/faq/pg00934.htm
 * All credit to John Ludeman for this code :)
 * 
 * Only change made is to cast the data access to pr2six
 * to type (int) to prevent gcc from moaning its head off 
 * while using -Wall
 */


#include <stdio.h>

/* Thanks to John Ludeman for this code...
 * Define translation matrix for Base64 decode.
 * it's fast and const should make it shared text page.
 */
const int pr2six[256] =
{
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
  52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
  10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
  28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
  64,64,64,64,64,64,64,64,64,64,64,64,64
};

/*
 * Decode a base64 encoded string
 */
void Base64Decode(
        char *bufcoded,
        unsigned char *bufplain,
        int outbufsize
)
{
    int                         nbytesdecoded;
    int                         nprbytes;
    char                        *bufin = bufcoded;
    unsigned char       *bufout = bufplain;

    /* Strip leading whitespace */
    while (*bufcoded == ' ' || *bufcoded == '\t')
        ++bufcoded;

    /*
     * Figure out how many characters are in the input buffer.
     * If this would decode into more bytes than would fit into
     * the output buffer, adjust the number of input bytes downwards.
     */
    bufin = bufcoded;
    while (pr2six[(int)*(bufin++)] <= 63)
        ;
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;
    if (nbytesdecoded > outbufsize)
                nprbytes = (outbufsize * 4) / 3;

    bufin = bufcoded;
    while (nprbytes > 0)
    {
        *(bufout++) =
            (unsigned char) (pr2six[(int)*bufin] << 2 | pr2six[(int)bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[(int)bufin[1]] << 4 | pr2six[(int)bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[(int)bufin[2]] << 6 | pr2six[(int)bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03)
    {
        if(pr2six[(int)bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }
    bufplain[nbytesdecoded] = '\0';
}
