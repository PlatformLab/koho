dist_check_SCRIPTS = end-to-end-tcp.py

installcheck-local:
	$(srcdir)/end-to-end-tcp.py
	$(srcdir)/run-inside-koho.py ./test_connection_refused.sh
