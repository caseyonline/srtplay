#include "srt/srtcore/srt.h"

#include "incommingRemoteMediaStream.hxx"

#include <cassert>

using namespace std;

IncommingRemoteMediaStream::IncommingRemoteMediaStream(const char* port, const char* addr)
{
  mPort = port;
  mAddr = addr;
}

IncommingRemoteMediaStream::~IncommingRemoteMediaStream()
{
  srt_close(mServ);
  srt_epoll_release(mEpid);
  srt_cleanup();
}

int IncommingRemoteMediaStream::start()
{
  const int no = 0;
  struct sockaddr_in sa;

  sa.sin_family = AF_INET;
  sa.sin_port = htons(atoi(mPort));
  if (inet_pton(AF_INET, mAddr, &sa.sin_addr) != 1)
    return 0;

  srt_startup();

  mServ = srt_create_socket();
  if (mServ == SRT_INVALID_SOCK) {
    cout << "srt_socket: " << srt_getlasterror_str() << endl;
    return 0;
  }

  mEpid = srt_epoll_create();
  if (mEpid < 0) {
     cout << "srt_epoll_create: " << srt_getlasterror_str() << endl;
     return 0;
  }

  if (SRT_ERROR == srt_setsockflag(mServ, SRTO_RCVSYN, &no, sizeof no )
    || SRT_ERROR == srt_setsockflag(mServ, SRTO_SNDSYN, &no, sizeof no))
    {
      cout << "SRTO_RCVSYN or SRTO_SNDSYN: " << srt_getlasterror_str() << endl;
      return 0;
    }

  int modes = SRT_EPOLL_OUT | SRT_EPOLL_ERR;
  if (SRT_ERROR == srt_epoll_add_usock(mEpid, mServ, &modes))
  {
    cout << "srt_epoll_add_sock: " << srt_getlasterror_str() << endl;
    return 0;
  }

  if (SRT_ERROR == srt_connect(mServ, (struct sockaddr*)&sa, sizeof sa))
  {
    cout << "srt_connect: " << srt_getlasterror_str() << endl;
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
        else if (s == mServ)
        {
           assert(status == SRTS_LISTENING);

           SRTSOCKET fhandle;
           sockaddr_storage clientaddr;
           int addrlen = sizeof(clientaddr);

           fhandle = srt_accept(mServ, (sockaddr*)&clientaddr, &addrlen);
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

           int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
           if (SRT_ERROR == srt_epoll_add_usock(mEpid, fhandle, &events))
           {
              cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
              return 0;
           }
        }
        else
        {
           while (true)
           {
              int ret = srt_recvmsg(s, data, sizeof(data));
              if (SRT_ERROR == ret)
              {
                 // EAGAIN for SRT READING
                 if (SRT_EASYNCRCV != srt_getlasterror(NULL))
                 {
                    cout << "srt_recvmsg: " << srt_getlasterror_str() << endl;
                    return 0;
                 }
                 break;
              }
              cout << ret << " bytes received" << endl;
           }
        }
     }
  }


























/*
  int rlen = 1;
  SRTSOCKET rready;
  int       wlen = 1;
  SRTSOCKET wready;

  int n = srt_epoll_wait(mEpid, &rready, &rlen, &wready, &wlen, -1, 0, 0, 0, 0);
  if (n != -1)
  {
      SRT_SOCKSTATUS state = srt_getsockstate(mServ);
      if (state != SRTS_CONNECTED || rlen > 0) // rlen > 0 - an error notification
      {
          fprintf(stderr, "srt_epoll_wait: reject reason %s\n", srt_rejectreason_str(srt_getrejectreason(rready)));
          return 1;
      }

      if (wlen != 1 || wready != mServ)
      {
          fprintf(stderr, "srt_epoll_wait: wlen %d, wready %d, socket %d\n", wlen, wready, mServ);
          return 1;
      }
  }
  else
  {
      fprintf(stderr, "srt_connect: %s\n", srt_getlasterror_str());
      return 1;
  }
*/

  return 1;
}






/*
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

  mServ = srt_create_socket();
  if (mServ == SRT_INVALID_SOCK) {
    cout << "srt_socket: " << srt_getlasterror_str() << endl;
    return 0;
  }

  bool no = false;
  if (srt_setsockopt(mServ, 0, SRTO_RCVSYN, &no, sizeof no)) {
    cout << "srt_setsocketopt: " << srt_getlasterror_str() << endl;
    return 0;
  }

  if (srt_bind(mServ, res->ai_addr, res->ai_addrlen)) {
    cout << "srt_bind: " << srt_getlasterror_str() << endl;
    return 0;
  }

  //freeaddrinfo(res);

  cout << "server ready on port: " << mPort << endl;

  if (SRT_ERROR == srt_listen(mServ, 10)) {
      cout << "srt_listen: " << srt_getlasterror_str() << endl;
      return 0;
   }

   mEpid = srt_epoll_create();
   if (mEpid < 0) {
      cout << "srt_epoll_create: " << srt_getlasterror_str() << endl;
      return 0;
   }

   int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
   if (SRT_ERROR == srt_epoll_add_usock(mEpid, mServ, &events)) {
      cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
      return 0;
   }








   if (getaddrinfo(mAddr, mPort, &hints, &res) != 0) {
     cout << "illegal port num or port in use\n" << endl;
     return 0;
   }

   if (SRT_ERROR == srt_connect(mServ, res->ai_addr, res->ai_addrlen))
    {
        int rej = srt_getrejectreason(mServ);
        std::cout << "connect: " << srt_getlasterror_str() << ":" << srt_rejectreason_str(rej) << std::endl;
        return 0;
    }

    freeaddrinfo(res);

    const int srtrfdslenmax = 100;
    SRTSOCKET srtrfds[srtrfdslenmax];
    char data[1500];

   // the event loop
   while (true)
   {
      int srtrfdslen = srtrfdslenmax;
      int n = srt_epoll_wait(mEpid, &srtrfds[0], &srtrfdslen, 0, 0, 100, 0, 0, 0, 0);
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
         else if (s == mServ)
         {
            assert(status == SRTS_LISTENING);

            SRTSOCKET fhandle;
            sockaddr_storage clientaddr;
            int addrlen = sizeof(clientaddr);

            fhandle = srt_accept(mServ, (sockaddr*)&clientaddr, &addrlen);
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

            int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
            if (SRT_ERROR == srt_epoll_add_usock(mEpid, fhandle, &events))
            {
               cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
               return 0;
            }
         }
         else
         {
            while (true)
            {
               int ret = srt_recvmsg(s, data, sizeof(data));
               if (SRT_ERROR == ret)
               {
                  // EAGAIN for SRT READING
                  if (SRT_EASYNCRCV != srt_getlasterror(NULL))
                  {
                     cout << "srt_recvmsg: " << srt_getlasterror_str() << endl;
                     return 0;
                  }
                  break;
               }
               //cout << ret << " bytes received" << endl;
            }
         }
      }
   }
*/
