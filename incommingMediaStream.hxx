#ifndef _INCOMMINGMEDIASTREAM_HXX_
#define _INCOMMINGMEDIASTREAM_HXX_

#include "srt/srtcore/srt.h"
#include <string>

class IncommingMediaStream {
public:
  IncommingMediaStream(const char* port);
  ~IncommingMediaStream();
  int start();
  int process(char *frame, int frameSize);

private:
  const char* mPort;
  SRTSOCKET mSfd;
  int mEpid;
};

#endif
