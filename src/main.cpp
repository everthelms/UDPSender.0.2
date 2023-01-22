
#if defined(_MSC_VER)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#else

#endif

#include<stdio.h>

#if defined(_MSC_VER)
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else
#include <string.h>
#endif







#include <string>
#include <iostream>
#include "MessageHub.h"
#include "RCMessage.h"

#define EOF (-1)


#if defined(_MSC_VER)
#pragma comment(lib, "RCLib.lib")
#else

#endif



#if defined(_MSC_VER)
#ifdef __cplusplus    // If used by C++ code,
extern "C"
{
// we need to export the C interface
#endif

__declspec(dllexport) void __cdecl run(std::string appid, MessageHub* messageHub)
{
#else
extern "C"
{
void run(std::string appid, MessageHub* messageHub)
{
#endif

    std::string myID = messageHub->createMessageQueue();     // send my data to robclub.com
    messageHub->sendLine(appid, "confirmready?type=UDPSender.0.2&id=" + myID);   // achtung: es ist noch nicht sicher ob der wirklich registriert ist


#if defined(_MSC_VER)
    SOCKET s;
    WSADATA wsa;
#else
    int s;
#endif



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


#if defined(_MSC_VER)
            //Initialise winsock
            //printf("\nInitialising Winsock...");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                printf("Failed. Error Code : %d", WSAGetLastError());
                exit(EXIT_FAILURE);
            }
            //printf("Initialised.\n");
#else

#endif


#if defined(_MSC_VER)
            //Create a socket
            if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
            {
                printf("Could not create socket : %d", WSAGetLastError());
            }
            //printf("Socket created.\n");
#else
            if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
            {
                printf("socket creation failed");
            }
#endif

            //std::cout << "Socket:" << s << "\n";
            int targetport = stoi(porstr);

            //setup address structure
            memset((char*)&si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(targetport);


#if defined(_MSC_VER)
            si_other.sin_addr.S_un.S_addr = inet_addr(ipstr.c_str());
#else
            si_other.sin_addr.s_addr = inet_addr(ipstr.c_str());
#endif

            messageHub->sendLine(confirmto, "confirm_init?id=" + myID);

        }
        if (nextmessage.getCommand() == "data_in")
        {
            //std::cout << myID << ":" << nextmessage.getMessage() << "\n";



            std::string pointerstr = nextmessage.getParameterValue("buffer");
            std::string bufferlengthstr = nextmessage.getParameterValue("length");
            long long pointer = std::stoll(pointerstr);
            char* sendbuf = (char*)pointer;
            int sendbuflen = std::stoi(bufferlengthstr);


#if defined(_MSC_VER)
            if (sendto(s, sendbuf, sendbuflen, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
            {
                printf("sendto() failed with error code : %d", WSAGetLastError());
            }
#else
            sendto(s, sendbuf,sendbuflen,0,(struct sockaddr *)&si_other, slen);
#endif

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
#if defined(_MSC_VER)
#ifdef __cplusplus
}
#endif
#else
}
#endif
