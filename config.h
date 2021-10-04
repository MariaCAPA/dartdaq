 struct V1725_CONFIG_SETTINGS {
    INT       acq_mode;                //!< 0x8100@[ 1.. 0]          // CAEN_DGTZ_ACQ_CONTROL_ADD
    BOOL      front_panel_ttl;         //!< 0x811C@[0]               // CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD
    DWORD     board_config;            //!< 0x8000@[19.. 0]          // CAEN_DGTZ_BROAD_CH_CTRL_ADD
    INT       buffer_organization;     //!< 0x800C@[ 3.. 0]          // CAEN_DGTZ_BROAD_NUM_BLOCK_ADD
    INT       custom_size;             //!< 0x8020@[31.. 0]          // CAEN_DGTZ_CUSTOM_SIZE_REG
    DWORD     channel_mask;            //!< 0x8120@[ 7.. 0]          // CAEN_DGTZ_CH_ENABLE_ADD
    DWORD     trigger_source;          //!< 0x810C@[31.. 0]          // CAEN_DGTZ_TRIGGER_SRC_ENABLE_ADD
    DWORD     trigger_output;          //!< 0x8110@[31.. 0]          // CAEN_DGTZ_FP_TRIGGER_OUT_ENABLE_ADD
    DWORD     post_trigger;            //!< 0x8114@[31.. 0]          // CAEN_DGTZ_POST_TRIG_ADD
    // Hard code the two fp_* settings to alway on (Alex 21/2/13)
    //    DWORD     fp_io_ctrl;        //!< 0x811C@[31.. 0]          // CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD
    DWORD     enable_zle;             //!< 0x816C@[31.. 0]           // NO LO TENGO
    DWORD     almost_full;             //!< 0x816C@[31.. 0]          // NO LO TENGO
    //    DWORD     fp_lvds_io_ctrl;   //!< 0x81A0@[31.. 0]          // NO LO TENGO
    DWORD     selftrigger_threshold[16];//!< 0x1n80@[11.. 0]         // ? CAEN_DGTZ_CHANNEL_THRESHOLD_BASE_ADDRESS
    DWORD     selftrigger_logic[8];    //!< 0x1n84@[11.. 0]          // ? CAEN_DGTZ_CHANNEL_OV_UND_TRSH_BASE_ADDRESS
    INT       zle_signed_threshold[16];//!< 0x1n24@[31.. 0]          // ? CAEN_DGTZ_CHANNEL_ZS_THRESHOLD_BASE_ADDRESS
    INT       zle_bins_before[16];     //!< 0x1n54@[31.. 16]        // ? CAEN_DGTZ_SAM_DAC_SPI_DATA_ADD
    INT       zle_bins_after[16];      //!< 0x1n58@[15.. 0]         // ? CAEN_DGTZ_SAM_START_CELL_CH0
    DWORD     dac[16];                 //!< 0x1n98@[15.. 0]         // ? CAEN_DGTZ_CHANNEL_DAC_BASE_ADDRESS
    BOOL      dynamic_range_2v[16];    //!< 0x1n28@[0]             // ? CAEN_DGTZ_CHANNEL_ZS_NSAMPLE_BASE_ADDRESS 
  } config; //!< instance of config structure

