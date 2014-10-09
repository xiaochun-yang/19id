#!/bin/sh
# the next line restarts using -*-Tcl-*-sh \
	 exec wish "$0" ${1+"$@"}
#
#                        Copyright 2001
#                              by
#                 The Board of Trustees of the 
#               Leland Stanford Junior University
#                      All rights reserved.
#
#                       Disclaimer Notice
#
#     The items furnished herewith were developed under the sponsorship
# of the U.S. Government.  Neither the U.S., nor the U.S. D.O.E., nor the
# Leland Stanford Junior University, nor their employees, makes any war-
# ranty, express or implied, or assumes any liability or responsibility
# for accuracy, completeness or usefulness of any information, apparatus,
# product or process disclosed, or represents that its use will not in-
# fringe privately-owned rights.  Mention of any product, its manufactur-
# er, or suppliers shall not, nor is it intended to, imply approval, dis-
# approval, or fitness for any particular use.  The U.S. and the Univer-
# sity at all times retain the right to use and disseminate the furnished
# items for any purpose whatsoever.                       Notice 91 02 01
#
#   Work supported by the U.S. Department of Energy under contract
#   DE-AC03-76SF00515; and the National Institutes of Health, National
#   Center for Research Resources, grant 2P41RR01209. 
#

package provide BLUICECompleteGui 1.0

# load the required standard packages
package require Itcl
package require Iwidgets
package require BWidget
package require BLT

# load the DCS packages
package require DCSDevice
package require DCSDeviceView
package require DCSOperationManager
package require DCSProtocol
package require DCSButton
package require DCSPrompt
package require DCSMotorControlPanel
package require DCSMdi
package require DCSUtil

# source all Tcl files
package require BLUICEHutchTab
package require BLUICESampleTab
package require BLUICECollectTab
package require BLUICESetupTab
package require BLUICEHutchOverview
package require BLUICEVideoNotebook
package require BLUICESamplePosition
package require BLUICETable
package require BLUICEMotorConfig
package require BLUICEScan
package require BLUICEMadScan
package require BLUICEUsersTab
package require BLUICEScreeningTab
package require BLUICESortingTab
package require BLUICEStatusBar
package require BLUICEQueueTab
package require BLUICEMicroSpecTab
package require Scan3DView
package require RasterTab
package require FloatGridTab

class BluIce {
	inherit ::itk::Widget

	itk_option define -controlSystem controlSystem ControlSystem "::dcss"
	itk_option define -periodicFile periodicFile PeriodicFile ""

    private method createHutchTab
    private method createSampleTab
    private method createCollectTab
    private method createQueueTab
    private method createScanTab
    private method createScreeningTab
    private method createUsersTab
    private method createSortingTab
    private method createStaffTab
    private method createUserLogTab
    private method createRasteringTab
    private method createMicroSpecTab
    private method createHelicalTab
    private method createLCLSL614Tab
    private method createLCLSCrystalTab
    private method createLCLSGridTab
    private method createPXL614Tab
    
	public method getLogin
	public method loginSuccessful
   public method handleStaffChange
   private method updateTabState

   private variable m_loggedIn 0 
   private variable m_staff 0


    public common objMicroSpecTab ""

	# public methods
	constructor { args } {
		
        set ring $itk_interior
        
		itk_component add notebook {
			DCS::TabNotebook $ring.n  \
				 -tabbackground lightgrey \
				 -background lightgrey \
				 -backdrop lightgrey \
				 -borderwidth 2\
				 -tabpos n \
				 -gap -4 \
				 -angle 20 \
				 -raiseselect 1 \
				 -bevelamount 4 \
				 -padx 5 -pady 4 \
				 -width 1000 -height 650
		} {}

		#create all of the tabs

		#getLogin

        set tabList [::config getBluIceTabOrder]

        foreach tab $tabList {
           switch $tab {
              Hutch {createHutchTab }
              Collect {createCollectTab }
              Queue {createQueueTab }
              Screening {createScreeningTab }
              Scan {createScanTab }
              Users {createUsersTab }
              Sorting {createSortingTab }
              Setup {createStaffTab }
              Staff {createStaffTab }
              UserLog {createUserLogTab }
              Sample {createSampleTab }
              Rastering {createRasteringTab }
              MicroSpec {createMicroSpecTab}
              Helical   {createHelicalTab}
              LCLSL614  {createLCLSL614Tab}
              LCLSCrystal {createLCLSCrystalTab}
              LCLSGrid    {createLCLSGridTab}
              PXL614  {createPXL614Tab}
              default {
                puts "+++++++++++++++++++++++++++++++++++++++++++++"
                puts "unknow TAB $tab"
                puts "+++++++++++++++++++++++++++++++++++++++++++++"
              }
           }
        }
        
		#start on the login tab
		$itk_component(notebook) select 0

      ::mediator register $this ::dcss staff handleStaffChange
   
		eval itk_initialize $args
		
		pack $itk_component(notebook) -expand yes -fill both
		#pack $itk_component(operationStatus) -expand yes -fill both

		$itk_option(-controlSystem) configure -forcedLoginCallback "$this getLogin"
		$itk_option(-controlSystem) configure -gotValidSessionIdCallback "$this loginSuccessful"

      updateTabState

      ::mediator announceExistence $this

		return
	}

}

body BluIce::createHutchTab {} {
   set childsite [$itk_component(notebook) add Hutch -label "Hutch"]

   itk_component add hutchTab {
      HutchTab $childsite.h
   } {
      keep -detectorType -gonioPhiDevice 
      keep -gonioOmegaDevice -gonioKappaDevice
      keep -detectorVertDevice -detectorHorzDevice
      keep -detectorZDevice -energyDevice
      keep -attenuationDevice -beamWidthDevice
      keep -beamHeightDevice -beamstopDevice
      keep -videoParameters
      keep -videoEnabled
   }

   $itk_component(hutchTab) addChildVisibilityControl $itk_component(notebook) activeTab Hutch
   pack $itk_component(hutchTab) -expand yes -fill both
}
        
body BluIce::createSampleTab {} {
   set childsite [$itk_component(notebook) add Sample -label "Sample"]

   itk_component add sampleTab {
      SampleTab $childsite.sample
   } {
      keep -videoParameters
      keep -videoEnabled
   }

   $itk_component(sampleTab) addChildVisibilityControl $itk_component(notebook) activeTab Sample
   pack $itk_component(sampleTab) -expand yes -fill both
}

body BluIce::createRasteringTab {} {
   set childsite [$itk_component(notebook) add Rastering -label "Raster"]

    set className [::config getStr "rasterClass"]
    if {$className == ""} {
        set className RasterTab
    }

   itk_component add rasteringTab {
      $className $childsite.sample
   } {
      keep -videoEnabled
      keep -videoParameters
      keep -beamHeightDevice -beamWidthDevice
      keep -imageServerHost -imageServerHttpPort
   }

   $itk_component(rasteringTab) addChildVisibilityControl $itk_component(notebook) activeTab Rastering
   pack $itk_component(rasteringTab) -expand yes -fill both

    if {$className == "FloatGridTab"} {
        $itk_component(notebook) register $itk_component(rasteringTab) \
        activeTab handleActiveTabUpdate
    }
}

body BluIce::createHelicalTab {} {
   set childsite [$itk_component(notebook) add Helical -label "Helical Collect"]

   itk_component add helicalTab {
      FloatGridTab $childsite.helical \
      -purpose forCrystal \
   } {
      keep -videoEnabled
      keep -videoParameters
      keep -beamHeightDevice -beamWidthDevice
      keep -imageServerHost -imageServerHttpPort
   }

   $itk_component(helicalTab) addChildVisibilityControl $itk_component(notebook) activeTab Helical
   pack $itk_component(helicalTab) -expand yes -fill both

    $itk_component(notebook) register $itk_component(helicalTab) activeTab \
    handleActiveTabUpdate
}

body BluIce::createLCLSL614Tab { } {
    set childsite [$itk_component(notebook) add L614 -label "XFEL Grid Collect"]

    itk_component add l614Tab {
        FloatGridTab $childsite.l614 \
        -purpose forL614 \
    } {
        keep -videoEnabled
        keep -videoParameters
        keep -beamHeightDevice -beamWidthDevice
        keep -imageServerHost -imageServerHttpPort
    }

    $itk_component(l614Tab) addChildVisibilityControl \
    $itk_component(notebook) activeTab L614 

    pack $itk_component(l614Tab) -expand yes -fill both

    $itk_component(notebook) register $itk_component(l614Tab) activeTab \
    handleActiveTabUpdate
}
body BluIce::createPXL614Tab { } {
    set childsite [$itk_component(notebook) add PXL614 -label "L614 Grid"]

    itk_component add pxl614Tab {
        FloatGridTab $childsite.pxl614 \
        -purpose forPXL614 \
    } {
        keep -videoEnabled
        keep -videoParameters
        keep -beamHeightDevice -beamWidthDevice
        keep -imageServerHost -imageServerHttpPort
    }

    $itk_component(pxl614Tab) addChildVisibilityControl \
    $itk_component(notebook) activeTab PXL614

    pack $itk_component(pxl614Tab) -expand yes -fill both

    $itk_component(notebook) register $itk_component(pxl614Tab) activeTab \
    handleActiveTabUpdate
}

body BluIce::createLCLSCrystalTab { } {
    set childsite [$itk_component(notebook) add LCLSCrystal -label "XFEL Helical Collect"]

    itk_component add lclsCrystalTab {
        FloatGridTab $childsite.lclsCrystal \
        -purpose forLCLSCrystal \
    } {
        keep -videoEnabled
        keep -videoParameters
        keep -beamHeightDevice -beamWidthDevice
        keep -imageServerHost -imageServerHttpPort
    }

    $itk_component(lclsCrystalTab) addChildVisibilityControl \
    $itk_component(notebook) activeTab LCLSCrystal

    pack $itk_component(lclsCrystalTab) -expand yes -fill both

    $itk_component(notebook) register $itk_component(lclsCrystalTab) activeTab \
    handleActiveTabUpdate
}

body BluIce::createLCLSGridTab { } {
    set childsite [$itk_component(notebook) add LCLSGrid \
    -label "XFEL Raster Collect"]

    itk_component add lclsGridTab {
        FloatGridTab $childsite.lclsGrid \
        -purpose forLCLS \
    } {
        keep -videoEnabled
        keep -videoParameters
        keep -beamHeightDevice -beamWidthDevice
        keep -imageServerHost -imageServerHttpPort
    }

    $itk_component(lclsGridTab) addChildVisibilityControl \
    $itk_component(notebook) activeTab LCLSGrid

    pack $itk_component(lclsGridTab) -expand yes -fill both

    $itk_component(notebook) register $itk_component(lclsGridTab) activeTab \
    handleActiveTabUpdate
}

body BluIce::createMicroSpecTab {} {
   set childsite [$itk_component(notebook) add MicroSpec -label "MicroSpec"]

   itk_component add microSpecTab {
      MicroSpecTab $childsite.microSpec
   } {
   }

   pack $itk_component(microSpecTab) -expand yes -fill both

    set objMicroSpecTab $itk_component(microSpecTab)
    puts "microSpecTab: $objMicroSpecTab"
}

body BluIce::createCollectTab { } {
   set childsite [$itk_component(notebook) add Collect -label "Collect"]

   itk_component add collectTab {
      DCS::CollectTab $childsite.s 
   } {
      keep -imageServerHost -imageServerHttpPort
   }
   pack $itk_component(collectTab) -expand yes -fill both
}

body BluIce::createQueueTab { } {
    puts "create QUEUE tab"
    set childsite [$itk_component(notebook) add Queue -label "Queue"]

    itk_component add queueTab {
        QueueTab $childsite.q 
    } {
        keep -videoParameters
        keep -videoEnabled
    }
    $itk_component(queueTab) addChildVisibilityControl $itk_component(notebook) activeTab Queue
    pack $itk_component(queueTab) -expand yes -fill both
}

body BluIce::createScreeningTab { } {
   set childsite [$itk_component(notebook) add Screening -label "Screening"]

   itk_component add screeningTab {
      ScreeningTab $childsite.s 
   } {
      #keep -detectorType
      keep -videoParameters
      keep -videoEnabled
   }

   $itk_component(screeningTab) addChildVisibilityControl $itk_component(notebook) activeTab Screening
   
   pack $itk_component(screeningTab) -expand yes -fill both
}


body BluIce::createScanTab { } {
   set childsite [$itk_component(notebook) add Scan -label "Scan"]
        
   set scanTabType [::config getBluIceScanTabType]
   if { $scanTabType == "motor" } {
      itk_component add scan {
		DCS::ScanWidget $childsite.scan ::device::gonio_phi -deviceList [::config getBluIceScanMotorList] 
	  } {}

	  pack $itk_component(scan)
   
   } else {
      itk_component add fluorescenceScan {
         ScanTab $childsite.fluor
      } {
         keep -periodicFile
      }
      pack $itk_component(fluorescenceScan) -fill both -expand 1
   } 
}

body BluIce::createUsersTab { } {

   set childsite [$itk_component(notebook) add User -label "Users"]

   itk_component add usersTab {
      UsersTab $childsite.u
   } {}
   pack $itk_component(usersTab) -expand yes -fill both
}
body BluIce::createSortingTab { } {

   set childsite [$itk_component(notebook) add Sort -label "Sorting"]

   itk_component add sortingTab {
      SortingTab $childsite.s
   } {}
   pack $itk_component(sortingTab) -expand yes -fill both
}


body BluIce::createStaffTab { } {
   set childsite [$itk_component(notebook) add Staff -label "Staff"]

   itk_component add setupTab {
      SetupTab $childsite.s
   } {
      keep -detectorType -gonioPhiDevice 
      keep -gonioOmegaDevice -gonioKappaDevice
      keep -detectorVertDevice -detectorHorzDevice
      keep -detectorZDevice -energyDevice
      keep -attenuationDevice -beamWidthDevice
      keep -beamHeightDevice -beamstopDevice
      keep -cameraZoomDevice -videoParameters
	  keep -sampleXDevice -sampleYDevice -sampleZDevice
      keep -videoEnabled
      keep -periodicFile
      keep -imageServerHost -imageServerHttpPort
   }
   pack $itk_component(setupTab) -expand yes -fill both
}
body BluIce::createUserLogTab { } {

   set childsite [$itk_component(notebook) add UserLog -label "Log"]

   itk_component add UserLogTab {
      DCS::UserLogView $childsite.log
   } {}
   pack $itk_component(UserLogTab) -expand yes -fill both
}

body BluIce::getLogin {} {
   set m_loggedIn 0  
   updateTabState
}

body BluIce::loginSuccessful {} {
   set m_loggedIn 1  
   updateTabState
}


body BluIce::handleStaffChange {- targetReady_ alias value_ -} {

	if { ! $targetReady_ } return
   if {$m_staff == $value_} return

   set m_staff $value_

   updateTabState
}


body BluIce::updateTabState {} {
    set last [$itk_component(notebook) index end]
    if {$m_loggedIn } {
        for {set i 0} {$i <= $last} {incr i} {
            foreach {dummy1 dummy2 dummy3 dummy4 tabLabel} \
            [$itk_component(notebook) pageconfigure $i -label] break

            set state normal
            if {$tabLabel == "Staff" && !$m_staff} {
                set state disabled
            }
            $itk_component(notebook) pageconfigure $i -state $state
        }

	    $itk_component(notebook) select 0
   } else {
        set userIndex -1
        for {set i 0} {$i <= $last} {incr i} {
            foreach {dummy1 dummy2 dummy3 dummy4 tabLabel} \
            [$itk_component(notebook) pageconfigure $i -label] break

            set state disabled
            if {$tabLabel == "Users"} {
                set state normal
                set userIndex $i
            }
            $itk_component(notebook) pageconfigure $i -state $state
        }

        if {$userIndex >= 0} {
            $itk_component(notebook) select $userIndex
        }
    }
}


proc setGeometry {} {

   set desiredX 1100
   set desiredY 800
        
   #get maximum size allowed by window manager
   set maxWindowX [lindex [wm maxsize .] 0]
   set maxWindowY [lindex [wm maxsize .] 1]
   
   #lets pretend the window is smaller to allow space for window manager tool bars etc.
   set maxWindowY [expr int($maxWindowY * 0.95) + 1]
   
   #pick conservative screen size
   set x [min $maxWindowX $desiredX] 
   set y [min $maxWindowY $desiredY] 

   wm geometry . ${x}x${y}
}

proc startBluIce { configuration_ } {

	global BLC_DATA
    global gMotorBeamWidth
    global gMotorBeamHeight
    global gMotorPhi
    global gMotorOmega
    global gMotorDistance
    global gMotorBeamStop
    global gMotorVert
    global gMotorHorz

    setGeometry

	set imageServerHost [$configuration_ getImgsrvHost]
	set imageServerHttpPort [$configuration_ getImgsrvHttpPort]
	
	#get the name of the periodic table specification file
	set periodicFile [$configuration_ getPeriodicFilename]
	if { $periodicFile != ""} {
		#add the directory if we know the name of the file
		set periodicFile [file join $BLC_DATA $periodicFile]
	}


   Destroyer .d


   ::iwidgets::panedwindow .pw 

   .pw add BluIce 
   .pw add Logger -minimum 10

	# create the apply button
	set statusBar [StatusBar .sb]

	set bluice [BluIce [.pw childsite 0].bluice \
         -detectorType Q315CCD \
         -gonioPhiDevice ::device::$gMotorPhi \
		 -gonioOmegaDevice ::device::$gMotorOmega \
		 -gonioKappaDevice ::device::gonio_kappa \
		 -detectorVertDevice ::device::$gMotorVert \
		 -detectorHorzDevice ::device::$gMotorHorz \
		 -detectorZDevice ::device::$gMotorDistance \
		 -energyDevice ::device::energy \
		 -attenuationDevice ::device::attenuation \
		 -beamWidthDevice ::device::$gMotorBeamWidth \
		 -beamHeightDevice ::device::$gMotorBeamHeight \
		 -beamstopDevice ::device::$gMotorBeamStop \
		 -cameraZoomDevice ::device::camera_zoom \
		 -videoParameters &resolution=high \
		 -sampleXDevice ::device::sample_x \
		 -sampleYDevice ::device::sample_y \
		 -sampleZDevice ::device::sample_z \
		 -videoEnabled 1 \
		 -periodicFile $periodicFile \
		 -imageServerHost $imageServerHost \
		 -imageServerHttpPort $imageServerHttpPort]

	#pack .bluice -expand yes -fill both


	DCS::ComponentGate ::dataCollectionActive
	::dataCollectionActive addInput "::device::collectRun status inactive {Data collection is in progress.}"
	::dataCollectionActive addInput "::device::collectRuns status inactive {Data collection is in progress.}"
	::dataCollectionActive addInput "::device::collectFrame status inactive {Data collection is in progress.}"
	
	set logView [DCS::LogView [.pw childsite 1].lv]
	#DCS::Prompt .p -text command -background white -logger .l


   .pw fraction 92 8

   pack .pw -expand 1 -fill both
	pack $bluice -expand 1 -fill both
	pack $logView -expand 1 -fill both
	pack $statusBar -expand 1 -fill x -side left
}

