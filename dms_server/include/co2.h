#ifndef CO2_H
#define CO2_H

class CO2Sensor {
public:
    CO2Sensor() = default;
    ~CO2Sensor() = default;

    bool read_cth(int* co2_ppm, int* temperature, int* humidity);
};

#endif