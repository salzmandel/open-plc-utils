/*====================================================================*
 *   
 *   Copyright (c) 2011 Qualcomm Atheros Inc.
 *   
 *   Permission to use, copy, modify, and/or distribute this software 
 *   for any purpose with or without fee is hereby granted, provided 
 *   that the above copyright notice and this permission notice appear 
 *   in all copies.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
 *   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL  
 *   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
 *   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 *   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 *   NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 *   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *   
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *
 *   plcget.c -
 *
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../plc/plc.h"
#include "../ether/channel.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Devices.c"
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/Request.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/GetProperty.c"
#endif

#ifndef MAKEFILE
#include "../tools/synonym.c"
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/todigit.c"
#include "../tools/binout.c"
#include "../tools/hexout.c"
#include "../tools/decout.c"
#include "../tools/chrout.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/MMECode.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif

/*====================================================================*
 *   
 *   int main (int argc, char const * argv[]);
 *   
 *   parse command line, populate plc structure and perform selected 
 *   operations; show help summary if asked; see getoptv and putoptv
 *   to understand command line parsing and help summary display; see
 *   plc.h for the definition of struct plc; 
 *
 *   the command line accepts multiple MAC addresses and the program 
 *   performs the specified operations on each address, in turn; the
 *   address order is significant but the option order is not; the
 *   default address is a local broadcast that causes all devices on
 *   the local H1 interface to respond but not those at the remote
 *   end of the powerline;
 *
 *   the default address is 00:B0:52:00:00:01; omitting the address
 *   will automatically address the local device; some options will
 *   cancel themselves if this makes no sense;
 *
 *   the default interface is eth1 because most people use eth0 as 
 *   their principle network connection; you can specify another 
 *   interface with -i or define environment string PLC to make
 *   that the default interface and save typing;
 *   
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv []) 

{
	extern struct channel channel;
	static char const * optv [] = 
	{
		"abdehi:n:qs:t:v",
		"device [device] [...]",
		"Qualcomm Atheros PLC Get Property",
		"a\tdisplay property in ASCII",
		"b\tdisplay property in binary",
		"d\tdisplay property in decimal",
		"e\tredirect stderr to stdout",
		"h\tdisplay property in hexadecimal",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"n n\tproperty identifier or version is (n) [0]",
		"q\tquiet mode",
		"s x\tsession identifier is (x) [" LITERAL (PLCSESSION) "]",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMER) "]",
		"v\tverbose mode",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct plcproperty plcproperty;
	signed c;
	if (getenv (PLCDEVICE)) 
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE));

#else

		channel.ifname = strdup (getenv (PLCDEVICE));

#endif

	}
	optind = 1;
	memset (&plcproperty, 0, sizeof (plcproperty));
	while ((c = getoptv (argc, argv, optv)) != -1) 
	{
		switch (c) 
		{
		case 'a':
			plcproperty.DATA_FORMAT = PLC_FORMAT_ASC;
			break;
		case 'b':
			plcproperty.DATA_FORMAT = PLC_FORMAT_BIN;
			break;
		case 'h':
			plcproperty.DATA_FORMAT = PLC_FORMAT_HEX;
			break;
		case 'd':
			plcproperty.DATA_FORMAT = PLC_FORMAT_DEC;
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'n':
			if (!plcproperty.PROP_NUMBER) 
			{
				plcproperty.PROP_FORMAT = 1;
				plcproperty.PROP_LENGTH = sizeof (uint32_t);
				plcproperty.PROP_NUMBER = (uint32_t)(uintspec (optarg, 0, UINT_MAX));
			}
			else 
			{
				plcproperty.PROP_VERSION = (uint32_t)(uintspec (optarg, 0, UINT_MAX));
			}
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 's':
			plc.cookie = (uint32_t)(basespec (optarg, 16, sizeof (plc.cookie)));
			break;
		case 't':
			channel.timer = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message)))) 
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc) 
	{
		GetProperty (&plc, &plcproperty);
	}
	while ((argc) && (* argv)) 
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices)))) 
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		GetProperty (&plc, &plcproperty);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

