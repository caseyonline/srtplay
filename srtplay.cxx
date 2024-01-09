#include <stdio.h>
#include "srt/srtcore/srt.h"
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include <sys/socket.h>

#include "incommingMediaStream.hxx"
#include "incommingRemoteMediaStream.hxx"
#include "outgoingRemoteMediaStream.hxx"
#include "outgoingRemoteMediaStream2.hxx"
#include "socket.hxx"

void socketOut();
void socketIn();
#define SOCKET_PORT 4000

using namespace std;

int main(int ac, char* av[]) {

  //printf("hello SRT: %s\n",av[1]);
  if (ac < 2)
    printf("srtplay [ -rec | -rmtrec | -rmtsnd | -rtmsnd2 | -broad | -streamIn | -streamOut | -socketOut | -socketIn ]\n");
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
      if (ret > 0) {
        if (outStream->process(frame, ret) == 0)
          break;
        }
    }
  }
  else if (strcmp(av[1], "-streamIn") == 0)
  {
    char frame[1500];

    IncommingMediaStream *inStream = new IncommingMediaStream("9000");
    inStream->start();

    mySocket sock(SOCKET_PORT);
    sock.initSend();

    while(true)
    {
      int ret;
      if ((ret = inStream->process(frame, 1500)) == 0)
        break;
      if (ret > 0) {
        sock.send(frame, ret);
      }
    }
  }
  else if (strcmp(av[1], "-streamOut") == 0)
  {
    char frame[1500];

    OutgoingRemoteMediaStream2 *outStream = new OutgoingRemoteMediaStream2("9001");
    outStream->start();

    mySocket sock(SOCKET_PORT);

    if (sock.initRec() < 0)
      return 0;

    while(true)
    {
      int n = sock.receive(frame, 1500);
      if (n > 0)
        if (outStream->process(frame, n) == 0)
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
  else if (strcmp(av[1],"-socketOut") == 0)
  {
    socketOut();
  }
  else if (strcmp(av[1],"-socketIn") == 0)
  {
    socketIn();
  }
  else
    printf("nope\n");


}

void socketOut() {

  //int client_fd, status;
  //struct sockaddr_in serv_addr;
  char* hello = (char *)"Hello from client";
  char* hello2= (char *)"What the hell is going on";
  char* hello3= (char *)"Anyone homne";
  //char buffer[1024] = {0};

  mySocket sock(SOCKET_PORT);

  sock.initSend();

  sock.send(hello, strlen(hello));
  sock.send(hello2, strlen(hello2));
  sock.send(hello3, strlen(hello3));

}

void socketIn() {

  char buf[1024];

  mySocket sock(SOCKET_PORT);

  if (sock.initRec() < 0)
    return;

  while(true)
  {
    int n = sock.receive(buf, sizeof(buf));
    cout << n << endl;
    sleep(1);
  }

}
