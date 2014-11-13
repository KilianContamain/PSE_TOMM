#include "pse.h"

#define    CMD      "serveur"

/*typedef struct Post{
	int Th_Id; // Identifiant du thread correspondant
	int Num_Pst; // Numero du post
	Date *Date_Post; // Date du post, générée automatiquement
	char Auteur[TAILLE_NOM]; // Nom de l'auteur
	int Id_Auteur; // Identifiant de l'auteur
	char Text[TAILLE_POST]; //
};*/

/* Structure d en-tete */
typedef struct En_tete{
	int nb_usr;												// Nombre d utilisateurs deja enregistres
	struct tm date_mod;										// Date de derniere modification (struct tm est une structure de <time.h>)
} En_tete;


typedef struct Utilisateur{ // Tous ces Utilisateurs sont Sauvegarder ds un fichier sur le Serveur, ici il est mlis de tel facon que seul le propriétaire peut lire le fichier

int ID_Ut; // Identifiant de l'utilisateur
int droit;// droit = 0 pour un client normal et droit = 1 pour un administrateur
char Nom_Ut[10]; // Nom de l'utilisateur tel qu'il apparait dans les post
char PwU[10]; // Mot de passe !!! Ce serait bien de ne pas l'afficher à la saisie
} User;



/* Donnees partagees par les differents threads */
int comptes;
char logi[10],motdp[10];
time_t now;
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

void create_count(void);
void effacercompte(void);
void authenticate(void);
void afficher(En_tete *en_tete);


int main(int argc, char *argv[]) {
  int ecoute, canal, ret, log, arret = FAUX, nblus, mode,lus;
  char texte[LIGNE_MAX];
  struct sockaddr_in adrEcoute, reception;
  socklen_t receptionlen;
  short port;
  
  if (argc != 2) {
    erreur("usage: %s port\n", argv[0]);
  }

  mode = O_WRONLY|O_APPEND|O_CREAT;
  log = open("journal.log", mode, 0660);
  if (log == -1) {
    erreur_IO("open log");
  }

  port = (short) atoi(argv[1]);
  
  printf("%s: creating a socket\n", CMD);
  ecoute = socket (AF_INET, SOCK_STREAM, 0);
  if (ecoute < 0) {
    erreur_IO("socket");
  }
  
  adrEcoute.sin_family = AF_INET;
  adrEcoute.sin_addr.s_addr = INADDR_ANY;
  adrEcoute.sin_port = htons(port);
  printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
  ret = bind (ecoute,  (struct sockaddr *) &adrEcoute, sizeof(adrEcoute));
  if (ret < 0) {
    erreur_IO("bind");
  }
  
  printf("%s: listening to socket\n", CMD);
  ret = listen (ecoute, 5);
  if (ret < 0) {
    erreur_IO("listen");
  }

  receptionlen = sizeof(reception);
  printf("%s: accepting a connection\n", CMD);
  canal = accept(ecoute, (struct sockaddr *) &reception, &receptionlen);
  if (canal < 0) {
    erreur_IO("accept");
  }
  printf("%s: adr %s, port %hu\n", CMD,
	 stringIP(ntohl(reception.sin_addr.s_addr)),
	 ntohs(reception.sin_port));

  while (arret == FAUX) {
    mode |= O_TRUNC;
    nblus = lireLigne (canal, texte);
    if (nblus == -1) {
      erreur_IO("lireLigne");
    }
    else if (nblus == LIGNE_MAX) {
      erreur("ligne trop longue\n");
    }
    else if (nblus == 0) {
      continue;
    }
    else {
      if (strcmp(texte, "fin") == 0) {
	printf("Serveur. arret demande.\n");
	arret = VRAI;
	continue;
      }
      else if (strcmp(texte, "init") == 0) {
	printf("Serveur. remise a zero du journal demandee.\n");
	if (close(log) == -1) {
	  erreur_IO("close log");
	}
	log = open("journal.log", mode, 0660);
	if (log == -1) {
	  erreur_IO("open trunc log");
	}
      }

      else if (strcmp(texte, "1") == 0) {
	printf("Authentification de l'utilisateur:\n");
	lus=lireLigne(canal,logi);
	printf("%d\n",lus);
	lus=lireLigne(canal,motdp);
	printf("%d\n",lus);
	authenticate();
      }

   else if (strcmp(texte, "2") == 0) {
	printf("Création du compte :\n");
	lus=lireLigne(canal,logi);
	printf("%d\n",lus);
	lus=lireLigne(canal,motdp);
	printf("%d\n",lus);
	create_count();
	printf("creation du compte ?");

      }

   else if (strcmp(texte, "3") == 0) {
	printf("compte effacé:\n");
	lus=lireLigne(canal,logi);
	printf("%d\n",lus);
	//effacercompte();

      }

      else {
	if (ecrireLigne(log, texte) == -1) {
	  erreur_IO("ecrireLigne");
	}
	printf("%s: ligne de %d octets ecrite dans le journal.\n", CMD, nblus);
      }
    }
  }

  exit(EXIT_SUCCESS);
}

void create_count(void){									// Fonction de creation de compte
	int mode, i;
	off_t pos;
	ssize_t nblus, nbecrits;
	User usr, temp;
	En_tete en_tete;

	printf("%s,%s\n",logi,motdp);
	printf("%d\n",en_tete.nb_usr);
	/* Ouverture du fichier "comptes.txt" */
	mode = O_RDWR | O_CREAT;								
	comptes = open("comptes.txt", mode, S_IRWXU);			
	if (comptes == -1){
		erreur_IO("open comptes");
	}
	printf("Ouverture de comptes.txt\n");

	/* Lecture de l en-tete */
	pos = lseek(comptes, 0, SEEK_SET);						
	if (pos == -1){
		erreur_IO("lseek");
	}
	en_tete.nb_usr = 0;
	nblus = read(comptes, &en_tete, sizeof(En_tete));	
	if(nblus != sizeof(En_tete)){
		if(nblus == 0){										
			en_tete.nb_usr = 0;	
		}
		else{
			erreur_IO("read");
		}
	}
	printf("%d\n",en_tete.nb_usr);
	/* Verrouillage mutex */
	if(pthread_mutex_lock(&verrou) != 0){
		erreur("mutex_lock");
		return;
	}
	printf("Verrouillage\n");
	
	/* Si le nombre d utilisateurs est nul, on ecrit un premier en-tete */
	if(en_tete.nb_usr == 0){
		now = time(NULL);
		en_tete.date_mod = *localtime(&now);					
		pos = lseek(comptes, 0, SEEK_SET);						
		if (pos == -1){
			erreur_IO("lseek");
		}
		nbecrits = write(comptes, &en_tete, sizeof(En_tete));
		if(nbecrits != sizeof(En_tete)){
			erreur_IO("write");
		}
		printf("En-tete cree\n");	
	}
	
	/* Deverrouillage mutex */
	if(pthread_mutex_unlock(&verrou) != 0){
		erreur("mutex_unlock");
		return;
	}
	printf("Deverrouillage\n");
	
	/* Affectation des variables d utilisateur */
	usr.ID_Ut = en_tete.nb_usr + 1;
	usr.droit = 0;
	printf("Veuillez choisir un login (10 caracteres maximum)\n");
	strcpy(usr.Nom_Ut,logi);
	if(strlen(usr.Nom_Ut) > 10){
		printf("Login trop long\nCompte non cree\n");
		if(close(comptes) == -1){
		erreur_IO("close");
		}
		printf("Fermeture de comptes.txt\n");
		return;
	}
	else{
		for(i = 0 ; i < (en_tete.nb_usr) ; i ++){
			pos = lseek(comptes, sizeof(En_tete) + i * sizeof(User), SEEK_SET);		
			if(pos == -1){
				erreur_IO("lseek");
			}
			if(en_tete.nb_usr != 0){
				nblus = read(comptes, &temp, sizeof(User));							
				if(nblus != sizeof(User)){
					erreur_IO("read");						
				}
				if(strcmp(usr.Nom_Ut,temp.Nom_Ut) == 0){							
					printf("Le login entre est deja utilise\nCompte non cree\n");
					if(close(comptes) == -1){
						erreur_IO("close");
					}
					printf("Fermeture de comptes.txt\n");
					return;
				}
			}
		}
	}
	printf("Veuillez choisir un mot de passe (10 caractères maximum)\n");
	strcpy(usr.PwU,motdp);
	if(strlen(usr.PwU) > 10){
		printf("Mot de passe trop long\nCompte non cree\n");
		if(close(comptes) == -1){
			erreur_IO("close");
		}
		printf("Fermeture de comptes.txt\n");
		return;
	}
	
	/* Verrouillage mutex */
	if(pthread_mutex_lock(&verrou) != 0){
		erreur("mutex_lock");
		return;
	}
	printf("Verrouillage\n");
	
	/* Ecriture d un utilisateur */
	pos = lseek(comptes, 0, SEEK_END);						
	if(pos == -1){
		erreur_IO("lseek");
	}
	nbecrits = write(comptes, &usr, sizeof(User));			
	if(nbecrits != sizeof(User)){
		erreur_IO("write");
	}
	printf("Compte cree avec succes\n");

	/* Affectation des variables et mise a jour de l en-tete */	
	(en_tete.nb_usr) ++;
	now = time(NULL);
	en_tete.date_mod = *localtime(&now);					
	pos = lseek(comptes, 0, SEEK_SET);						
	if (pos == -1){
		erreur_IO("lseek");
	}
	nbecrits = write(comptes, &en_tete, sizeof(En_tete));	
	if(nbecrits != sizeof(En_tete)){
		erreur_IO("write");
	}
	printf("En-tete mis a jour\n");	
	
	/* Deverrouillage mutex */
	if(pthread_mutex_unlock(&verrou) != 0){
		erreur("mutex_unlock");
		return;
	}
	printf("Deverrouillage\n");
	
	/* Fermeture de "comptes.txt" */
	if(close(comptes) == -1){
		erreur_IO("close");
	}
	printf("%d",en_tete.nb_usr);
	printf("Fermeture de comptes.txt\n");


void effacercompte(void){			//n'arrive pas  se lancer je ne sais pas pourquoi...						
	int mode, mode2, temp, i, j;
	char log[10];
	off_t pos;
	ssize_t nblus, nbecrits;
	User usr;
	En_tete en_tete;

	/* Ouverture du fichier "comptes.txt" */
	mode = O_RDWR;											
	comptes = open("comptes.txt", mode, S_IRWXU);			// Le proprietaire a tous les droits, les autres aucuns
	if (comptes == -1){
		erreur_IO("open comptes");
	}
	printf("Ouverture de comptes.txt\n");

	/* Lecture de l en-tete */
	pos = lseek(comptes, 0, SEEK_SET);					
	if (pos == -1){
		erreur_IO("lseek");
	}
	nblus = read(comptes, &en_tete, sizeof(En_tete));	
	if(nblus != sizeof(En_tete)){
			erreur_IO("read");
	}
	
	/* Demande du login du compte a supprimer */
	printf("Veuillez entrer le login du compte a supprimer\n");
	//scanf("%s", log);
	strcpy(log,logi);
	
	/* Verrouillage mutex */
	if(pthread_mutex_lock(&verrou) != 0){
		erreur("mutex_lock");
		return;
	}
	printf("Verrouillage\n");
	
	/* Suppression */
	for(i = 0 ; i < en_tete.nb_usr ; i ++){
		pos = lseek(comptes, sizeof(En_tete) + i * sizeof(User), SEEK_SET);
		if (pos == -1){
			erreur_IO("lseek");
		}
		nblus = read(comptes, &usr, sizeof(User));	
		if(nblus != sizeof(User)){
			erreur_IO("read");
		}
		if(strcmp(log, usr.Nom_Ut) == 0){							
			mode2 = O_RDWR | O_CREAT  ;							
			temp = open("temp.txt", mode2, S_IRWXU);			
			if (temp == -1){
				erreur_IO("open temp");
			}
			
			/* On recopie tous les utilisateurs qui étaient positionnés avant et après celui à supprimer dans temp.txt */
			for(j = 0 ; j < i ; j ++){								
				pos = lseek(comptes, sizeof(En_tete) + j * sizeof(User), SEEK_SET);
				if (pos == -1){
					erreur_IO("lseek");
				}
				nblus = read(comptes, &usr, sizeof(User));	
				if(nblus != sizeof(User)){
					erreur_IO("read");
				}
				nbecrits = write(temp, &usr, sizeof(User));	
				if(nbecrits != sizeof(User)){
					erreur_IO("write");
				}
			}
			for(j = i+1 ; j < en_tete.nb_usr ; j ++){
				pos = lseek(comptes, sizeof(En_tete) + j * sizeof(User), SEEK_SET);
				if (pos == -1){
					erreur_IO("lseek");
				}
				nblus = read(comptes, &usr, sizeof(User));	
				if(nblus != sizeof(User)){
					erreur_IO("read");
				}
				nbecrits = write(temp, &usr, sizeof(User));	
				if(nbecrits != sizeof(User)){
					erreur_IO("write");
				}
			}
			
			/* Ecrasement de comptes.txt et ouverture en mode ecriture seule */
			if(close(comptes) == -1){
				erreur_IO("close");
			}
			mode = O_TRUNC;											
			comptes = open("comptes.txt", mode, S_IRWXU);		
			if (comptes == -1){
				erreur_IO("open comptes");
			}
			if(close(comptes) == -1){
				erreur_IO("close");
			}
			mode = O_WRONLY;										
			comptes = open("comptes.txt", mode, S_IRWXU);		
			if (comptes == -1){
				erreur_IO("open comptes");
			}
			
			/* On ecrit le nouvel en-tete */
			en_tete.nb_usr --;
			now = time(NULL);
			en_tete.date_mod = *localtime(&now);
			nbecrits = write(comptes, &en_tete, sizeof(En_tete));	
			if(nbecrits != sizeof(En_tete)){
				erreur_IO("write");
			}
			
			/* On reecrit les utilisateurs non supprimes dans comptes.txt */
			for(i = 0 ; i < en_tete.nb_usr ; i ++){
				pos = lseek(temp, i * sizeof(User), SEEK_SET);
				if (pos == -1){
					erreur_IO("lseek");
				}
				nblus = read(temp, &usr, sizeof(User));					
				if(nblus != sizeof(User)){
					erreur_IO("read");
				}
				nbecrits = write(comptes, &usr, sizeof(User));
				if(nbecrits != sizeof(User)){
					erreur_IO("write");
				}
			}
			/* Deverrouillage mutex */
			if(pthread_mutex_unlock(&verrou) != 0){
				erreur("mutex_unlock");
				return;
			}
			printf("Deverrouillage\n");
			
			if(close(comptes) == -1){
				erreur_IO("close");
			}
			printf("Fermeture de comptes.txt\n");
			if(close(temp) == -1){
				erreur_IO("close");
			}
			remove("temp.txt");
			return;
		}
	}
	printf("Le compte que vous souhaitez supprimer n existe pas\nCompte non supprime\n");
	
/* Deverrouillage mutex */
if(pthread_mutex_unlock(&verrou) != 0){
	erreur("mutex_unlock");
	return;
}
	printf("Deverrouillage\n");
	
/* Fermeture de "comptes.txt" */
	if(close(comptes) == -1){
		erreur_IO("close");
	}
}
}


void authenticate(void){									// Fonction d authentification
	int mode, i;
	//char logi[10], motdp[10];
	off_t pos;
	ssize_t nblus;
	User usr;
	En_tete en_tete;
	
	printf("%s\n",logi);
	/* Ouverture du fichier "comptes.txt" */
	mode = O_RDONLY;										
	comptes = open("comptes.txt", mode, S_IRWXU);			
	if (comptes == -1){
		erreur_IO("open comptes");
	}
	printf("Ouverture de comptes.txt\n");
	
	/* Lecture de l en-tete */
	pos = lseek(comptes, 0, SEEK_SET);					
	if (pos == -1){
		erreur_IO("lseek");
	}
	nblus = read(comptes, &en_tete, sizeof(En_tete));	
	if(nblus != sizeof(En_tete)){
			erreur_IO("read");
	}
	
	/* Demande d authentification */
	printf("Veuillez entrer votre login\n");
	strcpy(usr.Nom_Ut,logi);
	printf("%s\n",logi);
	printf("Veuillez entrer votre mot de passe\n");
	strcpy(usr.PwU,motdp);
	/* Verification */
	for(i=0 ; i < en_tete.nb_usr ; i ++){
		pos = lseek(comptes, sizeof(En_tete) + i * sizeof(User), SEEK_SET);
		if (pos == -1){
			erreur_IO("lseek");
		}
		nblus = read(comptes, &usr, sizeof(User));	
		if(nblus != sizeof(User)){
			erreur_IO("read");
		}
		if(strcmp(logi, usr.Nom_Ut) == 0 && strcmp(motdp, usr.PwU) == 0){
			printf("Authentification reussie\n");
			if(close(comptes) == -1){
				erreur_IO("close");
			}
			printf("Fermeture de comptes.txt\n");
			return;
		}
	}
	printf("Echec d authentification\n");
	
	/* Fermeture de "comptes.txt" */
	if(close(comptes) == -1){
		erreur_IO("close");
	}
	printf("Fermeture de comptes.txt\n");
}

