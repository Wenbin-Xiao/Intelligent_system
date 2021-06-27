/*
 * 将模块与UNO控制器正确连接，切勿接错；
 * 程序效果：首先使得前方无障碍物，此时避障传感器为输出接口为低电平，将“NO”发送给上位机；
 *          将避障模块的红外探头对准障碍物，距离为3 – 30cm厘米以内，将“YES”发送给上位机；
 *          然后接受上位机信号，若为0，则设定LED 为输出接口为低电平灯灭，否则高电平灯亮
 */

int LedOfEastWest = 11;     //定义东西方向的LED 接口
int LedOfSouthNorth = 12;   //定义南北方向的LED 接口
int buttonpin = 3;          //定义避障传感器接口

int oldVal = LOW;           //定义避障传感器接口的数字变量val
int newVal = LOW;           //比oldval快一个loop

bool direct = 0;            //传感器检测的方向，0代表东西，1代表南北
int flag = 0;               //车辆计数
unsigned long startTime;    //时间槽的开始时间
const unsigned long timeInterval = 10000;   //检测车流量间隔10s

int len;                    // 上位机传下来的数据包长度
char dataBuffer[10];        // 上位机传下来的总数据内容：方向和绿灯时长
char timeFromUp[5];         // 上位机传下来的绿灯时长
int directFromUp;           // 上位机传下来的方向

int EastWestTime = 3000;    // 东西方向的绿灯时长
int SouthNorthTime = 3000;  // 南北方向的绿灯时长
int state = 0;              // 0代表检测东西方向的车流量，1代表检测南北方向的车流量，2代表显示两个方向的绿灯

void setup()
{
    pinMode(LedOfEastWest, OUTPUT);       //定义LedOfEastWest为输出接口
    pinMode(LedOfSouthNorth, OUTPUT);     //定义LedOfEastWest为输出接口
    pinMode(buttonpin, INPUT);            //定义避障传感器为输出接口
    Serial.begin(9600);                   //连接上位机，波特率为9600
    startTime = millis();
}

// 判断num的位数
int numLen(int num){
  if(num < 10){
    return 1;
  }
  else if(num < 100){
    return 2;
  }
  else if(num <1000){
    return 3;
  }
  else{
    return 1;
  }
}

//获取传感器的数据，每次有车辆通过则计数+1，时间间隔timeInterval
//间隔时间到，则进入数据处理
void get_sensor_data(){
        if (oldVal == LOW and newVal == HIGH){   
            flag ++;
        }
        if(millis() - startTime >= timeInterval){
            put_data_to_up();
        }  
}

//下位机对传感器数据的处理，包括编码、传输数据到上位机
//获取当前车辆数的长度，组装数据包，重置计数、时间，改变计数方向
void put_data_to_up(){
            int lenOfFlag = numLen(flag);
            Serial.print(lenOfFlag+2);
            Serial.print(flag);
            Serial.print(direct);
      
            // update
            direct = !direct;
            flag = 0;
            startTime = millis();
            state = state+1;
            if(state==2){
              delay(1000);
            }
}

//解析上位机数据包，更新亮灯时长
void analysis_data_from_up(){
        // 数据包内容解析
        len = Serial.read() - '0';
        Serial.readBytes(dataBuffer, len-1);
        dataBuffer[len-1] = '\0';
        directFromUp = dataBuffer[0] - '0';
        strcpy(timeFromUp, dataBuffer+1);
        timeFromUp[len-2] = '\0';

        // 更新两个方向的绿灯时长
        if(len > 0){
            if(directFromUp == 0){
                EastWestTime = atoi(timeFromUp) * 1000;
                if(EastWestTime < 3000){
                    EastWestTime = 3000;
                }
            } else if(directFromUp == 1){
                SouthNorthTime = atoi(timeFromUp) * 1000;
                if(SouthNorthTime < 3000){
                    SouthNorthTime = 3000;
                }
             } else{
                  EastWestTime = 1000;
                  SouthNorthTime = 1000;
             }
        } 
}

//信号灯控制程序
void show_light(){
        // 东西方向绿灯亮
        digitalWrite(LedOfEastWest, HIGH);
        digitalWrite(LedOfSouthNorth, LOW);
        delay(EastWestTime);
        // 南北方向绿灯亮
        digitalWrite(LedOfSouthNorth, HIGH);
        digitalWrite(LedOfEastWest, LOW);
        delay(SouthNorthTime);
        // 重新回到车流量检测状态
        digitalWrite(LedOfSouthNorth, LOW);
        digitalWrite(LedOfEastWest, LOW);
        startTime = millis();
        state = 0;  
}

void loop()
{
    // 接收传感器信号然后发送给上位机
    newVal = digitalRead(buttonpin); //将数字接口3的值读取赋给val

    // 检测东西方向车流量
    if(state == 0){
        get_sensor_data();
    }
    // 检测南北方向车流量
    else if(state == 1){
        get_sensor_data();
       
    }
    // 显示两个方向的绿灯
    else if(state == 2){     
        show_light();  
    }
    // 非法状态
    else{
      startTime = millis();
      state = 0;
    }
    // 更新接受传感器的旧数值
    oldVal = newVal;

    // 读取上位机传来的数据包
    if(Serial.available() > 0){
        analysis_data_from_up();    
    }
}
