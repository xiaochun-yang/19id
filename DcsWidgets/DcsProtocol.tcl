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

package provide DCSProtocol 1.0
package require Itcl

package require DCSMessageRouter

proc DCS::buildMessage { textMessage binaryMessage} {
    set textLength [string length $textMessage]
    set binaryLength [string length $binaryMessage]
    
    set header [format "%12d %12d" $textLength $binaryLength]

    return "${header} ${textMessage}${binaryMessage}"
}


#Base class handling generic socket and DCS protocols.  
class DCS::SocketProtocol {
    inherit DCS::MessageRouter

    #used by quick C code parser
    protected variable m_useQuickParser 0
    protected variable m_quickParser ""
    protected variable m_inProcessMsg 0

    #file name to save raw message
    public variable m_saveRawMsg "" {
        if {$m_quickParser != ""} {
            $m_quickParser save $m_saveRawMsg
        }
    }

    # private data members
    protected variable _state
    protected variable _textMessage ""
    protected variable _binaryMessage ""


    protected variable _accumulatedMessage ""
    protected variable _accumuSize 0
    protected variable _textSize
    protected variable _binarySize
    protected variable _headerLength 26
    protected variable _messageHandlers ""
    protected variable m_logger ""
    
    #public data members
    public variable _socket
    public variable _myaddr  ""
    public variable _myport  ""
    public variable _otheraddr  ""
    public variable _otherport  ""
    public variable _connectionGood    0
    public variable callback ""
    public variable networkErrorCallback ""
    public variable useSSL 0

    public variable _socketGood 0
    
    public method constructor { socket args}
    public method destructor {} {}
    
    #public methods
    public method sendMessage { textMessage {binaryMessage ""} }
    public method handleReadableEvent {}
    public method quickHandleReadableEvent {}
    public method registerMessageHandler {callback}
    public method breakConnection {}
    public method send_to_server { message {messageSize 200} }
    # private member functions
    protected method handleCompleteMessage {}

    protected method handleNetworkError { errorMessage }
    public method dcsConfigure {}

    public method registerLogger { logger } {
        set m_logger $logger
    }

    protected method outputLog { level msg } {
        if {$m_logger != ""} {
            $m_logger $level $msg
        }
    }
}

body DCS::SocketProtocol::constructor { args } {
    set _state "HEADER"

    eval configure $args

    CLibraryAvailable
    if {[llength [info commands NewDcsStringParser]] > 0} {
        set m_useQuickParser 1
        set m_quickParser [NewDcsStringParser]
        #puts "using C code quick parser: $m_quickParser"
    } else {
        puts "Note: Using TCL-only message parser.  "
    }

    array set m_dcsMsgs [list]

}

body DCS::SocketProtocol::dcsConfigure {} {
    fconfigure $_socket -translation binary -encoding binary -blocking 0 -buffering none
    # set up callback for incoming packets
    if {$m_useQuickParser} {
        fileevent $_socket readable "$this quickHandleReadableEvent"
    } else {
        fileevent $_socket readable "$this handleReadableEvent"
    }
}

body DCS::SocketProtocol::registerMessageHandler { callback} {
    append _messageHandlers $callback 
}

body DCS::SocketProtocol::sendMessage { textMessage {binaryMessage ""} } {
    #puts "PROTOCOL $this: out-> $textMessage"
    if { ! $_connectionGood } {
        puts "$this not connected"
        return
    }

    if { [catch {puts -nonewline $_socket [DCS::buildMessage $textMessage $binaryMessage]} badresult]} {
        handleNetworkError "Error writing to $_otheraddr: $badresult"
        return
    } 
    
    if { [ catch { flush $_socket } badresult ] } {
        handleNetworkError "Error writing to $_otheraddr: $badresult"
    }
    #puts "sent"
}

#allows sending dcs protocol 1.0
body DCS::SocketProtocol::send_to_server { message {messageSize 200} }  {
    #puts "[time_stamp] out-> $message"
    set length [string length $message]
    append message [string repeat " " [expr $messageSize - $length] ]
    
    #puts [string length $message]

    if { [catch {puts -nonewline $_socket $message} badresult]} {
        handleNetworkError "Error writing to $_otheraddr: $badresult"
        return
    } 
    
    if { [catch { flush $_socket } badresult ] } {
        handleNetworkError "Error writing to $_otheraddr: $badresult"
    }
}


body DCS::SocketProtocol::handleReadableEvent {} {
    #puts "$this: readable event"
    
    # make sure socket connection is still open
    if { [eof $_socket] } {
        handleNetworkError "Connection closed by $_otheraddr."
        return
    }
    
    # read a message from the server
    if { [catch { read $_socket} message] } {
        handleNetworkError "Error reading from $_otheraddr: $message"
        return
    }

    set lengthReceived [string length $message]

    if {$lengthReceived > 0 && $m_saveRawMsg != ""} {
        if {![catch {open $m_saveRawMsg "a"} rawChid]} {
            fconfigure $rawChid -translation binary -encoding binary
            puts -nonewline $rawChid $message
            close $rawChid
            puts "rawMsg: $lengthReceived"
        }
    }

    #puts "$this: $lengthReceived bytes"
    append _accumulatedMessage $message
    incr _accumuSize $lengthReceived

    #dumpBinary $_accumulatedMessage

    while { $_accumuSize > 0} {
        if { $_state == "HEADER" } {
            if { $_accumuSize >= $_headerLength } {
                #store the complete dcs header without any following payload
                set dcsHeader [string range $_accumulatedMessage 0 [expr $_headerLength - 1]]
                #store any remaining payload.
                set _accumulatedMessage [string range $_accumulatedMessage $_headerLength end]
                incr _accumuSize -$_headerLength
                
                #Parse the header and get the text and binary sizes to follow
                #Remove null terminated string if necessary.
                set header [string trimright $dcsHeader \0]
                #puts $header
                set _textSize [lindex  $header 0]
                set _binarySize [lindex $header 1]
                
                #puts "binarySize: $_binarySize"
                #dumpBinary $_binarySize

                #calculate the next thing to expect over the socket
                if { $_textSize > 0 } {
                    #text message expected next...
                    set _state "TEXT"
                } elseif { $_binarySize > 0} {
                    #header and binary data only...
                    set _state "BINARY"
                } else {
                    #empty header with no data...
                    set _state "HEADER"    
                }
            } else {
                #We didn't get the complete header with that read.
                #Return until we have another read event.
                return
            }
        }
        
        if { $_state == "TEXT" } {
            #puts "Message text: got [string length $_accumulatedMessage] bytes"
            if { $_accumuSize >= $_textSize } {
                #store the complete text message only
                set _textMessage [string range $_accumulatedMessage 0 [expr $_textSize - 1]]
                set _textMessage [string trimright $_textMessage \0]

                #store any remaining payload.
                set _accumulatedMessage [string range $_accumulatedMessage $_textSize end]
                incr _accumuSize -$_textSize
                
                #set binarySize [format "%d" $_binarySize]
                #encoding convertto [encoding system] $_binarySize
                if { $_binarySize > 0 } {
                    #binary data is next...
                    set _state "BINARY"
                    #puts "binary size $_binarySize"
                } else {
                    #message is text only. Wait for next header.
                    set _state "HEADER"    
                }
            } else {
                #puts "incomplete $_state."
                #We didn't get the complete text message with that event.
                #Return until we have complete message
                return
            }
        }

        if { $_state == "BINARY" } {
            if { $_accumuSize >= $_binarySize } {
                #store the complete text message only
                set _binaryMessage [string range $_accumulatedMessage 0 [expr $_binarySize -1]]
                #store any remaining payload.
                set _accumulatedMessage [string range $_accumulatedMessage $_binarySize end]
                incr _accumuSize -$_binarySize

            #    dumpBinary $_binaryMessage

                #next thing should be a header again
                set _state "HEADER"    
            } else {
                #We didn't get the complete text message with that event.
                #Return until we have complete message
                return
            }
        }


        #Complete DCS message now collected
        #puts "$this: in <- text:$_textMessage binary:$_binaryMessage"

        if { [catch {$this handleCompleteMessage} errorResult] } {
            #breakConnection
            global errorInfo
            puts $errorInfo

            return -code error $errorResult
        }

        # execute the message as a command in the messageHandler namespace
        
        
        #free up the potentially large binary message
        set _binaryMessage ""
    }
}

body DCS::SocketProtocol::quickHandleReadableEvent {} {
    incr m_inProcessMsg

    #puts "enter quick: $m_inProcessMsg"
    # make sure socket connection is still open
    if { [eof $_socket] } {
        handleNetworkError "Connection closed by $_otheraddr."
        incr m_inProcessMsg -1
        return
    }
    
    if {[catch {eval $m_quickParser read $_socket} em]} {
        handleNetworkError "reading socket error:$em"
        incr m_inProcessMsg -1
        return
    }

    if {$m_inProcessMsg != 1} {
        puts "re-enter quickHandleReadableEvent: $m_inProcessMsg: $_textMessage"
    }

    if {[catch {
        while {[$m_quickParser get _textMessage _binaryMessage]} {
            #puts "$this: in <- text:$_textMessage binary:$_binaryMessage"

            if { [catch {$this handleCompleteMessage} errorResult] } {
                #breakConnection
                global errorInfo
                puts $errorInfo
            }
        }
    } em] } {
        handleNetworkError "get message error:$em"
        incr m_inProcessMsg -1
        return
    }
    incr m_inProcessMsg -1
}

body DCS::SocketProtocol::handleCompleteMessage { } {
    set _socketGood 1
    if { $callback != "" } {
        $callback $_textMessage $_binaryMessage
    }
}

body DCS::SocketProtocol::breakConnection {} {
    set _connectionGood    0

    set clientState offline
    
    puts "$this closing socket..."
    #fileevent $_socket readable {}
    
    catch { close $_socket }
    #deconstruct this object
    #delete object $this
}

body DCS::SocketProtocol::handleNetworkError { message } {
    #puts "$this : network error: $message"
    $this breakConnection

    if {$m_useQuickParser} {
        $m_quickParser clear
    } else {
        set _accumulatedMessage ""
        set _accumuSize 0
        set _state "HEADER"
        set _textSize 0
        set _binarySize 0
    }

    set _textMessage ""
    set _binaryMessage ""

    outputLog logError "Network error: $message"
    eval $networkErrorCallback

    #destroy $this
}



# The DCS::ClientProtocolSocket allows the basic DCS protocol and
# variable length message reading and recieving,
# but does not provide master/slave handling with DCSS. 
class DCS::ClientProtocol {
    inherit DCS::SocketProtocol
    #configurable data members
    public variable _reconnectTime 5000

    # public member functions
    public method constructor { otherAddress otherPort args }
    public method connect { }
    public method handleNetworkError { errorMessage}
    public method sendMessage {text} { DCS::SocketProtocol::sendMessage $text}
}


body DCS::ClientProtocol::constructor { otherAddress otherPort args } {
    set _otheraddr $otherAddress
    set _otherport $otherPort
    eval configure $args
}




body DCS::ClientProtocol::connect { } {
    # disconnect from server if currently connected
    if { $_connectionGood } {
        breakConnection
    }

    puts "Connecting to server $_otheraddr on port $_otherport..."
    #log_note "Connecting to server $_otheraddr on port $_otherport."

    if {$useSSL} {
        if {[catch {::tls::socket -tls1 1 $_otheraddr $_otherport} result]} {
            outputLog logError "Could not connect to $_otheraddr."
            handleNetworkError "$this could not connect: $result"
            set _socketGood 0
            return
        }
        ::tls::handshake $result
    } else {
        if { [catch { socket $_otheraddr $_otherport } result] } {
            outputLog logError "Could not connect to $_otheraddr."
            handleNetworkError "$this could not connect: $result"
            set _socketGood 0
            return
        }
    }
 
    set _socket $result
    
    $this dcsConfigure
    set _connectionGood 1
}



body DCS::ClientProtocol::handleNetworkError { errorMessage } {
    #log_error "Disconnecting from server. $errorMessage"
    puts "$this : network error: $errorMessage"
    if { $_connectionGood} {
        $this breakConnection
    }
    
    if {$m_useQuickParser} {
        $m_quickParser clear buffer
    } else {
        set _accumulatedMessage ""
        set _accumuSize 0
        set _state "HEADER"
        set _textSize 0
        set _binarySize 0
    }

    set _textMessage ""
    set _binaryMessage ""
    
    eval $networkErrorCallback
    if {$_reconnectTime != 0 } {
        after $_reconnectTime "$this connect"
    }
}


class DCS::serverProtocol {
    inherit DCS::SocketProtocol
    private variable _serverName ""

    protected method handleCompleteMessage {}
    public method handleNetworkError {message}
    public method constructor {serverName sockHandle clientsAddress clientsPort args } {}
}

body DCS::serverProtocol::constructor {serverName sockHandle clientsAddress clientsPort args} {
    set _serverName $serverName
    set _socket $sockHandle
    set _otheraddr $clientsAddress
    set _otherPort $clientsPort
    #set _myaddr $myaddr
    #set _myport $myport
    puts "$_serverName received client connection from $clientsAddress"

    puts "created $this for client handler.  "
    
    fconfigure $_socket -translation binary -encoding binary -blocking 0
    fileevent $_socket readable "$this handleReadableEvent"

    eval configure $args

    set _connectionGood 1
}


body DCS::serverProtocol::handleCompleteMessage { } {
    set _socketGood 1
    if { $callback != "" } {
        eval [concat $callback {$this $_textMessage $_binaryMessage }]
    }
}

body DCS::serverProtocol::handleNetworkError {message } {
    puts "$this network error: $message"
    if { [catch {close $_socket} result] } {
        puts "closed socket with result: $result"
    } else {
        $_serverName removeClient $this
    }
    #deconstruct this object
    delete object $this
}

class DCS::ServerProtocol {
    private variable _socket
    private variable _myaddr
    private variable _myport
    private variable _connectedClients ""


    public method constructor {args}
    public method enable {}
    public method socketAcceptHandler { sock addr port }
    public method broadcastMessage { textMessage {binaryMessage ""}}
    public method removeClient { clientName}
    public method handleClientMessage { clientName textMessage binaryMessage }
    protected method dcsSocketConfigure {}
}

body DCS::ServerProtocol::constructor { listeningPort args } {
    set _myport $listeningPort
    eval configure $args
}

body DCS::ServerProtocol::enable {} {
    set _socket [socket -server "${this} socketAcceptHandler" $_myport]

    puts "enabled server $_socket"
    #set _socket [socket -server $this::socketAcceptHandler]
}

body DCS::ServerProtocol::socketAcceptHandler { sock addr port } {
    puts "$_socket accepted $sock from $addr $port"
    
    DCS::serverProtocol $this$sock $this $sock $addr $port -callback "$this handleClientMessage"
    lappend _connectedClients $this$sock
}

body DCS::ServerProtocol::broadcastMessage { textMessage {binaryMessage ""}} {
    foreach client $_connectedClients {
        $client sendMessage $textMessage $binaryMessage
    }
}

body DCS::ServerProtocol::removeClient {clientName} {
    #remove the client from the list
    set index [lsearch $_connectedClients $clientName]
    if { $index >= 0 } {
        set _connectedClients [lreplace $_connectedClients $index $index]
    }
}

body DCS::ServerProtocol::handleClientMessage {clientName textMessage binaryMessage} {
    $this broadcastMessage $textMessage $binaryMessage
    #dumpBinary $textMessage
    #dumpBinary $binaryMessage    
}


# The DCS::DcssUserProtocol class provides 
# 1) DCS protocol with variable length message 
#    reading and recieving, and
# 2) auth based authentication
# 3) Master/slave handling with DCSS.
class DCS::DcssUserProtocol {
    inherit DCS::ClientProtocol

    public variable authProtocol 2
    public variable clientState  offline     { updateRegisteredComponents clientState }

    public variable clientAuthed 0

    protected variable m_numTimeFailed 0
    protected common MAX_NUMTIME_FAIL  30
    ### after that, quit


    public variable clientID ""
    public method handleNetworkError { message } 

    public method connect
    public method addLoginWidget
    public variable forcedLoginCallback ""
    public variable gotValidSessionIdCallback ""

    public method requestSessionId
    public method setSessionId

    public method getUser
    public method getSessionId
    public method getActiveKey
    public proc getStoredSessionId
    public method getStaff {} {return $_staff}
    public method getRoaming {} {return $_roaming}
    public method getLocation {} {return $_location}

    public method registerForLogMessage { sender callBack }
    public method unregisterForLogMessage { sender callBack }

    private method updateLogListeners { sender }
    public method waitForActiveLock { key }
    public method waitForLoginComplete { }

    private variable m_logListenerListArray

    private variable _sessionId ""
    private variable _user ""
    private variable _preUser ""
    private variable _staff 0
    private variable _roaming 0
    private variable _location REMOTE
    private variable _activeKey new

    #statusMessage is throw-away code until I can get in a better status bar...
    private variable _statusMessage [list "Detector Idle" black]
    public method getStatusMessage {} {return $_statusMessage}

    #in the future, may be changed to really return the motors and clear it
    public method getStoppedMotors { } { return "" }
    public method getConfiguredMotors { } { return "" }

    private method checkPermissionForDeveloperMode { }

    #    public variable deviceNamespace "::device"

    
    private variable m_deviceFactory


    # to avoid race condition between login and auto reconnect
    private variable m_loginTimestamp 0
    private variable m_autoCheckTimestamp 0

    # public member functions
    constructor  {server port args} {

        ::DCS::Component::constructor { 
            clientState {cget -clientState}
            clientID {cget -clientID}
            sessionID {getSessionId}
            user {getUser}
         staff {getStaff}
         roaming {getRoaming}
         location {getLocation}
         statusMessage {getStatusMessage}
         motorStopped {getStoppedMotors}
         motorConfigured {getConfiguredMotors}
        }
        
        DCS::ClientProtocol::constructor $server $port
        
    } {
        global env

        #only load the following if this is a gui client.
#package require DCSLogin
##### DCSLogin need Iwidgets
##### following package is DCSLogin except GUI part
package require DCSAuthClient
package require http

package require DCSClientList
package require DCSDeviceFactory

        set m_deviceFactory [DCS::DeviceFactory::getObject]

        eval configure $args

        set _user $env(USER)

        array set m_logListenerListArray [list]

        announceExist
    }

    destructor {
    }

    protected method handleCompleteMessage
    public method becomeActive {}
    public method becomePassive {}
    public method abort {}
}

body DCS::DcssUserProtocol::connect { } {
    #puts "DCSS connect"

    if { $authProtocol == 2 } {

        if {$_sessionId == "" } {
            #try to get the stored value if possible
            set _sessionId [getStoredSessionId]
            updateRegisteredComponents sessionID
        }

        if {$_sessionId == "" } {
            #go to the login widget
            requestSessionId
            set clientAuthed 0
            return
        }

        # get the authentication server singleton
        set client [AuthClient::getObject]
        puts "client=$client sessionid=$_sessionId user=$_user" 
        if {[catch {$client validateSession $_sessionId $_user } err ] } {
            puts $err
            return
        }

        if {$err != 1} {
            outputLog logError "$_user authentication failed or not authorized to log in any beamline"
            requestSessionId
            set clientAuthed 0
            return
        }
      
        set beamlines [$client getBeamlines]
        set beamlines [split $beamlines \;]

        set userName [$client getUserName]
        set clientAuthed 1
        if {$beamlines == "ALL" } {
            outputLog logNote "$userName authorized to log in to all beam lines."
        } elseif {$beamlines == ""} {
            set clientAuthed 0
            outputLog logError "$userName authentication failed or not authorized to log in any beamline"
        } else {
            set targetBeamline [::config getConfigRootName]
            outputLog logNote "$userName authorized to log in to $beamlines."
            if { [lsearch $beamlines $targetBeamline] == -1 } {
                set clientAuthed 0
                outputLog logError "$userName not authorized to log in to $targetBeamline."
            }
        }
    }

    if {$clientAuthed} {
        set m_numTimeFailed 0
        #call the base class
        #puts "calling base connect"
        DCS::ClientProtocol::connect

        eval $gotValidSessionIdCallback
        if {$_user != $_preUser} {
            set _preUser $_user
            updateRegisteredComponents user
        }
        updateRegisteredComponents sessionID
    } else {
        incr m_numTimeFailed
        if {$m_numTimeFailed > $MAX_NUMTIME_FAIL} {
            puts "========================================================="
            puts "$userName not authorized to log in to $targetBeamline."
            puts "========================================================="
            exit
        }
        after $_reconnectTime "$this connect"
    }
}

body DCS::DcssUserProtocol::getStoredSessionId {} {
    global env

    if { [catch {
        set fileName [file join ~$env(USER) .bluice session]
        set fileName [file native $fileName]
        set fileId [open $fileName r]
        set sessionId [gets $fileId]
        close $fileId
    } err ] } {
        #puts "sessionId wasn't stored"
        return ""
    }
    
    #puts "found stored sessionId: $sessionId"
    return $sessionId
}

body DCS::DcssUserProtocol::requestSessionId {} {
    #puts "LOGIN getsession"

    if { $_sessionId == "" } {
        #puts "LOGIN ACTIVATE"
        eval $forcedLoginCallback
    }
}

body DCS::DcssUserProtocol::setSessionId { user_ id_} {
    #puts "user protocol setSessionId"

    set m_loginTimestamp [clock seconds]

    set _user $user_
    set _sessionId $id_
    
    connect
    #puts "LOGIN $_sessionId"
}

body DCS::DcssUserProtocol::getUser {} {
    return $_user
}

body DCS::DcssUserProtocol::getSessionId {} {
    return $_sessionId
}

body DCS::DcssUserProtocol::getActiveKey {} {
    return $_activeKey
}

body DCS::DcssUserProtocol::addLoginWidget { widget_ } {
    $widget_ configure -validSessionCallback "$this setSessionId"
}

body DCS::DcssUserProtocol::handleNetworkError { errorMessage } {
    set m_autoCheckTimestamp [clock seconds]

    set clientState offline
    updateRegisteredComponents clientState
    DCS::ClientProtocol::handleNetworkError $errorMessage
}


body DCS::DcssUserProtocol::handleCompleteMessage { } {
    global env
    set _socketGood 1
    
   #puts "dcss <- $_textMessage  "
    #handle gui authentication right here.
    if [catch {
    set arg1 ""
    set arg2 ""
    set arg3 ""
    foreach {command arg1 arg2 arg3} $_textMessage {break}
    } errMsg] {
        puts "failed foreach: {$_textMessage}"
        return
    }
    
    switch $command {
        stoc_send_client_type {
            if {$authProtocol == 1} {
                send_to_server "gtos_client_is_gui $_user $env(HOST) $env(DISPLAY)" 200
            } else {
                global gEncryptSID
                if {$gEncryptSID && $arg1 != ""} {
                    set cccc "$arg1:[clock seconds]:$_sessionId"
                    set cypher [DcsSslUtil encryptSID $cccc]
                    set l_c [string length $cypher]
                    puts "cypher length $l_c"
                    send_to_server "gtos_client_is_gui $_user DCS_CYPHER $env(HOST) $env(DISPLAY)" 200
                    sendMessage $cypher
                } else {
                    send_to_server "gtos_client_is_gui $_user $_sessionId $env(HOST) $env(DISPLAY)" 200
                }
            }
            return
        }

        stog_quit {
            puts "==========================================================="
            puts "Forced to quit from server."
            puts "==========================================================="
            exit
        }

        stog_authentication_failed {
         outputLog logError "Failed to authenticate using protocol $authProtocol."
            if {$authProtocol == 1} {
            outputLog logWarning "Switching to authentication protocol 2.0 ."
                puts "-----------------------------------------------------------"
                puts "Recieved an authenticaion protocol 2 message."
                puts "Change the dcss.authProtocol tag in the configuration file."
                puts "-----------------------------------------------------------"
                set authProtocol 2

                #try to get the stored value if possible
                set _sessionId [getStoredSessionId]

                #return and try the cached sessionId once
                if {$_sessionId != "" } return
            }

            #protocol #2 sends back this message
            if {$m_autoCheckTimestamp > $m_loginTimestamp} {
                eval $forcedLoginCallback
            }
        }

        stog_respond_to_challenge {
            if {$authProtocol == 2} {
                set authProtocol 1
                
            outputLog logWarning "Switching to authentication protocol 1.0 ."
                puts "-----------------------------------------------------------"
                puts "Received an authentication protocol 1 message."
                puts "Change the dcss.authProtocol tag in the configuration file."
                puts "Attempting to load the authentication protocol 1 libraries."
                puts "-----------------------------------------------------------"
                
                loadAuthenticationProtocol1Library
            }

            set response [generate_auth_response $env(USER) $_binaryMessage]
            send_to_server $response 200
            return
        }

        stog_become_master {
            global gWaitForActiveLock
            set clientState active
            if { [ info exist gWaitForActiveLock ] && $gWaitForActiveLock != "not_waiting" } {
                updateRegisteredComponentsNow clientState
                set gWaitForActiveLock success
            } else {
                updateRegisteredComponents clientState
            }
            outputLog logNote "This client is now in control of the beam line."

            if {$arg1 != ""} {
                set _activeKey [string trimleft $arg1 "master_lock_key="]
                outputLog logNote $arg1
            }

            return
        }
        
        stog_become_slave {
            if {$arg1 != ""} {
                log_error $arg1 $arg2 $arg3
            }
            global gWaitForActiveLock
            set clientState passive
            updateRegisteredComponents clientState
            outputLog logWarning "This client is no longer in control of the beam line."
            if { [ info exist gWaitForActiveLock ] && $gWaitForActiveLock != "not_waiting" } {
                set gWaitForActiveLock failed
            }
            return
        }

        stog_no_master {
            #set clientState passive
            #updateRegisteredComponents clientState
        }

        stog_login_complete {

            set clientState passive
            updateRegisteredComponents clientState
            
            set clientID $arg1
            updateRegisteredComponents clientID
            outputLog logNote "Successfully connected to the control system."


            return
        }

        stog_configure_pseudo_motor {
            set motor [$m_deviceFactory createPseudoMotor $arg1]
            
            #the configuration message is too big to parse here again.
            $motor configureDevice $_textMessage

            updateRegisteredComponents motorStopped
            updateRegisteredComponents motorConfigured
            return
        }

        stog_configure_real_motor {
            
            set motor [$m_deviceFactory createRealMotor $arg1]

            #send the complete configuration message
            $motor configureDevice $_textMessage

            updateRegisteredComponents motorStopped
            updateRegisteredComponents motorConfigured
            return
        }

        stog_set_motor_children {
            set childrenDevices ""
            foreach child [lrange $_textMessage 2 end] {
                lappend childrenDevices [$m_deviceFactory getObjectName $child]
            }
            set motor [$m_deviceFactory createPseudoMotor $arg1]
            $motor configure -childrenDevices $childrenDevices
            return
        }
        
        stog_set_motor_dependency {
            #puts $_textMessage 
        }

        stog_set_motor_base_units {
            set motor [$m_deviceFactory getObjectName $arg1]
            set bu [$motor cget -baseUnits]
            if {$arg2 != $bu} {
                log_error motor $motor wrong units $bu should be $arg2
                $motor configure -baseUnits $arg2
            }
        }
        
        stog_configure_ion_chamber {
            set obj [$m_deviceFactory createSignal $arg1]
            $obj configure \
            -controller $arg2 \
            -status inactive
        }
        
        stog_update_motor_position {
            #stog_update_motor_position motor position status
            set motor [$m_deviceFactory getObjectName $arg1]

            $motor positionUpdate $arg2 $arg3
            return
        }
        
        stog_motor_move_completed {
            set motor [$m_deviceFactory getObjectName $arg1]

            #stog_motor_move_complete motor position status
            $motor moveCompleted $arg2 $arg3

            updateRegisteredComponents motorStopped
            return
        }

        stog_motor_move_started {
            #stog_motor_move_start motor destination status
            set motor [$m_deviceFactory getObjectName $arg1]

            $motor moveStarted $arg2 $arg3
            return
        }

        stog_configure_operation {
            set operation [$m_deviceFactory createOperation $arg1]
            $operation configureDevice $arg2
            return
        }

        stog_start_operation {
            set operation [$m_deviceFactory getObjectName $arg1]
            $operation handleOperationStart $_textMessage
            return
        }

        stog_operation_completed {
            set operation [$m_deviceFactory getObjectName $arg1]
            $operation handleOperationComplete $_textMessage
            return
        }

        stog_operation_update {
            set operation [$m_deviceFactory getObjectName $arg1]
            $operation handleOperationUpdate $_textMessage
            return
        }

        stog_configure_string {
            set stringObject [$m_deviceFactory createString $arg1]
            $stringObject configureString $_textMessage
            return
        }
        
        stog_set_string_completed {
            set stringObject [$m_deviceFactory getObjectName $arg1]
            $stringObject setContents $arg2 [lrange $_textMessage 3 end]
            return
        }

        stog_configure_shutter {
            set shutterObject [$m_deviceFactory createShutter $arg1]
            $shutterObject configure -state $arg3 -controller $arg2
            return
        }

        stog_report_shutter_state {
            set shutterObject [$m_deviceFactory getObjectName $arg1]
            $shutterObject reported $arg2
            if {$arg3 != "" && $arg3 != "normal"} {
                log_error set shutter $arg1 failed: $arg3
            }
            return
        }

      stog_update_client_list {
          set clientList [DCS::ClientList::getObject] 
         $clientList handleNewClientList $_textMessage
      }
    
      stog_update_client {
          set clientList [DCS::ClientList::getObject] 
         $clientList handleNewClient $_textMessage
      }

      stog_configure_hardware_host {
          $m_deviceFactory createHardwareController $arg1 -hostname $arg2 -status $arg3
         return
       }

      stog_no_hardware_host {
         
      }

      stog_collect_stopped {
         set _statusMessage [list "Detector Idle" black]
         updateRegisteredComponents statusMessage
      }

      stog_note {
         switch $arg1 {
          image_ready {outputLog logNote "Loading $arg2..."}
         encoder_offline {outputLog logError "Mono encoder is offline!"}
         mono_corrected {outputLog logWarning "Mono_theta corrected"}
         changing_detector_mode {
            outputLog logNote "Changing detector mode..."
            set _statusMessage [list "Changing detector mode..." black]
            updateRegisteredComponents statusMessage
         }
         movedExistingFile {outputLog  logWarning "$arg2 moved to $arg3"}
         detector_z {outputLog logWarning "detector_z [lrange $_textMessage 2 end]"}
         failedToBackupExistingFile {outputLog logError "$arg2 already exists and could not be backed up."}
         ion_chamber {outputLog logNote ion_chamber $arg3 $arg4}
         Warning {outputLog logWarning [lrange $_textMessage 2 end]}
         Error {outputLog logError [lrange $_textMessage 2 end]}
         no_beam {
            outputLog logWarning "No Beam"
            set _statusMessage [list "No Beam" red]
            updateRegisteredComponents statusMessage
         }
         default {outputLog logWarning [lrange $_textMessage 2 end]}
         }
         
      }

      stog_log {
            if {[string range $arg1 0 4] == "user_"} {
                ::DCS::UserLogView::handleNewEntry $_textMessage
            } elseif {[string range $arg1 0 4] == "chat_"} {
                ::DCS::UserChatView::handleNewEntry $_textMessage
            } else {
                switch $arg1 {
                    note {outputLog logNote "$arg2 reports: [lrange $_textMessage 3 end]"}
                    warning {outputLog logWarning "$arg2 reports: [lrange $_textMessage 3 end]"}
                    severe -
                    error {outputLog logError "$arg2 reports: [lrange $_textMessage 3 end]"}
                }
            }
            updateLogListeners $arg2
      }
      
      stog_set_permission_level {
         set _staff $arg1
         set _roaming $arg2
         set _location $arg3
         checkPermissionForDeveloperMode
         updateRegisteredComponents staff 
         updateRegisteredComponents roaming
         updateRegisteredComponents location
         return
      }

      stog_report_ion_chambers {

         set time $arg1

         if {$time <= 0} {
              #each item following the time is an ion chamber name
              set icList [lrange $_textMessage 2 end-1]
              set result [lindex $_textMessage end]
              outputLog logError \
              "Ion chamber reading $icList failed: $result"
            
            foreach ionChamber $icList {
                set icObject [$m_deviceFactory getObjectName $ionChamber]
                $icObject reported $time $result
            }

              return
         }

         # get number of arguments
         set argc [llength $_textMessage]
    
          # initialize argument index to the first ion chamber
          set index 2
    
          while { $index < $argc } {

              set ion_chamber [lindex $_textMessage $index]
              incr index
              set counts [lindex $_textMessage $index]
              incr index
       
              #yangx outputLog logNote "Ion chamber $ion_chamber read $counts counts in $time seconds."
                set icObject [$m_deviceFactory getObjectName $ion_chamber]
                $icObject reported $time $counts
         }

      }
        stog_get_encoder_completed -
        stog_set_encoder_completed {
            set obj [$m_deviceFactory getObjectName $arg1]
            $obj completed $arg2 $arg3
        }
        stog_device_permission_bit {
            set obj [$m_deviceFactory getObjectName $arg1]
            $obj configure \
            -staffPermissions $arg2 \
            -userPermissions $arg3
        }
        stog_configure_encoder {
            set obj [$m_deviceFactory getObjectName $arg1]
            $obj configure \
            -controller $arg2 \
            -status inactive \
            -position $arg3
        }
        stog_dcss_end_update_all_device {
            global gNickName
            global gWaitForLoginComplete

            if {[info exists gNickName] && $gNickName != ""} {
                sendMessage "gtos_log location $_user $gNickName"
            }

            if {[info exist gWaitForLoginComplete]} {
                set gWaitForLoginComplete not_waiting
            }
        }
        stog_admin { }
        default {
           puts "Unhandled message: $_textMessage"
        }
    }
    
    #sendMessageToListeners $_textMessage
}

body DCS::DcssUserProtocol::becomeActive {} {
    sendMessage "gtos_become_master force"
}

body DCS::DcssUserProtocol::becomePassive {} {
    sendMessage "gtos_become_slave force"
}

#connect to DCSS and use vwait until the configuration has been read from dcss.
body DCS::DcssUserProtocol::waitForLoginComplete {  } {

    global gWaitForLoginComplete

    if {![info exist gWaitForLoginComplete]} {
        set gWaitForLoginComplete not_waiting
    }
    if {$gWaitForLoginComplete != "not_waiting"} {
        return -code error "only one wait for dcss login is suppported"
    }
    set gWaitForLoginComplete waiting

    set id [after 5000 { set gWaitForLoginComplete time_out }]
    connect
    addToWaitingList gWaitForLoginComplete
    vwait gWaitForLoginComplete
    removeFromWaitingList gWaitForLoginComplete
    set result $gWaitForLoginComplete
    set gWaitForLoginComplete not_waiting
    if {$result == "time_out"} {
        return -code error "time out waiting for dcss login"
    } else {
        catch {after cancel $id}
    }
    if {$result == "failed" } {
        return -code error "wait for login failed"
    } 

}

body DCS::DcssUserProtocol::waitForActiveLock { key } {
    global gWaitForActiveLock

    if {![info exist gWaitForActiveLock]} {
        set gWaitForActiveLock not_waiting
    }

    if {$gWaitForActiveLock != "not_waiting"} {
        return -code error "only one wait for active lock is suppported"
    }
    set gWaitForActiveLock waiting

    set id [after 5000 { set gWaitForActiveLock time_out }]

    if { $key == "new" } {
        sendMessage "gtos_become_master request_lock"
    } else {
        sendMessage "gtos_become_master master_lock_key=$key"
    }

    addToWaitingList gWaitForActiveLock
    vwait gWaitForActiveLock
    removeFromWaitingList gWaitForActiveLock
    set result $gWaitForActiveLock
    set gWaitForActiveLock not_waiting
    #puts "WAITFORACTIVELOCK RESULT $result"
    if {$result == "time_out"} {
        return -code error "time out waiting for active lock from dcss"
    } else {
        catch {after cancel $id}
    }
    if {$result == "aborting"} {
        return -code error "wait for active lock aborted"
    }
    if {$result == "failed" } {
        return -code error "wait for active lock rejected"
    } 
}

body DCS::DcssUserProtocol::abort {} {

   outputLog logNote "Sending 'abort'."
    sendMessage "gtos_abort_all soft"

    ###add abort for blu-ice itself
    abortAllWaiting
}

body DCS::DcssUserProtocol::registerForLogMessage { sender callBack } {
    lappend m_logListenerListArray($sender) $callBack
}
body DCS::DcssUserProtocol::unregisterForLogMessage { sender callBack } {
    set index [lsearch $m_logListenerListArray($sender) $callBack]
    if {$index != -1} {
        set m_logListenerListArray($sender) [lreplace $m_logListenerListArray($sender) $index $index]
    }
}
body DCS::DcssUserProtocol::updateLogListeners { sender } {
    if {[info exists m_logListenerListArray($sender)]} {
        foreach listener $m_logListenerListArray($sender) {
            lappend listener $_textMessage
            eval $listener
        }
    }
}
body DCS::DcssUserProtocol::checkPermissionForDeveloperMode { } {
    global BLC_DIR
    global gBluIceStyle
    if {$gBluIceStyle != "developer"} {
        return
    }

    ## staff OK
    if {$_staff} {
        return
    }
    ### special user OK
    set fn [file join $BLC_DIR .su]
    set fn [file native $fn]

    set allowed 0
    if {[file readable $fn]} {
        if {[catch {
            set h [open $fn]
            set nl [read -nonewline $h]
            close $h
            set nl [split $nl \n]
            foreach name $nl {
                if {$name == $_user} {
                    set allowed 1
                }
            }
        } errMsg] == 1} {
        }
    }
    if {$allowed} {
        puts "You are allowed to run developer mode temperarily"
        return
    }

    puts ""
    puts ""
    puts "================================="
    puts "Not allowed to run developer mode"
    puts "================================="
    puts ""
    puts ""

    exit
}
#
class DCS::DhsProtocol {
    inherit DCS::ClientProtocol

    public variable hardwareName ""
    # public member functions
    constructor  {server port args} {DCS::ClientProtocol::constructor $server $port} {
        eval configure $args
    }
    
    public method dcsConfigure {}
    public method handleFirstReadableEvent {}
    public method sendMessage {text} { DCS::ClientProtocol::sendMessage $text}
}


body DCS::DhsProtocol::dcsConfigure {} {
    #program the reading to be blocking until the connection is established later.
    fconfigure $_socket -translation binary -encoding binary -blocking 1 -buffering none
    # set up callback for incoming packets
    fileevent $_socket readable "$this handleFirstReadableEvent"
}

body DCS::DhsProtocol::handleFirstReadableEvent {} {
    # make sure socket connection is still open
    if { [eof $_socket] } {
        handle_network_error "Connection closed to server."
        return
    }

    # read a message from the server
    if { [catch {set message [read $_socket 200]}] } {
        handle_network_error "Error reading from server."
        return
    }

   set lengthReceived [string length $message]
    
    #reprogram the event handler to be non-blocking
    fconfigure $_socket -translation binary -encoding binary -blocking 0 -buffering none
    fileevent $_socket readable "$this handleReadableEvent"
    
    send_to_server "htos_client_type_is_hardware $hardwareName dcsProtocol_2.0" 200
}

