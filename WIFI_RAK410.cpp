#include "WIFI_RAK410.h"

/********************** The AT command definition **********************************/
#define    RAK_SCAN_CMD       "at+scan=0,TP-LINK_2.4GHz\r\n"	  //Scan the specified network
#define    RAK_PSK_CMD        "at+psk=lthonway123456\r\n"	  //Input of the specified network's password
#define    RAK_CONN_CMD       "at+connect=TP-LINK_2.4GHz\r\n"  //Connect to the specified network
#define    RAK_IPSTATIC_CMD   "at+ipstatic=192.168.1.253,255.255.255.0,192.168.1.1,0,0\r\n"//Set static IP
#define    RAK_IPDHCP_CMD    "at+ipdhcp=0\r\n"				//DHCP CLIENT														  
#define    RAK_PWRMODE_CMD    "at+pwrmode=0\r\n"			//power mode

#define    RAK_LTCP_CMD       "at+ltcp=25000\r\n"				   //Set up TCP Sever
#define    RAK_LUDP_CMD       "at+ludp=25000\r\n"				   //Set up UDP Sever
#define    RAK_UDP_CMD       "at+udp=192.168.1.54,25000,25000\r\n" //Set up UDP Client
#define    RAK_TCP_CMD       "at+tcp=192.168.1.54,25000,25000\r\n" //Set up TCP Client

#define    RAK_AP_PSK_CMD    "at+psk=1234567890\r\n"//Set AP PSK
#define    RAK_AP_CHANNEL_CMD    "at+channel=8\r\n"//Set AP channel
#define    RAK_AP_AP_CMD    "at+ap=arduino_wifi_test,0\r\n"//Set AP SSID
#define    RAK_AP_IPSTATIC_CMD    "at+ipstatic=192.168.2.2,255.255.255.0,192.168.2.1,0,0\r\n"//Set AP static IP
#define    RAK_AP_IPDHCP_CMD    "at+ipdhcp=1\r\n"//AP DHCP SERVER
#define    RAK_con_status_CMD    "at+con_status\r\n"//Query the Connection Status

uint8_t Send_buf[500] = {0};
uint8_t Receive_buf[500] = {0};

#define STA_MODE 1		//Set Operating Mode,1 is enable station mode,0 is enable AP mode
#define TCP_CLIENT 1	//1: enable TCP client,0: disable
#define UDP_CLIENT 0	//1: enable UDP client,0: disable
#define TCP_SERVER 0	//1: enable TCP server,0: disable
#define UDP_SERVER 0	//1: enable UDP server,0: disable

uint8_t Get_asc_length(uint16_t data)
{
	uint8_t n = 0;
	do{ 
		n++; 
	}while(data/=10);
	return n;
}
uint16_t DrvUART_Read(uint8_t *pu8RxBuf)
{
    uint16_t  u16Count=0;
	while(Serial.available())
	{
		pu8RxBuf[u16Count]= Serial.read();
		u16Count ++;
		delay(10);
	}
    return u16Count; 
}
/**
  * @brief  Wifi send data
  * @param  flag: Connection identifier.
  *			pu8TxBuf:Send data buffer
  *			data_length:Buffer length
  * @retval Success:return 1
  *			Error:return 0.
 */
uint8_t Wifi_uart_send_data(uint8_t flag,uint8_t *pu8TxBuf,uint16_t data_length)
{
	uint8_t data[20]={0x61,0x74,0x2b,0x73,0x65,0x6e,0x64,0x5f,0x64,0x61,0x74,0x61,0x3d,0x30,0x2c,0,0,0,0,0};//asc: at+send_data=< flag>,<data_length>
	uint16_t i = 0,ASC_length = 0;
	data[13] = 0x30+flag;
	ASC_length = Get_asc_length(data_length);
	
	switch(ASC_length)
	{
		case 1:
				 data[15] = 0x30+data_length;
				break;
		case 2:
				 data[15] = 0x30+(data_length / 10);
				 data[16] = 0x30+(data_length % 10);
				break;
		case 3:
				 data[15] = 0x30+(data_length / 100);
				 data[16] = 0x30+((data_length % 100) / 10);
				 data[17] = 0x30+((data_length % 100) % 10);
				break;
		case 4:
				 data[15] = 0x30+(data_length / 1000);
				 data[16] = 0x30+((data_length % 1000) / 100);
				 data[17] = 0x30+(((data_length % 1000) % 100) / 10);
				 data[18] = 0x30+(((data_length % 1000) % 100) % 10);
				break;
	}
	data[15+ASC_length] = 0x2c;
	for(i = (15+ASC_length)+data_length;i >=(16+ASC_length);i --)
	{
		pu8TxBuf[i]=pu8TxBuf[i-(16+ASC_length)];
	}
	for(i = 0;i < (16+ASC_length);i ++)
	{
		pu8TxBuf[i]=data[i];
	}
	pu8TxBuf[data_length + 16 + ASC_length] = 0x0d;
	pu8TxBuf[data_length + 17 + ASC_length] = 0x0a;
	if(Serial.available() > 0)
	{
		return 0;
	}
	Serial.write(pu8TxBuf,data_length + 18 + ASC_length);
	i = 0;
	while(!i)
	{
		if(Serial.available()>0)
		{
			i = DrvUART_Read(Receive_buf);
		}	
	}
	if(strncmp((char *)Receive_buf,"OK",2))
		return 0;
	else return 1;
}
void Reset_Target(void)
{
    digitalWrite(RESET_PIN,LOW);
    delay(100);
    digitalWrite(RESET_PIN,HIGH);
}

uint8_t Wifi_init(void)
{
	uint16_t comRbytes = 0;
	do{
		delay(500);
		Reset_Target();//Reset wifi module
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}
	}while( strncmp((char *)Receive_buf,"Welcome to RAK410",17) != 0); //Wait for "Welcome to RAK410"
#if STA_MODE
	Serial.write((uint8_t *)RAK_SCAN_CMD,strlen(RAK_SCAN_CMD)); //Send scan command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_PSK_CMD,strlen(RAK_PSK_CMD)); //Send set psk command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_CONN_CMD,strlen(RAK_CONN_CMD));//Send connect command 
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
	Serial.write((uint8_t *)RAK_PWRMODE_CMD,strlen(RAK_PWRMODE_CMD));//Send set power mode command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
#else
	Serial.write((uint8_t *)RAK_AP_CHANNEL_CMD,strlen(RAK_AP_CHANNEL_CMD)); //Send set channel command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_AP_PSK_CMD,strlen(RAK_AP_PSK_CMD)); //Send set psk command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_AP_IPSTATIC_CMD,strlen(RAK_AP_IPSTATIC_CMD)); //Send set static ip command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_AP_IPDHCP_CMD,strlen(RAK_AP_IPDHCP_CMD)); //Send dhcp server mode command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	Serial.write((uint8_t *)RAK_AP_AP_CMD,strlen(RAK_AP_AP_CMD)); //Send create ap command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}

	do{
		delay(100);
		Serial.write((uint8_t *)RAK_con_status_CMD,strlen(RAK_con_status_CMD)); //Query the Connection Status command
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}			
	}while( strncmp((char *)Receive_buf,"OK",2) != 0); //Wait for "OK";
#endif

#if TCP_CLIENT
	Serial.write((uint8_t *)RAK_IPDHCP_CMD,strlen(RAK_IPDHCP_CMD));  //Send dhcp client mode command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
	delay(500);
	Receive_buf[0] = 0x55;
	do{
		Serial.write((uint8_t *)RAK_TCP_CMD,strlen(RAK_TCP_CMD)); ////Create TCP client,connect TCP server
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}	
		delay(5);
	}while( strncmp((char *)Receive_buf,"OK",2) != 0); //Wait for "OK";
#endif

#if UDP_CLIENT
	Serial.write((uint8_t *)RAK_IPSTATIC_CMD ,strlen(RAK_IPSTATIC_CMD )); //Send set static ip command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
	delay(500);
	Receive_buf[0] = 0x55;
	do{
		Serial.write((uint8_t *)RAK_UDP_CMD,strlen(RAK_UDP_CMD)); //Create UDP client
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}	
		delay(5);
	}while( strncmp((char *)Receive_buf,"OK",2) != 0); //Wait for "OK";
#endif

#if TCP_SERVER
	Serial.write((uint8_t *)RAK_IPSTATIC_CMD ,strlen(RAK_IPSTATIC_CMD )); //Send set static ip command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
	delay(500);
	do{
		Serial.write((uint8_t *)RAK_LTCP_CMD,strlen(RAK_LTCP_CMD)); //Create TCP server
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}	
		delay(5);
	}while( strncmp((char *)Receive_buf,"OK",2) != 0); //Wait for "OK";
#endif

#if UDP_SERVER
	Serial.write((uint8_t *)RAK_IPSTATIC_CMD ,strlen(RAK_IPSTATIC_CMD )); //Send set static ip command
	comRbytes = 0;
	while(!comRbytes)
	{
		if(Serial.available()>0)
		{
			comRbytes = DrvUART_Read(Receive_buf);
		}	
	}
	delay(500);
	do{
		Serial.write((uint8_t *)RAK_LUDP_CMD,strlen(RAK_LUDP_CMD)); //Create UDP server
		comRbytes = 0;
		while(!comRbytes)
		{
			if(Serial.available()>0)
			{
				comRbytes = DrvUART_Read(Receive_buf);
			}	
		}	
		delay(5);
	}while( strncmp((char *)Receive_buf,"OK",2) != 0); //Wait for "OK";
#endif
	return 1;
}
