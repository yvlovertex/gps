#ifndef LECTURECSV_H
#define LECTURECSV_H

#include <vector>
#include "ville.h"
#include "graphe.h"


//classe pour charger les données depuis les fichiers CSV
//les méthodes sont statiques pour être utilisées sans instancier la classe.
class CSVReader {
public:
    //charge les infos des villes (nom, lat, lon, population...)
    static std::vector<Ville> lireVilles(const std::string& fichier);
    //charge les liaisons entre les villes et rempli le Graphe
    static void lireTemps(const std::string& fichier, Graphe& g, const std::vector<Ville>& villes);
};

#endif // LECTURECSV_H
