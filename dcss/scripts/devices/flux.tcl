### used by ion chamber i_flux
global gFluxIonChamberTimeScaled
set gFluxIonChamberTimeScaled 0
proc flux_initialize {} {
    namespace eval ::fluxLUT { 
        array set condition [list]
        array set flux_lut [list]
        array set stringFieldIndex [list]
        array set ionChamberTimeScaled [list]

        set lut_length 0
        set last_reading 0

    }

    flux_loadLUT

    variable ::fluxLUT::stringFieldIndex
    variable ::fluxLUT::ionChamberTimeScaled
    set flux_triggerList [list]
    set triggerList [list]
    ::config getRange flux.trigger flux_triggerList

    foreach trigger $flux_triggerList {
        if {[llength $trigger] > 1} {
            foreach {name index} $trigger break
            set stringFieldIndex($name) $index
            #puts "flux trigger on $name field $index"
            lappend triggerList $name
        } else {
            #puts "flux trigger on $trigger"
            lappend triggerList $trigger
            if {[isIonChamber $trigger]} {
                set ionChamberTimeScaled($trigger) \
                [::config getInt $trigger.time_scaled 0]

                if {$ionChamberTimeScaled($trigger)} {
                    global gFluxIonChamberTimeScaled
                    set gFluxIonChamberTimeScaled 1
                    puts "$trigger time_scaled"
                }
            }
        }
    }
    if {[llength $triggerList] > 0} {
        eval set_triggers $triggerList
    }
}

proc flux_move { - } {
    flux_loadLUT

    read_ion_chambers 0.1 i2
    wait_for_devices i2
}


proc flux_set { - } {
    flux_loadLUT

    read_ion_chambers 0.1 i2
    wait_for_devices i2
}


proc flux_update {} {
	return [flux_calculate]
}

proc flux_trigger { triggerDevice } {
    variable ::fluxLUT::last_reading

    if {[isIonChamber $triggerDevice]} {
        variable ::fluxLUT::ionChamberTimeScaled
        variable ::fluxLUT::condition

        if {$ionChamberTimeScaled($triggerDevice)} {
            global gDevice
            set last_reading [expr $gDevice($triggerDevice,cps) * \
            $condition(integration_time)]

            puts "DEBUG flux_trigger on ion chamber $triggerDevice"
            puts "scaled by time to $last_reading"
        } else {
            set last_reading [get_ion_chamber_counts $triggerDevice]
        }
        #puts "DEBUG flux_trigger on ion chamber $triggerDevice with reading=$last_reading"
    } elseif {[isString $triggerDevice]} {
        ### get field index
        variable ::fluxLUT::stringFieldIndex
        variable $triggerDevice

        if {[info exists stringFieldIndex($triggerDevice)]} {
            set index $stringFieldIndex($triggerDevice)
        } else {
            set index 0
        }

        set stringContents [set $triggerDevice]
        set last_reading [lindex $stringContents $index]
        #puts "DEBUG flux_trigger on string $triggerDevice with index=$index reading=$last_reading"
    }

	update_motor_position flux [flux_calculate] 1
}

proc flux_loadLUT { } {
    global DCS_DIR
    variable beamlineID

    variable ::fluxLUT::condition
    variable ::fluxLUT::flux_lut
    ## flag
    variable ::fluxLUT::lut_length

    set fullPath [file join $DCS_DIR dcsconfig tables $beamlineID flux.dat]

    if {![file exists $fullPath]} {
        log_error flux LUT $fullPath not exists
        return
    }
    if {![file readable $fullPath]} {
        log_error flux LUT $fullPath not readable
        return
    }
    if {[catch {open $fullPath r} fh ] } {
        log_error failed to open file $fullPath to read
        return
    }

    array unset condition
    array unset flux_lut
    set lut_length 0

    ##default:
    set condition(integration_time) 1.0

    ##read the whole file
    set lines [list]
    set i 0
    while {[gets $fh buffer] >= 0} {
        set line [string trim $buffer]
        set firstC [string index $line 0]
        if {[llength $line] >= 5 && $firstC != "#"} {
            lappend lines $line
            incr i
            puts "line $i: $line"
        } elseif {[string first flux_aperture $line] == 0} {
            set condition(aperture) [string range $line 14 end]
            puts "aperture: $condition(aperture)"
        } elseif {$firstC == "#"} {
            set spear_index [string first "Spear current: " $line]
            if {$spear_index > 0} {
                set start_index [expr $spear_index + 15]
                set spearLine [string range $line $start_index end]
                set condition(spear_current) [lindex $spearLine 0]
                puts "spear current: $condition(spear_current)"
            }
            set time_index [string first "Integration time: " $line]
            if {$time_index > 0} {
                set start_index [expr $time_index + 18]
                set timeLine [string range $line $start_index end]
                set condition(integration_time) [lindex $timeLine 0]
                puts "integration_time: $condition(integration_time)"
            }
        }
    }
    close $fh
    puts "total lines: $i"

    ## sort the lines
    set lines [lsort -real -index 0 $lines]

    foreach line $lines {
        set e [lindex $line 0]
        set e [expr int($e * 1000.0)]
        lappend flux_lut(energy) $e

        ### we totally separate open and close in case their i2 readings
        ### not sorted

        set numNode [expr int([llength $line] - 1) / 4]
        ### backward compatible
        if {$numNode == 1} {
            lappend flux_lut(closed,$e) [list 0.0 0.0]
            lappend flux_lut(open,$e)   [list 0.0 0.0]
        }

        for {set n 0} {$n < $numNode} {incr n} {
            set idxStart [expr $n * 4 + 1]
            set idxEnd [expr $idxStart + 3]
            foreach {f i c o} [lrange $line $idxStart $idxEnd] break

            lappend flux_lut(closed,$e) [list $c $f]
            lappend flux_lut(open,$e)   [list $o $f]
        }

        incr lut_length

        ### sort the tables
        set flux_lut(closed,$e) [lsort -real -index 0 $flux_lut(closed,$e)]
        set flux_lut(open,$e)   [lsort -real -index 0 $flux_lut(open,$e)]
        puts "flux_lut(closed,$e)=$flux_lut(closed,$e)"
        puts "flux_lut(open,$e)=$flux_lut(open,$e)"
    }
    if {$lut_length <= 1} { 
        log_error LUT needs at least 2 lines
    }

    if {![string is double -strict $condition(integration_time)]} {
        set condition(integration_time) 1.0
        log_error Integration time in flux LUT is not valid, reset to 1.0
        puts "ERROR Integration time in flux LUT is not valid, reset to 1.0"
    }
}
proc flux_calculate { } {
    variable ::fluxLUT::flux_lut
    variable ::fluxLUT::lut_length
    variable ::fluxLUT::last_reading
    global gDevice
    variable energy

    if {$lut_length < 2} {
        ##log_error DEBUG flux LUT not loaded yet.
        #puts "flux LUT not loaded yet"
        return 0
    }

    set i 0
    foreach e $flux_lut(energy) {
        if {$energy <= $e} {
            break
        }
        incr i
    }

    ## trim
    if {$i < 1} {
        set i 1
    }
    if {$i >= $lut_length} {
        set i [expr $lut_length -1]
    }
    #puts "flux_lut(energy)=$flux_lut(energy)"
    #puts "energy=$energy"
    #puts "i for energy: $i"

    set index1 [expr $i - 1]
    set index2 $i

    if {$gDevice(shutter,state) == "open"} {
        set lut_name open
    } else {
        set lut_name closed
    }

    ##2D LUT
    ## LUT on each neighbor energy first
    set e1 [lindex $flux_lut(energy) $index1]
    set e2 [lindex $flux_lut(energy) $index2]

    #puts "e1=$e1 e2=$e2"

    set f1 [flux_linearInterpolate $flux_lut($lut_name,$e1) $last_reading]
    set f2 [flux_linearInterpolate $flux_lut($lut_name,$e2) $last_reading]

    #puts "f1=$f1 f2=$f2"

    if {$e1 == $e2} {
        set result [expr ($f1 + $f2) * 0.5]
    } else {
        set result [expr $f1 + ($f2 - $f1) * ($energy - $e1) / ($e2 - $e1)]
    }

    #### units to e11
    set result [expr $result / 1.0e11]
    set result [format {%.3g} $result]

    #puts "flux result=$result"

    if {$result < 0} {
        #puts "flux < 0: $result"
        set result 0
    }

    return $result
}
proc flux_linearInterpolate { xyList x } {
    #### no negative ion chamber readings
    if {$x <= 0} {
        return 0.0
    }

    set ll [llength $xyList]

    if {$ll < 2} {
        log_error LUT must have at least 2 sets of numbers

        return -code error "LUT_not_enough_data"
    }

    ### look it up
    set i 0
    foreach xy $xyList {
        foreach {xp yp} $xy break

        if {$x < $xp} {
            break
        }
        incr i
    }
    #puts "xyList: $xyList"
    #puts "x=$x raw_i=$i"

    if {$i < 1} {
        set i 1
    }
    if {$i >= $ll} {
        set i [expr $ll - 1]
    }

    set xy1 [lindex $xyList [expr $i - 1]]
    set xy2 [lindex $xyList $i]

    foreach {x1 y1} $xy1 break
    foreach {x2 y2} $xy2 break

    if {$x1 == $x2} {
        set result [expr ($y1 + $y2) / 2.0]
    } else {
        set result [expr $y1 + ($y2 - $y1) * ($x - $x1) /( $x2 - $x1)]
    }

    if {$result < 0} {
        ## flux cannot be < 0
        set result 0
    }

    return $result
}
