#include "AF.h"
#include "OSAL.h"
#include "ZComDef.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ms.h"

#include "zcl_app.h"

#include "version.h"

#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"
#include "stub_aps.h"

/*********************************************************************
 * CONSTANTS
 */

#define APP_DEVICE_VERSION 2
#define APP_FLAGS 0

#define APP_HWVERSION 1
#define APP_ZCLVERSION 1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16 zclApp_clusterRevision_all = 0x0002;

// Basic Cluster
const uint8 zclApp_HWRevision = APP_HWVERSION;
const uint8 zclApp_ZCLVersion = APP_ZCLVERSION;
const uint8 zclApp_ApplicationVersion = 3;
const uint8 zclApp_StackVersion = 4;

const uint8 zclApp_ManufacturerName[] = {7, 'B', 'a', 'c', 'c', 'h', 'u', 's'};
const uint8 zclApp_ModelId[] = {13, 'W', 'a', 't', 'e', 'r', '_', 'S', 't', 'a', 't', 'i', 'o', 'n'};

const uint8 zclApp_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

#define DEFAULT_TimeLow           0
#define DEFAULT_TimeHigh          0
#define DEFAULT_Duration          30
#define DEFAULT_BeeperOnLeak      FALSE

application_config_t zclApp_Config = {
    .TimeLow           = DEFAULT_TimeLow,
    .TimeHigh          = DEFAULT_TimeHigh,
    .BeeperOnLeak      = DEFAULT_BeeperOnLeak,
    .PumpDurations[0]  = DEFAULT_Duration,
    .PumpDurations[1]  = DEFAULT_Duration,
    .PumpDurations[2]  = DEFAULT_Duration
};

uint32  zclApp_GenTime_TimeUTC = 0;

bool zclApp_Pumps[4] = {FALSE, FALSE, FALSE, FALSE};



uint16 zclApp_IdentifyTime = 0;
uint16 zclApp_MaxDuration = 0;//LEAKDETECTOR_ALARM_MAX_DURATION;
bool    zclApp_Beeper = FALSE;
bool    zclApp_Binder = FALSE;
uint8   zclApp_Alarm  = 0x0;

// IAS ZONE Cluster
uint8   zclApp_ZoneState = SS_IAS_ZONE_STATE_NOT_ENROLLED;
uint16  zclApp_ZoneType = SS_IAS_ZONE_TYPE_WATER_SENSOR;
uint16  zclApp_ZoneStatus = 0;
uint8   zclApp_ZoneID = 0;
uint8   zclApp_IAS_CIE_Address[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };



/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// First Pump EP

CONST zclAttrRec_t zclApp_AttrsFirstPumpEP[] = {
    {BASIC, {ATTRID_BASIC_ZCL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ZCLVersion}},
    {BASIC, {ATTRID_BASIC_APPL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ApplicationVersion}},
    {BASIC, {ATTRID_BASIC_STACK_VERSION, ZCL_UINT8, R, (void *)&zclApp_StackVersion}},
    {BASIC, {ATTRID_BASIC_HW_VERSION, ZCL_UINT8, R, (void *)&zclApp_HWRevision}},
    {BASIC, {ATTRID_BASIC_MANUFACTURER_NAME, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ManufacturerName}},
    {BASIC, {ATTRID_BASIC_MODEL_ID, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ModelId}},
    {BASIC, {ATTRID_BASIC_DATE_CODE, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_DateCode}},
    {BASIC, {ATTRID_BASIC_POWER_SOURCE, ZCL_DATATYPE_ENUM8, R, (void *)&zclApp_PowerSource}},
    {BASIC, {ATTRID_BASIC_SW_BUILD_ID, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_DateCode}},
    {BASIC, {ATTRID_CLUSTER_REVISION, ZCL_UINT16, R, (void *)&zclApp_clusterRevision_all}},
    {ZCL_CLUSTER_ID_GEN_IDENTIFY, {ATTRID_IDENTIFY_TIME, ZCL_DATATYPE_UINT16, RW,(void *)&zclApp_IdentifyTime}},
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RWR, (void *)&zclApp_Pumps[0]}},
    {GEN_ON_OFF, {ATTRID_DURATION, ZCL_UINT16, RW, (void *)&zclApp_Config.PumpDurations[0]}},
    {GEN_ON_OFF, {ATTRID_BEEPER_ON_LEAK, ZCL_BOOLEAN, RW, (void *)&zclApp_Config.BeeperOnLeak}},

};

uint8 CONST zclApp_AttrsFirstPumpEPCount = (sizeof(zclApp_AttrsFirstPumpEP) / sizeof(zclApp_AttrsFirstPumpEP[0]));

const cId_t zclApp_InClusterListFirstPumpEP[] = {
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  GEN_ON_OFF
};

const cId_t zclApp_OutClusterListFirstPumpEP[] = {
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_FIRST_PUMP_EP (sizeof(zclApp_OutClusterListFirstPumpEP) / sizeof(zclApp_OutClusterListFirstPumpEP[0]))
#define APP_MAX_INCLUSTERS_FIRST_PUMP_EP (sizeof(zclApp_InClusterListFirstPumpEP) / sizeof(zclApp_InClusterListFirstPumpEP[0]))

SimpleDescriptionFormat_t zclApp_FirstPumpEP = {
    FIRST_PUMP_ENDPOINT,                        //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_ON_OFF_SWITCH,              //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_FIRST_PUMP_EP,           //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListFirstPumpEP,   //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_FIRST_PUMP_EP,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListFirstPumpEP   //  byte *pAppOutClusterList;
};

// Second Pump EP

CONST zclAttrRec_t zclApp_AttrsSecondPumpEP[] = {
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RWR, (void *)&zclApp_Pumps[1]}},
    {GEN_ON_OFF, {ATTRID_DURATION, ZCL_UINT16, RW, (void *)&zclApp_Config.PumpDurations[1]}},
};

uint8 CONST zclApp_AttrsSecondPumpEPCount = (sizeof(zclApp_AttrsSecondPumpEP) / sizeof(zclApp_AttrsSecondPumpEP[0]));

const cId_t zclApp_InClusterListSecondPumpEP[] = {
  GEN_ON_OFF
};

const cId_t zclApp_OutClusterListSecondPumpEP[] = {
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_SECOND_PUMP_EP (sizeof(zclApp_OutClusterListSecondPumpEP) / sizeof(zclApp_OutClusterListSecondPumpEP[0]))
#define APP_MAX_INCLUSTERS_SECOND_PUMP_EP (sizeof(zclApp_InClusterListSecondPumpEP) / sizeof(zclApp_InClusterListSecondPumpEP[0]))


SimpleDescriptionFormat_t zclApp_SecondPumpEP = {
    SECOND_PUMP_ENDPOINT,                       //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_ON_OFF_SWITCH,              //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_SECOND_PUMP_EP,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListSecondPumpEP,  //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_SECOND_PUMP_EP,         //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListSecondPumpEP  //  byte *pAppOutClusterList;

};

// Third Pump EP

CONST zclAttrRec_t zclApp_AttrsThirdPumpEP[] = {
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RWR, (void *)&zclApp_Pumps[2]}},
    {GEN_ON_OFF, {ATTRID_DURATION, ZCL_UINT16, RW, (void *)&zclApp_Config.PumpDurations[2]}},
};

uint8 CONST zclApp_AttrsThirdPumpEPCount = (sizeof(zclApp_AttrsThirdPumpEP) / sizeof(zclApp_AttrsThirdPumpEP[0]));

const cId_t zclApp_InClusterListThirdPumpEP[] = {
  GEN_ON_OFF
};

const cId_t zclApp_OutClusterListThirdPumpEP[] = {
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_THIRD_PUMP_EP (sizeof(zclApp_OutClusterListThirdPumpEP) / sizeof(zclApp_OutClusterListThirdPumpEP[0]))
#define APP_MAX_INCLUSTERS_THIRD_PUMP_EP (sizeof(zclApp_InClusterListThirdPumpEP) / sizeof(zclApp_InClusterListThirdPumpEP[0]))


SimpleDescriptionFormat_t zclApp_ThirdPumpEP = {
    THIRD_PUMP_ENDPOINT,                        //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_ON_OFF_SWITCH,              //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_THIRD_PUMP_EP,           //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListThirdPumpEP,   //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_THIRD_PUMP_EP,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListThirdPumpEP   //  byte *pAppOutClusterList;

};



// All pumps EP

CONST zclAttrRec_t zclApp_AttrsAllPumpsEP[] = {
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RWR, (void *)&zclApp_Pumps[3]}},
};

uint8 CONST zclApp_AttrsAllPumpsEPCount = (sizeof(zclApp_AttrsAllPumpsEP) / sizeof(zclApp_AttrsAllPumpsEP[0]));

const cId_t zclApp_InClusterListAllPumpsEP[] = {
  GEN_ON_OFF
};

const cId_t zclApp_OutClusterListAllPumpsEP[] = {
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_ALL_PUMPS_EP (sizeof(zclApp_OutClusterListAllPumpsEP) / sizeof(zclApp_OutClusterListAllPumpsEP[0]))
#define APP_MAX_INCLUSTERS_ALL_PUMPS_EP (sizeof(zclApp_InClusterListAllPumpsEP) / sizeof(zclApp_InClusterListAllPumpsEP[0]))


SimpleDescriptionFormat_t zclApp_AllPumpsEP = {
    ALL_PUMPS_ENDPOINT,                         //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_ON_OFF_SWITCH,              //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_ALL_PUMPS_EP,            //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListAllPumpsEP,    //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_ALL_PUMPS_EP,           //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListAllPumpsEP    //  byte *pAppOutClusterList;

};


// Beeper EP

CONST zclAttrRec_t zclApp_AttrsBeeperEP[] = {
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RWR, (void *)&zclApp_Beeper}},
};

uint8 CONST zclApp_AttrsBeeperEPCount = (sizeof(zclApp_AttrsBeeperEP) / sizeof(zclApp_AttrsBeeperEP[0]));

const cId_t zclApp_InClusterListBeeperEP[] = {
  GEN_ON_OFF, 
};

const cId_t zclApp_OutClusterListBeeperEP[] = {
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_BEEPER_EP (sizeof(zclApp_OutClusterListBeeperEP) / sizeof(zclApp_OutClusterListBeeperEP[0]))
#define APP_MAX_INCLUSTERS_BEEPER_EP (sizeof(zclApp_InClusterListBeeperEP) / sizeof(zclApp_InClusterListBeeperEP[0]))


SimpleDescriptionFormat_t zclApp_BeeperEP = {
    BEEPER_ENDPOINT,                            //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_ON_OFF_SWITCH,              //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_BEEPER_EP,               //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListBeeperEP,      //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_BEEPER_EP,              //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListBeeperEP      //  byte *pAppOutClusterList;

};

// Water Leak EP

CONST zclAttrRec_t zclApp_AttrsWaterLeakEP[] = {
    {GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_BOOLEAN, RW, (void *)&zclApp_Binder}},
    {ZCL_CLUSTER_ID_SS_IAS_ZONE, {ATTRID_SS_IAS_ZONE_STATE, ZCL_DATATYPE_ENUM8, R, (void *)&zclApp_ZoneState}},
    {ZCL_CLUSTER_ID_SS_IAS_ZONE, {ATTRID_SS_IAS_ZONE_TYPE, ZCL_DATATYPE_ENUM16, R,(void *)&zclApp_ZoneType}},
		{ZCL_CLUSTER_ID_SS_IAS_ZONE, {ATTRID_SS_IAS_ZONE_STATUS, ZCL_DATATYPE_BITMAP16, R,(void *)&zclApp_ZoneStatus}},
		{ZCL_CLUSTER_ID_SS_IAS_ZONE, {ATTRID_SS_IAS_CIE_ADDRESS, ZCL_DATATYPE_IEEE_ADDR, RW, (void *)zclApp_IAS_CIE_Address}},
};

uint8 CONST zclApp_AttrsWaterLeakEPCount = (sizeof(zclApp_AttrsWaterLeakEP) / sizeof(zclApp_AttrsWaterLeakEP[0]));

const cId_t zclApp_InClusterListWaterLeakEP[] = {
  GEN_ON_OFF, 
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
};

const cId_t zclApp_OutClusterListWaterLeakEP[] = {
  GEN_ON_OFF,
};

#define APP_MAX_OUTCLUSTERS_WATER_LEAK_EP (sizeof(zclApp_OutClusterListWaterLeakEP) / sizeof(zclApp_OutClusterListWaterLeakEP[0]))
#define APP_MAX_INCLUSTERS_WATER_LEAK_EP (sizeof(zclApp_InClusterListWaterLeakEP) / sizeof(zclApp_InClusterListWaterLeakEP[0]))


SimpleDescriptionFormat_t zclApp_WaterLeakEP = {
    WATER_LEAK_ENDPOINT,                        //  int Endpoint;
    ZCL_HA_PROFILE_ID,                          //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_IAS_WARNING_DEVICE,         //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,                         //  int   AppDevVer:4;
    APP_FLAGS,                                  //  int   AppFlags:4;
    APP_MAX_INCLUSTERS_WATER_LEAK_EP,           //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterListWaterLeakEP,   //  byte *pAppInClusterList;
    APP_MAX_OUTCLUSTERS_WATER_LEAK_EP,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterListWaterLeakEP   //  byte *pAppOutClusterList;

};


void zclApp_ResetAttributesToDefaultValues(void) {
    zclApp_Config.TimeLow           = DEFAULT_TimeLow;
    zclApp_Config.TimeHigh          = DEFAULT_TimeHigh;
    zclApp_Config.PumpDurations[0]  = DEFAULT_Duration;
    zclApp_Config.PumpDurations[1]  = DEFAULT_Duration;
    zclApp_Config.PumpDurations[2]  = DEFAULT_Duration;
    zclApp_Config.BeeperOnLeak      = DEFAULT_BeeperOnLeak;
}

