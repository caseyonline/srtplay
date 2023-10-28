#ifndef _INCOMMINGREMOTEMEDIASTREAM_HXX_
#define _INCOMMINGREMOTEMEDIASTREAM_HXX_

#include "srt/srtcore/srt.h"
#include <string>

class IncommingRemoteMediaStream {
public:
  IncommingRemoteMediaStream(const char* port, const char* addr);
  ~IncommingRemoteMediaStream();
  int start();

private:
  const char* mPort;
  const char* mAddr;
  SRTSOCKET mServ;
  int mEpid;
};

#endif
