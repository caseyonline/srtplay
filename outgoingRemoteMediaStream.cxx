#include "srt/srtcore/srt.h"

#include "outgoingRemoteMediaStream.hxx"

#include <cassert>

using namespace std;

OutgoingRemoteMediaStream::OutgoingRemoteMediaStream(const char* port)
{
  mPort = port;
}

OutgoingRemoteMediaStream::~OutgoingRemoteMediaStream()
{
  srt_close(mSfd);
  srt_epoll_release(mEpid);
  srt_cleanup();
}

int OutgoingRemoteMediaStream::start()
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

  mSfd = srt_create_socket();
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

   const int srtrfdslenmax = 100;
   SRTSOCKET srtrfds[srtrfdslenmax];
   char data[1500];

   // the event loop
   while (true)
   {
      int srtrfdslen = srtrfdslenmax;
      int n = srt_epoll_wait(mEpid, &srtrfds[0], &srtrfdslen, 0, 0, 100, 0, 0, 0, 0);
      //printf("done: %i\n",n);
      assert(n <= srtrfdslen);
      for (int i = 0; i < n; i++)
      {
         SRTSOCKET s = srtrfds[i];
         SRT_SOCKSTATUS status = srt_getsockstate(s);
         if ((status == SRTS_BROKEN) ||
             (status == SRTS_NONEXIST) ||
             (status == SRTS_CLOSED))
         {
            cout << "source disconnected. status=" << status << endl;
            srt_close(s);
            continue;
         }
         else if (s == mSfd)
         {
            assert(status == SRTS_LISTENING);

            SRTSOCKET fhandle;
            sockaddr_storage clientaddr;
            int addrlen = sizeof(clientaddr);

            fhandle = srt_accept(mSfd, (sockaddr*)&clientaddr, &addrlen);
            if (SRT_INVALID_SOCK == fhandle)
            {
               cout << "srt_accept: " << srt_getlasterror_str() << endl;
               return 0;
            }

            char clienthost[NI_MAXHOST];
            char clientservice[NI_MAXSERV];
            getnameinfo((sockaddr *)&clientaddr, addrlen,
                        clienthost, sizeof(clienthost),
                        clientservice, sizeof(clientservice), NI_NUMERICHOST|NI_NUMERICSERV);
            cout << "new connection: " << clienthost << ":" << clientservice << endl;

            /*
            int events = SRT_EPOLL_OUT | SRT_EPOLL_ERR;
            if (SRT_ERROR == srt_epoll_add_usock(mEpid, fhandle, &events))
            {
               cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
               return 0;
            }
            */

            cout << "sending data .." << endl;

            char frame[] = {'A','C','E'};
            int len = 3;

             while (true)
             {
               int ttl = 4000;

               if (SRT_ERROR == srt_sendmsg(fhandle, (const char *)frame, len, ttl, false))
               {
                 cout << "SRT ERROR: " << srt_getlasterror_str() << endl;
               }
               sleep(1);
             }


         }
         else
         {
            ;
         }
      }
   }
   return 1;

}
