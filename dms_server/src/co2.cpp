#include "co2.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

CO2Sensor::CO2Sensor(const char* device)
    : device_path(device), fd(-1), co2_ppm(0), temperature(0), humidity(0) {
    if (!init()) {
        std::cerr << "Failed to initialize CO2 sensor on device: " << device_path << std::endl;
    }
}

CO2Sensor::~CO2Sensor() {
    if (fd >= 0) close(fd);
}

bool CO2Sensor::init() {
    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("CO2Sensor open");
        return false;
    }
    return true;
}

bool CO2Sensor::update() {
    if (fd < 0) return false;

    char buf[64] = {0};
    lseek(fd, 0, SEEK_SET);  //처음부터 읽기
    int len = read(fd, buf, sizeof(buf) - 1);
    if (len <= 0) return false;
    return parseData(buf);
}

bool CO2Sensor::parseData(const char* buf) {
    float t_f, h_f;
    if (sscanf(buf, "%d %f %f", &co2_ppm, &t_f, &h_f) != 3)
        return false;

    temperature = static_cast<int>(t_f + 0.5);  //반올림
    humidity    = static_cast<int>(h_f + 0.5);
    return true;
}

void CO2Sensor::read_CTH(int* co2, int* temp, int* hum) {
    update();
    *co2 = co2_ppm;
    *temp = temperature;
    *hum = humidity;
}
