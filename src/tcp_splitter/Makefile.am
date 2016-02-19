AM_CPPFLAGS = -I$(srcdir)/../util $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

noinst_LIBRARIES = libtcpsplitter.a
libtcpsplitter_a_SOURCES = tcp_splitter_server.hh tcp_splitter_server.cc \
                           tcp_splitter_client.hh tcp_splitter_client.cc \
                           tcp_splitter_common.hh split_tcp_connection.hh \
                           split_tcp_packet.hh split_tcp_packet.cc

bin_PROGRAMS = split-tcp-packet-test
split_tcp_packet_test_SOURCES = split_tcp_packet_test.cc
split_tcp_packet_test_LDADD = -lrt ../tcp_splitter/libtcpsplitter.a ../util/libutil.a
split_tcp_packet_test_LDFLAGS = -pthread

check-local:
	$(srcdir)/split-tcp-packet-test
