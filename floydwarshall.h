#ifndef FLOYDWARSHALL_H
#define FLOYDWARSHALL_H

#include "graphe.h"
#include <vector>
// cet algo calcule les plus courts chemins entre TOUTES les paires de villes.
class FloydWarshall {
public:
    //calcule les distances minimales et remplit la matrice 'prochain'
    static void calculer(Graphe& g);

    //reconstruit le chemin (liste d'IDs) entre deux villes après le calcul
    static std::vector<int> getChemin(int u, int v);

    //affiche un extrait de la matrice de poids dans la console pour le debug
    static void debugMatrice(const Graphe& g, int limite);

private:
    // matrice de reconstruction de chemin.
    //prochain[i][j] stocke l'indice de la ville immédiatement après 'i'
    //pour aller vers 'j' par le chemin le plus court.
    static int prochain[100][100];
};
#endif
