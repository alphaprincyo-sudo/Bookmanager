# Book Manager (Qt 6, C++ / Qt Widgets + SQLite)

## Lancer
Qt Creator → ouvrir `CMakeLists.txt` (Qt 6.4+, modules Widgets, Sql, Svg) → Ctrl+R.
La base `bookmanager.db` est créée dans le dossier d'exécution.
Pour voir l'appli remplie : Paramètres → « Charger des données de démonstration ».

## Modèle (SQLite)
livre(id_livre, titre, auteur, isbn, date_ajout) · exemplaire(id_exemplaire, id_livre, code_barre UNIQUE, etat)
lecteur(id_lecteur, nom, matricule UNIQUE, telephone) · pret(id_pret, id_exemplaire, id_lecteur, date_pret, date_limite, date_retour)
attente(id_attente, id_livre, id_lecteur, date_demande, statut)

## Règles
- Tous les exemplaires empruntés → on peut rejoindre la liste d'attente du livre.
- Au retour d'un exemplaire, le premier en attente devient « Prioritaire » : l'exemplaire lui est réservé (statut du livre « Réservé »).
- Durée de prêt et nombre max de prêts par lecteur : page Paramètres.

## Fichiers
- `mainwindow.ui`, `parametrespage.ui` : Qt Designer (la barre latérale est un QFrame promu en `SidebarFrame`)
- `data.cpp` : toutes les requêtes SQL et règles métier
- `dialogs.cpp` : fiche livre (double-clic) et ajout/modification d'un livre
- `pages.cpp`, `widgets.cpp`, `style.qss`, `icons/`
