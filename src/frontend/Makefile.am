AM_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../tcp_splitter $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = koho-server
koho_server_SOURCES = koho-server.cc
koho_server_LDADD = -lrt ../util/libutil.a ../tcp_splitter/libtcpsplitter.a
koho_server_LDFLAGS = -pthread

install-exec-hook:
	hown root $(DESTDIR)$(bindir)/koho-server
	chmod u+s $(DESTDIR)$(bindir)/koho-server
