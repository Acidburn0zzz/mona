//$Id: Dispatch.h 3263 2006-06-09 18:05:20Z eds1275 $
#pragma once
#include <monapi.h>
#include <sys/types.h>
#include "Nic.h"

namespace mones{

class Dispatch;
//Infomation structure represents a connection.
class ConnectionInfo{
protected:
    Dispatch* dispatcher;
public:
    dword remoteip;
    word  localport;
    word  remoteport;
    dword clientid;
    word  netdsc;
    MessageInfo msg; //too heavy? performance test is must item.
    ConnectionInfo(){msg.header=0x0;}
    virtual ~ConnectionInfo(){};
    virtual void CreateHeader(Ether* ,byte* ,word ){}
    virtual int  Strip(Ether*,byte**){return 0;}
    virtual bool IsMyPacket(Ether*){return false;};
    virtual bool WellKnownSVCreply(Ether*){return false;}
    void CreateIPHeader(Ether*,word,byte);
    word checksum(byte*,word); 
private:   

};

class ICMPCoInfo : public ConnectionInfo
{
public:
    word type;
    word seqnum;
    word idnum;
    ICMPCoInfo(Dispatch* p){dispatcher=p;}
    void CreateHeader(Ether* ,byte*,word );
    int Strip(Ether*,byte**);
    bool IsMyPacket(Ether*);
    bool WellKnownSVCreply(Ether*);
};

class UDPCoInfo : public ConnectionInfo
{ 
public:
    UDPCoInfo(Dispatch* p){dispatcher=p;}
    void CreateHeader(Ether* ,byte* ,word );  
    int Strip(Ether*, byte**);
    bool IsMyPacket(Ether*);  
    bool WellKnownSVCreply(Ether*);  
};

class TCPCoInfo : public ConnectionInfo
{
public:
    enum TCP_STAT{
        CLOSED     =0,
        LISTENING  =1,
        SYN_SENT   =2,
        SYN_RCVD   =3,
        ESTABLISHED=4,
        FIN_WAIT1  =5,
        FIN_WAIT2  =6,
        CLOSE_WAIT =7,
        LAST_ACK   =8,
        TIME_WAIT  =9
    };
    dword seqnum;
    dword acknum;
    byte  status;
    byte  flag;
    TCPCoInfo(Dispatch* p){dispatcher=p;}
    void CreateHeader(Ether*,byte* ,word);  
    int  Strip(Ether*, byte**);
    bool IsMyPacket(Ether*);
    bool WellKnownSVCreply(Ether*);
private:
    bool HandShakePASV(Ether*);
    bool HandShakeACTV(Ether*);
};

};

