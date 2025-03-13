#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
//WIFI�������� 
#define WIFI_SSID "ssss7777"
#define WIFI_PASSWORD "qqqqwwww"

//�������������� 
#define SERVER_IP "192.168.43.212"
#define SERVER_PORT 8082

// ???????(??:??)
#define CONNECT_DELAY 5000
#define COMMAND_DELAY 2000

uint16_t Num=0;
uint16_t p=0;
uint16_t r=0;
uint16_t CL=0;
uint16_t i;
float sum = 0.0;
uint16_t flag=0;
 uint32_t count = 0;
 uint32_t count1 = 0;


// ������ֵ�;�Ĭ��
float threshold = 1.5; // ��ֵ
int silent_period = 0; // ��Ĭ�ڼ�����
int jump_count = 0; 
int is_jumping = 0; 



// ����ƽ���˲�
float sliding_average_filter(float new_value, float *buffer, int buffer_size) {
    static int index = 0;
    buffer[index] = new_value;
    index = (index + 1) % buffer_size;

    for ( i = 0; i < buffer_size; i++) {
        sum += buffer[i];
    }
    return sum / buffer_size;
}

// ������ٶ�����

void process_acceleration(float value)
{
    static float filtered_value = 0.0;
    static float filter_buffer[10]; // ����ƽ���˲�������
    static int silent_period = 0; // ��Ĭ�ڼ�����

    // ����ƽ���˲�
    filtered_value = sliding_average_filter(value, filter_buffer, 10);

    if (silent_period > 0) 
	{
        silent_period--;
        return;
    }

    
}

//����1����1���ַ� 
//c:Ҫ���͵��ַ�
void usart1_send_char(u8 c)
{

	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,c);   

} 
//�������ݸ�����������λ�����(V2.6�汾)
//fun:������. 0XA0~0XAF
//data:���ݻ�����,���28�ֽ�!!
//len:data����Ч���ݸ���
void usart1_niming_report(u8 fun,u8*data,u8 len)
{
	u8 send_buf[32];
	u8 i;
	if(len>28)return;	//���28�ֽ����� 
	send_buf[len+3]=0;	//У��������
	send_buf[0]=0X88;	//֡ͷ
	send_buf[1]=fun;	//������
	send_buf[2]=len;	//���ݳ���
	for(i=0;i<len;i++)send_buf[3+i]=data[i];			//��������
	for(i=0;i<len+3;i++)send_buf[len+3]+=send_buf[i];	//����У���	
	for(i=0;i<len+4;i++)usart1_send_char(send_buf[i]);	//�������ݵ�����1 
}
//���ͼ��ٶȴ��������ݺ�����������
//aacx,aacy,aacz:x,y,z������������ļ��ٶ�ֵ
//gyrox,gyroy,gyroz:x,y,z�������������������ֵ
void mpu6050_send_data(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz)
{
	u8 tbuf[12]; 
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;
//	usart1_niming_report(0XA1,tbuf,12);//�Զ���֡,0XA1
}	
//ͨ������1�ϱ���������̬���ݸ�����
//aacx,aacy,aacz:x,y,z������������ļ��ٶ�ֵ
//gyrox,gyroy,gyroz:x,y,z�������������������ֵ
//roll:�����.��λ0.01�ȡ� -18000 -> 18000 ��Ӧ -180.00  ->  180.00��
//pitch:������.��λ 0.01�ȡ�-9000 - 9000 ��Ӧ -90.00 -> 90.00 ��
//yaw:�����.��λΪ0.1�� 0 -> 3600  ��Ӧ 0 -> 360.0��
void usart1_report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//��0
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;	
	tbuf[18]=(roll>>8)&0XFF;
	tbuf[19]=roll&0XFF;
	tbuf[20]=(pitch>>8)&0XFF;
	tbuf[21]=pitch&0XFF;
	tbuf[22]=(yaw>>8)&0XFF;
	tbuf[23]=yaw&0XFF;
//	usart1_niming_report(0XAF,tbuf,28);//�ɿ���ʾ֡,0XAF
} 
  
int main(void)
{ 
	u16 t=0,report=1;			//Ĭ�Ͽ����ϱ�
	u8 key;
	u16 cnnn=0;
	int max_gx=0,min_gx=0;
	int max_gy=0,min_gy=0;
	int max_gz=0,min_gz=0;
	u8 kk=0;
	float pitch,roll,yaw; 		//ŷ����
	short aacx,aacy,aacz;		//���ٶȴ�����ԭʼ����
	short gyrox,gyroy,gyroz;	//������ԭʼ����
	short temp;					//�¶�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ500000
	
	
	
	printf("AT\r\n");
	delay_ms (200);
	 printf("AT+RST\r\n");
	delay_ms(2000);
	printf("AT+CWQAP\r\n");
	delay_ms(2000);
	 printf("AT+CWMODE=1\r\n");//����WiFi
	delay_ms (200);
	
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
	delay_ms(CONNECT_DELAY);
	printf("AT+CIPMODE=1\r\n");
	delay_ms(COMMAND_DELAY);
	printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
	delay_ms(COMMAND_DELAY);
	printf("AT+CIPSEND\r\n");
	 delay_ms(2000);
	
	
	
	LED_Init();					//��ʼ��LED 
	KEY_Init();					//��ʼ������
 	LCD_Init();					//LCD��ʼ��
	MPU_Init();					//��ʼ��MPU6050
	BEEP_Init();
 	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,50,500,30,24,"  Climb counting system ");	
	LCD_ShowString(30,400,500,30,24,"Steps");
	LCD_ShowString(30,500,500,30,24,"floor");	
	while(mpu_dmp_init())
	{
		LCD_ShowString(30,130,200,16,16,"MPU6050 Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}
//    mpu_dmp_init();
	LCD_ShowString(30,130,200,16,16,"MPU6050 OK");
	LCD_ShowString(30,150,200,16,16,"KEY0:UPLOAD ON/OFF");
	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
 	LCD_ShowString(30,170,200,16,16,"UPLOAD ON ");	 
 	LCD_ShowString(30,200,200,16,16," Temp:    . C");	
 	LCD_ShowString(30,220,200,16,16,"Pitch:    . C");	
 	LCD_ShowString(30,240,200,16,16," Roll:    . C");	 
 	LCD_ShowString(30,260,200,16,16," Yaw :    . C");	
	LCD_ShowNum(100,400,Num,3,24);
	LCD_ShowNum(100,500,CL,3,24);		
	printf("start mmu\r\n");
	delay_ms(200);
 	while(1)
	{

		
		
		
		
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
		{ 
			temp=MPU_Get_Temperature();	//�õ��¶�ֵ
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//�õ����ٶȴ���������
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//�õ�����������
			if(report)mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);//���Զ���֡���ͼ��ٶȺ�������ԭʼ����
			if(report)usart1_report_imu(aacx,aacy,aacz,gyrox,gyroy,gyroz,(int)(roll*100),(int)(pitch*100),(int)(yaw*10));
//			printf("Frame ID: 0xA1\n");
//			delay_ms (200);
//            printf("AACX: %d, AACY: %d, AACZ: %d\n", aacx, aacy, aacz);
//			
//            printf("GYROX: %d, GYROY: %d, GYROZ: %d\n", gyrox, gyroy, gyroz);
			p=yaw+100;
			//printf("mmu %d %d %d %d %d %d %f %f %f\r\n",aacx,aacy,aacz,gyrox,gyroy,gyroz,pitch,roll,yaw);
			printf("mmu %d %d %d %d %d %d %d %d\r\n",aacx,aacy,aacz,gyrox,gyroy,gyroz,Num,CL);
			delay_ms(20);
			max_gx=aacx>max_gx?aacx:max_gx;//
			min_gx=aacx<min_gx?aacx:min_gx;
			max_gy=aacx>max_gy?aacx:max_gy;
			min_gy=aacx<min_gy?aacx:min_gy;
			max_gz=aacx>max_gz?aacx:max_gz;
			min_gz=aacx<min_gz?aacx:min_gz;
			kk=kk+1;
			if(kk>25)//բ���ж� 
			{
				if(max_gx-min_gx>8000||max_gz-min_gz>8000)
				{
				Num++;
				LCD_ShowNum(100,400,Num,3,24);		//��ʾ��������
				//printf("Steps %d\n\r",Num);
				if(Num%15==0)
				{
					BEEP_On();
					delay_ms (40);
					BEEP_Off();
					CL++;
					LCD_ShowNum(100,500,CL,3,24);		//��ʾ��������	
              		//printf("Floors reached %d\n\r",CL);
					

				}
				}
				max_gx=-65536;
				min_gx=65536;
				max_gy=-65536;
				min_gy=65536;
				max_gz=-65536;
				min_gz=65536;
				kk=0;
			}
			if(KEY2 == 0)
			{
				delay_ms(100);
				if(KEY2 == 0)
				{
					printf("start mmu\r\n");
					delay_ms(600);
					CL=0;
					Num=0;
					LCD_ShowNum(100,400,Num,3,24);
					LCD_ShowNum(100,500,CL,3,24);						
					continue;
				}
			}


			if((t%5)==0)
			{ 
				LCD_ShowString(30,200,200,16,16," Temp:    . C");	
 	LCD_ShowString(30,220,200,16,16,"Pitch:    . C");	
 	LCD_ShowString(30,240,200,16,16," Roll:    . C");	 
 	LCD_ShowString(30,260,200,16,16," Yaw :    . C");	 
				if(temp<0)
				{
					LCD_ShowChar(30+48,200,'-',16,0);		//��ʾ����
					temp=-temp;		//תΪ����
				}else LCD_ShowChar(30+48,200,' ',16,0);		//ȥ������ 
				LCD_ShowNum(30+48+8,200,temp/100,3,16);		//��ʾ��������	    
				LCD_ShowNum(30+48+40,200,temp%10,1,16);		//��ʾС������ 
				temp=pitch*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,220,'-',16,0);		//��ʾ����
					temp=-temp;		//תΪ����
				}else LCD_ShowChar(30+48,220,' ',16,0);		//ȥ������ 
				LCD_ShowNum(30+48+8,220,temp/10,3,16);		//��ʾ��������	    
				LCD_ShowNum(30+48+40,220,temp%10,1,16);		//��ʾС������ 
				temp=roll*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,240,'-',16,0);		//��ʾ����
					temp=-temp;		//תΪ����
				}else LCD_ShowChar(30+48,240,' ',16,0);		//ȥ������ 
				LCD_ShowNum(30+48+8,240,temp/10,3,16);		//��ʾ��������	    
				LCD_ShowNum(30+48+40,240,temp%10,1,16);		//��ʾС������ 
				temp=yaw*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,260,'-',16,0);		//��ʾ����
					temp=-temp;		//תΪ����
				}else LCD_ShowChar(30+48,260,' ',16,0);		//ȥ������ 
				LCD_ShowNum(30+48+8,260,temp/10,3,16);		//��ʾ��������	    
				LCD_ShowNum(30+48+40,260,temp%10,1,16);		//��ʾС������  
				t=0;
				LED0=!LED0;//LED��˸
			}
		}
		//if(cnnn >= 100)
		//{
		//	delay_ms(2000);
		//	printf("+++\r\n");
		//	delay_ms(2000);
		//	printf("AT\r\n");
		//	delay_ms(200);
		//	printf("AT+CIPCLOSE\r\n");
		//	delay_ms(2000);
		//	break;
		//}
		t++; 
		//cnnn++;
	} 	
}
