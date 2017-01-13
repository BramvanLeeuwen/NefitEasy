#ifndef TCPCLIENT_HPP
#define TCPCLIENT_HPP

#include <string>

#define TCP_MESSBUF_SIZE 	1024	  // Size of message buffer for TCP/IP

///*****************************************************************/////
////*******************         TCP-CLIENT         ******************////
////*****************************************************************////
// The tcpclient is used to send unrequested messages to the peer computer
// the ip-adress of the peer is provided by the server: the peer computer is responsible for the firrst contact
using namespace std;
class TcpClient
{
    public:
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

#endif // TCPCLIENT_HPP
