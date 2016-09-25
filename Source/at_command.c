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
#include "zha_project.h"
#include "GatewayFun.h"
#include "DeviceFun.h"

extern NODE_INFO_t AssoList[];
const char *AT_GROUP[AT_GROUP_NUM];
const char *DEVICE_NAME[DEVICE_NUM];
uint8 (*commandProcess[AT_GROUP_NUM])(RawData Setting);
uint8 TempDevice[200];
//uint8 TempDevice1[200];

/*
数据发送函数
*/
void SendDatatoComputer(char *p)
{
    HalUARTWrite(HAL_UART_PORT_0,p,strlen(p));
}

/*
清空Buffer
*/
void CleanTempDevice()
{
    osal_memset(TempDevice,0,sizeof(TempDevice));
}

/*
比较MAC地址
*/
uint8 CompareMac(uint8 *p, uint8 *mac)
{
    uint8 i=0;
    
    for(i=0;i<5;i++)
    {
        if(p[i] != mac[i])
            return 0;
    }
    return 1;
}

uint8 CovertACSIITWO(uint8 *buf)
{
    uint8 i;
    for(i=0;i<2;i++)
    {
        if(buf[i]<=57)
        {
            buf[i]=(buf[i]-48);
        }
        else if(buf[i]<=103 && buf[i]>=97)
        {
            buf[i]=(buf[i]-87);
        }
    }
    return ((buf[0]&0x0F)<<4)+(buf[1]&0x0F);


}

uint8 CovertACSIITWOB(uint8 *buf)
{
    uint8 i;
    for(i=0;i<2;i++)
    {
        if(buf[i]<=57)
        {
            buf[i]=(buf[i]-48);
        }
        else if(buf[i]<=71 && buf[i]>=65)
        {
            buf[i]=(buf[i]-55);
        }
    }
    return ((buf[0]&0x0F)<<4)+(buf[1]&0x0F);


}

/*
将字符串转换为不可见字符
*/
void StringToHEX(uint8 *data ,uint8 *target,uint8 length)
{
      uint8 i;
      for(i=0;i<(length/2);i++)
      {
        target[length/2-i-1]=CovertACSIITWOB(data);
            data=data+2;
      }

}

/*
将不可见字符转换为字符串
*/
void HEXtoString(uint8 *data , uint8 *targetBuf ,uint8 length )
{
    uint8 i;
    uint8 *xad;
    //uint8 targetBuf[Z_EXTADDR_LEN*2+1]=0;
    // Display the extended address.
    xad = data + length/2 - 1;

    for (i = 0; i < length; xad--)
    {
            uint8 ch;
            ch = (*xad >> 4) & 0x0F;
            targetBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
            ch = *xad & 0x0F;
            targetBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    }      
    
}

uint16 CovertACSIIFOUR(uint8 *buf)
{
        uint8 i;
        for(i=0;i<4;i++)
        {
            if(buf[i]<=57)
            {
                buf[i]=(buf[i]-48);
            }
            else if(buf[i]<=103 && buf[i]>=97)
            {
                buf[i]=(buf[i]-87);
            }
        }
    return ((buf[0]&0x0F)<<12)+((buf[1]&0x0F)<<8)+((buf[2]&0x0F)<<4)+(buf[3]&0x0F);

}

/*
将要发送的数据打包发送
*/
void sendDatatoComputer(RawData *Setting)
{
    uint8 i=0,j=0;
    uint8 *format;
    format = osal_mem_alloc(200);
    //char format[200];
    osal_memset(format,0,sizeof(format));
    char valueSign[3];
    if(Setting->length==0)
    {
        sprintf(format,"+OK\r\n");
        HalUARTWrite(HAL_UART_PORT_0,format,strlen(format));
        return;
    }
    j = sprintf(format,"+OK=");
    
    if(Setting->type == VALUE_D)
    {
        strcpy(valueSign,"%d");
        for(i=0;i<Setting->length;i++)
        {
            if(i!=0)
            {
                strcat(format,",");
            }
            j += sprintf(format+j+i,valueSign,*(Setting->data[i]));
        }
    }
    else
    {
        strcpy(valueSign,"%s");
        for(i=0;i<Setting->length;i++)
        {
            if(i!=0)
            {
                strcat(format,",");
            }
            j += sprintf(format+j+i,valueSign,(char *)(Setting->data[i]));
        }        

    }
    strcat(format,"\r\n");     
    HalUARTWrite(HAL_UART_PORT_0,format,strlen(format));
    osal_mem_free(format);
    osal_memset(&Setting,0,sizeof(Setting));
}

/*
数据处理函数
*/
void AnaDataProcess(RawData *Setting)
{
    uint8 i;
    char AT_Format[32];
    osal_memset(AT_Format,0,32);
    if(Setting->type == VALUE_MIX)
        strcat(AT_Format,"%[^,],%[^,],%d,%d,%d");
    for(i=0;i<Setting->length;i++)
    {
        if(i==0)
        {
            if(Setting->type == VALUE_D)
                    strcat(AT_Format,"%d"); 
            else if(Setting->type == CHAR)
                    strcat(AT_Format,"%[^,]"); 
            else
                break;
        }
        else
        {
            if(Setting->type == VALUE_D)
                    strcat(AT_Format,",%d"); 
            else if(Setting->type == CHAR)
                    strcat(AT_Format,",%[^,]"); 
            else
                break;
        }
    }
    //清零发送缓存
    if(Setting->RecSign == 0)//strlen(Setting->RecData)==0
    {	//读取指令
        sendDatatoComputer(Setting);	
        //Setting->RecSign = 0; 
    }
    else
    {	//写入指令
        sscanf(Setting->RecData,AT_Format,Setting->data[0],Setting->data[1],Setting->data[2],Setting->data[3],Setting->data[4]) ;
        Setting->length = 0;
        //Setting->RecSign = 1;
        sendDatatoComputer(Setting);
    }
        
}

/*
AT测试函数
*/
uint8 testat()
{
    uint8 buffer[256];
    //uint8 buf[256];
    osal_memset(buffer,2,sizeof(buffer));
    //osal_memcpy(buf,buffer,255);
    HalUARTWrite(0, buffer,255);
    osal_memset(buffer,1,sizeof(buffer));
    HalUARTWrite(0, buffer,255);
}

/*
获取MAC地址
*/
uint8 MAC(RawData Setting)
{
    uint8 buf[Z_EXTADDR_LEN*2+1];
    osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
    HEXtoString(aExtendedAddress,buf,16);
    Setting.data[0] =(uint32 *)&buf;
    Setting.length = 1;
    Setting.type = CHAR;
    Setting.RecSign = 0;  
    AnaDataProcess(&Setting);
    
}

/*
恢复出厂设置
*/
uint8 Factory(RawData Setting)
{
    uint16 panid;
    uint32 channel;
    panid = 0xffff;
    channel=0x800;
    
    osal_nv_write(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);
    osal_nv_write(ZCD_NV_CHANLIST, 0, 4, &channel);
#if ZG_BUILD_COORDINATOR_TYPE    
    osal_memset(&AssoList,0,(sizeof( NODE_INFO_t )*5));
    osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
#endif
    uint8 startOptions =  ZCD_STARTOPT_CLEAR_STATE;///ZCD_STARTOPT_CLEAR_CONFIG |
    osal_nv_write( ZCD_NV_STARTUP_OPTION,0, sizeof(uint8), &startOptions );
    HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",strlen("+OK\r\n"));
    osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
    //osal_set_event( zha_project_TaskID,RESET_EVT);
}

/*
重启设备，信息不丢失
*/
uint8 Reboot(RawData Setting)
{
    uint8 startOptions =  0;
    osal_nv_write( ZCD_NV_STARTUP_OPTION,0, sizeof(uint8), &startOptions );
    HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",strlen("+OK\r\n"));
    osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
    //osal_set_event( zha_project_TaskID,RESET_EVT);
}

/*
获取和设置CHANNEL
*/
uint32 _GetChannel()
{
    uint8 i;
    uint32 channel=0;
    uint32 channellist;
    osal_nv_read(ZCD_NV_CHANLIST,0, sizeof(uint32), &channellist);
    for(i=0;i<32;i++)
    {
        channellist=channellist>>1;
        if(channellist&&0x01==1)
              channel ++;
    }
    return channel;
}

void _SetChannel(uint32 channel)
{
    switch(channel)
    {
        case 0x0B:
                channel=0x800;
        break;
        case 0x0C:
                channel=0x00001000;
        break;
        case 0x0D:
                channel=0x00002000;
        break;
        case 0x0E:
                channel=0x00004000;
        break;
        case 0x0F:
                channel=0x00008000;
        break;
        case 0x10:
                channel=0x00010000;
        break;
        case 0x11:
                channel=0x00020000;
        break;
        case 0x12:
                channel=0x00040000;
        break;
        case 0x13:
                channel=0x00080000;
        break;
        case 0x14:
                channel=0x00100000;
        break;         
        case 0x15:
                channel=0x00200000;
        break;  
        case 0x16:
                channel=0x00400000;
        break;  
        case 0x17:
                channel=0x00800000;
        break;  
        case 0x18:
                channel=0x01000000;
        break;  
        case 0x19:
                channel=0x02000000;
        break;  
        case 0x1A:
                channel=0x04000000;
        break;  
        default:
                channel=0x800;
        break;
    }
    osal_nv_write(ZCD_NV_CHANLIST, 0, 4, &channel);
}

uint8 Channel(RawData Setting)
{
    uint32 channel=_GetChannel();
    Setting.data[0] =&channel;
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    _SetChannel(channel);
}

/*
获取和设置PANID
*/
uint8 PANID(RawData Setting)
{
    uint32 AutoSign=0;//0 auto
    uint16 panid;
    osal_nv_read(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);//硬盘上的数据
    if(panid==_NIB.nwkPanId)
    {
        AutoSign=1;
    }
    panid = _NIB.nwkPanId;
    Setting.data[0] = &AutoSign;
    Setting.data[1] =(uint32 *)&panid;
    Setting.length = 2;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    if(panid==0||panid==0xffff)
        return 0;
    
    if(AutoSign==1)
    {
        osal_nv_write(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);
        _NIB.nwkPanId = panid;
        NLME_UpdateNV(0x01);
    }
    else
    {
        panid = 0xffff;
        osal_nv_write(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);
    }

}

/*
获取Extended PANID
*/
uint8 ExtendedPANID(RawData Setting)
{
    uint8 zgExtendedPANID[8];
    osal_cpyExtAddr( zgExtendedPANID, _NIB.extendedPANID );
    uint8 buf[Z_EXTADDR_LEN*2+1];
    osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
    // Display the extended address.
    HEXtoString(zgExtendedPANID,buf,16);

    Setting.data[0] =(uint32 *)&buf[0];
    Setting.length = 1;
    Setting.type = CHAR;
    AnaDataProcess(&Setting);
}

/*
设置和获取序列号
*/
uint8 Serial(RawData Setting)
{
    uint8 SerialNumber[10];
    uint8 buf[21];
    osal_memset(SerialNumber,0,10);
    osal_nv_read(ZCD_NV_DEVICE_SERIAL, 0, sizeof(SerialNumber),  &SerialNumber);
    osal_memset(buf,0,21);
    if(SerialNumber[0] == 0)
    {
        HEXtoString(&SerialNumber[1],buf,18);
    }
    else
    {
        HEXtoString(&SerialNumber[1],buf,SerialNumber[0]);

    }
    
    Setting.data[0] =(uint32 *)&buf;
    Setting.length = 1;
    Setting.type = CHAR;
    AnaDataProcess(&Setting);
    SerialNumber[0] = strlen(buf);
    StringToHEX(buf,&SerialNumber[1],strlen(buf));
    osal_nv_write(ZCD_NV_DEVICE_SERIAL, 0, sizeof(SerialNumber),  &SerialNumber);
}


//#if ZG_BUILD_ENDDEVICE_TYPE
/*
设备类型初始化
*/
void Device_type_Init()
{
    DEVICE_NAME[light]="light";
    DEVICE_NAME[level]="level";
    DEVICE_NAME[colortem]="colortem";
    DEVICE_NAME[temp]="temp";
    DEVICE_NAME[pir]="pir";
    DEVICE_NAME[humility]="humility";
    DEVICE_NAME[doorsen]="doorsen";
    DEVICE_NAME[lumin]="lumin";
    DEVICE_NAME[slsensor]="slsensor";
    DEVICE_NAME[smoke]="smoke";
    DEVICE_NAME[watersen]="watersen";
    DEVICE_NAME[lightswitch]="lightswitch";
    DEVICE_NAME[cosensor]="cosensor";
    DEVICE_NAME[gassensor]="gassensor";
    DEVICE_NAME[glasssen]="glasssen";
    DEVICE_NAME[zonectrl]="zonectrl";
    DEVICE_NAME[outlet]="outlet";
};

unsigned char Indexofdevice(char *data)
{
    unsigned char i=0;
    for(i=0;i<=DEVICE_NUM;i++)
    {
        if(strcmp(data,DEVICE_NAME[i])==0)
            return i;
    }
    return 0;
};
//#endif

/*
设备类型
*/
uint8 Device(RawData Setting)
{
	uint8 devicetype;
    char buffer[16];
    osal_memset(buffer,0,16);
#if ZG_BUILD_COORDINATOR_TYPE 
        strcpy(buffer,"gateway");  
        Setting.data[0] =(uint32 *)&buffer[0];
        Setting.length = 1;
        Setting.type = CHAR;
        AnaDataProcess(&Setting);
#endif
#if ZG_BUILD_ENDDEVICE_TYPE
        osal_nv_read(ZCD_NV_DEVICE_TYPE,0, sizeof(uint8), &devicetype);
        osal_memcpy(buffer,DEVICE_NAME[devicetype],strlen(DEVICE_NAME[devicetype]));
        Setting.data[0] =(uint32 *)&buffer[0];
        Setting.length = 1;
        Setting.type = CHAR;
        AnaDataProcess(&Setting);
        devicetype = Indexofdevice(buffer);
        osal_nv_write(ZCD_NV_DEVICE_TYPE, 0,sizeof(uint8), &devicetype);
#endif
}

/*
AT指令索引，返回对应序号
*/
unsigned char IndexofAT(char *data)
{
    unsigned char i=1;
    for(i=1;i<=AT_GROUP_NUM;i++)
    {
        if(strcmp(data,AT_GROUP[i])==0)
            return i;
    }
    return 0;
};

/*
AT指令初始化
*/
void AT_Init()
{
    //通用控制指令
    AT_GROUP[ENUM_AT_MAC]="MAC";
    AT_GROUP[ENUM_AT_FMVER]="FMVER";
    AT_GROUP[ENUM_AT_FACTORY]="FACTORY";
    AT_GROUP[ENUM_AT_REBOOT]="REBOOT";
    AT_GROUP[ENUM_AT_CHANNEL]="CHANNEL";
    AT_GROUP[ENUM_AT_PANID]="PANID";
    AT_GROUP[ENUM_AT_GETEXTPID]="GETEXTPID";
    AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
    AT_GROUP[ENUM_AT_SERIAL]="SERIAL";
    commandProcess[ENUM_AT_MAC] = MAC;
    commandProcess[ENUM_AT_PANID] = PANID;
    commandProcess[ENUM_AT_FACTORY] = Factory;
    commandProcess[ENUM_AT_REBOOT] = Reboot;
    commandProcess[ENUM_AT_CHANNEL] = Channel;
    commandProcess[ENUM_AT_GETEXTPID] = ExtendedPANID;
    commandProcess[ENUM_AT_DEVICE] = Device;
    commandProcess[ENUM_AT_SERIAL] = Serial;

#if ZG_BUILD_COORDINATOR_TYPE   
    AT_GROUP[ENUM_AT_FORM]="FORM";
    AT_GROUP[ENUM_AT_LEAVE]="LEAVE";
    AT_GROUP[ENUM_AT_PERMITJOIN]="PERMITJOIN";
    AT_GROUP[ENUM_AT_DENYJOIN]="DENYJOIN";
    AT_GROUP[ENUM_AT_ADDINFO]="ADDINFO";
    AT_GROUP[ENUM_AT_ADDSTATUS]="ADDSTATUS";
    AT_GROUP[ENUM_AT_ACCEPTJOIN] = "ACCEPTJOIN";
    AT_GROUP[ENUM_AT_ONLINE]="ONLINE"; 
    AT_GROUP[ENUM_AT_REMOVEDEV]="REMOVEDEV";
    
    commandProcess[ENUM_AT_FORM] =FormNet;
    commandProcess[ENUM_AT_LEAVE] =LeaveNet;
    commandProcess[ENUM_AT_PERMITJOIN] = PermitJoin;
    commandProcess[ENUM_AT_ADDINFO] = AddInfo;
    commandProcess[ENUM_AT_REMOVEDEV] = RemoveDevice;
    commandProcess[ENUM_AT_ACCEPTJOIN] = AcceptJoin;
    commandProcess[ENUM_AT_ADDSTATUS] = AddStatus;
    commandProcess[ENUM_AT_ONLINE] = OnlineDevice;
    
#endif   
#if ZG_BUILD_ENDDEVICE_TYPE
    AT_GROUP[ENUM_AT_LIGHT]="LIGHT";
    AT_GROUP[ENUM_AT_PIR]="PIR";
    AT_GROUP[ENUM_AT_LEAVE]="LEAVE";
    AT_GROUP[ENUM_AT_TEMP]="TEMP";
    AT_GROUP[ENUM_AT_COLORTEM]="COLORTEM";
    AT_GROUP[ENUM_AT_HUMILITY]="HUMILITY";
    AT_GROUP[ENUM_AT_DOORSENSOR]="DOORSEN";
    AT_GROUP[ENUM_AT_SOUNDLIGHTSENSOR]="SLSENSOR";
    AT_GROUP[ENUM_AT_SMOKEDETECTOR]="SMOKE";
    AT_GROUP[ENUM_AT_LEVEL]="LEVEL";
    AT_GROUP[ENUM_AT_LUMIN]="LUMIN";
    AT_GROUP[ENUM_AT_COSENSOR]="COSENSOR";
    AT_GROUP[ENUM_AT_GASSENSOR]="GASSENSOR";
    AT_GROUP[ENUM_AT_GLASSSEN]="GLASSSEN";
    AT_GROUP[ENUM_AT_ZONECONTROL]="ZONECONTROL";
    AT_GROUP[ENUM_AT_LIGHTSWITCH]="LIGHTSWITCH";
    AT_GROUP[ENUM_AT_OUTLET]="OUTLET";    
    
    commandProcess[ENUM_AT_LEAVE] =LeaveNet;
    commandProcess[ENUM_AT_LIGHT] =Light;
    commandProcess[ENUM_AT_PIR] =Pir;
    commandProcess[ENUM_AT_TEMP] = Temp;
    commandProcess[ENUM_AT_COLORTEM] = Colortem;
    commandProcess[ENUM_AT_HUMILITY] = Humility;
    commandProcess[ENUM_AT_DOORSENSOR] = Doorsen;
    commandProcess[ENUM_AT_SOUNDLIGHTSENSOR] = Slsensor;
    commandProcess[ENUM_AT_SMOKEDETECTOR] = Smoke;  
    commandProcess[ENUM_AT_LEVEL] = Level;
    commandProcess[ENUM_AT_LUMIN] = Lumin;
    commandProcess[ENUM_AT_COSENSOR] = Cosensor;
    commandProcess[ENUM_AT_GASSENSOR] = Gassensor;
    commandProcess[ENUM_AT_GLASSSEN] = Glasssen;
    //commandProcess[ENUM_AT_ZONECONTROL] = Zonecontrol;
    commandProcess[ENUM_AT_OUTLET] = Outlet;
    //commandProcess[ENUM_AT_LIGHTSWITCH] = Lightswitch;
#endif
    
};

/*
AT指令集主函数
*/
void At_Command(char *recData)
{
    RawData Setting;
    char AT_Name[15];
    
    uint8 Index;
    
    char AT_Format[]="AT+%[^=]";
    osal_memset(AT_Name,0,sizeof(AT_Name));
    osal_memset(&Setting,0,sizeof(RawData));
	//第一步，读取AT指令名称
	if(sscanf(recData,AT_Format ,AT_Name)!=0)
	{	//解析成功
		Index = IndexofAT(AT_Name);//给赋值
        if(Index != 0)
        {
            if(recData[strlen(AT_Name)+3] == '=')
            {//设置指令
                Setting.RecData = 0;
                Setting.RecData = &recData[strlen(AT_Name)+4];//
                Setting.RecSign = 1;
                commandProcess[Index](Setting);
            }
            else
            {	//读取指令
                commandProcess[Index](Setting);
                Setting.RecSign = 0;  
            }
        }
        else
        {
            uint8 buffer[16];
            osal_memset(buffer,0,16);
            sprintf(buffer,"ERROR:=%s\r\n","NO SUCH COMMAND");
            SendDatatoComputer(buffer);            
        }
	}
	else
	{	//解析失败
		uint8 buffer[16];
        osal_memset(buffer,0,16);
        sprintf(buffer,"ERROR:=%s\r\n","COMMAND ERROR");
        SendDatatoComputer(buffer);
	}    
}
