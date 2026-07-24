#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout> // Ajouté pour le layout principal
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QtNetwork>
#include <QSlider>
#include <QScrollArea>
#include <QGroupBox>
#include "cartewidget.h"
#include "lecturecsv.h"
#include "graphe.h"
#include <map>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void changerDepart(int index);
    void changerArrivee(int index);
    void changerEtape(int index);
    void calculerTrajet();

    void mettreAJourMeteo(const QString& ville, int type);
    void ajouterNouvelleEtape();
    void rafraichirInfosEtapes();

    void ajouterAuTableau();
    void supprimerVilleDuTableau();
    void exporterItineraire();

private:
    void setupSmartCombo(QComboBox* combo, const QString& placeholder);
    QGroupBox* creerGroupeInfo(int idVille, QString titre);
    void lancerMeteoSpecifique(const QString& ville, QLabel* labelCible); // Pour rafraichirInfosEtapes

    CarteWidget *carte;

    QComboBox *comboDepart;
    QComboBox *comboArrivee;
    QComboBox *comboEtape;

    QPushButton *btnCalculer;
    QPushButton *btnAnimer;
    QPushButton *btnExporter;
    QPushButton *btnToggleCarte;
    QPushButton *btnAjouterEtape;
    QPushButton *btnAjouterTableau;
    QPushButton *btnSupprimerLigne;

    QLabel *labelResultat;
    QLabel *labelChemin;
    QLabel *labelZoomMenu;

    QLabel *labelNomDep, *labelPopDep, *labelRegDep, *labelDensiteDep, *labelMeteoDep;
    QLabel *labelNomArr, *labelPopArr, *labelRegArr, *labelDensiteArr, *labelMeteoArr;
    QLabel *labelNomEta, *labelPopEta, *labelRegEta, *labelDensiteEta, *labelMeteoEta;

    QTableWidget* tableauComparatif;
    QSlider* sliderZoom;

    QVBoxLayout *layoutGauche;
    QVBoxLayout *layoutListeEtapes;
    QHBoxLayout *mainLayout;

    QScrollArea *scrollAreaDroite;
    QWidget *conteneurScroll;
    QVBoxLayout *layoutScroll;

    std::vector<Ville> villes;
    Graphe* graphe;
    std::map<int, int> idToIndex;
    QList<int> villesDansTableau;
    QList<QComboBox*> listeComboEtapes;
};

#endif
