#include <REGX51.H>
#include <absacc.h>
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char u8;
typedef unsigned int u16;
#define ALESTART P2_3
#define ADCBUSY  P2_5
#define DATA P0
#define SEG_Port P1  //数码管显示
sbit LW4=P3^3;
sbit LW3=P3^2;
sbit LW2=P3^1;
sbit LW1=P3^0;
sbit EN=P2^7;//373锁存器开关
float Volt[8];

u8 channel=0;//通道
//循环标志
u8 h=0;
u8 q=0;
u8 p=0;
u8 flag=0;//ADC采集完成标志位
u8 flag1=0;//数码管数字两秒跳动标志位
u8 SEG_Sel[4]={0x08,0x04,0x02,0x01};   //数码管位选数据
u8 SEG[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};  //数码管段选数据


//-----------------------------------------------

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define SFR */
sbit TEST_LED = P1^0;               //work LED, flash once per second

/* define variables */
WORD count;                         //1000 times counter
WORD count1;                         //1000 times counter
/*****函数声明*****/
void ADC_Init(void);
float Get_Vlot(void);
void Delay(u8 time);
void SEG_Show(void);
void Delay_ms(xms);

//-----------------------------------------------

/* Timer0 interrupt routine */
void tm1_isr() interrupt 3
{
    TL1 = T1MS;                     //reload timer1 low byte
    TH1 = T1MS >> 8;                //reload timer1 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 10000;               //reset counter
//       TEST_LED = ! TEST_LED;      //work LED flash
		if(flag==0)
		{
			for(h=0;h<8;h++)
			 {
		   channel=h;
		   Volt[h]=Get_Vlot();
			 }
			 if(h==8)
					 h=0;
		flag=1;
		}
		EN = 1;

    }
}

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1
{
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count1-- == 0)               //1ms * 1000 -> 1s
    {
        count1 = 2000;               //reset counter
        flag1=1;
		h++;
		if(h==8)
		 {
			 h=0;
		    
		 }
    }
}


void Delay_ms(xms)
{
	int x,y;
	for(x=xms;x>0;x--)
	for(y=110;y>0;y--);
}
void ADC_Init(void) //ADC初始化
{
 DATA=0xff;  //IO口用做输入时，先输出1
 ADCBUSY=1;  //IO口用做输入时，先输出1
 ALESTART=0;
}
float Get_Vlot(void)//电压采集函数
{
 u16 num;
 P2&=0xf8;
 P2|=channel;   //ADDA,ADDB,ADDC模拟通道选择地址信号
 ALESTART=1;
 Delay(2);
 ALESTART=0;
 Delay(2);
 while(ADCBUSY==0);  //等待EOC置高
 num=DATA;           //读取数据
 return (num*5000.0/255.0);  //毫伏显示
}
void Delay(u8 time)
{
 unsigned char i, j;
 for (i=0; i<time; i++)
  for (j=0; j<12; j++);
}
void SEG_Show(void) //数码管显示
{
	
	int temp;
           if(flag1==1)
		   {
			u8 ord;
			temp=(int)Volt[h];  
			LW1=1;
			SEG_Port=SEG[h+1];
			Delay(10);
			LW1=0;
			SEG_Port=0xff;
			LW2=1;
			SEG_Port=SEG[temp/1000]&0x7f;
			Delay(10);
			SEG_Port=0xff;
			LW2=0;
			LW3=1;
			SEG_Port=SEG[(temp/100)%10];
			 Delay(10);
			SEG_Port=0xff;
			LW3=0; 
			LW4=1;
			SEG_Port=SEG[(temp/10)%10];
			 Delay(10);
			SEG_Port=0xff;
			LW4=0;
			flag1==0;
		}
	
		
}
/* main program */
void main()
{
    TMOD = 0x11;                    //set timer1 as mode1 (16-bit)
    TL1 = T1MS;                     //initial timer1 low byte
    TH1 = T1MS >> 8;                //initial timer1 high byte
    TR1 = 1;                        //timer1 start running
    ET1 = 1;                        //enable timer1 interrupt
	TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter
    count1 = 0;                      //initial counter
	EN = 0;
    ADC_Init();
	 while(1)
	 {
	
		 SEG_Show();
		 if(h==8)
		 {
			EN = 0;
			h = 0;
		    flag = 0;
			 for(q=0;q<8;q++) //向外部ARM写入数据
			 {
				XBYTE[0x0100+p]=Volt[q];
				p++;
			 }
		 }
		

	 }
}
