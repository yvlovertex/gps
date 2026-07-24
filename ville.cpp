#include "ville.h"

Ville::Ville(int id, std::string nom, double lat, double lon, std::string reg, long pop, double den, std::string cap)
    : id(id), nom(nom), latitude(lat), longitude(lon), region(reg), population(pop), densite(den), capital(cap) {}
