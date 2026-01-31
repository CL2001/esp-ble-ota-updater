#include <Arduino.h>
#include "bluetooth.hpp"

#ifndef FW_VERSION
#define FW_VERSION "0.0.0"
#endif

int l = 0;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Serial Begin");
  Serial.println();

  Serial.print("Firmware v");
  Serial.print(FW_VERSION);
  Serial.print(".");

  Hardware::set(RealHardware::instance());
  Hardware::get().bluetoothInit();
}

void loop(void) {
  std::string msg = Hardware::get().getBluetoothMessage();
  std::string msg2 = msg + " was received by the esp32 v2 + loop " + std::to_string(l);
  Hardware::get().sendBluetoothMessage(msg2);
  l++;
}
