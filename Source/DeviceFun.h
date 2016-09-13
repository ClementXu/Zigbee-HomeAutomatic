
#ifdef __cplusplus

extern "C"
{
#endif

uint8 LeaveNet(RawData Setting);
uint8 Light(RawData Setting);
uint8 Level(RawData Setting);
uint8 Pir(RawData Setting);
uint8 Temp(RawData Setting);
uint8 Colortem(RawData Setting);
uint8 Humility(RawData Setting);
uint8 Doorsen(RawData Setting);
uint8 Slsensor(RawData Setting);
uint8 Smoke(RawData Setting);
uint8 Lumin(RawData Setting);
uint8 Watersen(RawData Setting);
uint8 Cosensor(RawData Setting);
uint8 Gassensor(RawData Setting);
uint8 Glasssen(RawData Setting);
uint8 Outlet(RawData Setting);
void sendReport();
#ifdef __cplusplus
}
#endif