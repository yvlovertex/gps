#ifndef GRAPHE_H
#define GRAPHE_H

#include <vector>

//classe représentant la structure du réseau routier (Villes + Routes).
//utilise une matrice d'adjacence pour stocker les distances.
class Graphe {
private:
    int n; //nombre total de sommets (villes)
    std::vector<std::vector<int>> matrice; // tableau stockant les distances

public:
    //initialise une matrice carrée n x n
    Graphe(int taille);

    //ajoute une connexion entre deux villes avec un poids (distance en km)
    void ajouterArete(int u, int v, int poids);

    //récupère le poids entre deux villes (retourne INF si aucune route)
    int getPoids(int u, int v) const;

    //retourne une référence vers la matrice pour permettre à Floyd-Warshall de la modifier
    std::vector<std::vector<int>>& getMatrice();

    //retourne le nombre de villes dans le graphe
    int getTaille() const;
};


#endif // GRAPHE_H
