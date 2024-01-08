#include "srt/srtcore/srt.h"

#include "incommingMediaStream.hxx"

#include <cassert>

using namespace std;

IncommingMediaStream::IncommingMediaStream(const char* port)
{
  mPort = port;
}

IncommingMediaStream::~IncommingMediaStream()
{
  srt_close(mSfd);
  srt_epoll_release(mEpid);
  srt_cleanup();
}

int IncommingMediaStream::start()
{

  int r = srt_startup();    printf("\nsrt_startup: %i\n",r);
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

  mSfd = srt_create_socket(); cout << mSfd << endl;
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

int IncommingMediaStream::process(char *frame, int frameSize)
{
   //cout << "In " << mSfd << endl;
   const int srtrfdslenmax = 100;
   SRTSOCKET srtrfds[srtrfdslenmax];

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
          return 0;
      }
      else if (s == mSfd)
      {
          // TODO: look to prevent more than one incoming connection?
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

          int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
          if (SRT_ERROR == srt_epoll_add_usock(mEpid, fhandle, &events))
          {
            cout << "srt_epoll_add_usock: " << srt_getlasterror_str() << endl;
            return 0;
          }
      }
      else
      {
          //cout << "Rec" << s << endl;
          int ret = srt_recvmsg(s, frame, frameSize);
          if (SRT_ERROR == ret)
          {
            // EAGAIN for SRT READING
            if (SRT_EASYNCRCV != srt_getlasterror(NULL))
            {
              cout << "srt_recvmsg: " << srt_getlasterror_str() << endl;
            }
            return 0;
          }
          //printf("%02X\n",(unsigned char)frame[1000]);
          return ret;
      }
   }
   return -1;

}
