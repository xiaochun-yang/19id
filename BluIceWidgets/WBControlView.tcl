##########################################################################
#only for 19ID at NSLS2

package provide BLUICEWBControl 1.0

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

class DCS::WBControlWidget {
 	inherit ::DCS::CanvasShapes

	itk_option define -mdiHelper mdiHelper MdiHelper ""

 public proc getMotorList { } {
        return [list \
        white_beam_filter1 \
        white_beam_filter2 \
        white_beam_filter_1 \
        white_beam_filter_2 \
        white_beam_mask_x \
        white_beam_mask_y \
        white_beam_slit_upper \
        white_beam_slit_lower \
	white_beam_slit_vert  \
	white_beam_slit_vert_gap
        ]
    }

        constructor { args} {

                place $itk_component(control) -x 150 -y 445

                # construct the table widgets
                motorView white_beam_filter1 125  90 sw
                motorView white_beam_filter2 300 90 sw
                motorView white_beam_filter_1 125 170 sw
                motorView white_beam_filter_2 300 170 sw
                motorView white_beam_mask_x 125 250 sw
                motorView white_beam_mask_y 300 250 sw
                motorView white_beam_slit_lower 125 320 sw
                motorView white_beam_slit_upper 300 320 sw
                motorView white_beam_slit_vert 125 390 sw
                motorView white_beam_slit_vert_gap 300 390 sw

                # draw the table
                #rect_solid 180 120 250 20 40 60 40
		itk_component add filter1 {
                 label $itk_component(canvas).filter1 \
                      -text "#: Filter1\n1:  500um\n2:  250um\n3:  1 mm \n4:  open " \
                      -relief flat \
                      -width 10 \
              	} {
                           keep -foreground
              	}
		itk_component add motor1 {
                 label $itk_component(canvas).motor1 \
                      -text "Motor Pos\n1:  -35.5 mm\n2:  -17.5 mm\n3:  1.00 mm \n4:  19.2 mm " \
                      -relief flat \
                      -width 12 \
              	} {
                           keep -foreground
              	}
		itk_component add filter2 {
                  label $itk_component(canvas).filter2 \
                      -text "#: Filter2\n5:  Open \n6:  open \n7:  2 mm \n8:  Open " \
                      -relief flat \
                      -width 10 \
              	} {
                           keep -foreground
              	}
		itk_component add motor2 {
                 label $itk_component(canvas).motor2 \
                      -text "Motor Pos\n5:  -38.3 mm\n6:  -20.0 mm\n7:  -2.50 mm\n8:  16.23 mm" \
                      -relief flat \
                      -width 12 \
              	} {
                           keep -foreground
              	}
		itk_component add unit1 {
                  # make the optimize beam button
                  label $itk_component(canvas).unit1 \
                      -text "Num" \
                      -relief flat \
                      -width 4 \
              	} {
                           keep -foreground
              	}
		itk_component add unit2 {
                  # make the optimize beam button
                  label $itk_component(canvas).unit2 \
                      -text "Num" \
                      -relief flat \
                      -width 4 \
              	} {
                           keep -foreground
              	}
		place $itk_component(filter1) -x 20 -y 100 -anchor sw
		place $itk_component(motor1)  -x 20 -y 190 -anchor sw
		place $itk_component(filter2) -x 450 -y 100 -anchor sw
		place $itk_component(motor2) -x 450 -y 190 -anchor sw
		place $itk_component(unit1) -x 220 -y 85 -anchor sw
		place $itk_component(unit2) -x 395 -y 85 -anchor sw
		
              #        -background #c0c0ff \
              #        -activebackground #c0c0ff

                eval itk_initialize $args
                $itk_component(canvas) configure -width 560 -height 500

                configure -serialMove 1
        }
}
