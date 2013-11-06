#include <WIFI_RAK410.h>
#define  LED_PIN 13

extern uint8_t Send_buf[500];
extern uint8_t Receive_buf[500];

uint8_t flag = 0;//EN:Connect flag
uint16_t data_length=0;

void setup(void)
{
	pinMode(RESET_PIN,OUTPUT); 
	pinMode(LED_PIN,OUTPUT);
	Serial.begin(115200);
	flag = Wifi_init();//EN:WIFI module init
	digitalWrite(LED_PIN,1);
	delay(500);
	digitalWrite(LED_PIN,0);
	delay(500);
	digitalWrite(LED_PIN,1);
}
void loop(void)
{
	if(flag)
	{				
		data_length = 0;
		while(!data_length)//EN:Waiting for data
		{
			if(Serial.available()>0)
			{
				data_length = DrvUART_Read(Receive_buf);//EN:Receive serial data 
			}	
		}	
		if((Receive_buf[13])==0x81)	 //EN:TCP has been disconnected
		{
			flag=0;
			while(1)
			{
				digitalWrite(LED_PIN, 0);
				delay(500);
				digitalWrite(LED_PIN, 1);
				delay(500);
			}
			Receive_buf[13] = 0;
		}
		else//EN:Send serial received data
		{
			if(!Wifi_uart_send_data(0,Receive_buf,data_length))//EN:Wifi send data,returns 1 if successful, else it returns 0 中：Wifi发送数据，发送成功返回1，否则返回0；
			{
				while(1)
				{
					digitalWrite(LED_PIN, 0);
					delay(100);
					digitalWrite(LED_PIN, 1);
					delay(100);
				}
			}
			else
			{
				digitalWrite(LED_PIN, 0);
				delay(100);
				digitalWrite(LED_PIN, 1);		
			}
		}
	}
}
