#!/bin/sh
# start_daq.sh
#odbedit -c clean # MARIA 210921 lo comento por ahora
#   start mhttpd on default port. (Mongoose https version)
mhttpd  -D  -e anod32 # you can optionally restrict access to localhost and other specified hosts - see mhttpd
xterm -e /home/dart/anod32/fe2730Th -e anod32&
xterm -e /home/dart/anod32/analysis/wfViewer.exe -Eanod32&
sleep 1
mlogger -D -e anod32
#end file
# para analizar
# midas2root.exe /storage/online/run00032_??.mid.lz4
