#ifndef Seeed_HSP24_h
#define Seeed_HSP24_h

#include <SoftwareSerial.h> // 包括软件串口库
#include <Arduino.h>

#define BUFFER_SIZE 256     // 串口缓冲区大小

class Seeed_HSP24
{
    public:
        // Seeed_HSP24(SoftwareSerial &serial, HardwareSerial &debugSerial); // 增加调试口的构造函数
        Seeed_HSP24(Stream &serial);                                        // 直接与模块通信的构造函数
        explicit Seeed_HSP24(Stream &serial, Stream &debugSerial); // 增加调试口的构造函数
        int enterATMode();                                                    // 让模块进入at模式，成功：1 失败：0
        int exitATMode();                                                     // 让模块退出at模式,成功：1 失败：0
        int getVer();                                                         // 获取版本号

        static const int FRAME_START_SIZE = 4;
        static const int FRAME_END_SIZE = 4;

        static const byte frameStart[FRAME_START_SIZE];
        static const byte frameEnd[FRAME_END_SIZE];
        static const byte frameAskStart[FRAME_START_SIZE];
        static const byte frameAskEnd[FRAME_END_SIZE];

        struct RadarMovePower // 每个运动距离门的能量值
        {
            int moveGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
        };

        struct RadarStaticPower // 每个静止距离门的能量值
        {
            int staticGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
        };

        //  定义TargetStatus枚举类
        enum class AskStatus : byte
        {
            Success = 0x00, // 成功
            Error = 0x01,   // 失败
        };

        //  定义TargetStatus枚举类
        enum class TargetStatus : byte
        {
            NoTarget = 0x00,     // 没有目标
            MovingTarget = 0x01, // 移动目标
            StaticTarget = 0x02, // 静止目标
            BothTargets = 0x03,  // 可以理解为运动，指的是设置的运动和静止阈值都达到了设定值以上
            ErrorFrame = 0x04 // 状态获取失败
        };

        // 定义RadarStatus结构体
        struct RadarStatus
        {
            TargetStatus targetStatus = TargetStatus::ErrorFrame; // 雷达的目标状态
            int distance = -1;                                                                                // 雷达的目标距离，单位mm
            int moveSetDistance = -1;                                                                         // 雷达的运动探测距离门数量，一般不用配置
            int staticSetDistance = -1;                                                                       // 雷达的静止探测距离门数量，一般不用配置
            int detectionDistance = -1;                                                                       // 雷达的最远探测距离门
            int resolution = -1;                                                                              // 雷达的距离门分辨率
            int noTargrtduration = -1;                                                                        // 无人持续时间
            int radarMode = -1;                                                                          // 用来区分模块处于基本上报模式（2）还是工程上报模式（1）
            RadarMovePower radarMovePower;                                              // 运动能量值
            RadarStaticPower radarStaticPower;                                          // 静止能量值
            int photosensitive = -1;                                                                          // 光敏 0-255
        };

        // 用来返回下发指令的结果
        struct DataResult
        {
            byte *resultBuffer; // 指向动态分配的数组
            int length;         // 数组的长度
        };

        // 获取雷达感应状态
        RadarStatus getStatus();
        // 发送指令
        DataResult sendCommand(const byte* sendData, int sendDataLength);
        // 使能配置指令
        AskStatus enableConfigMode();
        // 结束使能配置指令
        AskStatus disableConfigMode();
        // 读取雷达固件版本
        String getVersion();
        // 配置最大距离门和无人持续时间
        AskStatus setDetectionDistance(int distance,int times);
        // 配置距离门灵敏度
        AskStatus setGatePower(int gate,int movePower, int staticPower);
        // 读取参数命令
        RadarStatus getConfig();
        // 设置距离门分辨率,1为0.25M，0为0.75M。默认0.75M
        AskStatus setResolution(int resolution);
        // 获取距离门分辨率,1为0.25M，0为0.75M。默认0.75M
        RadarStatus getResolution();
        // 重启指令
        AskStatus rebootRadar();
        // 回复出厂指令
        AskStatus refactoryRadar();
        // 使能工程模式
        AskStatus enableEngineeringModel();
        // 退出工程模式
        AskStatus disableEngineeringModel();

    private:
        // 可以在这里定义私有变量或方法
        // HardwareSerial &_serial; // 成员变量，用于在库中引用Serial对象
        Stream* _serial;
        Stream* _debugSerial;
        char buffer[BUFFER_SIZE]; // 将 buffer 定义为成员变量
        unsigned long receiveStartTime;
        const unsigned long RECEIVE_TIMEOUT = 100; // 接收超时时间，单位毫秒
        int bufferIndex;
        int isInATMode; // 用来确认是不是在AT模式下
        int checkBuffer();
        int sendATCommand(String command);
        int sendATCommandWithExit(String command);

        byte buffer_hsp24[BUFFER_SIZE];

        int bufferIndex_hsp24;
        int findSequence(byte *arr, int arrLen, const byte *seq, int seqLen);
        
};



#endif