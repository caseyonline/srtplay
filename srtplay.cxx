#include <stdio.h>
#include "srt/srtcore/srt.h"
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>

#include "incommingMediaStream.hxx"
#include "incommingRemoteMediaStream.hxx"
#include "outgoingRemoteMediaStream.hxx"

using namespace std;

int main(int ac, char* av[]) {

  //printf("hello SRT: %s\n",av[1]);
  if (ac < 2)
    printf("srtplay [ -rec | -rmtrec | -rmtsnd ]\n");
  else if (strcmp(av[1],"-rec") == 0)
    {
      IncommingMediaStream *stream = new IncommingMediaStream("9000");
      stream->start();
    }
  else if (strcmp(av[1],"-rmtrec") == 0)
  {
    IncommingRemoteMediaStream *stream = new IncommingRemoteMediaStream("9000","172.16.12.128");
    stream->start();
  }
  else if (strcmp(av[1],"-rmtsnd") == 0)
  {
    OutgoingRemoteMediaStream *stream = new OutgoingRemoteMediaStream("9000");
    stream->start();
  }
  else
    printf("nope\n");




}
