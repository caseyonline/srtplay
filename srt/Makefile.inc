LIBSRTDIR ?= .

SRTCOREDIR ?= $(LIBSRTDIR)/srtcore
HAICRYPTDIR ?= $(LIBSRTDIR)/haicrypt

CCFLAGS += \
	-DCRYSPR2 -DENABLE_MONOTONIC_CLOCK=1 -DENABLE_NEW_RCVBUFFER=1 -DENABLE_SOCK_CLOEXEC=1 -DHAI_ENABLE_SRT=1 -DHAI_PATCH=1 -DHAVE_INET_PTON=1 -DHAVE_PTHREAD_GETNAME_NP=1 -DHAVE_PTHREAD_SETNAME_NP=1 -DNDEBUG -DSRT_DYNAMIC -DSRT_ENABLE_APP_READER -DSRT_ENABLE_CLOSE_SYNCH -DSRT_ENABLE_ENCRYPTION -DSRT_EXPORTS -DSRT_VERSION=\"1.5.0\" -DUSE_OPENSSL=1 -D_GNU_SOURCE

CCFLAGS += -DENABLE_BONDING=1

CXXFLAGS += \
	-Wno-deprecated-declarations \
	$(CCFLAGS)

LIBSRT_SRCS = \
	$(SRTCOREDIR)/api.cpp \
	$(SRTCOREDIR)/buffer.cpp \
	$(SRTCOREDIR)/buffer_rcv.cpp \
	$(SRTCOREDIR)/cache.cpp \
	$(SRTCOREDIR)/channel.cpp \
	$(SRTCOREDIR)/common.cpp \
	$(SRTCOREDIR)/congctl.cpp \
	$(SRTCOREDIR)/core.cpp \
	$(SRTCOREDIR)/crypto.cpp \
	$(SRTCOREDIR)/epoll.cpp \
	$(SRTCOREDIR)/fec.cpp \
	$(SRTCOREDIR)/group.cpp \
	$(SRTCOREDIR)/group_backup.cpp \
	$(SRTCOREDIR)/group_common.cpp \
	$(SRTCOREDIR)/handshake.cpp \
	$(SRTCOREDIR)/list.cpp \
	$(SRTCOREDIR)/logger_default.cpp \
	$(SRTCOREDIR)/logger_defs.cpp \
	$(SRTCOREDIR)/md5.cpp \
	$(SRTCOREDIR)/packet.cpp \
	$(SRTCOREDIR)/packetfilter.cpp \
	$(SRTCOREDIR)/queue.cpp \
	$(SRTCOREDIR)/socketconfig.cpp \
	$(SRTCOREDIR)/srt_c_api.cpp \
	$(SRTCOREDIR)/strerror_defs.cpp \
	$(SRTCOREDIR)/sync.cpp \
	$(SRTCOREDIR)/sync_posix.cpp \
	$(SRTCOREDIR)/tsbpd_time.cpp \
	$(SRTCOREDIR)/window.cpp

#	$(SRTCOREDIR)/sync_cxx11.cpp \

HAICRYPT_SRCS = \
	$(HAICRYPTDIR)/cryspr-openssl.c \
	$(HAICRYPTDIR)/cryspr.c \
	$(HAICRYPTDIR)/hcrypt.c \
	$(HAICRYPTDIR)/hcrypt_ctx_rx.c \
	$(HAICRYPTDIR)/hcrypt_ctx_tx.c \
	$(HAICRYPTDIR)/hcrypt_rx.c \
	$(HAICRYPTDIR)/hcrypt_sa.c \
	$(HAICRYPTDIR)/hcrypt_tx.c \
	$(HAICRYPTDIR)/hcrypt_xpt_srt.c

#	$(HAICRYPTDIR)/hcrypt_ut.c \
#	$(HAICRYPTDIR)/cryspr-openssl-evp.c \
#	$(HAICRYPTDIR)/cryspr-gnutls.c \
#	$(HAICRYPTDIR)/cryspr-mbedtls.c \

LIBSRT_OBJS = $(LIBSRT_SRCS:.cpp=$(SYS).o) $(HAICRYPT_SRCS:.c=$(SYS).o)

INC += -I$(HAICRYPTDIR) -I$(SRTCOREDIR)
