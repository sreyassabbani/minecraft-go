#include <Arduino.h>
#include <utils/general.h>

const int L = 10;
bool blocks[L * L * L];

// 480x320
int map[480 * 320];

// (i, j, k)
// projected = (i, j, k) - n.scale((camera_center - (i, j, k)).dot(n) / n.magsq());

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}