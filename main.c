#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>

#include "cloud.h"
#include "config.h"
#include "device.h"

sem_t deviceConfSem;
int main(int argc, char** argv)
{
	printf("enter main\n");
	if(sem_init(&deviceConfSem,0,1) == -1)
	{
		exit(1);
	}
	
    if (fork() == 0)
    {  
        if(execl("/bin/sh","sh","fly4gStart.sh" ,NULL) < 0 )
        {  
            exit(0);  
        }  
    }
    else
    {  
        exit(0);
    }  
	
	configInit();
	deviceInit();
	cloudInit();
	while(1) {}
}