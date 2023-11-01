#include "HLKModuleConfig.h"
#define BUFFER_SIZE 256 // 串口缓冲区大小

HLKModuleConfig::HLKModuleConfig(HardwareSerial &serial, SoftwareSerial &debugSerial) : _serial(serial), _debugSerial(debugSerial),
                                                                                        bufferIndex(0), receiveStartTime(0), isInATMode(0), bufferIndex_2410(0)
{
}

HLKModuleConfig::HLKModuleConfig(HardwareSerial &serial) : _serial(serial), bufferIndex(0), receiveStartTime(0), isInATMode(0),
                                                           bufferIndex_2410(0)
{
}

// B35
int HLKModuleConfig::enterATMode()
{

    _serial.print("+++"); // 发送指令+++

    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial.available())
        {
            char receivedChar = _serial.read();
            buffer[bufferIndex] = receivedChar;
            bufferIndex++;

            // 更新接收开始时间
            receiveStartTime = millis();
        }

        // 检查是否达到发送条件
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            if (this->bufferIndex == 1 && buffer[0] == 'a')
            {
                _serial.print('a');
            }
            else if (buffer[0] == 'O' && buffer[1] == 'K')
            { // 进入at模式
                if (_debugSerial.isListening())
                {
                    _debugSerial.println("Enter AT Mode Success!");
                }

                isInATMode = 1;
                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 1;
            }
            else
            {
                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 0;
            }
            // 清空缓冲区和索引
            memset(this->buffer, 0, BUFFER_SIZE);
            this->bufferIndex = 0;
        }

        if (millis() - startTime > 5000)
        {             // 超过5秒等待时间
            return 0; // 超时，未成功进入AT模式
        }
    }

    return 0; // 未成功进入AT模式
}

int HLKModuleConfig::exitATMode()
{
    _serial.println("at+reconn=1");

    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial.available())
        {
            char receivedChar = _serial.read(); //   接收串口数据
            buffer[bufferIndex] = receivedChar; //   接收到的数据都存入缓冲区里
            bufferIndex++;

            // 更新接收开始时间
            receiveStartTime = millis();
        }

        // 检查是否达到发送条件
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            // 检查缓冲区中是否包含 "OK"
            char *found = strstr(this->buffer, "ok");
            if (found != NULL)
            {
                if (_debugSerial.isListening())
                {
                    _debugSerial.println("Setting Success!");
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 1;
            }
            else
            {
                if (_debugSerial.isListening())
                {
                    // 发送缓冲区中的数据到另一个串口
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial.print(buffer[i]);
                    }
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 0;
            }
        }

        if (millis() - startTime > 3000) // 超过3秒等待时间
        {
            return 0; // 超时，未成功进入AT模式
        }
    }
}

int HLKModuleConfig::checkBuffer()
{
    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial.available())
        {
            char receivedChar = _serial.read(); //   接收串口数据
            buffer[bufferIndex] = receivedChar; //   接收到的数据都存入缓冲区里
            bufferIndex++;

            // 更新接收开始时间
            receiveStartTime = millis();
        }

        // 检查是否达到发送条件
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            // 检查缓冲区中是否包含 "OK"
            char *found = strstr(this->buffer, "ok");
            if (found != NULL)
            {
                if (_debugSerial.isListening())
                {
                    _debugSerial.println("Setting Success!");
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;

                return 1;
            }
            else
            {
                if (_debugSerial.isListening())
                {
                    // 发送缓冲区中的数据到另一个串口
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial.print(buffer[i]);
                    }
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;

                return 0;
            }
        }

        if (millis() - startTime > 3000) // 超过3秒等待时间
        {
            return 0; // 超时，未成功进入AT模式
        }
    }
}

int HLKModuleConfig::sendATCommandWithExit(String command)
{
    _serial.println(command);
    int ret = checkBuffer();
    exitATMode();
    return ret;
}

int HLKModuleConfig::sendATCommand(String command)
{
    _serial.println(command);
    int ret = checkBuffer();
    return ret;
}

int HLKModuleConfig::getVer()
{
    enterATMode(); // 进入AT模式
    return sendATCommandWithExit("at+ver=?");
}

int HLKModuleConfig::setNetwork(String ssid, String password)
{
    enterATMode(); // 进入AT模式
    sendATCommand("at+netmode=2");
    sendATCommand("at+wifi_conf=" + ssid + ",none," + password);
    sendATCommandWithExit("at+net_commit=1");
    return 1;
}

// 2410
const byte HLKModuleConfig::frameStart_2410[HLKModuleConfig::FRAME_START_SIZE_2410] = {0xF4, 0xF3, 0xF2, 0xF1};
const byte HLKModuleConfig::frameEnd_2410[HLKModuleConfig::FRAME_END_SIZE_2410] = {0xF8, 0xF7, 0xF6, 0xF5};

const byte HLKModuleConfig::frameAskStart_2410[HLKModuleConfig::FRAME_START_SIZE_2410] = {0xFD, 0xFC, 0xFB, 0xFA};
const byte HLKModuleConfig::frameAskEnd_2410[HLKModuleConfig::FRAME_END_SIZE_2410] = {0x04, 0x03, 0x02, 0x01};
HLKModuleConfig::TargetStatus_2410 status = HLKModuleConfig::TargetStatus_2410::NoTarget;

HLKModuleConfig::RadarStatus_2410 HLKModuleConfig::getStatus_2410()
{

    bufferIndex_2410 = 0; // 重置缓冲区索引
    HLKModuleConfig::RadarStatus_2410 radarStatus_2410;
    const int MIN_FRAME_LENGTH_BASE = 23;        // 最小帧长度,基本上报
    const int MIN_FRAME_LENGTH_ENGINEERING = 45; // 最小帧长度,工程上报

    // 从串口读取数据并存储到buffer中
    while (_serial.available() && bufferIndex_2410 < BUFFER_SIZE)
    {
        buffer_2410[bufferIndex_2410] = _serial.read();
        bufferIndex_2410++;

        // 检查是否收到完整的帧
        int startIndex = findSequence(buffer_2410, bufferIndex_2410, frameStart_2410, 4);
        int endIndex = findSequence(buffer_2410, bufferIndex_2410, frameEnd_2410, 4);

        // 检查是否收到完整的帧
        int startAskIndex = findSequence(buffer_2410, bufferIndex_2410, frameAskStart_2410, 4);
        int endAskIndex = findSequence(buffer_2410, bufferIndex_2410, frameAskEnd_2410, 4);

        uint8_t finalBuffer_2410[(endIndex + 4) - startIndex];
        // 接收到完整帧
        if (startIndex != -1 && endIndex != -1 && endIndex > startIndex)
        {
            uint8_t tmp_buffer[(endIndex + 4) - startIndex];
            for (int i = startIndex; i < endIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_2410[i];
            }
            // 解析数组
            int lastFrameStart = -1;
            int lastFrameEnd = -1;
            int tmp_bufferSize = sizeof(tmp_buffer);
            for (int i = tmp_bufferSize - 4; i >= 0; i--)
            {
                if (tmp_buffer[i] == 0xF4 && tmp_buffer[i + 1] == 0xF3 && tmp_buffer[i + 2] == 0xF2 && tmp_buffer[i + 3] == 0xF1 && lastFrameStart == -1)
                {
                    lastFrameStart = i;
                }
                if (tmp_buffer[i] == 0xF8 && tmp_buffer[i + 1] == 0xF7 && tmp_buffer[i + 2] == 0xF6 && tmp_buffer[i + 3] == 0xF5 && lastFrameEnd == -1)
                {
                    lastFrameEnd = i;
                }
            }
            if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd && (lastFrameEnd - lastFrameStart + 4) >= MIN_FRAME_LENGTH_BASE)
            {
                int j = 0;
                for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                {
                    finalBuffer_2410[j++] = tmp_buffer[i];
                }
            }
            else
            {
                // 如果帧不完整或太短，则清空finalBuffer_2410
                memset(finalBuffer_2410, 0, sizeof(finalBuffer_2410));
                return radarStatus_2410;
            }
            // 解析数组结束
            // 获取数组长度
            int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

            // 判断第六位是工程模式数据还是基本模式数据
            int mode = finalBuffer_2410[6];
            radarStatus_2410.radarMode_2410 = mode;
            if (mode == 1) // 工程模式上报
            {
                if (finalBufferSize > 45)
                {
                    return radarStatus_2410;
                }
                else
                {
                    radarStatus_2410.moveSetDistance = finalBuffer_2410[17];   // 最远运动距离门
                    radarStatus_2410.staticSetDistance = finalBuffer_2410[18]; // 最远静止距离门
                    radarStatus_2410.photosensitive = finalBuffer_2410[37];    // 光敏
                    for (int i = 0; i < 9; i++)                                // 运动和静止每个距离门的能量值
                    {
                        radarStatus_2410.radarMovePower.moveGate[i] = finalBuffer_2410[i + 19];
                        radarStatus_2410.radarStaticPower.staticGate[i] = finalBuffer_2410[i + 28];
                    }
                }
            }
            else if (mode == 2) // 基本模式上报
            {
                if (finalBufferSize > 23)
                {
                    return radarStatus_2410;
                }
            }

            // 提取雷达上报状态
            if (_debugSerial.isListening())
            {
                _debugSerial.println(finalBuffer_2410[8]);
            }
            status = static_cast<HLKModuleConfig::TargetStatus_2410>(finalBuffer_2410[8]);
            radarStatus_2410.targetStatus = static_cast<HLKModuleConfig::TargetStatus_2410>(finalBuffer_2410[8]);

            // 提取目标距离
            int distance = finalBuffer_2410[15] | (finalBuffer_2410[16] << 8); // 小端序解析
            if (_debugSerial.isListening())
            {
                _debugSerial.println("distance: " + String(distance));
            }
            radarStatus_2410.distance = distance;

            // _debugSerial.print("nature: ");
            // for (int i = 0; i < sizeof(tmp_buffer); i++)
            // {
            //     if (tmp_buffer[i] < 0x10)
            //         _debugSerial.print("0");
            //     _debugSerial.print(tmp_buffer[i], HEX);
            //     _debugSerial.print(" ");
            // }
            // _debugSerial.println();

            // _debugSerial.print("Payload: ");
            // for (int i = 0; i < finalBufferSize; i++)
            // {
            //     if (finalBuffer_2410[i] < 0x10)
            //         _debugSerial.print("0");
            //     _debugSerial.print(finalBuffer_2410[i], HEX);
            //     _debugSerial.print(" ");
            // }
            // _debugSerial.println();

            // 清除缓冲区
            int bytesToMove = bufferIndex_2410 - (endIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_2410[i] = buffer_2410[endIndex + 4 + i];
            }
            bufferIndex_2410 = bytesToMove;

            return radarStatus_2410;
        }
        else if (startAskIndex != -1 && endAskIndex != -1 && endAskIndex > startAskIndex)
        {
            uint8_t tmp_buffer[(endAskIndex + 4) - startAskIndex];
            for (int i = startAskIndex; i < endAskIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_2410[i];
            }

            // 解析数组
            int lastFrameStart = -1;
            int lastFrameEnd = -1;
            int tmp_bufferSize = sizeof(tmp_buffer);
            for (int i = tmp_bufferSize - 4; i >= 0; i--)
            {
                if (tmp_buffer[i] == 0xFD && tmp_buffer[i + 1] == 0xFC && tmp_buffer[i + 2] == 0xFB && tmp_buffer[i + 3] == 0xFA && lastFrameStart == -1)
                {
                    lastFrameStart = i;
                }
                if (tmp_buffer[i] == 0x04 && tmp_buffer[i + 1] == 0x03 && tmp_buffer[i + 2] == 0x02 && tmp_buffer[i + 3] == 0x01 && lastFrameEnd == -1)
                {
                    lastFrameEnd = i;
                }
            }
            if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd)
            {
                int j = 0;
                for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                {
                    finalBuffer_2410[j++] = tmp_buffer[i];
                }
            }
            else
            {
                // 如果帧不完整或太短，则清空finalBuffer_2410
                memset(finalBuffer_2410, 0, sizeof(finalBuffer_2410));
                // return HLKModuleConfig::AskStatus_2410::Error;
                return radarStatus_2410;
            }
            // 解析数组结束

            // 获取数组长度
            int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

            // _debugSerial.print("nature: ");
            // for (int i = 0; i < sizeof(tmp_buffer); i++)
            // {
            //     if (tmp_buffer[i] < 0x10)
            //         _debugSerial.print("0");
            //     _debugSerial.print(tmp_buffer[i], HEX);
            //     _debugSerial.print(" ");
            // }
            // _debugSerial.println();

            _debugSerial.print("Payload: ");
            for (int i = 0; i < finalBufferSize; i++)
            {
                if (finalBuffer_2410[i] < 0x10)
                    _debugSerial.print("0");
                _debugSerial.print(finalBuffer_2410[i], HEX);
                _debugSerial.print(" ");
            }
            _debugSerial.println();

            // 清除缓冲区
            int bytesToMove = bufferIndex_2410 - (endAskIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_2410[i] = buffer_2410[endAskIndex + 4 + i];
            }
            bufferIndex_2410 = bytesToMove;

            if (finalBuffer_2410[8] == 0)
            {
                // return HLKModuleConfig::AskStatus_2410::Success;
                return radarStatus_2410;
            }
            else
            {
                // return HLKModuleConfig::AskStatus_2410::Error;
                return radarStatus_2410;
            }
        }
    }
    return radarStatus_2410;
}

HLKModuleConfig::DataResult_2410 HLKModuleConfig::sendCommand_2410(const byte *sendData, int sendDataLength)
{
    bufferIndex_2410 = 0; // 重置缓冲区索引
    HLKModuleConfig::DataResult_2410 dataResult_2410;

    /*
        应该是写一个while true用户发送指令的时候就进入，一直循环接收，直到接收到符合要求的回复包再退出循环，如果超过2秒没有
        回复，那就再次发送一次，再等待两秒。最多三次，退出循环。返回结果
    */
    int tryTimes = 0;                        // 重试次数
    unsigned long startTime = millis();      // 记录开始时间
    unsigned long lastSendTime = 0;          // 记录上一次发送的时间
    const unsigned long sendInterval = 1000; // 发送间隔，1000ms即1秒
                                             // 清空缓冲区
    memset(buffer_2410, 0, sizeof(buffer_2410));
    bufferIndex_2410 = 0;
    while (true)
    {
        if (millis() - lastSendTime >= sendInterval)
        {
            // 串口发送使能指令
            _serial.write(sendData, sendDataLength);
            tryTimes++;
            lastSendTime = millis(); // 更新发送时间
            if (_debugSerial.isListening())
            {
                _debugSerial.println("times: " + String(tryTimes));
            }
        }

        // 从串口读取数据并存储到buffer中
        while (_serial.available() && bufferIndex_2410 < BUFFER_SIZE)
        {
            buffer_2410[bufferIndex_2410] = _serial.read();
            bufferIndex_2410++;

            // 检查是否收到完整的帧
            int startIndex = findSequence(buffer_2410, bufferIndex_2410, frameAskStart_2410, 4);
            int endIndex = findSequence(buffer_2410, bufferIndex_2410, frameAskEnd_2410, 4);

            // static uint8_t finalBuffer_2410[128];
            uint8_t *finalBuffer_2410 = nullptr;
            // 接收到完整帧
            if (startIndex != -1 && endIndex != -1 && endIndex > startIndex)
            {
                uint8_t tmp_buffer[(endIndex + 4) - startIndex];
                if (_debugSerial.isListening())
                {
                    _debugSerial.println("start: " + String(startIndex) + "  end: " + String(endIndex));
                }
                int j = 0;
                for (int i = startIndex; i < endIndex + 4; i++)
                {
                    tmp_buffer[j++] = buffer_2410[i];
                }
                // 解析数组
                int lastFrameStart = -1;
                int lastFrameEnd = -1;
                int tmp_bufferSize = sizeof(tmp_buffer);
                for (int i = tmp_bufferSize - 4; i >= 0; i--)
                {
                    if (tmp_buffer[i] == 0xFD && tmp_buffer[i + 1] == 0xFC && tmp_buffer[i + 2] == 0xFB && tmp_buffer[i + 3] == 0xFA && lastFrameStart == -1)
                    {
                        lastFrameStart = i;
                    }
                    if (tmp_buffer[i] == 0x04 && tmp_buffer[i + 1] == 0x03 && tmp_buffer[i + 2] == 0x02 && tmp_buffer[i + 3] == 0x01 && lastFrameEnd == -1)
                    {
                        lastFrameEnd = i;
                    }
                }

                finalBuffer_2410 = new uint8_t[(endIndex + 4) - startIndex];
                if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd)
                {
                    int j = 0;
                    for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                    {
                        finalBuffer_2410[j++] = tmp_buffer[i];
                    }
                    // _debugSerial.println("finnsh!!!");
                    dataResult_2410.length = (endIndex + 4) - startIndex;
                    dataResult_2410.resultBuffer = finalBuffer_2410;

                    if (_debugSerial.isListening())
                    {
                        _debugSerial.println("resultBuffer: ");

                        for (int i = 0; i < dataResult_2410.length; i++)
                        {
                            if (dataResult_2410.resultBuffer[i] < 0x10)
                                _debugSerial.print("0");
                            _debugSerial.print(dataResult_2410.resultBuffer[i], HEX); // 打印每一个byte，您可以根据需要调整格式
                            _debugSerial.print(" ");
                        }
                        _debugSerial.println("");

                        _debugSerial.println("length: " + String(dataResult_2410.length));
                    }
                    return dataResult_2410;
                }
                else
                {
                    // 如果帧不完整或太短，则清空finalBuffer_2410
                    memset(finalBuffer_2410, -1, sizeof(finalBuffer_2410));
                    if (_debugSerial.isListening())
                    {
                        _debugSerial.println("error!!!");
                    }
                    dataResult_2410.length = -1;
                    return dataResult_2410;
                }
                // 解析数组结束

                // 获取数组长度
                int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

                // 清除缓冲区
                int bytesToMove = bufferIndex_2410 - (endIndex + 4);
                for (int i = 0; i < bytesToMove; i++)
                {
                    buffer_2410[i] = buffer_2410[endIndex + 4 + i];
                }
                bufferIndex_2410 = bytesToMove;
            }
        }

        if (millis() - startTime > 6000 || tryTimes > 6) // 超过3秒等待时间
        {
            // return HLKModuleConfig::AskStatus_2410::Error; // 超时，未成功进入AT模式
            dataResult_2410.length = -1;
            return dataResult_2410;
        }
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::enableConfigMode_2410()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand_2410(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return HLKModuleConfig::AskStatus_2410::Success;
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::disableConfigMode_2410()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand_2410(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return HLKModuleConfig::AskStatus_2410::Success;
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

String HLKModuleConfig::getVersion_2410()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA0, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            char versionBuffer[32];
            sprintf(versionBuffer, "V%d.%02d.%06d",
                    dataResult.resultBuffer[11],
                    dataResult.resultBuffer[12],
                    (dataResult.resultBuffer[13] << 24) | (dataResult.resultBuffer[14] << 16) |
                        (dataResult.resultBuffer[15] << 8) | dataResult.resultBuffer[16]);

            disableConfigMode_2410();

            return versionBuffer;
        }
        else
        {
            disableConfigMode_2410();
            return "Get Version Error!";
        }
    }
    else
    {
        return "Into AT Mode Error!";
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setDetectionDistance_2410(int distance, int times)
{
    if (distance >= 1 && distance < 9 && times > 0)
    {
        HLKModuleConfig::DataResult_2410 dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x14, 0x00, 0x60, 0x00,
                           0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
                           0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
                           0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
                           0x04, 0x03, 0x02, 0x01};
        sendData[10] = distance;
        sendData[16] = distance;

        // 赋值 times，小端在前
        sendData[22] = times & 0xFF;         // 低字节
        sendData[23] = (times >> 8) & 0xFF;  // 次低字节
        sendData[24] = (times >> 16) & 0xFF; // 次高字节
        sendData[25] = (times >> 24) & 0xFF; // 高字节

        int sendDataLength = sizeof(sendData);

        if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
        {
            dataResult = sendCommand_2410(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode_2410();

                return HLKModuleConfig::AskStatus_2410::Success;
            }
            else
            {
                disableConfigMode_2410();
                return HLKModuleConfig::AskStatus_2410::Error;
            }
        }
        else
        {
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setGatePower_2410(int gate, int movePower, int staticPower)
{
    if (gate >= 1 && gate < 9 && movePower > 0 && staticPower > 0 && movePower <= 100 && staticPower <= 100)
    {
        HLKModuleConfig::DataResult_2410 dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x14, 0x00, 0x64, 0x00,
                           0x00, 0x00,
                           0x08, 0x00, 0x00, 0x00,
                           0x01, 0x00,
                           0x08, 0x00, 0x00, 0x00,
                           0x02, 0x00,
                           0x05, 0x00, 0x00, 0x00,
                           0x04, 0x03, 0x02, 0x01};
        sendData[10] = gate;

        sendData[16] = movePower & 0xFF;         // 低字节
        sendData[17] = (movePower >> 8) & 0xFF;  // 次低字节
        sendData[18] = (movePower >> 16) & 0xFF; // 次高字节
        sendData[19] = (movePower >> 24) & 0xFF; // 高字节

        sendData[22] = staticPower & 0xFF;         // 低字节
        sendData[23] = (staticPower >> 8) & 0xFF;  // 次低字节
        sendData[24] = (staticPower >> 16) & 0xFF; // 次高字节
        sendData[25] = (staticPower >> 24) & 0xFF; // 高字节

        int sendDataLength = sizeof(sendData);

        if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
        {
            dataResult = sendCommand_2410(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode_2410();

                return HLKModuleConfig::AskStatus_2410::Success;
            }
            else
            {
                disableConfigMode_2410();
                return HLKModuleConfig::AskStatus_2410::Error;
            }
        }
        else
        {
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::RadarStatus_2410 HLKModuleConfig::getConfig_2410()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    HLKModuleConfig::RadarStatus_2410 radarStatus_2410;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x61, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus_2410.detectionDistance = dataResult.resultBuffer[11];
            radarStatus_2410.moveSetDistance = dataResult.resultBuffer[12];
            radarStatus_2410.staticSetDistance = dataResult.resultBuffer[13];
            int j = 0;
            for (int i = 14; i < 23; i++)
            {
                radarStatus_2410.radarMovePower.moveGate[j++] = dataResult.resultBuffer[i];
            }
            j = 0;
            for (int i = 23; i < 32; i++)
            {
                radarStatus_2410.radarStaticPower.staticGate[j++] = dataResult.resultBuffer[i];
            }

            int noTargrtduration = dataResult.resultBuffer[32] | (dataResult.resultBuffer[32] << 8);
            radarStatus_2410.noTargrtduration = noTargrtduration;

            disableConfigMode_2410();

            return radarStatus_2410;
        }
        else
        {
            disableConfigMode_2410();
            return radarStatus_2410;
        }
    }
    else
    {
        return radarStatus_2410;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setResolution_2410(int resolution)
{
    if (resolution == 0 || resolution == 1)
    {
        HLKModuleConfig::DataResult_2410 dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
        if (resolution == 1)
        {
            sendData[8] = 1;
        }
        int sendDataLength = sizeof(sendData);

        if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
        {
            dataResult = sendCommand_2410(sendData, sendDataLength);

            if (dataResult.resultBuffer[8] == 0x00)
            {
                rebootRadar();
                return HLKModuleConfig::AskStatus_2410::Success;
            }
            else
            {
                rebootRadar();
                return HLKModuleConfig::AskStatus_2410::Error;
            }
        }
        else
        {
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::RadarStatus_2410 HLKModuleConfig::getResolution_2410()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    HLKModuleConfig::RadarStatus_2410 radarStatus_2410;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xAB, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus_2410.resolution = dataResult.resultBuffer[10];

            disableConfigMode_2410();

            return radarStatus_2410;
        }
        else
        {
            disableConfigMode_2410();
            return radarStatus_2410;
        }
    }
    else
    {
        return radarStatus_2410;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::rebootRadar()
{

    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA3, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            return HLKModuleConfig::AskStatus_2410::Success;
        }
        else
        {
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::refactoryRadar()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA2, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            rebootRadar();
            return HLKModuleConfig::AskStatus_2410::Success;
        }
        else
        {
            rebootRadar();
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::enableEngineeringModel()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x62, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode_2410();
            return HLKModuleConfig::AskStatus_2410::Success;
        }
        else
        {
            disableConfigMode_2410();
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

HLKModuleConfig::AskStatus_2410 HLKModuleConfig::disableEngineeringModel()
{
    HLKModuleConfig::DataResult_2410 dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x63, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode_2410() == HLKModuleConfig::AskStatus_2410::Success)
    {
        dataResult = sendCommand_2410(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode_2410();
            return HLKModuleConfig::AskStatus_2410::Success;
        }
        else
        {
            disableConfigMode_2410();
            return HLKModuleConfig::AskStatus_2410::Error;
        }
    }
    else
    {
        return HLKModuleConfig::AskStatus_2410::Error;
    }
}

int HLKModuleConfig::findSequence(byte *arr, int arrLen, const byte *seq, int seqLen)
{
    for (int i = 0; i < arrLen - seqLen + 1; i++)
    {
        bool found = true;
        for (int j = 0; j < seqLen; j++)
        {
            if (arr[i + j] != seq[j])
            {
                found = false;
                break;
            }
        }
        if (found)
            return i;
    }
    return -1;
}