#ifndef WATERSUPPLYMANAGER_CITY_H
#define WATERSUPPLYMANAGER_CITY_H

#include <string>

class City {
private:
    int id;
    std::string code;
    std::string name;
    int demand;
    unsigned population;
public:
    City(int id, std::string code, std::string name, double demand, int population): id(id), code(std::move(code)), name(std::move(name)), demand(std::move(demand)), population(population) {};
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] int getDemand() const;
    [[nodiscard]] unsigned getPopulation() const;
    int getID() const;
    std::string getCode();
};


#endif //WATERSUPPLYMANAGER_CITY_H
