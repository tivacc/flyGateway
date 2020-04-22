#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct commonMsg_tag  
{
    long mtype;
    char msgtext[2048];  
}commonMsg_t;

// #define MSG_NEW_CONFIG_KEY 			0x11278

// #define MSG_APP2DEV_CONFIG_KEY 		0x13236
// #define MSG_DEV2APP_CONFIG_KEY 		0x13236

// #define MSG_APP2CLOUD_KEY 			0x13574
// #define MSG_CLOUD2DEV_KEY 			0x13574

// #define MSG_DEV2CLOUD_KEY			0X14768
// #define MSG_CLOUD2DEV_KEY			0X14768

#define MSG_CONFIG_KEY 			0x11278
#define MSG_LOCAL_KEY 			0x13236
#define MSG_CLOUD_KEY 			0x13574

#define MSG_MAX_SIZE			2048

#endif