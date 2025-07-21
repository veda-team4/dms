#ifndef Gps_H
#define Gps_H
#include <string>

class Gps {
public:
    Gps(const std::string& device = "/dev/ttyS0", int baudrate = 9600);
    ~Gps();

    bool cur_location(double* _latitute, double* _longitude);

private:
    int fd;
    int baudrate;
    std::string device;
    double latitude = -1, longitude = -1;

    bool init();
    bool update();
    bool parseNMEA(const std::string& sentence);
    double convertToDecimal(const std::string& raw, char direction);
};

#endif