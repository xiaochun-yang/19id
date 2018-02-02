### SPEAR now uses gapReady to indicate gap is ready and we have the ownership.

proc 19id_gap_initialize {} {

	set_children
	set_siblings

    registerAbortCallback 19id_gap_abort
}

proc 19id_gap_move { pos } {
    variable gapRequest
    variable gapStatus
    variable gap

    #set gapRequest $pos will move the gap to the $pos		
    puts "yangx right before moving gap gapStatus = $gapStatus"
    set gapRequest $pos
    puts "yangx right after  moving gap gapStatus = $gapStatus"
#   or usr caput command to set the new position
#   set a [exec caput $gap $pos] 

#    vwait gGapMoveDoneFlag

    #loop gapStatus to check if it's finished
#    while {!gapStatus} {
	#wait for 500 ms
	after 500
#    }
     puts "yangx after moving gap gapStatus = $gapStatus"

    return

    set diffFromDesired [expr abs($gap - $pos)]
    if {$diffFromDesired > 0.010} {
        wait_for_time 1000
        set newDiff [expr abs($gap - $pos)]
        if {$newDiff < $diffFromDesired} {
            log_severe after extra waiting for 1 second, 19id diff reduced to $newDiff from $diffFromDesired
            set diffFromDesired $newDiff
        }
    }

    if {$diffFromDesired > 0.020} {
        log_severe gap $gap not reach the desired position $pos
        return -code error move_gap_failed
    }
    if {$diffFromDesired > 0.002} {
        log_warning gap $gap not reach the desired position $pos
    }
}

##this is the proc called in config
#we will use this to inform user that config is not supported
proc 19id_gap_set { new_19id_gap } {
    log_error Cannot config 19id_gap.  Please do it through EPICS

    undltrUpdateConfig

    return -code error "config not supported"
}

proc 19id_gap_update {} {
    variable gap
    return $gap
}

proc undltrUpdateConfig { } {
    variable gap
    variable gapRequestDrvH
    variable gapRequestDrvL

    dcss2 sendMessage "htos_configure_device 19id_gap $gap $gapRequestDrvH $gapRequestDrvL 1 1 0"
}

proc 19id_gap_abort { } {
    variable gapAbort

    log_warning aborted 19id gap too
    set gapAbort 1
