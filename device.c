#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>   
#include <sys/msg.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>

#include "modbus/modbus.h"
#include "cjson/cJSON.h"

#include "common.h"
#include "device.h"
#include "config.h"


#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# ifndef _MSC_VER
# include <stdint.h>
# else
# include "stdint.h"
# endif
#endif

#define SERVER_ID         17
#define INVALID_SERVER_ID 22

/*-----------------------------------------------------------*/
#define pdevToCloudUpdateContent 	"{ \
	\"DeviceID\": \"12345678987654321\", \
	\"DeviceIP\": \"109.56.23.44\", \
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"ActiveUpload\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"primaryPipeMeasureData\": {\
			\"supplyWaterTemperature\": 60.01,\
			\"backWaterTemperature\": 60.01,\
			\"supplyWaterPressure\": 1.600,\
			\"backWaterPressure\": 1.600,\
			\"flowRate\": 100.01,\
			\"totalFlow\": 100.01,\
			\"instantaneousHeat\": 1000.01,\
			\"totalHeat\": 100.0001\
		},\
		\"secondaryPipeMeasureData\": {\
			\"supplyWaterTemperature\": 60.01,\
			\"backWaterTemperature\": 60.01,\
			\"supplyWaterPressure\": 1.600,\
			\"backWaterPressure\": 1.600,\
			\"flowRate\": 100.01,\
			\"totalFlow\": 100.01,\
			\"instantaneousHeat\": 1000.01,\
			\"totalHeat\": 100.0001\
		},\
		\"otherMeasureData\": {\
			\"waterTankLevel\": 99.99,\
			\"waterMeter\": 99.99,\
			\"outDoorTemperature\": -23.01,\
			\"sumpPitWaterLevel\": 20.01,\
			\"sumpPitWaterState\": 0,\
			\"powerInterupt\": 0,\
			\"emergencyStop\": 0,\
			\"electricalParameters\": {\
				\"UA\": 380.00,\
				\"UB\": 380.00,\
				\"UC\": 380.00,\
				\"IA\": 60.00,\
				\"IB\": 60.00,\
				\"IC\": 60.00\
			}\
		},\
		\"primaryPipeDeviceData\": {\
			\"regulatingValve\": {\
				\"opening\": 67.01,\
				\"error\": 0,\
				\"controlMethod\": 0\
			},\
			\"electricValve\": {\
				\"opening\": 67.01,\
				\"error\": 0,\
				\"controlMethod\": 0\
			},\
			\"distributedPump\": {\
				\"state\": 1,\
				\"error\": 0,\
				\"frequency\": 47.01,\
				\"current\": 21.23,\
				\"controlMethod\": 1\
			}\
		},\
		\"SecondaryPipeDeviceData\": {\
			\"circulatingPump\": {\
				\"state\": 1,\
				\"error\": 0,\
				\"frequency\": 47.01,\
				\"current\": 21.23,\
				\"controlMethod\": 1\
			},\
			\"feedWaterPump\": {\
				\"state\": 1,\
				\"error\": 0,\
				\"frequency\": 47.01,\
				\"current\": 21.23,\
				\"controlMethod\": 1\
			},\
			\"feedWaterValve\": {\
				\"state\": 0,\
				\"error\": 0,\
				\"controlMethod\": 0\
			},\
			\"releaseValve\": {\
				\"state\": 0,\
				\"error\": 0,\
				\"controlMethod\": 1\
			}\
		},\
		\"otherDeviceData\": {\
			\"submergsiblePump\": {\
				\"state\": 0,\
				\"error\": 0,\
				\"controlMethod\": 1\
			},\
			\"waterTankValve\": {\
				\"state\": 0,\
				\"error\": 0,\
				\"controlMethod\": 1\
			},\
			\"alarmOut\": {\
				\"state\": 0,\
				\"controlMethod\": 1\
			}\
		}\
	}\
}"

#define pDevToCloudReplySingleContent  		"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"QueryDataReturn\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"isSuccess\": 0,\
		\"primaryPipeMeasureData\": {\
			\"supplyWaterTemperature\": 60.01\
		}\
	}\
}"

#define pDevToCloudReplyLocationContent  	"test"

#define pDevToCloudReplyAllContent  		"test"

#define pDevToCloudReplyAllDistContent  	"test"

#define pDevToCloudAlarmThresholdContent 	"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"Alarm\",\
	\"time\": \"2019-08-12 20:04:59\",\
	\"area\": \"xxx换热站2号换热机组\",\
	\"Message\": {\
		\"primaryPipeMeasureData\": {\
			\"supplyWaterTemperature\": {\
				\"alarmType\": \"ThresholdAlarm\",\
				\"alarmMessage\": \"LoLo\",\
				\"currentValue\": 15.12,\
				\"setLimit\": 20.12\
			}\
		}\
	}\
}"

#define pDevToCloudAlarmStateContent  		"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"Alarm\",\
	\"time\": \"2019-08-12 20:04:59\",\
	\"area\": \"xxx换热站2号换热机组\",\
	\"Message\": {\
		\"otherMeasureData\": {\
			\"powerInterupt\": {\
				\"alarmType\": \"StateAlarm\",\
				\"alarmMessage\": \"powerInterupt\"\
			}\
		}\
	}\
}"

#define pDevToCloudAlarmDeviceContent 		"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"Alarm\",\
	\"time\": \"2019-08-12 20:04:59\",\
	\"area\": \"xxx换热站2号换热机组\",\
	\"Message\": {\
		\"primaryPipeDeviceData\": {\
			\"distributedPump\": {\
				\"alarmType\": \"DeviceAlarm\",\
				\"alarmMessage\": \"distributedPumpError\",\
				\"currentValue\": {\
					\"state\": 1,\
					\"error\": 0,\
					\"frequency\": 47.01,\
					\"current\": 21.23,\
					\"controlMethod\": 1\
				}\
			}\
		}\
	}\
}"


#define pDevToCloudReplyResponseContent  	"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"ControlCommandReturn\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"isSuccess\": 0,\
		\"primaryPipeDeviceData\": {\
			\"regulatingValve\": {\
				\"opening\": 67.01,\
				\"error\": 0,\
				\"controlMethod\": 0\
			}\
		}\
	},\
	\"SecondaryPipeDeviceData\": {\
		\"circulatingPump\": {\
			\"state\": 0,\
			\"error\": 1,\
			\"frequency\": 0.00,\
			\"current\": 0.00,\
			\"controlMethod\": 0\
		}\
	}\
}"

#define pDevToCloudReplySetTableContent  	"{\
	\"DeviceID\": \"12345678987654321\",\
	\"DeviceIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"UpdateCurveReturn\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"isSuccess\": 0\
	}\
}"

#define pDevToCloudReplyGetTableContent  	"{\
	\"DeviceID\": \"12345678987654321\",\
	\"serverIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"UpdateOutdoorTemperatureReturn\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"isSuccess\": 0\
	}\
}"

#define pDevToCloudReplySetTempContent  	"{\
	\"DeviceID\": \"12345678987654321\",\
	\"serverIP\": \"109.56.23.44\",\
	\"Network\": \"NB-IOT\",\
	\"protocolVersion\": \"V0.0.1\",\
	\"messageType\": \"UpdateOutdoorTemperatureReturn\",\
	\"time\": \"2019-08-12 18:04:59\",\
	\"area\": \"xxx换热站1号换热机组\",\
	\"Message\": {\
		\"isSuccess\": 0\
	}\
}"


/*-----------------------------------------------------------*/
static modbus_t *ctx = NULL;
static modbus_t *ctx1 = NULL;
static modbus_t *ctx2 = NULL;
static modbus_t *ctx3 = NULL;
static modbus_t *ctxTcp = NULL;

static int localCmdMsgId;
static commonMsg_t localCmdMsg;

static int cloudCmdMsgId;
static commonMsg_t cloudCmdMsg;

static uint8_t modbusRtuNewConf =  0;
static uint8_t modbusRtu1NewConf = 0;
static uint8_t modbusRtu2NewConf = 0;
static uint8_t modbusRtu3NewConf = 0;
static uint8_t modbusTcpNewConf =  0;
static uint8_t modbusRtuTimeOut =  0;
static uint8_t modbusRtu1TimeOut = 0;
static uint8_t modbusRtu2TimeOut = 0;
static uint8_t modbusRtu3TimeOut = 0;
static uint8_t modbusTcpTimeOut =  0;

static int localConfigCnt = 5;
static struct timeval 	timeout={1,0};
static endDeviceConf_t 	*tempConfig;

extern sem_t 				deviceConfSem;
extern modbusConfParam_t 	flyModbusConf;
extern endDeviceConf_t		endDeviceConf;



static speed_t getBaudrate(int baudrate)
{
	switch(baudrate) {
	case 0: return B0;
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;
	case 460800: return B460800;
	case 500000: return B500000;
	case 576000: return B576000;
	case 921600: return B921600;
	case 1000000: return B1000000;
	case 1152000: return B1152000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 2500000: return B2500000;
	case 3000000: return B3000000;
	case 3500000: return B3500000;
	case 4000000: return B4000000;
	default: return -1;
	}
}

static void devMsgDectect(void)
{
	int ret;
	memset(&localCmdMsg,0,sizeof(commonMsg_t));
	ret = msgrcv(localCmdMsgId,&localCmdMsg,sizeof(commonMsg_t)-sizeof(long),0,IPC_NOWAIT);  //
	if(ret < 0)
	{
		printf("errno :%d\n",errno);
	}
	else
	{
		printf("new message rcv.\n");
		while(sem_wait(&deviceConfSem) == -1)
		{
			exit(1);
		}

		if(localCmdMsg.mtype == 1)
		{
			modbusRtuNewConf = 1;
			modbusRtu1NewConf = 1;
			modbusRtu2NewConf = 1;
			modbusRtu3NewConf = 1;
			modbusTcpNewConf = 1;

			localConfigCnt = 5;
		}
		else if(localCmdMsg.mtype == 2)
		{			
			tempConfig = (endDeviceConf_t *)(localCmdMsg.msgtext);
			if(!strcmp((tempConfig->protocalType),"ModbusRtu"))
			{
				if(!strcmp((tempConfig->localPort),"RS485./dev/ttymxc1"))
				{
					modbusRtuNewConf = 2;
				}
				else if(!strcmp((tempConfig->localPort),"RS485./dev/ttymxc2"))
				{
					modbusRtu1NewConf = 2;
				}
				else if(!strcmp((tempConfig->localPort),"RS485./dev/ttymxc3"))
				{
					modbusRtu2NewConf = 2;
				}
				else if(!strcmp((tempConfig->localPort),"RS485./dev/ttymxc4"))
				{
					modbusRtu3NewConf = 2;
				}
			}
			else if(!strcmp((tempConfig->protocalType),"ModbusTcp"))
			{
				modbusTcpNewConf = 2;
			}
		}
	}
}

static int initModbusRtuParam(modbus_t *modbusCtx, char *comName, char *comBaudrate)
{
	modbusCtx = modbus_new_rtu(comName, getBaudrate(atoi(comBaudrate)), 'N', 8, 1);
	if (modbusCtx == NULL) 
	{
        printf("Unable to allocate libmodbus context\n");
        return -1;
    }
    else
    {
		printf("Open Serial Port %s Successed\n",comName);
	}

    return 1;
}

static int initModbusRtuConnect(modbus_t *modbusCtx,int modbusId)
{
	uint32_t oldResponseToSec;
    uint32_t oldResponseToUsec;
    uint32_t newResponseToSec;
    uint32_t newResponseRoUsec;
	modbus_set_error_recovery(modbusCtx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);
	modbus_set_slave(modbusCtx, modbusId);
	modbus_get_response_timeout(modbusCtx, &oldResponseToSec, &oldResponseToUsec);
	if (modbus_connect(modbusCtx) == -1) {
        modbus_free(modbusCtx);
        return -1;
    }
    modbus_get_response_timeout(ctx, &newResponseToSec, &newResponseRoUsec);
    if((oldResponseToSec == newResponseToSec) &&
                oldResponseToUsec == newResponseRoUsec)
    {
    	return -1;
    }

    return 1;
}

static int initModbusTcpConnect(modbus_t *modbusCtx, char *netAddr, int netPort)
{
	uint32_t oldResponseToSec;
    uint32_t oldResponseToUsec;
    uint32_t newResponseToSec;
    uint32_t newResponseRoUsec;

	modbusCtx = modbus_new_tcp(netAddr, netPort);
	if (modbusCtx == NULL) 
	{
        printf("Unable to allocate libmodbus context\n");
        return -1;
    }
    else
    {
		printf("Open Serial Port %s Successed\n",((flyModbusConf.modbusRtuport).com1Param).comName);
	}
	modbus_set_error_recovery(modbusCtx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);
	modbus_get_response_timeout(modbusCtx, &oldResponseToSec, &oldResponseToUsec);
	if (modbus_connect(modbusCtx) == -1) {
        printf("Connection failed: %s\n", netAddr);
        modbus_free(modbusCtx);
        return -1;
    }
    modbus_get_response_timeout(modbusCtx, &newResponseToSec, &newResponseRoUsec);
    if((oldResponseToSec == newResponseToSec) &&
                oldResponseToUsec == newResponseRoUsec)
    {
    	return -1;
    }

    return 1;
}


static int modbusRtuReadData(modbus_t *ctxIndex,dataUnit_t dataUnit,cJSON *jsonData,char *jsonItem)
{
	int result = -1;

	int curLen = 0;
	int curAddr = 0;
	int curCode = 0;
	int curScale = 0;

	if(!strcmp(dataUnit.RegisterType,"CoilStatus"))
	{
		curCode = 1;
	}
	else if(!strcmp(dataUnit.RegisterType,"InputStatus"))
	{
		curCode = 2;
	}
	else if(!strcmp(dataUnit.RegisterType,"HoldingRegister"))
	{
		curCode = 3;
	}
	else if(!strcmp(dataUnit.RegisterType,"InputRegister"))
	{
		curCode = 4;
	}

	curAddr = dataUnit.Address;
	curLen = dataUnit.Lenth;

	unsigned char buffer[16];
	unsigned short int  buffer1[16];
	curScale = dataUnit.Scale;
	switch(curCode)
	{
		case 1:
			result = modbus_read_bits(ctxIndex, curAddr, curLen, buffer);
			break;
		case 2:
			result = modbus_read_input_bits(ctxIndex, curAddr,  curLen, buffer);
			break;
		case 3:
			result = modbus_read_registers(ctxIndex, curAddr, curLen, buffer1);
			break;
		case 4:
			result = modbus_read_input_registers(ctxIndex, curAddr,  curLen, buffer1);
			break;
		default:
			break;
	}

	if(!strcmp(dataUnit.DataType,"Uint16"))
	{
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber(*((unsigned short int *)buffer1)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Int16"))
	{
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber(*((short int *)buffer1)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Uint32"))
	{
		uint32_t *temp1Data = NULL;
		temp1Data = (uint32_t *)buffer1;
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber((*temp1Data)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Int32"))
	{
		int32_t *temp2Data = NULL;
		temp2Data = (int32_t *)buffer1;
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber((*temp2Data)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Byte"))
	{
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber((*buffer)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Float32"))
	{
		float *temp3Data = NULL;
		temp3Data = (float *)buffer1;
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber((*temp3Data)/curScale));
	}
	else if(!strcmp(dataUnit.DataType,"Float64"))
	{
		;
	}
	else if(!strcmp(dataUnit.DataType,"Bit"))
	{
		cJSON_ReplaceItemInObject(jsonData,jsonItem,cJSON_CreateNumber(((*buffer))/curScale));
	}

	return result;
}


static void deviceStateUpdate(modbus_t *modbusCtx,endDeviceConf_t *devNewConfig)
{
	if(devNewConfig)
	{		
		cJSON *devToCloudUpdateContent = NULL;
		cJSON *itemDevToCloudUpdateContent = NULL;
		cJSON *itemDevToCloudUpdateContent1 = NULL;
		cJSON *itemDevToCloudUpdateContent2 = NULL;
		devToCloudUpdateContent = cJSON_Parse(pdevToCloudUpdateContent);
		if(!devToCloudUpdateContent)
		{
			return ;
		}

		itemDevToCloudUpdateContent = cJSON_GetObjectItem(devToCloudUpdateContent, "Message");
		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "primaryPipeMeasureData");
		//itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorSupplyWaterTemp).supplyWaterTemperature,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorBackWaterTemp).backWaterTemperature,itemDevToCloudUpdateContent1,"backWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorSupplyWaterPressure).supplyWaterPressure,itemDevToCloudUpdateContent1,"supplyWaterPressure");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorBackWaterPressure).backWaterPressure,itemDevToCloudUpdateContent1,"backWaterPressure");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorFlowMeter).flowRate,itemDevToCloudUpdateContent1,"flowRate");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorFlowMeter).totalFlow,itemDevToCloudUpdateContent1,"totalFlow");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorFlowMeter).instantaneousHeat,itemDevToCloudUpdateContent1,"instantaneousHeat");
		modbusRtuReadData(modbusCtx,(devNewConfig->primarySensorFlowMeter).totalHeat,itemDevToCloudUpdateContent1,"totalHeat");

		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "primaryPipeDeviceData");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "distributedPump");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).setFrequency,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).frequency,itemDevToCloudUpdateContent2,"frequency");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).current,itemDevToCloudUpdateContent2,"current");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorDistPump).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "regulatingValve");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorRegularValve).opening,itemDevToCloudUpdateContent1,"opening");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorRegularValve).setOpening,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorRegularValve).error,itemDevToCloudUpdateContent1,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorRegularValve).controlMethod,itemDevToCloudUpdateContent1,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "electricValve");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).open,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).close,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		//modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).operateTime,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).opening,itemDevToCloudUpdateContent2,"opening");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->primaryOperatorElectricValve).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");

		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "secondaryPipeMeasureData");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorSupplyWaterTemp).supplyWaterTemperature,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorBackWaterTemp).backWaterTemperature,itemDevToCloudUpdateContent1,"backWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorSupplyWaterPressure).supplyWaterPressure,itemDevToCloudUpdateContent1,"supplyWaterPressure");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorBackWaterPressure).backWaterPressure,itemDevToCloudUpdateContent1,"backWaterPressure");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorFlowMeter).flowRate,itemDevToCloudUpdateContent1,"flowRate");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorFlowMeter).totalFlow,itemDevToCloudUpdateContent1,"totalFlow");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorFlowMeter).instantaneousHeat,itemDevToCloudUpdateContent1,"instantaneousHeat");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondarySensorFlowMeter).totalHeat,itemDevToCloudUpdateContent1,"totalHeat");

		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "SecondaryPipeDeviceData");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "circulatingPump");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).setFrequency,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).frequency,itemDevToCloudUpdateContent2,"frequency");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).current,itemDevToCloudUpdateContent2,"current");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorCirculatPump).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "feedWaterPump");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).setFrequency,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).frequency,itemDevToCloudUpdateContent2,"frequency");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).current,itemDevToCloudUpdateContent2,"current");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterPump).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "feedWaterValve");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterValve).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterValve).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorFeedWaterValve).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "releaseValve");
		//modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorReleaseValve).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorReleaseValve).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->secondaryOperatorReleaseValve).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");

		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "otherMeasureData");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorWaterTankLevel).waterTankLevel,itemDevToCloudUpdateContent1,"waterTankLevel");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorWaterMeter).waterMeter,itemDevToCloudUpdateContent1,"waterMeter");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorOutTemp).outDoorTemperature,itemDevToCloudUpdateContent1,"outDoorTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorSumpPitWaterLevel).sumpPitWaterLevel,itemDevToCloudUpdateContent1,"sumpPitWaterLevel");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorSumpPitWaterLevel).sumpPitWaterState,itemDevToCloudUpdateContent1,"sumpPitWaterState");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorPowerInt).powerInterupt,itemDevToCloudUpdateContent1,"powerInterupt");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorEmergencyStop).emergencyStop,itemDevToCloudUpdateContent1,"emergencyStop");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "electricalParameters");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).UA,itemDevToCloudUpdateContent2,"UA");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).UB,itemDevToCloudUpdateContent2,"UB");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).UC,itemDevToCloudUpdateContent2,"UC");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).IA,itemDevToCloudUpdateContent2,"IA");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).IB,itemDevToCloudUpdateContent2,"IB");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherSensorElectricParam).IC,itemDevToCloudUpdateContent2,"IC");

		itemDevToCloudUpdateContent1 = cJSON_GetObjectItem(itemDevToCloudUpdateContent, "otherDeviceData");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "submergsiblePump");
		//modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorSubPump).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorSubPump).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorSubPump).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorSubPump).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");
		itemDevToCloudUpdateContent2 = cJSON_GetObjectItem(itemDevToCloudUpdateContent1, "waterTankValve");
		//modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorWaterValve).startstop,itemDevToCloudUpdateContent1,"supplyWaterTemperature");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorWaterValve).state,itemDevToCloudUpdateContent2,"state");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorWaterValve).error,itemDevToCloudUpdateContent2,"error");
		modbusRtuReadData(modbusCtx,(devNewConfig->otherOperatorWaterValve).controlMethod,itemDevToCloudUpdateContent2,"controlMethod");

		char *tempString = (char *)cJSON_PrintUnformatted(devToCloudUpdateContent);
		int tempStringLen = strlen(tempString);
		memset(&cloudCmdMsg,0,sizeof(commonMsg_t));
		cloudCmdMsg.mtype = 2;
		memcpy(cloudCmdMsg.msgtext,tempString,tempStringLen);
		if((msgsnd(cloudCmdMsgId,(void *)&cloudCmdMsg,tempStringLen/*MSG_MAX_SIZE*/,0)) == -1)
		{
			exit(1);
		}

		cJSON_free(tempString);
		cJSON_free(devToCloudUpdateContent);
	}
}

static void* modbusRtuThreadFunc(void* arg)
{
	//default operation,init the modbus
	//memset(&modbusRtu,0,sizeof(modbusRtuport_t));
	memcpy(((flyModbusConf.modbusRtuport).com1Param).comName, "/dev/ttymxc1", strlen("/dev/ttymxc1"));
	memcpy(((flyModbusConf.modbusRtuport).com1Param).comBaudrate , "9600", strlen("9600"));
	initModbusRtuParam(ctx,((flyModbusConf.modbusRtuport).com1Param).comName,((flyModbusConf.modbusRtuport).com1Param).comBaudrate);

	while (1) 
	{
		if(modbusRtuNewConf == 1)
		{
			localConfigCnt--;
			modbusRtuNewConf = 0;
			//if port has been opened,closed port first,then init the port with new parameters
			if(ctx)
			{
			    modbus_close(ctx);
			    modbus_free(ctx);
			}

			initModbusRtuParam(ctx,((flyModbusConf.modbusRtuport).com1Param).comName, ((flyModbusConf.modbusRtuport).com1Param).comBaudrate);
		}
		else if(modbusRtuNewConf == 2)
		{
			modbusRtuNewConf = 0;

			if(ctx)
			{
				initModbusRtuConnect(ctx,tempConfig->slaveId);
				deviceStateUpdate(ctx,tempConfig);
			}			

			//post semphore
			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}

		if(modbusRtuTimeOut)
		{
			modbusRtuTimeOut = 0;
			deviceStateUpdate(ctx,tempConfig);
			printf("modbusRtuThreadFunc timeout\n");
		}
		else
		{
			sleep(2);
		}
	}

	pthread_exit(0);

}

static void* modbusRtu1ThreadFunc(void* arg)
{
	//com2
	memcpy(((flyModbusConf.modbusRtuport).com2Param).comName, "/dev/ttymxc2", strlen("/dev/ttymxc2"));
	memcpy(((flyModbusConf.modbusRtuport).com2Param).comBaudrate , "9600", strlen("9600"));
	initModbusRtuParam(ctx1,((flyModbusConf.modbusRtuport).com2Param).comName,((flyModbusConf.modbusRtuport).com2Param).comBaudrate);

	while(1) 
	{
		if(modbusRtu1NewConf == 1)
		{
			localConfigCnt--;
			modbusRtu1NewConf = 0;
			//if port has been opened,closed port first,then init the port with new parameters
			if(ctx1)
			{
			    modbus_close(ctx1);
			    modbus_free(ctx1);
			}

			initModbusRtuParam(ctx1,((flyModbusConf.modbusRtuport).com2Param).comName, ((flyModbusConf.modbusRtuport).com2Param).comBaudrate);
		}
		else if(modbusRtu1NewConf == 2)
		{
			modbusRtu1NewConf = 0;

			if(ctx1)
			{
				initModbusRtuConnect(ctx1,tempConfig->slaveId);
				deviceStateUpdate(ctx1,tempConfig);
			}			

			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}

		if(modbusRtu1TimeOut)
		{
			modbusRtu1TimeOut = 0;
			printf("modbusRtu1ThreadFunc timeout\n");
		}
		else
		{
			sleep(2);
		}
	}

	pthread_exit(0);
}

static void* modbusRtu2ThreadFunc(void* arg)
{
	//com2
	memcpy(((flyModbusConf.modbusRtuport).com3Param).comName, "/dev/ttymxc3", strlen("/dev/ttymxc3"));
	memcpy(((flyModbusConf.modbusRtuport).com3Param).comBaudrate , "9600", strlen("9600"));
	initModbusRtuParam(ctx2,((flyModbusConf.modbusRtuport).com3Param).comName,((flyModbusConf.modbusRtuport).com3Param).comBaudrate);
	while(1) 
	{
		if(modbusRtu2NewConf == 1)
		{
			localConfigCnt--;
			modbusRtuNewConf = 0;
			//if port has been opened,closed port first,then init the port with new parameters
			if(ctx2)
			{
			    modbus_close(ctx2);
			    modbus_free(ctx2);
			}

			initModbusRtuParam(ctx2,((flyModbusConf.modbusRtuport).com3Param).comName,((flyModbusConf.modbusRtuport).com3Param).comBaudrate);
		}
		else if(modbusRtu2NewConf == 2)
		{
			modbusRtu2NewConf = 0;

			if(ctx2)
			{
				initModbusRtuConnect(ctx2,tempConfig->slaveId);
				deviceStateUpdate(ctx2,tempConfig);
			}			

			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}

		if(modbusRtu2TimeOut)
		{
			modbusRtu2TimeOut = 0;
			printf("modbusRtu2ThreadFunc timeout\n");
		}
		else
		{
			sleep(2);
		}
	}

	pthread_exit(0);
}

static void* modbusRtu3ThreadFunc(void* arg)
{
	while(1) 
	{	
		if(modbusRtu3NewConf == 1)
		{
			localConfigCnt--;
		}
		else if(modbusRtu3NewConf == 2)
		{
			modbusRtu3NewConf = 0;

			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}
		
		sleep(2);
	}

	pthread_exit(0);
}


static void* modbusTcpThreadFunc(void* arg)
{
	while(1) 
	{
		if(modbusTcpNewConf == 1)
		{
			localConfigCnt--;
			modbusTcpNewConf = 0;
		}
		else if(modbusTcpNewConf == 2)
		{
			modbusTcpNewConf = 0;

			if(ctxTcp)
			{
			    modbus_close(ctxTcp);
			    modbus_free(ctxTcp);
			}
			initModbusTcpConnect(ctxTcp,tempConfig->slaveIp,tempConfig->slavePort);
			deviceStateUpdate(ctxTcp,tempConfig);

			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}


		if(modbusTcpTimeOut)
		{
			modbusTcpTimeOut = 0;
			printf("modbusTcpThreadFunc timeout\n");
		}
		else
		{
			sleep(2);
		}

	}

	pthread_exit(0);
}


static void modbusTcpTimer(struct timeval time)
{
	select(0,NULL,NULL,NULL,&time);
	return ;
}

static void* modbusTimerThreadFunc(void* arg)
{
	printf("modbusTimerThreadFunc success\n");
	while(1) 
	{	
		devMsgDectect();
		modbusTcpTimer(timeout);
		modbusRtuTimeOut = 1;
		modbusRtu1TimeOut = 1;
		modbusRtu2TimeOut = 1;
		modbusRtu3TimeOut = 1;
		modbusTcpTimeOut = 1;

		if(localConfigCnt == 0)
		{	
			localConfigCnt = 0xFF;
			if(sem_post(&deviceConfSem) == -1)
			{
				exit(1);
			}
		}
	}

	pthread_exit(0);
}


int deviceInit(void)
{	
	int ret=0;

	localCmdMsgId = msgget(MSG_LOCAL_KEY,0666 | IPC_CREAT);
	if(localCmdMsgId < 0)
	{ 
	  	return -1;
	}

	cloudCmdMsgId = msgget(MSG_CLOUD_KEY,0666 | IPC_CREAT);
	if(cloudCmdMsgId < 0)
	{ 
	  	return -1;
	}	
	
	pthread_t modbusRtuThreadId, modbusRtu1ThreadId, modbusRtu2ThreadId, modbusRtu3ThreadId, modbusTcpThreadId;
	pthread_t modbusTimerThreadId;
	memset(&modbusRtuThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusRtuThreadId, NULL, modbusRtuThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	memset(&modbusRtu1ThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusRtu1ThreadId, NULL, modbusRtu1ThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	memset(&modbusRtu2ThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusRtu2ThreadId, NULL, modbusRtu2ThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	memset(&modbusRtu3ThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusRtu3ThreadId, NULL, modbusRtu3ThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	memset(&modbusTcpThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusTcpThreadId, NULL, modbusTcpThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	memset(&modbusTimerThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&modbusTimerThreadId, NULL, modbusTimerThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	return 1;
}