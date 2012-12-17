#include	"ccd_dc_ext.h"

/*
 *-------------------------------------------
 *
 *	Miscellaneous routines in this module.
 *
 *-------------------------------------------
 */

/*
 *	ccd_initialize
 *
 *	ccd_initialize translates logical names
 *	into actual file names (environment for
 *	UNIX, logical name table for VMS) and
 *	opens them in their appropriate states.
 *
 *	Next, ccd_initialize assigns default values
 *	of scanner times or attributes to the
 *	variables labled "specific...".
 *
 *	Then ccd_initialize reads the user's
 *	configuration file and alters the
 *	"specific..." variables based on that
 *	file's contents.
 *
 *	Finally, the "specific..." variables are
 *	used to assign values to variables which
 *	the scanner actually uses.
 *
 *	So the order of precedence:
 *
 *	  1)	mardefs.h	(lowest)
 *	  2)	user config	(next)
 *	  3)	values from profile (highest)
 */

ccd_initialize()
  {
	long	clock;
	char	*cptr;

	fpout = NULL;
	fplog = NULL;
	fpconfig = NULL;
	fdcom = fdout = fdstat = fdxfcm = -1;
	fddetcmd = fddetstatus = fdblcmd = fdblstatus = -1;
	if(NULL == (fpnull = fopen("/dev/null","r+")))
	  {
	    fprintf(stderr,"ccd_dc: ccd_initialize: cannot open /dev/null\n");
	    exit(0);
	  }

	ccd_init_files();
/*
 *	Log the time to the errorlog for startup
 */
	time(&clock);
	cptr = (char *) ctime(&clock);
	fprintf(fplog,"=============================\n");
	fprintf(fplog,"ccd_dc: started %s\n",cptr);
	fprintf(fplog,"=============================\n");

	ccd_init_defaults();
	ccd_init_config(fpnull);
	ccd_init_vars();
	strcpy(mdc_alert,"");
	dc_in_progress = 0;
	perform_beamstop_check = 0;
	checking_direct_beam = 0;
	if(NULL != (cptr = (char *) getenv("CCD_BCHK_SEMIPHORE_FILE")))
	  {
		perform_beamstop_check = 1;
		strcpy(bchk_semiphore_file,cptr);
	  }  
  }

/*
 *	This routine assigns the first round of
 *	values to the "specific..." variables.
 *
 *	The defaults come from this programs
 * 	ccd_dc_defs.h file.  Variables which change
 *	from scanner to scanner or from some
 *	other reason will be altered in the next
 *	phase of initialization.
 */

ccd_init_defaults()
  {
	specific_erase_time = SPECIFIC_ERASE_TIME;
	specific_scan_time = SPECIFIC_SCAN_TIME;
	specific_dc_erase_time = SPECIFIC_DC_ERASE_TIME;	
	specific_total_valid_blocks = SPECIFIC_TOTAL_VALID_BLOCKS;
	specific_total_pixels_x = SPECIFIC_TOTAL_PIXELS_X;
	specific_total_pixels_y = SPECIFIC_TOTAL_PIXELS_Y;
	specific_multiplier = SPECIFIC_MULTIPLIER;
	specific_phi_steps_deg = SPECIFIC_PHI_STEPS_DEG;
	specific_dist_steps_mm = SPECIFIC_DIST_STEPS_MM;
	specific_lift_steps_mm = SPECIFIC_LIFT_STEPS_MM;
	specific_phi_top_speed = SPECIFIC_PHI_TOP_SPEED;
	specific_dist_top_speed = SPECIFIC_DIST_TOP_SPEED;
	specific_lift_top_speed = SPECIFIC_LIFT_TOP_SPEED;
	specific_dist_max_point = SPECIFIC_DIST_MAX_POINT;
	specific_dist_min_point = SPECIFIC_DIST_MIN_POINT;
	specific_lift_max_point = SPECIFIC_LIFT_MAX_POINT;
	specific_lift_min_point = SPECIFIC_LIFT_MIN_POINT;
	specific_units_per_sec = SPECIFIC_UNITS_PER_SEC;
	specific_units_per_dose = SPECIFIC_UNITS_PER_DOSE;
	specific_wavelength = SPECIFIC_WAVELENGTH;
	specific_is_distance = SPECIFIC_IS_DIST;
	specific_is_phi = SPECIFIC_IS_PHI;
	specific_is_lift = SPECIFIC_IS_LIFT;
	specific_flags = SPECIFIC_FLAGS;
	specific_nc_pointer = SPECIFIC_NC_POINTER;
	specific_nc_index = SPECIFIC_NC_INDEX;
	specific_nc_x = SPECIFIC_NC_X;	
	specific_nc_y = SPECIFIC_NC_Y;
	specific_nc_rec = SPECIFIC_NC_REC;
	specific_nc_poff = SPECIFIC_NC_POFF;
	specific_scsi_id = SPECIFIC_SCSI_ID;
	specific_scsi_controller = SPECIFIC_SCSI_CONTROLLER;
	specific_spiral_check = SPECIFIC_SPIRAL_CHECK;
	specific_read_fast = SPECIFIC_READ_FAST;
	specific_read_slow = SPECIFIC_READ_SLOW;
	specific_read_overhead = SPECIFIC_READ_OVERHEAD;
	specific_bin_factor = SPECIFIC_BIN_FACTOR;
	specific_is_kappa = SPECIFIC_IS_KAPPA;
	specific_is_omega = SPECIFIC_IS_OMEGA;
	specific_def_dezinger = SPECIFIC_DEF_DEZINGER;
	specific_is_2theta = SPECIFIC_IS_2THETA;
	specific_pcshutter = SPECIFIC_PCSHUTTER;
	specific_dark_interval = SPECIFIC_DARK_INTERVAL;
	specific_pixel_size = SPECIFIC_PIXEL_SIZE;
	specific_dk_before_run = SPECIFIC_DK_BEFORE_RUN;
	specific_repeat_dark = repeat_dark_current; 
	specific_outfile_type = SPECIFIC_OUTFILE_TYPE;
	specific_detector_sn = SPECIFIC_DETECTOR_SN;
	specific_no_transform = SPECIFIC_NO_TRANSFORM;
	specific_output_raws = SPECIFIC_OUTPUT_RAWS;
	specific_j5_trigger = SPECIFIC_J5_TRIGGER;
	specific_timecheck = SPECIFIC_TIMECHECK;
	specific_constrain_omega = SPECIFIC_CONSTRAIN_OMEGA;
	specific_constrain_phi   = SPECIFIC_CONSTRAIN_PHI;
	specific_constrain_kappa = SPECIFIC_CONSTRAIN_KAPPA;
	specific_strip_ave = SPECIFIC_STRIP_AVE;
	specific_bchk_time = SPECIFIC_BCHK_TIME;
	specific_bchk_deltaphi = SPECIFIC_BCHK_DELTAPHI;
	specific_is_wavelength = SPECIFIC_IS_WAVELENGTH;
	specific_approach_start = SPECIFIC_APPROACH_START;
	specific_chip_size_x = SPECIFIC_CHIP_SIZE_X;
	specific_chip_size_y = SPECIFIC_CHIP_SIZE_Y;
	specific_kappa_const = SPECIFIC_KAPPA_CONST;
	specific_madrun_naming = SPECIFIC_MADRUN_NAMING;
	specific_retryshort = SPECIFIC_RETRYSHORT;
	specific_ccd_modular = SPECIFIC_CCD_MODULAR;
        specific_pf_mod = SPECIFIC_PF_MOD;
        specific_usestop_immediate = SPECIFIC_USESTOP_IMMEDIATE;
	specific_min_velocity = SPECIFIC_MIN_VELOCITY;
	specific_max_velocity = SPECIFIC_MAX_VELOCITY;
	specific_allow_stills = SPECIFIC_ALLOW_STILLS;
	specific_pitch_tune_delta = SPECIFIC_PITCH_TUNE_DELTA;
	specific_bm8_mod = SPECIFIC_PITCH_TUNE_DELTA;
	specific_bl_is_server = SPECIFIC_BL_IS_SERVER;
	specific_beam_sense = SPECIFIC_BEAM_SENSE;
	specific_t2k_detector = SPECIFIC_T2K_DETECTOR;
	specific_adsc_slit = SPECIFIC_ADSC_SLIT;
  }

/*
 *	This routine initializes scanner global
 *	variables now that the program has decided
 *	what the specific parameters actually are.
 */

ccd_init_vars()
  {
	int	mdc_sim_progress();
	int	ccd_hw_progress();
	void	mdc_sim_start();
	void	ccd_hw_start();

	if(mdc_simulation)
	  {
		dt_stat = 1.0;
		erase_time = specific_erase_time;
		scan_time = specific_scan_time;
		dc_erase_time = specific_dc_erase_time;
		phi_steps_deg = specific_phi_steps_deg;
		dist_steps_mm = specific_dist_steps_mm;
		lift_steps_mm = specific_lift_steps_mm;
		phi_top_speed = specific_phi_top_speed;
		dist_top_speed = specific_dist_top_speed;
		lift_top_speed = specific_lift_top_speed;
		dist_max_ref_point = specific_dist_max_point;
		dist_min_ref_point = specific_dist_min_point;
		lift_max_point = specific_lift_max_point;
		lift_min_point = specific_lift_min_point;
		units_per_second = specific_units_per_sec;
		units_per_dose = specific_units_per_dose;
		stat_wavelength = specific_wavelength;
		stat_multiplier = specific_multiplier;
		magic_flags = specific_flags;
		is_distance = specific_is_distance;
		is_phi = specific_is_phi;
		is_lift = specific_is_lift;
		is_2theta = specific_is_2theta;
		dark_current_interval = specific_dark_interval;
		pixel_size = specific_pixel_size;
		dk_before_run = specific_dk_before_run;
		repeat_dark_current = specific_repeat_dark;
		strip_ave = specific_strip_ave;
		bchk_time = specific_bchk_time;
		bchk_deltaphi = specific_bchk_deltaphi;
		is_wavelength = specific_is_wavelength;
		approach_start = specific_approach_start;
		chip_size_x = specific_chip_size_x;
		chip_size_y = specific_chip_size_y;
		kappa_const = specific_kappa_const;
		madrun_naming = specific_madrun_naming;
		retryshort = specific_retryshort;
		ccd_modular = specific_ccd_modular;
		pf_mod = specific_pf_mod;
		usestop_immediate = specific_usestop_immediate;
		stat_attenuator = -1;	/* always begins as unknown */
		stat_experiment_mode = 0;	/* always begins as unknown */
		stat_hslit = -1;
		stat_vslit = -1;
		min_velocity = specific_min_velocity;
		max_velocity = specific_max_velocity;
		allow_stills = specific_allow_stills;
		pitch_tune_delta = specific_pitch_tune_delta;
		bm8_mod = specific_bm8_mod;
		bl_is_server = specific_bl_is_server;
		beam_sense = specific_beam_sense;
		t2k_detector = specific_t2k_detector;
		adsc_slit = specific_adsc_slit;

		nc_pointer = specific_nc_pointer;
		nc_index = specific_nc_index;
		nc_x = specific_nc_x;	
		nc_y = specific_nc_y;
		nc_rec = specific_nc_rec;
		nc_poff = specific_nc_poff;

		stat_mode = 0;
		scsi_id = specific_scsi_id;
		scsi_controller = specific_scsi_controller;
		spiral_check = specific_spiral_check;

		beam_xcen = 45.;
		beam_ycen = 45.;

		outfile_type = specific_outfile_type;
		detector_sn = specific_detector_sn;
		raw_ccd_image = specific_no_transform;
		output_raws = specific_output_raws;
		constrain_omega = specific_constrain_omega;
		constrain_phi   = specific_constrain_phi;
		constrain_kappa = specific_constrain_kappa;

		mdc_cmd_progress = mdc_sim_progress;
		mdc_cmd_start = mdc_sim_start;

		mdc_sim_initial_status();
	  }
	 else
	  {
		dt_stat = 1.0;
		erase_time = specific_erase_time;
		dc_erase_time = specific_dc_erase_time;
		scan_time = specific_scan_time;
		phi_steps_deg = specific_phi_steps_deg;
		dist_steps_mm = specific_dist_steps_mm;
		lift_steps_mm = specific_lift_steps_mm;
		phi_top_speed = specific_phi_top_speed;
		dist_top_speed = specific_dist_top_speed;
		lift_top_speed = specific_lift_top_speed;
		dist_max_ref_point = specific_dist_max_point;
		dist_min_ref_point = specific_dist_min_point;
		lift_max_point = specific_lift_max_point;
		lift_min_point = specific_lift_min_point;
		units_per_second = specific_units_per_sec;
		units_per_dose = specific_units_per_dose;
		stat_wavelength = specific_wavelength;
		stat_multiplier = specific_multiplier;
		magic_flags = specific_flags;
		is_distance = specific_is_distance;
		is_phi = specific_is_phi;
		is_lift = specific_is_lift;
		read_fast = specific_read_fast;
		read_slow = specific_read_slow;
		read_overhead = specific_read_overhead;
		bin_factor = specific_bin_factor;
		is_kappa = specific_is_kappa;
		is_omega = specific_is_omega;
		def_dezinger = specific_def_dezinger;
		is_2theta = specific_is_2theta;
		use_pc_shutter = specific_pcshutter;
		use_j5_trigger = specific_j5_trigger;
		use_timecheck = specific_timecheck;
		dark_current_interval = specific_dark_interval;
		pixel_size = specific_pixel_size;
		dk_before_run = specific_dk_before_run;
		repeat_dark_current = specific_repeat_dark;
		strip_ave = specific_strip_ave;
		bchk_time = specific_bchk_time;
		bchk_deltaphi = specific_bchk_deltaphi;
		is_wavelength = specific_is_wavelength;
		approach_start = specific_approach_start;
		chip_size_x = specific_chip_size_x;
		chip_size_y = specific_chip_size_y;
		kappa_const = specific_kappa_const;
		madrun_naming = specific_madrun_naming;
		retryshort = specific_retryshort;
		ccd_modular = specific_ccd_modular;
		pf_mod = specific_pf_mod;
		usestop_immediate = specific_usestop_immediate;
		stat_attenuator = -1;		/* always begins as unknown */
		stat_experiment_mode = 0;		/* always begins as unknown */
		stat_hslit = -1;
		stat_vslit = -1;
		min_velocity = specific_min_velocity;
		max_velocity = specific_max_velocity;
		allow_stills = specific_allow_stills;
		pitch_tune_delta = specific_pitch_tune_delta;
		bm8_mod = specific_bm8_mod;
		bl_is_server = specific_bl_is_server;
		beam_sense = specific_beam_sense;
		t2k_detector = specific_t2k_detector;
		adsc_slit = specific_adsc_slit;

		nc_pointer = specific_nc_pointer;
		nc_index = specific_nc_index;
		nc_x = specific_nc_x;	
		nc_y = specific_nc_y;
		nc_rec = specific_nc_rec;
		nc_poff = specific_nc_poff;

		stat_mode = 0;
		scsi_id = specific_scsi_id;
		scsi_controller = specific_scsi_controller;
		spiral_check = specific_spiral_check;

		beam_xcen = 45.;
		beam_ycen = 45.;

		outfile_type = specific_outfile_type;
		detector_sn = specific_detector_sn;
		raw_ccd_image = specific_no_transform;
		output_raws = specific_output_raws;
		constrain_omega = specific_constrain_omega;
		constrain_phi   = specific_constrain_phi;
		constrain_kappa = specific_constrain_kappa;

		mdc_cmd_progress = ccd_hw_progress;
		mdc_cmd_start = ccd_hw_start;

		fdmar = -1;	/* this module does NOT perform hardware operatons */
	  }
  }

/*
 *	This routine handles the name translation
 *	and file opens, leaving all in their proper
 *	state.
 *
 *	Network version:
 *	  Open up the log file.
 *	  Open up the config file.
 *	  Open up the profile file.
 */

ccd_init_files()
  {
	int	i;
	char	temp[50];

	/*
	 *	Translate logical names.
	 */

	if(0 == trnlog(trntable,"CCDSCANDIR",scan_dir))                      
	  {
	    fprintf(stderr,
	      "Please set the logical name or environment variable CCDSCANDIR\n");
	    fprintf(stderr,
	      "Then re-execute ccd_dc.\n");
	    cleanexit(BAD_STATUS);
	  }
	i = strlen(scan_dir);
	if(i > 0)
	  {
	    if(scan_dir[i-1] != '/' && scan_dir[i-1] != '\\')
	      {
		scan_dir[i] = '/'; scan_dir[i+1] = '\0';
	      }
	  }
	if(0 == trnlog(trntable,"CCDSCANDIR_EXPORT",scan_dir_export))                      
	  {
	    fprintf(stderr,
	      "Please set the logical name or environment variable CCDSCANDIR_EXPORT\n");
	    fprintf(stderr,
	      "Then re-execute ccd_dc.\n");
	    cleanexit(BAD_STATUS);
	  }
	i = strlen(scan_dir_export);
	if(i > 0)
	  {
	    if(scan_dir_export[i-1] != '/' && scan_dir[i-1] != '\\')
	      {
		scan_dir_export[i] = '/'; scan_dir_export[i+1] = '\0';
	      }
	  }

	if(0 == trnlog(trntable,CCD_DC_LOCAL_LOG,lfname))
	  {
	    fprintf(stderr,
	      "Please set the logical name or environment variable.\n");
	    fprintf(stderr,
	      "Then re-execute ccd_dc.\n");
	    cleanexit(BAD_STATUS);
	  }
        if(0 == trnlog(trntable,CCD_DC_CONFIG,confname))
          {
            fprintf(stderr,
              "Please set the logical name or environment variable.\n");
            fprintf(stderr,
              "Then re-execute ccd_dc.\n");
            cleanexit(BAD_STATUS);
          }
        if(0 != trnlog(trntable,CCD_N_CTRL,temp))
	    sscanf(temp,"%d",&n_ctrl);
	  else
	    n_ctrl = 1;

	/*
	 *	Open up log file only.
	 */
	
	if(NULL == (fplog = fopen(lfname,OPENA_REC)))
	  {
	    fprintf(stderr,"Cannot open %s as ccd log file\n",lfname);
	    cleanexit(BAD_STATUS);
	  }

  }

/*
 *	ccd_init_config
 *
 *	This routine allows the user to override default
 *	values for the scanner specific variables from
 *	a configuration file.
 *
 *	The format of the configuration file is:
 *
 *	keyword		value
 *
 *	The user may specify as little of the formal
 *	keyword as is necessary for unambiguous
 *	determination of the keyword.
 */

/*
 *	keywords:
 */

struct config_key {
			char	*key_name;
			char	*key_abbr;
			int	key_value;
		  };

enum {
	KEY_ERASE_TIME	=      0,
	KEY_SCAN_TIME		,
	KEY_DC_ERASE_TIME	,
	KEY_TOTAL_VALID_BLOCKS	,
	KEY_TOTAL_PIXELS_X	,
	KEY_TOTAL_PIXELS_Y	,
	KEY_MULTIPLIER		,
	KEY_PHI_STEPS_DEG	,
	KEY_DIST_STEPS_MM	,
	KEY_PHI_TOP_SPEED	,
	KEY_DIST_TOP_SPEED	,
	KEY_DIST_MAX_POINT	,
	KEY_DIST_MIN_POINT	,
	KEY_UNITS_PER_SEC	,
	KEY_UNITS_PER_DOSE	,
	KEY_WAVELENGTH		,
	KEY_IS_DIST		,
	KEY_IS_PHI		,
	KEY_FLAGS		,
	KEY_NC_POINTER		,
	KEY_NC_INDEX		,
	KEY_NC_X		,
	KEY_NC_Y		,
	KEY_NC_REC		,
	KEY_NC_POFF		,
	KEY_SCSI_ID		,
	KEY_SCSI_CONTROLLER	,
	KEY_SPIRAL_CHECK	,
	KEY_LIFT_STEPS_MM       ,
	KEY_LIFT_TOP_SPEED      ,
	KEY_LIFT_MAX_POINT      ,
	KEY_LIFT_MIN_POINT      ,
	KEY_IS_LIFT             ,
	KEY_READ_FAST		,
	KEY_READ_SLOW		,
	KEY_READ_OVERHEAD	,
	KEY_BIN_FACTOR		,
	KEY_USEKAPPA		,
	KEY_USEOMEGA		,
	KEY_DEZINGER		,
	KEY_USE2THETA		,
	KEY_PCSHUTTER		,
	KEY_DARKINTERVAL	,
	KEY_PIXEL_SIZE		,
	KEY_DK_BEFORE_RUN	,
	KEY_REPEAT_DARK		,
	KEY_OUTFILE_TYPE	,
	KEY_DETECTOR_SN		,
	KEY_NO_TRANSFORM	,
	KEY_OUTPUT_RAWS		,
	KEY_J5_TRIGGER		,
	KEY_TIMECHECK		,
	KEY_CONSTRAIN_OMEGA	,
	KEY_CONSTRAIN_PHI	,
	KEY_CONSTRAIN_KAPPA	,
	KEY_STRIP_AVE		,
	KEY_BCHK_TIME		,
	KEY_BCHK_DELTAPHI	,
	KEY_USEWAVELENGTH	,
	KEY_APPROACH_START	,
	KEY_CHIP_SIZE_X		,
	KEY_CHIP_SIZE_Y		,
	KEY_KAPPA_CONST		,
	KEY_DAEMON_EXIT		,
	KEY_USEZERO_ANGLES	,
	KEY_USEGON_MANUAL	,
	KEY_MADRUN_NAMING	,
	KEY_RETRYSHORT		,
	KEY_CCD_MODULAR		,
	KEY_PF_MOD		,
	KEY_MIN_VELOCITY	,
	KEY_MAX_VELOCITY	,
	KEY_ALLOW_STILLS	,
	KEY_PITCH_TUNE_DELTA	,
	KEY_BM8_MOD		,
	KEY_USESTOP_IMMEDIATE	,
	KEY_BL_IS_SERVER	,
	KEY_BEAM_SENSE		,
	KEY_T2K_DETECTOR	,
	KEY_ADSC_SLIT		,
	KEY_ADSC_4SLIT
  };

struct config_key config_list[] = 
  {
		"erasetime","erasetime",KEY_ERASE_TIME, 
		"scantime","scantime",KEY_SCAN_TIME,
		"dcerasetime","dcerasetime",KEY_DC_ERASE_TIME,
		"blocks","blocks",KEY_TOTAL_VALID_BLOCKS,
		"pixelsx","pixelsx", KEY_TOTAL_PIXELS_X,
		"pixelsy","pixelsy",KEY_TOTAL_PIXELS_Y,
		"multiplier","multiplier",KEY_MULTIPLIER,
		"phisteps","phisteps",KEY_PHI_STEPS_DEG,
		"diststeps","diststeps",KEY_DIST_STEPS_MM,
		"phitop","phit",KEY_PHI_TOP_SPEED,
		"disttop","distt",KEY_DIST_TOP_SPEED,
		"distmax","distmax",KEY_DIST_MAX_POINT,
		"distmin","distmin",KEY_DIST_MIN_POINT,
		"unitsec","units",KEY_UNITS_PER_SEC,
		"unitdose","unitd",KEY_UNITS_PER_DOSE,
		"wavelength","wavelength",KEY_WAVELENGTH,
		"usedistance","usedistance",KEY_IS_DIST,
		"usephi","usephi",KEY_IS_PHI,
		"flags","flags",KEY_FLAGS,
		"nc_pointer","nc_poi",KEY_NC_POINTER,
		"nc_index","nc_i",KEY_NC_INDEX,
		"nc_x","nc_x",KEY_NC_X,
		"nc_y","nc_y",KEY_NC_Y,
		"nc_rec","nc_r",KEY_NC_REC,
		"nc_poff","nc_pof",KEY_NC_POFF,
		"scsi_id","scsi_id",KEY_SCSI_ID,
		"scsi_controller","scsi_controller",KEY_SCSI_CONTROLLER,
		"spiral_check","spiral_check",KEY_SPIRAL_CHECK,
		"liftsteps","liftsteps",KEY_LIFT_STEPS_MM,
		"lifttop","lifttop",KEY_LIFT_TOP_SPEED,
		"liftmax","liftmax",KEY_LIFT_MAX_POINT,
		"liftmin","liftmin",KEY_LIFT_MIN_POINT,
		"uselift","uselift",KEY_IS_LIFT,
		"read_fast","read_fast",KEY_READ_FAST,
		"read_slow","read_slow",KEY_READ_SLOW,
		"read_overhead","read_overhead",KEY_READ_OVERHEAD,
		"bin_factor","bin_factor",KEY_BIN_FACTOR,
		"usekappa","usekappa",KEY_USEKAPPA,
		"useomega","useomega",KEY_USEOMEGA,
		"dezinger","dezinger",KEY_DEZINGER,
		"use2theta","use2theta",KEY_USE2THETA,
		"pcshutter","pcshutter",KEY_PCSHUTTER,
		"darkinterval","darkinterval",KEY_DARKINTERVAL,
		"pixel_size","pixel_size",KEY_PIXEL_SIZE,
		"dk_before_run","dk_before_run",KEY_DK_BEFORE_RUN,
		"repeat_dark","repeat_dark",KEY_REPEAT_DARK,
		"outfile_type","outfile_type",KEY_OUTFILE_TYPE,
		"detector_sn","detector_sn",KEY_DETECTOR_SN,
		"no_transform","no_transform",KEY_NO_TRANSFORM,
		"output_raws","output_raws",KEY_OUTPUT_RAWS,
		"j5_trigger","j5_trigger",KEY_J5_TRIGGER,
		"timecheck","timecheck",KEY_TIMECHECK,
		"constrain_omega","constrain_omega",KEY_CONSTRAIN_OMEGA,
		"constrain_phi","constrain_phi",KEY_CONSTRAIN_PHI,
		"constrain_kappa","constrain_kappa",KEY_CONSTRAIN_KAPPA,
		"strip_ave","strip_ave",KEY_STRIP_AVE,
		"beamcheck_time","beamcheck_time",KEY_BCHK_TIME,
		"beamcheck_deltaphi","beamcheck_deltaphi",KEY_BCHK_DELTAPHI,
		"usewavelength","usewavelength",KEY_USEWAVELENGTH,
		"approach_start","approach_start",KEY_APPROACH_START,
		"chip_size_x","chip_size_x",KEY_CHIP_SIZE_Y,
		"chip_size_y","chip_size_y",KEY_CHIP_SIZE_X,
		"kappa_const","kappa_const",KEY_KAPPA_CONST,
		"daemon_exit","daemon_exit",KEY_DAEMON_EXIT,
		"usezero_angles","usezero_angles",KEY_USEZERO_ANGLES,
		"usegon_manual","usegon_manual",KEY_USEGON_MANUAL,
		"madrun_naming","madrun_naming",KEY_MADRUN_NAMING,
		"retryshort","retryshort",KEY_RETRYSHORT,
		"modular", "modular", KEY_CCD_MODULAR,
		"pf_mod", "pf_mod", KEY_PF_MOD,
		"min_velocity", "min_velocity", KEY_MIN_VELOCITY,
		"max_velocity", "max_velocity", KEY_MAX_VELOCITY,
		"allow_stills", "allow_stills", KEY_ALLOW_STILLS,
		"pitch_tune_delta","pitch_tune_delta", KEY_PITCH_TUNE_DELTA,
		"bm8_mod","bm8_mod", KEY_BM8_MOD,
		"usestop_immediate", "usestop_immediate", KEY_USESTOP_IMMEDIATE,
		"bl_is_server", "bl_is_server", KEY_BL_IS_SERVER,
		"beam_sense", "beam_sense", KEY_BEAM_SENSE,
		"t2k_detector", "t2k_detector", KEY_T2K_DETECTOR,
		"adsc_slit", "adsc_slit", KEY_ADSC_SLIT,
		"adsc_4slit", "adsc_4slit", KEY_ADSC_4SLIT,
		NULL,NULL,0,
  };

ccd_init_config(fpmsg)
FILE	*fpmsg;
  {
	char	tname[256];
	char	line[132];
	char	string1[132],string2[132];
	int	i,j;

	strcpy(tname,confname);

	if(NULL == (fpconfig = fopen(tname,"r")))
	  {
	    fprintf(stderr,"ccd_dc: config: cannot open config file %s\n",tname);
	    fprintf(fplog,"ccd_dc: config: cannot open config file %s\n",tname);
	    fflush(fplog);
	    return;
	  }

	while(NULL != fgets(line,sizeof line,fpconfig))
	  {
	    if(line[0] == '!' || line[0] == '#')
	      {
		fprintf(fpmsg,"%s",line);
		fprintf(fplog,"%s",line);
		continue;
	      }
	    i = sscanf(line,"%s%s",string1,string2);
	    if(i != 2)
	      {
		fprintf(stderr,"ccd_dc: config: not enough params (need 2):\n");
		fprintf(stderr,"%s",line);
		fprintf(stderr,"ccd_dc: config: ignoring that line.\n");
		continue;
	      }
	    j = 0;
	    for(i = 0; config_list[i].key_name != NULL; i++)
	      if(0 == strncmp(config_list[i].key_abbr,string1,strlen(config_list[i].key_abbr)))
		{
			j = 1;
			break;
		}
	    if(j == 0)
	      {
		fprintf(stderr,"ccd_dc: config: unrecognized keyword:\n");
		fprintf(stderr,"%s",line);
		fprintf(stderr,"ccd_dc: config: ignoring that line.\n");
		continue;
	      }
	    switch(config_list[i].key_value)
	      {
		case	KEY_ERASE_TIME:
			sscanf(string2,"%f",&specific_erase_time);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_erase_time);
			fprintf(fplog,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_erase_time);
			break;
		case	KEY_SCAN_TIME:
			sscanf(string2,"%f",&specific_scan_time);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_scan_time);
			fprintf(fplog,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_scan_time);
			break;
		case	KEY_DC_ERASE_TIME:
			sscanf(string2,"%f",&specific_dc_erase_time);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_dc_erase_time);
			fprintf(fplog,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_dc_erase_time);
			break;
		case	KEY_TOTAL_VALID_BLOCKS:
			sscanf(string2,"%d",&specific_total_valid_blocks);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_valid_blocks);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_valid_blocks);
			break;
		case	KEY_TOTAL_PIXELS_X:
			sscanf(string2,"%d",&specific_total_pixels_x);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_pixels_x);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_pixels_x);
			break;
		case	KEY_TOTAL_PIXELS_Y:
			sscanf(string2,"%d",&specific_total_pixels_y);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_pixels_y);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_total_pixels_y);
			break;
		case	KEY_MULTIPLIER:
			sscanf(string2,"%f",&specific_multiplier);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_multiplier);
			fprintf(fplog,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_multiplier);
			break;
		case	KEY_PHI_STEPS_DEG:
			sscanf(string2,"%d",&specific_phi_steps_deg);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_phi_steps_deg);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_phi_steps_deg);
			break;
		case	KEY_DIST_STEPS_MM:
			sscanf(string2,"%d",&specific_dist_steps_mm);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_steps_mm);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_steps_mm);
			break;
		case	KEY_PHI_TOP_SPEED:
			sscanf(string2,"%d",&specific_phi_top_speed);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_phi_top_speed);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_phi_top_speed);
			break;
		case	KEY_DIST_TOP_SPEED:
			sscanf(string2,"%d",&specific_dist_top_speed);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_top_speed);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_top_speed);
			break;
		case	KEY_DIST_MAX_POINT:
			sscanf(string2,"%d",&specific_dist_max_point);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_max_point);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_max_point);
			break;
		case	KEY_DIST_MIN_POINT:
			sscanf(string2,"%d",&specific_dist_min_point);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_min_point);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dist_min_point);
			break;
		case	KEY_UNITS_PER_SEC:
			sscanf(string2,"%d",&specific_units_per_sec);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_units_per_sec);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_units_per_sec);
			break;
		case	KEY_UNITS_PER_DOSE:
			sscanf(string2,"%d",&specific_units_per_dose);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_units_per_dose);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_units_per_dose);
			break;
		case	KEY_WAVELENGTH:
			sscanf(string2,"%f",&specific_wavelength);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_wavelength);
			fprintf(fplog,"ccd_dc: config: %s set to %10.2f\n",
				config_list[i].key_name,specific_wavelength);
			break;
		case	KEY_IS_DIST:
			sscanf(string2,"%d",&specific_is_distance);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_distance);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_distance);
			break;
		case	KEY_IS_PHI:
			sscanf(string2,"%d",&specific_is_phi);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_phi);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_phi);
			break;
		case	KEY_FLAGS:
			sscanf(string2,"%d",&specific_flags);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_flags);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_flags);
			break;
		case	KEY_NC_POINTER:
			sscanf(string2,"%d",&specific_nc_pointer);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_pointer);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_pointer);
			break;
		case	KEY_NC_INDEX:
			sscanf(string2,"%d",&specific_nc_index);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_index);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_index);
			break;
		case	KEY_NC_X:
			sscanf(string2,"%d",&specific_nc_x);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_x);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_x);
			break;
		case	KEY_NC_Y:
			sscanf(string2,"%d",&specific_nc_y);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_y);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_y);
			break;
		case	KEY_NC_REC:
			sscanf(string2,"%d",&specific_nc_rec);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_rec);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_rec);
			break;
		case	KEY_NC_POFF:
			sscanf(string2,"%d",&specific_nc_poff);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_poff);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_nc_poff);
			break;
		case	KEY_SCSI_ID:
			sscanf(string2,"%d",&specific_scsi_id);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_scsi_id);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_scsi_id);
			break;
		case	KEY_SCSI_CONTROLLER:
			sscanf(string2,"%d",&specific_scsi_controller);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_scsi_controller);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_scsi_controller);
			break;
		case	KEY_SPIRAL_CHECK:
			sscanf(string2,"%d",&specific_spiral_check);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",config_list[i].key_name,specific_spiral_check);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",config_list[i].key_name,specific_spiral_check);
			break;
		case	KEY_LIFT_STEPS_MM:
			sscanf(string2,"%d",&specific_lift_steps_mm);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_steps_mm);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_steps_mm);
			break;
		case	KEY_LIFT_TOP_SPEED:
			sscanf(string2,"%d",&specific_lift_top_speed);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_top_speed);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_top_speed);
			break;
		case	KEY_LIFT_MAX_POINT:
			sscanf(string2,"%d",&specific_lift_max_point);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_max_point);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_max_point);
			break;
		case	KEY_LIFT_MIN_POINT:
			sscanf(string2,"%d",&specific_lift_min_point);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_min_point);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_lift_min_point);
			break;
		case	KEY_IS_LIFT:
			sscanf(string2,"%d",&specific_is_lift);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_lift);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_lift);
			break;
		case	KEY_READ_FAST:
			sscanf(string2,"%f",&specific_read_fast);
			fprintf(fpmsg,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_fast);
			fprintf(fplog,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_fast);
			break;
		case	KEY_READ_SLOW:
			sscanf(string2,"%f",&specific_read_slow);
			fprintf(fpmsg,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_slow);
			fprintf(fplog,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_slow);
			break;
		case	KEY_READ_OVERHEAD:
			sscanf(string2,"%f",&specific_read_overhead);
			fprintf(fpmsg,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_overhead);
			fprintf(fplog,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_read_overhead);
			break;
		case	KEY_BIN_FACTOR:
			sscanf(string2,"%f",&specific_bin_factor);
			fprintf(fpmsg,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_bin_factor);
			fprintf(fplog,"ccd_dc: config: %s set to %5.2f\n",
				config_list[i].key_name,specific_bin_factor);
			break;
		case	KEY_USEKAPPA:
			sscanf(string2,"%d",&specific_is_kappa);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_kappa);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_kappa);
			break;
		case	KEY_USEOMEGA:
			sscanf(string2,"%d",&specific_is_omega);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_omega);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_omega);
			break;
		case	KEY_DEZINGER:
			sscanf(string2,"%d",&specific_def_dezinger);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_def_dezinger);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_def_dezinger);
			break;
		case	KEY_USE2THETA:
			sscanf(string2,"%d",&specific_is_2theta);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_2theta);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_2theta);
			fprintf(stderr,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_2theta);
			break;
		case	KEY_USEWAVELENGTH:
			sscanf(string2,"%d",&specific_is_wavelength);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_wavelength);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_wavelength);
			fprintf(stderr,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_is_wavelength);
			break;
		case	KEY_APPROACH_START:
			sscanf(string2,"%f",&specific_approach_start);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_approach_start);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_approach_start);
			fprintf(stderr,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_approach_start);
			break;
                case    KEY_CHIP_SIZE_X:
                        sscanf(string2,"%d",&specific_chip_size_x);
                        fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_x);
                        fprintf(fplog,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_x);
                        fprintf(stderr,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_x);
                        break;
                case    KEY_CHIP_SIZE_Y:
                        sscanf(string2,"%d",&specific_chip_size_y);
                        fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_y);
                        fprintf(fplog,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_y);
                        fprintf(stderr,"ccd_dc: config: %s set to %d\n",
                                config_list[i].key_name,specific_chip_size_y);
                        break;
		case	KEY_PCSHUTTER:
			sscanf(string2,"%d",&specific_pcshutter);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pcshutter);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pcshutter);
			break;
		case	KEY_DARKINTERVAL:
			sscanf(string2,"%d",&specific_dark_interval);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dark_interval);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dark_interval);
			break;
		case	KEY_PIXEL_SIZE:
			sscanf(string2,"%f",&specific_pixel_size);
			fprintf(fpmsg,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_pixel_size);
			fprintf(fplog,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_pixel_size);
			break;
		case	KEY_BCHK_TIME:
			sscanf(string2,"%f",&specific_bchk_time);
			fprintf(fpmsg,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_bchk_time);
			fprintf(fplog,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_bchk_time);
			break;
		case	KEY_BCHK_DELTAPHI:
			sscanf(string2,"%f",&specific_bchk_deltaphi);
			fprintf(fpmsg,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_bchk_deltaphi);
			fprintf(fplog,"ccd_dc: config: %s set to %f\n",
				config_list[i].key_name,specific_bchk_deltaphi);
			break;
		case	KEY_DK_BEFORE_RUN:
			sscanf(string2,"%d",&specific_dk_before_run);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dk_before_run);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_dk_before_run);
			break;
		case	KEY_REPEAT_DARK:
			sscanf(string2,"%d",&specific_repeat_dark);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_repeat_dark);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_repeat_dark);
			break;
		case	KEY_DETECTOR_SN:
			sscanf(string2,"%d",&specific_detector_sn);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_detector_sn);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_detector_sn);
			break;
		case	KEY_NO_TRANSFORM:
			sscanf(string2,"%d",&specific_no_transform);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_no_transform);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_no_transform);
			break;
		case	KEY_OUTPUT_RAWS:
			sscanf(string2,"%d",&specific_output_raws);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_output_raws);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_output_raws);
			break;
		case	KEY_J5_TRIGGER:
			sscanf(string2,"%d",&specific_j5_trigger);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_j5_trigger);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_j5_trigger);
			break;
		case	KEY_STRIP_AVE:
			sscanf(string2,"%d",&specific_strip_ave);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_strip_ave);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_strip_ave);
			break;
		case	KEY_TIMECHECK:
			sscanf(string2,"%d",&specific_timecheck);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_timecheck);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_timecheck);
			break;
		case	KEY_CONSTRAIN_OMEGA:
			sscanf(string2,"%d",&specific_constrain_omega);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_omega);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_omega);
			break;
		case	KEY_CONSTRAIN_PHI:
			sscanf(string2,"%d",&specific_constrain_phi);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_phi);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_phi);
			break;
		case	KEY_CONSTRAIN_KAPPA:
			sscanf(string2,"%d",&specific_constrain_kappa);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_kappa);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_constrain_kappa);
			break;

			/*
			 *	These three are for the GUI only.  No effect on ccd_dc.
			 */

		case	KEY_DAEMON_EXIT:
			break;
		case	KEY_USEZERO_ANGLES:
			break;
		case	KEY_USEGON_MANUAL:
			break;

		case	KEY_KAPPA_CONST:
			sscanf(string2,"%f",&specific_kappa_const);
			fprintf(fpmsg,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_kappa_const);
			fprintf(fplog,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_kappa_const);
			break;
		case	KEY_MADRUN_NAMING:
			sscanf(string2,"%f",&specific_madrun_naming);
			fprintf(fpmsg,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_madrun_naming);
			fprintf(fplog,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_madrun_naming);
			break;
		case	KEY_RETRYSHORT:
			sscanf(string2,"%f",&specific_retryshort);
			fprintf(fpmsg,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_retryshort);
			fprintf(fplog,"ccd_dc: config: %s set to %7.3f\n",
				config_list[i].key_name,specific_retryshort);
			break;
		case	KEY_CCD_MODULAR:
			sscanf(string2,"%d",&specific_ccd_modular);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_ccd_modular);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_ccd_modular);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_ccd_modular);
			break;
		case	KEY_PF_MOD:
			sscanf(string2,"%d",&specific_pf_mod);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pf_mod);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pf_mod);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pf_mod);
			break;
		case	KEY_BL_IS_SERVER:
			sscanf(string2,"%d",&specific_bl_is_server);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bl_is_server);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bl_is_server);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bl_is_server);
			break;
		case	KEY_ALLOW_STILLS:
			sscanf(string2,"%d",&specific_allow_stills);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_allow_stills);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_allow_stills);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_allow_stills);
			break;
		case	KEY_MIN_VELOCITY:
			sscanf(string2,"%f",&specific_min_velocity);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_min_velocity);
			fprintf(fplog,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_min_velocity);
			fprintf(stdout,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_min_velocity);
			break;
		case	KEY_MAX_VELOCITY:
			sscanf(string2,"%f",&specific_max_velocity);
			fprintf(fpmsg,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_max_velocity);
			fprintf(fplog,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_max_velocity);
			fprintf(stdout,"ccd_dc: config: %s set to %10.5f\n",
				config_list[i].key_name,specific_max_velocity);
			break;
		case	KEY_PITCH_TUNE_DELTA:
			sscanf(string2,"%d",&specific_pitch_tune_delta);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pitch_tune_delta);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pitch_tune_delta);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_pitch_tune_delta);
			break;
		case	KEY_BM8_MOD:
			sscanf(string2,"%d",&specific_bm8_mod);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bm8_mod);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bm8_mod);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_bm8_mod);
			break;
		case	KEY_BEAM_SENSE:
			sscanf(string2,"%d",&specific_beam_sense);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_beam_sense);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_beam_sense);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_beam_sense);
			break;
		case	KEY_T2K_DETECTOR:
			sscanf(string2,"%d",&specific_t2k_detector);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_t2k_detector);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_t2k_detector);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_t2k_detector);
			break;
		case	KEY_ADSC_SLIT:
			sscanf(string2,"%d",&specific_adsc_slit);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_slit);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_slit);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_slit);
			break;
		case	KEY_ADSC_4SLIT:
			sscanf(string2,"%d",&specific_adsc_4slit);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_4slit);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_4slit);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_adsc_4slit);
			break;
		case	KEY_USESTOP_IMMEDIATE:
			sscanf(string2,"%d",&specific_usestop_immediate);
			fprintf(fpmsg,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_usestop_immediate);
			fprintf(fplog,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_usestop_immediate);
			fprintf(stdout,"ccd_dc: config: %s set to %d\n",
				config_list[i].key_name,specific_usestop_immediate);
			break;
		case	KEY_OUTFILE_TYPE:
			if(0 == strcmp(string2,"signed_long") ||
			   0 == strcmp(string2,"int"))
				outfile_type = 1;
			if(outfile_type == 0)
			  {
			    fprintf(fpmsg,"ccd_dc: config: %s set to unsigned_short\n",
				config_list[i].key_name);
			    fprintf(fplog,"ccd_dc: config: %s set to unsigned_short\n",
				config_list[i].key_name);
			  }
			 else
			  {
			    fprintf(fpmsg,"ccd_dc: config: %s set to signed_long\n",
				config_list[i].key_name);
			    fprintf(fplog,"ccd_dc: config: %s set to signed_long\n",
				config_list[i].key_name);
			  }
			break;
	      }
	  }
	fflush(fplog);
	fclose(fpconfig);
  }

static	char	timeholder[120];

char	*ztime()
  {
	long	clock;
	char	*cptr;

	time(&clock);
	cptr = (char *) ctime(&clock);
	strcpy(timeholder,cptr);
	timeholder[strlen(timeholder) - 1] = '\0';
	return(timeholder);
  }
