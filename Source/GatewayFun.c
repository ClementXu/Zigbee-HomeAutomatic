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
#include "zcl_general.h"
#include "zcl_lighting.h"
#include "zha_project.h"
#include "zcl_ha.h"
#include "zcl_ss.h"
#include "zcl_ms.h"
#include "zcl_ezmode.h"
#include "zcl_diagnostic.h"
#include "ZGlobals.h"

extern uint8 netState;

uint8 setFlag = 0;

NODE_INFO_t AssoList[5];

extern uint8 TempDevice[190];
DEVICE_STATUS_t DeviceStatus[5];
extern uint8  zha_project_OnOff;
extern uint8  zha_project_Level_to_Level;
extern uint16 zha_project_Alarm_Status;
extern uint16 zha_project_Alarm_Type;
extern uint16 zha_project_Humidity_Value;
extern uint16 zha_project_Light_Color_Status;
extern int16 zha_project_Temperature_Value;
extern uint8 zha_project_WD_Duration;
extern uint8 zha_project_Warning;
extern uint8 zha_project_WD_SQUAWK;
extern uint16 zha_project_HUE_Status;
extern uint16 zha_project_Illumiance_Value;

void DelayMS(uint16 msec)
{ 
    uint16 i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<536*2; j++);
}


void UpdateDeviceStatus1(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[0]=data[0];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

void UpdateDeviceStatus2(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[1]=data[1];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

void UpdateDeviceStatus3(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[2]=data[2];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

/*
保存MAC地址和短地址
*/
void SetTempDeviceSA(uint16 data,uint8 *mac)
{   
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {   //审查身份
        if(p->device[i].uiNwk_Addr==data)
            return;
    }
    
    for(i=0;i<=5;i++)
    {   
        if(p->device[i].uiNwk_Addr==0)
        {
            p->device[i].uiNwk_Addr = data;
            osal_memcpy(p->device[i].aucMAC,mac,8);
            
            return;
        }
    }    
}

/*
保存版本号
*/
void SetTempDeviceHW(uint16 shortAddr,uint8 version)
{   
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].version = version;
            return;
        }
    }  
}

/*
保存Endpoint
*/
void SetTempDeviceEP(uint16 shortAddr,uint8 *buffer )
{
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].ep,0,5);
            osal_memcpy(p->device[i].ep,buffer,5);
            //p->device[i].ep = ep;
            return;
        }
    }  
}

/*
保存厂商名
*/
void SetTempDeviceManuName(uint16 shortAddr,uint8 *buffer)
{
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].factoryName,0,17);
            osal_memcpy(p->device[i].factoryName,buffer,16);
            return;
        }
    }  
}

/*
保存电池电量
*/
void SetTempDeviceBAT(uint16 shortAddr,uint8 battery)
{  
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].batteryValue = battery;
            return;
        }
    }  
}

/*
保存设备类型
*/
void SetTempDeviceType(uint16 shortAddr,uint16 change)
{   
    uint8 i;
    uint16 supportOD=0;
    uint8 deviceType[16];
    osal_memset(deviceType,0,sizeof(deviceType));
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    switch(change)
    {
        case 0x000d:
            osal_memcpy(deviceType,"pir",sizeof("pir"));
            break;
        case 0x0015:
            osal_memcpy(deviceType,"doorsen",sizeof("doorsen")); 
            break;
        case 0x0028:
            osal_memcpy(deviceType,"smoke",sizeof("smoke")); 
            break;                                          
        case 0x002a:
            osal_memcpy(deviceType,"watersen",sizeof("watersen")); 
            break;   
        case 0x0100: 
            osal_memcpy(deviceType,"light",sizeof("light")); 
            break;
        case 0x0115:
            osal_memcpy(deviceType,"zonectrl",sizeof("zonectrl"));
            break;   
        case 0x0202:
            osal_memcpy(deviceType,"outlet",sizeof("outlet"));
            break;
        case 0x0226:
            osal_memcpy(deviceType,"glasssen",sizeof("glasssen"));
            break;   
        case 0x0227:
            osal_memcpy(deviceType,"cosensor",sizeof("cosensor"));
            break;   
        case 0x0225:
            osal_memcpy(deviceType,"slsensor",sizeof("slsensor"));
            break;             
        case 0x0302:
            osal_memcpy(deviceType,"temp",sizeof("temp"));
            break;
        case 0x0101:
            osal_memcpy(deviceType,"level",sizeof("level"));
            break;                            
        case 0x0102:
            osal_memcpy(deviceType,"colortem",sizeof("colortem"));
            break;                            
        case 0x0307:
            osal_memcpy(deviceType,"humility",sizeof("humility"));
            break;   
        case 0x0308:
            osal_memcpy(deviceType,"lumin",sizeof("lumin"));
            break;  
        case 0x0403:
            osal_memcpy(deviceType,"slsensor",sizeof("slsensor"));
            break;   
        default:

            return;
    }
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].deviceType,0,16);
            osal_memcpy(p->device[i].deviceType,deviceType,strlen(deviceType));
            return;
        }
    }
}


/*
建立网络
*/
uint8 FormNet(RawData Setting)
{
    uint8 logicalType;
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        if(netState ==0 )
        {
            ZDOInitDevice(0);
            ZDO_StartDevice(ZG_DEVICETYPE_COORDINATOR,MODE_HARD,15, 15);
            Setting.length = 0;
            AnaDataProcess(&Setting);
        }
        else if(netState == 1)
        {
            HalUARTWrite(HAL_UART_PORT_0,"Network already established.\r\n",strlen("Network already established.\r\n"));
        }
    }

}



/*
离开网络
*/
uint8 _LeaveNet(uint8 *p)
{
    uint8 logicalType;
    NLME_LeaveReq_t req;
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        //#NLME_LeaveReq_t req;
        req.extAddr = p;
        req.removeChildren = FALSE;
        req.rejoin         = TRUE;
        req.silent         = FALSE;
        if(NLME_LeaveReq(&req)==ZSuccess)
        {
            return 1;
        }
    }
    return 0;
}
uint8 LeaveNet(RawData Setting)
{
    uint8 buffer[8];
    osal_memset(buffer,0,8);
    if(netState ==0)
    {
        HalUARTWrite(HAL_UART_PORT_0,"Please establish network first.\r\n",strlen("Please establish network first.\r\n"));
        return 0;
    }
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
允许设备入网
*/
uint8 PermitJoin(RawData Setting)
{
    uint8 jointime;
    uint8 i,logicalType;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    Setting.data[0] =(uint32 *)&jointime;
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType); 
    if(logicalType==0)
    {       
        for(i=0;i<=5;i++)
        {
            if(p->device[i].uiNwk_Addr!=0)
                AssocRemove(p->device[i].aucMAC);
        }
        CleanTempDevice();
        //DelayMS(100);
        NLME_PermitJoiningRequest(jointime);
    }  
}

/*
移除设备
*/
void _RemoveProcess(uint8 *data)
{
    uint8 i,k=0;
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
    uint8 buf[8];
    osal_memset(buf,0,8);
    StringToHEX(data,buf,16);
    //NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<5;i++)
    {
        if(CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            _LeaveNet(AssoList[i].aucMAC);
            osal_memset(&AssoList[i],0,sizeof(NODE_INFO_t));
        }  
    }
    osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
}
uint8 RemoveDevice(RawData Setting)
{
    uint8 i;
    uint8 mac[5][17];
    osal_memset(mac,0,sizeof(mac));
    Setting.data[0] =(uint32 *)&mac[0][0];
    Setting.data[1] =(uint32 *)&mac[1][0];
    Setting.data[2] =(uint32 *)&mac[2][0];
    Setting.data[3] =(uint32 *)&mac[3][0];
    Setting.data[4] =(uint32 *)&mac[4][0];
    Setting.length = 5;
    Setting.type = CHAR;
    AnaDataProcess(&Setting);
    for(i=0;i<5;i++)
    {
        if(mac[i][0]==0x00)
            break;
        _RemoveProcess(mac[i]);
    }

}

/*
允许鉴权通过的设备入网
*/
void _AcceptProcess(uint8 *data)
{
   uint8 i,k,flag=0,flag1=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    //NODE_INFO_Group *h=(NODE_INFO_Group *)buffer;
    uint8 buf[8];
    osal_memset(buf,0,8);
    //NODE_INFO_t buffer[5];
    //osal_memset(buffer,1,sizeof( NODE_INFO_t )*5);
    //HEXtoString(data,buf);
    //step.1 字符串转数值
    //step.2 往硬盘上写
        // 循环5次 找相等 ->覆盖
        //没找到 ->在第一个为0
    //step.3 在找到的位置写入数据
    StringToHEX(data,buf,16);
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(CompareMac(buf,p->device[i].aucMAC)==1)
        {
            k=i;
            flag=1;
            break;
        }
    }
    if(flag==0)//内存无数据，上位机发送有误
        return;
    flag=0;
    for(i=0;i<5;i++)
    {
        if(CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            osal_memcpy(&AssoList[i],&p->device[k],sizeof(NODE_INFO_t));
            osal_memset(&p->device[k],0,sizeof(NODE_INFO_t));
            osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
            return;
        }
        if(AssoList[i].uiNwk_Addr == 0 || AssoList[i].uiNwk_Addr == 0xffff && flag==0)
        {
            flag1=i;
            flag=1;
        }
    }
    if(flag == 0)//硬盘无空间存储数据
        return;
    osal_memcpy(&AssoList[flag1],&p->device[k],sizeof(NODE_INFO_t));
    osal_memset(&p->device[k],0,sizeof(NODE_INFO_t)); 
    osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );

}
void _RemoveJoin()
{
    uint8 i=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<5;i++)
    {
        if(p->device[i].uiNwk_Addr !=0 )
            _LeaveNet(p->device[i].aucMAC);
    }
}
uint8 AcceptJoin(RawData Setting)
{
    uint8 i;
    uint8 *mac;
    //uint8 mac[5][16];
    mac = osal_mem_alloc(80);
    osal_memset(mac,0,80);
    Setting.data[0] =(uint32 *)&mac[0];
    Setting.data[1] =(uint32 *)&mac[16];
    Setting.data[2] =(uint32 *)&mac[32];
    Setting.data[3] =(uint32 *)&mac[48];
    Setting.data[4] =(uint32 *)&mac[64];
    Setting.length = 5;
    Setting.type = CHAR;
    AnaDataProcess(&Setting);
    for(i=0;i<5;i++)
    {
        if(mac[16*i]==0x00)
            break;
        _AcceptProcess(&mac[16*i]);
    }
    _RemoveJoin();    
    osal_mem_free(mac);
}

/*
AddInfo
*/
uint8 AddInfo(RawData Setting)
{
    uint8 i=0,j=0,k=0;
    char buffer[256];
    afAddrType_t dstAddr;
    osal_memset(buffer,0,256);
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    addInfo    *s=(addInfo *)&buffer[4];
    strcat(buffer,"+OK=");

    for(j=0;j<5;j++)
    {
        for(i=0;i<5;i++)
        {
            if(p->device[i].uiNwk_Addr==AssociatedDevList[j].shortAddr && p->device[i].uiNwk_Addr !=0 && p->device[i].uiNwk_Addr !=0xFFFF)
            {
                if(k!=0)
                    strcat(buffer,";");
                s=(addInfo *)&buffer[strlen(buffer)];
                HEXtoString(p->device[i].aucMAC,(uint8 *)(s)->mac,16);
                strcat( buffer,",");
                osal_memcpy(&s->name,&p->device[i].factoryName[1],16);
                //strcat( buffer,",00");
                //osal_memcpy(s->sel,s->mac,16);
                k++;
            }                   
        }
    }

    strcat(buffer,"\r\n"); 
    SendDatatoComputer(buffer);
}

/*
AddStatus
*/
uint8 AddStatus(RawData Setting)
{
    /*
    example:+OK=sel,deviceName,batteryValue,version,sensorType,0x31
    */
    uint8 i=0,j=0,k=0;
    uint8 buffer[256];
    osal_memset(buffer,0,256);
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    addStatus    *s=(addStatus *)&buffer[4];
    strcat(buffer,"+OK=");
    uint8 buf[Z_EXTADDR_LEN*2+1];
    osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
    //Device_type_Init();
    for(j=0;j<5;j++)
    {
        for(i=0;i<5;i++)
        {
            if(p->device[i].uiNwk_Addr==AssociatedDevList[j].shortAddr && p->device[i].uiNwk_Addr !=0 && p->device[i].uiNwk_Addr !=0xFFFF)
            {
                if(k!=0)
                    strcat(buffer,";");
                s=(addStatus *)&buffer[strlen(buffer)];
                HEXtoString(p->device[i].aucMAC,(uint8 *)(s)->mac,16);
                strcat( buffer,",");
                osal_memcpy(&buffer[strlen(buffer)],p->device[i].deviceType,strlen(p->device[i].deviceType));
                //strcat( buffer,"TestDevice");
                strcat(buffer,",");
                //strcat(buffer,"3");
                sprintf(&buffer[strlen(buffer)],"%d",p->device[i].batteryValue);
                strcat(buffer,",");
                strcat( buffer,"1.5");
                strcat(buffer,",");
                //sprintf(&buffer[strlen(buffer)],"%d",p->device[i].sensorType);
                //strcat(buffer,",");
                strcat(buffer,"1");
                k++;
            }                   
        }
    }
    strcat(buffer,"\r\n"); 
    SendDatatoComputer(buffer);
}

/*
OnlineDevice
*/
void SendCommond()
{
    afAddrType_t dstAddr;
    devicetype_t device;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=DeviceStatus[setFlag].uiNwk_Addr;
    dstAddr.endPoint=1;
    device = Indexofdevice(DeviceStatus[setFlag].deviceType);
    switch(device)
    {

        case light:
            if(DeviceStatus[setFlag].status[0]==0)
                zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
            else if(DeviceStatus[setFlag].status[0]==1)
                zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
            break; 
        case level:
            if(DeviceStatus[setFlag].status[0]==0)
                zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
            else if(DeviceStatus[setFlag].status[0]==1)
                zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
            DelayMS(100);
            zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,DeviceStatus[setFlag].status[1],10,  false, 0);
            break; 
        case colortem: 
            if(DeviceStatus[setFlag].status[0]==0)
                zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
            else if(DeviceStatus[setFlag].status[0]==1)
                zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
            zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,DeviceStatus[setFlag].status[1],10,  FALSE, 0);
            zha_project_Light_Color_Status=DeviceStatus[setFlag].status[2];
            zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 1, &dstAddr,zha_project_Light_Color_Status, 10,   FALSE, 0 );                  
            break;                     
            
        case slsensor:
            zclWriteCmd_t AttriList;
            if(DeviceStatus[setFlag].status[0]<=3)
            {
                zha_project_Warning = DeviceStatus[setFlag].status[0];
                //zclSS_Send_IAS_WD_StartWarningCmd(1, &dstAddr, zha_project_Warning,  false, 0);
            }
            else if(DeviceStatus[setFlag].status[0]>=4 & DeviceStatus[setFlag].status[0]<6)
            {
                zha_project_WD_SQUAWK = DeviceStatus[setFlag].status[0]-4;
                //zclSS_Send_IAS_WD_SquawkCmd(1, &dstAddr, zha_project_WD_SQUAWK,  false, 0);
            }
            zha_project_WD_Duration = DeviceStatus[setFlag].status[1];
            AttriList.numAttr=1;
            AttriList.attrList[0].attrID=ATTRID_SS_IAS_WD_MAXIMUM_DURATION;
            AttriList.attrList[0].dataType=ZCL_DATATYPE_UINT8;
            AttriList.attrList[0].attrData = &zha_project_WD_Duration;
            zcl_SendWrite(1, &dstAddr,
                        ZCL_CLUSTER_ID_SS_IAS_WD, &AttriList,
                        ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
            break;
        case outlet:
            if(DeviceStatus[setFlag].status[0]==0)
                zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
            else if(DeviceStatus[setFlag].status[0]==1)
                zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
            break;                      
        default:
            break;                
    
    }

}


uint16 __GetTime(uint16 addr)
{
    uint8 i;
    uint16 time;
    for(i=0;i<5;i++)
    {
        if(addr == AssociatedDevList[i].shortAddr)
          time = AssociatedDevList[i].endDev.deviceTimeout - AssociatedDevList[i].timeoutCounter;
          return time;
            
    }
    return 0;
}

void _SetOnlineDeveice(uint8 *data)
{
    //1.查询mac地址是否在硬盘表中
    //2.如果存在，则将短地址设为目的地址
    //3.根据设备的OD来判断发送的指令
    uint8 i;
    devicetype_t device;
    uint8 buf[8];
    osal_memset(buf,0,8);
    afAddrType_t dstAddr;
    StringToHEX(&data[0],buf,16);
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );

    for(i=0;i<5;i++)
    {
        if(CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            osal_memcpy(DeviceStatus[i].deviceType,AssoList[i].deviceType,16);
            DeviceStatus[i].status[0]=data[17];
            DeviceStatus[i].status[1]=data[18];
            DeviceStatus[i].status[2]=((data[19] &0x00FF)<<8)+data[20];
            DeviceStatus[i].uiNwk_Addr=AssoList[i].uiNwk_Addr;
            setFlag = i;
            //osal_set_event( zha_project_TaskID, SET_DEVICE_STATE_EVT );
            osal_start_timerEx( zha_project_TaskID, SET_DEVICE_STATE_EVT ,100);
//            dstAddr.addrMode=afAddr16Bit;
//            dstAddr.addr.shortAddr=AssoList[i].uiNwk_Addr;
//            dstAddr.endPoint=1;
//            device = Indexofdevice(AssoList[i].deviceType);
//            switch(device)
//            {
//
//                case light:
//                    if(data[17]==0)
//                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
//                    else if(data[17]==1)
//                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
//                    break; 
//                case level:
//                    if(data[17]==0)
//                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
//                    else if(data[17]==1)
//                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
//                    DelayMS(100);
//                    zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,data[18],10,  false, 0);
//                    break; 
//                case colortem: 
//                    if(data[17]==0)
//                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
//                    else if(data[17]==1)
//                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
//                    zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,data[18],10,  FALSE, 0);
//                    zha_project_Light_Color_Status=((data[19] &0x00FF)<<8)+data[20];
//                    zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 1, &dstAddr,zha_project_Light_Color_Status, 10,   FALSE, 0 );                  
//                    break;                     
//                    
//            }


//                case slsensor:
//                    zclWriteCmd_t AttriList;
//                    if(data[17]<=3)
//                    {
//                        zha_project_Warning = data[17];
//                        //zclSS_Send_IAS_WD_StartWarningCmd(1, &dstAddr, zha_project_Warning,  false, 0);
//                    }
//                    else if(data[17]>=4 & data[17]<6)
//                    {
//                        zha_project_WD_SQUAWK = data[17]-4;
//                        //zclSS_Send_IAS_WD_SquawkCmd(1, &dstAddr, zha_project_WD_SQUAWK,  false, 0);
//                    }
//                    zha_project_WD_Duration = data[18];
//                    AttriList.numAttr=1;
//                    AttriList.attrList[0].attrID=ATTRID_SS_IAS_WD_MAXIMUM_DURATION;
//                    AttriList.attrList[0].dataType=ZCL_DATATYPE_UINT8;
//                    AttriList.attrList[0].attrData = &zha_project_WD_Duration;
//                    zcl_SendWrite(1, &dstAddr,
//                                ZCL_CLUSTER_ID_SS_IAS_WD, &AttriList,
//                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
//                    break;
//                case outlet:
//                    if(data[17]==0)
//                        //zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, FALSE, 0 );
//                        zcl_SendCommand( 1, &dstAddr, ZCL_CLUSTER_ID_GEN_ON_OFF, COMMAND_OFF, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0, 0, 0, NULL );
//                    else if(data[17]==1)
//                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
//                    break;                      
//                default:
//                    break;                
//            
//            }
        }

    }
}

void _GetOnlineDeveice()
{
    //1.查询在线设备
    //2.根据设备的序列号来索引相应的设备状态
    uint8 i,j=0,k=0;
    uint16 time;
    devicetype_t device;
    uint8 lightlevel;
    int num=0;
    uint8 sendBuffer[200];
    uint8 buf[17];
    osal_memset(sendBuffer,0,200);
    osal_memset(buf,0,17);
    afAddrType_t dstAddr;
    j = sprintf(sendBuffer,"+OK=");
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(AssoList[i].uiNwk_Addr!=0 && AssoList[i].uiNwk_Addr!=0xffff)
        {
            osal_memcpy(DeviceStatus[i].deviceType,AssoList[i].deviceType,16);
            DeviceStatus[i].uiNwk_Addr=AssoList[i].uiNwk_Addr;
            HEXtoString(AssoList[i].aucMAC,buf,16);
            time = __GetTime(AssoList[i].uiNwk_Addr);
            if(k!=0)
                strcat(sendBuffer,";");
            device = Indexofdevice(AssoList[i].deviceType);
            switch(device)
            {
                case temp:
                    zha_project_Temperature_Value = (int)DeviceStatus[i].status[0];
                    num = (zha_project_Temperature_Value/100);
                    if(abs(zha_project_Temperature_Value- num*100) < 10)
                        j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d.0%d",buf,"temp",time,num,abs(zha_project_Temperature_Value- num*100));
                    else
                        j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d.%d",buf,"temp",time,num,abs(zha_project_Temperature_Value- num*100));
                    break;  
                case light:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d",buf,"light",time,DeviceStatus[i].status[0]);
                    break; 
                case level:
                    lightlevel = DeviceStatus[i].status[1]/2.55;
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"level",time,DeviceStatus[i].status[0],lightlevel);
                    break; 
                case colortem: 
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d,%d",buf,"colortem",time,DeviceStatus[i].status[0],DeviceStatus[i].status[1],DeviceStatus[i].status[2]);
                    break; 
                case pir:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"pir",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break; 
                case humility:
                    zha_project_Humidity_Value = DeviceStatus[i].status[0];
                    num = (zha_project_Humidity_Value/100);
                    if((zha_project_Humidity_Value- num*100) < 10)
                        j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d.0%d",buf,"humility",time,num,zha_project_Humidity_Value- num*100);
                    else
                        j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d.%d",buf,"humility",time,num,zha_project_Humidity_Value- num*100);
                    break;  
                case doorsen:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"doorsen",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break;    
                case lumin:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d",buf,"lumin",time,DeviceStatus[i].status[0]);
                    break; 
                case slsensor:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"slsensor",time,(DeviceStatus[i].status[0]  + DeviceStatus[i].status[1]),DeviceStatus[i].status[2]); 
                    break; 
                case smoke:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"smoke",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1));
                    break; 
                case 0x2BE1:
                    break;   
                case watersen:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"watersen",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break; 
                case cosensor:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"cosensor",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break;
                case gassensor:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"gassensor",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break;
                case glasssen:
                    j += sprintf(sendBuffer+j+k,"%s,%s,%d,%d,%d",buf,"glasssen",time,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break;
                case 0x2C88:
                    
                    break;
                case 0x2B00:
                    
                    break;
                case outlet:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,"outlet",time,DeviceStatus[i].status[0]);
                    break;
                default:
                    break;
            }
            k++;
        }

    }
    strcat(sendBuffer,"\r\n");
    HalUARTWrite(HAL_UART_PORT_0,sendBuffer,strlen(sendBuffer));
}

uint8 OnlineDevice(RawData Setting)
{
    char AT_Format[]="%[^,],%d,%d,%d";
    uint8 buffer[20];
    uint8 i,sign=0;
    osal_memset(buffer,0,20);
    afAddrType_t dstAddr;

    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == ',')
            {
                sign = 1;
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(sign == 1)
        {
            sscanf(Setting.RecData,AT_Format,&buffer[0],&buffer[17],&buffer[18],&buffer[19]);
//            afAddrType_t dstAddr;
//            dstAddr.addrMode=afAddr16Bit;
//            dstAddr.addr.shortAddr=AssociatedDevList[0].shortAddr;
//            dstAddr.endPoint=1;
//            zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, FALSE, 0 );
//            zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,1,10,  false, 0);
            _SetOnlineDeveice(buffer);
            HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        }
    }else
    {
        _GetOnlineDeveice();
    }

}

void CheckDeviceStatus()
{
    //1.遍历硬盘表中的设备类型，将硬盘里有的设备的短地址和设备OD放入到设备状态表中
    //2.根据设备类型和短地址发送相应的查询信息
    uint8 i,k=0;
    devicetype_t device;
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList[3];
    uint16 clusterID[3];
    
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(AssoList[i].uiNwk_Addr!=0 && AssoList[i].uiNwk_Addr !=0xffff)
        {
            DeviceStatus[k].uiNwk_Addr = AssoList[i].uiNwk_Addr;
            osal_memcpy(DeviceStatus[k].deviceType,AssoList[i].deviceType,sizeof(AssoList[i].deviceType));
            k++;
        }
    }
    for(k=0;k<5;k++)
    {
        osal_memset(&BasicAttrsList,0,3*sizeof(zclReadCmd_t));
        osal_memset(&clusterID,0,3*sizeof(uint16));
        if(DeviceStatus[k].uiNwk_Addr!=0 && DeviceStatus[k].uiNwk_Addr !=0xffff)
        {
            dstAddr.addrMode=afAddr16Bit;
            dstAddr.addr.shortAddr=DeviceStatus[k].uiNwk_Addr;
            dstAddr.endPoint=1;
            device = Indexofdevice(DeviceStatus[k].deviceType);
            switch(device)
            {
                case light:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_ON_OFF;
                    clusterID[0] = ZCL_CLUSTER_ID_GEN_ON_OFF;
                    break; 
                case level:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_ON_OFF;
                    clusterID[0] = ZCL_CLUSTER_ID_GEN_ON_OFF;
                    BasicAttrsList[1].numAttr = 1;
                    BasicAttrsList[1].attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
                    clusterID[1] = ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL;
                    break; 
            
                default:
                    break;
            }
            for(i=0;i<3;i++)
            {
                if(BasicAttrsList[i].numAttr !=0)
                {
                    zcl_SendRead( 1, &dstAddr,
                                clusterID[i], &BasicAttrsList[i],
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, DeviceStatus[k].seq);    
                }
                
            }
        }  
        
    }
}