#ifndef _IVR_IMU_AGM_FUSION_H_
#define _IVR_IMU_AGM_FUSION_H_

namespace ivr
{
    class ImuAGMFusion
    {
    public:
        ImuAGMFusion();
        void ahrsUpdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
        void updateCV(float cqw, float cqx, float cqy, float cqz);
        void getEuler(float &yaw, float &pitch, float &roll);
        void getQuaternion(float &x, float &y, float &z, float &w);
        void setSensorSampleFrequency(float frequency);
        void setFusionAlpha(float alpha);

    private:
        void UpdateParameters();
        void UpdateCVParameters();
        void UpdateCVIParameters();
        void VectorToDevice(float x, float y, float z, float &outx, float &outy, float &outz);
        void VectorToWorld(float x, float y, float z, float &outx, float &outy, float &outz);
        void CVectorToDevice(float x, float y, float z, float &outx, float &outy, float &outz);
        void CVectorToWorld(float x, float y, float z, float &outx, float &outy, float &outz);
        void CVectorToIMU(float x, float y, float z, float &outx, float &outy, float &outz);

    private:
        float _Cei;
        float _Cqi;
        float _alpha;
        float _qw, _qx, _qy, _qz;
        float _cx, _cy, _cz;
        float _cqw, _cqx, _cqy, _cqz;
        float _qicw, _qicx, _qicy, _qicz;
        float _exInt, _eyInt, _ezInt;
        float _tanMagRef = 0.0f;
        float _magBiasX = 0.0f;
        float _magBiasY = 0.0f;
        float _magBiasZ = 0.0f;
        float _magRefZ = 0.0f;
        float _magRefY = 0.0f;
        float qwqw, qwqx, qwqy, qwqz, qxqx, qxqy, qxqz, qyqy, qyqz, qzqz;
        float cqwqw, cqwqx, cqwqy, cqwqz, cqxqx, cqxqy, cqxqz, cqyqy, cqyqz, cqzqz;
        float icqwqw, icqwqx, icqwqy, icqwqz, icqxqx, icqxqy, icqxqz, icqyqy, icqyqz, icqzqz;
        bool _moved = false;
        int _mag_recalc_count = 0;
        bool _inited;
    };

} // namespace ivr

#endif //  _IVR_IMU_AGM_FUSION_H_
