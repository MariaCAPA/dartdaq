#export ROOTSYS=/home/anais/buildRoot
export ROOTSYS=/home/anais/buildRoot_v6-20-00
export PATH=$PATH:$ROOTSYS/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTSYS/lib

export ANAIS=/home/anais
export ANALYSIS=$ANAIS/analysis/anais/AnaDAQ/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/bin:/usr/lib:$ANAIS/analysis/libraries/libs:$ANALYSIS
export PATH=$PATH:$ANALYSIS:$ROOTSYS/bin
export FILTERSPATH=$ANAIS/analysis/anais/analyze/filters/


######################
# MIDAS
export GIT_EDITOR="vi"	
export MIDASSYS=$HOME/packages/midas	#Base directory of the MIDAS package
export MIDAS_EXPTAB=/etc/exptab	#MIDAS experiment table
#export MIDAS_EXPT_NAME=siz	#MIDAS experiment name
export ROOTANASYS=$HOME/packages/rootana
export JSROOTSYS=$HOME/packages/jsroot
export PATH=$PATH:$MIDASSYS/bin:$MIDASSYS/build/progs
export PATH=$PATH:/home/anais/AnodDAQ/daq
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/packages/rootana/lib

# DART
export LD_LIBRARY_PATH=/home/anais/AnodDAQ/daq/analysis:$LD_LIBRARY_PATH
export PATH=/home/anais/AnodDAQ/daq/analysis:$PATH


/home/anais/AnodDAQ/daq/enableTrigger
