AM_CPPFLAGS = -I$(srcdir)/../util -I../tcp_splitter $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

noinst_LIBRARIES = libpacket.a

libpacket_a_SOURCES = bindworkaround.hh tunnelclient.hh tunnelclient.cc \
		      tunnelserver.hh tunnelserver.cc
