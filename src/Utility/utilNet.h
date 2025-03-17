#ifndef OpenKAI_src_Utility_utilNet_H_
#define OpenKAI_src_Utility_utilNet_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

	inline bool parseIP(const char *pIn, uint8_t* pIP)
	{
		NULL_F(pIn);
		NULL_F(pIP);

		int ip1, ip2, ip3, ip4;

		// Parse the string using sscanf
		IF_F(sscanf(pIn, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4)

		// Ensure values are within valid ranges
		IF_F(ip1 < 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 ||
			ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255)

		// Store IP address bytes
		pIP[0] = (uint8_t)ip1;
		pIP[1] = (uint8_t)ip2;
		pIP[2] = (uint8_t)ip3;
		pIP[3] = (uint8_t)ip4;

		return true;
	}

	inline bool parseIPport(const char *pIn, uint8_t* pIP, uint16_t *pPort)
	{
		NULL_F(pIn);
		NULL_F(pIP);
		NULL_F(pPort);

		int ip1, ip2, ip3, ip4, port;

		// Parse the string using sscanf
		IF_F(sscanf(pIn, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port) != 5)

		// Ensure values are within valid ranges
		IF_F(ip1 < 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 ||
			ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255 ||
			port < 0 || port > 65535)

		// Store IP address bytes
		pIP[0] = (uint8_t)ip1;
		pIP[1] = (uint8_t)ip2;
		pIP[2] = (uint8_t)ip3;
		pIP[3] = (uint8_t)ip4;

		// Store port as a 16-bit value
		*pPort = (uint16_t)port;

		return true;
	}

}
#endif
