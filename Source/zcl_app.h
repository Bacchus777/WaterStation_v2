#ifndef ZCL_APP_H
#define ZCL_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "version.h"
#include "zcl.h"
#include "zcl_ss.h"

/*********************************************************************
 * CONSTANTS
 */
#define PUMPS 3
 
#define APP_REPORT_DELAY ((uint32)300 * (uint32)1000) // 5 minute

// Application Events
#define APP_REPORT_EVT          0x0001
#define APP_SAVE_ATTRS_EVT      0x0002
#define APP_STOP_PUMP_1_EVT     0x0004
#define APP_STOP_PUMP_2_EVT     0x0008
#define APP_STOP_PUMP_3_EVT     0x0010
#define APP_READ_SENSORS_EVT    0x0020
// 
#define FIRST_PUMP_ENDPOINT           1
#define SECOND_PUMP_ENDPOINT          2
#define THIRD_PUMP_ENDPOINT           3
#define ALL_PUMPS_ENDPOINT            4
#define BEEPER_ENDPOINT               5
#define WATER_LEAK_ENDPOINT           6

/*********************************************************************
 * MACROS
 */
#define NW_APP_CONFIG 0x0402

#define R     ACCESS_CONTROL_READ
#define RW    (R | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE)
#define RR    (R | ACCESS_REPORTABLE)
#define RWR   (RW | RR)

#define BASIC           ZCL_CLUSTER_ID_GEN_BASIC
#define GEN_ON_OFF      ZCL_CLUSTER_ID_GEN_ON_OFF
#define POWER_CFG       ZCL_CLUSTER_ID_GEN_ON

#define ZCL_BOOLEAN     ZCL_DATATYPE_BOOLEAN
#define ZCL_UINT8       ZCL_DATATYPE_UINT8
#define ZCL_UINT16      ZCL_DATATYPE_UINT16
#define ZCL_INT16       ZCL_DATATYPE_INT16
#define ZCL_INT8        ZCL_DATATYPE_INT8
#define ZCL_INT32       ZCL_DATATYPE_INT32
#define ZCL_UINT32      ZCL_DATATYPE_UINT32
#define ZCL_SINGLE      ZCL_DATATYPE_SINGLE_PREC
#define ZCL_BITMAP8     ZCL_DATATYPE_BITMAP8
  
  
#define ATTRID_DURATION         0xF003
#define ATTRID_BEEPER_ON_LEAK   0xF005

/*********************************************************************
 * TYPEDEFS
 */

typedef struct {
    uint32  TimeLow;
    uint32  TimeHigh;
    uint16  PumpDurations[3];
    bool    BeeperOnLeak;
} application_config_t;

/*********************************************************************
 * VARIABLES
 */

extern SimpleDescriptionFormat_t zclApp_FirstPumpEP;
extern CONST zclAttrRec_t zclApp_AttrsFirstPumpEP[];
extern CONST uint8 zclApp_AttrsFirstPumpEPCount;

extern SimpleDescriptionFormat_t zclApp_SecondPumpEP;
extern CONST zclAttrRec_t zclApp_AttrsSecondPumpEP[];
extern CONST uint8 zclApp_AttrsSecondPumpEPCount;

extern SimpleDescriptionFormat_t zclApp_ThirdPumpEP;
extern CONST zclAttrRec_t zclApp_AttrsThirdPumpEP[];
extern CONST uint8 zclApp_AttrsThirdPumpEPCount;

extern SimpleDescriptionFormat_t zclApp_AllPumpsEP;
extern CONST zclAttrRec_t zclApp_AttrsAllPumpsEP[];
extern CONST uint8 zclApp_AttrsAllPumpsEPCount;

extern SimpleDescriptionFormat_t zclApp_BeeperEP;
extern CONST zclAttrRec_t zclApp_AttrsBeeperEP[];
extern CONST uint8 zclApp_AttrsBeeperEPCount;

extern SimpleDescriptionFormat_t zclApp_WaterLeakEP;
extern CONST zclAttrRec_t zclApp_AttrsWaterLeakEP[];
extern CONST uint8 zclApp_AttrsWaterLeakEPCount;

extern const uint8 zclApp_ManufacturerName[];
extern const uint8 zclApp_ModelId[];
extern const uint8 zclApp_PowerSource;

extern application_config_t zclApp_Config;

extern bool zclApp_Pumps[4];

extern uint16 zclApp_IdentifyTime;
extern uint16 zclApp_MaxDuration;

// IAS ZONE Cluster
extern uint8 zclApp_ZoneState;
extern uint16 zclApp_ZoneType;
extern uint16 zclApp_ZoneStatus;
extern uint8 zclApp_IAS_CIE_Address[8];
extern uint8 zclApp_ZoneID;
extern bool zclApp_Beeper;
extern bool zclApp_Binder;
extern uint8 zclApp_Alarm;

// APP_TODO: Declare application specific attributes here

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialization for the task
 */
extern void zclApp_Init(byte task_id);

/*
 *  Event Process for the task
 */
extern UINT16 zclApp_event_loop(byte task_id, UINT16 events);

extern void zclApp_ResetAttributesToDefaultValues(void);

// Функции команд управления
static void zclApp_OnOffCB(uint8);


/*********************************************************************
 *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_APP_H */
