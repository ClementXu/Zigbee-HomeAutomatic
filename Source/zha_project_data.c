/**************************************************************************************************
  Filename:       zha_project_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ss.h"
#include "zcl_se.h"
#include "zcl_ms.h"
#include "zcl_lighting.h"
#include "zcl_ezmode.h"
#include "zcl_poll_control.h"
#include "zcl_electrical_measurement.h"
#include "zcl_diagnostic.h"
#include "zcl_meter_identification.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"
#include "ZGlobals.h"
#include "zha_project.h"

/*********************************************************************
 * CONSTANTS
 */

#define zha_project_DEVICE_VERSION     0
#define zha_project_FLAGS              0

#define SAMPLELIGHT_HWVERSION          1
#define SAMPLELIGHT_ZCLVERSION         1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Basic Cluster
const uint8 zha_project_HWRevision = SAMPLELIGHT_HWVERSION;
const uint8 zha_project_ZCLVersion = SAMPLELIGHT_ZCLVERSION;
const uint8 zha_project_ManufacturerName[] = { 7, 'T','e','s','t','0','0','1' };
const uint8 zha_project_ModelId[] = { 16, 'T','I','0','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zha_project_DateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zha_project_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 zha_project_LocationDescription[17] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 zha_project_PhysicalEnvironment = 0;
uint8 zha_project_DeviceEnable = DEVICE_ENABLED;

// Identify Cluster
uint16 zha_project_IdentifyTime = 0;
#ifdef ZCL_EZMODE
uint8  zha_project_IdentifyCommissionState;
#endif
uint8 zha_project_BatteryVoltage=5;
uint8 zha_project_BatteryPercent=30;


// On/Off Cluster
uint8  zha_project_OnOff = LIGHT_OFF;

uint8  zha_project_Level_to_Level = 0;

uint16 zha_project_Alarm_Status=0;

uint16 zha_project_Alarm_Type=0x00;

uint16 zha_project_Smoke_Type=0x00;

uint16 zha_project_Light_Color_Status=0x0000;

int16 zha_project_Temperature_Value=0x0000;

uint16 zha_project_Humidity_Value=0x00;

uint16 zha_project_Illumiance_Value=0x0000;

uint8 zha_project_Saturation=0x00;

uint16 zha_project_HUE_Status;

uint8 zha_project_WD_Duration;
uint8 zha_project_Warning;
uint8 zha_project_WD_SQUAWK;
extern uint8 SerialNumber[];

// Level Control Cluster
#ifdef ZCL_LEVEL_CTRL
uint8  zha_project_LevelCurrentLevel = ATTR_LEVEL_MIN_LEVEL;
uint16 zha_project_LevelRemainingTime;
uint16 zha_project_LevelOnOffTransitionTime = 20;
uint8  zha_project_LevelOnLevel = ATTR_LEVEL_MID_LEVEL;
uint16 zha_project_LevelOnTransitionTime = 20;
uint16 zha_project_LevelOffTransitionTime = 20;
uint8  zha_project_LevelDefaultMoveRate = 0;   // as fast as possible
#endif

#if ZCL_DISCOVER
CONST zclCommandRec_t zha_project_Cmds[] =
{
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    COMMAND_BASIC_RESET_FACT_DEFAULT,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_ON,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_TOGGLE,
    CMD_DIR_SERVER_RECEIVED
  },
#ifdef ZCL_LEVEL_CONTROL
  ,{
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  }
#endif // ZCL_LEVEL_CONTROL
};

CONST uint8 zclCmdsArraySize = ( sizeof(zha_project_Cmds) / sizeof(zha_project_Cmds[0]) );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t zha_project_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zha_project_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zha_project_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zha_project_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zha_project_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zha_project_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zha_project_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zha_project_LocationDescription
    }
  },
    {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_SERIAL_NUMBER,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)&SerialNumber
    }
  },
  
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_DeviceEnable
    }
  },
  // ***Power Configuration Cluster Attributes***
   {
    ZCL_CLUSTER_ID_GEN_POWER_CFG,
    { // Attribute record
      ATTRID_POWER_CFG_BATTERY_VOLTAGE,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zha_project_BatteryPercent
    }
  },
   {
    ZCL_CLUSTER_ID_GEN_POWER_CFG,
    { // Attribute record
      0x0021,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zha_project_BatteryVoltage
    }
  },
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_IdentifyTime
    }
  },
 #ifdef ZCL_EZMODE
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_COMMISSION_STATE,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ),
      (void *)&zha_project_IdentifyCommissionState
    }
  },
 #endif // ZCL_EZMODE
#endif

  
#if ZG_BUILD_ENDDEVICE_TYPE  
  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ,
      (void *)&zha_project_OnOff
    }
  },
    // *** Level Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_CURRENT_LEVEL,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_Level_to_Level
    }
  },
  
      // *** Level Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_SS_IAS_ZONE,
    { // Attribute record
      ATTRID_SS_IAS_ZONE_STATUS,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Alarm_Status
    }
  },
  

  {
    ZCL_CLUSTER_ID_SS_IAS_ZONE,
    { // Attribute record
      ATTRID_SS_IAS_ZONE_TYPE,
      ZCL_DATATYPE_ENUM16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Smoke_Type
    }
  },
    {
    ZCL_CLUSTER_ID_SS_IAS_WD,
    { // Attribute record
      COMMAND_SS_IAS_WD_START_WARNING,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_Warning
    }
  },
    {
    ZCL_CLUSTER_ID_SS_IAS_WD,
    { // Attribute record
      ATTRID_SS_IAS_WD_MAXIMUM_DURATION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_WD_Duration
    }
  },
    {
    ZCL_CLUSTER_ID_SS_IAS_WD,
    { // Attribute record
      COMMAND_SS_IAS_WD_SQUAWK,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_WD_SQUAWK
    }
  },  
    {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    { // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Light_Color_Status
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    { // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_HUE,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_HUE_Status
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    { // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zha_project_Saturation
    }
  },
  {
    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
    { // Attribute record
      ATTRID_MS_TEMPERATURE_MEASURED_VALUE,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Temperature_Value
    }
  },
    {
    ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
    { // Attribute record
      ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Humidity_Value
    }
  },
  {
    ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
    { // Attribute record
      ATTRID_MS_ILLUMINANCE_MEASURED_VALUE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
#endif 
#if 0 
//Outlet Attributes
  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_SE_CURRENT_SUMMATION_DELIVERED,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },

  {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_MASK_SE_METER_STATUS,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
  
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_MASK_SE_FORMATTING,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_MASK_SE_FORMATTING,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_SE_MULTIPLIER,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_SE_DIVISOR,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_SE_SUMMATION_FORMATTING,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },  
   {
    ZCL_CLUSTER_ID_SE_SIMPLE_METERING,
    { 
      ATTRID_SE_METERING_DEVICE_TYPE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },    
  
  
  {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE,
      ZCL_DATATYPE_BITMAP32,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
  {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
  {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
  },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR,
      ZCL_DATATYPE_INT8,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK,
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },
    {
    ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
    { 
      ATTRID_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD,
      ZCL_DATATYPE_INT16,
      ACCESS_CONTROL_READ ,
      (void *)&zha_project_Illumiance_Value
    }
    },

#endif
#ifdef ZCL_DIAGNOSTIC
   {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NUMBER_OF_RESETS,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_RX_BCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_BCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_RX_UCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_RX_BCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_BCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_RX_UCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_SUCCESS,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_RETRY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_TX_UCAST_FAIL,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_ROUTE_DISC_INITIATED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_ADDED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_REMOVED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NEIGHBOR_STALE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_JOIN_INDICATION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_CHILD_MOVED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NWK_FC_FAILURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_FC_FAILURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_UNAUTHORIZED_KEY,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_NWK_DECRYPT_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_APS_DECRYPT_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_RELAYED_UCAST,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
    {  // Attribute record
      ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI,
      ZCL_DATATYPE_INT8,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
#endif // ZCL_DIAGNOSTIC
};

uint8 CONST zha_project_NumAttributes = ( sizeof(zha_project_Attrs) / sizeof(zha_project_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#if ZG_BUILD_COORDINATOR_TYPE 
const cId_t zha_project_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
  ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
  ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
  ZCL_CLUSTER_ID_SS_IAS_WD,
  ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_GROUPS,
#ifdef ZCL_LEVEL_CTRL
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL
#endif
};
// work-around for compiler bug... IAR can't calculate size of array with #if options.
#ifdef ZCL_LEVEL_CTRL
 #define ZCLSAMPLELIGHT_MAX_INCLUSTERS   6
#else
 #define ZCLSAMPLELIGHT_MAX_INCLUSTERS   5
#endif

const cId_t zha_project_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
  ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
  ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
  ZCL_CLUSTER_ID_SS_IAS_WD,
  ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_GROUPS,
};
#define ZCLSAMPLELIGHT_MAX_OUTCLUSTERS  (sizeof(zha_project_OutClusterList) / sizeof(zha_project_OutClusterList[0]))

SimpleDescriptionFormat_t zha_project_SimpleDesc =
{
  SAMPLELIGHT_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId;
#ifdef ZCL_LEVEL_CTRL
  ZCL_HA_DEVICEID_DIMMABLE_LIGHT,        //  uint16 AppDeviceId;
#else
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,          //  uint16 AppDeviceId;
#endif
  0,            //  int   AppDevVer:4;
  0,                     //  int   AppFlags:4;
  ZCLSAMPLELIGHT_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)&zha_project_InClusterList, //  byte *pAppInClusterList;
  ZCLSAMPLELIGHT_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)&zha_project_OutClusterList //  byte *pAppInClusterList;
};
#endif




#if ZG_BUILD_ENDDEVICE_TYPE

const cId_t zclZHAtest_InClusterList1[6] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  //ZCL_CLUSTER_ID_SS_IAS_ZONE,
  //GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList2[7] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,  
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
//  GENERICAPP_CLUSTERID
};
const cId_t zclZHAtest_InClusterList3[8] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
  ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,  
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList4[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  //ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
  //ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList5[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList6[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  //ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList7[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,  
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList8[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  //ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,  
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList10[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,  
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList9[4] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_WD, 
  ZCL_CLUSTER_ID_SS_IAS_ZONE, 
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList11[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
//  GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_InClusterList14[7] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  //ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_SE_METERING,
  ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
 //   GENERICAPP_CLUSTERID
};


const cId_t zclZHAtest_OutClusterList1[4] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  //ZCL_CLUSTER_ID_SS_IAS_ZONE
 //   GENERICAPP_CLUSTERID
};


const cId_t zclZHAtest_OutClusterList2[4] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList3[4] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList4[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,  
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList5[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
 //   GENERICAPP_CLUSTERID
};



const cId_t zclZHAtest_OutClusterList6[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
  //GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList7[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
  //GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList8[5] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList9[4] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
  ZCL_CLUSTER_ID_SS_IAS_WD,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList10[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList11[3] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_SS_IAS_ZONE,
 //   GENERICAPP_CLUSTERID
};

const cId_t zclZHAtest_OutClusterList14[7] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_POWER_CFG,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_SE_METERING,
  ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
 //   GENERICAPP_CLUSTERID
};


SimpleDescriptionFormat_t zclZHAtest_SimpleDesc1 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  6,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList1, //  byte *pAppInClusterList;
  4,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList1 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc2 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_DIMMABLE_LIGHT,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  7,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList2, //  byte *pAppInClusterList;
  4,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList2 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc3 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_COLORED_DIMMABLE_LIGHT,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  8,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList3, //  byte *pAppInClusterList;
  4,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList3 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc4 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_TEMPERATURE_SENSOR,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  5,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList4, //  byte *pAppInClusterList;
  5,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList4 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc5 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_IAS_ZONE,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  3,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList5, //  byte *pAppInClusterList;
  3,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList5 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc6 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  0x0307,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  5,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList6, //  byte *pAppInClusterList;
  5,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList6 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc7 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_IAS_ZONE,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  3,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList7, //  byte *pAppInClusterList;
  3,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList7 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc8 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  0x0308,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  5,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList8, //  byte *pAppInClusterList;
  5,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList8 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc9 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_IAS_ZONE,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  4,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList9, //  byte *pAppInClusterList;
  4,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList9 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc10 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_IAS_ZONE,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  3,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList10, //  byte *pAppInClusterList;
  3,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList10 //  byte *pAppInClusterList;
};


SimpleDescriptionFormat_t zclZHAtest_SimpleDesc11 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_IAS_ZONE,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  3,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList11, //  byte *pAppInClusterList;
  3,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList11 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc12 =
{
  1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_ON_OFF_LIGHT_SWITCH,        //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  4,         //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList1, //  byte *pAppInClusterList;
  6,        //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList1 //  byte *pAppInClusterList;
};

SimpleDescriptionFormat_t zclZHAtest_SimpleDesc14 =
{
  1,                                    //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId[2];
  0x0202,                           //  uint16 AppDeviceId[2];
  zha_project_DEVICE_VERSION,            //  int   AppDevVer:4;
  zha_project_FLAGS,                     //  int   AppFlags:4;
  7,                                //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_InClusterList14, //  byte *pAppInClusterList;
  7,                                //  byte  AppNumInClusters;
  (cId_t *)zclZHAtest_OutClusterList14 //  byte *pAppInClusterList;
};


#endif
/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/****************************************************************************
****************************************************************************/


