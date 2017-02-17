#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>




//#include <string.h>
#include "Tcp.hpp"
#include "main.hpp"
#include <nefit-easy.h>

using namespace std;

char     tcpclient_incoming_message[TCP_MESSBUF_SIZE];  // buffer used for incomming message
char     tcpclient_outgoing_message[TCP_MESSBUF_SIZE];  // buffer used outgoing data
char     tcpserver_incoming_message[TCP_MESSBUF_SIZE];  // buffer used for incomming message, and
char     tcpserver_outgoing_message[TCP_MESSBUF_SIZE];  // and buffer for outgoing data
extern char     client_tcpsocket;
extern TcpClient	tcpclient;
extern int          tcpserver_internal_portnr;
extern bool         tcpserver_connected; //true when tcp socket is bound to external socket
extern bool         keepThreadsRunning;
extern unsigned int nrRunningThreads;
extern sockaddr     pin;
extern int          socket_descriptor_current;
extern char         **paths;
extern xmpp_ctx_t   *ctx_t;
extern struct nefit_easy easy;
extern unsigned int nr_values_to_obtain;



void *TcpServerThread(void *threadarg) //creates a blocking tcp server
//thread lives as long as the main program does not close
{
    int				listening_socket,len_incoming,i;
    unsigned int  	addrlen;
    sockaddr_in 	sockin;
    //sockaddr 		pin;   global variable
    bool			client_did_not_close_connection,bound=false;
    threadarg=threadarg; //to appease the compiler (avoid warning)

    listening_socket=  socket(AF_INET,SOCK_STREAM,0);
    if (listening_socket <0)
        printf("Error in opening socket\n\r");

    memset(&sockin, 0, sizeof(sockin));     // complete the socket structure
    sockin.sin_family 		= AF_INET;   	//allows for other network protocols
    sockin.sin_addr.s_addr 	= INADDR_ANY;	//0

    i=0;
    do {      // bind the socket to the port number
        sockin.sin_port 	= htons(TCP_IP_PORT+i);  //Host TO Network Short :Convert port number to "network byte order" MOST SIGNIFICANT BYTE FIRST
        bound= (bind (listening_socket, (__CONST_SOCKADDR_ARG)&sockin, sizeof(sockin)) == 0);
    }	while (++i<=4 && !bound );
    tcpserver_internal_portnr=TCP_IP_PORT+i-1;
    if (!bound){
        perror("tcp server didn't bind");
        std::exit(1);
    }

    if (listen(listening_socket, 5) == -1){    // show that we are willing to listen
        perror("listen");
        std::exit(1);
    }

    while (keepThreadsRunning)
    { printf("Waiting to accept a client\n");
      addrlen = sizeof(pin);
      //	wait for a new client to welcome the client
      socket_descriptor_current= accept(listening_socket,&pin, &addrlen);    //socket_descriptor_current is the actual socket descriptor for communication with this client
      if (socket_descriptor_current == -1){
            perror("accept");
            std::exit(1);
      }
      TcpServer   tcpServer(listening_socket,socket_descriptor_current);
      int   peerport=((int)(unsigned char)pin.sa_data[0])*256+(int)(unsigned char)pin.sa_data[1];
      printf("Client accepted, from IP address and portnumber client: %i.%i.%i.%i:%i\n\r",(int)(unsigned char)pin.sa_data[2],(int)(unsigned char)pin.sa_data[3],(int)(unsigned char)pin.sa_data[4],(int)(unsigned char)pin.sa_data[5],peerport);
      client_did_not_close_connection=true;
      tcpserver_connected=true;

      struct sockaddr_in *spin = (struct sockaddr_in *) &pin;
      in_addr_t serverip=(in_addr_t)spin->sin_addr.s_addr;
      //tcpclient.SetHostSocketAddress(serverip);


      while(client_did_not_close_connection){                                 //while client did not order to close the connection
          // wait for the client to accept (next) order
          // get the message from the client
          printf("Awaiting next order from (peer)client\n");
          len_incoming=recv(socket_descriptor_current, tcpserver_incoming_message, TCP_MESSBUF_SIZE-1, 0);
          if (len_incoming == -1){
                perror("recv");
                std::exit(1);
          }
          tcpserver_incoming_message[len_incoming]=0;
          printf("\nMessage recieved:%s",tcpserver_incoming_message);
          if (tcpServer.strcmp_recieved_tcp_data("Quit")){
                client_did_not_close_connection=false;
                keepThreadsRunning=false;
          }
          else
          if (tcpServer.strcmp_recieved_tcp_data("Close"))
                client_did_not_close_connection=false;
              //client wants to close the socket: close up both sockets
          else
              // otherwise process the incoming order
              // process order  return 1 if order is valid and processed
          {   if (tcpServer.process_socket())
                  // return 1 on non valid order
                  // answering..: send message
                    printf("Order processed.\n");
              else{
                    printf("Invalid order recieved from client! Length %i.\n",len_incoming);
                    if (len_incoming== 0){
                        client_did_not_close_connection=false;
                        printf("Client disconnected?\n");
                    }
              }

            }
            //printf("1: Test before end while cient didnt close\n");

        } //end while client did notclose connection
        // close up both sockets

        printf("Connection is closed by client on: %i.%i.%i.%i:%i\n",(int)(unsigned char)pin.sa_data[2],(int)(unsigned char)pin.sa_data[3],(int)(unsigned char)pin.sa_data[4],(int)(unsigned char)pin.sa_data[5],peerport);
        strcpy(tcpserver_outgoing_message,"203 Server socket closed\n");
        if (send(socket_descriptor_current, tcpserver_outgoing_message,strlen(tcpserver_outgoing_message), 0) == -1) {
                perror("send");
                std::exit(1);}// give client a chance to properly shutdown
        tcpserver_connected=false;
        sleep(1);
        close(socket_descriptor_current);
        printf("TCP server socket closed\n");
        tcpserver_incoming_message[0]=0;
        //tcpclient.Close();

    }   //while mainprogram is not closing : awaiting next client
        //until main thread wants to close
    close(listening_socket);
    //tcpclient.Close();
    nrRunningThreads--;
    cout<<"TCP Listening server socket closed; ";
    pthread_exit(NULL);
    return NULL;
}

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
       send(ok_answer);
       easy_get(&easy,"/ecus/rrc/uiStatus");
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
{ return
   strncmp(tcpserver_incoming_message,str,strlen(str)+1)==0;
}

void TcpServer::send(string& str)
{ char buff[1024];
  str.copy(buff,str.length(),(string::size_type)0);
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




