/*
This Object handles the push of data to an external SRT client
the external SRT client initiates the connection
*/

#ifndef _OUTGOINGREMOTEMEDIASTREAM2_HXX_
#define _OUTGOINGREMOTEMEDIASTREAM2_HXX_

#include "srt/srtcore/srt.h"
#include <string>

#define MAX_CLIENTS 10

class OutgoingRemoteMediaStream2 {
public:
  OutgoingRemoteMediaStream2(const char* port);
  ~OutgoingRemoteMediaStream2();
  int start();
  int process();
  //void process(const char* frame, int len);

private:
  const char* mPort;
  const char* mAddr;
  SRTSOCKET mSfd;
  int mEpid;
  struct {
    SRTSOCKET srtSock;
    char clienthost[NI_MAXHOST];
    char clientservice[NI_MAXSERV];
  } mConnections[MAX_CLIENTS];

  void initConnections();
  void removeConnection(SRTSOCKET *s);
  int newConnection();
};

#endif
