// Structure implémentées :
typedef struct Date{
    int Jour;
    int Mois;
    int Annee;
    int Heure;
};

typedef struct Post{
    int Th_Id; // Identifiant du thread correspondant
    int Num_Pst; // Numero du post
    Date *Date_Post; // Date du post, générée automatiquement
    char Auteur[TAILLE_NOM]; // Nom de l'auteur
    int Id_Auteur; // Identifiant de l'auteur
    char Text[TAILLE_POST]; //
};



// Fonctions annexes :
void read_Message(Message *Mes_Lu);

void write_Message(Message *Mes_Ec);

Date *btenir_Date(Date *Nouvelle_Date); // générer la date actuelle, sur le serveur


void read_Message(Message *Mes_Lu){





}

void write_Message(Message *Mes_Ec){




}

Date *Obtenir_Date(){ // Renvoi la date actuelle du serveur
    // Suffit de trouver comment on fait :)
    Date* Nouvelle_Date;

    return Nouvelle_Date;
}

void Remplir_Post(Post *Nouveau_Post){ // Fonction serveur remplit le nouveau un nouveau post
    // Remplir l'auteur qui doit etre identifié
    Nouveau_Post.Id_Auteur = ;
    Nouveau_Post.Auteur = ;

    Nouveau_Post.Date_Post = Obtenir_Date(); // date du post

    Nouveau_Post.Th_Id =  ;  // Remplir le thread

    Nouveau_Post.Num_Pst = // Nombre de post dans le thread + 1, mettre a jour le nombre de post ds le thread

    Nouveau_Post.Text = ; // Lire ce qui est envoyé dans par le client
}

void Instance_Client(){// Je sais pas ce que l'on passe, Utilisateur ou autre ... il doit faloir un identifint pour le client ou autre...
    // La connection est établie

}



