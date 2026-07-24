#ifndef VILLE_H
#define VILLE_H

#include <string>

class Ville {
public:
    int id;
    std::string nom;
    double latitude;
    double longitude;
    std::string region;
    long population;
    double densite;
    std::string capital;

    Ville(int id, std::string nom, double lat, double lon, std::string reg, long pop, double den, std::string cap);
};

#endif // VILLE_H
