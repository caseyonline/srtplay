#include "srt/srtcore/srt.h"

#include "outgoingRemoteMediaStream2.hxx"

#include <cassert>
#include <unistd.h>

using namespace std;

OutgoingRemoteMediaStream2::OutgoingRemoteMediaStream2(const char* port)
{
  mPort = port;

  initConnections();
}

OutgoingRemoteMediaStream2::~OutgoingRemoteMediaStream2()
{
  srt_close(mSfd);
  srt_epoll_release(mEpid);
  srt_cleanup();
}

int OutgoingRemoteMediaStream2::start()
{

  srt_startup();
  srt_setloglevel(srt_logging::LogLevel::debug);

  addrinfo hints;
  addrinfo* res;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if (getaddrinfo(NULL, mPort, &hints, &res) != 0) {
    cout << "illegal port num or port in use\n" << endl;
    return 0;
  }

  mSfd = srt_create_socket();  cout << mSfd << endl;
  if (mSfd == SRT_INVALID_SOCK) {
    cout << "srt_socket: " << srt_getlasterror_str() << endl;
    return 0;
  }

  bool no = false;
  if (srt_setsockopt(mSfd, 0, SRTO_RCVSYN, &no, sizeof no)) {
    cout << "srt_setsocketopt: " << srt_getlasterror_str() << endl;
    return 0;
  }

  if (srt_bind(mSfd, res->ai_addr, res->ai_addrlen)) {
    cout << "srt_bind: " << srt_getlasterror_str() << endl;
    return 0;
  }

  freeaddrinfo(res);

  cout << "server ready on port: " << mPort << endl;

  if (SRT_ERROR == srt_listen(mSfd, 10)) {
      cout << "srt_listen: " << srt_getlasterror_str() << endl;
      return 0;
   }

   mEpid = srt_epoll_create();
   if (mEpid < 0) {
      cout << "srt_epoll_create: " << srt_getlasterror_str() << endl;
      return 0;
   }

   int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
   if (SRT_ERROR == srt_epoll_add_usock(mEpid, mSfd, &events)) {
      cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
      return 0;
   }

   return 1;
 }

 int OutgoingRemoteMediaStream2::process(char *frame, int frameSize)
 {
   //cout << "Out" << mSfd << endl;
   const int srtrfdslenmax = 100;
   SRTSOCKET srtrfds[srtrfdslenmax];
   int srtrfdslen = srtrfdslenmax;

   int n = srt_epoll_wait(mEpid, &srtrfds[0], &srtrfdslen, 0, 0, 1, 0, 0, 0, 0);

   for(int i = 0; i < n; i++)
   {
     SRTSOCKET s = srtrfds[i];
     SRT_SOCKSTATUS status = srt_getsockstate(s);
     // check if a connection has been dropped
     if ((status == SRTS_BROKEN) ||
         (status == SRTS_NONEXIST) ||
         (status == SRTS_CLOSED))
     {
        cout << "source disconnected. status=" << status << endl;
        srt_close(s);
        removeConnection(&s);
     }
     else if (s == mSfd)
     {
        // check for new connections
        assert(status == SRTS_LISTENING);

        // can we accept this new connectioin?
        int connIndx = newConnection();
        if (connIndx == -1)
        {
          cout << "unable accept new connection: connection queue is full" << endl;
          return 0;
        }

        sockaddr_storage clientaddr;
        int addrlen = sizeof(clientaddr);

        mConnections[connIndx].srtSock = srt_accept(mSfd, (sockaddr*)&clientaddr, &addrlen);
        if (SRT_INVALID_SOCK == mConnections[connIndx].srtSock)
        {
           cout << "srt_accept: " << srt_getlasterror_str() << endl;
           mConnections[connIndx].srtSock = SRT_INVALID_SOCK;
           return 0;
        }

        getnameinfo((sockaddr *)&clientaddr, addrlen,
                    mConnections[connIndx].clienthost, sizeof(mConnections[connIndx].clienthost),
                    mConnections[connIndx].clientservice, sizeof(mConnections[connIndx].clientservice), NI_NUMERICHOST|NI_NUMERICSERV);
        cout << "new connection: " << mConnections[connIndx].clienthost << ":" << mConnections[connIndx].clientservice << endl;

        int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
        if (SRT_ERROR == srt_epoll_add_usock(mEpid, mConnections[connIndx].srtSock, &events))
        {
           cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
           mConnections[connIndx].srtSock = SRT_INVALID_SOCK;
           return 0;
        }
     }
     else
     {
        // handle new incoming data
        ;
     }
   }

   // loop thru all open connections to send send data
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (mConnections[i].srtSock != SRT_INVALID_SOCK)
      {
        //cout << "sending data .." << endl;

        //char frame[] = {'A','C','E'};
        //int len = 3;

        int ttl = 10; //4000;

        //cout << "Snd" << mConnections[i].srtSock << endl;
        if (SRT_ERROR == srt_sendmsg(mConnections[i].srtSock, (const char *)frame, frameSize, ttl, false))
        {
          cout << "SRT ERROR: " << srt_getlasterror_str() << endl;
        }

        //usleep(100000);

      }
   }

   return 1;
}

void OutgoingRemoteMediaStream2::initConnections()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        mConnections[i].srtSock = SRT_INVALID_SOCK;
}

int OutgoingRemoteMediaStream2::newConnection()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
      if (mConnections[i].srtSock == SRT_INVALID_SOCK)
        return i;

    return -1;
}

void OutgoingRemoteMediaStream2::removeConnection(SRTSOCKET *s)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
      if (mConnections[i].srtSock == *s)
        {
          mConnections[i].srtSock = SRT_INVALID_SOCK;
          break;
        }
}
