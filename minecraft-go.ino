#include <Arduino.h>
#include "utils/general.h"
#include "utils/algebra.h"
#include "utils/gyro.h"

using namespace algebra;


// const int L = 10;
// bool blocks[L * L * L];

// 480x320sdf
// int map[480 * 320];

// (i, j, k)
// projected = (i, j, k) - n.scale((camera_center - (i, j, k)).dot(n) / n.magsq());

void setup() {
  // put your setup code here, to run once:

  // Vector<3> a { 1, 2, 3 };
  // Vector<3> b { -2, 4, 0 };

  // auto c = a + b;

  Serial.begin(9600);
  Serial.print("HI");
  Gyro gyro;

  gyro.begin();

  auto v = gyro.get_position();

  println(v[0], v[1], v[2]);


  // println("Size check: ", c.length());
  // println("Element check: ", c[0], ", ", c[1], ", ", c[2]);

  // Vector<3> moved = std::move(c);
  // println("Move check, size after move: ", moved.length());
  // println("Element check: ", moved[0], ", ", moved[1], ", ", moved[2]);
  // println("Move check, size after move: ", c.length());
  // println("Element check: ", c[0], ", ", c[1], ", ", c[2]);


}

void loop() {
  // put your main code here, to run repeatedly:

}