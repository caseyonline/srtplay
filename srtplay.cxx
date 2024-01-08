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
    printf("srtplay [ -rec | -rmtrec | -rmtsnd | -rtmsnd2 | -broad | -socketOut | socketIn ]\n");
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
  /*
  if ( (client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    cout << "Socket creation error" << endl;
    return;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SOCKET_PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    cout << "invalid address" << endl;
    return;
  }

  sendto(client_fd, hello, strlen(hello), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  sendto(client_fd, hello, strlen(hello2), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  sendto(client_fd, hello, strlen(hello3), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  cout << "data sent" << endl;

  close(client_fd);
  */
}



void socketIn() {

  int server_fd, new_socket;
  struct sockaddr_in address, client_addr;
  socklen_t addrlen = sizeof(address);
  char buf[1024];

  if ( (server_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    cout << "Socket creation error" << endl;
    return;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(SOCKET_PORT);

  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
  {
    cout << "bind error" << endl;
    return;
  }

  socklen_t len = sizeof(client_addr);

  while(true)
  {
    int n = (int)recvfrom(server_fd, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&client_addr, &len);
    cout << "bytes=" << n << endl;
    sleep(1);
  }


}
