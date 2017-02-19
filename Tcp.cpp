#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>




//#include <string.h>
#include "Tcp.hpp"
#include "main.hpp"
#include "nefit-easy.h"

using namespace std;

char     tcpclient_incoming_message[TCP_MESSBUF_SIZE];  // buffer used for incomming message
char     tcpclient_outgoing_message[TCP_MESSBUF_SIZE];  // buffer used outgoing data
char     tcpserver_incoming_message[TCP_MESSBUF_SIZE];  // buffer used for incomming message, and
char     tcpserver_outgoing_message[TCP_MESSBUF_SIZE];  // and buffer for outgoing data
extern char     client_tcpsocket;
extern TcpClient	tcpclient;
extern int          tcpserver_internal_portnr;
extern bool         tcpserver_connected; //true when tcp socket is bound to external socket
extern sockaddr     pin;
extern int          socket_descriptor_current;
extern char         *paths[];
extern xmpp_ctx_t   *ctx_t;
extern struct nefit_easy easy;
extern unsigned int nr_values_to_obtain;





//____________________________________________________________________________________________
//____________________________________________________________________________________________
//____________________________________________________________________________________________
//           ********************* TcpClient class **************************
//           * TCP Client is used for messages to the server on the peer PC *
//           * these messages are initiated by the linux device             *
//           * (without request from the peer computer)                     *
//___________****************************************************************_________________
//____________________________________________________________________________________________
TcpClient::TcpClient()
{

}


TcpClient::TcpClient(const int hostport)
{	connected= false;
    host_portnr= hostport;
    memset(&clientsockin, 0, sizeof(clientsockin));   // complete the socket structure
    clientsockin.sin_family 		= AF_INET;   	  //allows for other network protocols
    clientsockin.sin_addr.s_addr 	= INADDR_ANY;	  //0
    clientsockin.sin_port 			= 0;
    clientsockin.sin_port 			= htonl(INADDR_ANY);
    socketaddressserver_is_set      = false;
    socketaddressserver             = -1;
}

TcpClient::~TcpClient()
{
}

void TcpClient::Open()
{	sockaddr_in     server_address;
    bool			bound=false;

    client_tcpsocket=  socket(AF_INET,SOCK_STREAM,0);
    if ( client_tcpsocket <0)
        printf("Error in opening socket\n");
    bound= (bind (client_tcpsocket, (__CONST_SOCKADDR_ARG)&clientsockin, sizeof(clientsockin)) == 0);
    if (!bound){
        perror("tcp client didn't bind");
        exit(1);
    }
    //else	cout<< "BBBs Tcp client is bound\n\r";
    if (!socketaddressserver_is_set)
        cout<< "Program error: Socketadressserver is not set\n";
    server_address.sin_family       = AF_INET;
    server_address.sin_addr.s_addr 	= socketaddressserver;
    server_address.sin_port         = htons(host_portnr);
    //printf(" connecting with server on ipaddress %s\n",socketaddressserver);

    if (connect(client_tcpsocket, (struct sockaddr*) &server_address,sizeof(server_address))< 0){
        perror("\rconnection of client to server on host computer failed");
        exit(1);
    }
    //else printf("Client is connected to server\n");
    connected= true;
}

void TcpClient::SetHostSocketAddress(in_addr_t& socket_addressserver)
{   socketaddressserver         = socket_addressserver;
    socketaddressserver_is_set  = true;
}

void TcpClient::Close()
{	close(client_tcpsocket);
    //printf("Client tcp/ip socket closed\n");
    connected=false;
}

void TcpClient::SendTCPClient(const string& str)
{	strcpy(tcpclient_outgoing_message,str.c_str());
    unsigned int    length=strlen(tcpclient_outgoing_message);
    if (tcpclient_outgoing_message[length-1]!='\n' )
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

//______________________________________________________________________________________________________//
//           ********************* TcpServer class **************************                       	//
//           *   TCP server awaits orders and requests from the peer        *                           //
//___________****************************************************************___________________________//
//______________________________________________________________________________________________________//

TcpServer::TcpServer(int socket_nr,int sock_desc_currrent)
{  listening_socket=socket_nr;
   current_socket_descriptor= sock_desc_currrent;
}

bool TcpServer::process_socket()  						//returns true on recognized order, false otherise
{  char error_answer[25] = "400 Order not recognized";
   char ok_answer   [4]	 = "OK!";
   //int				i,nr;
   // orders    "EasyInfo"  "Set Temp  ##.#"
   if (strcmp_recieved_tcp_data("EasyInfo")) {
       //send(ok_answer);
       easy_get(&easy,paths[1]);
       nr_values_to_obtain++;
       easy_get(&easy,paths[0]);
       nr_values_to_obtain++;
       easy_get(&easy,paths[2]);
       nr_values_to_obtain++;
       xmpp_run(easy.xmpp_ctx);     //waiting for answers on my calls above in the easy_get function calls
       return true;
   }

   send(error_answer);
   return false;

}

void TcpServer::SetSocketDescriptors(int socket_nr,int sock_desc_currrent)
{
  listening_socket=socket_nr;
  current_socket_descriptor= sock_desc_currrent;
}

bool TcpServer::strcmp_recieved_tcp_data(char const* str)
{   return
       strncmp(tcpserver_incoming_message,str,strlen(str)+1)==0;
}

void TcpServer::send(string& str)
{ char buff[1024];
  string::size_type len=str.length();
  str.copy(buff,len,(string::size_type)0);
  buff[len]=0;
  send(buff);
}

void TcpServer::send(char* str,int length)
{  char nrbuf[5];
  if (length ==-1)
      length= strlen(str);
  strcpy(tcpserver_outgoing_message,str);
  strncpy(nrbuf,str,3);
  nrbuf[3]=0;
  int i=atoi(nrbuf);
  if (i<100 || i>999 ||str[3]!= ' '){              //Default command number is 203 meaning: OK
      strncpy(tcpserver_outgoing_message,"203 ",4);
      strncpy(tcpserver_outgoing_message+4,str,length);
      length+=4;
  }
  else
  strcpy(tcpserver_outgoing_message,str);
  if (str[length-2]!= '\r'||str[length-1]!= '\n'){     //ends string with return & linefeed?
     tcpserver_outgoing_message[length++]='\r';
     tcpserver_outgoing_message[length++]='\n';
     tcpserver_outgoing_message[length]=0;
  }
  ::send(current_socket_descriptor,tcpserver_outgoing_message,length,0);
}

void TcpServer::send_socket_buf(int length)      //sock_busf must be prepared with strinh without \r\n
{  tcpserver_outgoing_message[length++]='\r';
   tcpserver_outgoing_message[length++]='\n';
   tcpserver_outgoing_message[length]=0;
   ::send(current_socket_descriptor,tcpserver_outgoing_message,length,0);
}

void TcpServer::send_socket_buf()      //sock_busf must be prepared with strinh without \r\n
{   send_socket_buf(strlen(tcpserver_outgoing_message));
}




