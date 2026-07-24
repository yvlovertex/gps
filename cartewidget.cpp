#include "cartewidget.h"
#include <QPainter>
#include <QDir>
#include <QDebug>
#include <QtMath>
#include <QToolTip>
#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <algorithm>
#include <utility>

CarteWidget::CarteWidget(QWidget* parent) : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    this->setScene(m_scene);


    this->setDragMode(QGraphicsView::ScrollHandDrag); //curseur devient une main
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); //zoom se concentre sur le curseur
    this->setRenderHint(QPainter::Antialiasing); //lisse les bords
    this->setRenderHint(QPainter::SmoothPixmapTransform); // ameliore image et zoom

    //chargement de l'image de fond
    if (!fondPixmap.load("data/france_map.png")) {
        qDebug() << "ERREUR : Image non trouvée";
        m_scene->setSceneRect(0, 0, 2000, 2000);
    } else {
        QGraphicsPixmapItem* bg = m_scene->addPixmap(fondPixmap);
        bg->setZValue(-1); // pour quil reste derriere
        m_scene->setSceneRect(fondPixmap.rect());
    }

    // timer pour l'anim du point jaune sur le trajet
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() { //toutes les X millisecondes le code entre {} est executé
        if (cheminActuel.empty()) return;

        indexAnimation++; //passe a ville suivante
        if (indexAnimation >= (int)cheminActuel.size()) { //fin du voyage
            animationTimer->stop();
            animationEnCours = false;
        }
        updateDessin(); // refait le dessin pour deplacer le point jaune
    });
}

void CarteWidget::demarrerAnimation() {
    if (cheminActuel.size() < 2) return; // pas besoin d'anim
    indexAnimation = 0;
    animationEnCours = true;
    animationTimer->start(300); // une ville toute les 300ms
}

void CarteWidget::wheelEvent(QWheelEvent* event) {
    // facteur de zoom
    double scaleFactor = 1.15;

    // recup le zoom actuel (m11 est le ratio horizontal)
    double oldZoom = transform().m11();
    double nextZoom;

    // calcul cible
    if (event->angleDelta().y() > 0) {
        nextZoom = oldZoom * scaleFactor;
    } else {
        nextZoom = oldZoom / scaleFactor;
    }

    // appliquer limites entre 1x et 10x
    if (nextZoom < 1.0) nextZoom = 1.0;
    if (nextZoom > 10.0) nextZoom = 10.0;

    //calcul le ratio relatif pour l'appliquer a la vue
    double relativeScale = nextZoom / oldZoom;
    this->scale(relativeScale, relativeScale);

    // synchroniser le reste de l'application
    emit zoomChanged(nextZoom); // pour informer le slider que la valeur du zoom a changé

    this->updateDessin(); // force le redessin
}

void CarteWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF scenePos = mapToScene(event->pos());
    bool villeTrouvee = false;

    //tooltip au survol des villes
    for (const Ville& v : villes) {
        if (m_coordsProjetees.contains(v.id)) {
            QPointF vPos = m_coordsProjetees[v.id];
            if (QLineF(scenePos, vPos).length() < 10.0 / transform().m11()) {
                QToolTip::showText(event->globalPosition().toPoint(),
                                   QString::fromStdString(v.nom) + "\nPopulation: " + QString::number(v.population),
                                   this);
                villeTrouvee = true;
                break;
            }
        }
    }

    if (!villeTrouvee) {
        QToolTip::hideText();
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CarteWidget::ajouterTrajetPermanent(const std::vector<int>& chemin, QColor couleur) {
    trajetsPermanents.append({chemin, couleur});
    this->updateDessin();
}

void CarteWidget::setChemin(const std::vector<int>& chemin) {
    this->cheminActuel = chemin;
    this->updateDessin();
}

void CarteWidget::setVilles(const std::vector<Ville>& v) {
    this->villes = v;
    this->updateDessin();
}

void CarteWidget::selectionnerVilleDepart(int id) {
    idVilleDepart = id;
    updateDessin();
}

void CarteWidget::selectionnerVilleArrivee(int id) {
    idVilleArrivee = id;
    updateDessin();
}

void CarteWidget::dessinerCourbe(QPointF p1, QPointF p2, QColor coul) {
    //crée un effet de courbe entre deux points
    QPainterPath path(p1);
    QPointF mid = (p1 + p2) / 2;
    qreal dist = QLineF(p1, p2).length();
    //crée un point de controle vers le haut (Y-) proportionnellement a la distance
    mid.setY(mid.y() - dist * 0.15);

    path.quadTo(mid, p2);

    QPen pen(coul, 2);
    pen.setCosmetic(true); //l'épaisseur reste de 2px peu importe le zoom
    m_scene->addPath(path, pen);
}

void CarteWidget::updateDessin() {
    if (!m_scene || villes.empty()) return;

    m_scene->clear();
    m_coordsProjetees.clear();// On vide la carte avant de recalculer

    //fond de carte
    if (!fondPixmap.isNull() && imageFondVisible) {
        QGraphicsPixmapItem* bg = m_scene->addPixmap(fondPixmap);
        bg->setZValue(-1);
    }
    // si image cachée alors on met un fond gris
    if (!imageFondVisible) {
        m_scene->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    } else {
        m_scene->setBackgroundBrush(Qt::NoBrush);
    }

    double currentZoom = transform().m11();

    //calcul des bornes
    double minLat = villes[0].latitude, maxLat = villes[0].latitude;
    double minLon = villes[0].longitude, maxLon = villes[0].longitude;
    for (const Ville& v : villes) {
        minLat = std::min(minLat, (double)v.latitude); //() force le type de variable (pour min et max)
        maxLat = std::max(maxLat, (double)v.latitude);
        minLon = std::min(minLon, (double)v.longitude);
        maxLon = std::max(maxLon, (double)v.longitude);
    }

    //difference entre le point le plus haut et le plus bas/gauche droite (+ securité si jamais divise par 0)
    double latRange = (maxLat - minLat == 0) ? 1.0 : (maxLat - minLat);
    double lonRange = (maxLon - minLon == 0) ? 1.0 : (maxLon - minLon);

    // transforme les coordonnées gps en pixels d'image
    //ma marge
    double margeWidth = 350.0;
    double margeHeight = 50.0;
    double drawWidth = fondPixmap.width() - (2 * margeWidth);
    double drawHeight = fondPixmap.height() - (2 * margeHeight);

    for (const Ville& v : villes) {
        double x = ((v.longitude - minLon) / lonRange * drawWidth) + margeWidth;
        // En pixels le Y est inversé par rapport aux latitudes
        double y = ((maxLat - v.latitude) / latRange * drawHeight) + margeHeight;
        m_coordsProjetees[v.id] = QPointF(x, y);
    }

    //dessin des trajets
    auto dessinerLignes = [&](const std::vector<int>& chemin, QColor coul, int epaisseur, int z) { //[&] donne l'acces aux variables alentours
        if (chemin.size() < 2) return;
        for (size_t i = 0; i < chemin.size() - 1; ++i) { // parcours 2 par 2
            if (m_coordsProjetees.contains(chemin[i]) && m_coordsProjetees.contains(chemin[i+1])) {
                dessinerCourbe(m_coordsProjetees[chemin[i]], m_coordsProjetees[chemin[i+1]], coul); //dessine la courbe entre 2 points
            }
        }
    };

    dessinerLignes(cheminActuel, Qt::magenta, 3, 2);
    for (int i = 0; i < trajetsPermanents.size(); ++i) {
        dessinerLignes(trajetsPermanents[i].ids, trajetsPermanents[i].couleur, 2, 1);
    }

    // affichage des points avec le lod
    for (const Ville& v : villes) {
        bool visibleZoom = false;
        QString typeVille = QString::fromStdString(v.capital);

        // visibile par rapport au niveau de zoom
        if (typeVille == "primary") {
            // Palier 1 : Uniquement la capitale au début
            visibleZoom = (currentZoom > 1 && currentZoom < 2.5);
        }
        else if (typeVille == "admin") {
            // Palier 2 : Les préfectures apparaissent quand Paris disparaît
            // et s'effacent quand on veut voir le détail
            visibleZoom = (currentZoom > 1 && currentZoom < 2.5);
        }
        else {
            // Palier 3 : Le détail (minor et autres) n'apparaît qu'en zoom élevé
            visibleZoom = (currentZoom >= 2.5);
        }

        //EXCEPTION : On force l'affichage si la ville fait partie du trajet calculé
        //cela permet de toujours voir son départ/arrivée même si on dézoome (zoom 1x)
        bool estVital = (v.id == idVilleDepart || v.id == idVilleArrivee ||
                         std::find(cheminActuel.begin(), cheminActuel.end(), v.id) != cheminActuel.end());

        if (!visibleZoom && !estVital) continue;

        // --- DESSIN (Conservé à l'identique) ---
        if (!m_coordsProjetees.contains(v.id)) continue;
        QPointF pos = m_coordsProjetees[v.id];


        // Taille du point : plus gros pour les villes vitales (trajet); qreal pour le + de précision possible!!
        qreal r = (estVital ? 7.0 : (3.0 + qSqrt(v.densite) / 10.0)) / currentZoom; //si c une ville de depart ou d'arrivée on donne taille de 7
                                    //sinon on calcule la taille selon la densité (current zoom pour eviter que en x10 le point soit enorme)
        //code couleur: rouge(départ), vert(arrivée), magenta(trajet), bleu(reste)
        QColor coul = Qt::blue;
        if (v.id == idVilleDepart) coul = Qt::red;
        else if (v.id == idVilleArrivee) coul = Qt::green;
        else if (estVital) coul = Qt::magenta;

        QPen p(Qt::white);
        p.setWidthF(1.5 / currentZoom); // Contour fin proportionnel au zoom

        QGraphicsEllipseItem* dot = m_scene->addEllipse(QRectF(pos.x() - r, pos.y() - r, r*2, r*2), p, QBrush(coul));
        dot->setZValue(10);

        //Affichage du nom de la ville
        if (!v.nom.empty()) {
            QGraphicsTextItem* txt = m_scene->addText(QString::fromStdString(v.nom));
            txt->setScale(1.0 / currentZoom); //texte lisible quel que soit le zoom
            txt->setPos(pos.x() + r, pos.y() - (15.0 / currentZoom));
            txt->setZValue(11);
            if (estVital) {
                QFont f = txt->font(); f.setBold(true); txt->setFont(f);
            }
        }

    }

    //animation du curseur jaune
    if (animationEnCours && indexAnimation < (int)cheminActuel.size()) {
        int idVilleAnim = cheminActuel[indexAnimation];
        if (m_coordsProjetees.contains(idVilleAnim)) {
            QPointF pos = m_coordsProjetees[idVilleAnim];
            QGraphicsEllipseItem* cursor = m_scene->addEllipse(pos.x()-5, pos.y()-5, 10, 10, QPen(Qt::black), QBrush(Qt::yellow));
            cursor->setZValue(100); //pour premier plan
        }
    }
}

void CarteWidget::setAfficherImageFond(bool visible) {
    imageFondVisible = visible;
    updateDessin(); // On redessine tout
}
