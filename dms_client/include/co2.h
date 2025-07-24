#ifndef CO2_H
#define CO2_H

class CO2Sensor {
public:
    CO2Sensor(const char* device = "/dev/dms_co2");
    ~CO2Sensor();

    void read_CTH(int* co2, int* temp, int* hum);

private:
    int fd;
    const char* device_path;
    int co2_ppm;
    int temperature;
    int humidity;

    bool init();                       
    bool update();                     
    bool parseData(const char* buf); 
};

#endif
