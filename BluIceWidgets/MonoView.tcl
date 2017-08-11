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
##########################################################################
#
#                       Permission Notice
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTA-
# BILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
# EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##########################################################################

package provide BLUICEMonoView 1.0

# load standard packages
package require Iwidgets
package require BWidget

# load other DCS packages
package require DCSUtil
package require DCSSet
package require DCSComponent

package require DCSDeviceView
package require DCSProtocol
package require DCSOperationManager
package require DCSHardwareManager
package require DCSPrompt
package require DCSMotorControlPanel
package require BLUICECanvasShapes
package require BLUICECanvasGifView
package require DCSScriptCommand 1.0


class SingleCrystalHorizFocusMonoView {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
		mono_slit_horz \
		mono_angle \
		mono_theta \
		energy \
		mono_bend \
		table_slide \
		table_2theta \
        ]
    }

	constructor { args} {

		place $itk_component(control) -x 220 -y 310
		eval itk_initialize $args

		motorView mono_slit_horz 150 189 n
		motorView mono_angle 337 89 s
		motorView mono_theta 337 192 n
		motorView energy 725 150 s
		motorView mono_bend 438 110 sw
		motorView table_slide 559 248  n
		motorView table_2theta 659 232 nw

		pack $itk_component(canvas)

	   motorArrow mono_slit_horz 202 164 {} 168 187 193 157 173 172
      motorArrow mono_theta 335 164 { 315 174 335 184 355 174 } 340 164 323 157 346 157 
      motorArrow mono_bend 403 128 {} 438 105 409 111 430 96
      motorArrow table_slide 562 226 {} 604 226 567 236 600 236
      motorArrow table_2theta 650 230 {725 227} 675 210  655 241  679 201
      #motorArrow mono_angle 335 84 { 315 94 335 104 355 94 } 340 84 323 77 346 77 

      # draw the spear slit
      draw_slit 199 118

      # draw the x-ray beam
      $itk_component(canvas) create line 67 158  199 144 -fill red -width 2
      # draw the crystal 
      rect_solid 270 110 130 30 5 7 3
      $itk_component(canvas) create line 235 140 331 129 733 175 -fill red -width 2 -arrow last
      $itk_component(canvas) create line 338 113 338 95 -fill blue -width 2
      $itk_component(canvas) create line 338 190 338 147 -fill blue -width 2
      $itk_component(canvas) create poly 360 175 380 175 645 205 635 205 360 175 -fill	#c0c0ff
      $itk_component(canvas) create line 360 175 400 175 645 205 605 205 360 175
		
      $itk_component(canvas) create poly 360 176 360 185 605 217 605 207 360 176 -fill #b0b0ee
      $itk_component(canvas) create line 360 176 360 185 605 217 605 206
      $itk_component(canvas) create poly 605 217 605 206 645 206 645 217 605 217 -fill #b0b0ee
	   $itk_component(canvas) create line 605 217 605 205 645 205 645 217 605 217

		eval itk_initialize $args
      configure -width 800 -height 400
	}
}

class SingleCrystalHorizFocusMonoViewBL113 {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
		mono_slit_horz \
		mono_angle \
		mono_theta \
		energy \
		mono_bend \
        ]
    }

	constructor { args} {

		place $itk_component(control) -x 220 -y 310
		eval itk_initialize $args

		motorView mono_slit_horz 550 189 n
		motorView mono_angle 337 89 s
		motorView mono_theta 337 192 n
		motorView energy 725 150 s
		motorView mono_bend 438 110 sw
		#motorView table_horz 559 248  n
		#motorView table_horz_2 659 232 nw

		pack $itk_component(canvas)

      $itk_component(canvas) create line 338 113 338 95 -fill blue -width 2
      $itk_component(canvas) create line 338 190 338 147 -fill blue -width 2

	   motorArrow mono_slit_horz 560 164 {} 528 187 533 172 553 157 
      motorArrow mono_theta 335 164 { 315 174 335 184 355 174 } 340 164 323 157 346 157 
      motorArrow mono_bend 403 128 {} 438 105 409 111 430 96
      #motorArrow table_horz 562 226 {} 604 226 567 236 600 236
      #motorArrow table_horz_2 650 230 {725 227} 675 210  655 241  679 201
      #motorArrow mono_angle 335 84 { 315 94 335 104 355 94 } 340 84 323 77 346 77 


      # draw the x-ray beam
      #$itk_component(canvas) create line 67 158  199 144 -fill red -width 2
      # draw the crystal 
      rect_solid 270 110 130 30 5 7 3
      $itk_component(canvas) create line 67 158 235 140 331 129 733 175 -fill red -width 2 -arrow first

      # draw the slit
      draw_slit 500 118

      #$itk_component(canvas) create poly 360 175 380 175 645 205 635 205 360 175 -fill	#c0c0ff

      #$itk_component(canvas) create line 360 175 400 175 645 205 605 205 360 175
      #$itk_component(canvas) create line 360 176 360 185 605 217 605 206
		
      #$itk_component(canvas) create poly 360 176 360 185 605 217 605 207 360 176 -fill #b0b0ee
      #$itk_component(canvas) create poly 605 217 605 206 645 206 645 217 605 217 -fill #b0b0ee
	   #$itk_component(canvas) create line 605 217 605 205 645 205 645 217 605 217

		eval itk_initialize $args
      configure -width 800 -height 400
	}
}




class DoubleCrystalMonoView {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_ssrl \
        mono_slit_spear \
        mono_slit_lower \
        mono_slit_vert \
	mono_c2_perp \
	mono_c2_para \
        mono_c2_pitch \
        mono_c2_roll \
	mono_c2_yaw\
	mono_c2_bend_1\
	mono_c2_bend_2\
	mono_c2_bend\
	mono_c1_bend\
        mono_theta \
        energy \
        ]
    }

	constructor { args} {

      set m_deviceFactory [::DCS::DeviceFactory::getObject]

      # construct the goniometer widgets
      motorView mono_slit_ssrl 347 89 s 
      motorView mono_slit_spear 120 189 n
      motorView mono_slit_lower 267 244 n
      motorView mono_slit_vert 230 42 e
      motorView mono_c2_pitch 645 130 w
      motorView mono_c2_roll 645 150 w
      motorView mono_c2_yaw 645 170 w
      motorView mono_theta 515 263 n
      motorView energy 725 226 n


		place $itk_component(control) -x 220 -y 360

	   motorArrow mono_slit_ssrl 308 96 {} 274 119 305 111 283 126
	   motorArrow mono_slit_spear 202 164 {} 168 187 193 157 173 172
	   motorArrow mono_slit_lower 248 199 {} 248 244 258 204 258 239
	   motorArrow mono_slit_vert 248 41 {} 248 86 238 46 238 81 
	   motorArrow mono_c2_pitch 610 205 {640 195 640 155} 610 145 625 213 625 138
	   motorArrow mono_c2_roll 470 165 { 480 150 490 170 480 190 } 470 175 463 158 463 181
      motorArrow mono_theta 540 230 {530 260 490 260} 480 230 551 238 467 241

      # draw the ssrl slit 
      draw_slit 241 90

      # draw the spear slit
	   draw_slit 199 118

      # draw the lower slit
      draw_slit 230 139

      # draw the vert slit
      draw_slit 230 74

      # draw the fixed crystal 
      rect_solid 400 195 100 10 20 30 20	

      # draw the x-ray beam
      $itk_component(canvas) create line 68 105  199 140 -fill red -width 2
      $itk_component(canvas) create line 235 149 461 205 561 180 733 225 -fill red -width 2 -arrow last

      # draw the moving crystal 
      rect_solid 500 160 100 10 20 30 20

      $itk_component(canvas) create line 486 156 489 162 489 178 483 187 -fill lightgrey
	   $itk_component(canvas) create line 508 171 460 171 -fill blue -width 2
 
		eval itk_initialize $args

      configure -width 800 -height 400
	}

}

class DoubleCrystalMonoViewDoubleSet {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_ssrl \
        mono_slit_spear \
        mono_slit_lower \
        mono_slit_upper \
        mono_b_pitch \
        mono_b_roll \
        mono_theta \
        mono_trans \
        mono_vert \
        energy \
        ]
    }
    public method gotoToroidView { } {
        $itk_option(-mdiHelper) openToolChest toroid
    }


	constructor { args} {

      set m_deviceFactory [::DCS::DeviceFactory::getObject]

		# construct the goniometer widgets
      motorView mono_slit_ssrl 347 89 s 
      motorView mono_slit_spear 120 189 n
      motorView mono_slit_lower 267 244 n
      motorView mono_slit_upper 230 42 e
      motorView mono_b_pitch 645 130 w
      motorView mono_b_roll 485 150 s
      motorView mono_theta 515 263 n
      motorView energy 705 226 n
      motorView mono_trans 510 89 sw
      motorView mono_vert 50 360 w

        $itk_component(mono_vert) configure -entryWidth 10


		place $itk_component(control) -x 220 -y 360

	   motorArrow mono_slit_ssrl 308 96 {} 274 119 305 111 283 126
	   motorArrow mono_slit_spear 202 164 {} 168 187 193 157 173 172
	   motorArrow mono_slit_lower 248 199 {} 248 244 258 204 258 239
	   motorArrow mono_slit_upper 248 41 {} 248 86 238 46 238 81 
	   motorArrow mono_b_pitch 610 205 {640 195 640 155} 610 145 625 213 625 138
	   motorArrow mono_b_roll 470 165 { 480 150 490 170 480 190 } 470 175 463 158 463 181
      motorArrow mono_theta 540 230 {530 260 490 260} 480 230 551 238 467 241
	   motorArrow mono_trans 570 91 {} 570 160 580 96 580 155
	   motorArrow mono_vert 30 270 {} 30 380 40 275 40 375

      # draw the ssrl slit 
      draw_slit 241 90

      # draw the spear slit
	   draw_slit 199 118

      # draw the lower slit
      draw_slit 230 139

      # draw the vert slit
      draw_slit 230 74

      # draw the fixed crystal 
      rect_solid 400 195 100 10 20 30 20	

      # draw the x-ray beam
      $itk_component(canvas) create line 68 105  199 140 -fill red -width 2
      $itk_component(canvas) create line 235 149 461 205 561 180 733 225 -fill red -width 2 -arrow last

      # draw the moving crystal 
      rect_solid 500 160 100 10 20 30 20

      $itk_component(canvas) create line 486 156 489 162 489 178 483 187 -fill lightgrey
	   $itk_component(canvas) create line 508 171 460 171 -fill blue -width 2

        #draw box of whole mono
	    $itk_component(canvas) create line 50 320 50 15 780 15 -width 2 -dash .
	    $itk_component(canvas) create rectangle 50 320 780 330 -fill #a0a0dd
        
        #### link button
        itk_component add link {
            button $itk_component(canvas).link \
            -text "goto toroidView ==>" \
            -command "$this gotoToroidView"
        } {
        }
	    place $itk_component(link) -x 780 -y 340 -anchor ne
 
		eval itk_initialize $args

      configure -width 800 -height 400
	}

}
class DoubleCrystalMonoViewDoubleSet9_2 {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_ssrl \
        mono_slit_spear \
        mono_slit_lower \
        mono_slit_upper \
        mono_b_pitch \
        mono_b_roll \
        mono_theta \
        mono_trans \
        mono_vert \
        energy \
        ]
    }
    public method gotoToroidView { } {
        $itk_option(-mdiHelper) openToolChest toroid
    }


	constructor { args} {
        set m_deviceFactory [::DCS::DeviceFactory::getObject]

        loadBackdropImage BL9-2Mono.gif
		place $itk_component(control) -x 220 -y 460

        motorView mono_slit_ssrl 709 208 se
        motorView mono_slit_spear 539 322 n
        motorView mono_slit_lower 130 314 n
        motorView mono_slit_upper 130 60 s
        motorView mono_b_pitch 538 156 s
        motorView mono_b_roll 271 188 s
        motorView mono_theta 271 314 n
        motorView energy 271 412 s
        motorView mono_trans 394 125 s
        motorView mono_vert 50 460 w

        moveHotSpot mono_slit_ssrl 676 222 positive 0
        moveHotSpot mono_slit_ssrl 655 229 negative 0

        moveHotSpot mono_slit_spear 535 273 positive 0
        moveHotSpot mono_slit_spear 509 279 negative 0

        moveHotSpot mono_slit_lower 143 260 positive 0
        moveHotSpot mono_slit_lower 143 291 negative 0

        moveHotSpot mono_slit_upper 120 68  positive 0
        moveHotSpot mono_slit_upper 120 96  negative 0

        moveHotSpot mono_b_pitch 523 235 positive 0
        moveHotSpot mono_b_pitch 523 161 negative 0

        moveHotSpot mono_b_roll 294 199 positive 0
        moveHotSpot mono_b_roll 294 222 negative 0

        moveHotSpot mono_theta 414 304 positive 0
        moveHotSpot mono_theta 328 304 negative 0

        moveHotSpot mono_trans 407 132 positive 0
        moveHotSpot mono_trans 407 204 negative 0

        moveHotSpot mono_vert 34  384 positive 0
        moveHotSpot mono_vert 34  455 negative 0

        $itk_component(mono_vert) configure -entryWidth 10

        #### link button
        itk_component add link {
            button $itk_component(canvas).link \
            -text "goto toroidView ==>" \
            -command "$this gotoToroidView"
        } {
        }
	    place $itk_component(link) -x 690 -y 440 -anchor ne
 
		eval itk_initialize $args

        configure -width 710 -height 500
	}

}

class DoubleCrystalMonoViewDoubleSetID19 {
        inherit ::DCS::CanvasGifView

    itk_option define -mdiHelper mdiHelper MdiHelper ""
    private variable m_deviceFactory
    private variable m_strPiezoVoltage

    public proc getMotorList { } {
        return [list \
        mono_c2_para \
        mono_c2_perp \
        mono_c2_pitch \
        mono_c2_roll \
	mono_c2_yaw \
	mono_c2_bend_1 \
	mono_c2_bend_2 \
	mono_c2_bend \
	mono_c1_bend \
	mono_angle \
        mono_theta \
        energy \
        ]
    }

    public method handleEncoderUpdate
    public method handleMotorTempChange

    public method setPitchValue { } {
	set deviceFactory [::DCS::DeviceFactory::getObject]
	set obj1 [$deviceFactory getObjectName ion_chamber4]
      	set volt [$itk_component(canvas).pitch get]
	$obj1 set_position $volt
    }

    public method setRollValue { } {
	set deviceFactory [::DCS::DeviceFactory::getObject]
        set obj2 [$deviceFactory getObjectName ion_chamber5]
	set volt [$itk_component(canvas).roll get]
        $obj2 set_position $volt
    }


    constructor { args} {
        set deviceFactory [::DCS::DeviceFactory::getObject]
	set obj1 [$deviceFactory getObjectName ion_chamber4]
	set obj2 [$deviceFactory getObjectName ion_chamber5]
        set m_strBeamCurrent [$deviceFactory createString analogInStatus10]

        loadBackdropImage id19-mono-yang-8.gif
                place $itk_component(control) -x 220 -y 460

        motorView mono_c2_para 510 80 ne
        motorView mono_c2_perp 670 80 ne
	motorView mono_c2_bend_1 50 140 nw
	motorView mono_c2_bend_2 185 140 nw
	motorView mono_c2_bend 185 80 nw
	motorView mono_c1_bend 50 200 nw
	motorView mono_c2_pitch 670 300 se
        motorView mono_c2_roll  670 370 se
	motorView mono_c2_yaw   670 440 se
	motorView mono_angle 80 420 s
        motorView mono_theta 210 420 s
        motorView energy 340 420 s


#### yangx pitch button

        itk_component add pitch {
            DCS::Entry $itk_component(canvas).pitch \
            -background  #d0d000 \
            -systemIdleOnly 0 \
            -activeClientOnly 1 \
            -entryType positiveFloat \
	    -entryWidth 5 \
	    -promptText "Fine Pitch" \
	    -units v \
	    -onSubmit "$this setPitchValue"
        } {
        }

	itk_component add pitchl {
                label $itk_component(canvas).pitchl \
                      -text [format %4.2f [$obj1 cget -position]] \
                      -relief sunken \
                      -width 4
                 #     -background #c0c0ff \
                 #     -activebackground #c0c0ff
                } {
                           keep -foreground
        }

  itk_component add roll {
            DCS::Entry $itk_component(canvas).roll \
            -background  #d0d000 \
            -systemIdleOnly 0 \
            -activeClientOnly 1 \
	    -entryWidth 5 \
            -entryType positiveFloat \
	    -promptText "Fine Roll" \
            -units v \
	    -onSubmit "$this setRollValue"
        } {
        }

	itk_component add rolll {
                label $itk_component(canvas).rolll \
                      -text [format %4.2f [$obj2 cget -position]] \
                      -relief sunken \
                      -width 4
                 #     -background #c0c0ff \
                 #     -activebackground #c0c0ff
                } {
                           keep -foreground
        }

        for {set i 0} {$i < 7} {incr i} {

              itk_component add temp$i {
                  # make the optimize beam button
                  label $itk_component(canvas).temp$i \
                      -text " " \
                      -relief sunken \
                      -width 4 \
                      -background #c0c0ff \
                      -activebackground #c0c0ff
              } {
                           keep -foreground
              }
        }

	place $itk_component(temp1) -x 675 -y 270 -anchor se
	place $itk_component(temp2) -x 675 -y 337 -anchor se
	place $itk_component(temp3) -x 525 -y 100 -anchor se
	place $itk_component(temp4) -x 195 -y 220 -anchor se
	place $itk_component(temp5) -x 335 -y 160 -anchor se
        place $itk_component(temp6) -x 675 -y 410 -anchor se

        place $itk_component(pitchl) -x 495 -y 295 -anchor se
	place $itk_component(pitch) -x 515 -y 320 -anchor se 
      	place $itk_component(rolll) -x 495 -y 355 -anchor se
      	place $itk_component(roll) -x 515 -y 380 -anchor se

        
	$m_strBeamCurrent register $this contents handleMotorTempChange
	$obj1 register $this -value handleEncoderUpdate
	$obj2 register $this -value handleEncoderUpdate
        eval itk_initialize $args
	
        configure -width 730 -height 500
   }

   destructor {
        set deviceFactory [::DCS::DeviceFactory::getObject]
        set obj1 [$deviceFactory getObjectName ion_chamber4]
        set obj2 [$deviceFactory getObjectName ion_chamber5]
        set m_strBeamCurrent [$deviceFactory createString analogInStatus10]

	$obj1 unregister $this -value handleEncoderUpdate
	$obj2 unregister $this -value handleEncoderUpdate
	$m_strBeamCurrent unregister $this contents handleMotorTempChange
   }
}

body DoubleCrystalMonoViewDoubleSetID19::handleEncoderUpdate {name_ targetReady_ - value_ -} {
    
     if {!$targetReady_} return
     set tex [format %4.2f $value_]
     #puts "name = $name_"
     if {$name_ == "::device::ion_chamber4"} { 
     	$itk_component(canvas).pitchl configure -text $tex 
     } else {
     	$itk_component(canvas).rolll configure -text $tex 
     }
}

body DoubleCrystalMonoViewDoubleSetID19::handleMotorTempChange { name_ targetReady_ - contents_ - } {
    if { ! $targetReady_} return
    #puts "contents=$contents_ \n"
    if { $contents_ == "" } return

    for {set i 0} {$i < 7} {incr i} {
           set value [lindex $contents_ $i]
           $itk_component(temp$i) configure \
                -text [format "%.1f" $value] \
                -state normal
    }

    #if {[string is double -strict $contents_]} {
    #    set display [format "%.3f" $contents_]
    #} else {
    #    set display $contents_
    #}
    #$itk_component(ioncurrent) configure -text $display
#    log_error "yangxx display=$display"
}

class DoubleCrystalMonoViewDoubleSet12_2 {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_ssrl \
        mono_slit_spear \
        mono_slit_lower \
        mono_slit_upper \
        mono_a_pitch \
        mono_a_roll \
        mono_theta \
        mono_trans \
        energy \
        ]
    }
    public method gotoFocusingMirrorsView { } {
        $itk_option(-mdiHelper) openToolChest focusing_mirrors
    }
    public method gotoMirrorView { } {
        $itk_option(-mdiHelper) openToolChest mirror
    }

	constructor { args} {

      set m_deviceFactory [::DCS::DeviceFactory::getObject]

		# construct the goniometer widgets
#      motorView mono_slit_ssrl 347 89 s 
      motorView mono_a_pitch 347 89 s
      motorView mono_slit_spear 120 189 n
      motorView mono_slit_lower 267 244 n
      motorView mono_slit_upper 230 42 e
#      motorView mono_a_pitch 645 130 w
	motorView mono_slit_ssrl 645 130 w
      motorView mono_a_roll 485 150 s
      motorView mono_theta 515 263 n
      motorView energy 680 230 n
      motorView mono_trans 510 89 sw
      

		place $itk_component(control) -x 220 -y 360

	   motorArrow mono_slit_ssrl 308 96 {} 274 119 305 111 283 126
	   motorArrow mono_slit_spear 202 164 {} 168 187 193 157 173 172
	   motorArrow mono_slit_lower 248 199 {} 248 244 258 204 258 239
	   motorArrow mono_slit_upper 248 41 {} 248 86 238 46 238 81 
	   motorArrow mono_a_pitch 610 205 {640 195 640 155} 610 145 625 213 625 138
	   motorArrow mono_a_roll 470 165 { 480 150 490 170 480 190 } 470 175 463 158 463 181
      motorArrow mono_theta 540 230 {530 260 490 260} 480 230 551 238 467 241
	   motorArrow mono_trans 570 91 {} 570 160 580 96 580 155

      # draw the ssrl slit 
      draw_slit 241 90

      # draw the spear slit
	   draw_slit 199 118

      # draw the lower slit
      draw_slit 230 139

      # draw the vert slit
      draw_slit 230 74

      # draw the fixed crystal 
      rect_solid 400 195 100 10 20 30 20	

      # draw the x-ray beam
      $itk_component(canvas) create line 68 105  199 140 -fill red -width 2
      $itk_component(canvas) create line 235 149 461 205 561 180 833 250 -fill red -width 2 -arrow last

      # draw the moving crystal 
      rect_solid 500 160 100 10 20 30 20

      $itk_component(canvas) create line 486 156 489 162 489 178 483 187 -fill lightgrey
	   $itk_component(canvas) create line 508 171 460 171 -fill blue -width 2



      motorView mono_diagnostic_vert 825 110 s
	  motorArrow mono_diagnostic_vert 825 110 {} 825 200 840 115 840 195
      draw_aperature_base 815 200 15 20 8 8


       
        #### link button
        itk_component add link_ds {
            button $itk_component(canvas).ds \
            -text "goto Focusing Mirrors View ==>" \
            -command "$this gotoFocusingMirrorsView"
        } {
        }
        itk_component add link_us {
            button $itk_component(canvas).us \
            -text "<==goto Mirror View" \
            -command "$this gotoMirrorView"
        } {
        }
	    place $itk_component(link_ds) -x 780 -y 320 -anchor ne
	    place $itk_component(link_us) -x 20 -y 320 -anchor nw
 
		eval itk_initialize $args

      configure -width 900 -height 400
	}

}

class DoubleCrystalMonoViewDoubleSetAperture12_2 {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_horiz \
        mono_slit_horiz_gap \
        mono_slit_vert \
        mono_slit_vert_gap \
        mono_a_pitch \
        mono_a_roll \
        mono_theta \
        mono_trans \
        energy \
        ]
    }
    public method gotoFocusingMirrorsView { } {
        $itk_option(-mdiHelper) openToolChest focusing_mirrors
    }
    public method gotoMirrorView { } {
        $itk_option(-mdiHelper) openToolChest mirror_aperture
    }

	constructor { args} {

      set m_deviceFactory [::DCS::DeviceFactory::getObject]

        draw_aperature 10 0 mono_slit_vert mono_slit_vert_gap \
        mono_slit_horiz mono_slit_horiz_gap

      motorView mono_a_pitch 645 130 w
      motorView mono_a_roll 485 150 s
      motorView mono_theta 515 263 n
      motorView energy 680 230 n
      motorView mono_trans 510 89 sw
      
		place $itk_component(control) -x 220 -y 360

	   motorArrow mono_a_pitch 610 205 {640 195 640 155} 610 145 625 213 625 138
	   motorArrow mono_a_roll 470 165 { 480 150 490 170 480 190 } 470 175 463 158 463 181
      motorArrow mono_theta 540 230 {530 260 490 260} 480 230 551 238 467 241
	   motorArrow mono_trans 570 91 {} 570 160 580 96 580 155

      # draw the fixed crystal 
      rect_solid 400 195 100 10 20 30 20	

      # draw the x-ray beam
      $itk_component(canvas) create line 68 105  177 133 -fill red -width 2
      $itk_component(canvas) create line 194 138 461 205 561 180 833 250 -fill red -width 2 -arrow last

      # draw the moving crystal 
      rect_solid 500 160 100 10 20 30 20

      $itk_component(canvas) create line 486 156 489 162 489 178 483 187 -fill lightgrey
	   $itk_component(canvas) create line 508 171 460 171 -fill blue -width 2



      motorView mono_diagnostic_vert 825 110 s
	  motorArrow mono_diagnostic_vert 825 110 {} 825 200 840 115 840 195
      draw_aperature_base 815 200 15 20 8 8


       
        #### link button
        itk_component add link_ds {
            button $itk_component(canvas).ds \
            -text "goto Focusing Mirrors View ==>" \
            -command "$this gotoFocusingMirrorsView"
        } {
        }
        itk_component add link_us {
            button $itk_component(canvas).us \
            -text "<==goto Mirror View" \
            -command "$this gotoMirrorView"
        } {
        }
	    place $itk_component(link_ds) -x 780 -y 320 -anchor ne
	    place $itk_component(link_us) -x 20 -y 320 -anchor nw
 
		eval itk_initialize $args

      configure -width 900 -height 400
	}

}



class DoubleCrystalMonoViewDown {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_vert \
        mono_slit_gap_vert \
        mono_slit_horz \
        mono_slit_gap_horz \
        mono_pitch \
        mono_roll \
        mono_theta \
        energy \
        ]
    }

	constructor { args} {

      set m_deviceFactory [::DCS::DeviceFactory::getObject]

		draw_aperature 50 0 \
			 mono_slit_vert \
			 mono_slit_gap_vert \
			 mono_slit_horz \
			 mono_slit_gap_horz
		
      motorView mono_pitch 700 180 w
      motorView mono_roll 484 200 n
      motorView mono_theta 591 5 n
      motorView energy 765 65 n

		place $itk_component(control) -x 220 -y 285

	   #motorArrow mono_pitch 648 181 {690 172 690 149} 671 137 643 186 669 127
	   motorArrow mono_pitch 672 131 {680 149 680 167} 672 185 674 121 674 195 
	   motorArrow mono_roll 490 156 { 500 141 510 161 500 181 } 498 166 483 172 483 149 
      motorArrow mono_theta 510 90 {550 77 590 77} 630 90 633 100 503 100 

      # draw the moving crystal 
      rect_solid 550 145 100 10 20 30 20
      $itk_component(canvas) create line 68 177 215 164 -fill red -width 2
      $itk_component(canvas) create line 234 163 505 141 609 150 755 137 -fill red -width 2 -arrow last

      # draw the fixed crystal 
      rect_solid 450 110 100 10 20 30 20	

      #$itk_component(canvas) create line 486 156 489 162 489 148 483 187 -fill lightgrey
	   $itk_component(canvas) create line 554 157 490 157 -fill blue -width 2
 
		eval itk_initialize $args

      configure -width 850 -height 320
	}

}




class UpwardMirrorView {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
	    mirror_slit_upper \
	    mirror_vert_chin \
	    mirror_slit_lower \
	    mirror_pitch \
	    mirror_vert \
	    mirror_bend \
	    mirror_slit_vert \
	    mirror_slit_vert_gap \
        ]
    }

	constructor { args} {

		place $itk_component(control) -x 180 -y 350

   	# draw the lower slit
	   rect_solid 160 152 5 50 20 30 30

	   # draw the mirror
	   rect_solid 240 190 200 10 20 30 20

	   # draw the x-ray beam
	   $itk_component(canvas) create line 64 129 349 198 612 110 -fill red -width 2 -arrow last

	   # draw the upper slit
	   rect_solid 200 97 5 50 20 30 30

	   motorView mirror_slit_upper 214 55 s
	   motorView mirror_vert_chin 349 282 n
	   motorView mirror_slit_lower 176 262 n
	   motorView mirror_pitch 500 197 w
	   motorView mirror_vert 356 128 s
	   motorView mirror_bend 544 303 n 
	   motorArrow mirror_vert 350 132 {} 350 182 361 139 361 179
	   motorArrow mirror_vert_chin 350 232 {} 350 282 361 239 361 279
	   motorArrow mirror_slit_upper 217 59 {} 217 109 205 64 205 102
	   motorArrow mirror_slit_lower 178 212 {} 178 262 191 217 191 258 
   #	motorArrow mirror_pitch  460 235 {490 225 490 185} 460 175 481 238 480 173 
	   motorArrow mirror_pitch 460 175 {490 185 490 225 } 460 235 480 173 481 238 	
	   motorArrow mirror_bend 620 322 {} 620 372 631 369 631 329

        motorView mirror_slit_vert     30 160 nw
        motorView mirror_slit_vert_gap 30  70 nw
	    motorArrow mirror_slit_vert 10 20 {} 10 205 20 25 20 200
	    motorArrow mirror_slit_vert_gap 170 120 {} 170 155 180 125 180 150



		eval itk_initialize $args
      configure -width 660 -height 380
	}
}



class DownwardMirrorView {

 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
   	    mirror_slit_upper \
   	    mirror_vert_chin \
   	    mirror_bend \
   	    mirror_slit_lower \
   	    mirror_pitch \
   	    mirror_vert \
   	    mirror_slit_vert \
   	    mirror_slit_vert_gap \
        ]
    }

	constructor { args} {

		place $itk_component(control) -x 220 -y 300

   	# draw the lower slit
   	rect_solid 160 152 5 50 20 30 30

   	# draw the x-ray beam
   	$itk_component(canvas) create line 27 183 355 135 612 180 -fill red -width 2 -arrow last

   	# draw the upper slit
   	rect_solid 200 97 5 50 20 30 30

   	# draw the mirror
   	rect_solid 240 110 200 10 20 30 20

   	motorView mirror_slit_upper 190 10 n
   	motorView mirror_slit_lower 145 265 n
   	motorView mirror_slit_vert   20 110 nw
   	motorView mirror_slit_vert_gap   230 165 nw

   	motorView mirror_vert_chin 449 165 n
   	motorView mirror_bend 544 220 n
   	motorView mirror_pitch 500 107 w
   	motorView mirror_vert 356 68 s
   	motorArrow mirror_vert 355 72 {} 355 122 366 79 366 119
   	motorArrow mirror_slit_upper 217 59 {} 217 109 205 64 205 102
   	motorArrow mirror_slit_lower 178 212 {} 178 262 191 217 191 258 
   	motorArrow mirror_pitch  460 145 {490 135 490 95} 460 85 481 148 480 83 

   	motorArrow mirror_slit_vert 155 85 {} 155 220 145 90 145 215
   	motorArrow mirror_slit_vert 220 165 {} 220 210 210 170 210 205

		eval itk_initialize $args
      configure -width 660 -height 350
	}
}

class UpwardMirrorViewBL113 {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""
	# create the canvas to draw the mirror in

    public proc getMotorList { } {
        return [list \
	    mirror_vert \
	    mirror_pitch \
	    mirror_bend \
        ]
    }

	constructor { args} {
   
   rect_solid 180 130 240 10 20 70 20

	# draw the x-ray beam
	$itk_component(canvas) create line 159 80 295 140 440 80 -fill red -width 2 -arrow first 

	motorView mirror_vert 305 70 s
	motorView mirror_pitch 10 137 w
	motorView mirror_bend 304 216 n
	motorArrow mirror_vert 295 80 {} 295 130 306 80 306 114
	motorArrow mirror_bend 295 162 {} 295 212  306 206 306 168
	motorArrow mirror_pitch  180 120 {150 130 150 170} 180 180 186 114 183 187

	place $itk_component(control) -x 160 -y 300
   eval itk_initialize $args
   configure -width 520 -height 350
   configure -serialMove 0 
   }  
}

class ssrlwardMirrorView {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
	    mirror_slit_ssrl \
	    mirror_slit_spear \
	    mirror_horz \
	    mirror_yaw \
        ]
    }

    public method gotoDownStreamView { } {
        $itk_option(-mdiHelper) openToolChest mono
    }

	constructor { args} {

		place $itk_component(control) -x 180 -y 350

	    # draw the ssrl slit
	    rect_solid 100 120 5 50 20 30 30
	    # draw the x-ray beam
	    $itk_component(canvas) create line 20 150 340 225 612 180 -fill red -width 2 -arrow last
   	    # draw the spear slit
	    rect_solid 60 152 5 50 20 30 30
	    # draw the mirror
	    rect_solid 240 200 200 50 5 5 5


	    motorView mirror_slit_ssrl 200 50 n
	    motorView mirror_slit_spear 100 250 n
	    motorView mirror_horz 560 250 n
	    motorView mirror_yaw 450 120 n 
	    motorArrow mirror_slit_ssrl 200 110 {} 140 150 190 100 145 130
	    motorArrow mirror_slit_spear 60 200 {} 0 240 55 215 5 250
	    motorArrow mirror_horz 500 230 {} 440 270 495 245 445 280
	    $itk_component(canvas) create line 340 150 340 204 -fill blue -width 2
	    motorArrow mirror_yaw 330 160 {320 170 340 180 360 170} 350 160 355 150 325 150

        itk_component add link_ds {
            button $itk_component(canvas).ds \
            -text "goto Mono View ==>" \
            -command "$this gotoDownStreamView"
        } {
        }
	    place $itk_component(link_ds) -x 640 -y 320 -anchor ne
		eval itk_initialize $args
        configure -width 660 -height 380
	}
}

class ssrlwardMirrorApertureView {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
	    mirror_slit_horiz\
	    mirror_slit_horiz_gap \
	    mirror_horz \
	    mirror_yaw \
        ]
    }

    public method gotoDownStreamView { } {
        $itk_option(-mdiHelper) openToolChest mono_aperture
    }

	constructor { args} {

		place $itk_component(control) -x 180 -y 350

		#draw_aperature -50 0 "" "" mirror_slit_horiz mirror_slit_horiz_gap
        set id [draw_aperature_base 100 100 32 100 16 20]
	    # draw the x-ray beam
	    set line_id [$itk_component(canvas) create line 20 150 340 225 612 180 -fill red -width 2 -arrow last]

        ###raise one wall
        $itk_component(canvas) raise $id $line_id


	    # draw the mirror
	    rect_solid 240 200 200 50 5 5 5


	    motorView mirror_slit_horiz 100 250 n
	    motorView mirror_slit_horiz_gap 200 50 n
	    motorView mirror_horz 560 250 n
	    motorView mirror_yaw 450 120 n 
	    motorArrow mirror_slit_horiz 106 170 {} 46 210 101 185 51 220
	    #motorArrow mirror_slit_horiz_gap 130  70 {}  70 110 120  60  75  90
	    motorArrow mirror_slit_horiz_gap 130  70 {}  100 90 120  60  105  70
	    motorArrow mirror_horz 500 230 {} 440 270 495 245 445 280
	    $itk_component(canvas) create line 340 150 340 204 -fill blue -width 2
	    motorArrow mirror_yaw 330 160 {320 170 340 180 360 170} 350 160 355 150 325 150

        itk_component add link_ds {
            button $itk_component(canvas).ds \
            -text "goto Mono View ==>" \
            -command "$this gotoDownStreamView"
        } {
        }
	    place $itk_component(link_ds) -x 640 -y 320 -anchor ne
		eval itk_initialize $args
        configure -width 660 -height 380
	}
}

class ToroidView {

 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        toroid_vert_1 \
        toroid_vert \
        toroid_vert_2 \
        toroid_horz_1 \
        toroid_horz \
        toroid_horz_2 \
        toroid_yaw \
        toroid_pitch \
        toroid_bend \
        ]
    }

	constructor { args} {

		place $itk_component(control) -x 220 -y 300

      # draw the toroid
      rect_solid 180 120 250 20 40 60 40
 
      # create views for each motor
      motorView toroid_vert_1 190  90 s
      motorView toroid_vert   330  90 s
      motorView toroid_vert_2 470  90 s
      motorView toroid_horz_1 135 197 n
      motorView toroid_horz   275 197 n
      motorView toroid_horz_2 415 197 n
      motorView toroid_yaw    143 137 e
      motorView toroid_pitch  507 137 w
      motorView toroid_bend  507 260 w

      # create arrow for each motor
      motorArrow toroid_vert_1 210  90 {} 210 140 222 94 222 140
      motorArrow toroid_vert   330  90 {} 330 140     342 94 342 140
      motorArrow toroid_vert_2 450  90 {} 450 140 462 95 462 140
      motorArrow toroid_horz_1 190 170 {} 158 197 198 172 176 196
      motorArrow toroid_horz   305 170 {} 272 197 313 172 291 196
      motorArrow toroid_horz_2 430 170 {} 403 197 442 177 422 196
      motorArrow toroid_yaw    165 165 {125 137} 200 135 154 172 186 125
      motorArrow toroid_pitch  470  110 {500 120 500 160} 470 170 486 104 486 176
	  motorArrow toroid_bend   500 230 {} 500 300 512 230 512 300
 
      eval itk_initialize $args
      configure -width 660 -height 350
      configure -serialMove 1
   }
}

class ToroidView92 {
	
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""
	# create the canvas to draw the mirror in

    public proc getMotorList { } {
        return [list \
	    toroid_bend \
	    toroid_pitch \
	    toroid_vert \
	    toroid_yaw \
        beam_pipe_vert \
        ]
    }

    public method gotoMonoView { } {
        $itk_option(-mdiHelper) openToolChest mono
    }

	constructor { args} {
   
   rect_solid 70 90 200 10 20 30 20

	# draw the x-ray beam
	$itk_component(canvas) create line 49 40 185 100 330 40 -fill red -width 2 -arrow last

	motorView toroid_bend 195 60 s
	motorView toroid_pitch 331 97 w
	motorView toroid_vert 194 176 n
	motorView toroid_yaw 20 145 w 
    motorView mono_vert 470 270 e
    motorView beam_pipe_vert 520 270 w
	
    $itk_component(mono_vert) configure -entryWidth 10

	motorArrow toroid_vert 185 122 {} 185 172 196 128 196 166
	motorArrow toroid_pitch  290 70 {320 80 320 120} 290 130 306 64 303 137
	motorArrow toroid_yaw 55 110 {5 92} 80 90 47 118 72 78
	motorArrow mono_vert 480 200 {} 480 300 490 205 490 295
	motorArrow beam_pipe_vert 510 200 {} 510 300 500 205 500 295

	place $itk_component(control) -x 170 -y 300


    ###draw gird and box
    $itk_component(canvas) create rectangle 10 230 470 240 -fill #a0a0dd
    $itk_component(canvas) create line 470 230 470 10 10 10 -width 2 -dash .

    ### draw pipe line
    $itk_component(canvas) create rectangle 520 230 740 240 -fill #a0a0dd
    $itk_component(canvas) create line 520 230 520 10 740 10 740 230 -width 2 -dash .
    #draw stopper tank
    rect_solid 550 50 150 150 20 30 20
    $itk_component(canvas) create text 630 120 -text "Stopper Tank" -font *-courier-bold-r-normal--14-*-*-*-*-*-*-*
    

    #### link button
    itk_component add link {
        button $itk_component(canvas).link \
        -text "<== goto monoView" \
        -command "$this gotoMonoView"
    } {
    }
	place $itk_component(link) -x 10 -y 250 -anchor nw


   eval itk_initialize $args
   configure -width 760 -height 350
   configure -serialMove 0 
   }  
}

class BL12-2FocusingMirrorsView {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        focusing_mirror_1_horz \
        focusing_mirror_1_yaw \
        focusing_mirror_2_vert \
        focusing_mirror_2_pitch \
        focusing_mirror_2_vert_1 \
        focusing_mirror_2_vert_2 \
        focusing_mirror_1_bend_1 \
        focusing_mirror_1_bend_2 \
        focusing_mirror_2_bend_1 \
        focusing_mirror_2_bend_2 \
        ]
    }

    public method gotoUpStreamView { } {
        $itk_option(-mdiHelper) openToolChest mono
    }

	constructor { args} {

      puts $this

      loadBackdropImage BL12-2FocusingMirrors.gif

      place $itk_component(control) -x 353 -y 414

      # create views for each motor
      motorView focusing_mirror_1_bend_1 594 81 sw
      motorView focusing_mirror_1_bend_2 763 81 sw

      motorView focusing_mirror_2_bend_1 94 176 sw
      motorView focusing_mirror_2_bend_2 321 176 sw

      motorView focusing_mirror_1_horz 517 220 nw
      motorView focusing_mirror_1_yaw 690 220 nw

      motorView focusing_mirror_2_vert_1 10 338 nw
      motorView focusing_mirror_2_vert_2 358 338 nw

      motorView focusing_mirror_2_vert 218 104 sw
      motorView focusing_mirror_2_pitch 190 300 nw

      moveHotSpot focusing_mirror_1_bend_1 639 88 negative  
      moveHotSpot focusing_mirror_1_bend_1 604 123 positive 

      moveHotSpot focusing_mirror_1_bend_2 878 90 negative
      moveHotSpot focusing_mirror_1_bend_2 844 123 positive

      moveHotSpot focusing_mirror_2_bend_1 155 229 negative
      moveHotSpot focusing_mirror_2_bend_1 155 185 positive

      moveHotSpot focusing_mirror_2_bend_2 392 230 negative
      moveHotSpot focusing_mirror_2_bend_2 392 185 positive

      moveHotSpot focusing_mirror_1_horz 604 205 negative
      moveHotSpot focusing_mirror_1_horz 637 178 positive

      moveHotSpot focusing_mirror_1_yaw 704 209 negative
      moveHotSpot focusing_mirror_1_yaw 764 209 positive

      moveHotSpot focusing_mirror_2_vert_1 140 325 negative
      moveHotSpot focusing_mirror_2_vert_1 140 274 positive

      moveHotSpot focusing_mirror_2_vert_2 375 328 negative
      moveHotSpot focusing_mirror_2_vert_2 375 274 positive

      moveHotSpot focusing_mirror_2_vert 282 208 negative
      moveHotSpot focusing_mirror_2_vert 282 119 positive

      moveHotSpot focusing_mirror_2_pitch 200 276 negative
      moveHotSpot focusing_mirror_2_pitch 261 274 positive

      itk_component add link_us {
         button $itk_component(canvas).ds \
            -text "<==goto Mono View" \
            -command "$this gotoUpStreamView"
      } {
      }

      place $itk_component(link_us) -x 28 -y 31 -anchor nw
      eval itk_initialize $args
      configure -width 952 -height 466
   }
}



class BL14-1FocusingMirrorsView {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        focusing_mirror_vert_1 \
        focusing_mirror_vert_2 \
        focusing_mirror_yaw \
        focusing_mirror_pitch \
        focusing_mirror_vert \
        focusing_mirror_bend \
        ]
    }

    constructor { args} {

        puts $this

        loadBackdropImage BL14-1FocusingMirror.gif

        place $itk_component(control) -x 132 -y 388

        # create views for each motor
        motorView focusing_mirror_vert_1 115 92 sw
        motorView focusing_mirror_vert_2 290 92 sw

        motorView focusing_mirror_yaw 28 197 nw
        motorView focusing_mirror_pitch 385 191 sw

        motorView focusing_mirror_vert 185 249 nw
        motorView focusing_mirror_bend 452 325 sw

        moveHotSpot focusing_mirror_vert_1 175 109 positive false
        moveHotSpot focusing_mirror_vert_1 175 147 negative false

        moveHotSpot focusing_mirror_vert_2 335 109 positive false
        moveHotSpot focusing_mirror_vert_2 335 146 negative false
    
        moveHotSpot focusing_mirror_yaw 104 188 positive false
        moveHotSpot focusing_mirror_yaw 127 147 negative false

        moveHotSpot focusing_mirror_pitch 362 134 positive false
        moveHotSpot focusing_mirror_pitch 358 208 negative false

        moveHotSpot focusing_mirror_vert 252 198 positive false
        moveHotSpot focusing_mirror_vert 252 236 negative false

        eval itk_initialize $args
        configure -width 617 -height 423
    }
}

class ID19MirrorView {
        inherit ::DCS::CanvasGifView

        itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
                mirror_slit_upper \
                mirror_slit_lower \
		mirror_slit_ring \
		mirror_slit_lobs \
                mirror_slit_vert \
                mirror_slit_vert_gap \
		mirror_slit_horz \
		mirror_slit_horz_gap \
                mirror_bend_1 \
                mirror_bend_2 \
                mirror_bend \
                mirror_roll \
                mirror_pitch \
		mirror_vert \
		mirror_horz \
        ]
    }
   
    public method handleBeamCurrentChange
 
    private variable m_deviceFactory
    private variable m_strBeamCurrent

   constructor { args} {

    if {[catch {
        loadBackdropImage id19-mirror-yang-9.gif
    } errMsg]} {
        log_error failed to load image: $errMsg
    }

    motorView mirror_slit_upper 10 70 sw
    motorView mirror_slit_lower 170 70 sw
    motorView mirror_slit_vert 10 330 nw
    motorView mirror_slit_vert_gap 170 330 nw

    motorView mirror_slit_lobs 10 100 nw
    motorView mirror_slit_ring 10 170 nw
    motorView mirror_slit_horiz 10 250 nw
    motorView mirror_slit_horiz_gap 170 250 nw

#    motorArrow mirror_slit_vert 35 115 {} 35 305 25 120 25 300
#    motorArrow mirror_slit_vert_gap 35 320 {} 35 365 25 325 25 360

   motorView mirror_bend_1 350 150 sw
   motorView mirror_bend_2 560 150 sw
   motorView mirror_roll 350 290 nw
   motorView mirror_bend 440 71 sw
   motorView mirror_pitch 596 200 nw
   motorView mirror_vert 560 280 nw
   motorView mirror_horz 560 360 nw

   for {set i 0} {$i < 2} {incr i} {

   	itk_component add ioncurrent$i {
        	# make the optimize beam button
                label $itk_component(canvas).ioncurrent$i \
                      -text " " \
                      -relief sunken \
                      -width 8 \
                      -background #c0c0ff \
                      -activebackground #c0c0ff
                } {
                           keep -foreground
                }
       }
       place $itk_component(ioncurrent0) -x 350 -y 155
       place $itk_component(ioncurrent1) -x 560 -y 155


#   moveHotSpot mirror_slit_upper 176 36 positive
#   moveHotSpot mirror_slit_upper 176 73 negative

#   moveHotSpot mirror_slit_lower 162 189 positive
#   moveHotSpot mirror_slit_lower 162 229 negative
   
#   moveHotSpot mirror_bend_downstream 314 93 positive
#   moveHotSpot mirror_bend_downstream 314 132 negative

#   moveHotSpot mirror_bend_upstream 574 93 positive
#   moveHotSpot mirror_bend_upstream 574 132 negative

#   moveHotSpot mirror_bend 728 332 positive
#   moveHotSpot mirror_bend 728 372 negative
   
#   moveHotSpot mirror_slit_vert 458 142 positive
#   moveHotSpot mirror_slit_vert 458 182 negative

#   moveHotSpot mirror_roll 596 243 positive
#   moveHotSpot mirror_roll 596 283 negative

#   moveHotSpot mirror_pitch 577 200 positive
#   moveHotSpot mirror_pitch 577 240 negative

        eval itk_initialize $args

	::mediator announceExistence $this
	set m_deviceFactory [DCS::DeviceFactory::getObject]
        set m_strBeamCurrent [$m_deviceFactory createString analogInStatus7]
        $m_strBeamCurrent register $this contents handleBeamCurrentChange
   }

   destructor { }

}

body ID19MirrorView::destructor {} {
        $m_strBeamCurrent unregister $this contents handleBeamCurrentChange
        mediator announceDestruction $this
}

body ID19MirrorView::handleBeamCurrentChange { name_ targetReady_ - contents_ - } {
    if { ! $targetReady_} return
#puts "contents=$contents_ \n"
    if { $contents_ == "" } return

    for {set i 0} {$i < 2} {incr i} {
           set value [lindex $contents_ $i]
           $itk_component(ioncurrent$i) configure \
                -text [format "%.5f" $value] \
                -state normal
    }

    #if {[string is double -strict $contents_]} {
    #    set display [format "%.3f" $contents_]
    #} else {
    #    set display $contents_
    #}
    #$itk_component(ioncurrent) configure -text $display
#    log_error "yangxx display=$display"
}



class BL7-1MirrorView {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
		mirror_slit_upper \
		mirror_slit_lower \
		mirror_slit_vert \
		mirror_slit_vert_gap \
		mirror_vert_1 \
		mirror_vert_2 \
		mirror_vert \
		mirror_vert_chin \
		mirror_bend \
		mirror_pitch \
        ]
    }

	constructor { args} {
   
    if {[catch {
        loadBackdropImage BL7-1Mirror.gif
    } errMsg]} {
        log_error failed to load image: $errMsg
    }

    motorView mirror_slit_upper 35 80 sw
    motorView mirror_slit_lower 170 185 nw
    motorView mirror_slit_vert 40 260 nw
    motorView mirror_slit_vert_gap 40 320 nw
    motorArrow mirror_slit_vert 35 115 {} 35 305 25 120 25 300
    motorArrow mirror_slit_vert_gap 35 320 {} 35 365 25 325 25 360

   motorView mirror_vert_1 256 81 sw
   motorView mirror_vert_2 518 81 sw
   motorView mirror_vert 387 131 sw
   motorView mirror_vert_chin 381 288 nw
   motorView mirror_bend 577 313 nw
   motorView mirror_pitch 596 178 nw


   moveHotSpot mirror_slit_upper 176 36 positive
   moveHotSpot mirror_slit_upper 176 73 negative

   moveHotSpot mirror_vert_1 314 93 positive
   moveHotSpot mirror_vert_1 314 132 negative

   moveHotSpot mirror_vert_2 574 93 positive
   moveHotSpot mirror_vert_2 574 132 negative

   moveHotSpot mirror_vert 458 142 positive
   moveHotSpot mirror_vert 458 182 negative

   moveHotSpot mirror_slit_lower 162 189 positive
   moveHotSpot mirror_slit_lower 162 229 negative

   moveHotSpot mirror_vert_chin 458 243 positive
   moveHotSpot mirror_vert_chin 458 283 negative

   moveHotSpot mirror_bend 728 332 positive
   moveHotSpot mirror_bend 728 372 negative

   moveHotSpot mirror_pitch 577 177 positive
   moveHotSpot mirror_pitch 577 240 negative

	eval itk_initialize $args
   }

}


class BL14-1MirrorView {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
		mirror_slit_upper \
		mirror_slit_lower \
		mirror_vert_1 \
		mirror_vert_2 \
		mirror_vert \
		mirror_vert_chin \
		mirror_slit_lower \
		mirror_slit_upper \
		mirror_slit_vert \
		mirror_slit_vert_gap \
		mirror_slit_horz \
		mirror_bend \
		mirror_pitch \
        ]
    }

	constructor { args} {
   
    loadBackdropImage BL14-1Mirror.gif

    motorView mirror_slit_upper 86 175 sw
    motorView mirror_vert_1 469 70 sw
    motorView mirror_vert_2 740 70 sw
    motorView mirror_vert 607 104 sw
    motorView mirror_slit_lower 42 252 nw
    motorView mirror_slit_vert 218 112 sw
    motorView mirror_slit_vert_gap 249 250 nw
    motorView mirror_slit_horz 414 241 nw
    motorView mirror_bend 795 303 sw
    motorView mirror_pitch 816 166 sw
    motorView mirror_vert_chin 599 265 sw

    moveHotSpot mirror_slit_upper 236 128 positive false
    moveHotSpot mirror_slit_upper 236 164 negative false

    moveHotSpot mirror_vert_1 592 85 positive false
    moveHotSpot mirror_vert_1 592 120 negative false

    moveHotSpot mirror_vert_2 769 85 positive false
    moveHotSpot mirror_vert_2 769 120 negative false

    moveHotSpot mirror_vert 683 113 positive false
    moveHotSpot mirror_vert 681 155 negative false

    moveHotSpot mirror_slit_lower 218 280 positive false
    moveHotSpot mirror_slit_lower 218 320 negative false

    moveHotSpot mirror_slit_vert 376 74 positive false
    moveHotSpot mirror_slit_vert 376 111 negative false
    #moveHotSpot mirror_slit_vert_gap 249 255 positive
    #moveHotSpot mirror_vert_chin 599 265 negative

    moveHotSpot mirror_pitch 798 186 positive false
    moveHotSpot mirror_pitch 798 116 negative false

    moveHotSpot mirror_slit_vert_gap 315 203 positive false
    moveHotSpot mirror_slit_vert_gap 315 243 negative false

    moveHotSpot mirror_slit_horz 502 190 positive false
    moveHotSpot mirror_slit_horz 502 229 negative false

	eval itk_initialize $args
   }

}



class BL14-1MonoView {
 	inherit ::DCS::CanvasGifView

	itk_option define -mdiHelper mdiHelper MdiHelper ""

    public proc getMotorList { } {
        return [list \
        mono_slit_upper \
        mono_slit_vert \
        mono_slit_vert_gap \
        mono_slit_lower \
        mono_roll \
        mono_theta \
        mono_pitch \
        energy \
        ]
    }

	constructor { args} {
   
    loadBackdropImage BL14-1Mono.gif

    motorView mono_slit_upper 74 84 sw
    motorView mono_slit_vert_gap 62 185 nw
    motorView mono_slit_vert 265 100 sw
    motorView mono_slit_lower 206 250 nw
    motorView mono_roll 418 156 sw
    motorView mono_theta 456 270 nw
    motorView mono_pitch 648 158 sw
    motorView energy 647 232 nw
    
    moveHotSpot mono_slit_upper 210 63 positive false
    moveHotSpot mono_slit_upper 207 98 negative false

    moveHotSpot mono_slit_vert_gap 105 135 positive false
    moveHotSpot mono_slit_vert_gap 105 174 negative false

    moveHotSpot mono_slit_vert 361 109 positive false
    moveHotSpot mono_slit_vert 361 148 negative false

    moveHotSpot mono_slit_lower 258 207 postive false
    moveHotSpot mono_slit_lower 258 242 negative false

    moveHotSpot mono_roll 468 162 positive false
    moveHotSpot mono_roll 468 189 negative false

    moveHotSpot mono_theta 552 239 positive false
    moveHotSpot mono_theta 473 239 negative false

    moveHotSpot mono_pitch 630 215 positive false
    moveHotSpot mono_pitch 630 142 negative false

	eval itk_initialize $args
   }

}


