#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>   
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include "common.h"
#include "yaml.h"
#include "config.h"

//static int newCmdMsgId;
static int sockfd;
static commonMsg_t newCmdMsg;

static int localCmdMsgId;
static commonMsg_t localCmdMsg;

static int cloudCmdMsgId;
static commonMsg_t cloudCmdMsg;

static int endDeviceReadState1 = 0;
static int endDeviceReadState2 = 0;
static int endDeviceReadState3 = 0;

static  pthread_mutex_t devConfMut;

mqttConfParam_t 	flyMqttCong;
modbusConfParam_t 	flyModbusConf;
endDeviceConf_t		endDeviceConf;
device_table_t deviceTable[3][29] = {
	{
		{(char *)"supplyWaterTemperature",(void *)(&(endDeviceConf.primarySensorSupplyWaterTemp)),0,(int)(sizeof(primarySensorSupplyWaterTemp_t))},
		{(char *)"backWaterTemperature",(void *)(&(endDeviceConf.primarySensorBackWaterTemp)),0,(int)(sizeof(primarySensorBackWaterTemp_t))},
		{(char *)"supplyWaterPressure",(void *)(&(endDeviceConf.primarySensorSupplyWaterPressure)),0,(int)(sizeof(primarySensorSupplyWaterPressure_t))},
		{(char *)"backWaterPressure",(void *)&(endDeviceConf.primarySensorBackWaterPressure),0,(int)(sizeof(primarySensorBackWaterPressure_t))},
		{(char *)"flowMeterflowRate",(void *)&(endDeviceConf.primarySensorFlowMeter),0,(int)(sizeof(primarySensorFlowMeter_t))},
		{(char *)"flowMetertotalFlow",(void *)&(endDeviceConf.primarySensorFlowMeter),1,(int)(sizeof(primarySensorFlowMeter_t))},
		{(char *)"flowMeterinstantaneousHeat",(void *)&(endDeviceConf.primarySensorFlowMeter),2,(int)(sizeof(primarySensorFlowMeter_t))},
		{(char *)"flowMetertotalHeat",(void *)&(endDeviceConf.primarySensorFlowMeter),3,(int)(sizeof(primarySensorFlowMeter_t))},
		{(char *)"distributedPumpstart/stop",(void *)&(endDeviceConf.primaryOperatorDistPump),0,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumpsetFrequency",(void *)&(endDeviceConf.primaryOperatorDistPump),1,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumpfrequency",(void *)&(endDeviceConf.primaryOperatorDistPump),2,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumpcurrent",(void *)&(endDeviceConf.primaryOperatorDistPump),3,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumpstate",(void *)&(endDeviceConf.primaryOperatorDistPump),4,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumperror",(void *)&(endDeviceConf.primaryOperatorDistPump),5,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"distributedPumpcontrolMethod",(void *)&(endDeviceConf.primaryOperatorDistPump),6,(int)(sizeof(primaryOperatorDistPump_t))},
		{(char *)"regulatingValveopening",(void *)&(endDeviceConf.primaryOperatorRegularValve),0,(int)(sizeof(primaryOperatorRegularValve_t))},
		{(char *)"regulatingValvesetOpening",(void *)&(endDeviceConf.primaryOperatorRegularValve),1,(int)(sizeof(primaryOperatorRegularValve_t))},
		{(char *)"regulatingValveerror",(void *)&(endDeviceConf.primaryOperatorRegularValve),2,(int)(sizeof(primaryOperatorRegularValve_t))},
		{(char *)"regulatingValvecontrolMethod",(void *)&(endDeviceConf.primaryOperatorRegularValve),3,(int)(sizeof(primaryOperatorRegularValve_t))},
		{(char *)"electricValveopen",(void *)&(endDeviceConf.primaryOperatorElectricValve),0,(int)(sizeof(primaryOperatorElectricValve_t))},
		{(char *)"electricValveclose",(void *)&(endDeviceConf.primaryOperatorElectricValve),1,(int)(sizeof(primaryOperatorElectricValve_t))},
		{(char *)"electricValveoperateTime",(void *)&(endDeviceConf.primaryOperatorElectricValve),2,(int)(sizeof(primaryOperatorElectricValve_t))},
		{(char *)"electricValveopening",(void *)&(endDeviceConf.primaryOperatorElectricValve),3,(int)(sizeof(primaryOperatorElectricValve_t))},
		{(char *)"electricValveerror",(void *)&(endDeviceConf.primaryOperatorElectricValve),4,(int)(sizeof(primaryOperatorElectricValve_t))},
		{(char *)"electricValvecontrolMethod",(void *)&(endDeviceConf.primaryOperatorElectricValve),5,(int)(sizeof(primaryOperatorElectricValve_t))},
		{NULL,NULL,0,0},
	},
	{
		{(char *)"supplyWaterTemperature",(void *)&(endDeviceConf.secondarySensorSupplyWaterTemp),0,(int)(sizeof(secondarySensorSupplyWaterTemp_t))},
		{(char *)"backWaterTemperature",(void *)&(endDeviceConf.secondarySensorBackWaterTemp),0,(int)(sizeof(secondarySensorBackWaterTemp_t))},
		{(char *)"supplyWaterPressure",(void *)&(endDeviceConf.secondarySensorSupplyWaterPressure),0,(int)(sizeof(secondarySensorSupplyWaterPressure_t))},
		{(char *)"backWaterPressure",(void *)&(endDeviceConf.secondarySensorBackWaterPressure),0,(int)(sizeof(secondarySensorBackWaterPressure_t))},
		{(char *)"flowMeterflowRate",(void *)&(endDeviceConf.secondarySensorFlowMeter),0,(int)(sizeof(secondarySensorFlowMeter_t))},
		{(char *)"flowMetertotalFlow",(void *)&(endDeviceConf.secondarySensorFlowMeter),1,(int)(sizeof(secondarySensorFlowMeter_t))},
		{(char *)"flowMeterinstantaneousHeat",(void *)&(endDeviceConf.secondarySensorFlowMeter),2,(int)(sizeof(secondarySensorFlowMeter_t))},
		{(char *)"flowMetertotalHeat",(void *)&(endDeviceConf.secondarySensorFlowMeter),3,(int)(sizeof(secondarySensorFlowMeter_t))},
		{(char *)"circulatingPumpstart/stop",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),0,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumpsetFrequency",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),1,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumpfrequency",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),2,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumpcurrent",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),3,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumpstate",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),4,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumperror",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),5,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"circulatingPumpcontrolMethod",(void *)&(endDeviceConf.secondaryOperatorCirculatPump),6,(int)(sizeof(secondaryOperatorCirculatPump_t))},
		{(char *)"feedWaterPumpstart/stop",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),0,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumpsetFrequency",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),1,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumpfrequency",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),2,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumpcurrent",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),3,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumpstate",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),4,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumperror",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),5,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterPumpcontrolMethod",(void *)&(endDeviceConf.secondaryOperatorFeedWaterPump),6,(int)(sizeof(secondaryOperatorFeedWaterPump_t))},
		{(char *)"feedWaterValveopen/close",(void *)&(endDeviceConf.secondaryOperatorFeedWaterValve),0,(int)(sizeof(secondaryOperatorFeedWaterValve_t))},
		{(char *)"feedWaterValvestate",(void *)&(endDeviceConf.secondaryOperatorFeedWaterValve),1,(int)(sizeof(secondaryOperatorFeedWaterValve_t))},
		{(char *)"feedWaterValvecontrolMethod",(void *)&(endDeviceConf.secondaryOperatorFeedWaterValve),2,(int)(sizeof(secondaryOperatorFeedWaterValve_t))},
		{(char *)"releaseValveopen/close",(void *)&(endDeviceConf.secondaryOperatorReleaseValve),0,(int)(sizeof(secondaryOperatorReleaseValve_t))},
		{(char *)"releaseValvestate",(void *)&(endDeviceConf.secondaryOperatorReleaseValve),1,(int)(sizeof(secondaryOperatorReleaseValve_t))},
		{(char *)"releaseValvecontrolMethod",(void *)&(endDeviceConf.secondaryOperatorReleaseValve),2,(int)(sizeof(secondaryOperatorReleaseValve_t))},
		{NULL,NULL,0,0},
	},
	{
		{(char *)"waterTankLevel",(void *)&(endDeviceConf.otherSensorWaterTankLevel),0,(int)(sizeof(otherSensorWaterTankLevel_t))},
		{(char *)"waterMeter",(void *)&(endDeviceConf.otherSensorWaterMeter),0,(int)(sizeof(otherSensorWaterMeter_t))},
		{(char *)"outDoorTemperature",(void *)&(endDeviceConf.otherSensorOutTemp),0,(int)(sizeof(otherSensorOutTemp_t))},
		{(char *)"sumpPitWaterLevel",(void *)&(endDeviceConf.otherSensorSumpPitWaterLevel),0,(int)(sizeof(otherSensorSumpPitWaterLevel_t))},
		{(char *)"sumpPitWaterState",(void *)&(endDeviceConf.otherSensorSumpPitWaterLevel),1,(int)(sizeof(otherSensorSumpPitWaterLevel_t))},
		{(char *)"powerInterupt",(void *)&(endDeviceConf.otherSensorPowerInt),0,(int)(sizeof(otherSensorPowerInt_t))},
		{(char *)"emergencyStop",(void *)&(endDeviceConf.otherSensorEmergencyStop),0,(int)(sizeof(otherSensorEmergencyStop_t))},
		{(char *)"electricalParametersUA",(void *)&(endDeviceConf.otherSensorElectricParam),0,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"electricalParametersUB",(void *)&(endDeviceConf.otherSensorElectricParam),1,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"electricalParametersUC",(void *)&(endDeviceConf.otherSensorElectricParam),2,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"electricalParametersIA",(void *)&(endDeviceConf.otherSensorElectricParam),3,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"electricalParametersIB",(void *)&(endDeviceConf.otherSensorElectricParam),4,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"electricalParametersIC",(void *)&(endDeviceConf.otherSensorElectricParam),5,(int)(sizeof(otherSensorElectricParam_t))},
		{(char *)"submergsiblePumpstart/stop",(void *)&(endDeviceConf.otherOperatorSubPump),0,(int)(sizeof(otherOperatorSubPump_t))},
		{(char *)"submergsiblePumpstate",(void *)&(endDeviceConf.otherOperatorSubPump),1,(int)(sizeof(otherOperatorSubPump_t))},
		{(char *)"submergsiblePumperror",(void *)&(endDeviceConf.otherOperatorSubPump),2,(int)(sizeof(otherOperatorSubPump_t))},
		{(char *)"submergsiblePumpcontrolMethod",(void *)&(endDeviceConf.otherOperatorSubPump),3,(int)(sizeof(otherOperatorSubPump_t))},
		{(char *)"waterTankValveopen/close",(void *)&(endDeviceConf.otherOperatorWaterValve),0,(int)(sizeof(otherOperatorWaterValve_t))},
		{(char *)"waterTankValvestate",(void *)&(endDeviceConf.otherOperatorWaterValve),1,(int)(sizeof(otherOperatorWaterValve_t))},
		{(char *)"waterTankValveerror",(void *)&(endDeviceConf.otherOperatorWaterValve),2,(int)(sizeof(otherOperatorWaterValve_t))},
		{(char *)"waterTankValvecontrolMethod",(void *)&(endDeviceConf.otherOperatorWaterValve),3,(int)(sizeof(otherOperatorWaterValve_t))},
		{NULL,NULL,0,0},
	}
};


extern sem_t 		deviceConfSem;

/*----------------------------implemention---------------------------------------*/

static int app2DevConfMsgDectect(void)
{
	int ret;
	memset(&newCmdMsg,0,sizeof(commonMsg_t));
	ret = recv(sockfd,&newCmdMsg,sizeof(commonMsg_t),MSG_WAITALL);
	if(ret<0)
	{
		printf("recv failed");
		return 0;
	}
	else
	{
		return ret;
	}
}

static int configForMqttUpdate(char *anchor,char *value)
{
	if(!strcmp(anchor,"MQTT_TIME_INTERVAL"))
	{
		flyMqttCong.mqttInternal = atoi(value);
	}
	else if(!strcmp(anchor,"MQTT_PORT"))
	{
		flyMqttCong.mqttServerPort = atoi(value);
	}
	else if(!strcmp(anchor,"MQTT_SERVER"))
	{
		memcpy(flyMqttCong.mqttServerAddr,value,strlen(value));
	}

	return 1;
}

static dataUnit_t DataTempUnit;
static dataUnit_t *pDataTempUnit = NULL;
static void parseConfigParam(int curState1,int curState2,char *anchor,char *value)
{
	if(!anchor)
	{
		return;
	}

	if(!strcmp(anchor,"RegisterType"))
	{
		memcpy(DataTempUnit.RegisterType,value,strlen(value));
	}
	else if(!strcmp(anchor,"Address"))
	{
		DataTempUnit.Address = atoi(value);
	}
	else if(!strcmp(anchor,"Lenth"))
	{
		DataTempUnit.Lenth = atoi(value);
	}
	else if(!strcmp(anchor,"DataType"))
	{
		memcpy(DataTempUnit.DataType,value,strlen(value));
	}
	else if(!strcmp(anchor,"Scale"))
	{
		DataTempUnit.Scale = atoi(value);

		if(pDataTempUnit)
		{
			memcpy(pDataTempUnit,&DataTempUnit,sizeof(dataUnit_t));
			/*printf("%s,  %d   ,%d,   %s    %d\n",((endDeviceConf.primarySensorSupplyWaterTemp).supplyWaterTemperature).RegisterType, \
												((endDeviceConf.primarySensorSupplyWaterTemp).supplyWaterTemperature).Address, \
												((endDeviceConf.primarySensorSupplyWaterTemp).supplyWaterTemperature).Lenth,\
												((endDeviceConf.primarySensorSupplyWaterTemp).supplyWaterTemperature).DataType,\
												((endDeviceConf.primarySensorSupplyWaterTemp).supplyWaterTemperature).Scale);
												*/
			pDataTempUnit = NULL;
		}
	}
}

static int configForEndDeviceUpdate(char *anchor, char *value)
{
	int confKeyState = 0;

	FILE *fd = NULL;
	yaml_parser_t parser;
	yaml_token_t  token;

	char tempAliasAnchor[32];
	char tempScalarValue[32];

	fd = fopen(value, "r");
	if(fd != NULL)
	{
		//receive new configuration,clear all parameters
		memset(&endDeviceConf,0,sizeof(endDeviceConf_t));

		//parse new configuration
		if(yaml_parser_initialize(&parser))
		{
			yaml_parser_set_input_file(&parser, fd);

			do 
			{
				yaml_parser_scan(&parser, &token);
			    switch(token.type)
			    {
			    /* Stream start/end */
			    case YAML_STREAM_START_TOKEN: 
			    	break;
			    case YAML_STREAM_END_TOKEN: 
			    	break;
			    /* Block delimeters */
			    case YAML_BLOCK_SEQUENCE_START_TOKEN:  
			    	break;
			    case YAML_BLOCK_ENTRY_TOKEN:  
			    	break;
			    case YAML_BLOCK_END_TOKEN:  
			    	break;

			    /* Token types (read before actual token) */
			    case YAML_KEY_TOKEN:   
			    	confKeyState = 0; 
			    	break;
			    case YAML_VALUE_TOKEN: 
			    	confKeyState = 1; 
			    	break;
			    
			    /* Data */
			    case YAML_BLOCK_MAPPING_START_TOKEN: 
			    	break;
			    case YAML_SCALAR_TOKEN:
			    	if(confKeyState == 0)
			    	{			    		
			    		memset(tempAliasAnchor,0,sizeof(tempAliasAnchor)/sizeof(char));
						memcpy(tempAliasAnchor,(char *)token.data.scalar.value,strlen((char *)(token.data.scalar.value)));

						if(!strcmp(tempAliasAnchor,"ProtocolMap") || \
			    				!strcmp(tempAliasAnchor,"Definition") || \
			    				!strcmp(tempAliasAnchor,"RegisterType") || \
			    				!strcmp(tempAliasAnchor,"Address") || \
			    				!strcmp(tempAliasAnchor,"Lenth") || \
			    				!strcmp(tempAliasAnchor,"DataType") || \
			    				!strcmp(tempAliasAnchor,"Scale") || \
			    				!strcmp(tempAliasAnchor,"Area") || \
			    				!strcmp(tempAliasAnchor,"Protocal") || \
			    				!strcmp(tempAliasAnchor,"SlaveID") || \
			    				!strcmp(tempAliasAnchor,"LocalPort") || \
			    				!strcmp(tempAliasAnchor,"SlaveIP") || \
			    				!strcmp(tempAliasAnchor,"SlavePort") || \
			    				!strcmp(tempAliasAnchor,"Sensor") || \
			    				!strcmp(tempAliasAnchor,"Operator") || \
			    				!strcmp(tempAliasAnchor,"Device") || \
			    				!strcmp(tempAliasAnchor,"DEVICE"))
			    		{
			    			;
			    		}
			    		else if(!strcmp(tempAliasAnchor,"PrimaryPipeNet"))
			    		{
			    			endDeviceReadState1 = 1;
			    		}
			    		else if(!strcmp(tempAliasAnchor,"SecondaryPipeNet"))
			    		{
			    			endDeviceReadState1 = 2;
			    		}
			    		else if(!strcmp(tempAliasAnchor,"Other"))
			    		{
			    			endDeviceReadState1 = 3;
			    		}			    		
			    		else
			    		{
			    			int i = 0; 
							while((deviceTable[endDeviceReadState1-1][i].pKey) && (endDeviceReadState1>=1))
							{
								if(!strcmp(deviceTable[endDeviceReadState1-1][i].pKey, tempAliasAnchor))
								{
									printf("yyyyyyyyyyyyyyyyy:%s\n",tempAliasAnchor);
									if((deviceTable[endDeviceReadState1-1][i].pDevice))
									{
										dataUnit_t *pTemp = (deviceTable[endDeviceReadState1-1][i].pDevice);
										pDataTempUnit = pTemp+deviceTable[endDeviceReadState1-1][i].item;
										memset(&DataTempUnit,0,sizeof(dataUnit_t));
									}

									break;
								}

								i++;
							}
			    		}

			    		printf("xxxxxxxxxxxxxxxxxxxxx %s,%d,%d\n",token.data.scalar.value,endDeviceReadState1,endDeviceReadState2);
			    	}
			    	else
			    	{
			    		memset(tempScalarValue,0,sizeof(tempScalarValue)/sizeof(char));
			    		memcpy(tempScalarValue,(char *)token.data.scalar.value,strlen((char *)(token.data.scalar.value)));
			    		if(!strcmp(tempAliasAnchor,"Device"))
			    		{
			    			;
			    		}
			    		else if(!strcmp(tempAliasAnchor,"Area"))
			    		{
			    			memcpy(endDeviceConf.area,tempScalarValue,strlen(tempScalarValue));
			    		}
			    		else if(!strcmp(tempAliasAnchor,"Protocal"))
			    		{
			    			memcpy(endDeviceConf.protocalType,tempScalarValue,strlen(tempScalarValue));
			    		}
			    		else if(!strcmp(tempAliasAnchor,"SlaveID"))
			    		{
			    			endDeviceConf.slaveId = atoi(tempScalarValue);
			    		}
			    		else if(!strcmp(tempAliasAnchor,"LocalPort"))
			    		{
			    			memcpy(endDeviceConf.localPort,tempScalarValue,strlen(tempScalarValue));
			    		}
			    		else if(!strcmp(tempAliasAnchor,"SlaveIP"))
			    		{
			    			memcpy(endDeviceConf.slaveIp,tempScalarValue,strlen(tempScalarValue));
			    		}
			    		else if(!strcmp(tempAliasAnchor,"SlavePort"))
			    		{
			    			endDeviceConf.slavePort = atoi(tempScalarValue);
			    		}
			    		else
			    		{
			    			printf("state1:%d,state2:%d,anchor:%s,value:%s\n",endDeviceReadState1,endDeviceReadState2,tempAliasAnchor,tempScalarValue);
			    			parseConfigParam(endDeviceReadState1,endDeviceReadState2,tempAliasAnchor,tempScalarValue);
			    		}
			    	}

			    	break;
			    default:
			     	printf("Got token of type %d\n", token.type);
			      	break;
			    }

			    if(token.type != YAML_STREAM_END_TOKEN)
			    {
			      	yaml_token_delete(&token);
			    }
			} while(token.type != YAML_STREAM_END_TOKEN);
			yaml_token_delete(&token);

			/* Cleanup */
			yaml_parser_delete(&parser);
			fclose(fd);
		}
	}

	return 1;
}


static int configForLocalUpdate(int devType, char *anchor, char *value)
{
	comportParam_t *tempComportParam;
	netportParam_t *tempNetportParam;

	if((anchor != NULL) && (value != NULL))
	{
		return -1;
	}

	if(devType == 1)
	{
		tempComportParam = &((flyModbusConf.modbusRtuport).com1Param);
	}
	else if(devType == 2)
	{
		tempComportParam = &((flyModbusConf.modbusRtuport).com2Param);
	}
	else if(devType == 3)
	{
		tempComportParam = &((flyModbusConf.modbusRtuport).com3Param);
	}
	else if(devType == 4)
	{
		tempComportParam = &((flyModbusConf.modbusRtuport).com4Param);
	}
	else if(devType == 5)
	{
		tempNetportParam = &((flyModbusConf.modbusTcpport).net0Param);
	}
	else if(devType == 6)
	{
		tempNetportParam = &((flyModbusConf.modbusTcpport).net1Param);
	}

	if(devType<=4)
	{
		if(!strcmp(anchor,"BaudRate"))
		{
			memcpy(tempComportParam->comBaudrate,value,strlen(value));
		}
		else if(!strcmp(anchor,"comWordLen"))
		{
			tempComportParam->comWordLen = atoi(value);
			//memcpy(&(tempComportParam->comWordLen),value,strlen((const char *)value));
		}
		else if(!strcmp(anchor,"Parity"))
		{
			memcpy(tempComportParam->comParity,value,strlen(value));
		}
		else if(!strcmp(anchor,"StopBits"))
		{
			tempComportParam->comStopBit = atoi(value);
			//memcpy(&(tempComportParam->comStopBit),value,strlen((const char *)value));
		}

		//other parameters is not needed
		//to do
	}
	else
	{
		if(!strcmp(anchor,"address"))
		{
			memcpy(tempNetportParam->netAddr,value,strlen(value));
		}
		else if(!strcmp(anchor,"netmask"))
		{
			memcpy(tempNetportParam->netMask,value,strlen(value));
		}
		else if(!strcmp(anchor,"gateway"))
		{
			memcpy(tempNetportParam->netGateway,value,strlen(value));
		}
		else if(!strcmp(anchor,"broadcast"))
		{
			memcpy(tempNetportParam->netBroadcast,value,strlen(value));
		}
	}

	return 1;
}

static void* configThreadFunc(void* arg)
{
	printf("configThreadFunc success\n");

	int devSeq = 0;
	int keyState = 0;
	char tempAliasAnchor[16];
	char tempScalarValue[16];

	FILE *fd = NULL;
	yaml_parser_t parser;
	//yaml_event_t  event;
	yaml_token_t  token;

	while(1) 
	{	
		if(app2DevConfMsgDectect())
		{	
			//receive the config message is interface configuration
			if(newCmdMsg.mtype == 1)
			{
				fd = fopen("config/public.yml", "r");
				if(fd != NULL)
				{
					if(yaml_parser_initialize(&parser))
					{
						yaml_parser_set_input_file(&parser, fd);

						do 
						{
							yaml_parser_scan(&parser, &token);
						    switch(token.type)
						    {
						    /* Stream start/end */
						    case YAML_STREAM_START_TOKEN: 
						    	break;
						    case YAML_STREAM_END_TOKEN: 
						    	break;
						    /* Block delimeters */
						    case YAML_BLOCK_SEQUENCE_START_TOKEN:  
						    	break;
						    case YAML_BLOCK_ENTRY_TOKEN:  
						    	break;
						    case YAML_BLOCK_END_TOKEN:  
						    	break;

						    /* Token types (read before actual token) */
						    case YAML_KEY_TOKEN:   
						    	keyState = 0; 
						    	break;
						    case YAML_VALUE_TOKEN: 
						    	keyState = 1; 
						    	break;
						    
						    /* Data */
						    case YAML_BLOCK_MAPPING_START_TOKEN: 
						    	break;
						    case YAML_SCALAR_TOKEN:
						    	if(keyState == 0)
						    	{
						    		if(!strcmp((char *)(token.data.scalar.value),"/dev/ttymxc1"))
							    	{
							    		devSeq = 1;
							    		memset(&((flyModbusConf.modbusRtuport).com1Param),0,sizeof(comportParam_t));
							    		memcpy(((flyModbusConf.modbusRtuport).com1Param).comName, "/dev/ttymxc1", strlen("/dev/ttymxc1"));
							    	}
							    	else if(!strcmp((char *)(token.data.scalar.value),"/dev/ttymxc2"))
							    	{
							    		devSeq = 2;
							    		memset(&((flyModbusConf.modbusRtuport).com2Param),0,sizeof(comportParam_t));
							    		memcpy(((flyModbusConf.modbusRtuport).com2Param).comName, "/dev/ttymxc2", strlen("/dev/ttymxc2"));
							    	}
							    	else if(!strcmp((char *)(token.data.scalar.value),"/dev/ttymxc3"))
							    	{
							    		devSeq = 3;
							    		memset(&((flyModbusConf.modbusRtuport).com3Param),0,sizeof(comportParam_t));
							    		memcpy(((flyModbusConf.modbusRtuport).com3Param).comName, "/dev/ttymxc3", strlen("/dev/ttymxc3"));
							    	}
							    	else if(!strcmp((char *)(token.data.scalar.value),"/dev/ttymxc4"))
							    	{
							    		devSeq = 4;
							    		memset(&((flyModbusConf.modbusRtuport).com4Param),0,sizeof(comportParam_t));
							    		memcpy(((flyModbusConf.modbusRtuport).com4Param).comName, "/dev/ttymxc4", strlen("/dev/ttymxc4"));
							    	}
							    	else if(!strcmp((char *)(token.data.scalar.value),"eth0"))
							    	{
							    		devSeq = 5;
							    		memset(&((flyModbusConf.modbusTcpport).net0Param),0,sizeof(netportParam_t));
							    		memcpy(((flyModbusConf.modbusTcpport).net0Param).netName, "eth0", strlen("eth0"));
							    	}
							    	else if(!strcmp((char *)(token.data.scalar.value),"eth1"))
							    	{
							    		devSeq = 6;
							    		memset(&((flyModbusConf.modbusTcpport).net1Param),0,sizeof(netportParam_t));
							    		memcpy(((flyModbusConf.modbusTcpport).net1Param).netName, "eth1", strlen("eth1"));
							    	}
							    	else if((!strcmp((char *)(token.data.scalar.value),"HARDWARE_INTERFACE")) || \
							    				(!strcmp((char *)(token.data.scalar.value),"RS485")) || \
							    				(!strcmp((char *)(token.data.scalar.value),"HardwareFlowControl")) || \
							    				(!strcmp((char *)(token.data.scalar.value),"Ethernet")))
							    	{
							    		;
							    	}
							    	else
							    	{
							    		memset(tempAliasAnchor,0,sizeof(tempAliasAnchor)/sizeof(char));
								    	memcpy(tempAliasAnchor,(char *)token.data.scalar.value,strlen((char *)(token.data.scalar.value)));
							    	}
						    	}
						    	else if(keyState == 1)
						    	{
						    		memset(tempScalarValue,0,sizeof(tempScalarValue)/sizeof(char));
						    		memcpy(tempScalarValue,(char *)token.data.scalar.value,strlen((char *)(token.data.scalar.value)));
						    		configForLocalUpdate(devSeq,tempAliasAnchor,tempScalarValue);
						    	}
						    	break;
						    default:
						     	printf("Got token of type %d\n", token.type);
						      	break;
						    }

						    if(token.type != YAML_STREAM_END_TOKEN)
						    {
						      	yaml_token_delete(&token);
						    }
						} while(token.type != YAML_STREAM_END_TOKEN);
						yaml_token_delete(&token);

						/* Cleanup */
						yaml_parser_delete(&parser);
						fclose(fd);
					}

					//finish interface update
					memset(&localCmdMsg,0,sizeof(commonMsg_t));
					localCmdMsg.mtype = 1;
					memcpy(localCmdMsg.msgtext,&flyModbusConf,sizeof(modbusConfParam_t));
					if((msgsnd(localCmdMsgId,(void *)&localCmdMsg,sizeof(modbusConfParam_t),0)) == -1)
					{
						exit(1);
					}
				}
			}
			//receive the message is end device configuration 
			else if(newCmdMsg.mtype == 2)
			{
				int devBlockStart = 0;
				fd = fopen("config/public.yml", "r");
				if(fd != NULL)
				{
					if(yaml_parser_initialize(&parser))
					{
						yaml_parser_set_input_file(&parser, fd);

						do 
						{
							yaml_parser_scan(&parser, &token);
						    switch(token.type)
						    {
						    /* Stream start/end */
						    case YAML_STREAM_START_TOKEN: 
						    	break;
						    case YAML_STREAM_END_TOKEN: 
						    	break;
						    /* Block delimeters */
						    case YAML_BLOCK_SEQUENCE_START_TOKEN:  
						    	break;
						    case YAML_BLOCK_ENTRY_TOKEN:
						    	devBlockStart = 1;
						    	break;
						    case YAML_BLOCK_END_TOKEN: 
						    	devBlockStart = 0;
						    	break;

						    /* Token types (read before actual token) */
						    case YAML_KEY_TOKEN:   
						    	keyState = 0; 
						    	break;
						    case YAML_VALUE_TOKEN: 
						    	keyState = 1; 
						    	break;
						    
						    /* Data */
						    case YAML_BLOCK_MAPPING_START_TOKEN: 
						    	break;
						    case YAML_SCALAR_TOKEN:
						    	if(devBlockStart == 1)
						    	{
						    		if(keyState == 0)
						    		{
						    			memset(tempAliasAnchor,0,sizeof(tempAliasAnchor)/sizeof(char));
								    	memcpy(tempAliasAnchor,(char *)token.data.scalar.value,strlen((char *)(token.data.scalar.value)));
						    		}
						    		else if(keyState == 1)
							    	{
							    		memset(tempScalarValue,0,sizeof(tempScalarValue)/sizeof(char));
			    						memcpy(tempScalarValue,(char *)(token.data.scalar.value),strlen((char *)(token.data.scalar.value)));
							    		
							    		while(sem_wait(&deviceConfSem) == -1)
							    		{
							    			exit(1);
							    		}

							    		if(configForEndDeviceUpdate(tempAliasAnchor,tempScalarValue))
							    		{
							    			memset(&localCmdMsg,0,sizeof(commonMsg_t));
							    			localCmdMsg.mtype = 2;
				    						memcpy(localCmdMsg.msgtext,&endDeviceConf,sizeof(endDeviceConf_t));
				    						if((msgsnd(localCmdMsgId,(void *)&localCmdMsg,sizeof(endDeviceConf_t),0)) == -1)
				    						{
				    							exit(1);
				    						}
							    		}		    						
			    						
			    						if(sem_post(&deviceConfSem) == -1)
			    						{
			    							exit(1);
			    						}
							    	}
						    	}

						    	break;
						    	
						    	
						    default:
						     	printf("Got token of type %d\n", token.type);
						      	break;
						    }

						    if(token.type != YAML_STREAM_END_TOKEN)
						    {
						      	yaml_token_delete(&token);
						    }
						} while(token.type != YAML_STREAM_END_TOKEN);
						yaml_token_delete(&token);

						/* Cleanup */
						yaml_parser_delete(&parser);
						fclose(fd);
					}
				}
			}
			//receive the message is cloud configuration 
			else if(newCmdMsg.mtype == 3)
			{
				fd = fopen("config/public.yml", "r");
				if(fd != NULL)
				{
					if(yaml_parser_initialize(&parser))
					{
						yaml_parser_set_input_file(&parser, fd);

						do 
						{
							yaml_parser_scan(&parser, &token);
						    switch(token.type)
						    {
						    /* Stream start/end */
						    case YAML_STREAM_START_TOKEN: 
						    	break;
						    case YAML_STREAM_END_TOKEN: 
						    	break;
						    /* Block delimeters */
						    case YAML_BLOCK_SEQUENCE_START_TOKEN:  
						    	break;
						    case YAML_BLOCK_ENTRY_TOKEN:  
						    	break;
						    case YAML_BLOCK_END_TOKEN:  
						    	break;

						    /* Token types (read before actual token) */
						    case YAML_KEY_TOKEN:   
						    	keyState = 0; 
						    	break;
						    case YAML_VALUE_TOKEN: 
						    	keyState = 1; 
						    	break;
						    
						    /* Data */
						    case YAML_BLOCK_MAPPING_START_TOKEN: 
						    	break;
						    case YAML_SCALAR_TOKEN:
						    	if(keyState == 0)
						    	{
						    		if((!strcmp((char *)(token.data.scalar.value),"HARDWARE_INTERFACE")) || \
				    				(!strcmp((char *)(token.data.scalar.value),"DEVICE")) || \
				    				(!strcmp((char *)(token.data.scalar.value),"device")))
							    	{
							    		break;
							    	}
							    	else
							    	{
							    		memset(tempAliasAnchor,0,sizeof(tempAliasAnchor)/sizeof(char));
								    	memcpy(tempAliasAnchor,(char *)(token.data.scalar.value),strlen((char *)(token.data.scalar.value)));
							    		break;
							    	}
						    	}
						    	else if(keyState == 1)
						    	{
						    		memset(tempScalarValue,0,sizeof(tempScalarValue)/sizeof(char));
			    					memcpy(tempScalarValue,(char *)(token.data.scalar.value),strlen((char *)(token.data.scalar.value)));
			    					configForMqttUpdate(tempAliasAnchor,tempScalarValue);
						    		break;
						    	}
						    	
						    default:
						      	break;
						    }

						    if(token.type != YAML_STREAM_END_TOKEN)
						    {
						      	yaml_token_delete(&token);
						    }
						} while(token.type != YAML_STREAM_END_TOKEN);
						yaml_token_delete(&token);

						/* Cleanup */
						yaml_parser_delete(&parser);
						fclose(fd);
					}

					memset(&cloudCmdMsg,0,sizeof(commonMsg_t));
	    			cloudCmdMsg.mtype = 1;
					memcpy(cloudCmdMsg.msgtext,&flyMqttCong,sizeof(mqttConfParam_t));
					if((msgsnd(cloudCmdMsgId,(void *)&cloudCmdMsg,sizeof(mqttConfParam_t),0)) == -1)
					{
						exit(1);
					}
				}
			}			
		}

		sleep(2);
	}

	close(sockfd); 
	pthread_exit(0);
}


void socketClient()
{
    struct sockaddr_in servaddr;
    char *serverAddress = "127.0.0.1";
    int  serverPort = 15666;
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0); 
    }
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port =htons(serverPort); 
    if( inet_pton(AF_INET, serverAddress, &servaddr.sin_addr) <= 0)
    { 
        exit(0); 
    } 
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    { 
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
        exit(0); 
    }
}

int configInit(void)
{
	int ret=0;
	printf("config init.\n");

	pthread_mutex_init(&devConfMut,NULL);
	
	/*
	newCmdMsgId = msgget(MSG_CONFIG_KEY,0666 | IPC_CREAT);
	if(newCmdMsgId < 0)
	{ 
	  	return -1;
	}
	*/

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


	socketClient();

	pthread_t configThreadId;
	memset(&configThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&configThreadId, NULL, configThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	return 1;
}