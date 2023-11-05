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
    printf("srtplay [ -rec | -rmtrec | -rmtsnd | -rtmsnd2 ]\n");
  else if (strcmp(av[1],"-rec") == 0)
    {
      IncommingMediaStream *stream = new IncommingMediaStream("9000");
      stream->start();
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
    OutgoingRemoteMediaStream2 *stream = new OutgoingRemoteMediaStream2("9000");
    stream->start();
    while (true)
    {
      if (stream->process() == 0)
        break;
    }
  }
  else
    printf("nope\n");




}
