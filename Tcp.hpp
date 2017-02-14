#ifndef TCP_HPP
#define TCP_HPP

#include <string>

#define TCP_MESSBUF_SIZE 	1024	  // Size of message buffer for TCP/IP
#define TCP_IP_PORT 		0x0FA8    // TCP/IP server listening on port 0x0FA0 = 4008 (decimal)

#include <sys/socket.h>
#include <netinet/in.h>

void *TcpServerThread (void *threadarg);  //creates a blocking tcp server on port 4000
///*****************************************************************/////
////*******************         TCP-CLIENT         ******************////
////*****************************************************************////
// The tcpclient is used to send unrequested messages to the peer computer
// the ip-adress of the peer is provided by the server: the peer computer is responsible for the first contact
using namespace std;
class TcpClient
{
    public:
    TcpClient();
    TcpClient(const int hostport);
    ~TcpClient();
    void Open();
    void Close();
    void SetHostSocketAddress(in_addr_t& socketaddressserver);
    void Send(const     string& strtosend);
    void Send();

    unsigned int  host_portnr;    ///the portnumber on the host computer

private:
    void SendTCPClient(const string& str);
    bool connected,socketaddressserver_is_set;
    in_addr_t socketaddressserver;
    sockaddr_in 	clientsockin;
};

class TcpServer
{  public:
      int listening_socket;
      int current_socket_descriptor;

      TcpServer(){listening_socket=-1;current_socket_descriptor= -1;};
      TcpServer(int socket_nr,int sock_desc_currrent);
      ~TcpServer(){};

      void SetSocketDescriptors(int socket_nr,int sock_desc_currrent);
      bool process_socket();
      void send(char *str,int i=-1);
      void send(string& str);
      void send_socket_buf (int length); //sends prepared databuf /r/n added on end
      void send_socket_buf() ;
      bool strcmp_recieved_tcp_data(char const* str);
};


#endif // TCP_HPP
