curl `route -n | tail -1 | cut -f 1 -d ' '`:1;
expr $? = 52 && echo "curl to a destination that should refuse connection returned exit code 52 as expected"
