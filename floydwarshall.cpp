#include "floydwarshall.h"
#include <algorithm>
#include <QDebug>

//initialisation de la matrice statique en mémoire
int FloydWarshall::prochain[100][100];

void FloydWarshall::calculer(Graphe& g) {
    int n = g.getTaille();
    std::vector<std::vector<int>>& W = g.getMatrice(); //matrice d'adjacence du graphe
    const int INF = 1000000000;

    //prépare la matrice 'prochain' avant de lancer les calculs.
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if (i == j) {
                prochain[i][j] = i; // La ville suivante de i pour aller a i c'est i.
            } else if (W[i][j] < INF) {
                prochain[i][j] = j; //si une route directe existe, le suivant de i est j.
            } else {
                prochain[i][j] = -1; //pas de route connue pour l'instant
            }
        }
    }

    //boucles de fw
    // test pour chaque paire (i, j) si passer par une ville intermédiaire 'k' est plus court.
    for(int k = 0; k < n; k++) { //pour chaque ville intermédiaire possible
        for(int i = 0; i < n; i++) { //pour chaque ville de départ
            for(int j = 0; j < n; j++) { //pour chaque ville d'arrivée
                //On vérifie que les segments (i->k) et (k->j) sont valides (pas INF)
                if (W[i][k] != INF && W[k][j] != INF) {

                    // Si le passage par 'k' réduit la distance totale entre i et j
                    if (W[i][k] + W[k][j] < W[i][j]) {
                        W[i][j] = W[i][k] + W[k][j]; // On met à jour la distance minimale

                        // INSPIRATION DU CODE :
                        // Pour aller de i à j via k, le prochain pas est le même
                        // que celui pour aller de i vers k.
                        prochain[i][j] = prochain[i][k];
                    }
                }
            }
        }
    }
}


std::vector<int> FloydWarshall::getChemin(int indexU, int indexV) {
    std::vector<int> cheminIndices;

    // Si aucun chemin n'a été trouvé (reste à -1), on retourne une liste vide
    if (prochain[indexU][indexV] == -1) return {};

    int courant = indexU;
    cheminIndices.push_back(courant);

    // On suit les etapes dans la matrice 'prochain' jusqu'à destination
    while (courant != indexV) {
        courant = prochain[courant][indexV];
        cheminIndices.push_back(courant);
    }
    return cheminIndices; //retourne la liste ordonnée des indices à parcourir
}

void FloydWarshall::debugMatrice(const Graphe& g, int limite) {
    qDebug() << "--- DEBUG MATRICE (Top" << limite << ") ---";
    // Remplace getNbVilles par getTaille
    int n = std::min(g.getTaille(), limite);

    for (int i = 0; i < n; ++i) {
        QString ligne = "";
        for (int j = 0; j < n; ++j) {
            int p = g.getPoids(i, j);
            if (p >= 1000000000) ligne += " INF ";
            else ligne += QString(" %1 ").arg(p).rightJustified(5);
        }
        qDebug() << ligne;
    }
}
