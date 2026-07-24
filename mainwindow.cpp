#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QCompleter>
#include <QLineEdit>
#include <QFileDialog>
#include <QTextStream>
#include "floydwarshall.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // style de la window
    this->setStyleSheet(
        "QMainWindow { background-color: #121212; }"
        "QLabel { color: white; }"
        "QComboBox { background-color: #2D2D2D; color: white; border: 1px solid #3F3F3F; }"
        "QPushButton { background-color: #3D3D3D; color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: #505050; }"
        "QGroupBox { color: white; border: 1px solid #3F3F3F; margin-top: 10px; padding-top: 10px; font-weight: bold; }"
        );

    // init et precalculs des donnees
    villes = CSVReader::lireVilles("data/villes.csv");
    for(int i = 0; i < (int)villes.size(); ++i) {
        idToIndex[villes[i].id] = i;
    }
    graphe = new Graphe(villes.size());
    CSVReader::lireTemps("data/temps.csv", *graphe, villes);
    FloydWarshall::calculer(*graphe); // calcul de fw

    // création widget/interface gauche
    comboDepart = new QComboBox();
    comboArrivee = new QComboBox();
    btnCalculer = new QPushButton("Calculer Temps");
    labelResultat = new QLabel("Temps : -");
    labelResultat->setStyleSheet("font-size: 14px; color: #4CAF50; font-weight: bold;");

    labelChemin = new QLabel("");
    labelChemin->setWordWrap(true);

    btnAnimer = new QPushButton("▶ Lancer l'animation");
    btnExporter = new QPushButton("💾 Exporter l'itinéraire");
    btnAjouterEtape = new QPushButton("+ Ajouter une étape");

    btnAjouterTableau = new QPushButton("Ajouter au comparateur");
    tableauComparatif = new QTableWidget(0, 0);
    tableauComparatif->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableauComparatif->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    btnSupprimerLigne = new QPushButton("Supprimer ville sélectionnée");
    btnSupprimerLigne->setStyleSheet("color: white; background-color: #d32f2f;");

    labelZoomMenu = new QLabel("Zoom : 1.00x");
    sliderZoom = new QSlider(Qt::Horizontal);
    sliderZoom->setRange(100, 1000); // equivaut à un zoom de 1.0x à 10.0x
    sliderZoom->setValue(100);

    btnToggleCarte = new QPushButton("Masquer l'image de fond");
    btnToggleCarte->setCheckable(true);

    // organisation du layout gauche
    layoutGauche = new QVBoxLayout();
    layoutGauche->addWidget(new QLabel("<b>Départ :</b>"));
    layoutGauche->addWidget(comboDepart);
    layoutGauche->addWidget(new QLabel("<b>Arrivée :</b>"));
    layoutGauche->addWidget(comboArrivee);

    layoutGauche->addWidget(new QLabel("<b>Étapes intermédiaires :</b>"));
    layoutListeEtapes = new QVBoxLayout(); // Sous-conteneur dédié aux étapes
    layoutGauche->addLayout(layoutListeEtapes);
    layoutGauche->addWidget(btnAjouterEtape);

    layoutGauche->addWidget(btnCalculer);
    layoutGauche->addWidget(labelResultat);
    layoutGauche->addWidget(labelChemin);
    layoutGauche->addWidget(btnAnimer);
    layoutGauche->addWidget(btnExporter);

    layoutGauche->addSpacing(20);
    layoutGauche->addWidget(new QLabel("<b>Comparateur de distances :</b>"));
    layoutGauche->addWidget(btnAjouterTableau);
    layoutGauche->addWidget(tableauComparatif);
    layoutGauche->addWidget(btnSupprimerLigne);

    layoutGauche->addStretch();
    layoutGauche->addWidget(labelZoomMenu);
    layoutGauche->addWidget(sliderZoom);
    layoutGauche->addWidget(btnToggleCarte);

    // création de la carte (centre)
    carte = new CarteWidget();
    carte->setVilles(villes);

    // création de l'interface droite (le scroll area défilant)
    labelMeteoDep = new QLabel("-");
    labelMeteoArr = new QLabel("-");

    scrollAreaDroite = new QScrollArea();
    scrollAreaDroite->setWidgetResizable(true);
    scrollAreaDroite->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    conteneurScroll = new QWidget();
    layoutScroll = new QVBoxLayout(conteneurScroll);
    layoutScroll->setAlignment(Qt::AlignTop);
    scrollAreaDroite->setWidget(conteneurScroll);

    // assemblage (structure en trois volets)
    QWidget* central = new QWidget();
    mainLayout = new QHBoxLayout(central);
    mainLayout->addLayout(layoutGauche, 2); //menu de contrôle
    mainLayout->addWidget(carte, 5); //zone centrale interactive
    mainLayout->addWidget(scrollAreaDroite, 2); //volet d'informations météo/techniques
    setCentralWidget(central);

    //config des Comboboxes avec la base de données ---
    setupSmartCombo(comboDepart, "Chercher un départ...");
    setupSmartCombo(comboArrivee, "Chercher une arrivée...");

    for(const Ville& v : villes) {
        QString nom = QString::fromStdString(v.nom);
        comboDepart->addItem(nom, v.id);
        comboArrivee->addItem(nom, v.id);
    }

    comboDepart->setCurrentIndex(-1);
    comboArrivee->setCurrentIndex(-1);
    rafraichirInfosEtapes();

    //gestion des Connexions (Signals & Slots)
    connect(comboDepart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changerDepart);
    connect(comboArrivee, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changerArrivee);
    connect(btnCalculer, &QPushButton::clicked, this, &MainWindow::calculerTrajet);
    connect(btnAjouterTableau, &QPushButton::clicked, this, &MainWindow::ajouterAuTableau);
    connect(btnSupprimerLigne, &QPushButton::clicked, this, &MainWindow::supprimerVilleDuTableau);
    connect(btnAnimer, &QPushButton::clicked, [this]() { carte->demarrerAnimation(); });
    connect(btnExporter, &QPushButton::clicked, this, &MainWindow::exporterItineraire);
    connect(btnAjouterEtape, &QPushButton::clicked, this, &MainWindow::ajouterNouvelleEtape);

    //toggle interrupteur pour le masquage/affichage de l'image de fond
    connect(btnToggleCarte, &QPushButton::toggled, [this](bool checked) {
        carte->setAfficherImageFond(!checked);
        btnToggleCarte->setText(checked ? "Afficher l'image de fond" : "Masquer l'image de fond");
    });

    // sync slider vers carte
    connect(sliderZoom, &QSlider::valueChanged, this, [this](int value) {
        double factor = value / 100.0;
        QTransform t;
        t.scale(factor, factor);
        carte->setTransform(t);
        labelZoomMenu->setText(QString("Zoom : %1x").arg(factor, 0, 'f', 2));
        carte->updateDessin();
    });

    // sync roulette souris vers slider
    connect(carte, &CarteWidget::zoomChanged, this, [this](double newZoom) {
        sliderZoom->blockSignals(true); //evite une boucle infinie d'événements
        sliderZoom->setValue(static_cast<int>(newZoom * 100));
        sliderZoom->blockSignals(false);
        labelZoomMenu->setText(QString("Zoom : %1x").arg(newZoom, 0, 'f', 2));
    });
}

void MainWindow::changerDepart(int index) {
    if (index <= 0) return;
    int id = comboDepart->itemData(index).toInt();
    carte->selectionnerVilleDepart(id);
    rafraichirInfosEtapes();
}

void MainWindow::changerArrivee(int index) {
    if (index <= 0) {
        carte->selectionnerVilleArrivee(-1);
        rafraichirInfosEtapes();
        return;
    }
    int id = comboArrivee->itemData(index).toInt();
    carte->selectionnerVilleArrivee(id);
    rafraichirInfosEtapes();
}

void MainWindow::changerEtape(int index) {
    if (index <= 0) {
        labelNomEta->setText("Nom : -");
        labelPopEta->setText("Population : -");
        labelMeteoEta->setText("Météo : -");
        return;
    }

    int id = comboEtape->itemData(index).toInt();
    for(const Ville& v : villes) {
        if(v.id == id) {
            labelNomEta->setText(QString("<b>Nom :</b> %1").arg(QString::fromStdString(v.nom)));
            labelRegEta->setText(QString("<b>Région :</b> %1").arg(QString::fromStdString(v.region)));
            labelPopEta->setText(QString("<b>Pop :</b> %1 hab").arg(v.population));
            labelDensiteEta->setText(QString("<b>Densité :</b> %1 hab/km²").arg(v.densite, 0, 'f', 2));
            labelMeteoEta->setText("<b>Météo :</b> <i>Chargement...</i>");

            mettreAJourMeteo(QString::fromStdString(v.nom), 2); //type 2 = Étape
            break;
        }
    }
}

void MainWindow::calculerTrajet() {
    int idDep = comboDepart->currentData().toInt();
    int idArr = comboArrivee->currentData().toInt();
    if (idDep == -1 || idArr == -1) return;

    // concatene l'itinéraire complet (départ -> etapes... -> arrivée)
    QList<int> points;
    points << idDep;
    for(QComboBox* cb : listeComboEtapes) {
        int id = cb->currentData().toInt();
        if(id != -1) points << id;
    }
    points << idArr;

    std::vector<int> cheminFinalIds;
    int tempsTotalCumule = 0;
    bool trajetPossible = true;

    //calcul par morceaux via le dictionnaire d'indices traduit
    for(int i = 0; i < points.size() - 1; ++i) {
        int u = idToIndex[points[i]];
        int v = idToIndex[points[i+1]];
        int poids = graphe->getMatrice()[u][v];

        if (poids >= 1000000000) { trajetPossible = false; break; } // dectecte si il y a un segment brisé (INF)

        tempsTotalCumule += poids;
        std::vector<int> indices = FloydWarshall::getChemin(u, v);

        // Intégration unifiée du sous-chemin en évitant les doublons aux points de raccord
        for (int idx : indices) {
            int idReel = villes[idx].id;
            if (cheminFinalIds.empty() || cheminFinalIds.back() != idReel)
                cheminFinalIds.push_back(idReel);
        }
    }

    if (!trajetPossible) {
        labelResultat->setText("<b>Trajet impossible</b>");
        labelChemin->setText("");
        carte->setChemin({});
    } else {
        int h = tempsTotalCumule / 60;
        int m = tempsTotalCumule % 60;
        labelResultat->setText(QString("<b>Temps estimé :</b> %1h %2min").arg(h).arg(m));

        // Construction dynamique de la chaîne textuelle du parcours fléché
        QStringList listeNomsChemin;
        for (int id : cheminFinalIds) {
            for (const Ville& v : villes) {
                if (v.id == id) {
                    listeNomsChemin << QString::fromStdString(v.nom);
                    break;
                }
            }
        }

        QString parcoursTexte = "<b>Itinéraire :</b><br>" + listeNomsChemin.join(" ➔ ");
        labelChemin->setText(parcoursTexte);
        labelChemin->setStyleSheet("color: #BBBBBB; font-size: 11px;");

        carte->setChemin(cheminFinalIds); //envoi du trajet définitif à la carte
    }
}

void MainWindow::rafraichirInfosEtapes() {
    // En Qt, supprimer un widget nécessite de le décrocher de son layout avant de libérer sa mémoire
    QLayoutItem *child;
    while ((child = layoutScroll->takeAt(0)) != nullptr) {
        if(child->widget()) {
            child->widget()->hide();
            child->widget()->deleteLater(); // suppression asynchrone sécurisée par Qt
        }
        delete child;
    }

    int idDep = (comboDepart->currentIndex() < 0) ? -1 : comboDepart->currentData().toInt();
    layoutScroll->addWidget(creerGroupeInfo(idDep, "Départ"));

    int idArr = (comboArrivee->currentIndex() < 0) ? -1 : comboArrivee->currentData().toInt();
    layoutScroll->addWidget(creerGroupeInfo(idArr, "Arrivée"));

    int i = 1;
    for(QComboBox* cb : listeComboEtapes) {
        int id = cb->currentData().toInt();
        if(id != -1) {
            layoutScroll->addWidget(creerGroupeInfo(id, QString("Étape %1").arg(i++)));
        }
    }
}

void MainWindow::ajouterNouvelleEtape() {
    QWidget* ligne = new QWidget();
    QHBoxLayout* l = new QHBoxLayout(ligne);
    l->setContentsMargins(0, 2, 0, 2);

    QComboBox* cb = new QComboBox();
    cb->addItem("- Choisir étape -", -1);
    for(const Ville& v : villes) cb->addItem(QString::fromStdString(v.nom), v.id);
    setupSmartCombo(cb, "Ville...");

    QPushButton* btnSuppr = new QPushButton("✕");
    btnSuppr->setFixedWidth(30);
    btnSuppr->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold;");

    l->addWidget(cb);
    l->addWidget(btnSuppr);
    layoutListeEtapes->addWidget(ligne);
    listeComboEtapes.append(cb);

    //connexions pour mettre à jour l'affichage en temps réel lors d'une modification ou suppression
    connect(cb, &QComboBox::currentIndexChanged, this, &MainWindow::rafraichirInfosEtapes);
    connect(btnSuppr, &QPushButton::clicked, [this, ligne, cb](){
        listeComboEtapes.removeAll(cb);
        ligne->deleteLater();
        //délai de 10ms pour laisser Qt détruire l'objet proprement avant de refresh l'espace
        QTimer::singleShot(10, this, &MainWindow::rafraichirInfosEtapes);
    });
}

QGroupBox* MainWindow::creerGroupeInfo(int idVille, QString titre) {
    QGroupBox* gb = new QGroupBox(titre);
    QVBoxLayout* l = new QVBoxLayout(gb);
    l->setSpacing(2);

    if(idVille == -1) {
        l->addWidget(new QLabel("<i>Aucune sélection</i>"));
        return gb;
    }

    for(const Ville& v : villes) {
        if(v.id == idVille) {
            QLabel* lblNom = new QLabel(QString::fromStdString(v.nom));
            lblNom->setStyleSheet("font-size: 14px; font-weight: bold; color: #4DB6AC;");
            l->addWidget(lblNom);

            l->addWidget(new QLabel(QString("<b>Région :</b> %1").arg(QString::fromStdString(v.region))));
            l->addWidget(new QLabel(QString("<b>Population :</b> %1 hab").arg(v.population)));
            l->addWidget(new QLabel(QString("<b>Densité :</b> %1 hab/km²").arg(v.densite, 0, 'f', 1)));

            QFrame* line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            line->setStyleSheet("background-color: #3F3F3F;");
            l->addWidget(line);

            QLabel* lblMeteo = new QLabel("<i>Chargement météo...</i>");
            lblMeteo->setWordWrap(true);
            l->addWidget(lblMeteo);
            lancerMeteoSpecifique(QString::fromStdString(v.nom), lblMeteo);

            break;
        }
    }
    return gb;
}

void MainWindow::mettreAJourMeteo(const QString& ville, int typeVille) {
    // requete http
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = "https://api.openweathermap.org/data/2.5/weather?q=" + ville +
                  ",france&appid=4ef32d6568211765a2ba66120c6b9c48&units=metric&lang=fr";

    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        QString info;

        if (reply->error() == QNetworkReply::NoError) {
            //lecture brute du flux réseau et json
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();

            // navig dans l'arbre json
            QJsonObject main = obj["main"].toObject();
            double temp = main["temp"].toDouble();
            int humidite = main["humidity"].toInt();

            QJsonArray weatherArray = obj["weather"].toArray();
            QString desc = weatherArray[0].toObject()["description"].toString();
            desc = desc.left(1).toUpper() + desc.mid(1);

            QJsonObject wind = obj["wind"].toObject();
            double vitesseVent = wind["speed"].toDouble() * 3.6; // conversion m/s -> km/h

            QJsonObject sys = obj["sys"].toObject();
            QString lever = QDateTime::fromSecsSinceEpoch(sys["sunrise"].toVariant().toLongLong()).toString("hh:mm");
            QString coucher = QDateTime::fromSecsSinceEpoch(sys["sunset"].toVariant().toLongLong()).toString("hh:mm");

            info = QString(
                       "<div style='line-height: 130%;'>"
                       "<b>Météo :</b> %1<br>"
                       "<b>Température :</b> %2°C (Hum. %3%)<br>"
                       "<b>Vent :</b> %4 km/h<br>"
                       "<b>Soleil :</b> ☀️ %5 / 🌙 %6"
                       "</div>")
                       .arg(desc).arg(temp, 0, 'f', 1).arg(humidite).arg(vitesseVent, 0, 'f', 0).arg(lever).arg(coucher);
        } else {
            info = "<b>Météo :</b> Indisponible";
        }

        switch(typeVille) {
        case 0: labelMeteoDep->setText(info); break;
        case 1: labelMeteoArr->setText(info); break;
        case 2: labelMeteoEta->setText(info); break;
        }

        reply->deleteLater();
        manager->deleteLater();
    });

    manager->get(QNetworkRequest(QUrl(url)));
}

void MainWindow::ajouterAuTableau() {
    int idVille = comboDepart->currentData().toInt();
    if (idVille == -1) return;

    for(int id : villesDansTableau) if(id == idVille) return; // anti doublons

    villesDansTableau.append(idVille);
    int n = villesDansTableau.size();

    tableauComparatif->setRowCount(n);
    tableauComparatif->setColumnCount(n);

    QStringList noms;
    for (int id : villesDansTableau) {
        for (const Ville& v : villes) {
            if (v.id == id) { noms << QString::fromStdString(v.nom); break; }
        }
    }
    tableauComparatif->setHorizontalHeaderLabels(noms);
    tableauComparatif->setVerticalHeaderLabels(noms);

    QColor bleuFonce(25, 25, 112);

    // tableau croisé
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                QTableWidgetItem *diag = new QTableWidgetItem(".");
                diag->setTextAlignment(Qt::AlignCenter);
                diag->setBackground(Qt::lightGray);
                tableauComparatif->setItem(i, j, diag);
                continue;
            }

            int idxI = idToIndex[villesDansTableau[i]];
            int idxJ = idToIndex[villesDansTableau[j]];
            int t = graphe->getMatrice()[idxI][idxJ];
            QString texte;
            if (t >= 1000000000) texte = "∞";
            else {
                int h = t / 60;
                int m = t % 60;
                texte = (m == 0) ? QString("%1h").arg(h) : QString("%1h%2").arg(h).arg(m, 2, 10, QChar('0'));
            }

            QTableWidgetItem *item = new QTableWidgetItem(texte);
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(bleuFonce);
            tableauComparatif->setItem(i, j, item);
        }
    }

    // reset des controles après ajout
    comboDepart->setCurrentIndex(0);
    comboArrivee->setCurrentIndex(0);
    labelResultat->setText("Temps : -");
    labelChemin->setText("");
    carte->setChemin({});
}

void MainWindow::supprimerVilleDuTableau() {
    int row = tableauComparatif->currentRow();
    if (row < 0) return;

    villesDansTableau.removeAt(row);
    tableauComparatif->removeRow(row);
    tableauComparatif->removeColumn(row); // maintient le tableau parfaitement carré

    QStringList noms;
    for (int id : villesDansTableau) {
        for (const Ville& v : villes) {
            if (v.id == id) { noms << QString::fromStdString(v.nom); break; }
        }
    }
    tableauComparatif->setHorizontalHeaderLabels(noms);
    tableauComparatif->setVerticalHeaderLabels(noms);
}

void MainWindow::exporterItineraire() {
    if (labelChemin->text().isEmpty()) return;

    QString nomFichier = QFileDialog::getSaveFileName(this,
                                                      "Sauvegarder l'itinéraire",
                                                      "mon_trajet.png",
                                                      "Images (*.png);;Images JPEG (*.jpg)");

    if (nomFichier.isEmpty()) return;

    // capture sous forme matricielle de pixels (QPixmap) le rendu exact de la vue carte
    QPixmap capture = carte->grab();
    capture.save(nomFichier); // enregistrement physique
}

void MainWindow::setupSmartCombo(QComboBox* combo, const QString& placeholder) {
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->lineEdit()->setPlaceholderText(placeholder);

    //moteur de recherche prédictive (auto complétion filtrante)
    QCompleter* completer = new QCompleter(combo->model(), combo);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setFilterMode(Qt::MatchContains); //recherche inclusive (Ex: "ris" trouvera "Paris")
    combo->setCompleter(completer);
}

void MainWindow::lancerMeteoSpecifique(const QString& ville, QLabel* labelCible) {
    //version ultra-allégée du traitement pour l'affichage condensé des QGroupBox
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = "https://api.openweathermap.org/data/2.5/weather?q=" + ville +
                  ",france&appid=4ef32d6568211765a2ba66120c6b9c48&units=metric&lang=fr";

    connect(manager, &QNetworkAccessManager::finished, [labelCible, manager](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject main = doc.object()["main"].toObject();
            double temp = main["temp"].toDouble();
            QString desc = doc.object()["weather"].toArray()[0].toObject()["description"].toString();

            labelCible->setText(QString("<b>%1°C</b>, %2").arg(temp, 0, 'f', 1).arg(desc));
        } else {
            labelCible->setText("Météo indisponible");
        }
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->get(QNetworkRequest(QUrl(url)));
}

MainWindow::~MainWindow() {
    delete graphe;
}
