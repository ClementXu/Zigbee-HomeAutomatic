
#ifdef __cplusplus

extern "C"
{
#endif
 
void SetTempDeviceManuName(uint16 shortAddr,uint8 *buffer);
void SetTempDeviceEP(uint16 shortAddr,uint8 *buffer );
void SetTempDeviceHW(uint16 shortAddr,uint8 version);
void SetTempDeviceSA(uint16 data,uint8 *mac);
void UpdateDeviceStatus3(uint16 shortAddr, uint16 *data);
void UpdateDeviceStatus2(uint16 shortAddr, uint16 *data);
void UpdateDeviceStatus1(uint16 shortAddr, uint16 *data);
  
  
  
  
void SetTempDeviceType(uint16 shortAddr,uint16 change);
void SetTempDeviceBAT(uint16 shortAddr,uint8 battery);
uint8 FormNet(RawData Setting);
uint8 LeaveNet(RawData Setting);
uint8 PermitJoin(RawData Setting);
uint8 RemoveDevice(RawData Setting);
uint8 AcceptJoin(RawData Setting);
uint8 AddStatus(RawData Setting);
uint8 AddInfo(RawData Setting);
uint8 OnlineDevice(RawData Setting);
void CheckDeviceStatus();
void SendCommond();

#ifdef __cplusplus
}
#endif