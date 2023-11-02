#include <SoftwareSerial.h>
#include <mmwave_for_xiao.h>

// 定义SoftwareSerial对象，RX为8号引脚，TX为9号引脚
SoftwareSerial hardwareSerial(D2, D3);

// 创建全局Serial对象和SoftwareSerial对象，连接2410的串口，RX为0号引脚，TX为1号引脚
#define debugSerial Serial

// 初始化雷达配置
Seeed_HSP24 xiao_config(hardwareSerial, debugSerial);
//Seeed_HSP24 xiao_config(hardwareSerial);
Seeed_HSP24::RadarStatus radarStatus;

void setup() {
  Serial.begin(115200);
  debugSerial.begin(115200);
}

void loop() {
  int retryCount = 0;
  const int MAX_RETRIES = 10;  // 最大重试次数，防止无限循环

  //获取雷达状态
  do {
    radarStatus = xiao_config.getStatus();
    retryCount++;
//    debugSerial.println(radarStatus.distance);
  } while (radarStatus.distance == -1 && retryCount < MAX_RETRIES);

  //解析雷达状态，并把结果从调试串口打印出来
  if (radarStatus.distance != -1) {
    debugSerial.print("Status: " + String(targetStatusToString(radarStatus.targetStatus)) + "  ----   ");
    debugSerial.println("Distance: " + String(radarStatus.distance) + "  Mode: " + String(radarStatus.radarMode));
    debugSerial.print("Move: ");
    if (radarStatus.radarMode == 1) {
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
const char* targetStatusToString(Seeed_HSP24::TargetStatus status) {
  switch (status) {
    case Seeed_HSP24::TargetStatus::NoTarget:
      return "NoTarget";
    case Seeed_HSP24::TargetStatus::MovingTarget:
      return "MovingTarget";
    case Seeed_HSP24::TargetStatus::StaticTarget:
      return "StaticTarget";
    case Seeed_HSP24::TargetStatus::BothTargets:
      return "BothTargets";
    default:
      return "Unknown";
  }
}
