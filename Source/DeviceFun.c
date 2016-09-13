#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "at_command.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "ZDApp.h"
#include "zcl.h"
#include "zcl_ss.h"
#include "zcl_general.h"
#include "zha_project.h"
#include "zcl_lighting.h"
#include "zcl_ha.h"
#include "zcl_ms.h"

extern uint8  zha_project_OnOff;

extern uint8  zha_project_Level_to_Level ;

extern uint16 zha_project_Alarm_Status;

extern uint16 zha_project_Alarm_Type;

extern uint16 zha_project_Smoke_Type;

extern uint16 zha_project_Light_Color_Status;

extern int16 zha_project_Temperature_Value;

extern uint16 zha_project_Humidity_Value;

extern uint16 zha_project_Illumiance_Value;

extern uint8 zha_project_Saturation;

extern uint16 zha_project_HUE_Status;

extern uint8 zha_project_WD_Duration;
extern uint8 zha_project_Warning;
extern uint8 zha_project_WD_SQUAWK;

uint8 seqnum=0;


void bitProcess(uint32 source,uint32 *target)
{
   uint8 i;
   for(i=0;i<sizeof(target);i++)
      target[i]=(source>>i) &0x01;

}

/*
离开网络
*/
uint8 _LeaveNet(uint8 *p)
{
    NLME_LeaveReq_t req;
    //#NLME_LeaveReq_t req;
    req.extAddr = NULL;
    req.removeChildren = FALSE;
    req.rejoin         = TRUE;
    req.silent         = FALSE;
    if(NLME_LeaveReq(&req)==ZSuccess)
    {
        return 1;
    }

    return 0;
}
uint8 LeaveNet(RawData Setting)
{
    uint8 buffer[8];
    osal_memset(buffer,0,8);
    if(_LeaveNet(buffer))
    {
        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
        return 1;
    }else
    {
        HalUARTWrite(HAL_UART_PORT_0,"Failed to leave.\r\n",strlen("Failed to leave.\r\n"));
        return 2;
    }

}

/*
Light控制
*/
uint8 Light(RawData Setting)
{
    uint32 sendData[5];
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    sendData[0] = zha_project_OnOff;
    Setting.data[0] =&sendData[0];
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_OnOff = sendData[0];
    if(Setting.RecSign == 1)
    {
        DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
        DstAddr.endPoint = 1;
        DstAddr.addr.shortAddr = 0;
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

        zcl_SendReportCmd( 1, &DstAddr,
                           ZCL_CLUSTER_ID_GEN_ON_OFF,
                           pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );     
    }
    if ( zha_project_OnOff == LIGHT_ON )
    {
        HalLedSet( 0x08, 0x01 );
    }
    else
    {
        HalLedSet( 0x08, 0x00 );
    }  
    return 1;
}

/*
亮度灯控制
*/
uint8 Level(RawData Setting)
{
    uint32 sendData[5];
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    DstAddr.endPoint = 1;
    DstAddr.addr.shortAddr = 0;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    sendData[0] = zha_project_OnOff;
    sendData[1] = zha_project_Level_to_Level/2.56;
    Setting.data[0] =&sendData[0];
    Setting.data[1] =&sendData[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_OnOff = sendData[0];
    zha_project_Level_to_Level = sendData[1]*2.56;
    if(Setting.RecSign == 1)
    {
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

        zcl_SendReportCmd( 1, &DstAddr,
                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_LEVEL_CURRENT_LEVEL;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Level_to_Level);

        zcl_SendReportCmd( 1, &DstAddr,
                   ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
    }
    Setting.RecSign = 0;
    return 1;
}

/*
本地人体热功能控制
*/
uint8 Pir(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;
}

/*
温度控制
*/
uint8 Temp(RawData Setting)
{
    char AT_Format[]="%d.%d";
    char O_Format[]="%d";
    uint8 buffer[16];
    uint8 i,len=0,sign=0;
    int8 data[2],data1[2];
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    DstAddr.endPoint = 1;
    DstAddr.addr.shortAddr = 0;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    osal_memset(buffer,0,16);
    osal_memset(data,0,2);
    osal_memset(data1,0,2);
    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == '.')
            {
                sign = 1;
                len = strlen(&Setting.RecData[i+1]);
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(len>=2)
        {
            strncpy(buffer,Setting.RecData,strlen(Setting.RecData)-len+2);
        }else if(len<2 && sign==1)
        {
            strcpy(buffer,Setting.RecData);
            strcat(buffer,"0");
        }else
        {
            strcpy(buffer,Setting.RecData);
        }
        if(sign == 1)
        {
            sscanf(buffer,AT_Format,data,data1);
        }else if(sign == 2)
        {
            sscanf(buffer,O_Format,data);
        }
        if(data[0]<0)
        {
            zha_project_Temperature_Value = ((data[0]<<8))/2.56 - data1[0];
        }
        else
        {
            zha_project_Temperature_Value = ((data[0]<<8))/2.56 + data1[0];
        }

        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_MS_TEMPERATURE_MEASURED_VALUE;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Temperature_Value);
        zcl_SendReportCmd( 1, &DstAddr,
                       ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
    }        
}

/*
色温控制
*/
uint8 Colortem(RawData Setting)
{
    uint32 sendData[2];
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    DstAddr.endPoint = 1;
    DstAddr.addr.shortAddr = 0;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    sendData[0] = zha_project_OnOff;
    sendData[1] = zha_project_Level_to_Level/2.56;
    Setting.data[0] =&sendData[0];
    Setting.data[1] =&sendData[1];
    Setting.data[2] =(uint32 *)&zha_project_Light_Color_Status;
    Setting.length = 3;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_OnOff = sendData[0];
    zha_project_Level_to_Level = sendData[1]*2.56;
    
    if(Setting.RecSign == 1)
    {
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

        zcl_SendReportCmd( 1, &DstAddr,
                           ZCL_CLUSTER_ID_GEN_ON_OFF,
                           pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
        
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_LEVEL_CURRENT_LEVEL;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Level_to_Level);

        zcl_SendReportCmd( 1, &DstAddr,
                           ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
                           pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );   
        
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Light_Color_Status);

        zcl_SendReportCmd( 1, &DstAddr,
                           ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                           pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
    }
    return 1;
}

/*
湿度传感器
*/
uint8 Humility(RawData Setting)
{
    char AT_Format[]="%d.%d";
    char O_Format[]="%d.%d";
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    DstAddr.endPoint = 1;
    DstAddr.addr.shortAddr = 0;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    uint8 buffer[16];
    uint8 i,len=0,sign=0;
    uint8 data[2],data1[2];
    osal_memset(buffer,0,16);
    osal_memset(data,0,2);
    osal_memset(data1,0,2);
    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == '.')
            {
                sign = 1;
                len = strlen(&Setting.RecData[i+1]);
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(len>=2)
        {
            strncpy(buffer,Setting.RecData,strlen(Setting.RecData)-len+2);
        }else if(len<2 && sign==1)
        {
            strcpy(buffer,Setting.RecData);
            strcat(buffer,"0");
        }else
        {
            strcpy(buffer,Setting.RecData);
        }
        if(sign == 1)
        {
            sscanf(buffer,AT_Format,data,data1);
        }else if(sign == 2)
        {
            sscanf(buffer,O_Format,data);
        }
        zha_project_Humidity_Value = ((data[0]<<8))/2.56 + data1[0];
        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Humidity_Value);  
        zcl_SendReportCmd( 1, &DstAddr,
                       ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );
    }  
}

/*
门磁传感器
*/
uint8 Doorsen(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;  
}  
  
/*
声光报警器
*/
uint8 Slsensor(RawData Setting)
{
    uint32 data[4];
    data[0] = zha_project_Warning | zha_project_WD_SQUAWK;
    data[1] = zha_project_WD_Duration;
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    if(data[0]<=3)
        zha_project_Warning = data[0];
    else if(data[0]>=4 & data[0]<6)
        zha_project_WD_SQUAWK = data[0]-4;
    zha_project_WD_Duration = data[1];  
} 

/*
烟雾报警器
*/
uint8 Smoke(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1; 
}

/*
光照传感器
*/
uint8 Lumin(RawData Setting)
{
    Setting.data[0] =(uint32 *)&zha_project_Illumiance_Value;
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);    
}

/*
水报警器
*/
uint8 Watersen(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;    
}

/*
一氧化碳报警器
*/
uint8 Cosensor(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;     
}

/*
天然气报警器
*/
uint8 Gassensor(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;      
}

/*
玻璃报警器
*/
uint8 Glasssen(RawData Setting)
{
    afAddrType_t dstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=0;
    dstAddr.endPoint=1;          
    uint32 data[4];
    bitProcess(zha_project_Alarm_Status,data);
    Setting.data[0] =&data[0];
    Setting.data[1] =&data[1];
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_Alarm_Status = data[0] + (data[1]<<1);
    if(Setting.RecSign == 1)
    {
        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zha_project_Alarm_Status,0,0x01,0x10,false, 0);
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
        zcl_SendReportCmd( 1, &dstAddr,
                       ZCL_CLUSTER_ID_SS_IAS_ZONE,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
    }
    Setting.RecSign = 0;
    return 1;
}

/*
插座控制
*/
uint8 Outlet(RawData Setting)
{
    uint32 sendData[5];
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    sendData[0] = zha_project_OnOff;
    Setting.data[0] =&sendData[0];
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    zha_project_OnOff = sendData[0];
    if(Setting.RecSign == 1)
    {
        DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
        DstAddr.endPoint = 1;
        DstAddr.addr.shortAddr = 0;
        pReportCmd->numAttr = 1;
        pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
        pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
        pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

        zcl_SendReportCmd( 1, &DstAddr,
                           ZCL_CLUSTER_ID_GEN_ON_OFF,
                           pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );     
    }
    return 1;
}

void sendReport()
{
    uint8 devicetype;
    zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
    afAddrType_t DstAddr;
    zclReportCmd_t *pReportCmd;
    DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    DstAddr.endPoint = 1;
    DstAddr.addr.shortAddr = 0;
    pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    if ( pReportCmd != NULL )
    {
        switch(devicetype)
        {
            case light:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );       
              
                break;
            case level:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
                DelayMS(200);
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_LEVEL_CURRENT_LEVEL;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Level_to_Level);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );                

                break;
     
            case colortem:
              pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
                DelayMS(200);
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_LEVEL_CURRENT_LEVEL;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Level_to_Level);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );   
                DelayMS(200);
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Light_Color_Status);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );   
              break;
            case temp:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_MS_TEMPERATURE_MEASURED_VALUE;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Temperature_Value);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );                  
            break;
            case pir:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
             break; 
            case humility:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Humidity_Value);  
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
              break;
            case doorsen:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
              break;
            case lumin:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_MS_ILLUMINANCE_MEASURED_VALUE;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Illumiance_Value);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
            break;
            case slsensor:
         
              break;
            case smoke:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
              break;
            case watersen:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
              break;  
            case cosensor:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
                break; 
            case gassensor:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
                break;
            case glasssen:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_Alarm_Status);
                zcl_SendReportCmd( 1, &DstAddr,
                               ZCL_CLUSTER_ID_SS_IAS_ZONE,
                               pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ ); 
                break;
            case zonectrl:

                break;
            case lightswitch:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );             
                break;
            case outlet:
                pReportCmd->numAttr = 1;
                pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
                pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
                pReportCmd->attrList[0].attrData = (void *)(&zha_project_OnOff);

                zcl_SendReportCmd( 1, &DstAddr,
                                   ZCL_CLUSTER_ID_GEN_ON_OFF,
                                   pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, seqnum++ );  
                break;
          
                
            default:
             break;   
        
        }


  }

  

  osal_mem_free( pReportCmd );  
}


