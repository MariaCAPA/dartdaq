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

  Created on:   Tue Jun  7 18:08:23 2022

\********************************************************************/

#ifndef EXCL_V2730_DATA00

#define bsl_BANK_DEFINED

typedef struct {
  float     bsl[32];
} BSL_BANK;

#define BSL_BANK_STR(_name) const char *_name[] = {\
"[.]",\
"bsl = FLOAT[32] :",\
"[0] 2842.79",\
"[1] 2826.539",\
"[2] -8.428499e+31",\
"[3] 4.591214e-41",\
"[4] 0",\
"[5] 0",\
"[6] 1.401298e-45",\
"[7] 0",\
"[8] 4.445659e+34",\
"[9] 3.075009e-41",\
"[10] 1.121039e-44",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"[27] 0",\
"[28] 0",\
"[29] 0",\
"[30] 0",\
"[31] 0",\
"",\
NULL }

#define rms_BANK_DEFINED

typedef struct {
  float     rms[32];
} RMS_BANK;

#define RMS_BANK_STR(_name) const char *_name[] = {\
"[.]",\
"rms = FLOAT[32] :",\
"[0] 2488.043",\
"[1] 2470.712",\
"[2] 7.287124e-38",\
"[3] 4.567672e-41",\
"[4] 1.3778e+34",\
"[5] 3.075009e-41",\
"[6] 1.527908e+32",\
"[7] 3.075009e-41",\
"[8] 1.401298e-45",\
"[9] 7.006492e-42",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"[27] 0",\
"[28] 0",\
"[29] 0",\
"[30] 0",\
"[31] 0",\
"",\
NULL }

#define V2730_DATA00_COMMON_DEFINED

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
} V2730_DATA00_COMMON;

#define V2730_DATA00_COMMON_STR(_name) const char *_name[] = {\
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
"Frontend name = STRING : [32] fe2730Th",\
"Frontend file name = STRING : [256] /home/daquser/dart2daq/fe2730Th.cxx",\
"Status = STRING : [256] fe2730Thlocalhost",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#define V2730_DATA00_SETTINGS_DEFINED

typedef struct {
  char      pulsePolarity;
  BOOL      externaltrigger;
  UINT32    recordlength;
  UINT32    pretrigger;
  UINT32    ch_enable[32];
  UINT32    ch_bslpercent[32];
  float     ch_threshold[32]; // in mV
  float     ch_gain[32]; // from 1 to 28
  INT32     triggerWidthNs[32];
  INT32     NRequestForCoincidence;
  BOOL      grid_display; // requested in new version
} V2730_DATA00_SETTINGS;

#define V2730_DATA00_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"pulse polarity (+,-) = CHAR : +",\
"external trigger (y,n) = BOOL : n",\
"record length (points) = UINT32 : 5000",\
"pretrigger (points) = UINT32 : 80",\
"enable channel = UINT32[32] :",\
"[0] 1",\
"[1] 1",\
"[2] 0",\
"[3] 0",\
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
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"[27] 0",\
"[28] 0",\
"[29] 0",\
"[30] 0",\
"[31] 0",\
"baseline position (%) = UINT32[32] :",\
"[0] 90",\
"[1] 90",\
"[2] 50",\
"[3] 50",\
"[4] 50",\
"[5] 50",\
"[6] 50",\
"[7] 50",\
"[8] 90",\
"[9] 10",\
"[10] 10",\
"[11] 50",\
"[12] 50",\
"[13] 50",\
"[14] 50",\
"[15] 50",\
"[16] 50",\
"[17] 50",\
"[18] 50",\
"[19] 50",\
"[20] 50",\
"[21] 50",\
"[22] 50",\
"[23] 50",\
"[24] 50",\
"[25] 50",\
"[26] 50",\
"[27] 50",\
"[28] 50",\
"[29] 50",\
"[30] 50",\
"[31] 50",\
"threshold (mV) = FLOAT[32] :",\
"[0] 200",\
"[1] 200",\
"[2] 0",\
"[3] 0",\
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
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"[27] 0",\
"[28] 0",\
"[29] 0",\
"[30] 0",\
"[31] 0",\
"gain (1,28) = FLOAT[32] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"[16] 1",\
"[17] 1",\
"[18] 1",\
"[19] 1",\
"[20] 1",\
"[21] 1",\
"[22] 1",\
"[23] 1",\
"[24] 1",\
"[25] 1",\
"[26] 1",\
"[27] 1",\
"[28] 1",\
"[29] 1",\
"[30] 1",\
"[31] 1",\
"trigger width (ns) = INT32[32] :",\
"[0] 40",\
"[1] 40",\
"[2] 40",\
"[3] 40",\
"[4] 40",\
"[5] 40",\
"[6] 40",\
"[7] 40",\
"[8] 40",\
"[9] 40",\
"[10] 40",\
"[11] 40",\
"[12] 40",\
"[13] 40",\
"[14] 40",\
"[15] 40",\
"[16] 40",\
"[17] 40",\
"[18] 40",\
"[19] 40",\
"[20] 40",\
"[21] 40",\
"[22] 40",\
"[23] 40",\
"[24] 40",\
"[25] 40",\
"[26] 40",\
"[27] 40",\
"[28] 40",\
"[29] 40",\
"[30] 40",\
"[31] 40",\
"N request for coincidence = INT32 : 1",\
"Grid display = BOOL : n",\
"",\
NULL }

#endif

