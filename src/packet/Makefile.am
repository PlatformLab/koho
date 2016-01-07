AM_CPPFLAGS = -I$(srcdir)/../util -I../protobuf $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

noinst_LIBRARIES = libpacket.a

libpacket_a_SOURCES = bindworkaround.hh tunnelclient.hh tunnelclient.cc \
		      tunnelserver.hh tunnelserver.cc \
              tcp_splitter_client.hh tcp_splitter_client.cc \
              tcp_splitter_server.hh tcp_splitter_server.cc \
              tcp_splitter_common.hh split_tcp_connection.hh
