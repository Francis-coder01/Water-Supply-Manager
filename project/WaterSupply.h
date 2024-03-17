#ifndef WATERSUPPLYMANAGER_WATERSUPPLY_H
#define WATERSUPPLYMANAGER_WATERSUPPLY_H
#include "City.h"
#include "datastructures/Graph.h"
#include "Reservoir.h"
#include "Station.h"
#include "unordered_map"

class WaterSupply {
private:
    Graph network;
    std::unordered_map<std::string, City> cities;
    std::unordered_map<std::string, Reservoir> reservoirs;
    std::unordered_map<std::string, Station> stations;
    void loadCities();
    void loadReservoir();
    void loadStations();
    void loadPipes();
    bool dataSet;
public:
    void load();
    void setDataSmall();
    void setDataDefault();
    Graph getNetwork();
    City getCity(std::string code);
    Reservoir getReservoir(std::string code);
    Station getStation(std::string code);
    std::unordered_map<std::string, City> getCities();
    std::unordered_map<std::string, Reservoir> getReservoirs();
    std::unordered_map<std::string, Station> getStations();
    std::string existsCityByID(int id);
    bool existsCityByCode(std::string code);
    std::string existsCityByName(std::string name);
    std::string existsReservoirByName(std::string name);
    std::string existsReservoirByID(int id);
    bool existsReservoirByCode(std::string code);
    std::vector<Reservoir> existsMunicipality(std::string municipality);
    void getSuperSource();
    void getSuperSink();
    void maxFlow(std::string source, std::string sink);
    void computeAverageAndVarianceOfPipes();
    void computeCitiesStatistics();
    std::vector<City> getCityMaxDemand();
    std::vector<City> getCityMinDemand();
    std::vector<City> getCityMaxPop();
    std::vector<City> getCityMinPop();
    std::vector<Reservoir> getReservoirMaxDel();
    std::vector<Reservoir> getReservoirMinDel();
    void setInfSuperSink();
    int computeMaxFlow();
    int getCityFlow(std::string city);
    void optimalResMaxFlow();
    void optimalExcessMaxFlow();
    void optimalExcessCityMaxFlow(std::string target);
    void cityMaxFlow(std::string target);
};


#endif //WATERSUPPLYMANAGER_WATERSUPPLY_H
