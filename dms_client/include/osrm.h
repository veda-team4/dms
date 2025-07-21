#ifndef OSRM_H
#define OSRM_H

#include <string>
#include <vector>

struct restArea {
  std::string name;
  double route_distance; // meters
  double route_duration; // seconds
  bool isRestArea;       // true = rest area, false = shelter
};

class Osrm {
public:
  Osrm(const std::string& csvPath = "./../../dms_client/resources/rest_areas.csv");
  restArea getRestAreas(double currLat, double currLon, int topN = 3);

private:
  struct InternalArea {
    std::string name;
    double longitude;
    double latitude;
    int isRestArea;
    double straight_distance;
    double route_distance = -1;
    double route_duration = -1;
  };

  std::vector<InternalArea> allAreas;

  void loadData(const std::string& csvPath);
  double haversine(double lat1, double lon1, double lat2, double lon2);
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* buffer);
  bool query_osrm(double fromLat, double fromLon, double toLat, double toLon, double& distOut, double& durOut);
};

#endif
