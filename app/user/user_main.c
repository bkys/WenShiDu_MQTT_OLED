/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#include "dht.h"

#include "i2c_oled.h"

MQTT_Client mqttClient;

static ETSTimer dhtTimer;

uint8 formatchar[200],formatcharsize,showformatchar[100];//����һ���ַ��������ڸ�ʽ��Ҫ������ַ�����һ��������¼�ַ����ĳ���

char* tttt="Wed Dec 07 16:34:45 2016";//���ڴ洢��ȡ��������ʱ��
char chshowtime[21]={""};//���ڳ�������Ľ�ȡ�����Ϣ����Ϊoledһ��ֻ��128���أ�һ���ַ�ռ��6������

void MypollDHTCb(void)
{
	pollDHTCb();

	os_sprintf(showformatchar,"%d *C     %d %%",wendu,shidu);

	//ͬ������ʱ��
	uint32_t time = sntp_get_current_timestamp();
	if(time>0)
	{
	   os_strcpy(tttt,sntp_get_real_time(time));
	   os_strncpy(chshowtime,tttt+4,20);
	   os_printf("date:%s\r\n",chshowtime);
	}
	else
	{
		os_strcpy(chshowtime,"not get time,wait..");
	}
	//	����
	OLED_CLS();
	OLED_ShowStr(0,1,chshowtime,1);
	OLED_ShowStr(5,3,"WENDU    SHIDU",2);
	OLED_ShowStr(5,6,showformatchar,2);

	if (mqttClient.connState == MQTT_DATA)
	{

		formatcharsize=os_sprintf(formatchar,"Date:%s\r\nWENDU= %d *C,SHIDU= %d %%",chshowtime,wendu,shidu);
	    MQTT_Publish(&mqttClient, MQTT_TOPIC, formatchar, formatcharsize, 0, 0);
	}

	os_printf("WEN DU= %d *C,SHIDU= %d %%\r\n",wendu,shidu);

}

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
		//�������ʼ��SNTP
		sntp_init();

	} else {
		MQTT_Disconnect(&mqttClient);
	}
}
void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	//����
	MQTT_Subscribe(client, MQTT_TOPIC, 0);
	//����
	MQTT_Publish(client, MQTT_TOPIC, "Hello MQTT!", 11, 0, 0);
}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

	if(strcmp(dataBuf,"guan")==0)//���ַ����Աȵķ�ʽ��dataBuf��Guan�ַ�����ȣ�Ҳ�����ж�Ϊ�յ����ǡ�Guan��
		{
			INFO("GUAN BI LA \r\n");
			GPIO_OUTPUT_SET(GPIO_ID_PIN(5),1);//����gpio4λ��
		}
		else if(strcmp(dataBuf,"kai")==0)
		{
			INFO("DA KAI LA \r\n");
			GPIO_OUTPUT_SET(GPIO_ID_PIN(5),0);
		}
		else if(strcmp(dataBuf,"zhuangtai")==0)
				{
					if(GPIO_INPUT_GET(GPIO_ID_PIN(5))){dataBuf="gaodianping";}
					if(GPIO_INPUT_GET(GPIO_ID_PIN(5))){dataBuf="didianping";}
				}
		else
		{
			INFO("WU XIAO ZI FU \r\n");
		}
	//������ѯָ��
	if(strcmp(dataBuf,"chaxun")==0)
	{
		MypollDHTCb();
	}

	os_free(topicBuf);
	os_free(dataBuf);
}




/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;
            
        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;
            
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;
            
        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
            
        default:
            rf_cal_sec = 0;
            break;
    }
    
    return rf_cal_sec;
}


void user_init(void)
{
	uart_init(115200, 115200);

	//��ʼ��DHT11����DHT22����������������
	DHTInit(SENSOR_DHT11);
	wendu=shidu=0;//��ʼ����ʪ��Ϊ0


	//Esp8266��I2c���Žӿڳ�ʼ������
	i2c_master_gpio_init();
	//	OLED��ʼ������ʾ
	OLED_Init();
	OLED_ShowStr(0,3,"System Start....",2);

	//����SNTP������,׼��ͬ������ʱ��
    sntp_setservername(0,"us.pool.ntp.org");
    sntp_setservername(1,"ntp.sjtu.edu.cn");
    sntp_setservername(2,"0.cn.pool.ntp.org");

	os_delay_us(60000);

	//mqtt���ӳ�ʼ��
	MQTT_InitConnection(&mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);
	//mqtt�ͻ��˳�ʼ��
	MQTT_InitClient(&mqttClient, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, MQTT_KEEPALIVE, 1);
    //mqtt��������
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
    //mqtt�������ʱ�Ļص�����
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
    //mqtt�Ͽ�����ʱ�Ļص�����
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
    //mqtt������Ϣ��ɺ�Ļص�����
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	//mqtt���յ���Ϣ��Ļص�����
	MQTT_OnData(&mqttClient, mqttDataCb);


	WIFI_Connect(STA_SSID, STA_PASS, wifiConnectCb);

	//����һ��ʱ�ӣ�ÿ��ָ����ʱ��(10��)����һ��pollDHTCb������ȡ��ʪ��ֵ
	os_timer_disarm(&dhtTimer);
	os_timer_setfn(&dhtTimer, MypollDHTCb, NULL);
	os_timer_arm(&dhtTimer, 10000, 1);


	INFO("\r\nSystem started ...\r\n");
}
