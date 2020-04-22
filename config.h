#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct  comportParam_tag
{
	char comName[32];
	char comBaudrate[16];
	unsigned char comWordLen;
	char comParity[8];
	unsigned char comStopBit;
	unsigned char comWaitForDsr;
	unsigned char comWaitForCts;
	unsigned char comDtrControl;
	unsigned char comRtsControl;
}comportParam_t;

typedef struct  netportParam_tag
{	
	char netName[8];
	char netAddr[32];
	char netMask[32];
	char netGateway[32];
	char netBroadcast[32];

}netportParam_t;

typedef struct modbusRtuport_tag
{
	comportParam_t com1Param;
	comportParam_t com2Param;
	comportParam_t com3Param;
	comportParam_t com4Param;
}modbusRtuport_t;

typedef struct modbusTcpport_tag
{
	netportParam_t net0Param;
	netportParam_t net1Param;
}modbusTcpport_t;

typedef struct modbusPort_tag
{
	modbusRtuport_t modbusRtuport;
	modbusTcpport_t modbusTcpport;
}modbusConfParam_t;

typedef struct mqttConfParam_tag
{
	int mqttInternal;
	int mqttServerPort;
	char mqttServerAddr[16];
}mqttConfParam_t;



//device data structor
/*----------------------------------------------*/
typedef struct dataUnit_tag
{
	char RegisterType[16];
	char DataType[16];
    int  Address; 
    int  Lenth;    
    int  Scale;
}dataUnit_t;

typedef struct primarySensorSupplyWaterTemp_tag
{
	dataUnit_t supplyWaterTemperature;
}primarySensorSupplyWaterTemp_t;


typedef struct primarySensorBackWaterTemp_tag
{
	dataUnit_t backWaterTemperature;
}primarySensorBackWaterTemp_t;


typedef struct  primarySensorSupplyWaterPressure_tag
{
	dataUnit_t supplyWaterPressure;
}primarySensorSupplyWaterPressure_t;

typedef struct primarySensorBackWaterPressure_tag
{
	dataUnit_t backWaterPressure;
}primarySensorBackWaterPressure_t;

typedef struct primarySensorFlowMeter_tag
{
	dataUnit_t flowRate;
    dataUnit_t totalFlow;
    dataUnit_t instantaneousHeat;
    dataUnit_t totalHeat;
}primarySensorFlowMeter_t;


typedef struct primaryOperatorDistPump_tag
{
	dataUnit_t startstop;
    dataUnit_t setFrequency;
    dataUnit_t frequency;
    dataUnit_t current;
    dataUnit_t state;
    dataUnit_t error;
    dataUnit_t controlMethod;
}primaryOperatorDistPump_t;


typedef struct primaryOperatorRegularValve_tag
{
	dataUnit_t opening;
    dataUnit_t setOpening;
    dataUnit_t error;
    dataUnit_t controlMethod;
}primaryOperatorRegularValve_t;

typedef struct primaryOperatorElectricValve_tag
{
	dataUnit_t open;
    dataUnit_t close;
    dataUnit_t operateTime;
    dataUnit_t opening;
    dataUnit_t error;
    dataUnit_t controlMethod;
}primaryOperatorElectricValve_t;


typedef struct secondarySensorSupplyWaterTemp_tag
{
	dataUnit_t supplyWaterTemperature;
}secondarySensorSupplyWaterTemp_t;


typedef struct secondarySensorBackWaterTemp_tag
{
	dataUnit_t backWaterTemperature;
}secondarySensorBackWaterTemp_t;


typedef struct secondarySensorSupplyWaterPressure_tag
{
	dataUnit_t supplyWaterPressure;
}secondarySensorSupplyWaterPressure_t;


typedef struct secondarySensorBackWaterPressure_tag
{
	dataUnit_t backWaterPressure;
}secondarySensorBackWaterPressure_t;


typedef struct secondarySensorFlowMeter_tag
{
	dataUnit_t flowRate;
    dataUnit_t totalFlow;
    dataUnit_t instantaneousHeat;
    dataUnit_t totalHeat;
}secondarySensorFlowMeter_t;


typedef struct secondaryOperatorCirculatPump_tag
{
	dataUnit_t startstop;
    dataUnit_t setFrequency;
    dataUnit_t frequency;
    dataUnit_t current;
    dataUnit_t state;
    dataUnit_t error;
    dataUnit_t controlMethod;
}secondaryOperatorCirculatPump_t;


typedef struct secondaryOperatorFeedWaterPump_tag
{
	dataUnit_t startstop;
    dataUnit_t setFrequency;
    dataUnit_t frequency;
    dataUnit_t current;
    dataUnit_t state;
    dataUnit_t error;
    dataUnit_t controlMethod;
}secondaryOperatorFeedWaterPump_t;


typedef struct secondaryOperatorFeedWaterValve_tag
{
	dataUnit_t startstop;
    dataUnit_t state;
    dataUnit_t controlMethod;
}secondaryOperatorFeedWaterValve_t;

typedef struct secondaryOperatorReleaseValve_tag
{
	dataUnit_t startstop;
    dataUnit_t state;
    dataUnit_t controlMethod;
}secondaryOperatorReleaseValve_t;

typedef struct otherSensorWaterTankLevel_tag
{
	dataUnit_t waterTankLevel;
}otherSensorWaterTankLevel_t;


typedef struct otherSensorWaterMeter_tag
{
	dataUnit_t waterMeter;
}otherSensorWaterMeter_t;

typedef struct otherSensorOutTemp_tag
{
	dataUnit_t outDoorTemperature;
}otherSensorOutTemp_t;

typedef struct otherSensorSumpPitWaterLevel_tag
{
	dataUnit_t sumpPitWaterLevel;
    dataUnit_t sumpPitWaterState;
}otherSensorSumpPitWaterLevel_t;


typedef struct otherSensorPowerInt_tag
{
	dataUnit_t powerInterupt;
}otherSensorPowerInt_t;


typedef struct otherSensorEmergencyStop_tag
{
	dataUnit_t emergencyStop;
}otherSensorEmergencyStop_t;


typedef struct otherSensorElectricParam_tag
{
	dataUnit_t UA;
    dataUnit_t UB;
    dataUnit_t UC;
    dataUnit_t IA;
    dataUnit_t IB;
    dataUnit_t IC;
}otherSensorElectricParam_t;


typedef struct otherOperatorSubPump_tag
{
	dataUnit_t startstop;
    dataUnit_t state;
    dataUnit_t error;
    dataUnit_t controlMethod;
}otherOperatorSubPump_t;

typedef struct otherOperatorWaterValve_tag
{
	dataUnit_t startstop;
    dataUnit_t state;
    dataUnit_t error;
    dataUnit_t controlMethod;
}otherOperatorWaterValve_t;

typedef struct endDeviceConf_tag
{
	char area[32];
	char protocalType[16];
	char slaveId;
	char localPort[16];
	char slaveIp[16];
	int  slavePort;
	primarySensorSupplyWaterTemp_t 		  primarySensorSupplyWaterTemp;
	primarySensorBackWaterTemp_t 		  primarySensorBackWaterTemp;
	primarySensorSupplyWaterPressure_t 	  primarySensorSupplyWaterPressure;
	primarySensorBackWaterPressure_t 	  primarySensorBackWaterPressure;
	primarySensorFlowMeter_t 		  	  primarySensorFlowMeter;
	primaryOperatorDistPump_t    		  primaryOperatorDistPump;
	primaryOperatorRegularValve_t 		  primaryOperatorRegularValve;
	primaryOperatorElectricValve_t 		  primaryOperatorElectricValve;
	secondarySensorSupplyWaterTemp_t	  secondarySensorSupplyWaterTemp;
	secondarySensorBackWaterTemp_t 		  secondarySensorBackWaterTemp;
	secondarySensorSupplyWaterPressure_t  secondarySensorSupplyWaterPressure;
	secondarySensorBackWaterPressure_t	  secondarySensorBackWaterPressure;
	secondarySensorFlowMeter_t  		  secondarySensorFlowMeter;
	secondaryOperatorCirculatPump_t		  secondaryOperatorCirculatPump;
	secondaryOperatorFeedWaterPump_t	  secondaryOperatorFeedWaterPump;
	secondaryOperatorFeedWaterValve_t	  secondaryOperatorFeedWaterValve;
	secondaryOperatorReleaseValve_t		  secondaryOperatorReleaseValve;
	otherSensorWaterTankLevel_t		  	  otherSensorWaterTankLevel;
	otherSensorWaterMeter_t 			  otherSensorWaterMeter;
	otherSensorOutTemp_t 	 			  otherSensorOutTemp;
	otherSensorSumpPitWaterLevel_t		  otherSensorSumpPitWaterLevel;
	otherSensorPowerInt_t 				  otherSensorPowerInt;
	otherSensorEmergencyStop_t 			  otherSensorEmergencyStop;
	otherSensorElectricParam_t 			  otherSensorElectricParam;
	otherOperatorSubPump_t 				  otherOperatorSubPump;
	otherOperatorWaterValve_t 			  otherOperatorWaterValve;
}endDeviceConf_t;

typedef struct device_table_tag
{
	char *pKey;
	void *pDevice;
	int item;
	int memSize;
}device_table_t;

int configInit(void);

#endif