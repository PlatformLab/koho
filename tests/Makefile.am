dist_check_SCRIPTS = end-to-end-tcp.py

installcheck-local:
	$(srcdir)/end-to-end-tcp.py
	$(srcdir)/run-inside-koho.py "curl `route -n | tail -1 | cut -f 1 -d ' '`:0" # curl to last route destination port 0, which should refuse connection
