#include	"ccd_dc_ext.h"

/*
 *----------------------------------------------
 *
 *	Module to sequence the control of the CCD
 *	hardware modules.
 *
 *	Two processes are used to run hardware:
 *		ccd_det		controls the detector.
 *		ccd_bl		controls the beamline.
 *
 *	In this implimentation, it is assumed that
 *	ccd_bl controls all hardware EXCEPT the detector.
 *
 *----------------------------------------------
 */

/*
 *	Routines to accurately calculate the
 *	time commands take to execute.  This
 *	is for the completeness statistics, for
 *	the most part, and does not actually
 *	alter functionality.
 */

static	time_t	tick_clock_val;

tick_set()
  {
	time(&tick_clock_val);
  }

int	tick_diff()
  {
	time_t	current_clock_val;
	int	diff_in_secs;

	time(&current_clock_val);
	diff_in_secs = ((int) current_clock_val) - ((int) tick_clock_val);
	tick_clock_val = current_clock_val;
	return(diff_in_secs);
  }

/*
 *	Operation codes for the initialize command.
 *
 *	These are software state variables to keep
 *	track of which initialize phase the program
 *	is doing.
 */

#define	INITOP_ABORT	0
#define	INITOP_RESET	1
#define	INITOP_SHUTTER	2
#define	INITOP_LOADTAB	3
#define	INITOP_DISTANCE	4

int	init_op;

/*
 *	Operation codes for the data collection command.
 *
 *	These are purely software states so the program
 *	can keep track of what part of data collection it
 *	is doing.
 */

#define	DCOP_COLLECT	0
#define	DCOP_SCAN	1
#define	DCOP_ERASE	2
#define	DCOP_RECRESET	3
#define	DCOP_RECERASE	4
#define	DCOP_RECSHUTTER	5

int	active_trans = 0;
int	scan_readout_in_progress = 0;	/* used to make sure we don't step on scan readout in progress */
int	scan_abort_readout = 0;		/* 1 to cause scan_readout to abort and close its files */

/*
 *	These are used to check to see if distance
 *	or phi actually move, and are used in conjunction
 *	with active_trans above.
 */

double	phi_value_saved;	/* value before a move */
double	omega_value_saved;	/* value before a move */
int	dist_value_saved;	/* value before a move */
int     lift_value_saved;
int	phi_no_motion;		/* 1 if no motion was initiated */
int     dist_no_motion;         /* 1 if no motion was initiated */
int     lift_no_motion;         /* 1 if no motion was initiated */

int	shutter_retry;		/* used to keep track for shutter errors */
int	collect_counter;	/* used to make sure we check c_error for collect */
int	reset_counter;		/* used to make sure we wait long enough */
int	init_dist_ctr;		/* used to make sure we wait long enough */
int	double_exp_flag;	/* 1 to do two exposures per actual image */
int	double_exp_ctr;		/* used to toggle double exposures */
int	doing_dark_current;	/* 1 if doing dark current exposures */
int	doing_second_dk;	/* 1 if doing second dark current */
int	doing_snapshot;		/* 1 if doing a shapshot */

int	goniostat_type = 0;	/* 0 for phi only, 1 for Eulerian, 2 for Kappa */
int	wedge_count;		/* used for counting down in a wedge */
int	wedge_side;		/* 0 if doing the primary run, 1 if doing the bijvoet */
int	wedge_run;		/* which_run is being collected */
int	wedge_imno;
int	wedge_size;
float	wedge_omega;
float	wedge_phi;
float	wedge_kappa;
char	wedge_prefix[256];	/* temp holding tank for prefix */

float	current_darkcurrent_time = 0;	/* keeps track of the length of the current dark current image */

static	int	gl_blret;
static	int	exposure_short_by;	/* number of seconds, approx., by which an exposure is "short" */
static	int	short_exposure_wait;	/* 1 when we are stalling for the "rest" of a short exposure */

static	int	cf_ret = CF_BL_READY;

static	int	no_image_taken = 1;

parse_file_name(s,t,inp)
char	*s,*t;
int	*inp;
  {
	int	i,j,k;
	
	j = strlen(s);
	for(i = j - 1; i > 0 && s[i] != '_'; i--);
	if(i == 0)
	  {
	    *inp = 0;
	    strcpy(t,s);
	    return;
	  }
	for(j = i + 1; s[j] != '\0'; j++)
	  if(s[j] < '0' || s[j] > '9')
	    {
	    	*inp = 0;
		strcpy(t,s);
		return;
	    }
	*inp = atoi(&s[i + 1]);
	for(j = 0; j < i; j++)
	  t[j] = s[j];
	t[i] = '\0';
  }

/*
 *	make_header_smv  -  make a suitable SMV header.
 */

#define	SHDSIZE		20480

char	made_header[SHDSIZE];

make_header_smv()
  {
	char	buf[32];
	int	i;
	char	*cptr;
	char	*ztime();

	clrhd(made_header);

	/*
	 *	standard items.
	 */
	
	puthd("DIM","2",made_header);

#if defined(alpha) || defined(linux)
	puthd("BYTE_ORDER","little_endian",made_header);
#else
	puthd("BYTE_ORDER","big_endian",made_header);
#endif /* alpha */

	puthd("TYPE","unsigned_short",made_header);

	if(stat_bin == 1)
	  {
            sprintf(buf,"%d",chip_size_x);
            puthd("SIZE1",buf,made_header);
            sprintf(buf,"%d",chip_size_y);
            puthd("SIZE2",buf,made_header);
	    sprintf(buf,"%7.5f",pixel_size);
	    puthd("PIXEL_SIZE",buf,made_header);
	    puthd("BIN","none",made_header);
	  }
	 else
	  {
            sprintf(buf,"%d",chip_size_x / stat_bin);
            puthd("SIZE1",buf,made_header);
            sprintf(buf,"%d",chip_size_y / stat_bin);
            puthd("SIZE2",buf,made_header);
	    sprintf(buf,"%7.5f",pixel_size * stat_bin);
	    puthd("PIXEL_SIZE",buf,made_header);
	    puthd("BIN","2x2",made_header);
	  }
	if(stat_adc == 0)
	    puthd("ADC","slow",made_header);
	  else
	    puthd("ADC","fast",made_header);
	if(0)
	{
	for(i = 0; i < n_ctrl; i++)
	  {
		sprintf(buf,"CCD_OFFSET%d",i);
		puthd(buf,"xx",made_header);
	  }
	}
	/*
	 *	adsc items.
	 */
	if(detector_sn > 0)
	  {
		sprintf(buf,"%d",detector_sn);
		puthd("DETECTOR_SN",buf,made_header);
	  }
	cptr = ztime();
	puthd("DATE",cptr,made_header);
	sprintf(buf,"%.2f",stat_time);
	puthd("TIME",buf,made_header);
	sprintf(buf,"%.3f",stat_dist);
	puthd("DISTANCE",buf,made_header);
	sprintf(buf,"%.3f",stat_osc_width);
	puthd("OSC_RANGE",buf,made_header);
	if(stat_axis == 1)
	  {
	    if(is_kappa)
	      {
		sprintf(buf,"%.3f",phi_value_saved);
		puthd("PHI",buf,made_header);
		puthd("OSC_START",buf,made_header);
		sprintf(buf,"%.3f",stat_omega);
		puthd("OMEGA",buf,made_header);
	        sprintf(buf,"%.3f",stat_kappa);
	        puthd("KAPPA",buf,made_header);
	      }
	     else
	      {
		sprintf(buf,"%.3f",phi_value_saved);
		puthd("PHI",buf,made_header);
		puthd("OSC_START",buf,made_header);
	      }
	  }
	 else
	  {
	    if(is_kappa)
	      {
		sprintf(buf,"%.3f",omega_value_saved);
		puthd("OMEGA",buf,made_header);
		puthd("OSC_START",buf,made_header);
		sprintf(buf,"%.3f",stat_phi);
		puthd("PHI",buf,made_header);
	        sprintf(buf,"%.3f",stat_kappa);
	        puthd("KAPPA",buf,made_header);
	      }
	     else
	      {
		sprintf(buf,"%.3f",omega_value_saved);
		puthd("OMEGA",buf,made_header);
		puthd("OSC_START",buf,made_header);
	      }
	  }
	if(is_2theta)
	  {
	    sprintf(buf,"%.3f",stat_2theta);
	    puthd("TWOTHETA",buf,made_header);
	  }
	if(stat_axis == 1)
	    puthd("AXIS","phi",made_header);
	  else
	    puthd("AXIS","omega",made_header);
	sprintf(buf,"%.5f",stat_wavelength);
	puthd("WAVELENGTH",buf,made_header);
	sprintf(buf,"%.3f",beam_xcen);
	puthd("BEAM_CENTER_X",buf,made_header);
	sprintf(buf,"%.3f",beam_ycen);
	puthd("BEAM_CENTER_Y",buf,made_header);
        if(doing_dark_current)
                sprintf(buf,"%d",doing_second_dk * 2 + 2 - double_exp_ctr);
          else
            {
                if(double_exp_flag)
                        sprintf(buf,"%d",4 + 2 - double_exp_ctr);
                    else
                        strcpy(buf,"5");
            }
	if(stat_attenuator != -1)
	{
		get_attenuator_name((int) (stat_attenuator + .01), buf);
		puthd("ATTENUATOR", buf, made_header);
	}
	padhd(made_header,512);
  }

/*      Routine to set the beamline to give a lock on the beamline process before starting 
*       data collection.  Right now, all it does it close the shutter, but when fully implemented
*       it should do the following:
*       1. Request a lock from the beamline process.  Upon getting a lock the "collection in progress"
*          signal line will be active.
*       2. If a lock cannot be obtained at this time (because another process is using the beamline),
*          issue a message to this effect and poll until the lock can be obtained.
*       3. After getting a lock, close the shutter.
*
*       There is also an unlock command.  Use of locks is entirely voluntary and is not enforced.
*/

     void send_bl_lock()
{
  /* output_blcmd(fdblcmd,"cmd","get_lock"); */
}
     void send_bl_unlock()
{
  /*output_blcmd(fdblcmd,"cmd","release_lock"); */
}

/*
 *	Routine to send the start detector exposing command.  Main purpose
 *	here is to build up filenames.
 */

send_det_start()
  {
	char	tempbuf[1024],tbuf[1024],infobuf[512],morebuf[100];
	char	im_3dig[4];
	char	suffix[20];
	int	hsize;
        int  detret;
	double	tmp;
	int	kind;
	char message[80];
	
	make_header_smv();
	gethdl(&hsize, made_header);

	sprintf(tempbuf,"start\nheader_size %d\n",hsize);

	if(doing_dark_current)
		sprintf(suffix,"dkx_%d",doing_second_dk * 2 + 2 - double_exp_ctr);
	  else
	    {
		if(double_exp_flag)
			sprintf(suffix,"imx_%d",2 - double_exp_ctr);
		    else
			sprintf(suffix,"imx_0");
	    }
	util_3digit(im_3dig,stat_image_number);
	sprintf(infobuf,"info %s_%s.%s\n",stat_prefix,im_3dig,suffix);

        sprintf(morebuf,"row_xfer %d\ncol_xfer %d\n",chip_size_y / stat_bin, chip_size_x / stat_bin);
        strcat(tempbuf,morebuf);

	if(use_pc_shutter)
	  {
	    if(doing_dark_current == 0)
	      strcat(tempbuf,"pcshutter 1\n");
	     else
	      strcat(tempbuf,"pcshutter 0\n");
	  }
	if(use_j5_trigger)
	      strcat(tempbuf,"j5_trigger 1\n");
	if(use_timecheck)
	      strcat(tempbuf,"timecheck 1\n");

	strcat(tempbuf,infobuf);
	sprintf(tbuf,"adc %d\nrow_bin %d\ncol_bin %d\ntime %f\n",stat_adc,stat_bin,stat_bin,stat_time);
	strcat(tempbuf,tbuf);
	if(ccd_modular)
	{
		sprintf(tbuf,"save_raw %d\n",output_raws);
		strcat(tempbuf,tbuf);

		if(raw_ccd_image == 0)
			sprintf(tbuf,"transform_image 1\n");
		else
			sprintf(tbuf,"transform_image 0\n");
		strcat(tempbuf,tbuf);

		if(doing_dark_current)
		    kind = 2 * doing_second_dk + 2 - double_exp_ctr;
		else
		  {
		    if(double_exp_flag)
		        kind = 4 + 2 - double_exp_ctr;
		    else
		        kind = 5;
		  }
		sprintf(tbuf,"image_kind %d\n",kind);
		strcat(tempbuf,tbuf);
	}

	detret = output_detcmd(fddetcmd,tempbuf,made_header,hsize);
        if (detret == CCD_DET_NOTCONNECTED) {
          dc_stop = 1;
	  sprintf(message," send_det_start");
	  issue_signal("CCD_SIG_DET_DROPPED",message);
          set_alert_msg("ERROR: Detector control not connected.");
        }
  }

/*
 *	Routine to send the start beam line exposing command.  Main purpose
 *	here is to build up filenames.
 */

int	send_bl_start()
  {
	char	tempbuf[1024],tbuf[1024];
	char	im_3dig[4];
	char	suffix[20];
	double	tmp;
	int 	bline_ret;
	char	*ztime();
        time_t  t_start,t_end;
        int     i_start,i_end,time_total;
	char message[80];

	if(doing_dark_current)
		sprintf(suffix,"dkc(%d)",doing_second_dk * 2 + 2 - double_exp_ctr);
	  else
	    {
		if(double_exp_flag)
			sprintf(suffix,"img(%d)",2 - double_exp_ctr);
		    else
			sprintf(suffix,"img");
	    }

	if(stat_axis == 1)
		tmp = phi_value_saved;
	    else
		tmp = omega_value_saved;
	sprintf(tempbuf,"collect\ntime %10.2f\ndistance %10.2f\nosc_width %10.3f\nphi_start %10.3f\n",
		stat_time,stat_dist,stat_osc_width,tmp);
	sprintf(tbuf,"wavelength %7.5f\ndirectory %s\nimage_prefix %s\nimage_suffix %s\nimage_number %d\n",
		stat_wavelength,stat_dir,stat_prefix,suffix,stat_image_number);
	strcat(tempbuf,tbuf);

	/*
	 *	To simplify for (older) software which does not support multiple axes, if
	 *	the data collection is not omega, omit the axis specification.
	 */

	if(stat_axis == 0)
		strcat(tempbuf,"axis 0\n");

	if(doing_dark_current)
	  strcat(tempbuf,"mode darkcurrent_dc\n");
         else {
             if (stat_mode == 1) {
               sprintf (tbuf,"mode dose\nstep_size %f\ndose_per_step %f\n",
                stat_step_size, stat_dose_step);
               strcat(tempbuf,tbuf);
             }
             else {
             strcat(tempbuf,"mode beamline_dc\n");
             }
           }

	sprintf(tbuf,"adc %d\nbin %d\n",stat_adc,stat_bin);
	strcat(tempbuf,tbuf);
	util_3digit(im_3dig,stat_image_number);
	fprintf(stdout,"send_bl_start: STRT:          at %s: %s_%s.%s\n",ztime(),stat_prefix,im_3dig,suffix);

        time(&t_start);
	i_start = (int) t_start;

	bline_ret = output_blcmd(fdblcmd,"cmd",tempbuf);
        time(&t_end);
        i_end = (int) t_end;
        time_total = t_end - t_start;

	switch (bline_ret) {
	  case CCD_BL_FATAL:
		dc_stop = 1;
	  break;
	  case CCD_BL_NOTCONNECTED:
	  case CCD_BL_DISCONNECTED:
		dc_stop = 1;
		sprintf(message," CCD_BL_NOTCONNECTED");
		issue_signal("CCD_SIG_BL_DROPPED");
                set_alert_msg("ERROR: Beamline control not connected.");
/* printf ("no beamline\n"); */
	  break;
          default:
	  break;
	}
	if(dc_stop == 0 && retryshort != 0)
	  {
            fprintf(stdout,"send_bl_start: TIMERS        at %s: total %d\n",ztime(),time_total);
	    fflush(stdout);
            if(time_total < (stat_time - 2.))   /* for sure too fast */
              {
                fprintf(stdout,"send_bl_start: DONE (retry)  at %s: EXPOSURE TOO SHORT\n",ztime());
	        fflush(stdout);
                bline_ret = CCD_BL_RETRY;
		exposure_short_by = (int) (stat_time - time_total);
		exposure_short_by += 3;		/* allow some slop here */
              }
          }
        return(bline_ret);
  }

mdc_command	command_while_waiting;
mdc_command	original_command;
mdc_command	*original_command_ptr;

void	switch_to_wait(mdccp, res)
mdc_command	*mdccp;
int		res;
{
	original_command = *mdccp;
	original_command_ptr = mdccp;

	command_while_waiting.cmd_used = 1;
	command_while_waiting.cmd_no = MDC_COM_HOLDING;
	command_while_waiting.cmd_err = 0;
	command_while_waiting.cmd_value = beam_sense;

	strcpy(stat_scanner_msg,"");
	strcpy(stat_scanner_op,"");
	strcpy(stat_scanner_control,"waiting_for_beam");
	set_alert_msg("Beam DOWN or DUMPED: Waiting for Beam");

	*mdccp = command_while_waiting;
}

int	holding_over()
{
	char	*ztime();
	char	msg_body[256];

	if(dc_stop)
	{
		*original_command_ptr = original_command;
		return(1);
	}
	output_blcmd(fdblcmd,"cmd", "bl_ready 1");		/* bl status*/
	if(bl_returned_string[0] == '\0')
	{
		fprintf(stdout,"holding_over : HOLD:          at %s: Beamline READY returned NOTHING\n",ztime());
		*original_command_ptr = original_command;
		return(1);
	}
	else if(NULL != strstr(bl_returned_string, "DOWN"))
	{
		if(0)
			fprintf(stdout,"holding_over : HOLD:          at %s: Beamline NOT ready: returned DOWN\n",ztime());
		return(0);
	}
	else
	{
		if(0)
		fprintf(stdout,"holding_over : HOLD:          at %s: Beamline READY: returned UP\n",ztime());
		if(command_while_waiting.cmd_value == beam_sense)
		{
			sprintf(msg_body, "Beam UP: Wating %d sec for Optics Stability", beam_sense);
			set_alert_msg(msg_body);
			strcpy(stat_scanner_msg,"");
			strcpy(stat_scanner_op,"");
			strcpy(stat_scanner_control,"waiting_for_optics_stab");
		}
		command_while_waiting.cmd_value -= 0.50;
		if(command_while_waiting.cmd_value > 0)
		{
			if(0)
			fprintf(stdout,
				"holding_over : HOLD:          at %s: %.2f seconds REMAINING before RESUME data collection\n",
					ztime(), command_while_waiting.cmd_value);
			return(0);
		}
		*original_command_ptr = original_command;
		return(1);
	}
}

/*
 *      To collect a frame, one gets a lock on the beamline process, starts the detector in collection
 *      mode, starts the beamline process, waits for completion indicating status, and then unlocks
 *     the detector.
 *
 */
int	 	collect_frame(mdccp)
mdc_command	*mdccp;
{
	char	*ztime();
	int	max_trys = 2;
	int	ntry;

	if(ccd_modular && beam_sense > 0 && mdccp->cmd_no == MDC_COM_COLL)
	{
		output_blcmd(fdblcmd,"cmd", "bl_ready 1");		/* bl status*/
		if(bl_returned_string[0] == '\0')
			fprintf(stdout,"collect_frame: BFOR:          at %s: Beamline READY returned NOTHING\n",ztime());
		else if(NULL != strstr(bl_returned_string, "DOWN"))
		{
			fprintf(stdout,"collect_frame: BFOR:          at %s: Beamline NOT ready: returned DOWN\n",ztime());
			return(CF_BL_NOTREADY_BEFORE);
		}
	}

#ifdef X8C
	send_bl_lock();
	output_blcmd(fdblcmd,"cmd","shutter 0");   /* close the shutter, just in case */
#endif /* X8C */

	send_det_start();
	gl_blret = send_bl_start();

#ifdef X8C
	send_bl_unlock();
#endif /* X8C */

	for(ntry = 0; ntry < max_trys; ntry++)
	{
	if(beam_sense > 0 && mdccp->cmd_no == MDC_COM_COLL)
	{
		output_blcmd(fdblcmd,"cmd", "bl_ready 1");		/* bl status*/
		if(bl_returned_string[0] == '\0')
			fprintf(stdout,"collect_frame: AFTR:          at %s: Beamline READY returned NOTHING\n",ztime());
		else if(NULL != strstr(bl_returned_string, "DOWN"))
		{
			fprintf(stdout,"collect_frame: AFTR:          at %s: Beamline NOT ready: returned DOWN\n",ztime());
			return(CF_BL_NOTREADY_AFTER);
		}
	}
	}

	return(CF_BL_READY);
}
send_copy_command()
  {
	int	len_xfcmd;
	char	xfcmd_buf[512];
        char    num[4];
        int     i,kind,raw_end;
	char	suffix[20],raw_suffix[20];
	char	body[512];
	char	dzstuff[100];
	int	hsize;

	gethdl(&hsize, made_header);

        if(raw_ccd_image == 1)
          sprintf(body,
 "copy\nreply 0\nrow_mm %f\ncol_mm %f\ndist_mm %f\ntwo_theta %f\nheader_size %d\nrow_xfer %d\ncol_xfer %d\nrow_bin %d\ncol_bin %d\n",
                beam_xcen,beam_ycen,stat_dist,stat_2theta,hsize,chip_size_y / stat_bin ,chip_size_x / stat_bin ,
                        stat_bin,stat_bin);
         else
          sprintf(body,
 "xform\nreply 0\nrow_mm %f\ncol_mm %f\ndist_mm %f\ntwo_theta %f\nheader_size %d\nrow_xfer %d\ncol_xfer %d\nrow_bin %d\ncol_bin %d\n",
                beam_xcen,beam_ycen,stat_dist,stat_2theta,hsize,chip_size_y / stat_bin,chip_size_x / stat_bin,
                        stat_bin,stat_bin);

	if(stat_compress == 1)
		strcat(body,"compress 1\n");
	    else
		strcat(body,"compress 0\n");
	if(detector_sn > 0)
	  {
		sprintf(dzstuff,"detector_sn %d\n",detector_sn);
		strcat(body,dzstuff);
	  }
	if(strip_ave)
	  {
		if(n_strip_ave == 4)
		  sprintf(dzstuff,"strip_ave %.3f_%.3f_%.3f_%.3f\n",strip_ave_vals[0],strip_ave_vals[1],
								    strip_ave_vals[2],strip_ave_vals[3]);
		 else
		  sprintf(dzstuff,"strip_ave %.3f\n",strip_ave_vals[0]);
		strcat(body,dzstuff);
	  }
	sprintf(dzstuff,"save_raw %d\n",output_raws);
	strcat(body,dzstuff);

	if(doing_dark_current)
	  {
	    if(doing_second_dk == 0)
	    	strcpy(suffix,"dkc");
	      else
		strcpy(suffix,"dkd");

	    strcpy(raw_suffix,"dkx");
	    kind = 2 * doing_second_dk + 2 - double_exp_ctr;
	    raw_end = kind;
	  }
	 else
	  {
	    strcpy(suffix,"img");
	    strcpy(raw_suffix,"imx");
	    if(double_exp_flag)
	      {
	        kind = 4 + 2 - double_exp_ctr;
		raw_end = 2 - double_exp_ctr;
		sprintf(dzstuff,"dzratio %f\n",stat_dzratio);
		strcat(body, dzstuff);
	      }
	     else
	      {
	        kind = 5;
		raw_end = 0;
	      }
	  }

	sprintf(dzstuff,"outfile_type %d\n",outfile_type);
	strcat(body,dzstuff);

        i = stat_image_number;
        num[0] = '0' + i / 100;
        i = i - 100 * (i / 100);
        num[1] = '0' + i / 10;
        i = i - 10 * (i / 10);
        num[2] = '0' + i;
        num[3] = '\0';

	if(-1 == fdxfcm)
	  {
		fprintf(stderr,"ccd_dc: xform command file is NOT connected.\n");
		fprintf(stderr,"ccd_dc: currently, THIS IS A WARNING\n");
	  }
	 else
	  {
		sprintf(xfcmd_buf,"%sinfile <socket>\noutfile %s%s_%s.%s\nrawfile %s%s_%s.%s_%d\nkind %d\n",
			body,stat_dir,stat_prefix,num,suffix,stat_dir,stat_prefix,num,raw_suffix,raw_end,kind);
		strcat(xfcmd_buf,"end_of_det\n");
		len_xfcmd = strlen(xfcmd_buf);
		if(len_xfcmd != rep_write(fdxfcm,xfcmd_buf,len_xfcmd))
		  {
			fprintf(stderr,"ccd_dc: xform process has disconnected.\n");
			fprintf(stderr,"ccd_dc: currently, THIS IS A WARNING.\n");
			notify_server_eof(fdxfcm);
		  }
	  }
  }

/*
 *	Get the next image from the runs file.
 */

get_next_image_from_runfile(mdccp)
mdc_command	*mdccp;
  {
	char	read_file_line[256],read_file_name[256];
	float	f1,f2,f3,f4,f5;
	char	tempbuf[2048];

	if(NULL != fgets(read_file_line,sizeof (read_file_line), fprun))
	  {
	    sscanf(read_file_line,"%s %f %f %f %f %f",read_file_name,&f1,&f2,&f3,&f4,&f5);
	    parse_file_name(read_file_name,stat_prefix,&stat_image_number);
	    mdccp->cmd_col_time = f1;
	    if(mdccp->cmd_col_kappas != -9999.)
	      {
		if(fabs(f5 - stat_kappa) > 0.0001)
		  {
			sprintf(tempbuf,"kappa_move %10.3f",f5);
	        	output_blcmd(fdblcmd,"cmd",tempbuf);
			stat_kappa = f5;
		  }
	      }
	    if(is_phi && mdccp->cmd_col_phis != -9999.)
	      {
		if(fabs(f4 - phi_value_saved) > 0.0001)
		{
			sprintf(tempbuf,"phi_move %10.3f",f4);
	        	output_blcmd(fdblcmd,"cmd",tempbuf);
		}
		stat_phi = f4;
	        phi_value_saved = f4;
	      }
	    if(is_omega && mdccp->cmd_col_omegas != -9999.)
	      {
		if(fabs(f3 - omega_value_saved) > 0.0001)
		{
			sprintf(tempbuf,"omega_move %10.3f",f3);
	        	output_blcmd(fdblcmd,"cmd",tempbuf);
		}
		stat_omega = f3;
		omega_value_saved = f3;
	      }
/*
 *	This needs to be changed so that it won't crash
 *	bl progs with no wavelength move:
 *	    if(fabs(f2 - stat_wavelength) > 0.001)
 */
 	    if(is_wavelength)
	      {
	        strcpy(stat_scanner_op,"setting_wavelength");
	        sprintf(tempbuf,"wavelength_move %.6f",f2);
	        output_blcmd(fdblcmd,"cmd",tempbuf);
	        stat_wavelength = f2;
	      }
	     else
	      {
 	        if(fabs(f2 - stat_wavelength) > 0.000005)
	          {
	            strcpy(stat_scanner_op,"setting_wavelength");
	            sprintf(tempbuf,"wavelength_move %.6f",f2);
	            output_blcmd(fdblcmd,"cmd",tempbuf);
	            stat_wavelength = f2;
	          }
	      }
	  }
	 else
	   totimg = 0;
  }

/*
 *	ccd_hw_start:
 *
 *	This is where all commands to the ccd detector system
 *	begin execution.
 *
 *	ccd_hw_start will begin the sequencing of possibly
 *	complex commands, splitting their functions between
 *	calls to the beamline control and detector control
 *	processes.
 */

void	ccd_hw_start(mdccp)
mdc_command	*mdccp;
  {
	int	current,wanted,nsteps_to_move;
	int	i,j,k,runno,need_new_dk;
	int	run_found;
	float	f1,f2,f3,f4,f5;
	char	read_file_line[256],read_file_name[256];
	char	tempfname[256];
	char	tempbuf[2048],tbuf[2048],prefix_without_run[80];
	int	bline_ret;
	int	ccd_hw_progress();
	int	read_bl_wavelength;
	float	wavelength_read;
	double	velocity;
	time_t	now;
	char	message[80];
	int	cntl_stat, exp_mode, exp_stat, snap_ok;

	double	fabs();

	active_trans = 0;
	command_rejected = 0;

	strcpy(stat_scanner_control,"active");

        if(mdccp->cmd_no != MDC_COM_STARTUP)
          {
            if(fdblcmd == -1)
              {
		sprintf(message," ccd_hw_start");
		issue_signal("CCD_SIG_BL_DROPPED",message);
                set_alert_msg("ERROR: Beamline process NOT connected.");
                return;
              }
          }

	switch(mdccp->cmd_no)
	  {
	    case MDC_COM_STARTUP:
		break;
	    case MDC_COM_INIT:

		strcpy(stat_scanner_op,"initializing");
		output_blcmd(fdblcmd,"cmd","initialize");

		break;

	    case MDC_COM_ERASE:

		break;

	    case MDC_COM_CONFIG:

		break;

	    case MDC_COM_DSET:

		strcpy(stat_scanner_op,"setting distance");
		sprintf(tempbuf,"distance_set %10.2f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);
		stat_dist = mdccp->cmd_value;

		break;

	    case MDC_COM_PSET:

		strcpy(stat_scanner_op,"setting phi");
		sprintf(tempbuf,"phi_set %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);
		stat_phi = mdccp->cmd_value;

		break;

	    case MDC_COM_OSET:

		strcpy(stat_scanner_op,"setting omega");
		sprintf(tempbuf,"omega_set %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);
		stat_omega = mdccp->cmd_value;

		break;

	    case MDC_COM_KSET:

		strcpy(stat_scanner_op,"setting kappa");
		sprintf(tempbuf,"kappa_set %10.23",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);
		stat_kappa = mdccp->cmd_value;

		break;

	    case MDC_COM_LSET:

		delta = mdccp->cmd_value;
		strcpy(stat_scanner_op,"setting lift");
		sprintf(tempbuf,"lift_set %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);
		stat_lift = delta;

		break;

            case MDC_COM_WSET:

                delta = mdccp->cmd_value;
                strcpy(stat_scanner_op,"setting wavelength");
                sprintf(tempbuf,"wavelength_set %7.5f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                stat_wavelength = delta;

                break;

	    case MDC_COM_SHUT:

		strcpy(stat_scanner_op,"Shutter control");
		if(use_pc_shutter == 0)
		{
		if(mdccp->cmd_value != 0)
		  {
			strcpy(stat_scanner_shutter,"open");
			strcpy(tempbuf,"shutter 1");
		  }
		 else
		  {
			strcpy(stat_scanner_shutter,"closed");
			strcpy(tempbuf,"shutter 0");
		  }
		output_blcmd(fdblcmd,"cmd",tempbuf);
		} else {
		    if(mdccp->cmd_value != 0)
		      {
			strcpy(stat_scanner_shutter,"open");
			strcpy(tempbuf,"shutter\npcshutter 1\n");
		      }
		     else
		      {
			strcpy(stat_scanner_shutter,"closed");
			strcpy(tempbuf,"shutter\npcshutter 0\n");
		      }
		    output_detcmd(fddetcmd,tempbuf,NULL,0);
		    /*
		     *	This is prurely cosmetic:  This gets the
		     *	shutter status properly set via the beamline
		     *	process, which does not actually do anything
		     *	BUT set this status.
		     */
		    if(mdccp->cmd_value != 0)
		      {
			strcpy(stat_scanner_shutter,"open");
			strcpy(tempbuf,"shutter 1");
		      }
		     else
		      {
			strcpy(stat_scanner_shutter,"closed");
			strcpy(tempbuf,"shutter 0");
		      }
		    output_blcmd(fdblcmd,"cmd",tempbuf);
		  }



		break;

	    case MDC_COM_SCAN:

		break;

	    case MDC_COM_DMOVE:
		strcpy(stat_scanner_op,"moving distance");
		sprintf(tempbuf,"distance_move %10.2f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_HOLDING:

		break;

            case MDC_COM_AMOVE:

                strcpy(stat_scanner_op,"moving_attenuator");
                sprintf(tempbuf,"attenuator_move %8.5f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

            case MDC_COM_AUTOALIGN:

                strcpy(stat_scanner_op,"autoaligning");
                sprintf(tempbuf,"autoalign %.1f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

	    case MDC_COM_SET_MASTER:
                strcpy(stat_scanner_op,"set_master");
                sprintf(tempbuf,"set_master %s",mdccp->cmd_col_dir);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_LMOVE:

		strcpy(stat_scanner_op,"moving lift");
		sprintf(tempbuf,"lift_move %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

            case MDC_COM_WMOVE:

                strcpy(stat_scanner_op,"moving wavelength");
                sprintf(tempbuf,"wavelength_move %7.5f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);

                break;

	    case MDC_COM_GET_CLIENTS:
                strcpy(stat_scanner_op,"get_clients");
                sprintf(tempbuf,"get_clients %.1f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

	    case MDC_COM_EXPERIMENT_MODE_MOVE:
                strcpy(stat_scanner_op,"experiment_mode_move");
                sprintf(tempbuf,"experiment_mode_move %.1f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

	    case MDC_COM_HSLIT:
                strcpy(stat_scanner_op,"move_horiz_slit");
                sprintf(tempbuf,"hslit_move %.3f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

	    case MDC_COM_VSLIT:
                strcpy(stat_scanner_op,"move_vert_slit");
                sprintf(tempbuf,"vslit_move %.3f",mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
                break;

	    case MDC_COM_XL_HS_MOVE:

                strcpy(stat_scanner_op,"moving_horiz_slit");
                sprintf(tempbuf, "xl_hs_move %.3f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_VS_MOVE:

                strcpy(stat_scanner_op,"moving_vert_slit");
                sprintf(tempbuf, "xl_vs_move %.3f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_GUARD_HS_MOVE:

                strcpy(stat_scanner_op,"moving_horiz_guard_slit");
                sprintf(tempbuf, "xl_guard_hs_move %.3f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_GUARD_VS_MOVE:

                strcpy(stat_scanner_op,"moving_vert_guard_slit");
                sprintf(tempbuf, "xl_guard_vs_move %.3f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_UP_HHS_MOVE:

                strcpy(stat_scanner_op,"moving_up_h_halfslit");
                sprintf(tempbuf, "xl_up_hhs_move %.1f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_UP_VHS_MOVE:

                strcpy(stat_scanner_op,"moving_up_v_halfslit");
                sprintf(tempbuf, "xl_up_vhs_move %.1f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_DN_HHS_MOVE:

                strcpy(stat_scanner_op,"moving_dn_h_halfslit");
                sprintf(tempbuf, "xl_dn_hhs_move %.1f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_XL_DN_VHS_MOVE:

                strcpy(stat_scanner_op,"moving_dn_v_halfslit");
                sprintf(tempbuf, "xl_dn_vhs_move %.1f", mdccp->cmd_value);
                output_blcmd(fdblcmd,"cmd",tempbuf);
		break;

	    case MDC_COM_PMOVE:

		strcpy(stat_scanner_op,"moving phi");
		sprintf(tempbuf,"phi_move %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_PMOVEREL:

		strcpy(stat_scanner_op,"moving phi");
		sprintf(tempbuf,"phi_move_rel %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_OMOVE:

		strcpy(stat_scanner_op,"moving omega");
		sprintf(tempbuf,"omega_move %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_OMOVEREL:

		strcpy(stat_scanner_op,"moving omega");
		sprintf(tempbuf,"omega_move_rel %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_KMOVE:

		strcpy(stat_scanner_op,"moving kappa");
		sprintf(tempbuf,"kappa_move %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_GONMAN:

		strcpy(stat_scanner_op,"Goniostat in Manual Mode");
		sprintf(tempbuf,"gon_manual %10.3f",mdccp->cmd_value);
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_HOME:

		strcpy(stat_scanner_op,"Homing Goniostat Angles");
		sprintf(tempbuf,"home");
		output_blcmd(fdblcmd,"cmd",tempbuf);

		break;

	    case MDC_COM_COLL:

	    case MDC_COM_SNAP:

		if(no_image_taken && mdccp->cmd_col_no_transform == 0)
		{
			mdccp->cmd_no = MDC_COM_COLL;
			mdccp->cmd_col_newdark = 1;
			no_image_taken = 0;
		}
		read_bl_wavelength = 1;
		if(bm8_mod)
		{
			while(1)
			{
				output_blcmd(fdblcmd, "cmd", "get_wavelength");
				if(bl_returned_string[0] == '\0')
				{
					fprintf(stderr,
						"ccd_dc_api   : WARN:          NO value returned from get_wavelength.\n");
					break;
				}
				else if(bl_returned_string[0] < '0' || bl_returned_string[0] > '9')
				{
					fprintf(stderr,
						"ccd_dc_api   : WARN:          get_wavelength returned an error: %s.\n",
							bl_returned_string);
					break;
				} else
				{
					char	*cpxx;
					double	wavelength_via_blret;

					cpxx = (char *) strstr(bl_returned_string, "OK");
					if(cpxx != NULL)
						*cpxx = '\0';
					wavelength_via_blret = atof(bl_returned_string);
					if(fabs(wavelength_via_blret - stat_wavelength) <= 0.000005)
						 break;
				}
			}
			
		}
#ifdef	X8C
		{
		  /* get the wavelength --JB 14-Jun-01 */
#include "../incl/issue_signal.h"
		     char message[80];
		     static char *msgname = "CCD_SIG_GETWAVE";
		     float  fw;
		     int retval;
		     
		     read_bl_wavelength = 0;
		     fw = 0;
		     message[0] = '\0';
		     retval = issue_signal(msgname, message);
		     if (retval == SIG_OK)   /* Normal response */
		       {
			 if (strlen(message) != 0)
			   {
			     sscanf(message,"%f",&fw);
			     if ((fw < 0.5) || (fw > 5.0))
			       {
				 fprintf(stdout,"Crazy wavelength of %f A read but ignored.\n",fw);
				 read_bl_wavelength = 0;
			       }
			     else
			       {
				 stat_wavelength = fw;
				 fprintf(stdout,"INFO: Wavelength set to: %f A\n",stat_wavelength);
				 read_bl_wavelength = 1;
				 wavelength_read = fw;
			       }
			   }
			 else
			   {
			     fprintf(stderr,"Error-%s did not return a value.\n", msgname);
			   }
		       }
		     else if (retval == SIG_SYSERR)
		       {
			 fprintf(stderr,"Error in system call to %s \n", msgname);
		       }
		     else if (retval == SIG_CMDERR)
		       {
			 fprintf(stderr,"Error-%s gave error message  %s", msgname,  message);
		       }
		     else if (retval == SIG_NULLMSG)
		       {
			 fprintf(stderr,"Error-%s did not return OK\n", msgname);
		       }
		   }
#endif /* X8C */

                if(fddetcmd == -1 || fdxfcm == -1)
                  {
                        strcpy(tempbuf,"ERROR: ");
			sprintf(message," MDC_COLL");
                        if(fddetcmd == -1)
			  {
			    strcat(tempbuf,"Detector process NOT connected.");
			    issue_signal("CCD_SIG_DET_DROPPED",message);
			  }
                        if(fdxfcm == -1)
			  {
			    strcat(tempbuf,"Transform process NOT connected.");
			    issue_signal("CCD_SIG_XFORM_DROPPED",message);
			  }
                        set_alert_msg(tempbuf);
			command_rejected = 1;
                        return;
                  }

		if(mdccp->cmd_col_osc_width == 0)
		{
			if(allow_stills == 0)
			{
				set_alert_msg("ERROR: A still (rotation width=0) is NOT allowed");
				command_rejected = 1;
				return;
			}
		}
		else
		{
			velocity = mdccp->cmd_col_osc_width / mdccp->cmd_col_time;
			if(velocity < min_velocity)
			{
				set_alert_msg("ERROR: velocity (time / width) too small for collect");
				command_rejected = 1;
				return;
			} 
			else
			if(velocity > max_velocity)
			{
				set_alert_msg("ERROR: velocity (time / width) too large for collect");
				command_rejected = 1;
				return;
			}
		}
		parse_file_name(mdccp->cmd_col_prefix,prefix_without_run,&runno);
		if(mdccp->cmd_col_restart_run == -1)
		  {
/*
 * JOELS
 *		    if(mdccp->cmd_col_mad_mode == 0)
 *		      {
 *			mdccp->cmd_col_mad_nwave = 1;
 *			if(read_bl_wavelength == 0 || mdccp->cmd_col_run_wave < 0)
 *				mdccp->cmd_col_mad_wavelengths[0] = stat_wavelength;
 *			else
 *				mdccp->cmd_col_mad_wavelengths[0] = wavelength_read;
 *		      }
 *		    mdccp->cmd_col_restart_run = runno;
 */		    mdccp->cmd_col_restart_image = mdccp->cmd_col_image_number;

/*
 *CNs
 */
                    if(mdccp->cmd_col_mad_mode == 0)
                      {
                        if(mdccp->cmd_col_run_wave <= 0 || mdccp->cmd_no == MDC_COM_SNAP)
                        {
                                mdccp->cmd_col_mad_nwave = 1;
                                mdccp->cmd_col_mad_wavelengths[0] = stat_wavelength;
                        }
                        else
                        {
                                mdccp->cmd_col_mad_nwave = 1;
                                mdccp->cmd_col_mad_wavelengths[0] = mdccp->cmd_col_run_wave;
                        }
                      }
                    mdccp->cmd_col_restart_run = runno;
                    mdccp->cmd_col_restart_image = mdccp->cmd_col_image_number;


		    generate_run_list(mdccp);
		  }
		fprintf(stdout,"ccd_dc_api   : INFO:          restart_run: %d restart_image: %d\n", 
			mdccp->cmd_col_restart_run, mdccp->cmd_col_restart_image);
		stat_2theta = mdccp->cmd_col_lift;

		/*
		 *	Find the first image in the runs file.
		 */
		sprintf(tempfname,"%s/%s.runlist",mdccp->cmd_col_dir,mdccp->cmd_col_prefix);
		if(NULL == (fprun = fopen(tempfname,"r")))
		  {
		    fprintf(stderr,"ccd_dc_api: FATAL: Cannot open run file %s\n",tempfname);
		    fprintf(stderr,"            This file is NECESSARY for the program to run\n");
		    fprintf(stderr,"            and should have been generated previously\n");
		    fprintf(stderr,"            by this program.\n");
		    cleanexit(BAD_STATUS);
		  }

		/*
		 *	Generate the file name of the actual beginning image
		 *	(except the .img part).
		 */

		util_3digit(tbuf,mdccp->cmd_col_restart_image);
		sprintf(tempbuf,"%s_%d_%s",prefix_without_run,mdccp->cmd_col_restart_run,tbuf);
		fprintf(stderr,"ccd_dc_api   : INFO:          looking for image %s as first image number\n",tempbuf);

		/*
		 *	Take this image name and read each line in
		 *	the runs file, one at a time, until this string is
		 *	matched.  When this occurs, we will have the necessary
		 *	angle, etc., parameters, for the image.  The logic
		 *	works weather restart or no.
		 */

		run_found = 0;
		while(NULL != fgets(read_file_line,sizeof (read_file_line), fprun))
		  if(NULL != (char *) strstr(read_file_line,tempbuf))
		    {
			run_found = 1;
			break;
		    }
		if(run_found == 0)
		  {
		    fprintf(stderr,"ccd_dc_api: Run %d, image %d NOT FOUND in current\n",
			mdccp->cmd_col_restart_run,mdccp->cmd_col_restart_image);
		    fprintf(stderr,"            Not performing this run.\n");
		    dc_abort = 1;
		    break;
		  }
		sscanf(read_file_line,"%s %f %f %f %f %f",read_file_name,&f1,&f2,&f3,&f4,&f5);
		parse_file_name(read_file_name,stat_prefix,&stat_image_number);
		mdccp->cmd_col_time = f1;
		if(mdccp->cmd_col_omegas != -9999.)
			mdccp->cmd_col_omegas = f3;
		if(mdccp->cmd_col_phis != -9999.)
			mdccp->cmd_col_phis = f4;
		if(mdccp->cmd_col_kappas != -9999.)
			mdccp->cmd_col_kappas = f5;
		mdccp->cmd_col_image_number = mdccp->cmd_col_restart_image;

		omega_value_saved = stat_omega;
		phi_value_saved = stat_phi;

		if(is_wavelength)
		  {
		    strcpy(stat_scanner_op,"setting_wavelength");
		    sprintf(tempbuf,"wavelength_move %7.5f",f2);
		    output_blcmd(fdblcmd,"cmd",tempbuf);
		    stat_wavelength = f2;
		  }
		else
		  {
		    if(fabs(f2 - stat_wavelength) > 0.000005)
		      {
		        strcpy(stat_scanner_op,"setting_wavelength");
		        sprintf(tempbuf,"wavelength_move %7.5f",f2);
		        output_blcmd(fdblcmd,"cmd",tempbuf);
		        stat_wavelength = f2;
		      }
		  }
		if(mdccp->cmd_col_atten_run != -1)
		{
//			if(mdccp->cmd_no == MDC_COM_COLL || fabs(stat_attenuator - mdccp->cmd_col_atten_run) > .001)
			if(mdccp->cmd_no == MDC_COM_COLL)
			{
		    		strcpy(stat_scanner_op,"setting_attenuator");
		    		sprintf(tempbuf,"attenuator_move %7.5f",mdccp->cmd_col_atten_run);
		    		output_blcmd(fdblcmd,"cmd",tempbuf);
		    		stat_attenuator = mdccp->cmd_col_atten_run;
			}
		}

		if(mdccp->cmd_col_autoal_run == 1)
		{
                	strcpy(stat_scanner_op,"autoaligning");
                	sprintf(tempbuf,"autoalign 1");
                	output_blcmd(fdblcmd,"cmd",tempbuf);
		}

		if(mdccp->cmd_col_hslit_run != -1)
		{
		    		strcpy(stat_scanner_op,"setting_horiz_slit");
		    		sprintf(tempbuf,"hslit %7.5f",mdccp->cmd_col_hslit_run);
		    		output_blcmd(fdblcmd,"cmd",tempbuf);
		    		stat_hslit = mdccp->cmd_col_hslit_run;
		}

		if(mdccp->cmd_col_vslit_run != -1)
		{
		    		strcpy(stat_scanner_op,"setting_vert_slit");
		    		sprintf(tempbuf,"vslit %7.5f",mdccp->cmd_col_vslit_run);
		    		output_blcmd(fdblcmd,"cmd",tempbuf);
		    		stat_vslit = mdccp->cmd_col_vslit_run;
		}

		if (mdccp->cmd_no == MDC_COM_SNAP) {
 		   doing_snapshot = 1;
		}
	        else {
		   doing_snapshot = 0;

		   // 8BM run CONSOLE housekeeping
		   if (bm8_mod) {

		     // make sure current client-server connection to CSERVER working.
		     // if not kill old connection and reattach
		     // if reattachment fails barf!

                      bline_ret = output_blcmd(fdblcmd,"cmd", "check_cons"); 
		      if (bline_ret == 0) {
			 set_alert_msg("Cannot re-attach to CONSOLE RPC-server/8BM");
			 dc_abort = 1;
			 break;
		      }

		      // if DCM4 note current CONSOLE script chain to it
                      bline_ret = output_blcmd(fdblcmd,"cmd", "current_script"); 
		   }
                }

		need_new_dk = 0;

		phi_speed_used = phi_top_speed;
		stat_time = mdccp->cmd_col_time;

		if(mdccp->cmd_col_omegas != -9999.)
			stat_start_omega = mdccp->cmd_col_omegas;
		if(mdccp->cmd_col_phis != -9999.)
			stat_start_phi = mdccp->cmd_col_phis;
		if(mdccp->cmd_col_kappas != -9999.)
			stat_start_kappa = mdccp->cmd_col_kappas;

		stat_axis = mdccp->cmd_col_axis;
		phi_value_saved = stat_start_phi;
		omega_value_saved = stat_start_omega;
		stat_omega = stat_start_omega;
		stat_phi = stat_start_phi;
		stat_kappa = stat_start_kappa;

		stat_osc_width = mdccp->cmd_col_osc_width;
		stat_n_images = mdccp->cmd_col_n_images;
		stat_n_passes = mdccp->cmd_col_n_passes;
		stat_image_number = mdccp->cmd_col_image_number;

		stat_compress = mdccp->cmd_col_compress;
		stat_anom = mdccp->cmd_col_anom;
		stat_wedge = mdccp->cmd_col_wedge;
		if(mdccp->cmd_col_dzratio != -1)
			stat_dzratio = mdccp->cmd_col_dzratio;
		if(mdccp->cmd_col_dkinterval != -1)
			dark_current_interval = mdccp->cmd_col_dkinterval;
		if(mdccp->cmd_col_rep_dark != -1)
			repeat_dark_current = mdccp->cmd_col_rep_dark;
		if(mdccp->cmd_col_dk_before != -1)
			dk_before_run = mdccp->cmd_col_dk_before;
		if(mdccp->cmd_col_outfile_type != -1)
			outfile_type = mdccp->cmd_col_outfile_type;
		if(mdccp->cmd_col_no_transform != -1)
		  {
			no_transform = mdccp->cmd_col_no_transform;
			raw_ccd_image = no_transform;
		  }
		if(mdccp->cmd_col_output_raws != -1)
			output_raws = mdccp->cmd_col_output_raws;
		if(stat_n_images > stat_wedge)
		    wedge_size = stat_wedge;
		  else
		    wedge_size = stat_n_images;

		if(stat_n_passes == 2)
			double_exp_flag = 1;
		  else
			double_exp_flag = 0;
		strcpy(stat_dir,mdccp->cmd_col_dir);

		if(0)	/* DEBUG 4/26/99: SUPERCEEDED BY MAD CODE HANDLING OF NAMES */
		  strcpy(stat_prefix,mdccp->cmd_col_prefix);

		if(stat_adc != mdccp->cmd_col_adc || stat_bin != mdccp->cmd_col_bin)
		    need_new_dk = 1;
		if(dk_before_run)
		    need_new_dk = 1;
		if(need_new_dk)
		    time((time_t *) &dark_current_time);

		stat_adc = mdccp->cmd_col_adc;
		stat_bin = mdccp->cmd_col_bin;

		if(stat_anom)	/* for anomalous, we need to figure out the goniostat type */
		  {
		    if(is_kappa)
		      {
		        goniostat_type = 2;
			wedge_kappa = stat_start_kappa;
			wedge_omega = stat_start_omega;
			wedge_phi =   stat_start_phi;
			kappa_init(kappa_const);
		      }
		     else
		      {
		        goniostat_type = 0;
			wedge_phi =   stat_start_phi;
		      }
		    wedge_count = wedge_size;
		    wedge_side = 0;
		    j = strlen(stat_prefix);
		    for(i = j - 1; i >= 0; i--)
		      if(stat_prefix[i] == '_')
			break;
		    wedge_run = atoi(&stat_prefix[i+1]);
		    wedge_imno = stat_image_number;
		    for(j = 0; j < i; j++)
		      wedge_prefix[j] = stat_prefix[j];
		    wedge_prefix[j] = '\0';
		  }

		beam_xcen = mdccp->cmd_col_xcen;
		beam_ycen = mdccp->cmd_col_ycen;
		totimg = stat_n_images;
		totpass = stat_n_passes;
		stat_mode = mdccp->cmd_col_mode;
		if(stat_mode == 1)	/* dose */
			stat_time = stat_dose_step;

		if(stat_n_passes == 2)
		  {
		    double_exp_flag = 1;
		    double_exp_ctr = 2;
		  }
		 else
		  {
		    double_exp_flag = 0;
		    double_exp_ctr = 1;
		  }

		if(mdccp->cmd_no == MDC_COM_COLL)
		  {
		    fprintf(stdout,
			    "ccd_dc_hw    : INFO:          doing collect, stat_time: %.2f current_darkcurrent_time: %.2f\n",
			    stat_time, current_darkcurrent_time);
		    if(mdccp->cmd_col_newdark == 1 || need_new_dk == 1 || stat_time != current_darkcurrent_time)
		      {
		        fprintf(stdout,"ccd_dc_hw    : INFO:          setting up to do a new darkcurrent\n");
			doing_dark_current = 1;
			doing_second_dk = 0;
			double_exp_ctr = 2;
			current_darkcurrent_time = stat_time;
		      }
		     else
			doing_dark_current = 0;
		    if(repeat_dark_current)
		      {
			fprintf(stdout,"ccd_dc_api   : INFO:          informational: dark currents will be repeated\n");
			fprintf(stdout,"ccd_dc_api   :                every %d seconds.\n",dark_current_interval);
		      }
		  }
		 else	/* snapshot with no recalc'd dark current */
		  {
		    doing_second_dk = 0;
		    doing_dark_current = 0;
		  }

		/*
		 *	Always move/set the distance for SNAP and COLLECT.
		 */
		if(pf_mod)
		{
			bline_ret = output_blcmd(fdblcmd, "cmd", "fancybox_status 0.0");
		    	switch (bline_ret) {
		      		case CCD_BL_NOTCONNECTED:
		      		case CCD_BL_DISCONNECTED:
					sprintf(message," CCD_BL_NOTCONNECTED");
					issue_signal("CCD_BL_DROPPED",message);
	               	 		set_alert_msg("ERROR: Beamline control not connected.");
		      		case CCD_BL_FATAL:	/* merges with code below */
                  			set_alert_msg("ERROR: Run ABORTED. Set experiment_mode FAILED.");
					dc_abort = 1;
					dc_stop = 1;
		        		break;
	              		default:
		  			break;
		      		}
			if(dc_abort)
			    break;

			fprintf(stdout,"ccd_dc_api   : INFO:          fancybox_status_string: %s", bl_reply);

			if(mdccp->cmd_no == MDC_COM_COLL)
			{
		        	strcpy(stat_scanner_op,"set_experiment_mode_dcmode");
		        	bline_ret = output_blcmd(fdblcmd,"cmd","experiment_mode_move 8.0");
		    		switch (bline_ret) {
		      			case CCD_BL_NOTCONNECTED:
		      			case CCD_BL_DISCONNECTED:
						sprintf(message," CCD_BL_NOTCONNECTED");
						issue_signal("CCD_BL_DROPPED",message);
	               	 			set_alert_msg("ERROR: Beamline control not connected.");
		      			case CCD_BL_FATAL:	/* merges with code below */
                  				set_alert_msg("ERROR: Run ABORTED. Set experiment_mode FAILED.");
						dc_abort = 1;
						dc_stop = 1;
		        			break;
	              			default:
		  				break;
		      			}
				if(dc_abort)
				    break;
			}
			else
			{
				sscanf(bl_reply, "%d,%d,%d", &cntl_stat, &exp_mode, &exp_stat);
				fprintf(stdout,"ccd_dc_api   : INFO:          cntl_stat: %d exp_mode: %d exp_stat: %d", 
						cntl_stat, exp_mode, exp_stat);
				snap_ok = 0;
				switch(exp_mode)
				{
					case TAK_FANCYMODE_DBM:
					case TAK_FANCYMODE_GSA:
					case TAK_FANCYMODE_FBSA:
					case TAK_FANCYMODE_RBSA:
					case TAK_FANCYMODE_DC:
						if(cntl_stat == 1 && exp_stat == 0)
							snap_ok = 1;
						break;
				}
				if(snap_ok == 0)
				{
		        	strcpy(stat_scanner_op,"set_experiment_mode_dcmode");
		        	bline_ret = output_blcmd(fdblcmd,"cmd","experiment_mode_move 8.0");
		    		switch (bline_ret) {
		      			case CCD_BL_NOTCONNECTED:
		      			case CCD_BL_DISCONNECTED:
						sprintf(message," CCD_BL_NOTCONNECTED");
						issue_signal("CCD_BL_DROPPED",message);
	               	 			set_alert_msg("ERROR: Beamline control not connected.");
		      			case CCD_BL_FATAL:	/* merges with code below */
                  				set_alert_msg("ERROR: Run ABORTED. Set experiment_mode FAILED.");
						dc_abort = 1;
						dc_stop = 1;
		        			break;
	              			default:
		  				break;
		      			}
				}
				if(dc_abort)
				    break;
			}
		}

		if(is_distance)
		{
			strcpy(stat_scanner_op,"moving distance");
			sprintf(tempbuf,"distance_move %10.2f",mdccp->cmd_col_dist);
			output_blcmd(fdblcmd,"cmd",tempbuf);
		}
		else
		  {
			strcpy(stat_scanner_op,"setting distance");
			sprintf(tempbuf,"distance_set %10.2f",mdccp->cmd_col_dist);
			output_blcmd(fdblcmd,"cmd",tempbuf);
		  }
/*
 *	This code was put into the ALS.  Removed at SPring8:
 *
 *		while(fabs(stat_dist - mdccp->cmd_col_dist) > 0.01)
 *			pause();
 */
		stat_dist = mdccp->cmd_col_dist;
		if(is_2theta && mdccp->cmd_col_lift != -9999.)
		  {
			if(0)
			{
			  fprintf(stdout,"ccd_dc_api: Would move lift before run start\n");
			}
			else
			{
			strcpy(stat_scanner_op,"moving 2theta");
			sprintf(tempbuf,"lift_move %10.3f",mdccp->cmd_col_lift);
			output_blcmd(fdblcmd,"cmd",tempbuf);
			}
		  }
		stat_2theta = mdccp->cmd_col_lift;

		/*
		 *	For the three goniostat angles, move them if they
		 *	exist for COLLECT.  SNAP is handled below.
		 */

		if(is_phi && mdccp->cmd_col_phis != -9999.)
		  {
			if(0)
			{
			  fprintf(stdout,"ccd_dc_api: Would move phi before run start\n");
			}
			else
			{
			strcpy(stat_scanner_op,"moving phi");
			sprintf(tempbuf,"phi_move %10.3f",mdccp->cmd_col_phis);
			output_blcmd(fdblcmd,"cmd",tempbuf);
			}
		  }

		if(is_omega && mdccp->cmd_col_omegas != -9999.)
		  {
		    if(0)
		    {
		      fprintf(stdout,"ccd_dc_api: Would move omega before run start\n");
		    }
		    else
		    {
		    strcpy(stat_scanner_op,"moving omega");
		    sprintf(tempbuf,"omega_move %10.3f",mdccp->cmd_col_omegas);
		    output_blcmd(fdblcmd,"cmd",tempbuf);
		    }
		  }
		if(is_kappa && mdccp->cmd_col_kappas != -9999.)
		  {
		    if(0)
		    {
		      fprintf(stdout,"ccd_dc_api: Would move kappa before run start\n");
		    }
		    else
		    {
		    strcpy(stat_scanner_op,"moving kappa");
		    sprintf(tempbuf,"kappa_move %10.3f",mdccp->cmd_col_kappas);
		    output_blcmd(fdblcmd,"cmd",tempbuf);
		    }
		  }
		
		/*
		 *	For a SNAP, ONLY move the one angle (omega or phi)
		 *	which is in motion (stat_axis).  If the goniostat has
		 *	an omega, ALL snapshots are in omega, else in phi.
		 */
		
		if(doing_snapshot == 1)
		  {
		    if(stat_axis == 1)
		      {
			strcpy(stat_scanner_op,"moving phi");
			sprintf(tempbuf,"phi_move %10.3f",mdccp->cmd_col_phis);
			output_blcmd(fdblcmd,"cmd",tempbuf);
		      }
		     else
		      {
		        strcpy(stat_scanner_op,"moving omega");
		        sprintf(tempbuf,"omega_move %10.3f",mdccp->cmd_col_omegas);
		        output_blcmd(fdblcmd,"cmd",tempbuf);
		      }
		  }

		dc_stop = 0;

		if(mdccp->cmd_col_blcmd[0] != '\0')
		  {
		    bline_ret = output_blcmd(fdblcmd,"before",mdccp->cmd_col_blcmd);
		    switch (bline_ret) {
		      case CCD_BL_NOTCONNECTED:
		      case CCD_BL_DISCONNECTED:
			dc_stop = 1;
			sprintf(message," CCD_BL_NOTCONNECTED");
			issue_signal("CCD_BL_DROPPED",message);
	                set_alert_msg("ERROR: Beamline control not connected.");
		  	break;
		      case CCD_BL_FATAL:
			dc_stop = 1;
			if(pf_mod)
			{
			}
		        break;
	              default:
		  	break;
		      }
		  }
		if(dc_stop == 1)
		  {
			dc_abort = 1;
			break;
		  }


                //time(&now);
	        //fprintf(stdout,"ccd_dc_api: pitch_tune_delta: %d pitch_tune_last: %d now: %d\n",
                //        pitch_tune_delta,pitch_tune_last,now);

		if(pitch_tune_delta)
		{
			time(&now);

			if ((now - pitch_tune_last) > pitch_tune_delta) {
			   bline_ret = output_blcmd(fdblcmd,"cmd", "pitch_tune");
			   pitch_tune_last = now;
			   usleep(500000);
			}
		}
		if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
		{
			switch_to_wait(mdccp, cf_ret);
			dc_abort = 0;
			break;
		}
		if(doing_dark_current == 0)
		  {
		    if(stat_axis == 1)
		        stat_phi += stat_osc_width;
		     else
		        stat_omega += stat_osc_width;
		  }

		dcop = DCOP_COLLECT;

		tick = stat_time * 1000;
		tick_set();
		units = tick;
		decrement = 1000 * (1. / dt_stat);
		dc_abort = 0;
		dc_in_progress = 1;

		break;
	  }
  }

/*
 *	This routine monitors the progress of commands issued by
 *	ccd_hw_start.  In general, most functions are taken care
 *	of by other processes; the only complex commands are scan
 *	and collect.
 */

extern	char	det_reply[2048];

int	ccd_hw_progress(mdccp)
mdc_command	*mdccp;
  {
	char	tempbuf[2048],tbuf[2048],u3dig[4];
	char	prefix_used[20],xform_out_name[256];
	int	done,detret;
	int	timediff,new_time;
	int	bline_ret,i;
	float	kappa_angs[3],euler_angs[3];
	char	*ztime();
	time_t	now;
	char	message[80];

	done = 0;

        if(mdccp->cmd_no != MDC_COM_STARTUP)
          {
            if(fdblcmd == -1)
	      {
                done = 1;
                return(done);
	      }
          }

	switch(mdccp->cmd_no)
	  {
	    case MDC_COM_STARTUP:
	    case MDC_COM_INIT:
	    case MDC_COM_ERASE:
	    case MDC_COM_DSET:
	    case MDC_COM_PSET:
	    case MDC_COM_LSET:
	    case MDC_COM_WSET:
	    case MDC_COM_CONFIG:
	    case MDC_COM_SHUT:
	    case MDC_COM_DMOVE:
	    case MDC_COM_PMOVE:
	    case MDC_COM_PMOVEREL:
	    case MDC_COM_LMOVE:
	    case MDC_COM_WMOVE:
	    case MDC_COM_OMOVE:
	    case MDC_COM_OMOVEREL:
	    case MDC_COM_OSET:
	    case MDC_COM_KMOVE:
	    case MDC_COM_KSET:
	    case MDC_COM_GONMAN:
	    case MDC_COM_HOME:
	    case MDC_COM_AMOVE:
	    case MDC_COM_AUTOALIGN:
            case MDC_COM_XL_HS_MOVE:
            case MDC_COM_XL_VS_MOVE:
	    case MDC_COM_XL_GUARD_HS_MOVE:
	    case MDC_COM_XL_GUARD_VS_MOVE:
            case MDC_COM_XL_UP_HHS_MOVE:
            case MDC_COM_XL_UP_VHS_MOVE:
            case MDC_COM_XL_DN_HHS_MOVE:
            case MDC_COM_XL_DN_VHS_MOVE:
            case MDC_COM_HSLIT:
            case MDC_COM_VSLIT:
	    case MDC_COM_EXPERIMENT_MODE_MOVE:
	    case MDC_COM_SET_MASTER:


		done = 1;
		break;

	    case MDC_COM_HOLDING:

		holding_over();

		done = 0;
		break;

	    case MDC_COM_SCAN:
	    case MDC_COM_COLL:
	    case MDC_COM_SNAP:
                if(fddetcmd == -1 || fdxfcm == -1)
                  {
                    done = 1;
                    return(done);
                  }
		if(dc_abort)
		  {
		    done = 1;
		    dc_abort = 0;
		    break;
		  }

		switch(cf_ret)
		{
			case CF_BL_READY:
				detret = output_detcmd(fddetcmd,"stop\n",NULL,0);
				break;
			case CF_BL_NOTREADY_BEFORE:
				if(ccd_modular)
				{
					output_detcmd(fddetcmd, "reset\n", NULL, 0);
					detret = CCD_DET_RETRY;
				}
				else
				{
					if(0 == doing_dark_current)
						detret = CCD_DET_RETRY;
				}
				break;
			case CF_BL_NOTREADY_AFTER:
				detret = output_detcmd(fddetcmd,"stop\n",NULL,0);
				if(ccd_modular)
				{
					output_detcmd(fddetcmd, "reset\n", NULL, 0);
					detret = CCD_DET_RETRY;
				}
				else
				{
					if(0 == doing_dark_current)
					{
						send_copy_command();
						detret = CCD_DET_RETRY;
					}
				}
				break;
		}

		if(strip_ave)
		  {
			output_detcmd(fddetcmd,"getparam\nstrip_ave\n",NULL,0);
			if(NULL == (char *) strstr(det_reply,"OK"))
			    n_strip_ave = 0;
			  else
			    {
				n_strip_ave = sscanf(&det_reply[3],"%f %f %f %f",
					&strip_ave_vals[0],&strip_ave_vals[1],
					&strip_ave_vals[2],&strip_ave_vals[3]);
			    }
			fprintf(stdout,"output_detcmd: STRIPS        at %s: ",ztime());
			for(i = 0; i < n_strip_ave; i++)
			  fprintf(stdout,"%.3f ",strip_ave_vals[i]);
			fprintf(stdout,"\n");
			fflush(stdout);
		  }

		if(detret == CCD_DET_RETRY)
		  {
                    if (dc_stop == 1) {
		      done = 1;
		      dc_stop = 0;
		      break;
                    }
		    if(doing_dark_current == 0)
		      {
		        if(stat_axis == 1)
		          sprintf(tempbuf,"phi_move %10.3f",phi_value_saved);
		        else
		          sprintf(tempbuf,"omega_move %10.3f",omega_value_saved);
		        output_blcmd(fdblcmd,"cmd",tempbuf);
		      }
			if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
			{
				switch_to_wait(mdccp, cf_ret);
				break;
			}
		    if(doing_dark_current == 0)
		      {
		        if(stat_axis == 1)
		            stat_phi += stat_osc_width;
		         else
		            stat_omega += stat_osc_width;
		      }
		    break;
		  }
                else if (detret == CCD_DET_NOTCONNECTED) {
                  dc_stop = 1;
		  sprintf(message," ccd_hw_progress");
		  issue_signal("CCD_SIG_DET_DROPPED",message);
                  set_alert_msg("ERROR: Detector control not connected.");
                }
		else if (detret == CCD_DET_FATAL){
		  dc_stop = 1;
		}

		send_copy_command();

	        if(gl_blret == CCD_BL_RETRY)
		  {
                    if (dc_stop == 1) 
		      {
		        done = 1;
		        dc_stop = 0;
		        break;
                      }

		    /*
		     *	This logic assumes that the goniostat device (Mar 300 base comes to mind)
		     *	is actually "continuing" the exposure through the requested time interval.
		     *	Hence we need to wait for the "rest" of the exposure to finish before actually
		     *	continuing with the steps below.
		     */

	            strcpy(stat_scanner_op,"Idling_rest_of_short_exp");

#ifdef X8C
		    short_exposure_stall(exposure_short_by,fdblcmd);
#endif /* X8C */

		    if(stat_axis == 1)
		      sprintf(tempbuf,"phi_move %10.3f",phi_value_saved);
		     else
		      sprintf(tempbuf,"omega_move %10.3f",omega_value_saved);
		    output_blcmd(fdblcmd,"cmd",tempbuf);
			if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
			{
				switch_to_wait(mdccp, cf_ret);
				break;
			}
		    if(stat_axis == 1)
		        stat_phi += stat_osc_width;
		     else
		        stat_omega += stat_osc_width;
		    break;
		  }
		if(dc_stop == 1)
		  {
		    output_detcmd(fddetcmd,"flush\n",NULL,0);
		    done = 1;
		    dc_stop = 0;
		    fprintf(stdout,"ccd_dc_api   : STOP: User requested STOP.  Exposures terminating.\n");
		    if(doing_dark_current && double_exp_ctr == 2)
		    	current_darkcurrent_time = 0;
		    break;
		  }

		if(doing_dark_current)
		  {
		    double_exp_ctr--;
		    if(double_exp_ctr > 0)
		    {
			if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
			{
				switch_to_wait(mdccp, cf_ret);
				break;
			}
		    }
		     else if(doing_second_dk || stat_dzratio == 1.0)
		      {
			stat_time /= stat_dzratio;
			doing_dark_current = 0;
			if(double_exp_flag)
			    double_exp_ctr = 2;
			  else
			    double_exp_ctr = 1;
			if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
			{
				switch_to_wait(mdccp, cf_ret);
				break;
			}
		      }
		      else if(stat_dzratio != 1.0)
			{
			  stat_time *= stat_dzratio;
			  double_exp_ctr = 2;
			  doing_second_dk = 1;
			if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
			{
				switch_to_wait(mdccp, cf_ret);
				break;
			}
			}
		    break;
		  }

		if(mdccp->cmd_col_blcmd[0] != '\0')
		  {
		    bline_ret = output_blcmd(fdblcmd,"during",mdccp->cmd_col_blcmd);
		    switch (bline_ret) {
		      case CCD_BL_FATAL:
			dc_stop = 1;
		        break;
		      case CCD_BL_NOTCONNECTED:
		      case CCD_BL_DISCONNECTED:
			dc_stop = 1;
			sprintf(message," after scan");
			issue_signal("CCD_BL_DROPPED",message);
	                set_alert_msg("ERROR: Beamline control not connected.");
		  	break;
	              default:
		  	break;
		      }
		  }
		if(dc_stop == 1)
		  {
			dc_abort = 1;
			if(cf_ret != CF_BL_NOTREADY_BEFORE)
		        	output_detcmd(fddetcmd,"flush\n",NULL,0);
		        done = 1;
		    	dc_stop = 0;
			break;
		  }
		double_exp_ctr--;
		if(double_exp_ctr == 0)
		  {
		    stat_time = mdccp->cmd_col_time;
		    if(double_exp_flag == 1)
			double_exp_ctr = 2;
		     else
			double_exp_ctr = 1;
		    stat_image_number++;
		    if(stat_axis == 1)
		      {
		        phi_value_saved += mdccp->cmd_col_osc_width;
			if(constrain_phi != 180) {
			if(phi_value_saved >= 360.)
				phi_value_saved -= 360.;
			} else {
			if(phi_value_saved >= 180.)
				phi_value_saved -= 360.;
			}

		      }
		    else
		      {
		        omega_value_saved += mdccp->cmd_col_osc_width;
			if(constrain_omega != 180) {
			if(omega_value_saved >= 360.)
				omega_value_saved -= 360.;
			} else {
			if(omega_value_saved >= 180.)
				omega_value_saved -= 180.;
			}
		      }
		    if(repeat_dark_current)
		      {
			time((time_t *) &new_time);
			timediff = new_time - dark_current_time;
			if(timediff > dark_current_interval)
			  {
				doing_dark_current = 1;
				doing_second_dk = 0;
				double_exp_ctr = 2;
				dark_current_time = new_time;
			  }
		      }
		    get_next_image_from_runfile(mdccp);
		  }
		 else
		  {
		    stat_time = mdccp->cmd_col_time * stat_dzratio;
		    if(stat_axis == 1)
		      sprintf(tempbuf,"phi_move %10.3f",phi_value_saved);
		    else
		      sprintf(tempbuf,"omega_move %10.3f",omega_value_saved);
		    output_blcmd(fdblcmd,"cmd",tempbuf);
		  }

		if(totimg == 0 || dc_stop == 1)
		  {
			output_detcmd(fddetcmd,"flush\n",NULL,0);
			done = 1;
			dc_stop = 0;
			if(mdccp->cmd_col_blcmd[0] != '\0')
			  {
			    bline_ret = output_blcmd(fdblcmd,"after",mdccp->cmd_col_blcmd);
			    switch (bline_ret) {
			      case CCD_BL_FATAL:
				dc_stop = 1;
			        break;
			      case CCD_BL_NOTCONNECTED:
			      case CCD_BL_DISCONNECTED:
				dc_stop = 1;
				sprintf(message," ccd_dc_hw 3");
				issue_signal("CCD_SIG_BL_DROPPED",message);
		                set_alert_msg("ERROR: Beamline control not connected.");
			  	break;
		              default:
			  	break;
			      }
			  }
			if(dc_stop == 1)
			  {
			        done = 1;
			    	dc_stop = 0;
				break;
			  }
			break;
		  }

		if(pitch_tune_delta)
		{
			time(&now);

			if ((now - pitch_tune_last) > pitch_tune_delta) {
			   bline_ret = output_blcmd(fdblcmd,"cmd", "pitch_tune");
			   pitch_tune_last = now;
			   usleep(500000);
			}
		}
		if(CF_BL_READY != (cf_ret = collect_frame(mdccp)))
		{
			switch_to_wait(mdccp, cf_ret);
			break;
		}
		if(doing_dark_current == 0)
		  {
		    if(stat_axis == 1)
		        stat_phi += stat_osc_width;
		     else
		        stat_omega += stat_osc_width;
		  }

		tick_set();
		tick = stat_time * 1000;
		units = tick;
		decrement = 1000 * (1. / dt_stat);
		totpass = stat_n_passes;

		break;
	  }

	if(done == 1)
	  {
		strcpy(stat_scanner_msg,"");
		strcpy(stat_scanner_op,"");
		strcpy(stat_scanner_control,"idle");
		dc_in_progress = 0;
		if(pf_mod && (mdccp->cmd_no == MDC_COM_COLL || (mdccp->cmd_no == MDC_COM_SNAP)))
		{
		}
	  }
	return (done);
  }

ccd_hw_initial_status()
  {
	stat_start_phi = 0.;
	stat_start_omega = 0.;
	stat_start_kappa = 0.;
	stat_axis = 1;
	stat_osc_width = 1.0;
	stat_n_images = 1;
	stat_n_passes = 1;
	stat_n_ccd_updates = 0;
	stat_time = 30;
	stat_dir[0] = '\0';
	stat_fname[0] = '\0';
	strcpy(stat_scanner_op,"none");
	stat_scanner_msg[0] = '\0';
	strcpy(stat_scanner_control,"idle");
	strcpy(stat_scanner_shutter,"closed");
  }
