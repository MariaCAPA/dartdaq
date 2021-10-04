/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Fri Apr 16 13:13:40 2021

\********************************************************************/

#ifndef EXCL_V1730_DATA00

#define V1730_DATA00_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} V1730_DATA00_COMMON;

#define V1730_DATA00_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 1",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 2",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 1",\
"Period = INT32 : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 0",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] fe1730",\
"Frontend file name = STRING : [256] /home/daquser/testDart//fe1730.cxx",\
"Status = STRING : [256] fe1730@localhost",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 100000",\
"",\
NULL }

#define V1730_DATA00_SETTINGS_DEFINED

typedef struct {
  char      pulsePolarity;
  BOOL      externaltrigger;
  UINT32    recordlength;
  UINT32    posttrigger;
  UINT32    ch_enable[16];
  UINT32    ch_bslpercent[16];
  UINT32    ch_threshold[16];
  float     ch_dynamicrange[16];
  char      ch2_logic[8][32];
  INT32     triggerWidthNs[8];
  INT32     NRequestForCoincidence;
  INT32     coincidenceWindowNs;
} V1730_DATA00_SETTINGS;

#define V1730_DATA00_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"pulse polarity (+,-) = CHAR : -",\
"external trigger (y,n) = BOOL : n",\
"record length (points) = UINT32 : 4000",\
"post-trigger (%) = UINT32 : 70",\
"enable channel = UINT32[16] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"baseline position (%) = UINT32[16] :",\
"[0] 50",\
"[1] 70",\
"[2] 50",\
"[3] 50",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"threshold (ADC counts) = UINT32[16] :",\
"[0] 8000",\
"[1] 8000",\
"[2] 200",\
"[3] 200",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"dynamic range (V) (0.5,2) = FLOAT[16] :",\
"[0] 2",\
"[1] 2",\
"[2] 2",\
"[3] 2",\
"[4] 2",\
"[5] 2",\
"[6] 2",\
"[7] 2",\
"[8] 2",\
"[9] 2",\
"[10] 2",\
"[11] 2",\
"[12] 2",\
"[13] 2",\
"[14] 2",\
"[15] 2",\
"trg (AND,OR,NONE,ONLY0,ONLY1) = STRING[8] :",\
"[32] AND",\
"[32] AND",\
"[32] NONE",\
"[32] NONE",\
"[32] NONE",\
"[32] NONE",\
"[32] NONE",\
"[32] NONE",\
"trigger width (ns) = INT32[8] :",\
"[0] 40",\
"[1] 40",\
"[2] 40",\
"[3] 40",\
"[4] 40",\
"[5] 40",\
"[6] 40",\
"[7] 40",\
"N request for coincidence = INT32 : 1",\
"coincidence window (ns) = INT32 : 120",\
"",\
NULL }

#endif

