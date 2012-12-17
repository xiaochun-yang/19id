#include <float.h>
#include <math.h>

#include "log_quick.h"
#include "DcsMessage.h"
#include "DcsMessageTwoWay.h"
#include "DcsMessageManager.h"
#include "XOSSingleLock.h"

#include "dbDefs.h"

#include "GwAPSMotor.h"

GwBaseMotor::MotorField GwAPSMotor::m_fields[CHID_END] = {
    //name       monitor   type      
    {".STOP",   false,      DBR_ENUM},
    {".DMOV",   true,       DBR_ENUM},
    {".VAL",    false,      DBR_DOUBLE},
    {".RBV",    true,       DBR_DOUBLE},
    {".SPMG",   true,       DBR_ENUM},
    {".HLS",    true,       DBR_ENUM},
    {".LLS",    true,       DBR_ENUM},
    {".HLM",    true,       DBR_DOUBLE},
    {".LLM",    true,       DBR_DOUBLE},
    {".MRES",   true,       DBR_DOUBLE},
    {".VELO",   true,       DBR_DOUBLE},
    {".ACCL",   true,       DBR_DOUBLE},
    {".BDST",   true,       DBR_DOUBLE},
    {".DIR",    true,       DBR_ENUM}
};
GwAPSMotor::GwAPSMotor( const char* name, const char* localName, bool real )
: GwBaseMotor( name, localName, real )
, m_stopToSend(1)
, m_dmovCurrent(1)
, m_spmgCurrent(3)
, m_hlsCurrent(0)
, m_llsCurrent(0)
, m_mresCurrent(0.001)
, m_veloCurrent(1.0)
, m_acclCurrent(5.0)
, m_bdstCurrent(0.0)
, m_dirCurrent(0)
, m_sendConfigPending(false)
{
    m_PVArray = m_pvMap;
    m_numPV = CHID_END;
    initPVMap( );

    if (!real) {
        m_numPV = CHID_END_OF_PSEUDO_MOTOR;
    }

    m_numBasicPV = CHID_END_OF_MIN;
    connectEPICS( );
}
void GwAPSMotor::fillPVMap( ) {
    unsigned long minDelay = m_pDcsConfig->getInt( "epicsgw.Motor.UpdateRate",
    getPollIndex( ) );
    char tagName[DCS_DEVICE_NAME_SIZE+32] = {0};
    strcpy( tagName, "epicsgw." );
    strcat( tagName, m_name );
    strcat( tagName, ".UpdateRate" );

    minDelay = m_pDcsConfig->getInt( tagName, minDelay );
    setPollIndex( minDelay );

    for (int i = 0; i < m_numPV; ++i) {
        strcpy( m_PVArray[i].name, m_localName );
        strcat( m_PVArray[i].name, m_fields[i].name );
        m_PVArray[i].needMonitor = m_fields[i].needMonitor;
        m_PVArray[i].type        = m_fields[i].type;
        m_PVArray[i].count       = 1;
        fillPVValPointers( i );
    }
}
void GwAPSMotor::sendAPSMoveCompleted( ) {
    const char* pStatus;

    if (getConnectState( ) != ALL_CONNECTED) {
        pStatus = "disconnected";
    } else if (m_hlsCurrent) {
        pStatus = DCS_MOTOR_CW_LIMIT;
    } else if (m_llsCurrent) {
        pStatus = DCS_MOTOR_CCW_LIMIT;
    } else {
        pStatus = DCS_MOTOR_NORMAL;
    }
    sendMoveCompleted( pStatus );

    if (m_sendConfigPending) {
        sendAPSConfig( );
    }
}
//fill baseMotor's variable from m_valCurrent
//then call baseMotor sendConfig
void GwAPSMotor::sendAPSConfig( ) {
    m_lockOn = (m_spmgCurrent==3)?0:1;

    m_limitOnLower = m_limitOnUpper =
    ( m_limitLower != 0.0 || m_limitUpper != 0.0);

    if (!m_realMotor) {
        GwBaseMotor::sendConfig( );
        return;
    }

    if (m_mresCurrent == 0.0) {
        char contents[DCS_DEVICE_NAME_SIZE + 1024];
        strcpy( contents, "APSMotor " );
        strcat( contents, m_name );
        strcat( contents, " mres=0, set scaleFactor to 1000" );
        
        DcsMessage* pMsg =
        m_pDcsMsgManager->NewLog( "error", "epicsgw", contents );
        sendDcsMsg( pMsg );

        m_scaleFactor = 1000.0;
        m_reverseOn = 0;
    } else {
        m_scaleFactor = 1.0 / fabs( m_mresCurrent);
        m_reverseOn = (m_mresCurrent < 0.0)?1:0;
        if (m_dirCurrent) {
            m_reverseOn = 1 - m_reverseOn;
        }
    }

    m_speed = (int)(m_veloCurrent * m_scaleFactor + 0.5);
    LOG_FINEST3( "APSMotor %s speed=%d from %lf", m_name, m_speed,
        m_veloCurrent
    );

    if (m_acclCurrent == 0.0) {
        char contents[DCS_DEVICE_NAME_SIZE + 1024];
        strcpy( contents, "APSMotor " );
        strcat( contents, m_name );
        strcat( contents, " accl=0, set acceleration to 5000" );
        
        DcsMessage* pMsg =
        m_pDcsMsgManager->NewLog( "error", "epicsgw", contents );
        sendDcsMsg( pMsg );
        m_acceleration = 5000;
    } else {
        m_acceleration = int(1000.0 * m_acclCurrent + 0.5);
    }

    m_backlashSteps = int(m_bdstCurrent * m_scaleFactor + 0.5);
    GwBaseMotor::sendConfig( );
}
void GwAPSMotor::updateDCSSByState( int triggerIndex ) {
    LOG_FINEST2( "APSMotor %s updateDCSSByState index %d",
        m_name, triggerIndex
    );
    switch (m_dcsStatus) {
    case DCS_DEVICE_WAITING_ACK:
    case DCS_DEVICE_ACTIVE:
    case DCS_DEVICE_ABORTING:
        sendAPSMoveCompleted( );
        break;

    default:
        if (getConnectState( ) == ALL_CONNECTED && allDataReady( ) ) {
            sendAPSConfig( );
        } else {
            sendAPSMoveCompleted( );
        }
    }
    m_dcsStatus = DCS_DEVICE_INACTIVE;
}
void GwAPSMotor::updateDCSSByData( int triggerIndex ) {
    switch (triggerIndex) {
    case CHID_RBV:
        updateDCSSByRBV( );
        break;

    case CHID_DMOV:
        updateDCSSByDMOV( );
        break;

    case CHID_HLS:
    case CHID_LLS:
        updateDCSSByLimits( triggerIndex );
        break;

    default:
        LOG_FINEST3( "updateDCSSByConfig for %s PV[%d]: %s", m_name,
            triggerIndex, m_PVArray[triggerIndex].name
        );
        updateDCSSByConfig( );
    }
}
void GwAPSMotor::updateDCSSByDMOV( ) {
    LOG_FINEST2( "updateDCSSByDMOV for %s DMOV=%hd", m_name, m_dmovCurrent );
    if (m_dmovCurrent) {
        sendAPSMoveCompleted( );
        m_dcsStatus = DCS_DEVICE_INACTIVE;
    } else {
        if (m_dcsStatus != DCS_DEVICE_ACTIVE) {
            sendMoveStarted( );
            m_dcsStatus = DCS_DEVICE_ACTIVE;
        } else {
            //may be RBV arrived first
            LOG_FINEST1( "APS motor %s got DMOV=false while active",
                m_name
            );
        }
    }
}
void GwAPSMotor::updateDCSSByRBV( ) {
    LOG_FINEST2( "updateDCSSByRBV for %s: pos=%lf", m_name, m_positionCurrent );
    LOG_FINEST1( "dcsStatus = %s", getDcsStatusText( ) );
    if (getPollIndex( )) {
        m_needUpdate = true;
        //let poll handle it
        return;
    }

    switch (m_dcsStatus) {
    case DCS_DEVICE_INACTIVE:
        if (getConnectState( ) == ALL_CONNECTED && allDataReady( )) {
            sendAPSConfig( );
        } else {
            sendAPSMoveCompleted( );
        }
        break;

    case DCS_DEVICE_WAITING_ACK:
        //both DMOV and RBV can transfer from WAITING_ACK to ACTIVE
        sendMoveStarted( );
        m_dcsStatus = DCS_DEVICE_ACTIVE;
        break;

    default:
        sendMoveUpdate( DCS_MOTOR_NORMAL );
    }
}
void GwAPSMotor::updateDCSSByLimits( int index ) {
    const char* pStatus = NULL;
    char msg[1024] = {0};

    switch (index) {
    case CHID_HLS:
        if (m_hlsCurrent == 0) {
            return;
        }
        pStatus = DCS_MOTOR_CW_LIMIT;
        strcpy( msg, " hit clockwise hardware limit" ); 
        break;

    case CHID_LLS:
        if (m_llsCurrent == 0) {
            return;
        }
        pStatus = DCS_MOTOR_CCW_LIMIT;
        strcpy( msg, " hit counterclockwise hardware limit" ); 
        break;
    }
    switch (getDcsStatus( )) {
    case DCS_DEVICE_WAITING_ACK:
    case DCS_DEVICE_ACTIVE:
    case DCS_DEVICE_ABORTING:
        sendMoveUpdate( pStatus );
        break;

    case DCS_DEVICE_INACTIVE:
    default:
        {
            char contents[DCS_DEVICE_NAME_SIZE + 1024];
            strcpy( contents, "APSMotor " );
            strcat( contents, m_name );
            strcat( contents, msg );
        
            DcsMessage* pMsg =
            m_pDcsMsgManager->NewLog( "error", "epicsgw", contents );
            sendDcsMsg( pMsg );
        }
    }
}
void GwAPSMotor::updateDCSSByConfig( ) {
    switch (getDcsStatus( )) {
    case DCS_DEVICE_WAITING_ACK:
    case DCS_DEVICE_ACTIVE:
    case DCS_DEVICE_ABORTING:
        m_sendConfigPending = true;
        return;

    case DCS_DEVICE_INACTIVE:
    default:
        ;
    }
    m_sendConfigPending = false;
    if (!allDataReady( )) {
        return;
    }
    sendAPSConfig( );
}
//2 cases:
// 1: FBK call back with minDelay
// 2: in WAITING_ACK state
void GwAPSMotor::updateDCSSByPoll( ) {
    switch (m_dcsStatus) {
    case DCS_DEVICE_WAITING_ACK:
        if (time( NULL ) > m_PVArray[CHID_VAL].tsPut + 2) {
            sendMoveCompleted( "timeout" );
            m_dcsStatus = DCS_DEVICE_INACTIVE;
        }
        break;

    case DCS_DEVICE_ACTIVE:
    case DCS_DEVICE_ABORTING:
        sendMoveUpdate( DCS_MOTOR_NORMAL );
        break;

    default:
        LOG_WARNING2( "APSMotor %s polled when dcsStatus=%s",
            m_name, getDcsStatusText( ) );

        sendAPSMoveCompleted( );
    }
}
//called when all monitoreddata are re-fetched from EPICS
void GwAPSMotor::updateDCSSByRefresh( ) {
    if (m_dmovCurrent) {
        m_dcsStatus = DCS_DEVICE_INACTIVE;
        sendAPSMoveCompleted( );
    } else {
        m_dcsStatus = DCS_DEVICE_ACTIVE;
        sendMoveStarted( );
    }
}
void GwAPSMotor::move( double newPosition ) {
    XOSSingleLock hold_lock( &m_lock );

    if (getConnectState( ) < BASIC_CONNECTED) {
        char contents[DCS_DEVICE_NAME_SIZE + 128];
        strcpy( contents, "APSMotor " );
        strcat( contents, m_name );
        strcat( contents, " disconnected" );
        DcsMessage* pMsg =
        m_pDcsMsgManager->NewLog( "error", "epicsgw", contents );
        sendDcsMsg( pMsg );
        
        sendAPSMoveCompleted( );
        return;
    }

    if (!m_dmovCurrent) {
        char contents[DCS_DEVICE_NAME_SIZE + 128];
        strcpy( contents, "APSMotor " );
        strcat( contents, m_name );
        strcat( contents, " busy dmov not 1" );
        DcsMessage* pMsg =
        m_pDcsMsgManager->NewLog( "error", "epicsgw", contents );
        sendDcsMsg( pMsg );
        
        sendMoveCompleted( "busy" );
        return;
    }

    double stepSize = 0.001;
    if (m_mresCurrent != 0.0) {
        stepSize = fabs( m_mresCurrent );
    }

    if (fabs( newPosition - m_positionCurrent ) < stepSize) {
        sendMoveCompleted( "normal" );
        return;
    }

    m_moveStartedByDCSS = true;

    //need push out to epics
    m_dcsStatus = DCS_DEVICE_WAITING_ACK;
    m_positionToSend = newPosition;
    m_PVArray[CHID_VAL].needPut = true;
    flushEPICS( );
    setupPollForTimeout( );
}
void GwAPSMotor::stop( ) {
    switch (getDcsStatus( )) {
    case DCS_DEVICE_WAITING_ACK:
    case DCS_DEVICE_ACTIVE:
        m_PVArray[CHID_STOP].needPut = true;
        m_dcsStatus = DCS_DEVICE_ABORTING;
        if (!flushEPICSOnePV( CHID_STOP )) {
            LOG_WARNING1( "aborting APSMotor %s failed", m_name );
        } else {
            LOG_FINEST1( "aborting APSMotor %s", m_name );
            ca_flush_io( );
        }
        break;

    case DCS_DEVICE_INACTIVE:
    case DCS_DEVICE_ABORTING:
    default:
        break;
    }
}

//internal, no safety check
void GwAPSMotor::fillPVValPointers( int index ) {
    switch (index) {
    case CHID_STOP:
        m_PVArray[index].pValToPut = &m_stopToSend;
        break;

    case CHID_DMOV:
        m_PVArray[index].pValFromMonitor = &m_dmovCurrent;
        break;

    case CHID_VAL:
        m_PVArray[index].pValToPut = &m_positionToSend;
        break;

    case CHID_RBV:
        m_PVArray[index].pValFromMonitor = &m_positionCurrent;
        break;

    case CHID_SPMG:
        m_PVArray[index].pValFromMonitor = &m_spmgCurrent;
        break;

    case CHID_HLS:
        m_PVArray[index].pValFromMonitor = &m_hlsCurrent;
        break;

    case CHID_LLS:
        m_PVArray[index].pValFromMonitor = &m_llsCurrent;
        break;

    case CHID_HLM:
        m_PVArray[index].pValFromMonitor = &m_limitUpper;
        break;

    case CHID_LLM:
        m_PVArray[index].pValFromMonitor = &m_limitLower;
        break;

    case CHID_MRES:
        m_PVArray[index].pValFromMonitor = &m_mresCurrent;
        break;

    case CHID_VELO:
        m_PVArray[index].pValFromMonitor = &m_veloCurrent;
        break;

    case CHID_ACCL:
        m_PVArray[index].pValFromMonitor = &m_acclCurrent;
        break;

    case CHID_BDST:
        m_PVArray[index].pValFromMonitor = &m_bdstCurrent;
        break;

    case CHID_DIR:
        m_PVArray[index].pValFromMonitor = &m_dirCurrent;
        break;

    default:
        break;
    }
}
