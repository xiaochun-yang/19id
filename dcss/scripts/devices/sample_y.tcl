# sample_y.tcl
# This is for x4a crystal logic goniometer z combo motion.
# tripot_1, tripot_2 and tripot_3 are equal distance to the
# It is 22.225mm. The center of tripot support surface to the
# sample is 97 mm (alone z direction).
# The X aixs will be the line going through tripot_3 and the 
# middle point of tripot_1 and tripot_2 (pass the center as well).
# The y direction will be going through center and parallel to 
# tripot_1 and tripot_2. z direction will be perpendicular to
# x-y sourface.

proc sample_y_initialize {} {

	# specify children devices
	set_children tripot_1 tripot_2 tripot_3
	set_siblings sample_x sample_z
}


proc sample_y_move { new_sample_y } {
	#global 
	global gDevice

	# global variables
    variable sample_x
    variable sample_z

	# calculate new positions of the two motors
	set new_tripot_1 [tripot_1_calculate_y $new_sample_y $gDevice(sample_x,target) $gDevice(sample_z,target)]
	set new_tripot_2 [tripot_2_calculate_y $new_sample_y $gDevice(sample_x,target) $gDevice(sample_z,target)]
	set new_tripot_3 [tripot_3_calculate_y $new_sample_y $gDevice(sample_z,target)]

	#check to see if the move can be completed by the real motors
	assertMotorLimit tripot_1 $new_tripot_1
	assertMotorLimit tripot_2 $new_tripot_2
	assertMotorLimit tripot_3 $new_tripot_3

	# move motors in order that avoids collisions
	move tripot_1 to $new_tripot_1
	move tripot_2 to $new_tripot_2
	move tripot_3 to $new_tripot_3

	# wait for the moves to complete
	wait_for_devices tripot_1 tripot_2 tripot_3
}

proc sample_y_set { new_sample_y } {

	# global variables
	variable tripot_1
	variable tripot_2
	variable tripot_3
	variable sample_x
	variable sample_z

	# set three motors
	set tripot_1 [tripot_1_calculate_y $new_sample_y $sample_x $sample_z]
	set tripot_2 [tripot_2_calculate_y $new_sample_y $sample_x $sample_z]
	set tripot_3 [tripot_3_calculate_y $new_sample_y $sample_z]
}


proc sample_y_update {} {

	# global variables
    variable tripot_1
    variable tripot_2
    variable tripot_3

	# calculate from real motor positions and motor parameters
	return [sample_y_calculate $tripot_1 $tripot_2 $tripot_3]
}


proc sample_y_calculate { t1 t2 t3 } {
	
    #return [expr 97*($t2-$t1)/2/19.225]
    return [expr (2*$t3-$t1-$t2)*97/3/22.225 ]

}

proc tripot_1_calculate_y { y x z } {

#	set temp [expr ($z + $x*11.1125/97 - $y*19.225/97)]
#	puts "yangx y=$y x=$x z=$z"

    return [expr ($z - $y*11.1125/97 - $x*19.225/97)]
}


proc tripot_2_calculate_y { y x z } {

    return [expr ($z - $y*11.1125/97 + $x*19.225/97)]
}

proc tripot_3_calculate_y { y z } {

       return [expr ($z + $y*22.225/97.0)  ]
}

