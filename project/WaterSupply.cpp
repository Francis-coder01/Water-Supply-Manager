#include "WaterSupply.h"
#include "Reservoir.h"
#include <fstream>
#include <sstream>
#include <string>
#include <climits>
#include <iostream>
#include <iomanip>
#include <stack>
#include <cmath>
#include <utility>
#ifdef _WIN32
#include <codecvt>
#endif

using namespace std;

bool floatEquality(double a, double b){
    return fabs(a-b) < 1e-9;
}

unsigned readPopulation(string pop){
    if(pop[0] != '"') {
        return stoi(pop);
    }
    string res;
    pop = pop.substr(1,pop.length() - 2);
    for(char c: pop){
        if(c != ',') res.push_back(c);
    }
    return stoi(res);
}

/********************** Load  ****************************/

void WaterSupply::load(std::string citiesPath, std::string reservoirsPath, std::string pipesPath, std::string stationsPath) {
    loadCities(std::move(citiesPath));
    loadReservoir(std::move(reservoirsPath));
    loadStations(std::move(stationsPath));
    loadPipes(std::move(pipesPath));
    addSuperSource();
    addSuperSink();
}

void WaterSupply::loadCities(std::string path) {
    path = "../" + path;
#ifdef _WIN32
    setlocale (LC_ALL, "");
    wifstream  file(path);
    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));
    wstring  s;
#else
    ifstream  file(path);
    string  s;
#endif
    getline(file, s);
    while (getline(file, s)) {
        string name, id, code, demand;
        string population;
        istringstream iss(string(s.begin(), s.end()));
        getline(iss, name, ',');
        getline(iss, id, ',');
        getline(iss, code, ',');
        getline(iss, demand, ',');
        getline(iss,population);
        if (!code.empty()) {
            network.addVertex(code);
            cities.emplace(code, City(stoi(id), code, name, stod(demand), readPopulation(population)));
        }
    }
    file.close();
}

void WaterSupply::loadReservoir(std::string path) {
    path = "../" + path;
#ifdef _WIN32
    setlocale (LC_ALL, "");
    wifstream  reservoirsFile(path);
    reservoirsFile.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t,0x10ffff, std::consume_header>));
    wstring line;
#else
    ifstream  reservoirsFile(path);
    string  line;
#endif
    getline(reservoirsFile, line);
    while (getline(reservoirsFile, line)) {
        string name, id, code, municipality, maxDelivery;
        istringstream iss(string(line.begin(), line.end()));
        getline(iss, name, ',');
        getline(iss, municipality, ',');
        getline(iss, id, ',');
        getline(iss, code, ',');
        getline(iss, maxDelivery, ',');
        if (!code.empty()) {
            network.addVertex(code);
            reservoirs.emplace(code, Reservoir(stoi(id), code, name, municipality, stoi(maxDelivery)));
        }
    }
    reservoirsFile.close();
}

void WaterSupply::loadStations(std::string path) {
    path = "../"+path;
    ifstream  stationsFile(path);
    string line;
    getline(stationsFile, line);
    while (getline(stationsFile, line)) {
        string id, code;
        istringstream iss(string(line.begin(), line.end()));
        getline(iss, id, ',');
        getline(iss, code, ',');
        if (!code.empty()) {
            network.addVertex(code);
            stations.emplace(code, Station(stoi(id), code));
        }
    }
    stationsFile.close();
}

void WaterSupply::loadPipes(std::string path) {
    path = "../"+path;
    ifstream  pipesFile(path);
    string line;
    getline(pipesFile, line);
    while (getline(pipesFile, line)) {
        string a, b, capacity, direction;
        istringstream iss(string(line.begin(), line.end()));
        getline(iss, a, ',');
        getline(iss, b, ',');
        getline(iss, capacity, ',');
        getline(iss, direction);
        if (stoi(direction)) network.addEdge(a, b, stoi(capacity));
        else network.addBidirectionalEdge(a,b,stoi(capacity));
    }
    pipesFile.close();
}

/********************** Getters  ****************************/

Graph* WaterSupply::getNetwork() {
    return &network;
}

City WaterSupply::getCity(const std::string& code) {
    return cities.at(code);
}

Reservoir WaterSupply::getReservoir(const std::string& code) {
    return reservoirs.at(code);
}

std::unordered_map<std::string, City> WaterSupply::getCities() {
    return cities;
}

std::unordered_map<std::string, Reservoir> WaterSupply::getReservoirs() {
    return reservoirs;
}

std::unordered_map<std::string, Station> WaterSupply::getStations() {
    return stations;
}

/********************** Setters  ****************************/

void WaterSupply::addSuperSource() {
    network.addVertex("src");
    for(const auto& v: reservoirs) {
        network.addEdge("src", v.first, v.second.getDelivery());
    }
}

void WaterSupply::addSuperSink() {
    network.addVertex("sink");
    for(const auto& v: cities) {
        network.addEdge( v.first, "sink", v.second.getDemand());
    }
}

void WaterSupply::setSuperSinkWithDemand() {
    Vertex* sink = network.findVertex("sink");
    for(const auto& e: sink->getIncoming()) {
        e->setWeight(cities.find(e->getOrig()->getInfo())->second.getDemand());
    }
}

void WaterSupply::setInfSuperSink() {
    Vertex* sink = network.findVertex("sink");
    for(const auto& e: sink->getIncoming()) {
        e->setWeight(INF);
    }
}

void WaterSupply::setSuperSinkNull() {
    Vertex* sink = network.findVertex("sink");
    for(const auto& e: sink->getIncoming()) {
        e->setWeight(0);
    }
}

/********************** Checkers  ****************************/

string WaterSupply::existsCityByID(int id) {
    for (const auto& c: cities) {
        if (c.second.getID() == id) return c.first;
    }
    return "";
}

bool WaterSupply::existsCityByCode(const std::string& code) {
    return cities.count(code);
}

bool WaterSupply::existsPipe(const std::string& source, const std::string& dest) {
    return network.findEdge(source, dest) != nullptr;
}

bool WaterSupply::existsStationByCode(const std::string& code) {
    return stations.count(code);
}

bool compareString(string s1, string s2) {
    int count = 0;
    for (int i = 0; i < s1.size(); i++) {
        if (s1[i] != s2[i]) count++;
    }
    return (count <= 2);
}

string WaterSupply::existsCityByName(const std::string& name) {
    for (const auto& c: cities) {
        string nameC = c.second.getName();
        transform(nameC.begin(), nameC.end(), nameC.begin(), ::toupper);
        if (compareString(nameC, name)) return c.first;
    }
    return "";
}

string WaterSupply::existsReservoirByName(const std::string& name) {
    for (const auto& r: reservoirs) {
        string nameR = r.second.getName();
        transform(nameR.begin(), nameR.end(), nameR.begin(), ::toupper);
        if (compareString(nameR, name)) return r.first;
    }
    return "";
}

std::string WaterSupply::existsReservoirByID(int id) {
    for (const auto& r: reservoirs) {
        if (r.second.getId() == id) return r.first;
    }
    return "";
}

bool WaterSupply::existsReservoirByCode(const std::string& code) {
    return reservoirs.count(code);
}

vector<Reservoir> WaterSupply::existsMunicipality(const std::string& municipality) {
    vector<Reservoir> res;
    for (const auto& r: reservoirs) {
        string mun = r.second.getMunicipality();
        transform(mun.begin(), mun.end(), mun.begin(), ::toupper);
        if (compareString(municipality, mun)) res.push_back(r.second);
    }
    return res;
}

/********************** Statistics  ****************************/

vector<City> WaterSupply::getCityMaxDemand() {
    int maxDemand = 0;
    vector<City> max;
    for (const auto& c: cities) {
        if (c.second.getDemand() > maxDemand) {
            max.clear();
            max.push_back(c.second);
            maxDemand = c.second.getDemand();
        }
        else if (c.second.getDemand() == maxDemand) {
            max.push_back(c.second);
        }
    }
    return max;
}

vector<City> WaterSupply::getCityMinDemand() {
    double maxDemand = max_element(cities.begin(), cities.end(),
                                   [](const pair<std::string, City>& p1, const pair<std::string, City>& p2)
    {return p1.second.getDemand() < p2.second.getDemand();})->second.getDemand();
    vector<City> max;
    for (const auto &c: cities) {
        if (c.second.getDemand() == maxDemand) {
            max.push_back(c.second);
        }
    }
    return max;
}

vector<City> WaterSupply::getCityMaxPop() {
    unsigned maxPop = 0;
    vector<City> max;
    for (const auto& c: cities) {
        if (c.second.getPopulation() > maxPop) {
            max.clear();
            max.push_back(c.second);
            maxPop = c.second.getPopulation();
        }
        else if (c.second.getPopulation() == maxPop) {
            max.push_back(c.second);
        }
    }
    return max;
}

vector<City> WaterSupply::getCityMinPop() {
    unsigned minPop = std::numeric_limits<unsigned>::max();
    vector<City> max;
    for (const auto& c: cities) {
        if (c.second.getPopulation() < minPop) {
            max.clear();
            max.push_back(c.second);
            minPop = c.second.getPopulation();
        }
        else if (c.second.getPopulation() == minPop) {
            max.push_back(c.second);
        }
    }
    return max;
}

std::vector<Reservoir> WaterSupply::getReservoirMaxDel() {
    int maxDel = 0;
    vector<Reservoir> max;
    for (const auto& c: reservoirs) {
        if (c.second.getDelivery() > maxDel) {
            max.clear();
            max.push_back(c.second);
            maxDel = c.second.getDelivery();
        }
        else if (c.second.getDelivery() == maxDel) {
            max.push_back(c.second);
        }
    }
    return max;
}

std::vector<Reservoir> WaterSupply::getReservoirMinDel() {
    int maxDel = UINT_MAX;
    vector<Reservoir> max;
    for (const auto& c: reservoirs) {
        if (c.second.getDelivery() < maxDel) {
            max.clear();
            max.push_back(c.second);
            maxDel = c.second.getDelivery();
        }
        else if (c.second.getDelivery() == maxDel) {
            max.push_back(c.second);
        }
    }
    return max;
}

double WaterSupply::computeMaxDiffCapacityFlow() {
    double maxDiff = 0;
    for(const auto& v: network.getVertexSet()){
        for(Edge* e: v.second->getAdj()){
            if (e->checkActive()) {
                double flow = e->getFlow();
                if (maxDiff < (e->getWeight() - flow)) maxDiff = (e->getWeight() - flow);
            }
        }
    }
    return maxDiff;
}

double WaterSupply::computeAverageDiffCapacityFlow() {
    int n_edges = 0;
    double sum = 0;
    for(const auto& v: network.getVertexSet()){
        for(Edge* e: v.second->getAdj()){
            if (e->checkActive()) {
                double flow = e->getFlow();
                if (e->getReverse() && e->getFlow() > e->getReverse()->getFlow()) {
                    flow = abs(e->getFlow()-e->getReverse()->getFlow());
                }
                sum += (e->getWeight() - flow);
                n_edges++;
            }
        }
    }
    return sum / ((double)n_edges);
}

double WaterSupply::computeVarianceDiffCapacityFlow(double average) {
    int n_edges = 0;
    double square_diff = 0;
    for(const auto& v: network.getVertexSet()){
        for(Edge* e: v.second->getAdj()){
            if (e->checkActive()) {
                double flow = e->getFlow();
                if (e->getReverse() && e->getFlow() > e->getReverse()->getFlow()) {
                    flow = abs(e->getFlow()-e->getReverse()->getFlow());
                }
                square_diff += ((e->getWeight() - flow) - average) * ((e->getWeight() - flow) - average);
                n_edges++;
            }
        }
    }
    return square_diff / n_edges;
}

int WaterSupply::computeCityFlow(const std::string& city) {
    Vertex* end = network.findVertex(city);
    double flow = 0;
    for (Edge* e: end->getIncoming()) {
        flow += e->getFlow();
    }
    return (int)round(flow);
}

int WaterSupply::computeFlow() {
    double flow = 0;
    for (const auto& v: cities) {
        Vertex* end = network.findVertex(v.first);
        double cityFlow = 0;
        for (Edge* e: end->getIncoming()) {
            cityFlow += e->getFlow();
        }
        flow += cityFlow;
    }
    return (int)round(flow);
}



/********************** MaxFlow Options  ****************************/

void WaterSupply::optimalResMaxFlow() {
    setSuperSinkWithDemand();
    network.resetFlow();
    tester.maxFlow("src", "sink", network);
}

void WaterSupply::optimalExcessMaxFlow() {
    setSuperSinkWithDemand();
    network.resetFlow();
    tester.maxFlow("src", "sink", network);
    setInfSuperSink();
    tester.maxFlow("src", "sink", network);
}

void WaterSupply::optimalExcessCityMaxFlow(const std::string& target) {
    setSuperSinkWithDemand();
    network.findEdge(target, "sink")->setWeight(INF);
    network.resetFlow();
    tester.maxFlow("src", "sink", network);
}

void WaterSupply::optimalCityMaxFlow(const vector<std::string>& cityList) {
    setSuperSinkNull();
    network.resetFlow();
    for(const auto& city : cityList){
        network.findEdge(city, "sink")->setWeight(cities.find(city)->second.getDemand());
        tester.maxFlow("src", "sink", network);
    }
    setSuperSinkWithDemand();
    tester.maxFlow("src", "sink", network);
}

void WaterSupply::cityMaxFlow(const std::string& target) {
    network.resetFlow();
    tester.maxFlow("src", target, network);
}

void WaterSupply::OutputToFile(const string& fileName, const string& text){
    ofstream out(fileName);
    out << text;
    out.close();
}

void WaterSupply::deleteReservoirMaxReverse(const std::string& reservoir) {
    optimalResMaxFlow();
    tester.reverseMaxFlow(reservoir, "sink", network);
    network.findVertex(reservoir)->desactivate();
    tester.maxFlow("src", "sink", network);
}

void WaterSupply::reliabilityPrep() {
    tester.reliabilityPrep(network);
}

void WaterSupply::activate(const std::string& p) {
    network.findVertex(p)->activate();
}

void WaterSupply::activatePipe(const std::string& source, const std::string& dest) {
    network.findEdge(source, dest)->activate();
}

void WaterSupply::deleteReservoir(const std::string& reservoir) {
    tester.deleteReservoir(reservoir, network);
}

void WaterSupply::deleteStation(const std::string& station) {
    tester.deleteStation(station, network);
}

bool WaterSupply::existsCode(const std::string& code) {
    return (cities.count(code) || stations.count(code) || reservoirs.count(code));
}

void WaterSupply::deletePipe(const std::string& source, const std::string& dest) {
    tester.deletePipe(source, dest, network);
}

void dfsVisit(Vertex* v, stack<string>& aux){
    v->setVisited(true);
    v->setProcessing(true);
    for(Edge* adj : v->getAdj()){
        if (adj->checkActive()) {
            if(adj->getReverse() && adj->getDest()->isProcessing()) adj->desactivate();
            if(!adj->getDest()->isVisited()) dfsVisit(adj->getDest(), aux);
        }
    }
    v->setProcessing(false);
    aux.push(v->getInfo());
}

vector<string> WaterSupply::topsort() {
    vector<string> res;
    stack<string> aux;
    for(auto v : network.getVertexSet()){
        v.second->setVisited(false);
        v.second->setProcessing(false);
    }
    for(const auto& v : network.getVertexSet()){
        if(!v.second->isVisited()){
            dfsVisit(v.second, aux);
        }
    }
    while (!aux.empty()) {
        res.push_back(aux.top());
        aux.pop();
    }
    return res;
}

void WaterSupply::activateAll() {
    for (const auto& v: network.getVertexSet()) {
        for (auto e: v.second->getAdj()) {
            e->activate();
        }
    }
}

void WaterSupply::transformBidirectionalEdges(){
    for(const auto& v: network.getVertexSet()){
        for(auto e : v.second->getAdj()){
            auto reverse = e->getReverse();
            if(reverse){
                auto resultingFlow = abs(reverse->getFlow() - e->getFlow());
                reverse->setFlow(reverse->getFlow() > e->getFlow() ? resultingFlow : 0);
                e->setFlow(reverse->getFlow() == 0 ? resultingFlow : 0);
                e->getFlow() == 0 ? e->desactivate() : reverse->desactivate();
            }
        }
    }

}

vector<Edge*> WaterSupply::getMaxPathTo(Vertex* city){
    vector<Edge*> res;
    auto order = topsort();

    for(auto v: network.getVertexSet()){
        v.second->setDist(INF);
        v.second->setPath(nullptr);
    }
    auto source = network.findVertex("src");
    source->setDist(0);
    for(const auto& code : order){
        if(code == city->getInfo()) break;
        auto v = network.findVertex(code);
        for(auto adj : v->getAdj()){
            if(!adj->checkActive()) continue;
            if(adj->getDest()->getDist() > v->getDist() + (adj->getWeight() - adj->getFlow())){
                adj->getDest()->setDist(v->getDist() + (adj->getWeight() - adj->getFlow()));
                adj->getDest()->setPath(adj);
            }
        }
    }
    auto curr = city;
    while(curr->getPath() != nullptr){
        res.push_back(curr->getPath());
        curr = curr->getPath()->getOrig();
    }
    return res;
}

vector<Edge*> WaterSupply::findMinAugPath(Vertex * city) {
    vector<Edge*> res;
    auto order = topsort();
    for(auto v: network.getVertexSet()){
        v.second->setDist(INF);
        v.second->setPath(nullptr);
    }
    auto source = network.findVertex("src");
    source->setDist(0);
    for(const auto& code : order){
        if(code == city->getInfo()) break;
        auto v = network.findVertex(code);
        for(auto adj : v->getAdj()){
            if(!adj->checkActive()) continue;
            if( adj->getFlow() < adj->getWeight()  && adj->getDest()->getDist() > v->getDist() + 1/(adj->getWeight() - adj->getFlow())){
                adj->getDest()->setDist(v->getDist() + 1/(adj->getWeight() - adj->getFlow()));
                adj->getDest()->setPath(adj);
            }
        }
    }
    auto curr = city;
    while(curr->getPath() != nullptr){
        res.push_back(curr->getPath());
        curr = curr->getPath()->getOrig();

    }
    return res;
}

void WaterSupply::balancingViaMinCost(){
    for(auto v: network.getVertexSet()){
        v.second->setVisited(false);
        v.second->setPath(nullptr);
    }

    transformBidirectionalEdges();
    if(!network.isDAG()) {
        cout << "NETWORK IS NOT DAG\n";
    }
    for(int i = 1; i <= cities.size(); i++){
        auto city = network.findVertex("C_"+ to_string(i));

        auto path = getMaxPathTo(city);

        if(!path.empty() && PathHasFlow(path)){
            for(auto e : path){
                e->setFlow(e->getFlow()-1);
            }
        }
        else continue;
        auto minPath = findMinAugPath(city);

        bool equals = true;
        for(int j = 0; j < minPath.size(); j++){
            if(minPath[j] != path[j]) {
                equals = false;
                break;
            }
        }
        if(equals) {
            for(auto e : path){
                e->setFlow(e->getFlow()+1);
            }
            continue;
        }
        else {
            for(auto e : minPath){
                e->setFlow(e->getFlow()+1);
            }
            i--;
        }
    }
}

void WaterSupply::exportToFile(bool flow) {
    fstream file;
    file.open("../graph.csv", ios::out);
    unordered_map<string, int> ids;
    int id = 1;
    for(const auto& v: network.getVertexSet()){
        if(v.first == "src" || v.first == "sink") continue;
        ids.emplace(v.first,id++);
    }
    ids.emplace("src", 0);
    ids.emplace("sink", id);
    tester.maxFlow("src", "sink", network);
    for (const auto& v: network.getVertexSet()) {
        for (auto w: v.second->getAdj()) {
            file << w->getOrig()->getInfo() << ' ' << w->getDest()->getInfo() << ' ' << w->getWeight() ;
            if (flow) file << "/" << w->getFlow();
            file << '\n';
        }
    }
    file.close();
    ofstream file2("../graph2.csv");
    ifstream file1("../graph.csv");
    string line;
    while(getline(file1,line)){
        istringstream iss(line);
        unsigned long long src, dest, c;
        iss >> src >> dest >> c;
        file2 << "g.addEdge(" << src << "," << dest << "," << c <<");\n";
    }
    file2.close();
}

bool WaterSupply::PathHasFlow(std::vector<Edge *> path) {
    return std::all_of(path.begin(), path.end(), [](Edge* e) {return e->getFlow() >= 1;});
}
