#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include "adx.h"

#if defined(__SVR4) || defined(SYSTYPE_SVR4)
#include <sys/statvfs.h>
#else
#if defined (sun) || defined(__linux__)
#include <sys/vfs.h>
#endif
#ifdef Alpha
#include <sys/statvfs.h>
#endif
#ifdef sgi
#include <sys/statfs.h>
#endif
#endif /* SVR4 */
#include "creation-c.h"

static int anomalous, anomalous_wedge;
double get_dzratio();

#ifdef hp
#include <sys/vfs.h>
#endif /* HPUX */

#define   EV_ANGSTROM     (12398.4243)

/* Return Free Disk space (in megaytes) of directory.
 */
disk_free(directory)
char *directory;
{
	int n_meg_free;
#if defined(__SVR4) || defined(SYSTYPE_SVR4) || defined(Alpha)
	struct statvfs buf;
	
	if (statvfs (directory, &buf) == -1) {
		return -1;
	}

	if (debug) {
		fprintf(stderr,"Called disk_free(%s)...\n",directory); 
		fprintf(stderr,"Block Size:  Preferred: %d Fundamental %d\n",
			buf.f_frsize,buf.f_bsize);
		fprintf(stderr,"Free Blocks: Total: %d Available: %d\n",
			buf.f_bfree,buf.f_bavail);
	}

	n_meg_free = (int)((double)(buf.f_bavail)*(buf.f_frsize)/(1000.0*1024.0));
#else
	struct statfs buf;
	
#ifdef __linux__
	if (statfs (directory, &buf) == -1)
#else
#ifdef hp
	if (statfs (directory, &buf) == -1)
#else
	if (statfs (directory, &buf, sizeof(struct statfs), 0) == -1)
#endif /*  hp       */
#endif /* __linux__ */
		return -1;

	if (debug) {
		fprintf(stderr,"Called disk_free \"%s\"...\n",directory); 
		fprintf(stderr,"Block Size:  %d\n", buf.f_bsize);
#ifdef sgi
		fprintf(stderr,"Free Blocks: Total: %d Available: %d\n",
			buf.f_blocks,buf.f_bfree);
#else
		fprintf(stderr,"Free Blocks: Total: %d Available: %d\n",
			buf.f_bfree,buf.f_bavail);
#endif /* sgi */
	}

#ifdef sgi
	n_meg_free = (int)((double)(buf.f_bfree)*(buf.f_bsize)/(1024.0*1024.0));
#else
	n_meg_free = (int)((double)(buf.f_bavail)*(buf.f_bsize)/(1024.0*1024.0));
#endif /* sgi */
#endif /* SVR4 */

	return(n_meg_free);
}

TextFieldSetInt(w,value)
Widget w;
int value;
{
	char str[32];

	sprintf(str,"%d", value);
	XmTextFieldSetString(w,str);

	XmTextSetInsertionEnd(w);
}

TextFieldSetFloat(w,value)
Widget w;
double value;
{
	char str[32];

	sprintf(str,"%1.2f", value);
	XmTextFieldSetString(w,str);

	XmTextSetInsertionEnd(w);
}

TextFieldSetFloat4(w,value)
Widget w;
double value;
{
	char str[32];

	sprintf(str,"%1.4f", value);
	XmTextFieldSetString(w,str);

	XmTextSetInsertionEnd(w);
}

TextFieldSetFloat6(w,value)
Widget w;
double value;
{
	char str[32];

	sprintf(str,"%1.6f", value);
	XmTextFieldSetString(w,str);

	XmTextSetInsertionEnd(w);
}

TextFieldGetInt(w)
Widget w;
{
	char *str;
	int value=0;

	str = XmTextFieldGetString(w);
	sscanf(str,"%d",&value);
	XtFree(str);
	return(value);
}

double
TextFieldGetFloat(w)
Widget w;
{
	char *str;
	double value=0;

	str = XmTextFieldGetString(w);
	sscanf(str,"%lf",&value);
	XtFree(str);
	return(value);
}


/* Save contents of XmText widget, w, to a file.
 */
save_XmText(w, filename)
Widget w;
char *filename;
{
	FILE *fp;
	char *string = (char *)XmTextGetString(w);
	char *directory, *prefix, *comment;
	char *x_beam, *y_beam;
	char *wedge, *step_size;
	double wave;
	double energy_to_wavelength(), wavelength_to_energy();

	if ((fp = fopen(filename,"w")) == NULL) {
		sprintf(error_msg,"can not write file: %s",filename);
		emess(error_msg);
		return(-1);
	}

	directory = XmTextFieldGetString(strategy_directory_textField);
	prefix =    XmTextFieldGetString(strategy_image_prefix_textField);
	comment =   XmTextFieldGetString(strategy_comment_textField);
	x_beam =   XmTextFieldGetString(strategy_beamx_textField);
	y_beam =   XmTextFieldGetString(strategy_beamy_textField);
	wedge  =   XmTextFieldGetString(strategy_wedge_textField);
	step_size  =   XmTextFieldGetString(options_deg_dose_textField);

	fprintf(fp,"Directory: %s\n",directory);
	fprintf(fp,"Image_Prefix: %s\n",prefix);

	/* Mode */
	if (XmToggleButtonGetState(strategy_time_mode_toggleButton) == True)
		fprintf(fp,"Mode: Time\n");
	else
	if (XmToggleButtonGetState(strategy_dose_mode_toggleButton) == True) {
		fprintf(fp,"Mode: Dose\n");
		fprintf(fp,"Dose Step: %s\n",step_size);
	}
	else {
		fflush(stderr);
		fprintf(fp,"Mode: ????\n");
	}

	/* ADC */
	if (XmToggleButtonGetState(strategy_fast_toggleButton) == True)
		fprintf(fp,"ADC: Fast\n");
	else
	if (XmToggleButtonGetState(strategy_slow_toggleButton) == True)
		fprintf(fp,"ADC: Slow\n");
	else {
		fprintf(fp,"ADC: ????\n");
	}

	/* Anomalous */
	if (XmToggleButtonGetState(strategy_anomyes_toggleButton) == True)
		fprintf(fp,"Anomalous: Yes\n");
	else
	if (XmToggleButtonGetState(strategy_anomno_toggleButton) == True)
		fprintf(fp,"Anomalous: No\n");
	else {
		fprintf(fp,"Anomalous: ????\n");
	}
	fprintf(fp,"Anom_Wedge: %s\n",wedge);

	/* Compression */
	if (XmToggleButtonGetState(strategy_comp_none_toggleButton) == True)
		fprintf(fp,"Compression: None\n");
	else
	if (XmToggleButtonGetState(strategy_comp_Z_toggleButton) == True)
		fprintf(fp,"Compression: .Z\n");
	else
	if (XmToggleButtonGetState(strategy_comp_pck_toggleButton) == True)
		fprintf(fp,"Compression: .pck\n");
	else {
		fprintf(fp,"Compression: ????\n");
	}

	/* Binning */
	if (XmToggleButtonGetState(strategy_bin1_toggleButton) == True)
		fprintf(fp,"Binning: None\n");
	else
	if (XmToggleButtonGetState(strategy_bin2_toggleButton) == True)
		fprintf(fp,"Binning: 2x2\n");
	else {
		fprintf(fp,"Binning: ????\n");
	}
	fprintf(fp,"Comment: %s\n",comment);
	fprintf(fp,"Beam_Center: %s %s\n",x_beam, y_beam);

	/* Mad Info */


		fprintf(fp, "\nMAD:\n");
		fprintf(fp, "Energy to Use:\n");

		if (XmToggleButtonGetState(enable_wavelength1_toggleButton) == True)
			fprintf(fp," X ");
		wave = TextFieldGetFloat(energy1_textField);
		if (wave < 0.0) wave = 0.0;
		fprintf(fp,"	%10.5f eV ( %1.6f A )\n",wave,energy_to_wavelength(wave));

		if (XmToggleButtonGetState(enable_wavelength2_toggleButton) == True)
			fprintf(fp," X");
		wave = TextFieldGetFloat(energy2_textField);
		if (wave < 0.0) wave = 0.0;
		fprintf(fp,"	%10.5f eV ( %1.6f A )\n",wave,energy_to_wavelength(wave));

		if (XmToggleButtonGetState(enable_wavelength3_toggleButton) == True)
			fprintf(fp," X");
		wave = TextFieldGetFloat(energy3_textField);
		if (wave < 0.0) wave = 0.0;
		fprintf(fp,"	%10.5f eV ( %1.6f A )\n",wave,energy_to_wavelength(wave));

		if (XmToggleButtonGetState(enable_wavelength4_toggleButton) == True)
			fprintf(fp," X");
		wave = TextFieldGetFloat(energy4_textField);
		if (wave < 0.0) wave = 0.0;
		fprintf(fp,"	%10.5f eV ( %1.6f A )\n",wave,energy_to_wavelength(wave));

		if (XmToggleButtonGetState(enable_wavelength5_toggleButton) == True)
			fprintf(fp," X");
		wave = TextFieldGetFloat(energy5_textField);
		if (wave < 0.0) wave = 0.0;
		fprintf(fp,"	%10.5f eV ( %1.6f A )\n",wave,energy_to_wavelength(wave));

		if (XmToggleButtonGetState(mad_option2_toggleButton) == True) {
			/* Change Energy Each Run */
			fprintf(fp,"Change Energy Each Run\n");
		}
		else
		if (XmToggleButtonGetState(mad_option3_toggleButton) == True) {
			/* Change Energy Each Anomalous Wedge */
			fprintf(fp,"Change Energy Each Anomalous Wedge\n");
		}
		else
		if (XmToggleButtonGetState(mad_option4_toggleButton) == True) {
			/* Change Energy Each N frames */
			int nf;
			nf = TextFieldGetInt(mad_nframes_textField);
			if (nf < 0 || nf > 10000) {
				fprintf(stderr,"warning: N frames = %d, set to %d\n",nf,0);
				nf = 0;
			}
			fprintf(fp,"Change Energy Every %d Frames\n",nf);
		}
		else
			fprintf(fp,"Change Energy Never\n");


	fprintf(fp,"\nRun(s):\n\n");

	/*fprintf(fp,"%s",string);*/
	fputs(string, fp);
	fclose(fp);

	XtFree(string);
	XtFree(directory);
	XtFree(wedge);
	XtFree(step_size);
	XtFree(prefix);
	XtFree(comment);
	XtFree(x_beam);
	XtFree(y_beam);

	return(0);
}


/* Load contents of XmText widget, w, from a file.
 */
load_XmText(w, filename)
Widget w;
char *filename;
{
	FILE *fp;
	char buf[BUFSIZ];
	char key[256], value[256];
	int i, pos;
	double energy_to_wavelength();

	if ((fp = fopen(filename,"r")) == NULL) {
		sprintf(error_msg,"can not open file: %s",filename);
		emess(error_msg);
		return(-1);
	}

	/* Clear existing text... */
	XmTextReplace(w, 0, XmTextGetLastPosition(w), "");

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		strcpy(key,"");
		strcpy(value,"");
		/*sscanf(buf,"%s%s",key,value);*/
		sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
		if(!strncmp(key,"Directory:",strlen("Directory:"))){
			XmTextFieldSetString(strategy_directory_textField,value);
		}
		else
		if(!strncmp(key,"Image_Prefix:",strlen("Image_Prefix:"))){
			XmTextFieldSetString(strategy_image_prefix_textField,value);
		}
		else
		if(!strncmp(key,"Anom_Wedge:",strlen("Anom_Wedge:"))){
			XmTextFieldSetString(strategy_wedge_textField,value);
		}
		else
		if(!strncmp(key,"Beam_Center:",strlen("Beam_Center:"))){
			double x_beam, y_beam;
			sscanf(value,"%lf%lf",&x_beam,&y_beam);
			TextFieldSetFloat(strategy_beamx_textField,x_beam);
			TextFieldSetFloat(strategy_beamy_textField,y_beam);
		}
		else
		if(!strncmp(key,"Mode:",strlen("Mode:"))){
			if (!strncmp(value,"Time",strlen("Time"))){
				XmToggleButtonSetState(strategy_time_mode_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_dose_mode_toggleButton,
					False,False);
			}
			else
			if (!strncmp(value,"Dose",strlen("Dose"))){
				XmToggleButtonSetState(strategy_time_mode_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_dose_mode_toggleButton,
					True,False);
			}
			else {
				XmToggleButtonSetState(strategy_time_mode_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_dose_mode_toggleButton,
					False,False);
			}
			
		}
		else
		if(!strncmp(key,"ADC:",strlen("ADC:"))){
			if (!strncmp(value,"Slow",strlen("Slow"))){
				XmToggleButtonSetState(strategy_slow_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_fast_toggleButton,
					False,False);
			}
			else
			if (!strncmp(value,"Fast",strlen("Fast"))){
				XmToggleButtonSetState(strategy_slow_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_fast_toggleButton,
					True,False);
			}
			else {
				XmToggleButtonSetState(strategy_slow_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_fast_toggleButton,
					False,False);
			}
			
		}
		else
		if(!strncmp(key,"Anomalous:",strlen("Anomalous:"))){
			if (!strncmp(value,"Yes",strlen("Yes"))){
				XmToggleButtonSetState(strategy_anomyes_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_anomno_toggleButton,
					False,False);
    				XtSetSensitive(label18,True);
    				XtSetSensitive(label29,True);
    				XtSetSensitive(strategy_wedge_textField,True);
    				XtSetSensitive(label97,True);
    				XtSetSensitive(mad_option3_toggleButton,True);
			}
			else
			if (!strncmp(value,"No",strlen("No"))){
				XmToggleButtonSetState(strategy_anomyes_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_anomno_toggleButton,
					True,False);
    				XtSetSensitive(label97,False);
    				XtSetSensitive(mad_option3_toggleButton,False);
			}
			else {
				XmToggleButtonSetState(strategy_anomno_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_anomyes_toggleButton,
					False,False);
    				XtSetSensitive(label97,False);
    				XtSetSensitive(mad_option3_toggleButton,False);
			}
			
		}
		else
		if(!strncmp(key,"Binning:",strlen("Binning:"))){
			if (!strncmp(value,"None",strlen("None"))){
				XmToggleButtonSetState(strategy_bin1_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_bin2_toggleButton,
					False,False);
			}
			else
			if (!strncmp(value,"2x2",strlen("2x2"))){
				XmToggleButtonSetState(strategy_bin1_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_bin2_toggleButton,
					True,False);
			}
			else {
				XmToggleButtonSetState(strategy_bin1_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_bin2_toggleButton,
					False,False);
			}
			
		}
		else
		if(!strncmp(key,"Compression:",strlen("Compression:"))){
			if (!strncmp(value,"None",strlen("None"))){
				XmToggleButtonSetState(strategy_comp_none_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_comp_pck_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_comp_Z_toggleButton,
					False,False);
			}
			else
			if (!strncmp(value,".pck",strlen(".pck"))){
				XmToggleButtonSetState(strategy_comp_none_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_comp_pck_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_comp_Z_toggleButton,
					False,False);
			}
			else
			if (!strncmp(value,".Z",strlen(".Z"))){
				XmToggleButtonSetState(strategy_comp_none_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_comp_pck_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_comp_Z_toggleButton,
					True,False);
			}
			else {
				XmToggleButtonSetState(strategy_comp_none_toggleButton,
					True,False);
				XmToggleButtonSetState(strategy_comp_pck_toggleButton,
					False,False);
				XmToggleButtonSetState(strategy_comp_Z_toggleButton,
					False,False);
			}
			
		}
		else
		if(!strncmp(key,"Comment:",strlen("Comment:"))){
			if (strlen(buf) > 1) {
				if (buf[strlen(buf)-1] == '\n')
					buf[strlen(buf)-1] = 0;
			}
			XmTextFieldSetString(strategy_comment_textField,buf+strlen("Comment: "));
		}
		else
		if(!strncmp(key,"MAD:",strlen("MAD:"))){
			double energy, wavelength;
			fgets(buf, sizeof(buf), fp); /* skip line */

			/* 5 times */
			fgets(buf, sizeof(buf), fp); 
			strcpy(key,"");
			strcpy(value,"");
			sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
			remove_white(key);
			if (key[0] == 'X') {
				XmToggleButtonSetState(enable_wavelength1_toggleButton,True,False);
				XtSetSensitive(wavelength1_textField,True);
				XtSetSensitive(energy1_textField,True);
				energy = atof(value);
			}
			else {
				XmToggleButtonSetState(enable_wavelength1_toggleButton,False,False);
				XtSetSensitive(wavelength1_textField,False);
				XtSetSensitive(energy1_textField,False);
				energy = atof(key);
			}
			if (energy < 0.0) energy = 0.0;
			TextFieldSetFloat(energy1_textField,energy);
			wavelength = energy_to_wavelength(energy);
			TextFieldSetFloat6(wavelength1_textField,wavelength);

			fgets(buf, sizeof(buf), fp); 
			strcpy(key,"");
			strcpy(value,"");
			sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
			remove_white(key);
			if (key[0] == 'X') {
				XmToggleButtonSetState(enable_wavelength2_toggleButton,True,False);
				XtSetSensitive(wavelength2_textField,True);
				XtSetSensitive(energy2_textField,True);
				energy = atof(value);
			}
			else {
				XmToggleButtonSetState(enable_wavelength2_toggleButton,False,False);
				XtSetSensitive(wavelength2_textField,False);
				XtSetSensitive(energy2_textField,False);
				energy = atof(key);
			}
			if (energy < 0.0) energy = 0.0;
			TextFieldSetFloat(energy2_textField,energy);
			wavelength = energy_to_wavelength(energy);
			TextFieldSetFloat6(wavelength2_textField,wavelength);

			fgets(buf, sizeof(buf), fp); 
			strcpy(key,"");
			strcpy(value,"");
			sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
			remove_white(key);
			if (key[0] == 'X') {
				XmToggleButtonSetState(enable_wavelength3_toggleButton,True,False);
				XtSetSensitive(wavelength3_textField,True);
				XtSetSensitive(energy3_textField,True);
				energy = atof(value);
			}
			else {
				XmToggleButtonSetState(enable_wavelength3_toggleButton,False,False);
				XtSetSensitive(wavelength3_textField,False);
				XtSetSensitive(energy3_textField,False);
				energy = atof(key);
			}
			if (energy < 0.0) energy = 0.0;
			TextFieldSetFloat(energy3_textField,energy);
			wavelength = energy_to_wavelength(energy);
			TextFieldSetFloat6(wavelength3_textField,wavelength);

			fgets(buf, sizeof(buf), fp); 
			strcpy(key,"");
			strcpy(value,"");
			sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
			remove_white(key);
			if (key[0] == 'X') {
				XmToggleButtonSetState(enable_wavelength4_toggleButton,True,False);
				XtSetSensitive(wavelength4_textField,True);
				XtSetSensitive(energy4_textField,True);
				energy = atof(value);
			}
			else {
				XmToggleButtonSetState(enable_wavelength4_toggleButton,False,False);
				XtSetSensitive(wavelength4_textField,False);
				XtSetSensitive(energy4_textField,False);
				energy = atof(key);
			}
			if (energy < 0.0) energy = 0.0;
			TextFieldSetFloat(energy4_textField,energy);
			wavelength = energy_to_wavelength(energy);
			TextFieldSetFloat6(wavelength4_textField,wavelength);

			fgets(buf, sizeof(buf), fp); 
			strcpy(key,"");
			strcpy(value,"");
			sscanf(buf,"%s%*[ \t]%[^\n]",key,value);
			remove_white(key);
			if (key[0] == 'X') {
				XmToggleButtonSetState(enable_wavelength5_toggleButton,True,False);
				XtSetSensitive(wavelength5_textField,True);
				XtSetSensitive(energy5_textField,True);
				energy = atof(value);
			}
			else {
				XmToggleButtonSetState(enable_wavelength5_toggleButton,False,False);
				XtSetSensitive(wavelength5_textField,False);
				XtSetSensitive(energy5_textField,False);
				energy = atof(key);
			}
			if (energy < 0.0) energy = 0.0;
			TextFieldSetFloat(energy5_textField,energy);
			wavelength = energy_to_wavelength(energy);
			TextFieldSetFloat6(wavelength5_textField,wavelength);

			/* Change Energy Never
			 */
			XmToggleButtonSetState(mad_option1_toggleButton,True,False);
			XmToggleButtonSetState(mad_option2_toggleButton,False,False);
			XmToggleButtonSetState(mad_option3_toggleButton,False,False);
			XmToggleButtonSetState(mad_option4_toggleButton,False,False);

			fgets(buf, sizeof(buf), fp); 
			if (!strncmp(buf,"Change Energy Never",
			     strlen("Change Energy Never"))) {
				XmToggleButtonSetState(mad_option1_toggleButton,True,False);
			}
			else
			if (!strncmp(buf,"Change Energy Each Run",
			     strlen("Change Energy Each Run"))) {
				XmToggleButtonSetState(mad_option2_toggleButton,True,False);
				XmToggleButtonSetState(mad_option1_toggleButton,False,False);
			}
			else
			if (!strncmp(buf,"Change Energy Each Anomalous Wedge",
			     strlen("Change Energy Each Anomalous Wedge"))) {
				XmToggleButtonSetState(mad_option3_toggleButton,True,False);
				XmToggleButtonSetState(mad_option1_toggleButton,False,False);
			}
			else
			if (!strncmp(buf,"Change Energy Every",
			     strlen("Change Energy Every"))) {
			     	int nf=10;
				XmToggleButtonSetState(mad_option4_toggleButton,True,False);
				XmToggleButtonSetState(mad_option1_toggleButton,False,False);
				nf = atoi(buf+strlen("Change Energy Every "));
				TextFieldSetInt(mad_nframes_textField,nf);
			}
		}
		else
		if(!strncmp(key,"Run(s):",strlen("Run(s):"))){
			break;
		}
		else {
		}
	}
	XmTextReplace(w, 0, XmTextGetLastPosition(w), "");

	/*
	for (pos = 0; fgets(buf, sizeof(buf), fp); ) {
		if (!iswhite(buf)) {
			XmTextReplace(w, pos, pos, buf);
			pos += strlen(buf);
		}
	}
	*/
	for (pos = 0; fgets(buf, sizeof(buf), fp); ) {
		if (iswhite(buf))
			continue;
		
		if (strlen(buf) > (fld[n_fields-1].col_end+1)) {
			buf[fld[n_fields-1].col_end+1] = '\n';
			buf[fld[n_fields-1].col_end+2] = 0;
		}

		if (beamline_mode==True) {
			if (strlen(buf) < fld[12].col_start) {
				buf[strlen(buf)-1] = ' ';
				for(i=strlen(buf); i<=fld[12].col_end;i++)
					buf[i] = ' ';
				buf[i] = '\n';
				buf[i+1] = 0;
			}
		}
		else {
			if (strlen(buf) <= fld[11].col_end) {
				buf[strlen(buf)-1] = ' ';
				for(i=strlen(buf); i<=fld[11].col_end;i++)
					buf[i] = ' ';
				buf[i] = '\n';
				buf[i+1] = 0;
			}
		}
		if (!iswhite(buf)) {
			XmTextReplace(w, pos, pos, buf);
			pos += strlen(buf);
		}
	}
	XmTextReplace(w, pos, pos, "   ");

	{
		int line, last_line;

		last_line = find_row(w, pos);
		mvc_return=1;
		for(line=0;line<last_line;line++)
		for(i=0;i<n_fields;i++)
			clean_up(w, line, i);
		mvc_return=0;
		XmTextSetInsertionPosition(w, fld[0].col_end);
	}
	
	fclose(fp);
	return(0);
}

#ifdef NOT_USED

#define SMALL  (0)
#define MEDIUM (1)
#define LARGE  (2)

resize_main_window_fonts()
{
	int i;
	static int size=0;
	XmString xmstr;
	static XmFontList font_list;
	XmFontListEntry font_list_entry1, font_list_entry2, font_list_entry3;
	XFontStruct   *small_font, *medium_font, *large_font;
	String str1 = "SETUP";

	static int init=0;
	Arg args[2];

	if (!init) {
		init=1;
		/*
		small_font  = XLoadQueryFont(XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-80-75-75-*-*-iso8859-1");
		medium_font = XLoadQueryFont(XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1");
		large_font  = XLoadQueryFont(XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-240-75-75-*-*-iso8859-1");
		*/

		font_list_entry1 = XmFontListEntryLoad (XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-80-75-75-*-*-iso8859-1",
			XmFONT_IS_FONT, "SMALL_FONT");
		font_list = XmFontListAppendEntry (NULL, font_list_entry1);
		XmFontListEntryFree(&font_list_entry1); /* SEGV */

		font_list_entry2 = XmFontListEntryLoad (XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
			XmFONT_IS_FONT, "MEDIUM_FONT");
		font_list = XmFontListAppendEntry (font_list, font_list_entry2);
		XmFontListEntryFree(&font_list_entry2); /* SEGV */

		font_list_entry3 = XmFontListEntryLoad (XtDisplay(bulletinBoard),
			"-*-helvetica-bold-r-*-*-*-240-75-75-*-*-iso8859-1",
			XmFONT_IS_FONT, "LARGE_FONT");
		font_list = XmFontListAppendEntry (font_list, font_list_entry3);
		XmFontListEntryFree(&font_list_entry3); /* SEGV */
	}

	switch(size) {

		case SMALL:

			xmstr = XmStringCreate(str1, "SMALL_FONT");
			XtSetArg(args[0], XmNfontList, font_list);
			XtSetArg(args[1], XmNlabelString, xmstr);
			XtSetValues(setupButton,args,2);
        		/*XtVaSetValues(setupButton, XmNlabelString, xmstr, NULL);*/
    			XmStringFree(xmstr);
		break;
		case MEDIUM:
			xmstr = XmStringCreate(str1, "MEDIUM_FONT");
			XtSetArg(args[0], XmNfontList, font_list);
			XtSetArg(args[1], XmNlabelString, xmstr);
			XtSetValues(setupButton,args,2);
    			XmStringFree(xmstr);

		break;
		case LARGE:
			xmstr = XmStringCreate(str1, "LARGE_FONT");
			XtSetArg(args[0], XmNfontList, font_list);
			XtSetArg(args[1], XmNlabelString, xmstr);
			XtSetValues(setupButton,args,2);
    			XmStringFree(xmstr);

		break;
		default:
		break;
	}

	size = ++size %3;
}
#endif /* NOT_USED */

/* Return 1 if s1 consists only of white-space characters.
 */

iswhite(s1)
char *s1;
{
	if (!s1) {
		fprintf(stderr,"Error: s1 = NULL.\n");
		fflush(stderr);
		return(0);
	}

	while (*s1) {
		if (!isspace(*s1++))
			return(0);
	} 
	return(1);
}

#ifdef NOT_USED
int
wtoi(w)
Widget w;
{
	char *string;
	int val=0, atoi();

	string = XmTextFieldGetString(w);
	val = atoi(string);
	XtFree(string);
	return(val);
}
#endif /* NOT_USED */

double
wtof(w)
Widget w;
{
	char *string;
	double val=0;

	string = XmTextFieldGetString(w);
	val = atof(string);
	XtFree(string);
	return(val);
}

/* Read a line (terminated by a newline).
 * If not all white space then add to the command buffer.
 * Continue for all lines in string (tstr);
 */
collect_runs(tstr)
char *tstr;	/* Run Text Window String */
{
	int i, run, start, nframes, axis, retval;
	double phi, kappa, omega, delta_phi, distance, offset, exposure;
	char buf[4096], run_line[256], field[64], axis_str[32];
	FILE *flog; /* logfile */
	int runcount = 0;
	char *comment;	/* Comment string */
	char *centerx_str, *centery_str;
	char blstr[256];
	char dzngr[8];
	float center_x, center_y;
	double wave;
	int change_wavelength=0;
	char mad_str[256];

	strcpy(buf,"");
	strcpy(blstr,"");
	strcpy(mad_str,"");

	if (tstr == NULL) {
		fprintf(stderr,"ERROR: tstr = NULL\n"); fflush(stderr);
		return;
	}

	for(i=0; i <= MAX_RUNS; i++) {
		Run[i].nframes = 0;
	}

	Collect.mode = image_collect_mode;
        if (Collect.mode == DOSE_MODE) {
	  char *step_size  =   XmTextFieldGetString(options_deg_dose_textField);
	  Collect.step_size = atof(step_size);
	  XtFree(step_size);
	}
        else
          Collect.step_size = 0.0;

	Collect.mode = image_collect_mode;
	Collect.adc = image_adc_mode;
	Collect.bin = image_bin_mode;
	Collect.no_transform = !xform_mode;
	Collect.output_raw = saveraw_mode;
	strcpy(Collect.directory,image_directory);
	strcpy(Collect.prefix,image_prefix);
	strcpy(Collect.suffix,image_suffix);
	wavelength = TextFieldGetFloat(wavelength_textField);
	Collect.wavelength = wavelength;

	/* Read all Runs into RunStruct */
	while ((i = get_line(tstr, run_line)) > 0) {

	tstr += i;
	if (iswhite(run_line))
		continue;

	if (debug) {
		fprintf(stderr,"run_line= \"%s\"\n",run_line);
		fflush(stderr);
	}

	strcpy(buf,run_line);
	buf[fld[11].col_end+1] = 0;
	if (sscanf(buf,"%d%d%d%lf%lf%lf%lf%lf%s%lf%lf%s",
		&run,&start,&nframes,&distance,&offset,
		&phi,&kappa,&omega,axis_str,&delta_phi,&exposure,dzngr) != 12) {

		sprintf(error_msg,"\nBad Run (Not enough Fields):\n\n%s\n\nAborting Collect.",run_line);
		post_error(error_msg);
		return;
	}

	sscanf(run_line,"%d%d%d%lf%lf%lf%lf%lf%s%lf%lf%s%[^\n]",
		&run,&start,&nframes,&distance,&offset,
		&phi,&kappa,&omega,axis_str,&delta_phi,&exposure,dzngr,blstr);

	remove_white(axis_str);
	if(sc_conf.usephi == 0 && 0 == strcmp("phi",axis_str)){
		sprintf(error_msg,"\nAxis (%s) is NOT enabled as a rotation axis.\n\nAborting Collect.",
			axis_str);
		post_error(error_msg);
		return;
	}
	if(sc_conf.useomega == 0 && 0 == strcmp("omega",axis_str)){
		sprintf(error_msg,"\nAxis (%s) is NOT enabled as a rotation axis.\n\nAborting Collect.",
			axis_str);
		post_error(error_msg);
		return;
	}
	remove_white(dzngr);

	if (run > MAX_RUNS) {
		sprintf(error_msg,"\nRun Number Too Large: %d (Max = %d)\n\nAborting Collect.",
			run,MAX_RUNS);
		post_error(error_msg);
		return;
	}
	if (run <= 0) {
		sprintf(error_msg,"\nBad Run Number: %d\n\nAborting Collect.",
			run);
		post_error(error_msg);
		return;
	}

	/* Check for duplicate run number's */
	for(i=0; i < runcount; i++) {
		if ((Run[i].number == run) && (Run[i].nframes != 0) ) {
		/* Check for overlapping image numbers */
			
		if ((Run[i].start == start) ||
		   ((Run[i].start > start) && (Run[i].start < start+nframes)) ||
		   ((Run[i].start < start) && (Run[i].start+Run[i].nframes > start)) ) {

			if ((Run[i].exposure != (int)Run[i].exposure) && ( Run[i].exposure <= 100))
			sprintf(tmpstr,"%3d    %03d    %4d    %6.2f    %6.2f  %6.2f  %6.2f  %6.2f %5s  %5.3f  %5.2f   %4s",
				Run[i].number,Run[i].start,Run[i].nframes,
				Run[i].distance,Run[i].offset,Run[i].phi,
				Run[i].kappa,Run[i].omega,Run[i].axis,
				Run[i].delta_phi,Run[i].exposure,Run[i].dzngr);
			else
			sprintf(tmpstr,"%3d    %03d    %4d    %6.2f    %6.2f  %6.2f  %6.2f  %6.2f %5s  %5.3f  %5.0f   %4s",
				Run[i].number,Run[i].start,Run[i].nframes,
				Run[i].distance,Run[i].offset,Run[i].phi,
				Run[i].kappa,Run[i].omega,Run[i].axis,
				Run[i].delta_phi,Run[i].exposure,Run[i].dzngr);

			run_line[fld[11].col_end+1] = 0;
			sprintf(error_msg,
				"\nImage Numbers in the following Runs Overlap:\n\n%s\n%s\n\nAborting Collect.",
					tmpstr,run_line);
			post_error(error_msg);
			return;
		}
		}
	}
	centerx_str = XmTextFieldGetString(strategy_beamx_textField);
	centery_str = XmTextFieldGetString(strategy_beamy_textField);
	if (iswhite(centerx_str) || iswhite(centery_str)) {
		sprintf(error_msg,
			"\nCoordinates of the Beam Center (in mm) MUST be entered.\n\nThese values may be obtained from adxv or denzo.\n\nAborting Collect.");
		post_error(error_msg);
		XtFree(centerx_str);
		XtFree(centery_str);
		return;
	}

	center_x = atof(centerx_str);
	center_y = atof(centery_str);

	XtFree(centerx_str);
	XtFree(centery_str);

	Run[runcount].number = run;
	Run[runcount].start = start;
	Run[runcount].nframes = nframes;
	Run[runcount].distance = distance;
	Run[runcount].offset = offset;
	Run[runcount].phi = phi;
	Run[runcount].kappa = kappa;
	Run[runcount].omega = omega;
	strcpy(Run[runcount].axis,axis_str);
	/*
	if (fabs(delta_phi) < 0.01)
		delta_phi = 0.01;
	 */
	Run[runcount].delta_phi = delta_phi;

	if (fabs(exposure) < 0.01)
		exposure = 0.01;
	Run[runcount].exposure = exposure;
	strcpy(Run[runcount].dzngr,dzngr);
	strcpy(Run[runcount].blstr, blstr);

	if ((retval=check_run(Run,runcount)) < 0) {
		sprintf(error_msg,"\nBad Run (Check Field %d):\n\n%s\n\nAborting Collect.",-1*retval,run_line);
		post_error(error_msg);
		return;
	}

	if (file_overwrite_mode != 0)
	if (check_files(image_directory, image_prefix, image_suffix,  Run, runcount) < 0) {
		return;
	}

	/*
	if ((read_field(run_line,field,0,FIELD_INT,&run)) == BAD_FIELD){
		fprintf(stderr,"Bad Field: %d (Run#) \"%s\" \n",
			0,field); fflush(stderr);
		fprintf(stderr,"Skipping this Run.\n"); fflush(stderr);
		continue;
	}

	if ((read_field(run_line,field,1,FIELD_INT,&start)) == BAD_FIELD){
		fprintf(stderr,"Bad Field: %d (Frame#) \"%s\" \n",
			0,field); fflush(stderr);
		fprintf(stderr,"Skipping this Run (%d).\n",run); fflush(stderr);
		continue;
	}
	*/

	runcount++;

	}

	if (XmToggleButtonGetState(strategy_anomyes_toggleButton) == True)
		anomalous = True; 
	else
	if (XmToggleButtonGetState(strategy_anomno_toggleButton) == True)
		anomalous = False; 
	else
		anomalous = False;

	{
		char *wedge  =   XmTextFieldGetString(strategy_wedge_textField);
		if (!strcmp(wedge,"")) {
			sprintf(error_msg,"\nWarning: No Wedge size for anomalous.\n");
			post_error(error_msg);
			return;
		}
		anomalous_wedge = atoi(wedge);
		XtFree(wedge);
	}

	/* If we made it this far, all the runs are OK and
	 * we can send them.
	 */

	if (XmToggleButtonGetState(strategy_dose_mode_toggleButton) == True) {
          fprintf (stderr,"Dose mode, step size %7.3f\n",Collect.step_size);
          fflush(stderr);
	}

	beam_intensity_0 = beam_intensity;

	if (strcmp(marcollectfile,""))
		save_XmText(runtext, marcollectfile);

	strcpy(buf,"");
	for(i = 0; i < runcount; i++) {

		if (strlen(buf) > (4096-512)) { 
			if (!iswhite(buf) && (marcommandfp != NULL)) {
				fputs(buf,marcommandfp);
				fflush(marcommandfp);
			}
			strcpy(buf,"");
		}

		/* Send a Run */
		sprintf(buf+strlen(buf), "collect\n");
		if (anomalous == True) {
			sprintf(buf+strlen(buf), "anomalous 1\n");
			sprintf(buf+strlen(buf), "wedge %d\n",anomalous_wedge);
			Collect.anomalous = 1;
			Collect.wedge = anomalous_wedge;
		}
		else {
			Collect.anomalous = 0;
		}
		sprintf(buf+strlen(buf), "distance %1.4f\n", Run[i].distance);
		sprintf(buf+strlen(buf), "lift %1.4f\n", Run[i].offset);

		if (image_collect_mode==TIME_MODE) {
		  sprintf(buf+strlen(buf), "mode time\n");
		  sprintf(buf+strlen(buf), "time %1.4f\n", Run[i].exposure);
                }
		else if (image_collect_mode == DOSE_MODE) {
		  sprintf(buf+strlen(buf), "mode dose\n");
		  sprintf(buf+strlen(buf), "step_size %7.3f\n",Collect.step_size);
		  sprintf(buf+strlen(buf), "time 0.0\n");
		  sprintf(buf+strlen(buf), "dose_per_step %1.4f\n", 
		   Run[i].exposure);
                }
		else {
		  sprintf(buf+strlen(buf), "mode time\n");
		  sprintf(buf+strlen(buf), "time %1.4f\n", Run[i].exposure);
                }

		sprintf(buf+strlen(buf), "omega_start %1.4f\n", Run[i].omega);
		if(sc_conf.pf_mod == 0)
		{
			sprintf(buf+strlen(buf), "phi_start %1.4f\n", Run[i].phi);
			sprintf(buf+strlen(buf), "kappa_start %1.4f\n", Run[i].kappa);
		}
		else
		{
			sprintf(buf+strlen(buf), "phi_start %1.4f\n", 0.0);
			sprintf(buf+strlen(buf), "kappa_start %1.4f\n", 0.0);
			
			if(Run[i].phi > 10.)	/* is EV */
				sprintf(buf+strlen(buf), "wavelength %1.4f\n",EV_ANGSTROM / Run[i].phi);
			else if(Run[i].phi == 0)	/* someone left it zero, use current */
				sprintf(buf+strlen(buf), "wavelength %1.4f\n", wavelength);
			else
				sprintf(buf+strlen(buf), "wavelength %1.4f\n", Run[i].phi);
			if(Run[i].kappa >= 0)
				sprintf(buf+strlen(buf), "atten_run %1.2f\n", Run[i].kappa);
			
			if (XmToggleButtonGetState(strategy_autoaleveryyes_toggleButton) == True)
				sprintf(buf+strlen(buf), "autoal_run 1\n");
			sprintf(buf+strlen(buf), "hslit_run %s\n", XmTextFieldGetString(strategy_hslit_textField));
			sprintf(buf+strlen(buf), "vslit_run %s\n", XmTextFieldGetString(strategy_vslit_textField));
		}
		sprintf(buf+strlen(buf), "axis %s\n", Run[i].axis);
		sprintf(buf+strlen(buf), "osc_width %1.4f\n", Run[i].delta_phi);
		sprintf(buf+strlen(buf), "n_images %d\n", Run[i].nframes);
		sprintf(buf+strlen(buf), "image_number %d\n", Run[i].start);

		if((!strncmp(Run[i].dzngr,"n",1))||
		   (!strncmp(Run[i].dzngr,"N",1))||
		   (!strncmp(Run[i].dzngr,"0",1))) {
			sprintf(buf+strlen(buf), "de_zinger 0\n");
		}
		else {
			double dzratio;

			dzratio = get_dzratio(Run[i].dzngr);

			sprintf(buf+strlen(buf), "de_zinger 1\n");
			sprintf(buf+strlen(buf), "dzratio %f\n",dzratio);
		}

		sprintf(buf+strlen(buf), "directory %s\n",image_directory);
		sprintf(buf+strlen(buf), "image_prefix %s_%d\nimage_suffix %s\n",
			image_prefix, Run[i].number, image_suffix );
		if (!iswhite(Run[i].blstr))
			sprintf(buf+strlen(buf), "blcmd %s\n",Run[i].blstr);

		sprintf(buf+strlen(buf), "bin %d\n",Collect.bin);
		if(sc_conf.t2k_detector == 1)
		{
			if(Collect.bin == 1)
				Collect.adc = 0;
		}
		sprintf(buf+strlen(buf), "adc %d\n",Collect.adc);

		sprintf(buf+strlen(buf), "output_raw %d\n",Collect.output_raw);
		sprintf(buf+strlen(buf), "no_transform %d\n",Collect.no_transform);

		sprintf(buf+strlen(buf), "center %f %f\n",center_x,center_y);
		Collect.center_x = center_x;
		Collect.center_y = center_y;
		sprintf(buf+strlen(buf), "outfile_type %d\n",outfile_type);
		Collect.outfile_type = outfile_type;

		if (XmToggleButtonGetState(options_darkrun_toggleButton) == True) {
			sprintf(buf+strlen(buf), "dk_before_run %d\n", 1);
			Collect.dk_before_run = 1;
		}
		else {
			sprintf(buf+strlen(buf), "dk_before_run %d\n", 0);
			Collect.dk_before_run = 0;
		}

		if (XmToggleButtonGetState(options_darkinterval_toggleButton) == True) {
			sprintf(buf+strlen(buf), "repeat_dark 1\n");
			sprintf(buf+strlen(buf), "darkinterval %d\n",
				TextFieldGetInt(options_darkinterval_textField));
			Collect.repeat_dark = 1;
			Collect.darkinterval = TextFieldGetInt(options_darkinterval_textField);
		}
		else {
			sprintf(buf+strlen(buf), "repeat_dark 0\n");
			Collect.repeat_dark = 0;
		}

		/*
		sprintf(buf+strlen(buf), "max_deg_step %d\n", TextFieldGetInt(options_step_textField));
		 */

		switch  (image_compression) 
		{
			case COMP_NONE:
				sprintf(buf+strlen(buf), "compress none\n");
				break;
			case COMP_Z:
				sprintf(buf+strlen(buf), "compress .Z\n");
				break;
			case COMP_PCK:
				sprintf(buf+strlen(buf), "compress .pck\n");
				break;
			default:
				sprintf(buf+strlen(buf), "compress none\n");
				break;
		}
		Collect.compression = image_compression;

		if (XmToggleButtonGetState(strategy_MADno_toggleButton) == True) {
			/* Never Change Energy */
			change_wavelength=0;
			Collect.mad_mode=0;
		}
		else
		if (XmToggleButtonGetState(mad_option1_toggleButton) == True) {
			/* Never Change Energy */
			change_wavelength=0;
			Collect.mad_mode=0;
		}
		else
		if (XmToggleButtonGetState(mad_option2_toggleButton) == True) {
			/* Change Energy Each Run */
			sprintf(buf+strlen(buf), "mad run\n");
			change_wavelength=1;
			strcpy(mad_str,"         Mad: Change Energy Each Run\n");
			Collect.mad_mode=1;
		}
		else
		if (XmToggleButtonGetState(mad_option3_toggleButton) == True) {
			/* Change Energy Each Anomalous Wedge */
			sprintf(buf+strlen(buf), "mad wedge\n");
			change_wavelength=1;
			strcpy(mad_str,"         Mad: Change Energy Each Anom. Wedge\n");
			Collect.mad_mode=2;
/*
 *	Fix by cn 7/5/99: PF:  No anomalous but mad wedge is a rejected combo; post error and reject command.
 */
			if(anomalous == 0)
			  {
			  	strcpy(error_msg," Error:\n\n");
				strcat(error_msg," If you enable Each Anomalous Wedge on the MAD window\n\n");
				strcat(error_msg," You MUST specify Anomalous ---> Yes on the Runs window.\n\n");
				strcat(error_msg," Do NOT forget to enter your Wedge Size there as well.");
		    		post_error(error_msg);
				return;
			  }
		}
		else
		if (XmToggleButtonGetState(mad_option4_toggleButton) == True) {
			/* Change Energy Each N frames */
			int nf;
			nf = TextFieldGetInt(mad_nframes_textField);
			if (nf < 0 || nf > 10000) {
				fprintf(stderr,"warning: N frames = %d, set to %d\n",nf,0);
				nf = 0;
			}
			Collect.mad_nframes = nf;
			sprintf(buf+strlen(buf), "mad frames %d\n",nf);
			change_wavelength=1;
			sprintf(mad_str,"         Mad: Change Energy Every %d Frames\n",nf);
			Collect.mad_mode=3;
		}
		Collect.mad_nwave=0;
		if (change_wavelength) {
			sprintf(buf+strlen(buf), "mad_wave ");
			sprintf(mad_str+strlen(mad_str), "  Wavelength: ");
			if (XmToggleButtonGetState(enable_wavelength1_toggleButton) == True) {
				wave = TextFieldGetFloat(wavelength1_textField);
				if (wave > 0.0) {
					Collect.mad_wavelengths[Collect.mad_nwave] = wave;
					Collect.mad_nwave++;
					sprintf(buf+strlen(buf), "%1.6f ",wave);
					sprintf(mad_str+strlen(mad_str), "%1.6f ",wave);
				}
			}
			if (XmToggleButtonGetState(enable_wavelength2_toggleButton) == True) {
				wave = TextFieldGetFloat(wavelength2_textField);
				if (wave > 0.0) {
					Collect.mad_wavelengths[Collect.mad_nwave] = wave;
					Collect.mad_nwave++;
					sprintf(buf+strlen(buf), "%1.6f ",wave);
					sprintf(mad_str+strlen(mad_str), "%1.6f ",wave);
				}
			}
			if (XmToggleButtonGetState(enable_wavelength3_toggleButton) == True) {
				wave = TextFieldGetFloat(wavelength3_textField);
				if (wave > 0.0) {
					Collect.mad_wavelengths[Collect.mad_nwave] = wave;
					Collect.mad_nwave++;
					sprintf(buf+strlen(buf), "%1.6f ",wave);
					sprintf(mad_str+strlen(mad_str), "%1.6f ",wave);
				}
			}
			if (XmToggleButtonGetState(enable_wavelength4_toggleButton) == True) {
				wave = TextFieldGetFloat(wavelength4_textField);
				if (wave > 0.0) {
					Collect.mad_wavelengths[Collect.mad_nwave] = wave;
					Collect.mad_nwave++;
					sprintf(buf+strlen(buf), "%1.6f ",wave);
					sprintf(mad_str+strlen(mad_str), "%1.6f ",wave);
				}
			}
			if (XmToggleButtonGetState(enable_wavelength5_toggleButton) == True) {
				wave = TextFieldGetFloat(wavelength5_textField);
				if (wave > 0.0) {
					Collect.mad_wavelengths[Collect.mad_nwave] = wave;
					Collect.mad_nwave++;
					sprintf(buf+strlen(buf), "%1.6f ",wave);
					sprintf(mad_str+strlen(mad_str), "%1.6f ",wave);
				}
			}
			sprintf(buf+strlen(buf), "\n");
			sprintf(mad_str+strlen(mad_str), "\n");
		}
	}

	if (debug) {
		fprintf(stderr,"buf = \"%s\"\n",buf);
		fflush(stderr);
	}

	if (!iswhite(buf) && (marcommandfp != NULL)) {
		time_t t;
		char *c;

		strcat(buf,"eoc\n");

		fputs(buf,marcommandfp);
		fflush(marcommandfp);

		if (debug) {
			fputs(buf,stdout);
			fflush(stdout);
		}

		strcpy(tmpstr,image_directory);
		strcat(tmpstr,"/LOGFILE");
		if ((flog = fopen(tmpstr,"a")) != NULL) {
		
			comment =   XmTextFieldGetString(strategy_comment_textField);
			t = time(NULL);
			c = asctime(localtime(&t));
			/*fprintf(flog,"\n%s\n",c); fflush(flog);*/
			fprintf(flog,"\n        Date: %s",c);
			fprintf(flog,"   Directory: %s\n",image_directory);
			fprintf(flog,"Image Prefix: %s\n",image_prefix);
			fprintf(flog,"Image Suffix: %s\n",image_suffix);
			fprintf(flog,"  Wavelength: %f\n",wavelength);
			fprintf(flog," Beam Center: %1.3f %1.3f\n",center_x,center_y);
			fprintf(flog,"         ADC: %s\n",Collect.adc==0?"Slow":"Fast");
			fprintf(flog,"     Binning: %s\n",Collect.bin==1?"None":"2x2");
			if (Collect.mode == DOSE_MODE) 
				fprintf (flog,"  Dose Step: %7.3f\n", Collect.step_size);
			if (anomalous==True) {
				fprintf(flog,"   Anomalous: Yes, Wedge=%d\n",anomalous_wedge);
			}
			else {
				fprintf(flog,"   Anomalous: No\n");
			}
			if (strcmp(mad_str,"")) {
				fprintf(flog,"%s",mad_str);
			}
			fprintf(flog,"     Comment: %s\n\n",comment); fflush(flog);
			/*fprintf(flog,"\n<<%s>>\n\n",comment); fflush(flog);*/

#ifdef OLD
			fprintf(flog,
			    "Run# Start Total Distance Offset   Phi   Kappa  Omega   Axis D-Phi  %s De-Zinger\n",
			    (image_collect_mode==TIME_MODE)?"Time":"Dose");
			XtFree(comment);
			for(i=0; i <= MAX_RUNS; i++) {
				if (Run[i].nframes > 0) {
					fprintf(flog,
					 "%3d  %4d  %4d   %6.2f  %6.2f %6.2f %6.2f %6.2f %6s %5.2f %5.0f %2s\n",
						Run[i].number, Run[i].start, Run[i].nframes, Run[i].distance,
						Run[i].offset, Run[i].phi,
						Run[i].kappa,Run[i].omega,Run[i].axis,
						Run[i].delta_phi,
						Run[i].exposure, Run[i].dzngr);
				}
			}
#endif /* OLD */
			/* To print smaller on sgi do this:
			 * 	/usr/lib/print/lptops -FCourier-Bold -P8pt LOGFILE > LOGFILE.ps
			 */

			fprintf(flog,
			    "Run#  Start  Total  Distance  Offset    Phi    Kappa   Omega   Axis Step   %s  De-Zinger\n",
			    (image_collect_mode==TIME_MODE)?"Time":"Dose");
			XtFree(comment);
			for(i=0; i <= MAX_RUNS; i++) {
				if (Run[i].nframes > 0) {
					fprintf(flog,
					 "%3d  %4d    %4d    %6.2f   %6.2f  %6.2f  %6.2f  %6.2f %6s %5.3f %5.0f  %4s\n",
						Run[i].number, Run[i].start, Run[i].nframes, Run[i].distance,
						Run[i].offset, Run[i].phi,
						Run[i].kappa,Run[i].omega,Run[i].axis,
						Run[i].delta_phi,
						Run[i].exposure, Run[i].dzngr);
				}
			}
			fprintf(flog,"\n");
			fflush(flog);
			fclose(flog);
		}
	}
	collecting = 1;
	init_run_list();
}

/* Read a line (terminated by NULL or a NEWLINE) from str1 and
 * return in str2. Return number of characters read.
 */
get_line(str1, str2)
char *str1, *str2;
{
	char *tmp = str1;

	if (str1 == NULL) {
		*str2 = 0;
		return (0);
	}

	while (*str1 && (*str1 == '\n')) {
		str1++;
	}

	while (*str1 && (*str1 != '\n')) {
		*str2++ = *str1++;
	}
	*str2 = 0;
	if (debug) {
		fprintf(stderr,"in get_line str2 = \"%s\"\n",str2);
		fflush(stderr);
	}
	return (str1-tmp);
}

#ifdef NOT_USED
/* Extract and read field number index from str1 and return
 * in str2. Field is of type format (int or float) 
 */
read_field(str1, field, index, format, result)
char *str1, *field;
int index, format;
char *result;
{
	int f_start, f_length;
	int *int_field = (int *)result;
	double *float_field = (double *)result;

	f_start = fld[index].col_start;
	f_length = fld[index].col_end - fld[index].col_start + 1;

	if (str1 && (strlen(str1) >= (f_start+f_length))) {
		strncpy(field,str1+f_start,f_length);
		field[f_length] = 0;
		switch  (format) 
		{
			case FIELD_INT:
				*int_field = atoi(field);
				return(0);
				break;
			case FIELD_FLOAT:
				*float_field = atof(field);
				return(0);
				break;
			default:
				return(BAD_FIELD);
				break;
		}
	}
	else {
		return(BAD_FIELD);
	}
}
#endif /* NOT_USED */

init_text(w)
Widget w;
{
	static int init=0;
	int prev_mvc_return = mvc_return;

	if (!init) {
		Arg args[1];

		init = 1;
   		display  = XtDisplay(w);

		XtSetArg(args[0], XmNverifyBell, False);
		XtSetValues(w,args,1);

		mvc_return=1;
		XmTextReplace(w, 0, 0, "   ");
		mvc_return=prev_mvc_return;

		fld[0].col_start = 0; /* Run */
		fld[0].col_end = 2;

		fld[1].col_start = 6; /* Start */
		fld[1].col_end = 9;

		fld[2].col_start = 14; /* #frames */
		fld[2].col_end = 17;

		fld[3].col_start = 21; /* Distance */
		fld[3].col_end = 27;

		fld[4].col_start = 32; /* Offset */
		fld[4].col_end = 37;

		fld[5].col_start = 39; /* Phi */
		fld[5].col_end = 45;

		fld[6].col_start = 47; /* kappa */
		fld[6].col_end = 53;

		fld[7].col_start = 55; /* omega */
		fld[7].col_end = 61;

		fld[8].col_start = 63; /* axis */
		fld[8].col_end = 67;

		fld[9].col_start = 70; /* Step */
		fld[9].col_end = 74;

		fld[10].col_start = 77; /* Time */
		fld[10].col_end = 81;

		fld[11].col_start = 85; /* De-Zinger */
		fld[11].col_end = 88;

		if (beamline_mode == True) {
			fld[12].col_start = 94; /* Beamline Specific */
			fld[12].col_end = 116;
		}
	}
}

/* Copy the specified field (0 to n_fields-1) in the specified line number
 * to the string, str.
 */
extract_field(w,str,line,field)
Widget w;
char str[];
int line,field;
{
	char *s1 = (char *)XmTextGetString(w);

	if (find_index(w, line, fld[field].col_start) >= strlen(s1)) {
		str[0] = 0;
		return;
	}
	strncpy(str, s1 + (find_index(w, line, fld[field].col_start)), FIELD_LENGTH(field));
	str[FIELD_LENGTH(field)] = 0;

	XtFree(s1);
}

/* Copy str to the specifield field.
 */
insert_field(w,str,line,field)
Widget w;
char str[];
int line,field;
{
	int i,j, field_length, col_start, col_end;
	XmTextPosition last = XmTextGetLastPosition(w);
	char s1[64];
	char spaces[256];

	for(i=0;i<256;i++)
		spaces[i] = ' ';

	field_length = fld[field].col_end - fld[field].col_start + 1;

	if ((beamline_mode == True) && (field == n_fields-1))  {
		i = strlen(str);
		strcpy(s1,str);
	}
	else {
		/* Remove space from str */
		i=j=0;
		while(str[j]) {
			if (!isspace(str[j]))
				s1[i++] = str[j];
			j++;
		}
		s1[i] = 0;
	}

	/* Right adjust string */
	if (i < field_length) {
		s1[field_length] = 0;
		while(i--) {
			field_length--;
			s1[field_length] = s1[i];
		}
		while(field_length--)  {
			s1[field_length] = ' ';
		}
	}


	if (field < n_fields-1) {
		field_length = strlen(s1);
		for(i=0;i<(fld[field+1].col_start - fld[field].col_end -1);i++) {
			s1[field_length+i] = ' ';
		}
		s1[field_length+i] = 0;


		col_start = find_index(w,line,fld[field].col_start);
		if (last < col_start) {
			strcpy((char *)spaces+col_start-last,s1);
			XmTextReplace(w,last,last,spaces);
		}
		else
			XmTextReplace(w,find_index(w,line,fld[field].col_start),
				find_index(w,line,fld[field+1].col_start),s1);
	}
	else {
		col_start = find_index(w,line,fld[field].col_start);
		if (last < col_start) {
			strcpy((char *)spaces+col_start-last,s1);
			XmTextReplace(w,last,last,spaces);
		}
		else
			XmTextReplace(w,find_index(w,line,fld[field].col_start),
				find_index(w,line,fld[field].col_end+1),s1);
	}
}

/* Copy str to the beamline field.
 */
insert_field_bl(w,str,line,field)
Widget w;
char str[];
int line,field;
{
	int i,j, field_length, col_start, col_end, len;
	XmTextPosition last = XmTextGetLastPosition(w);
	char s1[64];
	char spaces[256];
	char prev_str[128];
	int prev_mvc_return = mvc_return;

	mvc_return = 1;

	for(i=0;i<256;i++)
		spaces[i] = ' ';

	field_length = fld[field].col_end - fld[field].col_start + 1;

	extract_field(w,prev_str,line,field);
	if (!iswhite(prev_str)) {
		if (!isspace(prev_str[(int)strlen(prev_str)-1]))
			strcat(prev_str," ");
		strcat(prev_str,str);
	}
	else
		strcpy(prev_str,str);

	col_start = find_index(w,line,fld[field].col_start);

	/* Truncate string at left if too long
	 */
	 while (strlen(prev_str) > field_length) {
		len = strlen(prev_str);
		for(i=0;i<len;i++) {
			if (isspace(prev_str[i])) {
				/*strcpy(prev_str,prev_str+i+1);*/
				memmove(prev_str,prev_str+i+1,strlen(prev_str)-i+1);
				break;
			}
		}
		if (i == len) {
			break;
		}
	 }

	/* Right adjust string */
	i = strlen(prev_str);
	if (i < field_length) {
		prev_str[field_length] = 0;
		while(i--) {
			field_length--;
			prev_str[field_length] = prev_str[i];
		}
		while(field_length--)  {
			prev_str[field_length] = ' ';
		}
	}


	if (last < col_start) {
		strcpy((char *)spaces+col_start-last,prev_str);
		XmTextReplace(w,last,last,spaces);
	}
	else {
		XmTextReplace(w,find_index(w,line,fld[field].col_start),
			find_index(w,line,fld[field].col_end+1),prev_str);
	}
	mvc_return = prev_mvc_return;
}

/* Make fields look "nice".
 */
clean_up(w, line, field)
Widget w;
int line, field;
{
	char str[64], axis[32], dzngr[8];
	int i;
	double d;
	int prev_mvc_return = mvc_return;

	mvc_return = 1;
	extract_field(w,str,line,field);
	switch(field)
	{
		case 0:	/* Run */
			if (sscanf(str,"%d",&i) != 1) {
				if (line < 1) {
					i = 1;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%d",&i);
					i += 1; /* Increment Run # */
				}
			}
			if ( i > MAX_RUNS) i = MAX_RUNS;
			sprintf(str,"%d",i);
			break;

		case 1: /* Starting Frame */
			if (sscanf(str,"%d",&i) != 1) {
				if (line < 1) {
					i = 1;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%d",&i);
				}
			}
			if ( i > 9999) i = 9999;
			sprintf(str,"%03d",i);
			break;

		case 2: /* Number of Frames */
			if (sscanf(str,"%d",&i) != 1) {
				if (line < 1) {
					i = 180;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%d",&i);
				}
			}
			if ( i > 9999) i = 9999;
			sprintf(str,"%d",i);
			break;

		case 3: /* Distance */
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = current_distance;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if ( d < Limit.distmin) d = Limit.distmin;
			if ( d > Limit.distmax) d = Limit.distmax;
			if (d >= 10000.0)
				sprintf(str,"%1.1f",d);
			else
				sprintf(str,"%1.2f",d);
			break;

		case 4: /* Offset */
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = current_offset;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if ( d < Limit.liftmin) d = Limit.liftmin;
			if ( d > Limit.liftmax) d = Limit.liftmax;
			sprintf(str,"%1.2f",d);
			break;

		case 5: /* Phi */
			if(sc_conf.pf_mod == 0)
			{
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = current_phi;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if (sc_conf.constrain_phi == 180) {
				while(d > 180.0)
					d -= 360.0;
				while(d < -180.0)
					d += 360.0;
			}
			else
			if (sc_conf.constrain_phi == 0) {
			}
			else {
				while(d > 360.0)
					d -= 360.0;
				while(d < 0.0)
					d += 360.0;
			}
			if (fabs(d) >= 1000)
				sprintf(str,"%1.1f",d);
			else
				sprintf(str,"%1.2f",d);
			}
			else
			{
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = wavelength;
					if(using_energy_on_runs())
						d = EV_ANGSTROM / d;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
				if(using_energy_on_runs())
				{
					if(d < 10)
						d = EV_ANGSTROM / d;
					sprintf(str,"%1.1f", d);
				}
				else
				{
					if(d > 10)	/* energy */
						d = EV_ANGSTROM / d;
					sprintf(str,"%1.5f", d);
				}
			}
			}
				
			break;

		case 6: /* Kappa */
			if(sc_conf.pf_mod == 0)
			{
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = 0;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if (sc_conf.constrain_kappa == 180) {
				while(d > 180.0)
					d -= 360.0;
				while(d < -180.0)
					d += 360.0;
			}
			else
			if (sc_conf.constrain_kappa == 0) {
			}
			else {
				while(d > 360.0)
					d -= 360.0;
				while(d < 0.0)
					d += 360.0;
			}
			if (fabs(d) >= 1000)
				sprintf(str,"%1.1f",d);
			else
				sprintf(str,"%1.2f",d);
			}
			else
			{
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = attenuator;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
					sprintf(str, "%1.2f", d);
			}
			}
			break;

		case 7: /* Omega */
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = 0;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if (sc_conf.constrain_omega == 180) {
				while(d > 180.0)
					d -= 360.0;
				while(d < -180.0)
					d += 360.0;
			}
			else
			if (sc_conf.constrain_omega == 0) {
			}
			else {
				while(d > 360.0)
					d -= 360.0;
				while(d < 0.0)
					d += 360.0;
			}
			if (fabs(d) >= 1000)
				sprintf(str,"%1.1f",d);
			else
				sprintf(str,"%1.2f",d);
			break;

		case 8: /* Axis */
			strcpy(axis,"");
			if (sscanf(str,"%s",axis) != 1) {
				if (line < 1) {
					if(sc_conf.usephi == 1)
						strcpy(axis,"phi");
					else
						strcpy(axis,"omega");
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%s",axis);
				}
			}
			if (axis[0] == 'p')
				sprintf(str,"%s","phi");
			else
			if (axis[0] == 'k')
				sprintf(str,"%s","kappa");
			else
			if (axis[0] == 'o')
				sprintf(str,"%s","omega");
			else
				sprintf(str,"%s","phi");
			break;

		case 9: /* Delta-Phi (Step Size) */
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = 1.0;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if ( d > 99.0) d = 99.0;
			/*if ((d*100 - (int)(d*100)) < 0.00001)*/
			if (d >= 10.0)
				sprintf(str,"%1.2f",d);
			else
				sprintf(str,"%1.3f",d);

			if (strlen(str) > 5) {
				str[5] = 0;
			}
		
#ifdef __linux__
			str[0] = str[0];
#endif /* __linux__ */
			break;

		case 10: /* Exposure (Time or Dose) */
			if (sscanf(str,"%lf",&d) != 1) {
				if (line < 1) {
					d = 5;
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%lf",&d);
				}
			}
			if ( d > 99999.0) d = 99999.0;
			if ((d != (int)d) && ( d <= 100))
				sprintf(str,"%1.2f",d);
			else
				sprintf(str,"%1.0f",d);
			break;

		case 11: /* De-Zinger */
			strcpy(dzngr,"");
			if (sscanf(str,"%s",dzngr) != 1) {
				if (line < 1) {
					if (sc_conf.dezinger == 1)
						strcpy(dzngr,"Y");
					else
						strcpy(dzngr,"N");
				}
				else {
					extract_field(w,str,line-1,field);
					sscanf(str,"%s",dzngr);
				}
			}
			if (dzngr[0] == 'Y')
				sprintf(str,"%s","Y");
			else
			if (dzngr[0] == 'N')
				sprintf(str,"%s","N");
			else {
				if (sc_conf.dezinger == 1)
					strcpy(dzngr,"Y");
				else
					strcpy(dzngr,"N");
			}
			break;
		case 12: /* Beamline */
			if (beamline_mode != True) {
				fprintf(stderr,"Error: BAD FIELD..\n"); fflush(stderr);
			}
			break;
		default:
			fprintf(stderr,"ERROR: BAD FIELD..\n"); fflush(stderr);
		break;
	}
	insert_field(w,str,line,field);

	mvc_return = prev_mvc_return;
}

/* Remove white space from fields
 */
no_white(w, line, field)
Widget w;
int line, field;
{
	char str[64];
	int prev_mvc_return = mvc_return;

	mvc_return = 1;
	extract_field(w,str,line,field);
	insert_field(w,str,line,field);
	mvc_return = prev_mvc_return;
}

/* Return the current row number (First row is 0).
 */
find_row(w, pos)
Widget w;
int pos;
{
	int i, row=0;
	char *s1 = (char *)XmTextGetString(w);

	if (pos > strlen(s1)) {
		fprintf(stderr,"ERROR (find_row): pos=%d strlen = %d\n",
			pos,strlen(s1)); 
		fflush(stderr);
		pos = strlen(s1);
	}
	for(i=0;i<pos;i++) {
		if (s1[i] == '\n')
			row++;
	}
	XtFree(s1);
	return(row);
}

/* Return the current column number (First Column is 0).
 */
find_column(w, pos)
Widget w;
int pos;
{
	int i, row=0, column=0;
	char *s1 = (char *)XmTextGetString(w);

	if (pos > strlen(s1)) {
		pos = strlen(s1);
		/*
		fprintf(stderr,"Error: find_column: pos=%d strlen = %d\n",
			pos,strlen(s1)); 
		fflush(stderr);
		 */
	}
	for(i=0;i<pos;i++) {
		column++;
		if (s1[i] == '\n') {
			row++;
			column = 0;
		}
	}
	XtFree(s1);
	return(column);
}


/*
find_field2(column)
int column;
{
	int current_field = 0;

	while (column > fld[current_field].space) {
		if (++current_field >= n_fields)
			return(n_fields-1);
	}
	return(current_field);
}
*/

/* Return the field (0 to n_fields-1) closest to given a column number.
 */
find_field(column)
int column;
{
	int current_field = 0;

	if (column <= fld[0].col_start)
		return (0);
	if (column >= fld[n_fields-1].col_end)
		return(n_fields-1);

	for(current_field = 0 ; current_field < n_fields; current_field++) {
		if (column <= fld[current_field].col_end) {
			if (column >= fld[current_field].col_start) {
				return(current_field);
			}
			if (abs(column-fld[current_field].col_start) <
			    abs(column-fld[current_field-1].col_end)) {
				return(current_field);
			}
			else
				return(current_field-1);
		}
	}
	return(n_fields-1);
}

/* Return the field (0 to n_fields-1) closest to given a column number.
 */
find_field2(column)
int column;
{
	int current_field = 0;

	if (column <= fld[0].col_start)
		return (0);
	if (column >= fld[n_fields-1].col_end)
		return(n_fields-1);

	for(current_field = n_fields ; current_field--; ) {
		if (column >= fld[current_field].col_start) {
				return(current_field);
		}
	}
	return(n_fields-1);
}

/* Find position of row, column.
 * If column is < 0, then return last position in row.
 */
find_index(w, row, column)
Widget w;
int row;
int column;
{
	int i=0;
	char *s1 = (char *)XmTextGetString(w);

	if (s1 == NULL) {
		fprintf(stderr,"ERROR: s1=NULL\n"); fflush(stderr);
		return(0);
	}

	if (row < 0) {
		fprintf(stderr,"ERROR: row=%d\n",row); fflush(stderr);
		XtFree(s1);
		return(0);
	}

	while (row) {
		if (s1[i] == 0) {
			if (column != 0) {
				fprintf(stderr,"ERROR in find_index: row=%d col=%d i=%d s1=\n%s\n",row,column,i,s1);
				fflush(stderr);
			}
			break;
		}

		if (s1[i] == '\n')
			row--;
		i++;
	}

	if (column < 0) {

	column = 0;
	while ((s1[i+column] != 0) && (s1[i+column] != '\n')) {
			column++;
	}
	}

	/*
	if ((i+column) > strlen(s1)) 
		column = strlen(s1) - i;
	 */

	XtFree(s1);
	return (i + column);
}

/* Used for get_run_number and get_frame number */
#define MAX_DIGITS (3)

/* Given a string of the form XXX[0-9][0-9][0-9]_XXX.XXX
 * return the "run" number. For example:
 *
 * 	test_1_001.img   -> 1
 * 	test_2_252.img   -> 2
 * 	test_999_018.img -> 999
 *
 * If no "run" number is found, return -1.
 */
get_run_number(ostr) 
char *ostr;
{
	char *pos, digits[MAX_DIGITS+1];
	int i, n=0, run = -1;
	char str[256];

	if ((ostr == NULL) || (strlen(ostr)==0))
		return -1;

	strcpy(str,ostr);

	for(i=0;i<MAX_DIGITS;i++)
		digits[i] = ' ';
	digits[i] = 0;

	/* Find last '.' */
	if ((pos = strrchr(str,'.')) == NULL)
		return -1;
	*pos = 0;

	/* Find last '_' */
	if ((pos = strrchr(str,'_')) == NULL)
		return -1;

	pos--;
	while ((pos >= str) && isdigit(*pos)) {
		digits[MAX_DIGITS-1-n] = *pos;
		if (++n >= MAX_DIGITS)
			break;
		pos--;
	}
	sscanf(digits,"%d",&run);

	if (run >= 0)
		return run;
	else
		return -1;
}

/* Given a string of the form XXXXXX_[0-9][0-9][0-9].XXX
 * return the "frame" number. For example:
 *
 * 	test_1_001.img   -> 1
 * 	test_2_252.img   -> 252
 * 	test_999_018.img -> 18

 * If no "frame" number is found, return -1.
 */
get_frame_number(ostr) 
char *ostr;
{
	char *pos, digits[MAX_DIGITS+1];
	int i, n=0, frame = -1;
	char str[256];

	if ((ostr == NULL) || (strlen(ostr)==0))
		return -1;

	strcpy(str,ostr);

	for(i=0;i<MAX_DIGITS;i++)
		digits[i] = ' ';
	digits[i] = 0;

	/* Find last '.'
	 *if ((pos = strrchr(str,'.')) == NULL)
	 *	return -1;
	 *
	 */

	/* Find last '.' */
	if ((pos = strrchr(str,'.')) == NULL)
		return -1;
	
	*pos = 0;

	 /* Find last digit */

	 pos = str + strlen(str) - 1;
	 while ((pos >= str) && (!isdigit(*pos)))
		pos--;

	while ((pos >= str) && isdigit(*pos)) {
		digits[MAX_DIGITS-1-n] = *pos;
		if (++n >= MAX_DIGITS)
			break;
		pos--;
	}
	sscanf(digits,"%d",&frame);

	if (frame >= 0)
		return frame;
	else
		return -1;
}

#ifdef OLD

/* Calculate the run completion time for the
 * current run. Return a string of the
 * form hhh:mm (hours:minutes).
 */
char *
current_run_time( run, frame, mode, intensity)
int run;   /* Current run */
int frame; /* Current frame */
int mode;  /* TIME_MODE or DOSE_MODE */
double intensity; /* Beam intensity */
{
	double scanner_time;	/* Time for scan & erase */
	double total_time	/* Time in seconds */;
	static char timestr[7];	/* Time in seconds as a string */
	double dose_to_sec;	/* Conversion from Dose to seconds */
	int i, index;

	if (anomalous == True) {
		if (run >= 100)
			run -= 100;
	}

	for(i=0; i <= MAX_RUNS; i++) {
		if ((Run[i].number == run) && 
		    (frame >= Run[i].start) && 
		    (frame <= Run[i].start+Run[i].nframes)) {
			index = i;
			break;
		}
	}
	if ( i > MAX_RUNS) {
		if (0)
			fprintf(stderr,"Error: Can not find run: %d\n",run);
		index = 0;
	}

	if (mode == DOSE_MODE) {
		dose_to_sec = DOSE_SCALE / intensity;
	}
	else
		dose_to_sec = 1.0;

	if( image_adc_mode == 1) {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_fast + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_fast * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}
	else {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_slow + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_slow * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}

	if (Run[index].nframes <= 0)
		total_time = 0;
	else {
		total_time = (scanner_time + Run[index].exposure * dose_to_sec) *
			(Run[index].nframes - (frame -  Run[index].start));
	}

	if((!strncmp(Run[index].dzngr,"n",1))||
	   (!strncmp(Run[index].dzngr,"N",1))||
	   (!strncmp(Run[index].dzngr,"0",1))) {
		;
	}
	else {
		total_time *= (1 + get_dzratio(Run[index].dzngr));
	}

	if (anomalous == True)
		total_time *= 2;

	cvt_sec((int)total_time, timestr);

	return(timestr);

}

/* Calculate the run completion time for 
 * all runs. Return a string of the
 * form hhh:mm (hours:minutes).
 */
char *
all_run_time( run, frame, mode, intensity)
int run;   /* Current run */
int frame; /* Current frame */
int mode;  /* TIME_MODE or DOSE_MODE */
double intensity; /* Beam intensity */
{
	double scanner_time;	/* Time for scan & erase */
	double total_time	/* Time in seconds */;
	static char timestr[7];	/* Time in seconds as a string */
	double dose_to_sec;	/* Conversion from Dose to seconds */
	int i, index;
	double dz_ratio;

	if (anomalous == True) {
		if (run >= 100)
			run -= 100;
	}

	for(i=0; i <= MAX_RUNS; i++) {
		if ((Run[i].number == run) && 
		    (frame >= Run[i].start) && 
		    (frame <= Run[i].start+Run[i].nframes)) {
			index = i;
			break;
		}
	}
	if ( i > MAX_RUNS) {
		if (0)
			fprintf(stderr,"Error: Can not find run: %d\n",run);
		index = 0;
	}

	if (mode == DOSE_MODE) {
		dose_to_sec = DOSE_SCALE / intensity;
	}
	else
		dose_to_sec = 1.0;

	if( image_adc_mode == 1) {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_fast + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_fast * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}
	else {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_slow + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_slow * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}

	if (Run[index].nframes <= 0)
		total_time = 0;
	else
		total_time = (scanner_time + Run[index].exposure * dose_to_sec) *
			(Run[index].nframes - (frame -  Run[index].start));

	if((!strncmp(Run[i].dzngr,"n",1))||
	   (!strncmp(Run[i].dzngr,"N",1))||
	   (!strncmp(Run[i].dzngr,"0",1))) {
		;
	}
	else {
		total_time *= (1 + get_dzratio(Run[index].dzngr));
	}

	for(i=index+1; i <= MAX_RUNS; i++) {
		if (Run[i].nframes > 0) {
			if((!strncmp(Run[i].dzngr,"n",1))||
	   		   (!strncmp(Run[i].dzngr,"N",1))||
	   		   (!strncmp(Run[i].dzngr,"0",1))) {
				/* + 2 for darks */
				total_time += (scanner_time + Run[i].exposure * dose_to_sec) *
					(Run[i].nframes + 2);
			}
			else {
				/* + 1 for darks */
				dz_ratio = get_dzratio(Run[i].dzngr);
				total_time += (1 + dz_ratio) * (scanner_time + Run[i].exposure * dose_to_sec) *
					(Run[i].nframes + 1);
			}
		}
	}



	if (anomalous == True)
		total_time *= 2;


	cvt_sec((int)total_time, timestr);

	return(timestr);
}

#endif /* OLD */

isdir(directory)
char *directory;
{
	struct stat buf[1];

	/*
	if (stat(directory, buf) == -1) {
		mkdir (directory, (mode_t)00755);
	}
	*/

	if (stat(directory, buf) == -1)
		return(0);

	if (S_ISDIR((buf[0].st_mode)))
		return(1);
	else
		return(0);
}

/* Convert number of seconds to hours and minutes in
 * a string of form hh:mm.
 *
 * For example 8820 seconds -> 02:27
 */
cvt_sec(seconds, str)
int seconds;
char *str;
{
	int hours, minutes;

	if (seconds < 0)
		seconds = 0;

	hours = seconds / ( 60 * 60);
	minutes = (seconds - (hours * 60 * 60)) / 60;

	if (hours > 999) {
		hours = 999;
		minutes = 99;
	}

	sprintf(str,"%3d:%2.2d",hours,minutes);
}

post_error(str) 
char *str;
{
	Window window;
	Dimension m_XmNx1, m_XmNy1, m_XmNwidth1, m_XmNheight1;
	Dimension m_XmNx2, m_XmNy2, m_XmNwidth2, m_XmNheight2;
	int dest_x, dest_y;
	static int init=0;
	Arg args[2];

	if (!init) {
		init=1;
		XtSetArg(args[0], XmNx, scr_width + ERRORDIALOG_X);
		XtSetArg(args[1], XmNy, ERRORDIALOG_Y);
		XtSetValues(error_Dialog,args,2);
	}
	raise_window(error_Dialog);

	/*
	XmString xmstr;
	Arg args[1];

	xmstr = XmStringCreateSimple(str);
	XtSetArg(args[0], XmNlabelString, xmstr);
	XtSetValues(error_Dialog_label,args,1);
	XmStringFree(xmstr);
	*/

	XmTextReplace(error_Dialog_text, 0, XmTextGetLastPosition(error_Dialog_text), "");
	XmTextInsert(error_Dialog_text,0,str);
	XtManageChild(error_Dialog);

	window  = XtWindow(error_Dialog);

	XtVaGetValues(error_pushButton,
		XmNx, &m_XmNx1,
		XmNy, &m_XmNy1,
		XmNwidth, &m_XmNwidth1,
		XmNheight, &m_XmNheight1,
	NULL);
	XtVaGetValues(error_pushButton,
		XmNx, &m_XmNx2,
		XmNy, &m_XmNy2,
		XmNwidth, &m_XmNwidth2,
		XmNheight, &m_XmNheight2,
	NULL);

	dest_x = (m_XmNx1 + m_XmNwidth1/2 + m_XmNx2 + m_XmNwidth2/2)/2;
	dest_y = (m_XmNy1 + m_XmNheight1/2 + m_XmNy2 + m_XmNheight2/2)/2;
	XWarpPointer(display, None, window, 0, 0, 0, 0, dest_x, dest_y);
}

/* Check Runs for reasonable values */
check_run(Run,index)
RunStruct Run[];
int index;
{
	if ((Run[index].number < 0) || (Run[index].number > MAX_RUNS) )
		return -1;

	if ((Run[index].start < 0) || (Run[index].start > 999))
		return -2;

	if ((Run[index].nframes < 0) || (Run[index].nframes > 9999))
		return -3;

	if (sc_conf.usedistance == 1) { 
		if (Run[index].distance < Limit.distmin)
			return -4;
		if (Run[index].distance > Limit.distmax)
			return -4;
	}

	if (sc_conf.uselift == 1) { 
		if (Run[index].offset < Limit.liftmin)
			return -5;
		if (Run[index].offset > Limit.liftmax)
			return -5;
	}

	if (strcmp(Run[index].axis,"phi") && 
	    strcmp(Run[index].axis,"omega") && 
	    strcmp(Run[index].axis,"kappa"))
		return -9;
	

	/* Leave Run[index].phi alone. How bad can it be. */

	/*
	if (fabs(Run[index].delta_phi) < 0.01)
		Run[index].delta_phi = 0.01;
	 */
		
	if (Run[index].exposure < 0)
		return -11;

	if (Run[index].exposure < 0.01)
		Run[index].exposure = 0.01;

	if (strncmp(Run[index].dzngr,"Y",1) &&
	    strncmp(Run[index].dzngr,"y",1) &&
	    strncmp(Run[index].dzngr,"N",1) &&
	    strncmp(Run[index].dzngr,"n",1) &&
	    strncmp(Run[index].dzngr,"/",1) &&
	    !isdigit(Run[index].dzngr[0]))
		return -12;

	return (0);
}

/* Check if any files would be overwritten by this run.
 */
check_files(directory, prefix, suffix, Run,index)
char *directory, *prefix, *suffix;
RunStruct Run[];
int index;
{
	int i;
	struct stat buf[1];
	char filename[256], run_no[16], image_no[16];

	remove_white(directory);
	remove_white(prefix);
	remove_white(suffix);
	for(i=0;i<Run[index].nframes;i++) {
		strcpy(filename,directory);
		if (directory[strlen(directory)-1] != '/')
			strcat(filename,"/");
		strcat(filename,prefix);
		strcat(filename,"_");
		sprintf(run_no,"%d",Run[index].number);
		strcat(filename,run_no);
		strcat(filename,"_");
		sprintf(image_no,"%03d",Run[index].start+i);
		strcat(filename,image_no);
		/*strcat(filename,suffix);*/
		strcat(filename,".img");
		if (stat(filename, buf) == 0) {
			if (file_overwrite_mode == 2)
				sprintf(error_msg,"\nFile Exists:\n\n%s\n\nModify Run(s) or Manually Delete File(s). Aborting Collect.",filename);
			else
				sprintf(error_msg,"\nFile Exists:\n\n%s\n\nWill Overwrite File(s).",filename);
			post_error(error_msg);
			return(-1);
		}

	}
	return(0);
}

/* Check if a directory is writable.
 */
is_writable(directory)
char *directory;
{
	int i, fd;
	char template[256];

	if (iswhite(directory))
		return (0);

	if (!isdir(directory)) {
		/* Try and create directory */
		for(i=0;i<strlen(directory);i++) {
			if (iscntrl(directory[i])) {
				fprintf(stderr,"Error: directory contains control char's: \"%s\"\n",
					directory);
				fflush(stderr);
				return(0);
			}
		}
		mkdir (directory, (mode_t)00755);
	}
	if (!isdir(directory))
		return (0);

	strcpy(template,directory);

	if (template[strlen(template)-1] != '/')
		strcat(template,"/");

	strcat(template,"XXXXXX");

	if ((fd = mkstemp(template)) == -1) {
		unlink(template);
		return (0);
	}
	close(fd);
	unlink(template);
	return (1);
}


/* Make the last non-white character of
 * "directory" be '/'.
 */
mk_directory(directory)
char *directory;
{
        int length = strlen(directory) -1 ;

        while ((length >= 0) && isspace(directory[length]))
                length--;

        if (length < 0)
                return;

        if (directory[length] != '/') {
                directory[length+1] = '/';
                directory[length+2] = 0;
        }
}

/* When the communication with the scanner has
 * been lost, deactivate the hardware control
 * functions.
 */
disable_control()
{
	XtSetSensitive(strategy_collect_Pushbutton,False);
	XtSetSensitive(strategy_restart_Pushbutton,False);
	XtSetSensitive(drive_phi90_pushButton,False);
	XtSetSensitive(drive_phi180_pushButton,False);
	XtSetSensitive(mc_shutter_button,False);
	XtSetSensitive(modify_distance_textField,False);
	XtSetSensitive(modify_phi_textField,False);
	XtSetSensitive(modify_kappa_textField,False);
	XtSetSensitive(modify_omega_textField,False);
	XtSetSensitive(modify_offset_textField,False);
	XtSetSensitive(snapshot_pushButton,False);

	XtSetSensitive(mc_phi_apply,False);
	XtSetSensitive(mc_kappa_apply,False);
	XtSetSensitive(mc_omega_apply,False);
	XtSetSensitive(mc_distance_apply,False);
	XtSetSensitive(mc_offset_apply,False);
}

#ifdef NOT_USED
enable_control()
{
	if (nocontrol==True)
		return;


	XtSetSensitive(strategy_collect_Pushbutton,True);
	XtSetSensitive(strategy_restart_Pushbutton,True);
	XtSetSensitive(drive_phi90_pushButton,True);
	XtSetSensitive(drive_phi180_pushButton,True);
	XtSetSensitive(mc_shutter_button,True);
	XtSetSensitive(modify_distance_textField,True);
	XtSetSensitive(modify_kappa_textField,True);
	XtSetSensitive(modify_omega_textField,True);
	XtSetSensitive(modify_phi_textField,True);
	XtSetSensitive(modify_offset_textField,True);
	XtSetSensitive(snapshot_pushButton,True);

	XtSetSensitive(mc_phi_apply,True);
	XtSetSensitive(mc_kappa_apply,True);
	XtSetSensitive(mc_omega_apply,True);
	XtSetSensitive(mc_distance_apply,True);
	XtSetSensitive(mc_offset_apply,True);
}
#endif /* NOT_USED */


/* Replace all elements (except \n) of str with ' ', and replace the
 * first character with c. All character between 2 newlines will
 * be deleted.
 */
strreplace(str,c)
char *str, c;
{
	char *temp = str;

	if (*str)  
		*str = c;
	else
		return;

	while(*++str) {
		if (*str != '\n')
			*str = ' ';
	}

	remove_blank_lines(temp);
}

/* Remover blank lines from str. A blank line must be
 * between 2 newlines.
 */
remove_blank_lines(str)
char *str;
{
	int found_nl=0;
	char *nlptr;
	
	while(*str) {
		if (*str == '\n') {
			if (found_nl) {
				memmove(nlptr,str,strlen(str)+1);
				str = nlptr;
			}
			else {
				found_nl = 1;
				nlptr = str;
			}
		}
		str++;
	}
}

#ifdef NOT_USED
/* Find the last integer in string s and decrement it.
 */

prev_image(s)
char *s;
{
	int i;
	char head[64], tail[64], digits[64];

	/* Assume file name is of form: *[0-9]*
	 * store this as head-digits-tail
	 */

	head[0] = digits[0] = tail[0] = 0;

	strrint(digits,s);

	for(i=strlen(s);i--;) {
		if (isdigit(s[i])) {
			strcpy(tail,&(s[i+1]));
			break;
		}
	}
	strncpy(head,s,strlen(s)-(strlen(digits)+strlen(tail)));
	head[strlen(s)-(strlen(digits)+strlen(tail))] = 0;

	if (strlen(digits)) {
		strdec(digits);
		strcpy(s,head);
		strcat(s,digits);
		strcat(s,tail);
		return(1);
	}
	return(0);
}
#endif /* NOT_USED */

/* Find the last integer in string s and increment it.
 */

next_image(s)
char *s;
{
	int i;
	char head[64], tail[64], digits[64];

	/* Assume file name is of form: *[0-9]*
	 * store this as head-digits-tail
	 */

	head[0] = digits[0] = tail[0] = 0;

	strrint(digits,s);

	for(i=strlen(s);i--;) {
		if (isdigit(s[i])) {
			strcpy(tail,&(s[i+1]));
			break;
		}
	}
	strncpy(head,s,strlen(s)-(strlen(digits)+strlen(tail)));
	head[strlen(s)-(strlen(digits)+strlen(tail))] = 0;

	if (strlen(digits)) {
		strinc(digits);
		strcpy(s,head);
		strcat(s,digits);
		strcat(s,tail);
		return(1);
	}
	return(0);
}

#ifdef NOT_USED
/* Decrement the integer stored in string s.
 */
strdec(s)
char *s;
{
	int val;
	char format[64];

	sscanf(s,"%d",&val);
	sprintf(format,"%%0%dd",strlen(s));
	if (val > 0)
		val--;
	sprintf(s,format,val);
}
#endif /* NOT_USED */

/* Increment the integer stored in string s.
 */
strinc(s)
char *s;
{
	int val;
	char format[64];

	sscanf(s,"%d",&val);
	sprintf(format,"%%0%dd",strlen(s));
	if (val < 999)
		val++;
	sprintf(s,format,val);
}

/* Copies the last integer in string s2 into string s1.
 * Copy NULL if there are no digits in the string.
 */
strrint(s1,s2)
register char *s1, *s2;
{
	register int i, length=strlen(s2);
	register char *ptr=0;

	if (isdigit(*s2))
		ptr = s2;
	for(i=1;i<length;i++)
		if (isdigit(s2[i]) && !isdigit(s2[i-1]))
			ptr = s2+i;
	if (ptr) {
		strcpy(s1,ptr);
		s1[strspn(s1,"0123456789")] = 0;
	}
	else
		*s1 = 0;
}

/* Remove white space from a string.
 */
remove_white(str)
char *str;
{
	int i, j=0, len;

	if (str == NULL)
		return;

	len = strlen(str);
	for(i=0;i<len;i++) {
		if (!isspace(str[i])) {
			str[j] = str[i];
			j++;
		}
	}
	str[j] = 0;
}

#ifdef NOT_USED
/* Check if filename (str) can be read.
 */
is_file(str)
char *str;
{
	FILE *fp;

	if ((fp = fopen(str,"r")) != NULL) {
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}
#endif /* NOT_USED */

raise_window(w)
Widget w;
{
	if (!XtIsManaged(w))
		XtManageChild(w);
	XMapWindow(display, XtWindow(XtParent(w))); /*de-iconify */
	XRaiseWindow(display,XtWindow(XtParent(w)));
}

double
get_dzratio(str)
char *str;
{
	char num[32], den[32];
	double numf, denf;

	strcpy(num,"1");
	strcpy(den,"1");
	if (str[0] == '/')
		sscanf(str,"%*[/]%[0-9]",den);
	else
		sscanf(str,"%[0-9]%*[/]%[0-9]",num,den);
	numf = atof(num);
	denf = atof(den);

	return(numf/denf);
}

/* Electron Volts per Angstrom
 */

double
energy_to_wavelength(energy) 
double energy;
{
	if (energy <= 0.0)
		return 0.0;

	return EV_ANGSTROM / energy;
}

double
wavelength_to_energy(wavelength) 
double wavelength;
{
	if (wavelength <= 0.0)
		return 0.0;

	return EV_ANGSTROM / wavelength;
}

/* Calculate the run completion time for the
 * current run. Return a string of the
 * form hhh:mm (hours:minutes).
 */
char *
current_run_time( run, frame, mode, intensity)
int run;   /* Current run */
int frame; /* Current frame */
int mode;  /* TIME_MODE or DOSE_MODE */
double intensity; /* Beam intensity */
{
	double scanner_time;	/* Time for scan & erase */
	double total_time	/* Time in seconds */;
	static char timestr[7];	/* Time in seconds as a string */
	double dose_to_sec;	/* Conversion from Dose to seconds */
	int i, index=0;
	int images_remaining;
	int ndarks=0;
	double time_per_image;
	int adx_run=run; /* Run in run's window (not run + 100) */

	if ((anomalous == True) || (Collect.mad_mode != 0)) {
		while (adx_run >= 100)
			adx_run -= 100;
	}

	for(i=0; i <= MAX_RUNS; i++) {
		if ((Run[i].number == adx_run) && 
		    (frame >= Run[i].start) && 
		    (frame <= Run[i].start+Run[i].nframes)) {
			index = i;
			break;
		}
	}

	if((!strncmp(Run[index].dzngr,"n",1))||
	   (!strncmp(Run[index].dzngr,"N",1))||
	   (!strncmp(Run[index].dzngr,"0",1))) {
	   	dezinger = 0;
	}
	else
		dezinger = 1;

	for(i=0; i < total_images; i++) {
		if ((run_list[i].run_no == run) && 
		    (run_list[i].frame_no == frame) )
			break;
	}

	i++;
	images_remaining = 0;
	while(i < total_images) {
		int r;
		r = run_list[i].run_no;

		if ((anomalous == True) || (Collect.mad_mode != 0)) {
			while (r >= 100)
				r -= 100;
		}
		if (r == adx_run)
			images_remaining++;
		i++;
	}

	if (mode == DOSE_MODE) {
		dose_to_sec = DOSE_SCALE / intensity;
	}
	else
		dose_to_sec = 1.0;

	if( image_adc_mode == 1) {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_fast + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_fast * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}
	else {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_slow + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_slow * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}

	/* Add one dark (2 images per run) */
	if (Collect.dk_before_run == 1) {
		ndarks = 0.5;
	}

	if(dezinger == 0) {
		time_per_image = (scanner_time + Run[index].exposure * dose_to_sec);
	}
	else {
		time_per_image = ((scanner_time * 2.0) + Run[index].exposure *
			(1 + get_dzratio(Run[index].dzngr) * dose_to_sec));
	}
	total_time = time_per_image * (images_remaining + ndarks);

	/* Collect dark every darkinterval seconds */
	if (Collect.repeat_dark == 1) {
		if (Collect.darkinterval == 0)
			ndarks = 0;
		else
			ndarks = (int)(total_time/Collect.darkinterval)/2;
	}

	total_time = time_per_image * (images_remaining + ndarks);
	if (Collect.repeat_dark == 1) {
		if (Collect.darkinterval == 0)
			ndarks = 0;
		else
			ndarks = (int)(total_time/Collect.darkinterval)/2;
	}
	total_time = time_per_image * (images_remaining + ndarks);

	cvt_sec((int)total_time, timestr);

	return(timestr);
}

/* Calculate the run completion time for 
 * all runs. Return a string of the
 * form hhh:mm (hours:minutes).
 */
char *
all_run_time( run, frame, mode, intensity)
int run;   /* Current run */
int frame; /* Current frame */
int mode;  /* TIME_MODE or DOSE_MODE */
double intensity; /* Beam intensity */
{
	double scanner_time;	/* Time for scan & erase */
	double total_time	/* Time in seconds */;
	static char timestr[7];	/* Time in seconds as a string */
	double dose_to_sec;	/* Conversion from Dose to seconds */
	int i, index=0;
	double dz_ratio;
	int images_remaining;
	int ndarks=0;
	double time_per_image;
	int adx_run=run; /* Run in run's window (not run + 100) */

	if ((anomalous == True) || (Collect.mad_mode != 0)) {
		while (adx_run >= 100)
			adx_run -= 100;
	}
	for(i=0; i <= MAX_RUNS; i++) {
		if ((Run[i].number == adx_run) && 
		    (frame >= Run[i].start) && 
		    (frame <= Run[i].start+Run[i].nframes)) {
			index = i;
			break;
		}
	}

	if((!strncmp(Run[index].dzngr,"n",1))||
	   (!strncmp(Run[index].dzngr,"N",1))||
	   (!strncmp(Run[index].dzngr,"0",1))) {
	   	dezinger = 0;
	}
	else
		dezinger = 1;

	if (Collect.dk_before_run == 1) {
		for(i=index+1; i <= MAX_RUNS; i++) {
			if (Run[i].nframes > 0) {
				ndarks++;
			}
		}
		if(dezinger == 0)
			ndarks *= 2;
	}


	for(i=0; i < total_images; i++) {
		if ((run_list[i].run_no == run) && 
		    (run_list[i].frame_no == frame) )
			break;
	}

	i++;
	images_remaining = 0;
	while(i < total_images) {
		images_remaining++;
		i++;
	}

	if (mode == DOSE_MODE) {
		dose_to_sec = DOSE_SCALE / intensity;
	}
	else
		dose_to_sec = 1.0;

	if( image_adc_mode == 1) {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_fast + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_fast * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}
	else {
		if (image_bin_mode == 1)
			scanner_time =
				sc_conf.read_slow + sc_conf.read_overhead;
		else
			scanner_time =
				sc_conf.read_slow * sc_conf.bin_factor +
					sc_conf.read_overhead;
	}

	if(dezinger == 0) {
		time_per_image = (scanner_time + Run[index].exposure * dose_to_sec);
	}
	else {
		time_per_image = ((scanner_time * 2.0) + Run[index].exposure *
			(1 + get_dzratio(Run[index].dzngr) * dose_to_sec));
	}
	total_time = time_per_image * (images_remaining + ndarks);

	/* Collect dark every darkinterval seconds */
	if (Collect.repeat_dark == 1) {
		if (Collect.darkinterval == 0)
			ndarks = 0;
		else
			ndarks = (int)(total_time/Collect.darkinterval)/2;
		if(dezinger == 0)
			ndarks *= 2;
	}

	total_time = time_per_image * (images_remaining + ndarks);
	if (Collect.repeat_dark == 1) {
		if (Collect.darkinterval == 0)
			ndarks = 0;
		else
			ndarks = (int)(total_time/Collect.darkinterval)/2;
		if(dezinger == 0)
			ndarks *= 2;
	}
	total_time = time_per_image * (images_remaining + ndarks);


	cvt_sec((int)total_time, timestr);

	return(timestr);
}

int	using_energy_on_runs()
{
	if(XtIsManaged(strategy_energy_label))
		return(1);
	else
		return(0);
}
