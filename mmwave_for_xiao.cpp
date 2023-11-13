#include "mmwave_for_xiao.h"


#define BUFFER_SIZE 256 // 串口缓冲区大小

Seeed_HSP24::Seeed_HSP24(Stream &serial)
    : _serial(&serial),
    _debugSerial(nullptr),
    bufferIndex(0),
    receiveStartTime(0),
    isInATMode(0),
    bufferIndex_hsp24(0)
{
}

Seeed_HSP24::Seeed_HSP24(Stream &serial, Stream &debugSerial)
    : _serial(&serial),
    _debugSerial(&debugSerial),
    bufferIndex(0),
    receiveStartTime(0),
    isInATMode(0),
    bufferIndex_hsp24(0)
{
}

// B35
int Seeed_HSP24::enterATMode()
{

    _serial->print("+++"); // 发送指令+++

    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial->available())
        {
            char receivedChar = _serial->read();
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
                _serial->print('a');
            }
            else if (buffer[0] == 'O' && buffer[1] == 'K')
            { // 进入at模式
                if (_debugSerial != nullptr)
                {
                    _debugSerial->println("Enter AT Mode Success!");
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

int Seeed_HSP24::exitATMode()
{
    _serial->println("at+reconn=1");

    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial->available())
        {
            char receivedChar = _serial->read(); //   接收串口数据
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
                if (_debugSerial != nullptr)
                {
                    _debugSerial->println("Setting Success!");
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 1;
            }
            else
            {
                if (_debugSerial != nullptr)
                {
                    // 发送缓冲区中的数据到另一个串口
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial->print(buffer[i]);
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

int Seeed_HSP24::checkBuffer()
{
    unsigned long startTime = millis(); // 记录开始时间

    while (true)
    {
        // 检查是否有数据可用于接收
        while (_serial->available())
        {
            char receivedChar = _serial->read(); //   接收串口数据
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
                if (_debugSerial != nullptr)
                {
                    _debugSerial->println("Setting Success!");
                }

                // 清空缓冲区和索引
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;

                return 1;
            }
            else
            {
                if (_debugSerial != nullptr)
                {
                    // 发送缓冲区中的数据到另一个串口
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial->print(buffer[i]);
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

int Seeed_HSP24::sendATCommandWithExit(String command)
{
    _serial->println(command);
    int ret = checkBuffer();
    exitATMode();
    return ret;
}

int Seeed_HSP24::sendATCommand(String command)
{
    _serial->println(command);
    int ret = checkBuffer();
    return ret;
}

int Seeed_HSP24::getVer()
{
    enterATMode(); // 进入AT模式
    return sendATCommandWithExit("at+ver=?");
}

Seeed_HSP24::TargetStatus status = Seeed_HSP24::TargetStatus::NoTarget;

// 主动上报
const byte Seeed_HSP24::frameStart[Seeed_HSP24::FRAME_START_SIZE] = {0xF4, 0xF3, 0xF2, 0xF1};
const byte Seeed_HSP24::frameEnd[Seeed_HSP24::FRAME_END_SIZE] = {0xF8, 0xF7, 0xF6, 0xF5};

// 查询返回
const byte Seeed_HSP24::frameAskStart[Seeed_HSP24::FRAME_START_SIZE] = {0xFD, 0xFC, 0xFB, 0xFA};
const byte Seeed_HSP24::frameAskEnd[Seeed_HSP24::FRAME_END_SIZE] = {0x04, 0x03, 0x02, 0x01};

Seeed_HSP24::RadarStatus Seeed_HSP24::getStatus()
{

    bufferIndex_hsp24 = 0; // 重置缓冲区索引
    Seeed_HSP24::RadarStatus radarStatus;
    const int MIN_FRAME_LENGTH_BASE = 23;        // 最小帧长度,基本上报
    const int MIN_FRAME_LENGTH_ENGINEERING = 45; // 最小帧长度,工程上报

    // 从串口读取数据并存储到buffer中
    while (_serial->available() && bufferIndex_hsp24 < BUFFER_SIZE)
    {
        buffer_hsp24[bufferIndex_hsp24] = _serial->read();
        bufferIndex_hsp24++;

        // 检查是否收到完整的帧
        int startIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameStart, 4);
        int endIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameEnd, 4);

        // 接收到完整帧，指的是基础模式
        if ((endIndex + 4 - startIndex) == MIN_FRAME_LENGTH_BASE)
        {
            uint8_t tmp_buffer[(endIndex + 4) - startIndex];
            for (int i = startIndex; i < endIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_hsp24[i];
            }

            int tmp_bufferSize = sizeof(tmp_buffer);
            

            // 判断第七位是工程模式数据还是基本模式数据
            int mode = tmp_buffer[6];
            radarStatus.radarMode = mode;
            if(mode != 2) return radarStatus;
            if(tmp_buffer[7] != 0xAA) return radarStatus;
            if(tmp_buffer[17] != 0x55) return radarStatus;
            if(tmp_buffer[18] != 0x00) return radarStatus;


            // 提取雷达上报状态
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("sensor status: " + String(tmp_buffer[8]));
            }
            status = static_cast<Seeed_HSP24::TargetStatus>(tmp_buffer[8]);
            radarStatus.targetStatus = static_cast<Seeed_HSP24::TargetStatus>(tmp_buffer[8]);

            // 提取目标距离
            int distance = tmp_buffer[15] | (tmp_buffer[16] << 8); // 小端序解析
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("distance: " + String(distance));
            }
            radarStatus.distance = distance;
            
            if (_debugSerial != nullptr)
            {
                _debugSerial->print("nature: ");
                for (int i = 0; i < sizeof(tmp_buffer); i++)
                {
                    if (tmp_buffer[i] < 0x10)
                        _debugSerial->print("0");
                    _debugSerial->print(tmp_buffer[i], HEX);
                    _debugSerial->print(" ");
                }
                _debugSerial->println();
            }

            // 清除缓冲区
            int bytesToMove = bufferIndex_hsp24 - (endIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_hsp24[i] = buffer_hsp24[endIndex + 4 + i];
            }
            bufferIndex_hsp24 = bytesToMove;
        }

        // 检查工程模式下数据帧是否完整
        else if ((endIndex + 4 - startIndex) == MIN_FRAME_LENGTH_ENGINEERING)
        {
            uint8_t tmp_buffer[(endIndex + 4) - startIndex];
            for (int i = startIndex; i < endIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_hsp24[i];
            }
            
            int tmp_bufferSize = sizeof(tmp_buffer);

            int mode = tmp_buffer[6];
            radarStatus.radarMode = mode;
            if(mode != 1) return radarStatus;
            if(tmp_buffer[7] != 0xAA) return radarStatus;
            if(tmp_buffer[39] != 0x55) return radarStatus;
            if(tmp_buffer[40] != 0x00) return radarStatus;

            // 提取雷达上报状态
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("sensor status: " + String(tmp_buffer[8]));
            }
            status = static_cast<Seeed_HSP24::TargetStatus>(tmp_buffer[8]);
            radarStatus.targetStatus = static_cast<Seeed_HSP24::TargetStatus>(tmp_buffer[8]);

            // 提取目标距离
            int distance = tmp_buffer[15] | (tmp_buffer[16] << 8); // 小端序解析
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("distance: " + String(distance));
            }
            radarStatus.distance = distance;

            // 提取距离门数据
            for(int i = 0; i < 9; i++) {
                radarStatus.radarMovePower.moveGate[i] = tmp_buffer[i + 19];
            }
            if (_debugSerial != nullptr)
            {
                _debugSerial->print("move energy:");
                for(int j = 0; j < 9; j++){
                    _debugSerial->print(" " + String(radarStatus.radarMovePower.moveGate[j]) + ",");
                }
                _debugSerial->println("");
            }

            for(int i = 0; i < 9; i++) {
                radarStatus.radarStaticPower.staticGate[i] = tmp_buffer[i + 28];
            }
            if (_debugSerial != nullptr)
            {
                _debugSerial->print("static energy:");
                for(int j = 0; j < 9; j++){
                    _debugSerial->print(" " + String(radarStatus.radarStaticPower.staticGate[j]) + ",");
                }
                _debugSerial->println("");
            }

            // 提取光敏值
            radarStatus.photosensitive = tmp_buffer[37];
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("photosensitive: " + String(radarStatus.photosensitive));
            }
            

            if(_debugSerial != nullptr)
            {
                _debugSerial->print("nature: ");
                for (int i = 0; i < sizeof(tmp_buffer); i++)
                {
                    if (tmp_buffer[i] < 0x10)
                        _debugSerial->print("0");
                    _debugSerial->print(tmp_buffer[i], HEX);
                    _debugSerial->print(" ");
                }
                _debugSerial->println();
            }

            // 清除缓冲区
            int bytesToMove = bufferIndex_hsp24 - (endIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_hsp24[i] = buffer_hsp24[endIndex + 4 + i];
            }
            bufferIndex_hsp24 = bytesToMove;
        }
    }
    return radarStatus;
}

Seeed_HSP24::DataResult Seeed_HSP24::sendCommand(const byte *sendData, int sendDataLength)
{
    bufferIndex_hsp24 = 0; // 重置缓冲区索引
    Seeed_HSP24::DataResult dataResult;

    /*
        应该是写一个while true用户发送指令的时候就进入，一直循环接收，直到接收到符合要求的回复包再退出循环，如果超过2秒没有
        回复，那就再次发送一次，再等待两秒。最多三次，退出循环。返回结果
    */
    int tryTimes = 0;                        // 重试次数
    unsigned long startTime = millis();      // 记录开始时间
    unsigned long lastSendTime = 0;          // 记录上一次发送的时间
    const unsigned long sendInterval = 1000; // 发送间隔，1000ms即1秒
                                             // 清空缓冲区
    memset(buffer_hsp24, 0, sizeof(buffer_hsp24));
    bufferIndex_hsp24 = 0;
    while (true)
    {
        if (millis() - lastSendTime >= sendInterval)
        {
            // 串口发送使能指令
            _serial->write(sendData, sendDataLength);
            tryTimes++;
            lastSendTime = millis(); // 更新发送时间
            if (_debugSerial != nullptr)
            {
                _debugSerial->println("times: " + String(tryTimes));
            }
        }

        // 从串口读取数据并存储到buffer中
        while (_serial->available() && bufferIndex_hsp24 < BUFFER_SIZE)
        {
            buffer_hsp24[bufferIndex_hsp24] = _serial->read();
            bufferIndex_hsp24++;

            // 检查是否收到完整的帧
            int startIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskStart, 4);
            int endIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskEnd, 4);

            // static uint8_t finalBuffer[128];
            uint8_t *finalBuffer = nullptr;
            // 接收到完整帧
            if (startIndex != -1 && endIndex != -1 && endIndex > startIndex)
            {
                uint8_t tmp_buffer[(endIndex + 4) - startIndex];
                if (_debugSerial != nullptr)
                {
                    _debugSerial->println("start: " + String(startIndex) + "  end: " + String(endIndex));
                }
                int j = 0;
                for (int i = startIndex; i < endIndex + 4; i++)
                {
                    tmp_buffer[j++] = buffer_hsp24[i];
                }

                dataResult.length = sizeof(tmp_buffer);
                dataResult.resultBuffer = tmp_buffer;

                if (_debugSerial != nullptr)
                {
                    _debugSerial->println("resultBuffer: ");

                    for (int i = 0; i < dataResult.length; i++)
                    {
                        if (dataResult.resultBuffer[i] < 0x10)
                            _debugSerial->print("0");
                        _debugSerial->print(dataResult.resultBuffer[i], HEX); // 打印每一个byte，您可以根据需要调整格式
                        _debugSerial->print(" ");
                    }
                    _debugSerial->println("");

                    _debugSerial->println("length: " + String(dataResult.length));
                }
                return dataResult;

                // 清除缓冲区
                int bytesToMove = bufferIndex_hsp24 - (endIndex + 4);
                for (int i = 0; i < bytesToMove; i++)
                {
                    buffer_hsp24[i] = buffer_hsp24[endIndex + 4 + i];
                }
                bufferIndex_hsp24 = bytesToMove;
            }
        }

        
        if (millis() - startTime > 6000 || tryTimes > 6) // 超过3秒等待时间
        {
            // return Seeed_HSP24::AskStatus::Error; // 超时，未成功进入AT模式
            dataResult.length = -1;
            return dataResult;
        }
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::enableConfigMode()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return Seeed_HSP24::AskStatus::Success;
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::disableConfigMode()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return Seeed_HSP24::AskStatus::Success;
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

String Seeed_HSP24::getVersion()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA0, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            char versionBuffer[32];
            sprintf(versionBuffer, "V%d.%02d.%06d",
                    dataResult.resultBuffer[11],
                    dataResult.resultBuffer[12],
                    (dataResult.resultBuffer[13] << 24) | (dataResult.resultBuffer[14] << 16) |
                        (dataResult.resultBuffer[15] << 8) | dataResult.resultBuffer[16]);

            disableConfigMode();

            return versionBuffer;
        }
        else
        {
            disableConfigMode();
            return "Get Version Error!";
        }
    }
    else
    {
        return "Into AT Mode Error!";
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setDetectionDistance(int distance, int times)
{
    if (distance >= 1 && distance < 9 && times > 0)
    {
        Seeed_HSP24::DataResult dataResult;
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

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode();

                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setGatePower(int gate, int movePower, int staticPower)
{
    if (gate >= 1 && gate < 9 && movePower > 0 && staticPower > 0 && movePower <= 100 && staticPower <= 100)
    {
        Seeed_HSP24::DataResult dataResult;
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

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode();

                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::RadarStatus Seeed_HSP24::getConfig()
{
    Seeed_HSP24::DataResult dataResult;
    Seeed_HSP24::RadarStatus radarStatus;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x61, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus.detectionDistance = dataResult.resultBuffer[11];
            radarStatus.moveSetDistance = dataResult.resultBuffer[12];
            radarStatus.staticSetDistance = dataResult.resultBuffer[13];
            int j = 0;
            for (int i = 14; i < 23; i++)
            {
                radarStatus.radarMovePower.moveGate[j++] = dataResult.resultBuffer[i];
            }
            j = 0;
            for (int i = 23; i < 32; i++)
            {
                radarStatus.radarStaticPower.staticGate[j++] = dataResult.resultBuffer[i];
            }

            int noTargrtduration = dataResult.resultBuffer[32] | (dataResult.resultBuffer[32] << 8);
            radarStatus.noTargrtduration = noTargrtduration;

            disableConfigMode();

            return radarStatus;
        }
        else
        {
            disableConfigMode();
            return radarStatus;
        }
    }
    else
    {
        return radarStatus;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setResolution(int resolution)
{
    if (resolution == 0 || resolution == 1)
    {
        Seeed_HSP24::DataResult dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
        if (resolution == 1)
        {
            sendData[8] = 1;
        }
        int sendDataLength = sizeof(sendData);

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[8] == 0x00)
            {
                rebootRadar();
                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                rebootRadar();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::RadarStatus Seeed_HSP24::getResolution()
{
    Seeed_HSP24::DataResult dataResult;
    Seeed_HSP24::RadarStatus radarStatus;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xAB, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus.resolution = dataResult.resultBuffer[10];

            disableConfigMode();

            return radarStatus;
        }
        else
        {
            disableConfigMode();
            return radarStatus;
        }
    }
    else
    {
        return radarStatus;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::rebootRadar()
{

    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA3, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::refactoryRadar()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA2, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            rebootRadar();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            rebootRadar();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::enableEngineeringModel()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x62, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::disableEngineeringModel()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x63, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}


// 返回seq数组在arr数组的第一个元素的索引值
int Seeed_HSP24::findSequence(byte *arr, int arrLen, const byte *seq, int seqLen)
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

