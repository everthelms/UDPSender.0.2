
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <string>
#include <iostream>
#include "MessageHub.h"
#include "RCMessage.h"

#define EOF (-1)


#pragma comment(lib, "RCLib.lib")



#ifdef __cplusplus    // If used by C++ code, 
extern "C"
{          // we need to export the C interface
#endif

	__declspec(dllexport) void __cdecl run(std::string appid, MessageHub* messageHub)
	{
		std::string myID = messageHub->createMessageQueue();     // send my data to robclub.com
		messageHub->sendLine(appid, "confirmready?type=UdpSender.0.2&id=" + myID);   // achtung: es ist noch nicht sicher ob der wirklich registriert ist

		SOCKET s;
		WSADATA wsa;
		struct sockaddr_in si_other;    // target socket
		int slen = sizeof(si_other);


		// *** Here starts the Messageloop ***  
		while (true)
		{
			std::string nextmessagestring = messageHub->receiveLine(myID, 10);   // 10 us, d.h. nur wenn was da ist
			RCMessage nextmessage = RCMessage(nextmessagestring);

			

			if (nextmessage.getCommand() == "init")
			{
				std::string ipstr = nextmessage.getParameterValue("ip");
				std::string porstr = nextmessage.getParameterValue("port");
				std::string confirmto = nextmessage.getParameterValue("confirmto");
				


				//Initialise winsock
				printf("\nInitialising Winsock...");
				if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
				{
					printf("Failed. Error Code : %d", WSAGetLastError());
					exit(EXIT_FAILURE);
				}
				printf("Initialised.\n");

				//Create a socket
				if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
				{
					printf("Could not create socket : %d", WSAGetLastError());
				}
				printf("Socket created.\n");
				//std::cout << "Socket:" << s << "\n";
				int targetport = stoi(porstr);

				//setup address structure
				memset((char*)&si_other, 0, sizeof(si_other));
				si_other.sin_family = AF_INET;
				si_other.sin_port = htons(targetport);
				si_other.sin_addr.S_un.S_addr = inet_addr(ipstr.c_str());

				messageHub->sendLine(confirmto, "confirm_init?id=" + myID);

			}
			if (nextmessage.getCommand() == "data_in")
			{
				//std::cout << myID << ":" << nextmessage.getMessage() << "\n";
				//std::cout <<"Socket:" << s << "\n";

				std::string pointerstr = nextmessage.getParameterValue("buffer");
				std::string bufferlengthstr = nextmessage.getParameterValue("length");
				long long pointer = std::stoll(pointerstr);
				char* sendbuf = (char*)pointer;
				int sendbuflen = std::stoi(bufferlengthstr);

				if (sendto(s, sendbuf, sendbuflen, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
				{
					printf("sendto() failed with error code : %d", WSAGetLastError());
				}
				free(sendbuf);
			}

			else if (nextmessage.getCommand() == "end")
			{
				messageHub->sendLine(appid, "endingmyself?from=" + myID);
				messageHub->deleteMessageQueue(myID);
				break;
			}
		}
	}

#ifdef __cplusplus
}
#endif