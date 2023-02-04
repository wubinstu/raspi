// 光子星物联网部分硬件驱动 
// Coding : UTF-8
// The File was Create By wubin
// QQ:2472446921 Mail:wubinstu@foxmail.com

#include "raspi_drive.h"
#include "log.h"

#ifdef    __WIRING_PI_H__

int initPi ()
{
    errno = 0;
    int err = wiringPiSetup ();
    if (err == -1)
    {
        perr_d (true, LOG_ERR, "wiringPi start up error");
        return err;
    }
    return 0;
}

float read_cpu_temp ()
{
    int cpu_fd = open (TEMP_PATH, O_RDONLY);
    char buf[10] = {0};
    read (cpu_fd, buf, 10);
    close (cpu_fd);
    return (float) strtod (buf, NULL) / 1000;
}

void turn_on_led (int pinNum)
{
    pinMode (pinNum, OUTPUT);
    digitalWrite (pinNum, LOW);
}

void turn_off_led (int pinNum)
{
    pinMode (pinNum, OUTPUT);
    digitalWrite (pinNum, HIGH);
}

void flash_led (int pinNum, int micro_secs)
{
    pinMode (pinNum, OUTPUT);
    int level = digitalRead (pinNum);
    if (level == HIGH)
    {
        digitalWrite (pinNum, LOW);
        delay (micro_secs);
        digitalWrite (pinNum, HIGH);
    } else if (level == LOW)
    {
        digitalWrite (pinNum, HIGH);
        delay (micro_secs);
        digitalWrite (pinNum, LOW);
    }

}

float disMeasure (int Trans, int Receive)
//返回值是单位为厘米的距离浮点数; 参数分别是两针脚的 wiringPi 数字编码号 (TX,RX)
{
    pinMode (Trans, OUTPUT);//设TX为输出模式
    pinMode (Receive, INPUT);//设RX为输入模式

    struct timeval tv1;
    struct timeval tv2;
    //结构体中有两个成员变量
    //其中，tv_sec为Epoch到创建struct timeval时的秒数，tv_usec为微秒数，即秒后面的零头

    long start, stop;
    float dis;

    digitalWrite (Trans, LOW);//写入阵脚电平为 0 (低电平) < 注意:低电平为有效电平 >
    delayMicroseconds (2); //时间延迟函数, 单位为毫秒
    digitalWrite (Trans, HIGH);
    delayMicroseconds (10);
    digitalWrite (Trans, LOW);

    while (digitalRead (Receive) != 1);//等待RX电平为 1 (高电平)
    gettimeofday (&tv1, NULL);//记录开始时间并写入结构体变量 tv1
    while (digitalRead (Receive) != 0);
    gettimeofday (&tv2, NULL);//记录结束时间并写入结构体变量 tv2

    start = tv1.tv_sec * 1000000 + tv1.tv_usec;//计算开始时间(微妙级)
    stop = tv2.tv_sec * 1000000 + tv2.tv_usec;
    dis = (float) (stop - start) / 1000000 * 34000 / 2; //计算距离 时间差(微秒) x 声速(厘米每秒)  x 二分之一
    return dis;//返回距离(单位:厘米)
}

bool TEST_IN (int LINE)//返回某阵脚的逻辑电平
{
    pinMode (LINE, INPUT);
    if (digitalRead (LINE) == HIGH)
        return true;
    else return false;
}

bool readSensorData (int pinNum, float *Humidity, float *Temperature)
//返回值表示数据读取成功或者失败  参数 pinNum 是数据线 wiringPi 数字编码号 
{
    unsigned long data = 1;
    pinMode (pinNum, OUTPUT);
    digitalWrite (pinNum, LOW);
    delay (25);
    digitalWrite (pinNum, HIGH);

    pinMode (pinNum, INPUT);
    pullUpDnControl (pinNum, PUD_UP);
    delayMicroseconds (27);
    if (digitalRead (pinNum) == LOW)//SENSOR ANS
    {
        while (digitalRead (pinNum) == LOW);//wait to high
        for (int i = 0; i < 32; i++)
        {
            while (digitalRead (pinNum) == HIGH);//data clock start
            while (digitalRead (pinNum) == LOW);//data start
            delayMicroseconds (32);
            data *= 2;
            if (digitalRead (pinNum) == HIGH)
                data++;
        }//不要问我怎么实现的,我也不知道
        *Humidity = (float) ((data >> 24) & 0xff) + (float) ((data >> 16) & 0xff) * (float) 0.1;
        *Temperature = (float) ((data >> 8) & 0xff) + (float) ((data >> 0) & 0xff) * (float) 0.1;
        //参数 data 是无符号长整形数据(32位),前16位代表湿度和后16位温度,然后每16位数的前八位和后八位分别代表小数点前后
        return true;
    } else return false;
    //DHT11是一款有已校准数字信号输出的温湿度传感器。
    //其精度湿度+-5%RH， 温度+-2℃，量程湿度20-90%RH， 温度0~50℃。
}


bool screen (int num, int dataPin, int clockPin)
{

#define TM16XX_CMD_DATA_AUTO 0x40 //地址
#define TM16XX_CMD_ADDRESS 0xC0
#define TM16XX_CMD_DATA_FIXED 0x44
#define TM16XX_CMD_DISPLAY 0x80

    typedef int byte;
    const byte TM16XX_NUMBER_FONT[] =
            {
                    0b00111111, // 0
                    0b00000110, // 1
                    0b01011011, // 2
                    0b01001111, // 3
                    0b01100110, // 4
                    0b01101101, // 5
                    0b01111101, // 6
                    0b00000111, // 7
                    0b01111111, // 8
                    0b01101111, // 9
                    0b01110111, // A
                    0b01111100, // B
                    0b00111001, // C
                    0b01011110, // D
                    0b01111001, // E
                    0b01110001  // F
            };
    //Initialization
    pinMode (dataPin, OUTPUT);
    pinMode (clockPin, OUTPUT);
    digitalWrite (clockPin, HIGH);

    // start
    digitalWrite (dataPin, LOW);
    digitalWrite (clockPin, LOW);
    delayMicroseconds (50);

    byte data = TM16XX_CMD_DATA_AUTO;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite (clockPin, LOW);
        delayMicroseconds (50);
        digitalWrite (dataPin, data & 1 ? HIGH : LOW);
        delayMicroseconds (50);
        data = data >> 1;
        digitalWrite (clockPin, HIGH);
        delayMicroseconds (50);
    }
    delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
    digitalWrite (clockPin, LOW);
    digitalWrite (dataPin, LOW);
    delayMicroseconds (50);

    //stop
    digitalWrite (clockPin, HIGH);
    digitalWrite (dataPin, HIGH);
    delayMicroseconds (50);




    //start
    digitalWrite (dataPin, LOW);
    digitalWrite (clockPin, LOW);
    delayMicroseconds (50);

    data = TM16XX_CMD_ADDRESS;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite (clockPin, LOW);
        delayMicroseconds (50);
        digitalWrite (dataPin, data & 1 ? HIGH : LOW);
        delayMicroseconds (50);
        data = data >> 1;
        digitalWrite (clockPin, HIGH);
        delayMicroseconds (50);
    }
    delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
    digitalWrite (clockPin, LOW);
    digitalWrite (dataPin, LOW);
    delayMicroseconds (50);

    for (int i = 0; i < 16; i++)
    {
        data = 0x00;
        for (int j = 0; j < 8; j++)
        {
            digitalWrite (clockPin, LOW);
            delayMicroseconds (50);
            digitalWrite (dataPin, data & 1 ? HIGH : LOW);
            delayMicroseconds (50);
            data = data >> 1;
            digitalWrite (clockPin, HIGH);
            delayMicroseconds (50);
        }
        delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
        digitalWrite (clockPin, LOW);
        digitalWrite (dataPin, LOW);
        delayMicroseconds (50);
    }
    //stop
    digitalWrite (clockPin, HIGH);
    digitalWrite (dataPin, HIGH);
    delayMicroseconds (50);




    //start
    digitalWrite (dataPin, LOW);
    digitalWrite (clockPin, LOW);
    delayMicroseconds (50);

    data = TM16XX_CMD_DISPLAY | 8 | 7;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite (clockPin, LOW);
        delayMicroseconds (50);
        digitalWrite (dataPin, data & 1 ? HIGH : LOW);
        delayMicroseconds (50);
        data = data >> 1;
        digitalWrite (clockPin, HIGH);
        delayMicroseconds (50);
    }
    delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
    digitalWrite (clockPin, LOW);
    digitalWrite (dataPin, LOW);
    delayMicroseconds (50);

    //stop
    digitalWrite (clockPin, HIGH);
    digitalWrite (dataPin, HIGH);
    delayMicroseconds (50);


    int digits;
    if (num % 1000 != 0)
        digits = 4;
    else if ((num % 1000 == 0) && (num % 100 != 0))
        digits = 3;
    else if (((num % 1000 == 0) && (num % 100 == 0)) && (num % 10 != 0))
        digits = 2;
    else
        digits = 1;
    for (byte i = 1; i <= digits; i++)
    {
        //start
        digitalWrite (dataPin, LOW);
        digitalWrite (clockPin, LOW);
        delayMicroseconds (50);
        data = TM16XX_CMD_DATA_FIXED;

        for (int j = 0; j < 8; j++)
        {
            digitalWrite (clockPin, LOW);
            delayMicroseconds (50);
            digitalWrite (dataPin, data & 1 ? HIGH : LOW);
            delayMicroseconds (50);
            data = data >> 1;
            digitalWrite (clockPin, HIGH);
            delayMicroseconds (50);
        }
        delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
        digitalWrite (clockPin, LOW);
        digitalWrite (dataPin, LOW);
        delayMicroseconds (50);

        //stop
        digitalWrite (clockPin, HIGH);
        digitalWrite (dataPin, HIGH);
        delayMicroseconds (50);




        //start
        digitalWrite (dataPin, LOW);
        digitalWrite (clockPin, LOW);
        delayMicroseconds (50);

        data = TM16XX_CMD_ADDRESS | (digits - i);
        for (int j = 0; j < 8; j++)
        {
            digitalWrite (clockPin, LOW);
            delayMicroseconds (50);
            digitalWrite (dataPin, data & 1 ? HIGH : LOW);
            delayMicroseconds (50);
            data = data >> 1;
            digitalWrite (clockPin, HIGH);
            delayMicroseconds (50);
        }
        delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
        digitalWrite (clockPin, LOW);
        digitalWrite (dataPin, LOW);
        delayMicroseconds (50);

        data = TM16XX_NUMBER_FONT[num % 10];
        for (int j = 0; j < 8; j++)
        {
            digitalWrite (clockPin, LOW);
            delayMicroseconds (50);
            digitalWrite (dataPin, data & 1 ? HIGH : LOW);
            delayMicroseconds (50);
            data = data >> 1;
            digitalWrite (clockPin, HIGH);
            delayMicroseconds (50);
        }
        delayMicroseconds (50); // NOTE: TM1638 specifies a Twait between bytes of minimal 1us.
        digitalWrite (clockPin, LOW);
        digitalWrite (dataPin, LOW);
        delayMicroseconds (50);

        //stop
        digitalWrite (clockPin, HIGH);
        digitalWrite (dataPin, HIGH);
        delayMicroseconds (50);

        num /= 10;
    }

    return true;
}

#else // __WIRING_PI_H__


int initPi ()
{
    errno = 0;
    perr_d (true, LOG_INFO, "The wiringpi library is not used. Enter the simulator mode. "
                            "All monitoring data will come from random numbers in the future");
    return 0;
}

int randnum ()
{
    srand ((unsigned int) time (0));
    return rand ();
}

float read_cpu_temp ()
{
    return (float) (randnum () % 31 + 30);
}

void turn_on_led (int pinNum)
{
    perr_d (true, LOG_INFO, "[simulator] pinNum:%d: led turned on", pinNum);
}

void turn_off_led (int pinNum)
{
    perr_d (true, LOG_INFO, "[simulator] pinNum:%d: led turned off", pinNum);
}

void flash_led (int pinNum, int micro_secs)
{
    perr_d (true, LOG_INFO, "[simulator] pinNum:%d: led flashed", pinNum);
}

float disMeasure (int Trans, int Receive)
{
    return (float) (randnum () % 701 + 20);
}

bool TEST_IN (int LINE)
{
    return true;
}

bool readSensorData (int pinNum, float *Humidity, float *Temperature)
{
    perr_d (true, LOG_INFO, "[simulator] pinNum:%d: readSensorData", pinNum);
    *Humidity = (float) (randnum () % 41 + 40);
    *Temperature = (float) (randnum () % 41 + 20);
    return true;
}

bool screen (int num, int dataPin, int clockPin)
{
    perr_d (true, LOG_INFO, "[simulator] print:%d", num);
    return true;
}

#endif // __WIRING_PI_H__