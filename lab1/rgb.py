"""
@name: lab1.py
@Describe: 通过python的串口库与下位机单片机通信
"""

import serial

serialPort = "COM2"  # 串口
baudRate = 9600  # 波特率
ser = serial.Serial(serialPort, baudRate, timeout=0.5) # 连接串口
print("参数设置：串口=%s ，波特率=%d" % (serialPort, baudRate))

def put_data_to_down(length,carNum):
    # ----------------组装数据包，传到下位机-----------------
    LenOfLightTime = length
    lightTime = carNum
    directionToLow = directionFromLow
    ser.write(str(LenOfLightTime).encode(encoding='gbk'))   # 数据包总长度
    ser.write(str(directionToLow).encode(encoding='gbk'))   # 方向
    ser.write(str(lightTime).encode(encoding='gbk'))        # 要更新的绿灯时长

def analysis_data_form_down():
    # -----------读取并解析下位机传来的数据包---------------
    # 数据包总长度
    length = ser.read(1).decode()
    # 空的数据包则跳过
    if len(length) == 0:
        return 0,0,0
    # 车辆数的位数(字节数）
    LenOfCarNum = int(length) - 2
    if LenOfCarNum < 1:
        carNum = int(ser.read(1).decode())
    else:
        carNum = int(ser.read(LenOfCarNum).decode())
    directionFromLow = int(ser.read(1).decode())
    return length,directionFromLow,carNum

while 1:
    #解析下位机传上来的数据,返回下位机数据长度、车辆方向、车辆数
    length,directionFromLow,carNum=analysis_data_form_down()

    if length !=0:
        print("方向(0代表东西，1代表南北）:", directionFromLow, "车流量:", carNum)
        #组装数据包、传输数据包到下位机
        put_data_to_down(length,carNum)


ser.close()