#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
//WIFI连接配置 
#define WIFI_SSID "ssss7777"
#define WIFI_PASSWORD "qqqqwwww"

//服务器连接配置 
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


// 定义阈值和静默期
float threshold = 1.5; // 阈值
int silent_period = 0; // 静默期计数器
int jump_count = 0; 
int is_jumping = 0; 



// 滑动平均滤波
float sliding_average_filter(float new_value, float *buffer, int buffer_size) {
    static int index = 0;
    buffer[index] = new_value;
    index = (index + 1) % buffer_size;

    for ( i = 0; i < buffer_size; i++) {
        sum += buffer[i];
    }
    return sum / buffer_size;
}

// 处理加速度数据

void process_acceleration(float value)
{
    static float filtered_value = 0.0;
    static float filter_buffer[10]; // 滑动平均滤波缓冲区
    static int silent_period = 0; // 静默期计数器

    // 滑动平均滤波
    filtered_value = sliding_average_filter(value, filter_buffer, 10);

    if (silent_period > 0) 
	{
        silent_period--;
        return;
    }

    
}

//串口1发送1个字符 
//c:要发送的字符
void usart1_send_char(u8 c)
{

	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,c);   

} 
//传送数据给匿名四轴上位机软件(V2.6版本)
//fun:功能字. 0XA0~0XAF
//data:数据缓存区,最多28字节!!
//len:data区有效数据个数
void usart1_niming_report(u8 fun,u8*data,u8 len)
{
	u8 send_buf[32];
	u8 i;
	if(len>28)return;	//最多28字节数据 
	send_buf[len+3]=0;	//校验数置零
	send_buf[0]=0X88;	//帧头
	send_buf[1]=fun;	//功能字
	send_buf[2]=len;	//数据长度
	for(i=0;i<len;i++)send_buf[3+i]=data[i];			//复制数据
	for(i=0;i<len+3;i++)send_buf[len+3]+=send_buf[i];	//计算校验和	
	for(i=0;i<len+4;i++)usart1_send_char(send_buf[i]);	//发送数据到串口1 
}
//发送加速度传感器数据和陀螺仪数据
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
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
//	usart1_niming_report(0XA1,tbuf,12);//自定义帧,0XA1
}	
//通过串口1上报结算后的姿态数据给电脑
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
void usart1_report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//清0
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
//	usart1_niming_report(0XAF,tbuf,28);//飞控显示帧,0XAF
} 
  
int main(void)
{ 
	u16 t=0,report=1;			//默认开启上报
	u8 key;
	u16 cnnn=0;
	int max_gx=0,min_gx=0;
	int max_gy=0,min_gy=0;
	int max_gz=0,min_gz=0;
	u8 kk=0;
	float pitch,roll,yaw; 		//欧拉角
	short aacx,aacy,aacz;		//加速度传感器原始数据
	short gyrox,gyroy,gyroz;	//陀螺仪原始数据
	short temp;					//温度
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为500000
	
	
	
	printf("AT\r\n");
	delay_ms (200);
	 printf("AT+RST\r\n");
	delay_ms(2000);
	printf("AT+CWQAP\r\n");
	delay_ms(2000);
	 printf("AT+CWMODE=1\r\n");//连接WiFi
	delay_ms (200);
	
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
	delay_ms(CONNECT_DELAY);
	printf("AT+CIPMODE=1\r\n");
	delay_ms(COMMAND_DELAY);
	printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
	delay_ms(COMMAND_DELAY);
	printf("AT+CIPSEND\r\n");
	 delay_ms(2000);
	
	
	
	LED_Init();					//初始化LED 
	KEY_Init();					//初始化按键
 	LCD_Init();					//LCD初始化
	MPU_Init();					//初始化MPU6050
	BEEP_Init();
 	POINT_COLOR=RED;//设置字体为红色 
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
	POINT_COLOR=BLUE;//设置字体为蓝色 
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
			temp=MPU_Get_Temperature();	//得到温度值
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
			if(report)mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);//用自定义帧发送加速度和陀螺仪原始数据
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
			if(kk>25)//闸门判断 
			{
				if(max_gx-min_gx>8000||max_gz-min_gz>8000)
				{
				Num++;
				LCD_ShowNum(100,400,Num,3,24);		//显示整数部分
				//printf("Steps %d\n\r",Num);
				if(Num%15==0)
				{
					BEEP_On();
					delay_ms (40);
					BEEP_Off();
					CL++;
					LCD_ShowNum(100,500,CL,3,24);		//显示整数部分	
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
					LCD_ShowChar(30+48,200,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,200,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,200,temp/100,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,200,temp%10,1,16);		//显示小数部分 
				temp=pitch*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,220,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,220,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,220,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,220,temp%10,1,16);		//显示小数部分 
				temp=roll*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,240,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,240,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,240,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,240,temp%10,1,16);		//显示小数部分 
				temp=yaw*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,260,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,260,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,260,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,260,temp%10,1,16);		//显示小数部分  
				t=0;
				LED0=!LED0;//LED闪烁
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
