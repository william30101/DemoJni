#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include "MyClient.h"
#include "example.h"

#undef	TCSAFLUSH
#define	TCSAFLUSH	TCSETSF
#ifndef	_TERMIOS_H_
#define	_TERMIOS_H_
#endif

static int fd , nanoFd , driveFd;
static int debugData = true;
struct termios newtio, oldtio;

static const char *classPathName = "com/example/demojni/MainActivity";
#define LOG_TAG "hello"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)


using namespace android;

extern "C"
{

	JNIEXPORT jint JNICALL Native_StartCal(JNIEnv *env,jobject mc)
	{

		//int *str = (char*)env->GetStringUTFChars(data, NULL);

		Circle* cir = new Circle(5);
		cir->area();
		LOGI("radius = %lf area=%lf",cir->radius,cir->area());
		//cir.


		  return 0;
	}

	JNIEXPORT jint JNICALL Native_WriteDemoData(JNIEnv *env,jobject mc, jintArray data, jint size)
	{

		//int *str = (char*)env->GetStringUTFChars(data, NULL);

		jsize len = env->GetArrayLength(data);
		jint *body = env->GetIntArrayElements(data, 0);
		for (int i=0; i < size; i++)
			LOGI("Hello from JNI - element: %d\n", body[i]);


		Circle* cir = new Circle(10);
		LOGI("radius = %lf ",cir->radius);
		//cir.

		  env->ReleaseIntArrayElements(data, body, 0);
		  return 0;
	}

	JNIEXPORT jint JNICALL Native_OpenUart(JNIEnv *env,jobject mc, jstring s , jint fdnum)
	{

		const char *str1 = "/dev/";
		char *str2 = (char*)env->GetStringUTFChars(s, NULL);
		char *sall = (char*) malloc(strlen(str1) + strlen(str2) + 1);

		strcpy(sall, str1);
		strcat(sall, str2);

		LOGI("open uart port device node = %s , fdnum=%d \n",sall,fdnum);
		if (fdnum == 1)
		{
			driveFd = open(sall, O_RDWR | O_NOCTTY | O_NDELAY);
		}
		else if (fdnum == 2)
		{
			nanoFd = open(sall, O_RDWR | O_NOCTTY | O_NDELAY);
		}

		env->ReleaseStringUTFChars(s, str2);

		free(sall);
		return driveFd;
	}

	JNIEXPORT void JNICALL Native_CloseUart(JNIEnv *env,jobject mc, jint fdnum)
	{

		close(fdnum);
	}

	JNIEXPORT jint JNICALL Native_SetUart(JNIEnv *env,jobject mc, jint i,jint fdnum)
	{
		int Baud_rate[] = { B9600, B115200};
		LOGI("Native_SetUart %d", i);

		if (fdnum == 1)
		{

		tcgetattr(driveFd, &oldtio);
		tcgetattr(driveFd, &newtio);
		cfsetispeed(&newtio, Baud_rate[i]);
		cfsetospeed(&newtio, Baud_rate[i]);

		newtio.c_lflag = 0;
		newtio.c_cflag = Baud_rate[i] | CS8 | CREAD | CLOCAL;
		newtio.c_iflag = BRKINT | IGNPAR | IXON | IXOFF | IXANY;
		newtio.c_oflag = 02;
		newtio.c_line = 0;
		newtio.c_cc[7] = 255;
		newtio.c_cc[4] = 0;
		newtio.c_cc[5] = 0;

			if (tcsetattr(driveFd, TCSANOW, &newtio) < 0)
			{
				LOGE("tcsetattr2 fail !\n");
				exit(1);
			}

			return driveFd;
		}
		else if (fdnum == 2)
		{
			tcgetattr(nanoFd, &oldtio);
			tcgetattr(nanoFd, &newtio);
			cfsetispeed(&newtio, Baud_rate[i]);
			cfsetospeed(&newtio, Baud_rate[i]);

			newtio.c_lflag = 0;
			newtio.c_cflag = Baud_rate[i] | CS8 | CREAD | CLOCAL;
			newtio.c_iflag = BRKINT | IGNPAR | IXON | IXOFF | IXANY;
			newtio.c_oflag = 02;
			newtio.c_line = 0;
			newtio.c_cc[7] = 255;
			newtio.c_cc[4] = 0;
			newtio.c_cc[5] = 0;

				if (tcsetattr(nanoFd, TCSANOW, &newtio) < 0)
				{
					LOGE("tcsetattr2 fail !\n");
					exit(1);
				}

				return nanoFd;
		}
			return -1;
	}

	JNIEXPORT jint JNICALL Native_SendMsgUart(JNIEnv *env,jobject mc, jstring str, jint fdnum)
	{
		int len;
		const char *buf;
		buf = env->GetStringUTFChars(str, NULL);
		len = env->GetStringLength(str);
		if (fdnum == 1)
		{
			write(driveFd, buf, len);
		}
		else if (fdnum == 2)
		{
			write(nanoFd, buf, len);
		}
		env->ReleaseStringUTFChars(str, buf);
	}

	JNIEXPORT jbyteArray JNICALL Native_ReceiveMsgUart(JNIEnv *env,jobject mc, jint fdnum)
	{
		char buffer[255];
		char buf[255];
		char buffertest[255] = {'a','b','c','d','\0'};
		int len, i = 0, k = 0 , count = 0;
		memset(buffer, 0, sizeof(buffer));
		memset(buf, 0, sizeof(buf));

		if (fdnum == 1)
			len = read(driveFd, buffer, 255);
		else if (fdnum == 2)
			len = read(nanoFd, buffer, 255);

		for (i =0;i< 255 ; i++)
		{
			if (debugData)
			{
				if (buffertest[i] != '\0')
					count++;
			}
			else
			{
				if (buffer[i] != '\0')
				count++;
			}
		}

		LOGI("read on native function leng = %d" ,count);
		if(count <= 0)
		{
			return NULL;
		}

		jbyteArray arr = env->NewByteArray(count);
		if (debugData)
		{
			env->SetByteArrayRegion(arr,0,count, (jbyte*)buffertest);
		}
		else
			env->SetByteArrayRegion(arr,0,count, (jbyte*)buffer);

		//env->ReleaseByteArrayElements(arr, 0 );

		return arr;
		/////////////////


/*
		if (len > 0)
		{
			buffer[len]='\0';

			LOGI("read buffer = %s ",buffer);

			//return env->NewStringUTF(buffer);
			return buffer;
		}
		else
			buffer[0] = '0';

			return buffer;*/
	}


	static JNINativeMethod gMethods[] = {
		//Java Name			(Input Arg) return arg   JNI Name
		{"ReceiveMsgUart",   "(I)[B",(void *)Native_ReceiveMsgUart},
		{"SendMsgUart",   "(Ljava/lang/String;I)I",  (void *)Native_SendMsgUart},
		{"SetUart",   "(II)I",   					(void *)Native_SetUart},
		{"OpenUart",   "(Ljava/lang/String;I)I",   	(void *)Native_OpenUart},
		{"WriteDemoData",   "([II)I",   	(void *)Native_WriteDemoData},
		{"StartCal",   "()I",   	(void *)Native_StartCal},
		{"CloseUart",   "(I)V",   	(void *)Native_CloseUart},


	};

	static int registerNativeMethods(JNIEnv* env, const char* className,
		JNINativeMethod* gMethods, int numMethods)
	{
		jclass clazz;
		clazz = env->FindClass(className);
		if (clazz == NULL)
		{
			LOGI("can't find className=%s  \n",className);
			return JNI_FALSE;
		}

		if (env->RegisterNatives(clazz, gMethods, numMethods) < 0)
		{
		LOGE("register nativers error");
			return JNI_FALSE;
		}

		return JNI_TRUE;
	}

	static int register_android_native_uart(JNIEnv *env){

		 if (!registerNativeMethods(env, classPathName,
				 gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
			return JNI_FALSE;
		  }
		  return JNI_TRUE;
	}


	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
		JNIEnv* env = NULL;
		jint result = -1;

		LOGI("Entering JNI_OnLoad \n");

		if (vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK)
			goto bail;

		if (!register_android_native_uart(env))
			goto bail;

		/* success -- return valid version number */
		result = JNI_VERSION_1_4;

		bail:
			LOGI("Leaving JNI_OnLoad (result=0x%x)\n", result);
			return result;
	}
}



