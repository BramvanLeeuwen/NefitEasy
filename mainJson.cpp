#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include <QString>
#include <json-c/json.h>
#include <string>
//#include <sstream>
#include <iostream>

#include <sys/queue.h>

#include <json-c/json.h>
#include <openssl/aes.h>
#include <strophe.h>
#include <sys/socket.h>
#include <unistd.h>    //sleep function



#include "main.hpp"
#include <jsonparser.hpp>
#include <iniparser.hpp>
#include <Tcp.hpp>

#define NUM_THREADS 1

void *PrintErrorThread(void *threadarg);

const char *paths[] = {
//	"/system/location/latitude",
//	"/system/location/longitude",
    "/system/sensors/temperatures/outdoor_t1",
/*	"/dhwCircuits/dhwA/dhwCurrentSwitchpoint",
    "/dhwCircuits/dhwA/dhwNextSwitchpoint",
    "/dhwCircuits/dhwA/dhwOffDuringAbsence",
    "/dhwCircuits/dhwA/dhwOffDuringNight",
    "/dhwCircuits/dhwA/dhwOperationMode",
    "/dhwCircuits/dhwA/dhwOperationType",
    "/dhwCircuits/dhwA/dhwProgram0",
    "/dhwCircuits/dhwA/dhwProgram1",
    "/dhwCircuits/dhwA/dhwProgram2",
    "/dhwCircuits/dhwA/extraDhw/duration",
    "/dhwCircuits/dhwA/extraDhw/status",
    "/dhwCircuits/dhwA/extraDhw/supported",
    "/dhwCircuits/dhwA/hotWaterSystem",
    "/dhwCircuits/dhwA/thermaldesinfect/lastresult",
    "/dhwCircuits/dhwA/thermaldesinfect/state",
    "/dhwCircuits/dhwA/thermaldesinfect/time",
    "/dhwCircuits/dhwA/thermaldesinfect/weekday",
    "/ecus/rrc/dayassunday/day",
    "/ecus/rrc/homeentrancedetection/userprofile",
    "/ecus/rrc/installerdetails",
    "/ecus/rrc/lockuserinterface",
    "/ecus/rrc/personaldetails",
    "/ecus/rrc/pirSensitivity",
    "/ecus/rrc/pm/closingvalve/status",
    "/ecus/rrc/pm/ignition/status",
    "/ecus/rrc/pm/refillneeded/status",
    "/ecus/rrc/pm/shorttapping/status",
    "/ecus/rrc/pm/systemleaking/status",
    "/ecus/rrc/recordings/yearTotal",
    "/ecus/rrc/selflearning/learnedprogram",
    "/ecus/rrc/selflearning/nextSwitchpoint",
    "/ecus/rrc/selflearning/nextSwitchpointEndtime",
    "/ecus/rrc/temperaturestep",     */
    "/ecus/rrc/uiStatus",
/*	"/ecus/rrc/userprogram/activeprogram",
    "/ecus/rrc/userprogram/fireplacefunction",
    "/ecus/rrc/userprogram/preheating",
    "/ecus/rrc/userprogram/program0",
    "/ecus/rrc/userprogram/program1",
    "/ecus/rrc/userprogram/program2",
    "/ecus/rrc/userprogram/userswitchpointname1",
    "/ecus/rrc/userprogram/userswitchpointname2",
    "/ecus/rrc/weatherDependent/basePointSupply",
    "/ecus/rrc/weatherDependent/endPointSupply",
    "/ecus/rrc/weatherDependent/forcedSwitchedOff",
    "/ecus/rrc/weatherDependent/maxSupply",
    "/ecus/rrc/weatherDependent/minSupply",
    "/ecus/rrc/weatherDependent/nightSwitchOff",
    "/ecus/rrc/weatherDependent/roomInfluence",
    "/ecus/rrc/weatherDependent/summerSwitchOff",
    "/gateway/brandID",
    "/gateway/remote/installername",
    "/gateway/remote/servicestate",
    "/gateway/remote/sid",
    "/gateway/remote/sidexptime",
    "/gateway/remote/tempsid",
    "/gateway/remote/tempsidexptime",
    "/gateway/serialnumber",
    "/gateway/update/strategy",
    "/gateway/uuid",
    "/gateway/versionFirmware",
    "/gateway/versionHardware",
    "/heatingCircuits/hc1/actualSupplyTemperature",
    "/heatingCircuits/hc1/control",
    "/heatingCircuits/hc1/holidayMode/activated",
    "/heatingCircuits/hc1/holidayMode/end",
    "/heatingCircuits/hc1/holidayMode/start",
    "/heatingCircuits/hc1/holidayMode/status",
    "/heatingCircuits/hc1/holidayMode/temperature",
    "/heatingCircuits/hc1/operationMode",
    "/heatingCircuits/hc1/temperatureAdjustment",
    "/heatingCircuits/hc1/type",
    "/notifications",
    "/system/appliance/boilermaintenancerequest",
    "/system/appliance/causecode",
    "/system/appliance/cm/type",
    "/system/appliance/cm/version",
    "/system/appliance/displaycode",
    "/system/appliance/serialnumber",       */
    "/system/appliance/systemPressure",
/*	"/system/appliance/type",
    "/system/appliance/version",
    "/system/interfaces/ems/brandbit",   */
    NULL
};

int          client_tcpsocket,tcp_ip_port;
sockaddr	 pin;
bool		 tcpserver_connected;  //true when tcp socket is bound to external socket
 TcpServer	 *tcpserver;
bool		 keepProgramRunning;
unsigned int nrRunningThreads;
int          tcpserver_internal_portnr=-1;
int          socket_descriptor_current;
extern char  tcpserver_incoming_message[];  // defined in Tcp.cpp
extern char  tcpserver_outgoing_message[];
struct nefit_easy  easy;
unsigned int nr_values_to_obtain;
xmpp_ctx_t   *ctx;



struct thread_data{
   int   thread_id;
   char  *message;
   float *floatingpoint;
};

int main(int argc, char **argv)
{
    //struct nefit_easy   easy;
    char const          **path;
    char                *serial_number=NULL, *access_key=NULL, *password=NULL;
    //struct thread_data 	td[NUM_THREADS];
    //pthread_t           threads[NUM_THREADS];
    //int                 rc, i;

    tcpserver_connected=false;
    keepProgramRunning= true;
    //easyptr=&easy;
    EASY_UNUSED(argc);
    EASY_UNUSED(argv);
    access_key=NULL;
	/* init library */
    xmpp_initialize();  
    ReadIniFile();  //read inifile if exists
    serial_number = getenv(NEFIT_SN);
    access_key =    getenv(NEFIT_AK);
    password =      getenv(NEFIT_PW);
	if (!serial_number || !access_key || !password) {
		printf("The following environmental variabled must be set\n");
        printf("c.q. an inifile "INIFILE_NAME" must be present\n");
        printf(NEFIT_SN"=123456789\n");
        printf(NEFIT_AK"=abcdefhijklmnopq\n");
        printf(NEFIT_PW"=my_password\n");
		exit(1);
	}

    easy_connect((struct nefit_easy *)&easy, (char const*)serial_number, (char const*)access_key, (char const*)password, (netif_easy_callback *)value_obtained);
    ctx=easy.xmpp_ctx;
	path = paths;
    nr_values_to_obtain=0;
	while (*path != NULL)
    {   easy_get(&easy, *path++);
        nr_values_to_obtain++;
    }
	/* enter the event loop */

    xmpp_run(easy.xmpp_ctx);     //waiting for answers on my calls above in the easy_get function calls
  //   for (int i=0;i<15 && nr_values_to_obtain>0;i++)
  //       xmpp_run_once(easy.xmpp_ctx,500);     //waiting for answers on my calls above in the easy_get function calls




 /*   nrRunningThreads  = 0;
    for(i=0; i < NUM_THREADS; i++ ){
       //printf("main() : creating thread,%i \n\r " ,i);
        td[i].thread_id = i;
        td[i].message   =(char*) "Started up";
        nrRunningThreads++;
        switch (i){
            case 0: rc = pthread_create(&threads[i], NULL,	TcpServerThread,	(void *)&td[i]);
                            break;
            default:rc = pthread_create(&threads[i], NULL,  PrintErrorThread,	(void *)&td[i]);
                            break;
        }
        if (rc){
        printf("main() : Error:unable to create thread,%i \n\r", rc );
        exit(-1);
        }
    }*/
    //while (keepThreadsRunning){   //keep main thread running until serverthread is closed by client oder "Close"
//     for (int i=0;i<2;i++)
//            xmpp_run_once(easy.xmpp_ctx,2000);     //waiting for answers on my calls above in the easy_get function calls
    //};
    //cout << "Joining threads with main: ";
    /* release our connection and context */
    TcpServerLoop();
    /* shutdown lib */
    xmpp_shutdown();
    xmpp_conn_release(easy.xmpp_conn);
    xmpp_ctx_free(easy.xmpp_ctx);

    return 1;
}


void *PrintErrorThread(void *threadarg)
{
  struct thread_data *my_data;

  my_data = (struct thread_data *) threadarg;
  my_data=my_data;  //junk code to appease the compiler
  //sprintf(log_string, "Thread ID : %i  Message : %s", my_data->thread_id ,my_data->message);
  //log_cl(log_string);
  nrRunningThreads--;
  pthread_exit(NULL);
  return NULL;
}

void TcpServerLoop() //creates a blocking tcp server
{
    int				listening_socket,len_incoming,i;
    unsigned int  	addrlen;
    sockaddr_in 	sockin;
    //sockaddr 		pin;   global variable
    bool			client_did_not_close_connection,bound=false;

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
        exit(1);
    }

    if (listen(listening_socket, 5) == -1){    // show that we are willing to listen
        perror("listen");
        exit(1);
    }

    while (keepProgramRunning)
    { printf("Waiting to accept a client\n");
      addrlen = sizeof(pin);
      //	wait for a new client to welcome the client
      socket_descriptor_current= accept(listening_socket,&pin, &addrlen);    //socket_descriptor_current is the actual socket descriptor for communication with this client
      if (socket_descriptor_current == -1){
            perror("accept");
            exit(1);
      }
      TcpServer   tcpServer(listening_socket,socket_descriptor_current);
      tcpserver=&tcpServer;
      int   peerport=((int)(unsigned char)pin.sa_data[0])*256+(int)(unsigned char)pin.sa_data[1];
      printf("Client accepted, from IP address and portnumber client: %i.%i.%i.%i:%i\n\r",(int)(unsigned char)pin.sa_data[2],(int)(unsigned char)pin.sa_data[3],(int)(unsigned char)pin.sa_data[4],(int)(unsigned char)pin.sa_data[5],peerport);
      client_did_not_close_connection=true;
      tcpserver_connected=true;
      //struct sockaddr_in *spin = (struct sockaddr_in *) &pin;
      //in_addr_t serverip=(in_addr_t)spin->sin_addr.s_addr;
      //tcpclient.SetHostSocketAddress(serverip);


      while(client_did_not_close_connection){                                 //while client did not order to close the connection
          // wait for the client to accept (next) order
          // get the message from the client
          printf("Awaiting next order from (peer)client\n");
          len_incoming=recv(socket_descriptor_current, tcpserver_incoming_message, TCP_MESSBUF_SIZE-1, 0);
          if (len_incoming == -1){
                perror("recv");
                exit(1);
          }
          tcpserver_incoming_message[len_incoming]=0;
          printf("Message recieved:%s\n\r",tcpserver_incoming_message);
          if (tcpServer.strcmp_recieved_tcp_data("Quit")){
                client_did_not_close_connection=false;
                keepProgramRunning=false;
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
                exit(1);}// give client a chance to properly shutdown
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
    cout<<"TCP Listening server socket closed; ";
}

