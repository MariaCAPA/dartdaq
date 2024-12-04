#!/bin/sh
# start_daq.sh
cd /storage/online
#odbedit -c clean # MARIA 210921 lo comento por ahora
#   start mhttpd on default port. (Mongoose https version)
mhttpd  -D  -e dart2daq # you can optionally restrict access to localhost and other specified hosts - see mhttpd
# fe2730Th -e dart2daq
xterm -e fe2730Th -e dart2daq&
xterm -e wfViewer.exe -Edart2daq&
sleep 1
mlogger -D -e dart2daq
#end file
# para analizar
# midas2root.exe /storage/online/run00032_??.mid.lz4
