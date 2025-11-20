#include "gyro.h"

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include "../utils/general.h"
#include "../utils/algebra.h"

using algebra::Vector;

struct Quaternion { float w, x, y, z; };

// Rotate a 3D vector by quaternion: v_world = q ⊗ v_body ⊗ q*
static Vector<3> quatRotate(const Quaternion& q, const Vector<3>& v)
{
    // Hamilton product q * v (as pure quaternion)
    float qw = q.w, qx = q.x, qy = q.y, qz = q.z;
    // q ⊗ v
    float rw = - qx*v[0] - qy*v[1] - qz*v[2];
    float rx =   qw*v[0] + qy*v[2] - qz*v[1];
    float ry =   qw*v[1] + qz*v[0] - qx*v[2];
    float rz =   qw*v[2] + qx*v[1] - qy*v[0];
    // (q ⊗ v) ⊗ q*
    return Vector<3>{
        -rw*qx + rx*qw - ry*qz + rz*qy,
        -rw*qy + ry*qw - rz*qx + rx*qz,
        -rw*qz + rz*qw - rx*qy + ry*qx
    };
}

// Convert quaternion to yaw/pitch/roll (Z-Y-X) in radians
static Vector<3> quatToEuler(const Quaternion& q)
{
    float qw = q.w, qx = q.x, qy = q.y, qz = q.z;

    // Roll (X)
    float sinr_cosp = 2.0f * (qw*qx + qy*qz);
    float cosr_cosp = 1.0f - 2.0f * (qx*qx + qy*qy);
    float roll = atan2f(sinr_cosp, cosr_cosp);

    // Pitch (Y)
    float sinp = 2.0f * (qw*qy - qz*qx);
    if (fabsf(sinp) >= 1.0f) {
        // use 90 degrees if out of range
        float pitch = copysignf(M_PI / 2.0f, sinp);
        // Yaw (Z)
        float siny_cosp = 2.0f * (qw*qz + qx*qy);
        float cosy_cosp = 1.0f - 2.0f * (qy*qy + qz*qz);
        float yaw = atan2f(siny_cosp, cosy_cosp);
        return Vector<3>{ yaw, pitch, roll };
    }
    float pitch = asinf(sinp);

    // Yaw (Z)
    float siny_cosp = 2.0f * (qw*qz + qx*qy);
    float cosy_cosp = 1.0f - 2.0f * (qy*qy + qz*qz);
    float yaw = atan2f(siny_cosp, cosy_cosp);

    return Vector<3>{ yaw, pitch, roll };
}

void Gyro::mahonyUpdate(float gx_rad, float gy_rad, float gz_rad,
                      float ax, float ay, float az, float dt)
    {
        // Normalize accelerometer (if valid)
        float norm = sqrtf(ax*ax + ay*ay + az*az);
        if (norm > 1e-6f) { ax /= norm; ay /= norm; az /= norm; }
        else { ax = ay = az = 0.0f; }

        // Estimated gravity direction from current quaternion (body frame)
        float qw = q.w, qx = q.x, qy = q.y, qz = q.z;
        float vx = 2.0f * (qx*qz - qw*qy);
        float vy = 2.0f * (qw*qx + qy*qz);
        float vz = qw*qw - qx*qx - qy*qy + qz*qz;

        // Error is cross product between measured and estimated gravity
        float ex = (ay * vz - az * vy);
        float ey = (az * vx - ax * vz);
        float ez = (ax * vy - ay * vx);

        // Apply integral feedback (bias estimation)
        if (twoKi > 0.0f) {
            integralFB[0] += twoKi * ex * dt;
            integralFB[1] += twoKi * ey * dt;
            integralFB[2] += twoKi * ez * dt;
        } else {
            integralFB = Vector<3>{0.0f, 0.0f, 0.0f};
        }

        // Apply proportional feedback + integral term to gyro
        gx_rad += twoKp * ex + integralFB[0];
        gy_rad += twoKp * ey + integralFB[1];
        gz_rad += twoKp * ez + integralFB[2];

        // Integrate quaternion rate: q̇ = 0.5 * q ⊗ ω
        float qw_dot = 0.5f * ( -qx*gx_rad - qy*gy_rad - qz*gz_rad );
        float qx_dot = 0.5f * (  qw*gx_rad + qy*gz_rad - qz*gy_rad );
        float qy_dot = 0.5f * (  qw*gy_rad + qz*gx_rad - qx*gz_rad );
        float qz_dot = 0.5f * (  qw*gz_rad + qx*gy_rad - qy*gx_rad );

        qw += qw_dot * dt;
        qx += qx_dot * dt;
        qy += qy_dot * dt;
        qz += qz_dot * dt;

        // Normalize quaternion
        float qnorm = sqrtf(qw*qw + qx*qx + qy*qy + qz*qz);
        if (qnorm > 1e-6f) {
            q.w = qw / qnorm;
            q.x = qx / qnorm;
            q.y = qy / qnorm;
            q.z = qz / qnorm;
        }
    }

    Gyro::Gyro() : mpu() {}

    void Gyro::begin() {
        Wire.begin();
        mpu.initialize();
        Serial.println(F("Testing MPU6050 connection..."));
        if (!mpu.testConnection()) {
            Serial.println(F("MPU6050 connection failed"));
        } else {
            Serial.println(F("MPU6050 connection successful"));
        }

        // Configure full-scale ranges (optional; adjust scales accordingly)
        // Jeff Rowberg library enums:
        //   Accel: MPU6050_ACCEL_FS_2/4/8/16
        //   Gyro : MPU6050_GYRO_FS_250/500/1000/2000
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        accelLsbPerG = 16384.0f; // ±2g

        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
        gyroLsbPerDPS = 16.4f;   // ±2000 °/s

        // Optional: low-pass filter / sample rate (tune if needed)
        // mpu.setDLPFMode(MPU6050_DLPF_BW_20);
        // mpu.setRate(4); // sample divider

        // Simple gyro bias estimate: average at rest (0.5 s)
        Vector<3> gb{0,0,0};
        const int N = 500;
        for (int i = 0; i < N; ++i) {
            int16_t ax, ay, az, gx, gy, gz;
            mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
            gb[0] += (gx / gyroLsbPerDPS) * (M_PI/180.0f);
            gb[1] += (gy / gyroLsbPerDPS) * (M_PI/180.0f);
            gb[2] += (gz / gyroLsbPerDPS) * (M_PI/180.0f);
            delay(1);
        }
        gyroBias = Vector<3>{ gb[0]/N, gb[1]/N, gb[2]/N };

        lastMicros = micros();
    }

    // Step the filter and integrators; return world-frame linear acceleration (m/s^2)
    Vector<3> Gyro::read() {
        // Δt
        unsigned long now = micros();
        float dt = (now - lastMicros) * 1e-6f;
        if (dt <= 0.0f || dt > 0.1f) dt = 0.001f; // guard
        lastMicros = now;

        // Read raw sensors
        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        mpu.getMotion6(&ax_raw,&ay_raw,&az_raw,&gx_raw,&gy_raw,&gz_raw);

        // Convert to physical units
        Vector<3> a_body_mps2{
            (ax_raw / accelLsbPerG) * G0,
            (ay_raw / accelLsbPerG) * G0,
            (az_raw / accelLsbPerG) * G0
        };
        Vector<3> g_body_rads{
            (gx_raw / gyroLsbPerDPS) * ((float)M_PI/180.0f) - gyroBias[0],
            (gy_raw / gyroLsbPerDPS) * ((float)M_PI/180.0f) - gyroBias[1],
            (gz_raw / gyroLsbPerDPS) * ((float)M_PI/180.0f) - gyroBias[2]
        };

        // Mahony update (uses accel normalized to 1g)
        float ax_n = ax_raw / accelLsbPerG;
        float ay_n = ay_raw / accelLsbPerG;
        float az_n = az_raw / accelLsbPerG;
        mahonyUpdate(g_body_rads[0], g_body_rads[1], g_body_rads[2],
                     ax_n, ay_n, az_n, dt);

        // Rotate body accel to world frame, then subtract gravity [0,0,G0]
        Vector<3> a_world_mps2 = quatRotate(q, a_body_mps2);
        Vector<3> a_linear_world{ a_world_mps2[0],
                                  a_world_mps2[1],
                                  a_world_mps2[2] - G0 };

        // Integrate velocity and position (very sensitive to drift!)
        // ZUPT: if nearly still, snap velocity to zero
        float accelMag_g = sqrtf(ax_n*ax_n + ay_n*ay_n + az_n*az_n);
        float gyroMag_dps = sqrtf(
            powf(gx_raw/gyroLsbPerDPS,2) +
            powf(gy_raw/gyroLsbPerDPS,2) +
            powf(gz_raw/gyroLsbPerDPS,2)
        );
        bool isStill = (fabsf(accelMag_g - 1.0f) < stillAccelThresh_g) &&
                       (gyroMag_dps < stillGyroThresh_dps);

        if (isStill) {
            velocity = Vector<3>{0.0f, 0.0f, 0.0f};
        } else {
            velocity = Vector<3>{
                velocity[0] + a_linear_world[0] * dt,
                velocity[1] + a_linear_world[1] * dt,
                velocity[2] + a_linear_world[2] * dt
            };
            // mild damping to limit drift growth
            velocity = Vector<3>{ velocity[0]*velDamping,
                                  velocity[1]*velDamping,
                                  velocity[2]*velDamping };
        }

        position = Vector<3>{
            position[0] + velocity[0] * dt,
            position[1] + velocity[1] * dt,
            position[2] + velocity[2] * dt
        };

        return a_linear_world; // world-frame linear acceleration (m/s^2)
    }

    // Orientation as yaw/pitch/roll (radians)
    Vector<3> Gyro::read_orientation() {
        return quatToEuler(q);
    }

    // Accessors for "absolute" position (world frame; starts at origin)
    Vector<3> Gyro::get_position() const { return position; }
    Vector<3> Gyro::get_velocity() const { return velocity; }

