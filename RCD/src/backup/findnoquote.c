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

#include <stdio.h>
#include <stdlib.h>

char buf[8000];
main()
{
	int dq=0;
	int sq=0;
	int lcnt=0;
	char *s;
	while (gets(buf))
	{

		s=buf;
		dq=0;	
		sq=0;
		lcnt++;
		while(*s)
		{
			switch(*s++)
			{
				case '\'':
					if(!(dq %2))
						sq++;
					break;
				case '"':
					dq++;
					break;
				default:
					break;
			}
		}
		if(sq % 2 || dq % 2)
		{
			printf("line: %d\n\t%s\n",lcnt,buf);
		}
	}
}

