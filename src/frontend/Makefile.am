AM_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../tcp_splitter $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = koho-client
koho_client_SOURCES = koho-client.cc
koho_client_LDADD = -lrt ../tcp_splitter/libtcpsplitter.a ../util/libutil.a
koho_client_LDFLAGS = -pthread

bin_PROGRAMS += koho-server
koho_server_SOURCES = koho-server.cc
koho_server_LDADD = -lrt ../tcp_splitter/libtcpsplitter.a ../util/libutil.a
koho_server_LDFLAGS = -pthread

install-exec-hook:
	chown root $(DESTDIR)$(bindir)/koho-client
	chmod u+s $(DESTDIR)$(bindir)/koho-client
	chown root $(DESTDIR)$(bindir)/koho-server
	chmod u+s $(DESTDIR)$(bindir)/koho-server
