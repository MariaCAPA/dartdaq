#!/bin/sh
# start_daq.sh
cd /storage/online
#odbedit -c clean # MARIA 210921 lo comento por ahora
#   start mhttpd on default port. (Mongoose https version)
mhttpd  -D  -e anais # you can optionally restrict access to localhost and other specified hosts - see mhttpd
# fe1730Th -e anais
xterm -e fe1730Th -e anais&
xterm -e wfViewer.exe -Eanais&
sleep 1
mlogger -D -e anais
#end file
# para analizar
# midas2root.exe /storage/online/run00032_??.mid.lz4
