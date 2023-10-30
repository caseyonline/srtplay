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

  // wait for connection response or timeout after 1 second
  int       rlen = 1;
  SRTSOCKET rready;
  int       wlen = 1;
  SRTSOCKET wready;
  if (srt_epoll_wait(mEpid, &rready, &rlen, &wready, &wlen, 1000, 0, 0, 0, 0) != -1)
  {
    SRT_SOCKSTATUS state = srt_getsockstate(mServ);
    if (state != SRTS_CONNECTED || rlen > 0) // rlen > 0 - an error notification
    {
        cout << "srt_epoll_wait: reject reason " << srt_rejectreason_str(srt_getrejectreason(rready)) << endl;
        return 0;
    }

    if (wlen != 1 || wready != mServ)
    {
        cout << "srt_epoll_wait: wlen " << wlen << ", wready " << wready << ", socket " << endl;
        return 0;
    }
  }
  else
  {
    cout << "srt_connect: " << srt_getlasterror_str() << endl;
    return 0;
  }

  cout << "connected!" << endl;

  char data[1500];
  while (true)
  {
     int ret = srt_recvmsg(mServ, data, sizeof(data));
     if (SRT_ERROR == ret)
     {
        // EAGAIN for SRT READING
        if (SRT_EASYNCRCV != srt_getlasterror(NULL))
        {
           cout << "srt_recvmsg: " << srt_getlasterror_str() << endl;
           return 0;
        }
     }
     else
        cout << ret << " bytes received" << endl;
  }

  return 0;

}
