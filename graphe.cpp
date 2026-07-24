#include "graphe.h"
#include <limits>

// utilisation d'une grosse valeur pour remplir la matrice avec des valeurs par défaut
// la base dans des algos de "plus court chemin", pour permettre de remplacer par les vrais valeurs lors du calcul
// si j'aurai rempli par des 0 car sinon les villes aurait était au meme endroit
#define INF 1000000000

Graphe::Graphe(int taille) : n(taille) {
    //redimensionne la matrice en n x n et remplit tout de "INF" (aucune route)
    matrice.resize(n, std::vector<int>(n, INF));

    //la distance pour aller d'une ville à elle-même est toujours 0
    for(int i = 0; i < n; i++)
        matrice[i][i] = 0;
}

//crée une route entre la ville u et la ville v.
void Graphe::ajouterArete(int u, int v, int poids) {
    matrice[u][v] = poids;
    matrice[v][u] = poids; // graphe non orienté
}

int Graphe::getPoids(int u, int v) const {
    return matrice[u][v];
}

std::vector<std::vector<int>>& Graphe::getMatrice() {
    return matrice;
}

int Graphe::getTaille() const {
    return n;
}
