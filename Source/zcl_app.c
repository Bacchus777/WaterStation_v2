
#include "AF.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "OSAL_PwrMgr.h"
#include "ZComDef.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "math.h"

#include "nwk_util.h"
#include "zcl.h"
#include "zcl_app.h"
#include "zcl_diagnostic.h"
#include "zcl_general.h"
#include "zcl_ms.h"
#include "zcl_ss.h"

#include "bdb.h"
#include "bdb_interface.h"

#include "gp_interface.h"

#include "Debug.h"

#include "OnBoard.h"

#include "commissioning.h"
#include "factory_reset.h"
/* HAL */

#include "hal_adc.h"
#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_led.h"

#include "utils.h"
#include "version.h"

#include <stdint.h>
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclApp_TaskID;

// Структура для отправки отчета
//afAddrType_t zclApp_DstAddr;
// Номер сообщения
uint8 SeqNum = 0;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
void user_delay_ms(uint32_t period);
void user_delay_ms(uint32_t period) { MicroWait(period * 1000); }
/*********************************************************************
 * LOCAL VARIABLES
 */

afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};

//uint8 zclApp_ZoneID;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclApp_Report(void);
static void zclApp_ReadSensors(void);
static void zclApp_BasicResetCB(void);
static void zclApp_RestoreAttributesFromNV(void);
static void zclApp_SaveAttributesToNV(void);
static void zclApp_HandleKeys(byte portAndAction, byte keyCode);
static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper);

static ZStatus_t zclApp_EnrollResponseCB(zclZoneEnrollRsp_t *rsp);

// Отправка прпотечки
static void zclApp_SendChangeNotification(uint8 endPoint, uint8 value);
//      
static void zclApp_ApplyPump(uint8 pump, bool value);
static void zclApp_StopPump(uint16 timer);

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclApp_CmdCallbacks = {
    zclApp_BasicResetCB, // Basic Cluster Reset command
    NULL,                // Identify Trigger Effect command
    zclApp_OnOffCB,      // On/Off cluster commands
    NULL,                // On/Off cluster enhanced command Off with Effect
    NULL,                // On/Off cluster enhanced command On with Recall Global Scene
    NULL,                // On/Off cluster enhanced command On with Timed Off
    NULL,                // RSSI Location command
    NULL                 // RSSI Location Response command
};

/*********************************************************************
* ZCL SS Profile Callback table
*/
static zclSS_AppCallbacks_t zclApp_SSCmdCallbacks =
{
  NULL,                                         // Change Notification command
  NULL,                                         // Enroll Request command
  zclApp_EnrollResponseCB,       // Enroll Reponse command
  NULL,                                         // Initiate Normal Operation Mode command
  NULL,                                         // Initiate Test Mode command
  NULL,                                         // Arm command
  NULL,                                         // Bypass command
  NULL,                                         // Emergency command
  NULL,                                         // Fire command
  NULL,                                         // Panic command
  NULL,                                         // Get Zone ID Map command
  NULL,                                         // Get Zone Information Command
  NULL,                                         // Get Panel Status Command
  NULL,                                         // Get Bypassed Zone List Command
  NULL,                                         // Get Zone Status Command
  NULL,                                         // ArmResponse command
  NULL,                                         // Get Zone ID Map Response command     
  NULL,                                         // Get Zone Information Response command     
  NULL,                                         // Zone Status Changed command     
  NULL,                                         // Panel Status Changed command     
  NULL,                                         // Get Panel Status Response command     
  NULL,                                         // Set Bypassed Zone List command     
  NULL,                                         // Bypass Response command     
  NULL,                                         // Get Zone Status Response command     
  NULL,                                         // Start Warning command
  NULL                                          // Squawk command                              
};

void zclApp_Init(byte task_id) {

    zclApp_RestoreAttributesFromNV();

    zclApp_TaskID = task_id;

    bdb_RegisterSimpleDescriptor(&zclApp_FirstPumpEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_FirstPumpEP.EndPoint, &zclApp_CmdCallbacks);
    zcl_registerAttrList(zclApp_FirstPumpEP.EndPoint, zclApp_AttrsFirstPumpEPCount, zclApp_AttrsFirstPumpEP);
    zcl_registerReadWriteCB(zclApp_FirstPumpEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    bdb_RegisterSimpleDescriptor(&zclApp_SecondPumpEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_SecondPumpEP.EndPoint, &zclApp_CmdCallbacks);
    zcl_registerAttrList(zclApp_SecondPumpEP.EndPoint, zclApp_AttrsSecondPumpEPCount, zclApp_AttrsSecondPumpEP);
    zcl_registerReadWriteCB(zclApp_SecondPumpEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    bdb_RegisterSimpleDescriptor(&zclApp_ThirdPumpEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_ThirdPumpEP.EndPoint, &zclApp_CmdCallbacks);
    zcl_registerAttrList(zclApp_ThirdPumpEP.EndPoint, zclApp_AttrsThirdPumpEPCount, zclApp_AttrsThirdPumpEP);
    zcl_registerReadWriteCB(zclApp_ThirdPumpEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    bdb_RegisterSimpleDescriptor(&zclApp_AllPumpsEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_AllPumpsEP.EndPoint, &zclApp_CmdCallbacks);
    zcl_registerAttrList(zclApp_AllPumpsEP.EndPoint, zclApp_AttrsAllPumpsEPCount, zclApp_AttrsAllPumpsEP);
    zcl_registerReadWriteCB(zclApp_AllPumpsEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    bdb_RegisterSimpleDescriptor(&zclApp_BeeperEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_BeeperEP.EndPoint, &zclApp_CmdCallbacks);
    zcl_registerAttrList(zclApp_BeeperEP.EndPoint, zclApp_AttrsBeeperEPCount, zclApp_AttrsBeeperEP);
    zcl_registerReadWriteCB(zclApp_BeeperEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    bdb_RegisterSimpleDescriptor(&zclApp_WaterLeakEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_WaterLeakEP.EndPoint, &zclApp_CmdCallbacks);
    zclSS_RegisterCmdCallbacks( zclApp_WaterLeakEP.EndPoint, &zclApp_SSCmdCallbacks );
    zcl_registerAttrList(zclApp_WaterLeakEP.EndPoint, zclApp_AttrsWaterLeakEPCount, zclApp_AttrsWaterLeakEP);
    zcl_registerReadWriteCB(zclApp_WaterLeakEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    zcl_registerForMsg(zclApp_TaskID);
    RegisterForKeys(zclApp_TaskID);

    LREP("Build %s \r\n", zclApp_DateCodeNT);

    osal_start_reload_timer(zclApp_TaskID, APP_REPORT_EVT, APP_REPORT_DELAY);
    osal_start_reload_timer(zclApp_TaskID, HAL_KEY_EVENT, 100);

}

static void zclApp_HandleKeys(byte portAndAction, byte keyCode) 
{
  uint8 hal_key_sw;
  uint8 pump;

  LREP("keyCode =0x%x \r\n", keyCode);

  if (keyCode & HAL_KEY_SW_1){
    zclFactoryResetter_HandleKeys(portAndAction, keyCode);
    zclCommissioning_HandleKeys(portAndAction, keyCode);
  }
  
  if ((keyCode & HAL_KEY_SW_2) || (keyCode & HAL_KEY_SW_3) || (keyCode & HAL_KEY_SW_4)) {
    if (zclApp_Alarm & SS_IAS_ZONE_STATUS_ALARM1_ALARMED) {
      zclApp_Alarm = 0x0;//^= SS_IAS_ZONE_STATUS_ALARM1_ALARMED;
      zclApp_SendChangeNotification(WATER_LEAK_ENDPOINT, zclApp_Alarm);
      HalLedSet(HAL_LED_5, HAL_LED_MODE_OFF);
      zclGeneral_SendOnOff_CmdOff(zclApp_WaterLeakEP.EndPoint, &inderect_DstAddr, TRUE, bdb_getZCLFrameCounter());
    }
    else {
      for(int i = 1; i <= PUMPS; i++ ){
        hal_key_sw = 1 << i;
        if (keyCode & hal_key_sw){
          pump = i - 1;
          zclApp_Pumps[pump] = !zclApp_Pumps[pump];
          zclApp_ApplyPump(pump, zclApp_Pumps[pump]);
        }
      }
    }
  }

  // Leakage
  if (keyCode & HAL_KEY_SW_5)
    zclApp_Alarm |= SS_IAS_ZONE_STATUS_ALARM1_ALARMED;

  // Empty
  if (keyCode & HAL_KEY_SW_6)
    zclApp_Alarm |= SS_IAS_ZONE_STATUS_ALARM2_ALARMED;
  else
    if (zclApp_Alarm & SS_IAS_ZONE_STATUS_ALARM2_ALARMED)
      zclApp_Alarm ^= SS_IAS_ZONE_STATUS_ALARM2_ALARMED;

  if (zclApp_Alarm & SS_IAS_ZONE_STATUS_ALARM1_ALARMED) {
    for(int i = 1; i <= PUMPS; i++ ){
      hal_key_sw = 1 << i;
      if (keyCode & hal_key_sw){
        pump = i - 1;
        zclApp_Pumps[pump] = FALSE;
        zclApp_ApplyPump(pump, zclApp_Pumps[pump]);
      }
    }

    
    if (zclApp_Config.BeeperOnLeak) {
      zclApp_Beeper = TRUE;
      HalLedSet(HAL_LED_5, HAL_LED_MODE_ON);
    }
    
    zclGeneral_SendOnOff_CmdOn(zclApp_WaterLeakEP.EndPoint, &inderect_DstAddr, FALSE, bdb_getZCLFrameCounter());
  }
    
  zclApp_SendChangeNotification(WATER_LEAK_ENDPOINT, zclApp_Alarm);
  

  zclApp_Report();  
  
}

uint16 zclApp_event_loop(uint8 task_id, uint16 events) {
//    LREP("events 0x%x \r\n", events);
    if (events & SYS_EVENT_MSG) {
        afIncomingMSGPacket_t *MSGpkt;
        while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(zclApp_TaskID))) {
//            LREP("MSGpkt->hdr.event 0x%X clusterId=0x%X\r\n", MSGpkt->hdr.event, MSGpkt->clusterId);
            switch (MSGpkt->hdr.event) {
            case KEY_CHANGE:
//                LREP("KEY_CHANGE\r\n");
                zclApp_HandleKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
                break;

            case ZCL_INCOMING_MSG:
                if (((zclIncomingMsg_t *)MSGpkt)->attrCmd) {
                    osal_mem_free(((zclIncomingMsg_t *)MSGpkt)->attrCmd);
                }
                break;

            default:
                break;
            }

            // Release the memory
            osal_msg_deallocate((uint8 *)MSGpkt);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if (events & APP_REPORT_EVT) {
        LREPMaster("APP_REPORT_EVT\r\n");
        zclApp_Report();
        return (events ^ APP_REPORT_EVT);
    }

    if (events & APP_SAVE_ATTRS_EVT) {
        LREPMaster("APP_SAVE_ATTRS_EVT\r\n");
        zclApp_SaveAttributesToNV();
        return (events ^ APP_SAVE_ATTRS_EVT);
    }
    if (events & APP_READ_SENSORS_EVT) {
        LREPMaster("APP_READ_SENSORS_EVT\r\n");
        zclApp_ReadSensors();
        return (events ^ APP_READ_SENSORS_EVT);
    }
    if (events & APP_STOP_PUMP_1_EVT) {
        LREPMaster("APP_READ_SENSORS_EVT\r\n");
        zclApp_StopPump(APP_STOP_PUMP_1_EVT);
        return (events ^ APP_STOP_PUMP_1_EVT);
    }
    if (events & APP_STOP_PUMP_2_EVT) {
        LREPMaster("APP_READ_SENSORS_EVT\r\n");
        zclApp_StopPump(APP_STOP_PUMP_2_EVT);
        return (events ^ APP_STOP_PUMP_2_EVT);
    }
    if (events & APP_STOP_PUMP_3_EVT) {
        LREPMaster("APP_READ_SENSORS_EVT\r\n");
        zclApp_StopPump(APP_STOP_PUMP_3_EVT);
        return (events ^ APP_STOP_PUMP_3_EVT);
    }

    return 0;
}

static void zclApp_Report(void) {
  osal_start_reload_timer(zclApp_TaskID, APP_READ_SENSORS_EVT, 100); 
}

static void zclApp_BasicResetCB(void) {
    LREPMaster("BasicResetCB\r\n");
    zclApp_ResetAttributesToDefaultValues();
    zclApp_SaveAttributesToNV();
}

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper) {
    LREPMaster("AUTH CB called\r\n");
    osal_start_timerEx(zclApp_TaskID, APP_SAVE_ATTRS_EVT, 2000);
    return ZSuccess;
}

static void zclApp_SaveAttributesToNV(void) {
    uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    LREP("Saving attributes to NV write=%d\r\n", writeStatus);
    LREP("Beeper=%d\r\n", zclApp_Config.BeeperOnLeak);
}

static void zclApp_RestoreAttributesFromNV(void) {
    uint8 status = osal_nv_item_init(NW_APP_CONFIG, sizeof(application_config_t), NULL);
    LREP("Restoring attributes from NV  status=%d \r\n", status);
    if (status == NV_ITEM_UNINIT) {
        uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
        LREP("NV was empty, writing %d\r\n", writeStatus);
    }
    if (status == ZSUCCESS) {
        LREPMaster("Reading from NV\r\n");
        osal_nv_read(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    }
}

// Обработчик команд кластера OnOff
static void zclApp_OnOffCB(uint8 cmd)
{
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();

  uint8 pump = pPtr->endPoint - 1;

  if (pPtr->endPoint < 5)  {
    switch (cmd) {
      case COMMAND_ON: {
        if (pump  < PUMPS)
          zclApp_Pumps[pump] = TRUE;
        else 
          for(int i = 0; i < PUMPS; i++ )
            zclApp_Pumps[i] = TRUE;
        break;
      }
      case COMMAND_OFF: {
        if (pump  < PUMPS)
          zclApp_Pumps[pump] = FALSE;
        else 
          for(int i = 0; i < PUMPS; i++ )
            zclApp_Pumps[i] = FALSE;
        break;
      }
      case COMMAND_TOGGLE: {
        if (pump  < PUMPS)
          zclApp_Pumps[pump] = !zclApp_Pumps[pump];
      }
    }
  }
  
  if (pPtr->endPoint == 5) {
    switch (cmd) {
      case COMMAND_ON: {
        zclApp_Beeper = TRUE;
        break;
      }
      case COMMAND_OFF: {
        if (!(zclApp_Alarm & SS_IAS_ZONE_STATUS_ALARM1_ALARMED)) 
          zclApp_Beeper = FALSE;
        break;
      }
      case COMMAND_TOGGLE: {
        if (!(zclApp_Alarm & SS_IAS_ZONE_STATUS_ALARM1_ALARMED)) 
          zclApp_Beeper = zclApp_Beeper;
        break;
      }
    }
    
    HalLedSet(HAL_LED_5, zclApp_Beeper ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);

  }
  
  zclApp_Pumps[3] = zclApp_Pumps[0] & zclApp_Pumps[1] % zclApp_Pumps[2];
  
    for(int i = 0; i < PUMPS; i++ )
      zclApp_ApplyPump(i, zclApp_Pumps[i]);
 
  zclApp_Report();  
  
}

static void zclApp_ApplyPump(uint8 pump, bool value)
{
  HalLedSet(1 << (pump + 1), value ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
  if (value & (zclApp_Config.PumpDurations[pump] > 0))
    osal_start_timerEx(zclApp_TaskID, 1 << (2 + pump), zclApp_Config.PumpDurations[pump] * 1000);
}

static void zclApp_StopPump(uint16 timer)
{
  uint8 pump = timer >> 3;
  zclApp_Pumps[pump] = FALSE;
  zclApp_Pumps[3] = zclApp_Pumps[0] & zclApp_Pumps[1] % zclApp_Pumps[2];
  zclApp_ApplyPump(pump, zclApp_Pumps[pump]);
  
  osal_stop_timerEx(zclApp_TaskID, timer);
  osal_clear_event(zclApp_TaskID, timer);

  zclApp_Report();  
}


static ZStatus_t zclApp_EnrollResponseCB(zclZoneEnrollRsp_t *rsp)
{
  switch(rsp->responseCode)
  {
  case SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_SUCCESS:
    //Must set the Zone State attribute to Enrolled if succesful
    zclApp_ZoneState = SS_IAS_ZONE_STATE_ENROLLED;
    zclApp_ZoneID = rsp->zoneID;
    break;
  default:
    break;
  }
  
  return (ZStatus_t)(0)  ;
}


static void zclApp_SendChangeNotification(uint8 endPoint, uint8 value)
{
  LREP("LEAK = %d\r\n", value);

  zclApp_ZoneStatus = value;
  
  //generates a Zone Status Change Notification Command
  zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(endPoint, &inderect_DstAddr, zclApp_ZoneStatus, 0, zclApp_ZoneID, 0, 1, 1 );
  
}


static void zclApp_ReadSensors(void) 
{
  static uint8 currentSensorsReadingPhase = 0;

  LREP("currentSensorsReadingPhase %d\r\n", currentSensorsReadingPhase);
    // FYI: split reading sensors into phases, so single call wouldn't block processor
    // for extensive ammount of time
  switch (currentSensorsReadingPhase++) {
  case 0: // 
    HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
    break;
  case 1:
    bdb_RepChangedAttrValue(FIRST_PUMP_ENDPOINT, GEN_ON_OFF, ATTRID_ON_OFF);
    break;
  case 2:
    bdb_RepChangedAttrValue(SECOND_PUMP_ENDPOINT, GEN_ON_OFF, ATTRID_ON_OFF);
    break;
  case 3:
    bdb_RepChangedAttrValue(THIRD_PUMP_ENDPOINT, GEN_ON_OFF, ATTRID_ON_OFF);
    break;
  case 4:
    bdb_RepChangedAttrValue(ALL_PUMPS_ENDPOINT, GEN_ON_OFF, ATTRID_ON_OFF);
    break;
  case 5:
    bdb_RepChangedAttrValue(BEEPER_ENDPOINT, GEN_ON_OFF, ATTRID_ON_OFF);
    break;
  default:
    HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
    osal_stop_timerEx(zclApp_TaskID, APP_READ_SENSORS_EVT);
    osal_clear_event(zclApp_TaskID, APP_READ_SENSORS_EVT);
    currentSensorsReadingPhase = 0;
    break;
  }
}

/****************************************************************************
****************************************************************************/
