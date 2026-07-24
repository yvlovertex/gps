#include "lecturecsv.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <QDebug>

std::vector<Ville> CSVReader::lireVilles(const std::string& fichier) {
    std::vector<Ville> villes;
    std::ifstream file(fichier);

    //force la lecture des points décimaux (.) peu importe la langue du PC
    std::locale::global(std::locale("C"));

    if (!file.is_open()) return villes;

    std::string line;
    //saute la premiere ligne
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;

        //decoupe par virgule
        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        //verif du nombre de colonnes (min 15 pour atteindre la population)
        if (row.size() >= 15) {
            try {
                villes.emplace_back( // emplace_back; c un push_back mais en mieux (ne crée pas d'objet temporaire)
                    std::stoi(row[1]),  //index 1 : city_id
                    row[0], //index 0 : city (nom)
                    std::stod(row[3]),  //index 3 : lat
                    std::stod(row[4]),  //index 4 : lng
                    row[8], //index 8 : admin_name (région)
                    std::stol(row[14]), //index 14 : population
                    std::stod(row[13]), //index 13 : density
                    row[12] //index 12 : capital
                    );
            } catch (...) { continue; } //continue si nimporte quelle erreur dans une ville
        }
    }
    file.close();
    qDebug() << villes.size() << "villes chargées.";
    return villes;
}

void CSVReader::lireTemps(const std::string& fichier, Graphe& g, const std::vector<Ville>& villes) {
    std::ifstream file(fichier);
    if (!file.is_open()) return;

    //mapping id csv pour indice matrice (0 à N)
    std::map<int, int> idToIndex;
    for(size_t i = 0; i < villes.size(); ++i) {
        idToIndex[villes[i].id] = (int)i;
    }

    std::string line;
    //pas d'en tête a sauter pour temps.csv
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string s_id1, s_id2, s_poids;

        //lecture directe : id1,id2,poids
        if (std::getline(ss, s_id1, ',') &&
            std::getline(ss, s_id2, ',') &&
            std::getline(ss, s_poids)) { //marche seulement si les 3 ont réussis

            int u = std::stoi(s_id1);
            int v = std::stoi(s_id2);
            int p = std::stoi(s_poids);

            if (idToIndex.count(u) && idToIndex.count(v)) {
                g.ajouterArete(idToIndex[u], idToIndex[v], p);
            }
        }
    }
    file.close();
}
