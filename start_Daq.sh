#!/bin/sh
# start_daq.sh
cd /storage/online
#odbedit -c clean # MARIA 210921 lo comento por ahora
#   start mhttpd on default port. (Mongoose https version)
mhttpd  -D  -e dartdaq # you can optionally restrict access to localhost and other specified hosts - see mhttpd
xterm -e fe1730 -e dartdaq&
# xterm -e wfViewer.exe -Edartdaq&
mlogger -D -e dartdaq
#end file
# para analizar
# midas2root.exe /storage/online/run00032_??.mid.lz4
