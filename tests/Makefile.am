dist_check_SCRIPTS = end-to-end-tcp.py

installcheck-local:
	$(srcdir)/end-to-end-tcp.py
	$(srcdir)/run-inside-koho.py "curl localhost:0"; expr $$? = 7 # curl should return peacefully exit code 7: Failed to connect to host.
