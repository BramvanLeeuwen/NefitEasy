#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "TcpClient.hpp"
using namespace std;

char     tcpclient_incoming_message[TCP_MESSBUF_SIZE];  // buffer used for incomming message
char     tcpclient_outgoing_message[TCP_MESSBUF_SIZE];  // buffer used outgoing data
int      client_tcpsocket;

//____________________________________________________________________________________________
//____________________________________________________________________________________________
//____________________________________________________________________________________________
//           ********************* TcpClient class **************************
//           * TCP Client is used for messages to the server on the peer PC *
//           * these messages are initiated by the linux device             *
//           * (without request from the peer computer)                     *
//___________****************************************************************_________________
//____________________________________________________________________________________________
TcpClient::TcpClient(const int hostport)
{	connected= false;
    host_portnr= hostport;
    memset(&clientsockin, 0, sizeof(clientsockin));   // complete the socket structure
    clientsockin.sin_family 		= AF_INET;   	  //allows for other network protocols
    clientsockin.sin_addr.s_addr 	= INADDR_ANY;	  //0
    clientsockin.sin_port 			= 0;
    clientsockin.sin_port 			= htonl(INADDR_ANY);
    socketaddressserver_is_set=false;
    socketaddressserver=-1;
}

TcpClient::~TcpClient()
{
}

void TcpClient::Open()
{	sockaddr_in     server_address;
    bool			bound=false;

    client_tcpsocket=  socket(AF_INET,SOCK_STREAM,0);
    if ( client_tcpsocket <0)
        printf("Error in opening socket\n\r");
    bound= (bind (client_tcpsocket, (__CONST_SOCKADDR_ARG)&clientsockin, sizeof(clientsockin)) == 0);
    if (!bound){
        perror("tcp client didn't bind");
        exit(1);
    }
    //else	cout<< "BBBs Tcp client is bound\n\r";
    if (!socketaddressserver_is_set)
        cout<< "Program error: Socketadressserver is not set\n\r";
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr 	=socketaddressserver;

    server_address.sin_port = htons(host_portnr);
    //printf(" connecting with server on ipaddress %s\n\r",socketaddressserver);

    if (connect(client_tcpsocket, (struct sockaddr*) &server_address,sizeof(server_address))< 0){
        perror("\rconnection of client to server on host computer failed");
        exit(1);
    }
    //else printf("Client is connected to server\n\r");
    connected= true;
}

void TcpClient::SetHostSocketAddress(in_addr_t& socket_addressserver)
{   socketaddressserver= socket_addressserver;
    socketaddressserver_is_set=true;
}

void TcpClient::Close()
{	close(client_tcpsocket);
    //printf("Client tcp/ip socket closed\n\r");
    connected=false;
}

void TcpClient::SendTCPClient(const string& str)
{	strcpy(tcpclient_outgoing_message,str.c_str());
    unsigned int    length=strlen(tcpclient_outgoing_message);
    if (tcpclient_outgoing_message[length-2]!='\n' )
        tcpclient_outgoing_message[length++]='\n';
    tcpclient_outgoing_message[length]=0;
    if (send(client_tcpsocket,tcpclient_outgoing_message,length,0) !=(int)length){
        perror("ERROR writing to socket");
        exit(1);
    }
    tcpclient_outgoing_message[length-1]=0;
    //printf("tcpclient send message: %s length %i.\r\n",tcpclient_outgoing_message,length);
}

void TcpClient::Send(const string& stringtosend)
{	Open();
    SendTCPClient(stringtosend);
    Close();
}

