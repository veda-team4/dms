#include "co2.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

bool CO2Sensor::read_cth(int* co2_ppm, int* temperature, int* humidity) {
    int fd = open("/dev/dms_co2", O_RDONLY);
    if (fd < 0) return false;

    char buf[64] = {0};
    int len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (len <= 0) return false;

    float t_f, h_f;
    if (sscanf(buf, "%d %f %f", co2_ppm, &t_f, &h_f) != 3)
        return false;

    *temperature = static_cast<int>(t_f + 0.5);  // 반올림
    *humidity    = static_cast<int>(h_f + 0.5);  // 반올림

    return true;
}
