AM_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../packet $(XCBPRESENT_CFLAGS) $(PANGOCAIRO_CFLAGS) $(CXX11_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS += koho-client
koho_client_SOURCES = koho-client.cc trivial_queue.hh
koho_client_LDADD = -lrt ../util/libutil.a ../packet/libpacket.a
koho_client_LDFLAGS = -pthread

bin_PROGRAMS += koho-server
koho_server_SOURCES = koho-server.cc trivial_queue.hh
koho_server_LDADD = -lrt ../util/libutil.a ../packet/libpacket.a
koho_server_LDFLAGS = -pthread

install-exec-hook:
	chown root $(DESTDIR)$(bindir)/koho-client
	chmod u+s $(DESTDIR)$(bindir)/koho-client
	chown root $(DESTDIR)$(bindir)/koho-server
	chmod u+s $(DESTDIR)$(bindir)/koho-server
