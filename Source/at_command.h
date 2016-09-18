


#ifdef __cplusplus
extern "C"
{
#endif
#define AT_GROUP_NUM 40
#define DEVICE_NUM 25 

  
typedef enum
{
	CHAR=0,
	VALUE_D,
        VALUE_MIX 
	
}DATA_TYPE;
  
typedef struct __rawData
{
	char *RecData;
	uint32 *data[10];//等待发送的数据（是个数组的首地址）
	uint8 length;//需要发送数据的个数
	DATA_TYPE type;//需要发送数据的类型
    uint8 RecSign;
}RawData;  
  
typedef enum
{
    ENUM_AT_MAC=1,
    ENUM_AT_FMVER,
    ENUM_AT_FACTORY,
    ENUM_AT_REBOOT,
    ENUM_AT_CHANNEL,
    ENUM_AT_PANID,
    ENUM_AT_GETEXTPID,
    ENUM_AT_DEVICE,
    ENUM_AT_SERIAL,
    
//#if ZG_BUILD_COORDINATOR_TYPE 
    ENUM_AT_FORM,
    ENUM_AT_LEAVE,
    ENUM_AT_PERMITJOIN,
    ENUM_AT_ADDINFO,
    ENUM_AT_ADDSTATUS,
    ENUM_AT_DENYJOIN,
    ENUM_AT_ACCEPTJOIN,
    ENUM_AT_ONLINE,
//#endif    



//#if ZG_BUILD_ENDDEVICE_TYPE    
    ENUM_AT_LIGHT,
    ENUM_AT_PIR,
    ENUM_AT_TEMP,        
    ENUM_AT_COLORTEM,
    ENUM_AT_HUMILITY,
    ENUM_AT_DOORSENSOR,
    ENUM_AT_SOUNDLIGHTSENSOR,
    ENUM_AT_SMOKEDETECTOR,
    ENUM_AT_LEVEL,
    ENUM_AT_LUMIN,    
    ENUM_AT_WATERSENSOR, 
    ENUM_AT_COSENSOR,
    ENUM_AT_GASSENSOR,
    ENUM_AT_GLASSSENSOR,
    ENUM_AT_ZONECONTROL,
    ENUM_AT_OUTLET,
    ENUM_AT_LIGHTSWITCH,
//#endif    
    ENUM_AT_REMOVEDEV,
}at_command;  

typedef enum
{
    light=0,
    level,
    colortem,
    temp,
    pir,
    humility,
    doorsen,
    lumin,
    slsensor,
    smoke,
    watersen,
    cosensor,
    gassensor,
    glasssen,
    zonectrl,
    alertleak,
    altermove,
    alterlight,
    outlet,
    doorbell,
    lightswitch,
}devicetype_t;

typedef struct __addInfo
{
    char mac[17];
    char name[17];
}addInfo;
typedef struct __addStatus
{
    char serial[18];
    char deviceName[10];
    uint8 batteryValue[2];
    uint8 version[4];
    uint8 sensorType[2];
    uint8 time[2];
     
}addStatus;

uint8 testat();
void SendDatatoComputer(char *p);
void CleanTempDevice();
void StringToHEX(uint8 *data ,uint8 *target,uint8 length);
void HEXtoString(uint8 *data , uint8 *targetBuf ,uint8 length );
uint16 CovertACSIIFOUR(uint8 *buf);
void sendDatatoComputer(RawData *Setting);
void AnaDataProcess(RawData *Setting);
unsigned char Indexofdevice(char *data);
void At_Command(char *recData);
void SendDatatoComputer(char *p);
void AT_Init();
uint8 CompareMac(uint8 *p, uint8 *mac);

#ifdef __cplusplus
}
#endif
