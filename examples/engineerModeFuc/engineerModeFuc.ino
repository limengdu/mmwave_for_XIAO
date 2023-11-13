#if defined(ARDUINO_SEEED_XIAO_NRF52840_SENSE) || defined(ARDUINO_SEEED_XIAO_NRF52840)
#error "XIAO nRF52840 please use the non-mbed-enable version."
#endif

#include <SoftwareSerial.h>
#include <mmwave_for_xiao.h>

// Define the SoftwareSerial object, D2 as RX, D3 as TX, connect to the serial port of the mmwave sensor
SoftwareSerial COMSerial(D2, D3);

// Creates a global Serial object for printing debugging information
#define ShowSerial Serial

// Initialising the radar configuration
// Seeed_HSP24 xiao_config(COMSerial, ShowSerial);
Seeed_HSP24 xiao_config(COMSerial);

Seeed_HSP24::RadarStatus radarStatus;

void setup() {
  COMSerial.begin(9600);
  ShowSerial.begin(9600);
  delay(500);

  ShowSerial.println("Programme Starting!");

  xiao_config.enableEngineeringModel();
}

void loop() {
  int retryCount = 0;
  const int MAX_RETRIES = 10;  // Maximum number of retries to prevent infinite loops

  //Get radar status
  do {
    radarStatus = xiao_config.getStatus();
    retryCount++;
  } while (radarStatus.targetStatus == Seeed_HSP24::TargetStatus::ErrorFrame && retryCount < MAX_RETRIES);

  //Parses radar status and prints results from debug serial port
  if (radarStatus.targetStatus != Seeed_HSP24::TargetStatus::ErrorFrame) {
    ShowSerial.print("Status: " + String(targetStatusToString(radarStatus.targetStatus)) + "  ----   ");
    ShowSerial.println("Distance: " + String(radarStatus.distance) + "  Mode: " + String(radarStatus.radarMode));
    
    if (radarStatus.radarMode == 1) {
      ShowSerial.print("Move:");
      for (int i = 0; i < 9; i++) {
        ShowSerial.print(" " + String(radarStatus.radarMovePower.moveGate[i]) + ",");
      }
      ShowSerial.println("");
      ShowSerial.print("Static:");
      for (int i = 0; i < 9; i++) {
        ShowSerial.print(" " + String(radarStatus.radarStaticPower.staticGate[i]) + ",");
      }
      ShowSerial.println("");
      ShowSerial.println("Photosensitive: " + String(radarStatus.photosensitive));
    }
  }
  delay(100);
}

// Parsing the acquired radar status
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
