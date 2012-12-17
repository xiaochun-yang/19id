#
#
#                        Copyright 2003
#                              by
#                 The Board of Trustees of the 
#               Leland Stanford Junior University
#                      All rights reserved.
#
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


proc centerLoop_initialize {} {
	variable centerLoopMaxRetries
	variable centerLoopMinZoom
	variable centerLoopMedZoom
	variable centerLoopMaxZoom
    variable centerLoopNormalPinSize
    set centerLoopNormalPinSize 400


	# give the namespace variables initial values
	set centerLoopMaxRetries 4
	set centerLoopMinZoom 0.0
	set centerLoopMedZoom 0.6
	set centerLoopMaxZoom 0.8
} 


proc centerLoop_start {{skipSaveRestoreLights 0}} {

	# access namespace variables
	variable centerLoopMaxRetries
	variable centerLoopMinZoom

    set restore_lights 1
    if {[catch {
        centerSaveAndSetLights $skipSaveRestoreLights
    } errMsg]} {
        log_warning lights control failed $errMsg
        set restore_lights 0
    }
    if {$skipSaveRestoreLights} {
        set restore_lights 0
    }

	# try centering up to specified number of times
	set tryCount 0
	while { $tryCount < $centerLoopMaxRetries } {

		# try to center the loop
		set errorStatus [catch { 

			# put the tip of the loop at the center of the system
			centerOnLoopTip loopLength

			# collect 18 images of the sample at 10.0 degree steps in phi
			createImageList 18 10.0 $loopLength

			# phase I worked. Try phase II
			centerOnBoundingBox 18 10.0

		} errorResult ] 
		
		if { $errorStatus == 0 } {
            if {$restore_lights} {
                centerRestoreLights
            }
		
			log_note "Loop centering was successful."
			return
	
		} else {		
			switch $errorResult {
            "error PhaseICenterResultIsNotGood" {
					log_error "Error from camera DHS."
            }
				TipNotInView  {
					log_error "Loop tip is not visible."
				}
				loopTooBig {
                    centerLoop_goodLuck
                    if {$restore_lights} {
                        centerRestoreLights
                    }
					log_error "Loop is too big to center."
					return -code error $errorResult
				}
				default {
                    centerLoop_goodLuck
                    if {$restore_lights} {
                        centerRestoreLights
                    }
					log_note "Error centering loop: $errorResult"
					return -code error $errorResult
				}
			}
			incr tryCount
		}
	}
    centerLoop_goodLuck
    if {$restore_lights} {
        centerRestoreLights
    }
	
	log_error "Exceeded retry limit for centering loop"
	return -code error "Exceeded retry limit for centering loop"
}


proc centerOnLoopTip { loopLengthRef } {

	# access namespace variables
	variable centerLoopMinZoom
	variable centerLoopMaxZoom
	
	upvar $loopLengthRef loopLength

	# center on loop tip at minimum zoom 
	centerTipAtZoomLevel $centerLoopMinZoom 

	# find the position of the pin base and store in reference variable
	determineLoopLength loopLength

    if {$loopLength == 0} {
        puts "loopLength failed"
    }

	# center on loop tip at minimum zoom 
	centerTipAtZoomLevel $centerLoopMaxZoom
}


proc centerTipAtZoomLevel { zoomLevel } {

	# go to the appropriate zoom level
	move camera_zoom to $zoomLevel
	wait_for_devices camera_zoom

	# center the tip of the loop in plane, iterating up to 4 times if needed
	centerLoopTip 4 0.2

	# center the tip 90 out of plane by rotating only 15 degrees
	centerOutOfPlane 15.0

	# center the tip of the loop in plane one more time
	centerLoopTip 1 0.1
}


proc determineLoopLength { loopLengthRef } {
	upvar $loopLengthRef loopLength
    variable camera_zoom
    variable centerLoopNormalPinSize

    set sampleImageWidth  [getSampleCameraConstant sampleImageWidth]
	getSampleScaleFactor scaleFactor null

    set pinSizeHint [expr int($centerLoopNormalPinSize / $scaleFactor)]
    if {$pinSizeHint < 1} {
        set pinSizeHint 1
    }
	
	# request the pin tip position and pin base position
	set handle [start_waitable_operation getLoopTip $pinSizeHint]
	set result [wait_for_operation $handle 30000]

	# parse the result
	set tipX [lindex $result 1]
	set baseX [lindex $result 3]

	# calculate distance between pin tip and base
	set pinBaseTipDistance [expr $tipX - $baseX]
	
	# handle cases of no pin base in image or pin base close to tip 
	if { $baseX < 0.001 || $pinBaseTipDistance <= 0.05} {
		
		# store a value of zero in the reference varialbe
		set loopLength 0

	} else {
		# calculate pin tip-base distance in um and store in reference variable
		set loopLength [expr $sampleImageWidth * $scaleFactor * $pinBaseTipDistance]
        puts "DEBUGCenterLoop: loop length: $pinBaseTipDistance in units: $loopLength scaleFactor: $scaleFactor zoom: $camera_zoom"
	}
}


proc createImageList { imageCount deltaPhi loopLength } {

	# Get Current Zoom Level's Rotation Axis
	variable centerLoopMedZoom
	variable centerLoopMaxZoom

    set sampleImageHeight  [getSampleCameraConstant sampleImageHeight]
	# try to collect images at two different zoom levels if needed
	foreach zoomLevel "$centerLoopMaxZoom $centerLoopMedZoom" {
	
		# move camera zoom
		move camera_zoom to $zoomLevel
		wait_for_devices camera_zoom

		# move the loop as needed to hide the base of the pin
		positionLoopForImaging $loopLength

		set imagingResult "OK"

		# try to collect the collected images
	   for { set i 0 } { $i < $imageCount } { incr i } {
	
			set handle [start_waitable_operation addImageToList $i]
			set result [wait_for_operation $handle 30000]

			# extract the curent loop height from the result
            set loopHeight [lindex $result 1]
			set loopHeightRelative [lindex $result 2]
            if {![string is double -strict $loopHeightRelative]} {
                ### old cameraDHS
                set loopHeightRelative \
                [expr double($loopHeight) / $sampleImageHeight]
            }
	
            # give up on this zoom level if the loop occupies more than 75% of the height of the image
            if { $loopHeightRelative > 0.75 } {	
				set imagingResult "loopTooBig"
				break	
			}

			# rotate phi in preparation for next image
			move gonio_phi by $deltaPhi
			wait_for_devices gonio_phi
		}
		
		# break out of loop if imaging went OK
		if { $imagingResult == "OK" } {
			break
		}
	}

	# throw exception if imaging was not OK
	if { $imagingResult != "OK" } {
		return -code error loopTooBig
	}
}


proc positionLoopForImaging { loopLength } {
    variable camera_zoom
    variable gonio_phi
    set sampleImageWidth  [getSampleCameraConstant sampleImageWidth]

	# get pin tip position
	determinePinTipPosition tipX tipY

    set start_phi $gonio_phi

	# get the scale factor for the current zoom level
	getSampleScaleFactor scaleFactor null


    if {$loopLength == 0} {
        set steps 3
        set delta [expr 90.0 / ($steps - 1)]
        for {set i 0} {$i < $steps} {incr i} {
            move gonio_phi to [expr $start_phi + $delta * $i]
            wait_for_devices gonio_phi
	        determineLoopLength newLength
            puts "new loop new length: $newLength ati=$i "
            if {$newLength > $loopLength} {
                set loopLength $newLength
            }
        }
        puts "new loop length: $loopLength in zoom=$camera_zoom"
    }

	# calculate width of the sample camera image in um
	set screenWidth [expr $scaleFactor * $sampleImageWidth]

	# calculate the fraction of the image width the loop length would occupy
	set loopLengthUnscaled [expr $loopLength / $screenWidth ]

    puts "DEBUGCenterLoop: length: $loopLength unscaled: $loopLengthUnscaled scalefactor: $scaleFactor zoom: $camera_zoom"

	# move pin tip such that the pin tip will not show
	if { $loopLengthUnscaled <= 0 || $loopLengthUnscaled > 0.75 } {
	
		# move tip to 0.8 in horizontal				
		moveSampleRelative [expr 0.8 - $tipX] 0	
	
	} else {
        puts "centerLoop: hide base"

		# move tip such that pin base is just hidden
		moveSampleRelative [expr $loopLengthUnscaled - $tipX - 0.05] 0
	}
}


proc centerOnBoundingBox { imageCount deltaPhi } {
    variable save_loop_size
    variable sample_x
    variable sample_y
    variable sample_z
    variable gonio_phi


    set sampleImageWidth   [getSampleCameraConstant sampleImageWidth]
    set sampleImageHeight  [getSampleCameraConstant sampleImageHeight]
    set zoomMaxXAxis       [getSampleCameraConstant zoomMaxXAxis]
    set zoomMaxYAxis       [getSampleCameraConstant zoomMaxYAxis]

	# get bounding box
	set handle [start_waitable_operation findBoundingBox Both Both]
	set result [wait_for_operation $handle 30000]
    set save_loop_size $result
	
	# extract orientation and dimensions of the bounding box from the result
    set box_status   [lindex $result 0]
	set maxImgIndex  [lindex $result 1]
	set minImgIndex  [lindex $result 2]
	set faceLeftX    [lindex $result 3]
	set faceUpperY   [lindex $result 4]
	set faceRightX   [lindex $result 5]
	set faceLowerY   [lindex $result 6]
	set edgeUpperY   [lindex $result 8]
	set edgeLowerY   [lindex $result 10]

    if {$box_status == "normal"} {
        if {abs( $maxImgIndex - $minImgIndex ) != $imageCount / 2} {
            log_warning loop face and edge not 90 degree away
            log_warning face: $maxImgIndex edge: $minImgIndex
        }
            
        set faceWidth [expr abs($faceRightX - $faceLeftX)]
        set faceHeight [expr abs($faceUpperY - $faceLowerY)]
        set edgeHeight [expr abs($edgeUpperY - $edgeLowerY)]

        getSampleScaleFactor umPerPixelHorizontal umPerPixelVertical

        set scaleX [expr $sampleImageWidth * $umPerPixelHorizontal / 1000.0]
        set scaleY [expr $sampleImageHeight * $umPerPixelVertical / 1000.0]

        set faceWmm [expr $faceWidth * $scaleX]
        set faceHmm [expr $faceHeight * $scaleY]
        set edgeHmm [expr $edgeHeight * $scaleY]
        #set save_loop_size "normal $faceWmm $faceHmm $edgeHmm $sample_x $sample_y $sample_z $gonio_phi"
    }

	# calculate delta phi to face-on and edge-on views of sample
	set faceOnDeltaPhi [expr -1.0 * $deltaPhi * ($imageCount - $maxImgIndex - 1)]
	set edgeOnDeltaPhi [expr -1.0 * $deltaPhi * ($imageCount - $minImgIndex - 1)]
	
	# move center of face to center of image
    # here 0.6 is because the loop is not an ellipse.
    # the center we want is closer to the loop head, not tail.
	set deltaX [expr $zoomMaxXAxis - $faceLeftX - ( $faceRightX - $faceLeftX ) * 0.6 ]
	set deltaY [expr $zoomMaxYAxis - ($faceUpperY + $faceLowerY) * 0.5]
	moveSampleRelative $deltaX $deltaY $faceOnDeltaPhi
	
	# move center of edge to center of image
	set deltaY [expr $zoomMaxYAxis - ($edgeUpperY + $edgeLowerY) * 0.5 ]
	moveSampleRelative 0 $deltaY $edgeOnDeltaPhi
	
	# move phi to view loop face-on
    puts "old face delta: $faceOnDeltaPhi"
    while {abs($faceOnDeltaPhi) > 90.0} {
        if {$faceOnDeltaPhi > 0.0} {
            set faceOnDeltaPhi [expr $faceOnDeltaPhi - 180.0]
        } else {
            set faceOnDeltaPhi [expr $faceOnDeltaPhi + 180.0]
        }
    }
    puts "new face delta: $faceOnDeltaPhi"
	move gonio_phi by $faceOnDeltaPhi
	wait_for_devices gonio_phi

    if {$box_status == "normal"} {
        set save_loop_size "normal $faceWmm $faceHmm $edgeHmm $sample_x $sample_y $sample_z $gonio_phi"

        ###### DEBUG: collect statistic data
        set debugStatus [catch {
	        set handle [start_waitable_operation getLoopTip -1]
	        set result [wait_for_operation $handle 30000]
            set isMicroMount [lindex $result 3]
            if {$isMicroMount == "1"} {
                log_warning microMount detected
                set save_loop_size "normal $faceWmm $faceHmm $edgeHmm $sample_x $sample_y $sample_z $gonio_phi 1"
            }
        } errMsg]
        if {$debugStatus == -1} {
            puts "DEBUG log failed: $errMsg"
        }
    }
}


proc centerLoopTip { maxAttempts maxError } {
    set zoomMaxXAxis       [getSampleCameraConstant zoomMaxXAxis]
    set zoomMaxYAxis       [getSampleCameraConstant zoomMaxYAxis]

	# iterate as requested
	for { set attempt 0} { $attempt < $maxAttempts } { incr attempt } {

		# look for the tip position
		determinePinTipPosition tipX tipY

		# calculate the correction needed to bring tip to center
		set deltaX [expr $zoomMaxXAxis - $tipX]
		set deltaY [expr $zoomMaxYAxis - $tipY]

		# make the correction of the sample position
		moveSampleRelative $deltaX $deltaY
		
		# break out of loop if correction was small
		if { sqrt($deltaX * $deltaX + $deltaY * $deltaY) < $maxError } {
			break
		}
	}

	# throw an exception if multiple tip centerings did not converge
	if { $attempt > $maxAttempts && $maxAttempts > 1 } {
		return -code error maxAttemptsExceeded
	}
	
	# NEED TO OPTIONALLY REPORT NUMBER OF RETRIES AS A WARNING TO BLU-ICE
}


proc centerOutOfPlane { rotationAngle } {
    set zoomMaxYAxis       [getSampleCameraConstant zoomMaxYAxis]

	# calculate difference between requested rotation angle 
	# and 90 degrees
	set correctionAngle [expr 90.0 - $rotationAngle]

	# rotate loop the the requested angle
	move gonio_phi by $rotationAngle
	wait_for_devices gonio_phi
	
	# find the loop tip in this partially rotated frame
	determinePinTipPosition tipX tipY
	
	# calculate how far the tip would be from the rotation axis 
	#if rotation had been 90 degrees
	set deltaY [expr ( $zoomMaxYAxis - $tipY ) / cos($correctionAngle / 180.0 * 3.141593) ]
	
	# move the sample by that amount in the 90-degree rotated reference frame
	moveSampleRelative 0 $deltaY $correctionAngle
}


proc centerSaveAndSetLights { {skipSaveRestoreLights 0} } {
    if {!$skipSaveRestoreLights} {
        lightsControl_start save
    }
    if {[lightsControl_start setup sample_tip]} {
        log_warning wait for lights stable
        wait_for_time 2000
    }
}
proc centerRestoreLights { } {
    if {[catch {
        lightsControl_start restore
    } errMsg]} {
        log_warning "restore lights failed: $errMsg"
    }
}
#### this is called when it failed to center it
#### it will move the tip to the right side ofthe beam size box
proc centerLoop_goodLuck { } {
    global gMotorBeamWidth
	variable centerLoopMaxZoom
    variable $gMotorBeamWidth

    puts "GOODLUCK centerLoop"
    if {[catch {
	    centerOnLoopTip loopLength
        set wid [set $gMotorBeamWidth]
        set dis [expr $wid / 2.0]
        move sample_z by $dis
        wait_for_devices sample_z
        puts "GOODLUCK centerLoop OK"
    } errMsg]} {
        puts "GOODLUCK centerLoop failed: $errMsg"
    }
}
