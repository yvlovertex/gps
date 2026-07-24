#ifndef CARTEWIDGET_H
#define CARTEWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QWheelEvent>
#include <QMap>
#include <vector>
#include "ville.h"
#include <QTimer>
#include <QVariantAnimation>

//structure pour stocker un itinéraire sauvegardé sur la carte
struct TrajetColore {
    std::vector<int> ids; // liste des id de villes composant le trajet
    QColor couleur; // couleur d'affichage
};

//widget personnalisé pour l'affichage de la carte de France et des données GPS
class CarteWidget : public QGraphicsView {
    Q_OBJECT
public:
    explicit CarteWidget(QWidget *parent = nullptr);

    // config des données
    void setVilles(const std::vector<Ville>& v);
    void selectionnerVilleDepart(int id);
    void selectionnerVilleArrivee(int id);
    void setChemin(const std::vector<int>& chemin);
    void ajouterTrajetPermanent(const std::vector<int>& chemin, QColor couleur);

    // controles d'affichage
    void setZoomFactor(double factor) { m_zoomFactor = factor; }
    void updateDessin(); // force le rafraichissement
    void setAfficherImageFond(bool visible);
    void demarrerAnimation(); // pour le curseur jaune
    void dessinerCourbe(QPointF p1, QPointF p2, QColor coul);
protected:
    // events qt pour interactiviter
    void wheelEvent(QWheelEvent *event) override; // zoom a la souris
    void mouseMoveEvent(QMouseEvent *event) override; // infos au survol
signals:
    void zoomChanged(double newZoom); //sync avec le slider de mainwindow
private:
    QGraphicsScene *m_scene;
    QPixmap fondPixmap; // image de la carte de France
    std::vector<Ville> villes; // base de données locale des villes

    // etat de la sélection actuelle
    int idVilleDepart = -1;
    int idVilleArrivee = -1;
    std::vector<int> cheminActuel;
    QList<TrajetColore> trajetsPermanents; // anciens trajets mémorisés

    // mise en cache des coordonnées (évite de recalculer la projection à chaque frame)
    QMap<int, QPointF> m_coordsProjetees;

    bool imageFondVisible = true;
    double m_zoomFactor = 1.0;

    // gestion de l'animation (jaune qui se deplace)
    QTimer* animationTimer;
    int indexAnimation = 0;
    bool animationEnCours = false;

    // (LOD) pour l'affichage progressif
    const double ZOOM_MAX_GROSSES_VILLES = 1.5;
    const double ZOOM_MIN_VILLAGES = 2.6;
    const double ZOOM_MAX_VILLAGES = 5.0;
};

#endif
