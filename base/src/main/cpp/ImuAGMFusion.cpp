#define _USE_MATH_DEFINES

#include "iostream"
#include <math.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImuAGMFusion.h"

#define DEG2RAD (float)(M_PI / 180.f)
#define RAD2DEG (float)(180.f / M_PI)

#define LI_ALG
// Magnetic switch
#define MAG_RESET_ALG

// PID parameters (P/I only)
#define Kp 1.5f  // proportional gain
#define Ki 0.05f // integral gain

// Sample frequency changed through setSensorSampleFrequency
// Frequency for each tracker type are defined in HMD properties
static const float DEFAULT_FREQUENCY = 1000.0f;
static const float DEFAULT_ALPHA = 1.0f;

static void GetRadEulerFromSensor(float ax, float ay, float az, float mx, float my, float mz, float &yaw, float &pitch, float &roll)
{
    float inv_norm;
    if ((ax == 0.0f && ay == 0.0f && az == 0.0f) ||
        (mx == 0.0f && my == 0.0f && mz == 0.0f))
    {
        return;
    }
    //// normalize the reading
    inv_norm = 1.f / sqrt(ax * ax + ay * ay + az * az);
    roll = atan2f(ax, ay);
    pitch = asinf(-az * inv_norm);
    yaw = atan2f(my * sinf(roll) - mx * cosf(roll), mz * cosf(pitch) + mx * sinf(pitch) * sinf(roll) + my * sinf(pitch) * cosf(roll));
}

static void GetEulerFromSensor(float ax, float ay, float az, float mx, float my, float mz, float &qw, float &qx, float &qy, float &qz)
{
    glm::vec3 euler;
    GetRadEulerFromSensor(ax, ay, az, mx, my, mz, euler.z, euler.y, euler.x);
    glm::quat temp(euler);
    qw = temp.w;
    qz = temp.x;
    qx = temp.y;
    qy = temp.z;
}

namespace ivr
{
    glm::quat _cvRotation;

    int cv_flag = 0;

    enum MoveState
    {
        EMOVE_STILL = 0,
        EMOVE_MOVING,
        EMOVE_STILL_IN,
    };

    ImuAGMFusion::ImuAGMFusion()
    {
        _qw = 1.0f;
        _qx = 0.0f;
        _qy = 0.0f;
        _qz = 0.0f;

        _cx = 0.0f;
        _cy = 0.0f;
        _cz = 1.0f;

        _exInt = 0.f, _eyInt = 0.f, _ezInt = 0.f;

        _qicw = 1.0f; //用于记录IMU和CV之间的四元数变换值
        _qicx = 1.0f;
        _qicy = 1.0f;
        _qicz = 1.0f;

        // Properties can be modified by configuration. Set defaults here.
        setSensorSampleFrequency(DEFAULT_FREQUENCY);
        setFusionAlpha(DEFAULT_ALPHA);

        _inited = false;
    }

    void ImuAGMFusion::UpdateParameters()
    {
        qwqw = _qw * _qw;
        qwqx = _qw * _qx;
        qwqy = _qw * _qy;
        qwqz = _qw * _qz;
        qxqx = _qx * _qx;
        qxqy = _qx * _qy;
        qxqz = _qx * _qz;
        qyqy = _qy * _qy;
        qyqz = _qy * _qz;
        qzqz = _qz * _qz;
    }

    void ImuAGMFusion::UpdateCVParameters()
    {
        cqwqw = _cqw * _cqw;
        cqwqx = _cqw * _cqx;
        cqwqy = _cqw * _cqy;
        cqwqz = _cqw * _cqz;
        cqxqx = _cqx * _cqx;
        cqxqy = _cqx * _cqy;
        cqxqz = _cqx * _cqz;
        cqyqy = _cqy * _cqy;
        cqyqz = _cqy * _cqz;
        cqzqz = _cqz * _cqz;
    }

    void ImuAGMFusion::UpdateCVIParameters()
    {
        icqwqw = _qicw * _qicw;
        icqwqx = _qicw * _qicx;
        icqwqy = _qicw * _qicy;
        icqwqz = _qicw * _qicz;
        icqxqx = _qicx * _qicx;
        icqxqy = _qicx * _qicy;
        icqxqz = _qicx * _qicz;
        icqyqy = _qicy * _qicy;
        icqyqz = _qicy * _qicz;
        icqzqz = _qicz * _qicz;
    }

    void ImuAGMFusion::VectorToDevice(float x, float y, float z, float &outx, float &outy, float &outz)
    {
        outx = 2.0f * x * (0.5f - qyqy - qzqz) + 2.0f * y * (qxqy + qwqz) + 2.0f * z * (qxqz - qwqy);
        outy = 2.0f * x * (qxqy - qwqz) + 2.0f * y * (0.5f - qxqx - qzqz) + 2.0f * z * (qwqx + qyqz);
        outz = 2.0f * x * (qxqz + qwqy) + 2.0f * y * (qyqz - qwqx) + 2.0f * z * (0.5f - qxqx - qyqy);
    }

    void ImuAGMFusion::VectorToWorld(float x, float y, float z, float &outx, float &outy, float &outz)
    {
        outx = 2 * x * (0.5f - qyqy - qzqz) + 2 * y * (qxqy - qwqz) + 2 * z * (qxqz + qwqy);
        outy = 2 * x * (qxqy + qwqz) + 2 * y * (0.5f - qxqx - qzqz) + 2 * z * (qyqz - qwqx);
        outz = 2 * x * (qxqz - qwqy) + 2 * y * (qyqz + qwqx) + 2 * z * (0.5f - qxqx - qyqy);
    }

    void ImuAGMFusion::CVectorToDevice(float x, float y, float z, float &outx, float &outy, float &outz)
    {
        outx = 2.0f * x * (0.5f - cqyqy - cqzqz) + 2.0f * y * (cqxqy + cqwqz) + 2.0f * z * (cqxqz - cqwqy);
        outy = 2.0f * x * (cqxqy - cqwqz) + 2.0f * y * (0.5f - cqxqx - cqzqz) + 2.0f * z * (cqwqx + cqyqz);
        outz = 2.0f * x * (cqxqz + cqwqy) + 2.0f * y * (cqyqz - cqwqx) + 2.0f * z * (0.5f - cqxqx - cqyqy);
    }

    void ImuAGMFusion::CVectorToWorld(float x, float y, float z, float &outx, float &outy, float &outz)
    {
        outx = 2 * x * (0.5f - cqyqy - cqzqz) + 2 * y * (cqxqy - cqwqz) + 2 * z * (cqxqz + cqwqy);
        outy = 2 * x * (cqxqy + cqwqz) + 2 * y * (0.5f - cqxqx - cqzqz) + 2 * z * (cqyqz - cqwqx);
        outz = 2 * x * (cqxqz - cqwqy) + 2 * y * (cqyqz + cqwqx) + 2 * z * (0.5f - cqxqx - cqyqy);
    }

    void ImuAGMFusion::CVectorToIMU(float x, float y, float z, float &outx, float &outy, float &outz)
    {
        outx = 2 * x * (0.5f - icqyqy - icqzqz) + 2 * y * (icqxqy - icqwqz) + 2 * z * (icqxqz + icqwqy);
        outy = 2 * x * (icqxqy + icqwqz) + 2 * y * (0.5f - icqxqx - icqzqz) + 2 * z * (icqyqz - icqwqx);
        outz = 2 * x * (icqxqz - icqwqy) + 2 * y * (icqyqz + icqwqx) + 2 * z * (0.5f - icqxqx - icqyqy);
    }

    void ImuAGMFusion::updateCV( float cqw, float cqx, float cqy, float cqz)
    {
        cv_flag = 1;
        _cvRotation = glm::quat(cqw, cqx, cqy, cqz);
        _cqw = cqw;
        _cqx = cqx;
        _cqy = cqy;
        _cqz = cqz;
    }

    void ImuAGMFusion::ahrsUpdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
    {
#define DISCARD_DATA_NUM 500
#define DISCARD_DATA_CAM 1
#define MAG_RECALC_INTERVAL 500
#define MAG_DRIFT_ANGLE_THRESHOLD 5.0f * M_PI / 180
#define KMAG 1.0f
#define THETA 0.1f

        float inv_norm;
        float hx, hy, hz;
        float vx, vy, vz;
        float ex, ey, ez;
        float alpha = _alpha; // external gain for mag

        static int discard_count = 0;
        static int discard_count_cam = 0;
        float mag_origx = mx, mag_origy = my, mag_origz = mz;
        if (discard_count < DISCARD_DATA_NUM)
        {
            discard_count++;
            return;
        }

        if ((ax == 0.0f && ay == 0.0f && az == 0.0f))
        {
            return;
        }

        if (!_inited)
        {
            GetEulerFromSensor(ax, ay, az, mx, my, mz, _qw, _qx, _qy, _qz); //~
            UpdateParameters();                                             //~
            VectorToWorld(mag_origx, mag_origy, mag_origz, hx, hy, hz);
            _magRefZ = sqrt(hx * hx + hz * hz); //北
            _magRefY = hy;
            _inited = true;
            cv_flag = 0;
            return;
        }
        // update integration coefficients
        if (dt > 0.0f)
        {
            _Cei = Ki * dt / 2.f;
            _Cqi = dt / 2.f;
        }
#ifdef MAG_RESET_ALG
        VectorToWorld(mag_origx, mag_origy, mag_origz, hx, hy, hz);
        hx -= _magBiasX;
        hy -= _magBiasY;
        hz -= _magBiasZ;
        VectorToDevice(hx, hy, hz, mx, my, mz);

        float gx1, gy1, gz1;
        VectorToDevice(gx, gy, gz, gx1, gy1, gz1);

        float gLength = (float)(gx1 * gx1 * 0.04 + gy1 * gy1 + gz1 * gz1 * 0.04) * RAD2DEG * RAD2DEG;

        // beta: gain for acc, theta:internal gain for mag, drift_thd:drift threshold
        float beta = 1.0f, theta = 0.1f, drift_thd = 0.01f;
        if (gLength < 36.0f)
        {
            drift_thd = 0.05f;
            // theta = 0.2;
        }

        // normalize the reading
        inv_norm = 1.f / sqrt(ax * ax + ay * ay + az * az);
        ax *= inv_norm;
        ay *= inv_norm;
        az *= inv_norm;

        if (cv_flag == 1) //决定是否用CV
        {
            if (discard_count_cam < DISCARD_DATA_CAM) //前1500个认为不稳定弃用
            {
                glm::quat _imuRotation(_qw, _qx, _qy, _qz);                           //~
                glm::quat _differRotation = glm::inverse(_imuRotation) * _cvRotation; //~
                _qicw = _differRotation.w;                                            //~
                _qicx = _differRotation.x;                                            //~
                _qicy = _differRotation.y;                                            //~
                _qicz = _differRotation.z;                                            //~
                UpdateCVIParameters();                                                //更新CV到IMU坐标系变换用的四元数  ~

                discard_count_cam++;
                VectorToDevice(0, 1.0f, 0, vx, vy, vz);
                ex = beta * (ay * vz - az * vy);
                ey = beta * (az * vx - ax * vz);
                ez = beta * (ax * vy - ay * vx);
            }
            else
            {
                UpdateCVParameters();                          //更新CV坐标系变换用的四元数
                CVectorToDevice(_cx, _cy, _cz, _cx, _cy, _cz); //向量转到直接相机坐标系
                CVectorToIMU(_cx, _cy, _cz, _cx, _cy, _cz);    //拉平IMU和相机的坐标系
                VectorToWorld(_cx, _cy, _cz, hx, hy, hz);      //将向量转到世界坐标系
                _cx = 0.0f;
                _cy = 0.0f;
                _cz = 1.0f;
                float mex = 0.0f, mey = 0.0f, mez = 0.0f;
                // Calculate the rotating direction
                std::cout << "hy:" << hy << " "
                          << "hx:" << hx << " "
                          << "hz:" << hz << " "
                          << "正切:" << hx / hz << std::endl;
                if (hx / hz > drift_thd)
                {
                    theta *= -1.0f;
                }
                else if (hx / hz > -drift_thd)
                {
                    theta = 0;
                }
                VectorToDevice(0, THETA * theta, 0, mex, mey, mez);

                // calculate the perpendicular direction
                VectorToDevice(0, 1.0f, 0, vx, vy, vz);
                // error is sum of cross product between reference direction of fields and direction measured by sensors
                ex = beta * (ay * vz - az * vy) + alpha * mex;
                ey = beta * (az * vx - ax * vz) + alpha * mey;
                ez = beta * (ax * vy - ay * vx) + alpha * mez;
            }
            cv_flag = 0;
        }
        else
        {
            VectorToDevice(0, 1.0f, 0, vx, vy, vz);
            ex = beta * (ay * vz - az * vy);
            ey = beta * (az * vx - ax * vz);
            ez = beta * (ax * vy - ay * vx);
        }

#else
        float gx1, gy1, gz1;
        VectorToDevice(gx, gy, gz, gx1, gy1, gz1);

        float gLength = (float)(gx1 * gx1 * 0.04 + gy1 * gy1 + gz1 * gz1 * 0.04) * RAD2DEG * RAD2DEG;

        VectorToDevice(0, 1.0f, 0, vx, vy, vz);

        float mex = 0.0f, mey = 0.0f, mez = 0.0f;
        float wx, wy, wz;
#if 1
        VectorToWorld(mx, my, mz, hx, hy, hz); //磁力计方向到世界坐标系
        VectorToDevice(sqrt(hx * hx + hz * hz), hy, 0, wx, wy, wz);
        mex = (my * wz - mz * wy);
        mey = (mz * wx - mx * wz); //求解夹角 磁场方向到夹角
        mez = (mx * wy - my * wx);
#else
        if (hx / hz > drift_thd)
        {
            theta *= -1.0f;
        }
        else if (hx / hz > -drift_thd)
        {
            theta = 0;
        }
        VectorToDevice(0, THETA * theta, 0, mex, mey, mez);
#endif

        // error is sum of cross product between reference direction of fields and direction measured by sensors
        ex = (ay * vz - az * vy) + alpha * mex; //计算a和v到
        ey = (az * vx - ax * vz) + alpha * mey;
        ez = (ax * vy - ay * vx) + alpha * mez;
#endif

        // integral error scaled integral gain
        _exInt += ex * _Cei;
        _eyInt += ey * _Cei;
        _ezInt += ez * _Cei;

        // adjusted gyroscope measurements
        gx += Kp * ex + _exInt;
        gy += Kp * ey + _eyInt;
        gz += Kp * ez + _ezInt;

        // integrate quaternion rate
        _qw += (-_qx * gx - _qy * gy - _qz * gz) * _Cqi;
        _qx += (_qw * gx + _qy * gz - _qz * gy) * _Cqi;
        _qy += (_qw * gy - _qx * gz + _qz * gx) * _Cqi;
        _qz += (_qw * gz + _qx * gy - _qy * gx) * _Cqi;

        // normalize quaternion
        inv_norm = 1.f / sqrt(_qw * _qw + _qx * _qx + _qy * _qy + _qz * _qz);
        _qw *= inv_norm;
        _qx *= inv_norm;
        _qy *= inv_norm;
        _qz *= inv_norm;

        UpdateParameters();

#ifdef MAG_RESET_ALG
        // Check if the magneto is interfered and reset mag bias
        VectorToWorld(mx, my, mz, hx, hy, hz);

        if (gLength < 36.0f)
        {
            if (_moved)
            {
                // calculate the deviation
                _mag_recalc_count++;
                if (hz != 0.f)
                {
                    _tanMagRef += (hx / hz);
                }
                // hz less than 0 means direction reversed.
                if (_mag_recalc_count >= MAG_RECALC_INTERVAL || hz < 0)
                {
                    // calculate the drift
                    float n_drift = atanf(_tanMagRef / _mag_recalc_count);
                    if (n_drift > MAG_DRIFT_ANGLE_THRESHOLD || n_drift < -MAG_DRIFT_ANGLE_THRESHOLD || hz < 0)
                    {
                        VectorToWorld(mag_origx, mag_origy, mag_origz, hx, hy, hz);
                        _magBiasX += (hx - _magBiasX) * KMAG;
                        _magBiasY += (hy - _magRefY - _magBiasY) * KMAG;
                        _magBiasZ += (hz - _magRefZ - _magBiasZ) * KMAG;
                    }
                    _mag_recalc_count = 0;
                    _tanMagRef = 0.0f;
                    _moved = false;
                }
            }
        }
        else
        {
            _moved = true;
            _mag_recalc_count = 0;
            _tanMagRef = 0;
        }

#endif
    }

    void ImuAGMFusion::getEuler(float &yaw, float &pitch, float &roll)
    {
        float qxqx = _qx * _qx;
        pitch = asin(2.f * (_qw * _qx - _qz * _qy)) * RAD2DEG;
        yaw = atan2(2.f * (_qw * _qy + _qx * _qz), 1.f - 2.f * (qxqx + _qy * _qy)) * RAD2DEG;
        roll = atan2(2.f * (_qw * _qz + _qx * _qy), 1.f - 2.f * (qxqx + _qz * _qz)) * RAD2DEG;
    }

    void ImuAGMFusion::getQuaternion(float &w, float &x, float &y, float &z)
    {
        w = _qw;
        x = _qx;
        y = _qy;
        z = _qz;
    }

    void ImuAGMFusion::setSensorSampleFrequency(float frequency)
    {
        _Cei = Ki / frequency / 2.0f;
        _Cqi = 1.0f / frequency / 2.0f;
    }

    void ImuAGMFusion::setFusionAlpha(float alpha)
    {
        _alpha = alpha;
    }
}
