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


#include "easyheader.hpp"
#include <jsonparser.hpp>
#include <iniparser.hpp>
#include <Tcp.hpp>

#define NUM_THREADS 1

void *PrintErrorThread(void *threadarg);

/* see https://github.com/robertklep/nefit-easy-core/wiki/List-of-endpoints */
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
TcpClient	 tcpclient;
bool		 keepThreadsRunning;
unsigned int nrRunningThreads;

struct thread_data{
   int   thread_id;
   char  *message;
   float *floatingpoint;
};

int main(int argc, char **argv)
{
	struct nefit_easy easy;
	char const **path;
    char *serial_number=NULL, *access_key=NULL, *password=NULL;
    struct thread_data 	td[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    int 	rc, i;
    tcpserver_connected=false;
    keepThreadsRunning= true;

    EASY_UNUSED(argc);
    EASY_UNUSED(argv);
    access_key=NULL;
	/* init library */
    xmpp_initialize();  
    ReadIniFile();  //read inifile if exists
    serial_number = getenv(NEFIT_SN);
    access_key =    getenv(NEFIT_AK);
    password =      getenv(NEFIT_PW);
//    char *string = (char*)"{\"sitename\" : \"joys of programming\",\"categories\" : [ \"c\" , [\"c++\" , \"c\" ], \"java\", \"PHP\" ],\"author-details\": { \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }}";
//    printf("JSON string: %s\n", string);
//    json_object * jobj = json_tokener_parse(string);
//    json_parse(jobj);
//
	if (!serial_number || !access_key || !password) {
		printf("The following environmental variabled must be set\n");
        printf("c.q. an inifile "INIFILE_NAME" must be present\n");
        printf(NEFIT_SN"=123456789\n");
        printf(NEFIT_AK"=abcdefhijklmnopq\n");
        printf(NEFIT_PW"=my_password\n");
		exit(1);
	}

    easy_connect((struct nefit_easy *)&easy, (char const*)serial_number, (char const*)access_key, (char const*)password, (netif_easy_callback *)value_obtained);

	path = paths;
	while (*path != NULL)
        easy_get(&easy, *path++);

	/* enter the event loop */
    for (int i=0;i<15;i++)
  //       xmpp_run(easy.xmpp_ctx);     //waiting for answers on my calls above in the easy_get function calls
           xmpp_run_once(easy.xmpp_ctx,2000);     //waiting for answers on my calls above in the easy_get function calls


	/* release our connection and context */
	xmpp_conn_release(easy.xmpp_conn);
	xmpp_ctx_free(easy.xmpp_ctx);

	/* shutdown lib */
	xmpp_shutdown();
    nrRunningThreads  = 0;
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
    }
    while (keepThreadsRunning)   //keep main thread running until serverthread is closed by client oder "Close"
    {sleep(2);};

    //sleep(50);
    cout << "Joining threads with main: ";
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
