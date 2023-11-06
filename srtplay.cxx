#include <stdio.h>
#include "srt/srtcore/srt.h"
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>

#include "incommingMediaStream.hxx"
#include "incommingRemoteMediaStream.hxx"
#include "outgoingRemoteMediaStream.hxx"
#include "outgoingRemoteMediaStream2.hxx"

using namespace std;

int main(int ac, char* av[]) {

  //printf("hello SRT: %s\n",av[1]);
  if (ac < 2)
    printf("srtplay [ -rec | -rmtrec | -rmtsnd | -rtmsnd2 | -broad ]\n");
  else if (strcmp(av[1],"-rec") == 0)
    {
      char frame[1500];

      IncommingMediaStream *stream = new IncommingMediaStream("9000");
      stream->start();
      while(true)
      {
        if (stream->process(frame, 1500) == 0)
          break;
        printf("X");
      }
    }
  else if (strcmp(av[1], "-broad") == 0)
  {
    char frame[1500];

    IncommingMediaStream *inStream = new IncommingMediaStream("9000");
    inStream->start();
    OutgoingRemoteMediaStream2 *outStream = new OutgoingRemoteMediaStream2("9001");
    outStream->start();

    while (true)
    {
      int ret;
      if ((ret = inStream->process(frame, 1500)) == 0)
        break;
      if (outStream->process(frame, ret) == 0)
        break;
    }
  }

  else if (strcmp(av[1],"-rmtrec") == 0)
  {
    IncommingRemoteMediaStream *stream = new IncommingRemoteMediaStream("9000","104.237.152.26");
    stream->start();
  }
  else if (strcmp(av[1],"-rmtsnd") == 0)
  {
    OutgoingRemoteMediaStream *stream = new OutgoingRemoteMediaStream("9000");
    stream->start();
  }
  else if (strcmp(av[1],"-rmtsnd2") == 0)
  {
    char frame[1500];

    OutgoingRemoteMediaStream2 *stream = new OutgoingRemoteMediaStream2("9000");
    stream->start();
    while (true)
    {
      if (stream->process(frame, 1500) == 0)
        break;
    }
  }
  else
    printf("nope\n");




}
