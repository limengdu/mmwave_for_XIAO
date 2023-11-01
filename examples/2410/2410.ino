#include <SoftwareSerial.h>
#include <HLKModuleConfig.h>

// 定义SoftwareSerial对象，RX为8号引脚，TX为9号引脚
SoftwareSerial debugSerial(7, 8);
// 创建全局Serial对象和SoftwareSerial对象，连接2410的串口，RX为0号引脚，TX为1号引脚
HardwareSerial& hardwareSerial = Serial;
// 初始化雷达配置
HLKModuleConfig hlk_config(hardwareSerial, debugSerial);
//HLKModuleConfig hlk_config(hardwareSerial);
HLKModuleConfig::RadarStatus_2410 radarStatus;

void setup() {
  Serial.begin(115200);
  debugSerial.begin(56000);
}

void loop() {
  int retryCount = 0;
  const int MAX_RETRIES = 10;  // 最大重试次数，防止无限循环

  //获取雷达状态
  do {
    radarStatus = hlk_config.getStatus_2410();
    retryCount++;
  } while (radarStatus.distance == -1 && retryCount < MAX_RETRIES);

  //解析雷达状态，并把结果从调试串口打印出来
  if (radarStatus.distance != -1) {
    debugSerial.print("Status: " + String(targetStatusToString(radarStatus.targetStatus)) + "  ----   ");
    debugSerial.println("Distance: " + String(radarStatus.distance) + "  Mode: " + String(radarStatus.radarMode_2410));
    debugSerial.print("Move: ");
    if (radarStatus.radarMode_2410 == 1) {
      for (int i = 0; i < 9; i++) {
        debugSerial.print(String(radarStatus.radarMovePower.moveGate[i]) + "  ,");
      }
      debugSerial.println("");
      debugSerial.print("Static: ");
      for (int i = 0; i < 9; i++) {
        debugSerial.print(String(radarStatus.radarStaticPower.staticGate[i]) + "  ,");
      }
      debugSerial.println("");
      debugSerial.println("Photosensitive: " + String(radarStatus.photosensitive));
    }
  }
  delay(200);
}

// 解析获取到的雷达状态
const char* targetStatusToString(HLKModuleConfig::TargetStatus_2410 status) {
  switch (status) {
    case HLKModuleConfig::TargetStatus_2410::NoTarget:
      return "NoTarget";
    case HLKModuleConfig::TargetStatus_2410::MovingTarget:
      return "MovingTarget";
    case HLKModuleConfig::TargetStatus_2410::StaticTarget:
      return "StaticTarget";
    case HLKModuleConfig::TargetStatus_2410::BothTargets:
      return "BothTargets";
    default:
      return "Unknown";
  }
}
