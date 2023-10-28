/*
This Object handles the push of data to an external SRT client
the external SRT client initiates the connection
*/

#ifndef _OUTGOINGREMOTEMEDIASTREAM_HXX_
#define _OUTGOINGREMOTEMEDIASTREAM_HXX_

#include "srt/srtcore/srt.h"
#include <string>

class OutgoingRemoteMediaStream {
public:
  OutgoingRemoteMediaStream(const char* port);
  ~OutgoingRemoteMediaStream();
  int start();
  //void process(const char* frame, int len);

private:
  const char* mPort;
  const char* mAddr;
  SRTSOCKET mSfd;
  int mEpid;
};

#endif
