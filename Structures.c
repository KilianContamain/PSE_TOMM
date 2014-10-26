typedef const TAILLE_NOM 20 // TAille des char court, nom TAILLE_NOM
typedef const TAILLE_POST 140 // Taille des char long TAILLE_POST
typedef const TAILLE_MES 200 // TAille des message à transferer : TAILLE_MES
typedef const TAILLE_PW 8 // Taille des mots de passe en octet

// Fichiers Headers de structures :
typedef struct Date{
    int Jour;
    int Mois;
    int Annee;
    int Heure;
};

typedef struct Thread{
    int Th_Id; // Identifiant du Thread
    char Nom[TAILLE_NOM]; // Nom du thread
    Date *Debut; // Date de Début généré automatiquement
    Date *Fin; // date de Fin spécifiée par l'utilisateur
    int nbPost; // Nombre de poste dans ce thread
    Post** Contenu; // Tableau sur les postes contenus ds le Thread
};

typedef struct Post{
    int Th_Id; // Identifiant du thread correspondant
    int Num_Pst; // Numero du post
    Date *Date_Post; // Date du post, générée automatiquement
    char Auteur[TAILLE_NOM]; // Nom de l'auteur
    int Id_Auteur; // Identifiant de l'auteur
    char Text[TAILLE_POST]; //
};
// A noter que le passge de pointeur risque d'"être compliqué" par socket TCP entre 2 PC différents !!
typedef struct Message{ // Idéalement c'est ce que l'on transmet par Internet mais bon si faut tout passer on passera tout
    char mes[TAILLE_MES]; // texte du message
    int descript; // descripteur du type de message
    bool ChkSum; // non implémenté
};

typedef struct Utilisateur{ // Tous ces Utilisateurs sont Sauvegarder ds un fichier sur le  Serveur, encrypter ?
    int ID_Ut; // Identifiant de l'utilisateur
    char Nom_Ut[TAILLE_NOM]; // Nom de l'utilisateur tel qu'il apparait dans les post
    char PwU[TAILLE_PW]; // Mot de passe !!! Ce serait bien de ne pas l'afficher à la saisie
};




