#include "osrm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

Osrm::Osrm(const std::string& csvPath) {
  loadData(csvPath);
}

restArea Osrm::getRestAreas(double currLat, double currLon, int topN) {
  for (auto& area : allAreas) {
    area.straight_distance = haversine(currLat, currLon, area.latitude, area.longitude);
  }

  std::sort(allAreas.begin(), allAreas.end(), [](const InternalArea& a, const InternalArea& b) {
    return a.straight_distance < b.straight_distance;
    });

  std::vector<InternalArea> candidates(allAreas.begin(), allAreas.begin() + std::min(10, (int)allAreas.size()));

  for (auto& area : candidates) {
    query_osrm(currLat, currLon, area.latitude, area.longitude, area.route_distance, area.route_duration);
  }

  std::sort(candidates.begin(), candidates.end(), [](const InternalArea& a, const InternalArea& b) {
    return a.route_duration < b.route_duration;
    });

  return { candidates[0].name, candidates[0].route_distance, candidates[0].route_duration, static_cast<bool>(candidates[0].isRestArea) };

  /*
  std::vector<restArea> result;
  for (int i = 0; i < std::min(topN, (int)candidates.size()); ++i) {
    const auto& c = candidates[i];
    result.push_back({ c.name, c.route_distance, c.route_duration, static_cast<bool>(c.isRestArea) });
  }
  return result;
  */
}

void Osrm::loadData(const std::string& csvPath) {
  std::ifstream file(csvPath);
  std::string line;
  std::getline(file, line); // skip header

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string name, lonStr, latStr, isRestStr;

    std::getline(ss, name, ',');
    std::getline(ss, lonStr, ',');
    std::getline(ss, latStr, ',');
    std::getline(ss, isRestStr, ',');

    InternalArea area;
    area.name = name;
    area.longitude = std::stod(lonStr);
    area.latitude = std::stod(latStr);
    area.isRestArea = std::stoi(isRestStr);

    allAreas.push_back(area);
  }
}

double Osrm::haversine(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000;
  double phi1 = lat1 * M_PI / 180.0, phi2 = lat2 * M_PI / 180.0;
  double dphi = (lat2 - lat1) * M_PI / 180.0;
  double dlambda = (lon2 - lon1) * M_PI / 180.0;
  double a = sin(dphi / 2) * sin(dphi / 2) + cos(phi1) * cos(phi2) * sin(dlambda / 2) * sin(dlambda / 2);
  return 2 * R * atan2(sqrt(a), sqrt(1 - a));
}

size_t Osrm::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
  size_t totalSize = size * nmemb;
  buffer->append((char*)contents, totalSize);
  return totalSize;
}

bool Osrm::query_osrm(double fromLat, double fromLon, double toLat, double toLon, double& distOut, double& durOut) {
  std::string url = "http://localhost:5000/route/v1/driving/";
  url += std::to_string(fromLon) + "," + std::to_string(fromLat) + ";" +
    std::to_string(toLon) + "," + std::to_string(toLat) + "?overview=false";

  CURL* curl = curl_easy_init();
  if (!curl) return false;

  std::string readBuffer;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) return false;

  try {
    auto j = json::parse(readBuffer);
    if (j["code"] != "Ok") return false;

    distOut = j["routes"][0]["distance"];
    durOut = j["routes"][0]["duration"];
    return true;
  }
  catch (...) {
    return false;
  }
}
