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

#include <pthread.h>
#include <signal.h>

#include "common.h"
#include "mosquitto.h"
#include "config.h"
#include "cloud.h"

/*	macro 	define		*/
#define FLY_MQTT_KEEP_ALIVE	60	

#define DEVICE_TO_CLOUD_UPDATE_TOPIC			"/fly/update"

#define CLOUD_TO_DEVICE_REQUEST_SINGLE_TOPIC	"/fly/request/single"
#define DEVICE_TO_CLOUD_REPLY_SINGLE_TOPIC		"/fly/reply/single"
#define CLOUD_TO_DEVICE_REQUEST_LOCATION_TOPIC	"/fly/request/location"
#define DEVICE_TO_CLOUD_REPLY_LOCATION_TOPIC	"/fly/reply/location"
#define CLOUD_TO_DEVICE_REQUEST_ALL_TOPIC		"/fly/request/all"
#define DEVICE_TO_CLOUD_REPLY_ALL_TOPIC			"/fly/reply/all"
#define CLOUD_TO_DEVICE_REQUEST_ALL_DIST_TOPIC	"/fly/request/alldist"
#define DEVICE_TO_CLOUD_REPLY_ALL__DIST_TOPIC	"/fly/reply/alldist"

#define DEVICE_TO_CLOUD_ALARM_THRESHOLDTOPIC	"/fly/alarm/threshold"
#define DEVICE_TO_CLOUD_ALARM_STATE_TOPIC		"/fly/alarm/state"
#define DEVICE_TO_CLOUD_ALARM_DEVICE_TOPIC		"/fly/alarm/device"
#define CLOUD_TO_DEVICE_REQUEST_ALARM_SINGLE_TOPIC		"/fly/request/alarm/single"
#define DEVICE_TO_CLOUD_REPLY_ALARM_SINGLE_TOPIC		"/fly/reply/alarm/alarm/single"
#define CLOUD_TO_DEVICE_REQUEST_ALARM_LOCATION_TOPIC	"/fly/request/alarm/location"
#define DEVICE_TO_CLOUD_REPLY_ALARM_LOCATION_TOPIC		"/fly/reply/alarm/alarm/location"
#define CLOUD_TO_DEVICE_REQUEST_ALARM_ALL_TOPIC			"/fly/request/alarm/all"
#define DEVICE_TO_CLOUD_REPLY_ALARM_ALL_TOPIC			"/fly/reply/alarm/alarm/all"
#define CLOUD_TO_DEVICE_REQUEST_ALARM_ALL_DIST_TOPIC	"/fly/request/alarm/alldist"
#define DEVICE_TO_CLOUD_REPLY_ALARM_ALL_DIST_TOPIC		"/fly/reply/alarm/alarm/alldist"

#define CLOUD_TO_DEVICE_CONTROL_TOPIC			"/fly/control"
#define DEVICE_TO_CLOUD_RESPONSE_TOPIC			"/fly/response"

#define CLOUD_TO_DEVICE_SET_TABLE_TOPIC			"/fly/set/table"
#define DEVICE_TO_CLOUD_REPLY_SET_TABLE_TOPIC	"/fly/reply/settable"
#define CLOUD_TO_DEVICE_GET_TABLE_TOPIC			"/fly/get/table"
#define DEVICE_TO_CLOUD_REPLY_GET_TABLE_TOPIC	"/fly/reply/gettable"

#define CLOUD_TO_DEVICE_SET_OUT_TEMP_TOPIC		"/fly/set/outtemp"
#define DEVICE_TO_CLOUD_REPLY_SET_TEMP_TOPIC	"/fly/reply/settemp"

/*	local variable		*/
static char mqttConfigState = 0;

static int cloudCmdMsgId;
static commonMsg_t cloudCmdMsg;

static int dev2cloudMsgDectect(void)
{
	int ret;
	memset(&cloudCmdMsg,0,sizeof(commonMsg_t));
	ret = msgrcv(cloudCmdMsgId,&cloudCmdMsg,sizeof(commonMsg_t)-sizeof(long),0,0);
	return ret;
}


static void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) 
{ 
	if(message->payloadlen)
	{ 
		printf("%s %s", (char *)message->topic, (char *)message->payload); 
	}
	else
	{ 
		printf("%s (null)\n", (char *)message->topic); 
	} 

	fflush(stdout); 
} 

static void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result) 
{ 
	if(!result)
	{ 
		/* Subscribe to broker information topics on successful connect. */ 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_REQUEST_SINGLE_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_REQUEST_LOCATION_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_REQUEST_ALL_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_REQUEST_ALL_DIST_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_CONTROL_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_SET_TABLE_TOPIC, 2); 
		mosquitto_subscribe(mosq, NULL, CLOUD_TO_DEVICE_GET_TABLE_TOPIC, 2); 
	}
	else
	{ 
		fprintf(stderr, "Connect failed\n"); 
	} 
} 

static void mqtt_disconnect_callback(struct mosquitto *mosq, void *userdata, int result) 
{
	if(cloudCmdMsg.mtype == 1)
	{
		cloudCmdMsg.mtype = 0;
	}
	else
	{
		while(mosquitto_reconnect_async(mosq))
		{ 
			printf("Unable to reconnect.\n"); 
			sleep(1);
		}

		printf("mqtt reconnect success.\n");
	}
}	

static void mqtt_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) 
{ 
	int i; 
	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]); 
	for(i=1; i<qos_count; i++)
	{ 
		printf(", %d", granted_qos[i]); 
	} 
	printf("\n"); 
} 

static void mqtt_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str) 
{ 
	/* Pring all log messages regardless of level. */ 
	printf("%s\n", str); 
}


static void* mqttThreadFunc(void* arg)
{
	printf("mqttThreadFunc success\n");
	struct mosquitto *mosq = NULL;
	//libmosquitto 库初始化 
	mosquitto_lib_init(); 
	//创建mosquitto客户端 
	mosq = mosquitto_new(NULL,true,NULL); 
	if(!mosq)
	{ 
		printf("create client failed..\n"); 
		mosquitto_lib_cleanup(); 
		return NULL; 
	}

	//设置回调函数，需要时可使用 
	//mosquitto_log_callback_set(mosq, mqtt_log_callback); 
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback); 
	mosquitto_message_callback_set(mosq, mqtt_message_callback); 
	mosquitto_disconnect_callback_set(mosq,mqtt_disconnect_callback);
	//mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);

    while(1)
	{	
		if(dev2cloudMsgDectect())
		{
			if(cloudCmdMsg.mtype == 1)
			{
				mqttConfParam_t *mqttConfParam;
				mqttConfParam = (mqttConfParam_t *)(cloudCmdMsg.msgtext);
				//客户端连接服务器 
				if(mqttConfigState)
				{
					mosquitto_disconnect(mosq);
					mosquitto_loop_stop(mosq, false);
				}
				
				while(mosquitto_connect_async(mosq, mqttConfParam->mqttServerAddr, mqttConfParam->mqttServerPort, FLY_MQTT_KEEP_ALIVE))
				{ 
					printf("Unable to connect.\n"); 
					sleep(1);
				} 


				printf("mqtt connect success.\n");
				int loop = mosquitto_loop_start(mosq);
			    if(loop != MOSQ_ERR_SUCCESS)
			    {
			        printf("mosquitto loop error\n");
			        return NULL;
			    }

			    mqttConfigState = 1;
			}
			else if(cloudCmdMsg.mtype == 2)
			{
				mosquitto_publish(mosq,NULL,DEVICE_TO_CLOUD_UPDATE_TOPIC,sizeof(commonMsg_t) - sizeof(long),cloudCmdMsg.msgtext,0,0);
			}
		}

		sleep(2);
	}

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup(); 

	pthread_exit(0);
}


int cloudInit(void)
{
	int ret=0;
	printf("mqtt init.\n");

	cloudCmdMsgId = msgget(MSG_CLOUD_KEY,0666 | IPC_CREAT);
	if(cloudCmdMsgId < 0)
	{ 
	  	return -1;
	}	

	pthread_t mqttThreadId;
	memset(&mqttThreadId, 0, sizeof(pthread_t));
	ret = pthread_create(&mqttThreadId, NULL, mqttThreadFunc, NULL);
	if(ret != 0)
	{
		printf("%s: %d\n",__func__, (ret));
		return -1;  
	}

	return 1;
}