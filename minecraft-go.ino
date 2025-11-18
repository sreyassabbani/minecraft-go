#include <Arduino.h>
#include <utils/general.h>
#include <utils/algebra.h>
using namespace algebra;

const int L = 10;
bool blocks[L * L * L];

// 480x320sdf
int map[480 * 320];

// (i, j, k)
// projected = (i, j, k) - n.scale((camera_center - (i, j, k)).dot(n) / n.magsq());

void setup() {
  // put your setup code here, to run once:

  Vector<int> s({1, 2, 3, 3});
  Vector<int> t({-1, 02, 32, 0});

  println(s);

  println(s + t);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}