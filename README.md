# 🗺️ Planificateur d'Itinéraires & Carte Interactive (Qt C++)

Une application C++ / Qt moderne et intuitive permettant d'interroger, d'analyser et de visualiser des trajets optimaux entre plusieurs villes françaises. Le projet s'appuie sur l'algorithme de **Floyd-Warshall** pour le calcul du plus court chemin, une interface graphique vectorielle fluide (`QGraphicsView`) et une intégration réseau via l'API **OpenWeatherMap**.

---

## 🚀 Fonctionnalités Clés

- **Calcul d'itinéraires multi-étapes :**
  - Recherche du chemin le plus rapide entre deux villes avec prise en compte dynamique d'étapes intermédiaires personnalisables.
  - Calcul et affichage en temps réel du temps cumulé de trajet.
- **Visualisation Cartographique Interactive :**
  - Projection géographique adaptative des coordonnées GPS sur fond de carte.
  - **LOD (Level of Detail) :** Affichage progressif des villes (capitales, préfectures, villes secondaires) en fonction du niveau de zoom.
  - Zoom fluide au curseur, déplacement de carte (*pan/drag*), tooltips d'informations au survol et tracé de routes en courbes de Bézier.
- **Animation de Trajet :**
  - Mode d'animation pas à pas matérialisant le déplacement d'un véhicule/curseur le long de l'itinéraire calculé.
- **Comparateur de Distances & Matrice :**
  - Module d'analyse comparative permettant de construire un tableau croisé des temps de parcours entre une sélection de villes.
- **Données Météo en Temps Réel :**
  - Récupération asynchrone (`QNetworkAccessManager`) des conditions météo (température, humidité, vent, éphémérides) pour le départ, l'arrivée et les étapes.
- **Export & Personnalisation :**
  - Exportation sous forme d'image (`.png` / `.jpg`) du rendu exact du trajet sur la carte.
  - Option de masquage du fond de carte avec bascule automatique sur un mode sombre (*Dark Mode*).
  - Barre de recherche prédictive à complétion automatique (*Smart Search*) dans la sélection des villes.

---

## 🛠️ Stack Technique

- **Langage :** C++17
- **Framework GUI :** Qt 5 / Qt 6 (Qt Widgets, Qt Network, Qt Graphics)
- **Algorithmique & Graphes :** Algorithme de **Floyd-Warshall** (reconstruction de chemin via matrice `prochain`)
- **API Extérieure :** OpenWeatherMap REST API (format JSON)
- **Format de Données :** Fichiers CSV (`villes.csv`, `temps.csv`)

---

## 📂 Structure du Projet

```text
.
├── main.cpp                 # Point d'entrée de l'application
├── mainwindow.h / .cpp      # Fenêtre principale & logique d'IHM
├── cartewidget.h / .cpp     # Composant QGraphicsView de la carte
├── floydwarshall.h / .cpp   # Algorithme du plus court chemin
├── graphe.h / .cpp          # Modélisation du graphe (Matrice d'adjacence)
├── lecturecsv.h / .cpp      # Lecteur/Parser de fichiers CSV
├── ville.h / .cpp          # Structure/Classe de représentation d'une ville
└── data/
    ├── france_map.png       # Image d'arrière-plan de la carte
    ├── villes.csv           # Base de données des villes (GPS, population, etc.)
    └── temps.csv            # Matrice des durées de trajet inter-villes
```
## ⚙️ Prérequis & Compilation

### Prérequis

- Un compilateur C++ compatible C++17 (GCC, Clang, MSVC).
- **Qt SDK** (Qt 5.15+ ou Qt 6.x) avec les modules :
  - `Core`
  - `Gui`
  - `Widgets`
  - `Network`

### Compilation via qmake (Terminal)

1. Cloner le dépôt :
   ```bash
   git clone [https://github.com/votre-compte/carte-itineraires-qt.git](https://github.com/votre-compte/carte-itineraires-qt.git)
   cd carte-itineraires-qt
   ```
2. Générer le Makefile et compiler :
   ```bash
   qmake
   make -j4
   ```
3. Lancer l'application :
   ```bash
   ./carte-itineraires-qt
   ```
Sinon, vous pouvez ouvrir directement le fichier .pro dans QTCreator

## 📖 Guide d'Utilisation

1. **Calculer un itinéraire :**
   - Sélectionnez une ville de **départ** et une ville d'**arrivée** via les barres de recherche prédictives.
   - *(Optionnel)* Cliquez sur **"+ Ajouter une étape"** pour insérer des points de passage intermédiaires.
   - Cliquez sur **"Calculer Temps"**.
2. **Explorer la carte :**
   - Utilisez la molette de la souris pour **zoomer/dézoomer** ou utilisez le slider de zoom en bas à gauche.
   - Maintenez le clic gauche enfoncé pour **déplacer la carte**.
   - Survolez une ville avec le curseur pour afficher ses informations (nom, population).
3. **Animer le trajet :**
   - Cliquez sur **"▶ Lancer l'animation"** pour visualiser le parcours étape par étape.
4. **Comparer plusieurs villes :**
   - Sélectionnez une ville puis cliquez sur **"Ajouter au comparateur"** pour enrichir le tableau croisé des temps de parcours.
5. **Exporter :**
   - Cliquez sur **"💾 Exporter l'itinéraire"** pour sauvegarder le rendu sous forme d'image.

---
