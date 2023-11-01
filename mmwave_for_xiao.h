#ifndef HLKModuleConfig_h
#define HLKModuleConfig_h

#include <Arduino.h>        // 包括Arduino核心库，以便使用Arduino函数
#include <SoftwareSerial.h> // 包括软件串口库
#define BUFFER_SIZE 256     // 串口缓冲区大小

class HLKModuleConfig
{
public:
    HLKModuleConfig(HardwareSerial &serial);                              // 直接与模块通信的构造函数
    HLKModuleConfig(HardwareSerial &serial, SoftwareSerial &debugSerial); // 增加调试口的构造函数
    int enterATMode();                                                    // 让模块进入at模式，成功：1 失败：0
    int exitATMode();                                                     // 让模块退出at模式,成功：1 失败：0
    int getVer();                                                         // 获取版本号
    int setNetwork(String ssid, String password);                         // 设置模块联网，作为STA，成功：1 失败：0

    // 2410
    struct RadarMovePower_2410 // 每个运动距离门的能量值
    {
        int moveGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    };

    struct RadarStaticPower_2410 // 每个静止距离门的能量值
    {
        int staticGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    };

    //  定义TargetStatus枚举类
    enum class AskStatus_2410 : byte
    {
        Success = 0x00, // 成功
        Error = 0x01,   // 失败
    };

    //  定义TargetStatus枚举类
    enum class TargetStatus_2410 : byte
    {
        NoTarget = 0x00,     // 没有目标
        MovingTarget = 0x01, // 移动目标
        StaticTarget = 0x02, // 静止目标
        BothTargets = 0x03,
        ErrorFrame = 0x04 // 状态获取失败
    };

    // 定义RadarStatus结构体
    struct RadarStatus_2410
    {
        HLKModuleConfig::TargetStatus_2410 targetStatus = HLKModuleConfig::TargetStatus_2410::ErrorFrame; // 雷达的目标状态
        int distance = -1;                                                                                // 雷达的目标距离，单位mm
        int moveSetDistance = -1;                                                                         // 雷达的运动探测距离门数量，一般不用配置
        int staticSetDistance = -1;                                                                       // 雷达的静止探测距离门数量，一般不用配置
        int detectionDistance = -1;                                                                       // 雷达的最远探测距离门
        int resolution = -1;                                                                              // 雷达的距离门分辨率
        int noTargrtduration = -1;                                                                        // 无人持续时间
        int radarMode_2410 = -1;                                                                          // 用来区分模块处于基本上报模式（2）还是工程上报模式（1）
        HLKModuleConfig::RadarMovePower_2410 radarMovePower;                                              // 运动能量值
        HLKModuleConfig::RadarStaticPower_2410 radarStaticPower;                                          // 静止能量值
        int photosensitive = -1;                                                                          // 光敏 0-255
    };

    // 用来返回下发指令的结果
    struct DataResult_2410
    {
        byte *resultBuffer; // 指向动态分配的数组
        int length;         // 数组的长度
    };

    // 获取雷达感应状态
    RadarStatus_2410 getStatus_2410();
    // 发送指令
    HLKModuleConfig::DataResult_2410 HLKModuleConfig::sendCommand_2410(const byte* sendData, int sendDataLength);
    // 使能配置指令
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::enableConfigMode_2410();
    // 结束使能配置指令
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::disableConfigMode_2410();
    // 读取雷达固件版本
    String HLKModuleConfig::getVersion_2410();
    // 配置最大距离门和无人持续时间
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setDetectionDistance_2410(int distance,int times);
    // 配置距离门灵敏度
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setGatePower_2410(int gate,int movePower, int staticPower);
    // 读取参数命令
    HLKModuleConfig::RadarStatus_2410 HLKModuleConfig::getConfig_2410();
    // 设置距离门分辨率,1为0.25M，0为0.75M。默认0.75M
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::setResolution_2410(int resolution);
    // 获取距离门分辨率,1为0.25M，0为0.75M。默认0.75M
    HLKModuleConfig::RadarStatus_2410 HLKModuleConfig::getResolution_2410();
    // 重启指令
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::rebootRadar();
    // 回复出厂指令
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::refactoryRadar();
    // 使能工程模式
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::enableEngineeringModel();
    // 退出工程模式
    HLKModuleConfig::AskStatus_2410 HLKModuleConfig::disableEngineeringModel();

private:
    // 可以在这里定义私有变量或方法
    HardwareSerial &_serial; // 成员变量，用于在库中引用Serial对象
    SoftwareSerial &_debugSerial;
    char buffer[BUFFER_SIZE]; // 将 buffer 定义为成员变量
    unsigned long receiveStartTime;
    const unsigned long RECEIVE_TIMEOUT = 100; // 接收超时时间，单位毫秒
    int bufferIndex;
    int isInATMode; // 用来确认是不是在AT模式下
    int checkBuffer();
    int sendATCommand(String command);
    int sendATCommandWithExit(String command);

    // 2410
    byte buffer_2410[BUFFER_SIZE];

    int bufferIndex_2410;
    static const byte frameStart_2410[];
    static const byte frameEnd_2410[];
    static const byte frameAskStart_2410[];
    static const byte frameAskEnd_2410[];
    static const int FRAME_START_SIZE_2410 = 4;
    static const int FRAME_END_SIZE_2410 = 4;
    int findSequence(byte *arr, int arrLen, const byte *seq, int seqLen);
};

#endif