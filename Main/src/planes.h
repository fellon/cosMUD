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

/* Copy this into MUD.H once planes.c is in working order and ready to
   be linked.. -- Altrag */
   
typedef struct plane_data   PLANE_DATA;
typedef struct afswap_data  AFSWAP_DATA;
typedef struct snswap_data  SNSWAP_DATA;

struct weather_data
{
  int mmhg;
  int change;
  int sky;
  int sunlight;
  int temp;
};

struct plane_data
{
  PLANE_DATA         *next;
  PLANE_DATA         *prev;
  AFSWAP_DATA        *first_afswap;
  AFSWAP_DATA        *last_afswap;
  SNSWAP_DATA        *first_snswap;
  SNSWAP_DATA        *last_snswap;
  char               *name;
  int                 stronger;
  int                 weaker;
  int                 nullified;
  int                 reverse;
  int                 reflected;
  sh_int              month_ofs;
  sh_int              mintemp;
  sh_int              maxtemp;
  sh_int              climate;
  sh_int              gravity;
  TIME_INFO_DATA      time_info;
  WEATHER_DATA        weather_data;
};

#define CLIMATE_ARCTIC       0  /* VERY cold */
#define CLIMATE_SUBARCTIC    1  /* cold, some rainfall */
#define CLIMATE_COASTAL      2  /* cold, lots of rainfall */
#define CLIMATE_TROPICAL     4  /* warm, lots of rainfall */
#define CLIMATE_HUMID        3  /* warm, some rainfall */
#define CLIMATE_ARID         5  /* warm, dry */
#define CLIMATE_DESERT       6  /* VERY hot */

#define GRAVITY_NORMAL       0
#define GRAVITY_ZERO         1
#define GRAVITY_REVERSE      2

struct snswap_data
{
  SNSWAP_DATA *next;
  SNSWAP_DATA *prev;
  sh_int       old_sn;
  sh_int       new_sn;
  sh_int       swap_chance;
};

struct afswap_data
{
  AFSWAP_DATA *next;
  AFSWAP_DATA *prev;
  int          old_af;
  int          new_af;
  sh_int       swap_chance;
};
