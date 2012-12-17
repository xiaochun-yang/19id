/*
 * README: Portions of this file are merged at file generation
 * time. Edits can be made *only* in between specified code blocks, look
 * for keywords <Begin user code> and <End user code>.
 */
/*
 * Generated by the ICS Builder Xcessory (BX).
 *
 * Builder Xcessory 4.0
 * Code Generator Xcessory 2.0 (09/09/96)
 *
 */
/*
 * Motif required Headers
 */
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#if (XmVersion >= 1002)
#include <Xm/RepType.h>
#endif
#include <Xm/MwmUtil.h>

/*
 * Globally included information.
 */


/*
 * Headers for classes used in this program
 */

/*
 * Common constant and pixmap declarations.
 */
#include "creation-c.h"

/*
 * Convenience functions from utilities file.
 */
extern void RegisterBxConverters();
extern XtPointer CONVERT();
extern XtPointer DOUBLE();
extern XtPointer SINGLE();
extern void MENU_POST();
extern Pixmap XPM_PIXMAP();
extern void SET_BACKGROUND_COLOR();

/* Begin user code block <globals> */
/* End user code block <globals> */

/*
 * Change this line via the Output Application Names Dialog.
 */
#define BX_APP_CLASS "BuilderProduct"

int main(argc, argv)
 int argc;
 char **argv;
{
    Widget       parent;
    XtAppContext app;
    Arg          args[256];
    Cardinal     ac;
    Boolean      argok=False;
    Widget   topLevelShell;
    
    /* Begin user code block <declarations> */
    /* End user code block <declarations> */
    
    /*
     * The applicationShell is created as an unrealized
     * parent for multiple topLevelShells.  The topLevelShells
     * are created as popup children of the applicationShell.
     * This is a recommendation of Paul Asente & Ralph Swick in
     * _X_Window_System_Toolkit_ p. 677.
     */
    
    parent = XtVaAppInitialize ( &app, BX_APP_CLASS, NULL, 0, 
#ifndef XlibSpecificationRelease
    (Cardinal *) &argc, 
#else
#if (XlibSpecificationRelease>=5)
    &argc, 
#else
    (Cardinal *) &argc, 
#endif
#endif
    argv, NULL, 
    NULL );
    
    RegisterBxConverters(app);
#if (XmVersion >= 1002) 
    XmRepTypeInstallTearOffModelConverter();
#endif
    
    /* Begin user code block <create_shells> */
    /* End user code block <create_shells> */
    
    /*
     * Create classes and widgets used in this program. 
     */
    
    /* Begin user code block <create_xmDialogShell7> */
    /* End user code block <create_xmDialogShell7> */
    
    configsiteDialog = (Widget)CreateconfigsiteDialog(parent);
    
    /* Begin user code block <create_xmDialogShell28> */
    /* End user code block <create_xmDialogShell28> */
    
    project_dialog = (Widget)Createproject_dialog(parent);

/* PF Start */

#ifdef PF_MOD
 
    stars_dialog = (Widget)Createstars_dialog(parent);
    
#endif /* PF_MOD */

/* PF End */

    /* Begin user code block <create_xmDialogShell34> */
    /* End user code block <create_xmDialogShell34> */
    
    define_kappa_Dialog = (Widget)Createdefine_kappa_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell33> */
    /* End user code block <create_xmDialogShell33> */
    
    define_omega_Dialog = (Widget)Createdefine_omega_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell32> */
    /* End user code block <create_xmDialogShell32> */
    
    optimize_add_fSB = (Widget)Createoptimize_add_fSB(parent);
    
    /* Begin user code block <create_xmDialogShell27> */
    /* End user code block <create_xmDialogShell27> */
    
    localSiteDialog = (Widget)CreatelocalSiteDialog(parent);
    
    /* Begin user code block <create_xmDialogShell26> */
    /* End user code block <create_xmDialogShell26> */
    
    weakbeamDialog = (Widget)CreateweakbeamDialog(parent);
    
    /* Begin user code block <create_xmDialogShell25> */
    /* End user code block <create_xmDialogShell25> */
    
    nobeamDialog = (Widget)CreatenobeamDialog(parent);
    
    /* Begin user code block <create_xmDialogShell23> */
    /* End user code block <create_xmDialogShell23> */
    
    mcinfoDialog = (Widget)CreatemcinfoDialog(parent);
    
    /* Begin user code block <create_xmDialogShell21> */
    /* End user code block <create_xmDialogShell21> */
    
    diskfullDialog = (Widget)CreatediskfullDialog(parent);
    
    /* Begin user code block <create_xmDialogShell17> */
    /* End user code block <create_xmDialogShell17> */
    
    error_Dialog = (Widget)Createerror_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell14> */
    /* End user code block <create_xmDialogShell14> */
    
    abortDialog = (Widget)CreateabortDialog(parent);
    
    /* Begin user code block <create_xmDialogShell12> */
    /* End user code block <create_xmDialogShell12> */
    
    delete_method_fSB = (Widget)Createdelete_method_fSB(parent);
    
    /* Begin user code block <create_xmDialogShell11> */
    /* End user code block <create_xmDialogShell11> */
    
    save_method_fSB = (Widget)Createsave_method_fSB(parent);
    
    /* Begin user code block <create_xmDialogShell10> */
    /* End user code block <create_xmDialogShell10> */
    
    load_method_fSB = (Widget)Createload_method_fSB(parent);
    
    /* Begin user code block <create_xmDialogShell4> */
    /* End user code block <create_xmDialogShell4> */
    
    exitDialog = (Widget)CreateexitDialog(parent);
    
    /* Begin user code block <create_xmDialogShell2> */
    /* End user code block <create_xmDialogShell2> */
    
    strategyDialog = (Widget)CreatestrategyDialog(parent);
    
    /* Begin user code block <create_xmDialogShell> */
    /* End user code block <create_xmDialogShell> */
    
    statusDialog = (Widget)CreatestatusDialog(parent);
    XtManageChild(statusDialog);
    
    /* Begin user code block <create_xmDialogShell1> */
    /* End user code block <create_xmDialogShell1> */
    
    snapshotDialog = (Widget)CreatesnapshotDialog(parent);
    
    /* Begin user code block <create_xmDialogShell3> */
    /* End user code block <create_xmDialogShell3> */
    
    manual_controlDialog = (Widget)Createmanual_controlDialog(parent);
    
    /* Begin user code block <create_xmDialogShell5> */
    /* End user code block <create_xmDialogShell5> */
    
    define_distance_Dialog = (Widget)Createdefine_distance_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell6> */
    /* End user code block <create_xmDialogShell6> */
    
    stopDialog = (Widget)CreatestopDialog(parent);
    
    /* Begin user code block <create_xmDialogShell8> */
    /* End user code block <create_xmDialogShell8> */
    
    define_phi_Dialog = (Widget)Createdefine_phi_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell9> */
    /* End user code block <create_xmDialogShell9> */
    
    define_offset_Dialog = (Widget)Createdefine_offset_Dialog(parent);
    
    /* Begin user code block <create_xmDialogShell15> */
    /* End user code block <create_xmDialogShell15> */
    
    restartRun_dialog = (Widget)CreaterestartRun_dialog(parent);
    
    /* Begin user code block <create_xmDialogShell16> */
    /* End user code block <create_xmDialogShell16> */
    
    versionDialog = (Widget)CreateversionDialog(parent);
    
    /* Begin user code block <create_xmDialogShell30> */
    /* End user code block <create_xmDialogShell30> */
    
    optimize_dialog = (Widget)Createoptimize_dialog(parent);
    
    /* Begin user code block <create_xmDialogShell19> */
    /* End user code block <create_xmDialogShell19> */
    
    adx_helpDialog = (Widget)Createadx_helpDialog(parent);
    
    /* Begin user code block <create_xmDialogShell35> */
    /* End user code block <create_xmDialogShell35> */
    
    alertDialog = (Widget)CreatealertDialog(parent);
    
    /* Begin user code block <create_xmDialogShell36> */
    /* End user code block <create_xmDialogShell36> */
    
    optionsDialog = (Widget)CreateoptionsDialog(parent);
    
    /* Begin user code block <create_xmDialogShell22> */
    /* End user code block <create_xmDialogShell22> */
    
    madDialog = (Widget)CreatemadDialog(parent);
    
    /* Begin user code block <create_topLevelShell> */
    /* End user code block <create_topLevelShell> */
    
    ac = 0;
    XtSetArg(args[ac], XmNtitle, "ADX"); ac++;
    XtSetArg(args[ac], XmNx, 873); ac++;
    XtSetArg(args[ac], XmNy, 4); ac++;
    XtSetArg(args[ac], XmNwidth, 132); ac++;
    XtSetArg(args[ac], XmNheight, 272); ac++;
    XtSetArg(args[ac], XmNbackground, 
        CONVERT(parent, "Papaya Whip", 
        XmRPixel, 0, &argok)); if (argok) ac++;
    topLevelShell = XtCreatePopupShell("topLevelShell",
        topLevelShellWidgetClass,
        parent,
        args, 
        ac);
    bulletinBoard = (Widget)CreatebulletinBoard(topLevelShell);
    XtManageChild(bulletinBoard);
    XtPopup(XtParent(bulletinBoard), XtGrabNone);
    
    /* Begin user code block <app_procedures> */


    /* End user code block <app_procedures> */
    
    /* Begin user code block <main_loop> */

	XtAddEventHandler(setup_arrowButton,\
		ButtonPressMask, False, MENU_POST, (XtPointer)setup_popupMenu);
	XtAddEventHandler(stop_arrowButton,
		ButtonPressMask, False, MENU_POST, (XtPointer)popupMenu);
	XtAddEventHandler(display_arrowButton,
		ButtonPressMask, False, MENU_POST, (XtPointer)display_popupMenu);
	XtAddEventHandler(process_arrowButton,
		ButtonPressMask, False, MENU_POST, (XtPointer)process_popupMenu);

	{
	void junk2(), junk2a(), junk3();
	XtAddEventHandler(runtext, ButtonReleaseMask, False, junk2, NULL);
	XtAddEventHandler(runtext, ButtonPressMask, False, junk3, NULL);


	XtAddEventHandler(runtext, KeyReleaseMask, False, junk2a, NULL);
	}

	init_adx(argc, argv); 

	/*
	XtSetSensitive(mad_option3_toggleButton, False);
	XtSetSensitive(label97, False);

	XtSetSensitive(mad_option4_toggleButton, False);
	XtSetSensitive(label108, False);
	XtSetSensitive(mad_nframes_textField, False);
	XtSetSensitive(label109, False);
	*/

	/*
	XtUnmanageChild(autoindexButton);
	XtUnmanageChild(xdsButton);
	XtUnmanageChild(mosflmButton);
	 */

    /* End user code block <main_loop> */
    
    XtAppMainLoop(app);
    
    /*
     * A return value regardless of whether or not the main loop ends. 
     */
     return(0); 
}
