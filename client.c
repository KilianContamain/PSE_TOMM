#include "pse.h"

#define CMD   "client"

int main(int argc, char *argv[]) {
  int sock, arret = FAUX, ret, nbecr, envoie;
  struct sockaddr_in *sa;
  char texte[LIGNE_MAX];
  char logi[10], motdp[10],msg[30];
  
  if (argc != 3) {
    erreur("usage: %s machine port\n", argv[0]);
  }

  printf("%s: creating a socket\n", CMD);
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    erreur_IO("socket");
  }

  printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
  sa = resolv(argv[1], argv[2]);
  if (sa == NULL) {
    erreur("adresses %s et port %s inconnus\n", argv[0], argv[1]);
  }
  printf("%s: adr %s, port %hu\n", CMD,
	 stringIP(ntohl(sa->sin_addr.s_addr)),
	 ntohs(sa->sin_port));

  /* connexion sur site distant */
  printf("%s: connecting the socket\n", CMD);
  ret = connect(sock, (struct sockaddr *) sa, sizeof(struct sockaddr_in));
  if (ret < 0) {
    erreur_IO("Connect");
  }
	printf("1--Authentification:\n2--Création d'un compte:\n3--Suppression d'un compte:\n");
  freeResolv();

  while (arret == FAUX) {
    printf("ligne> ");
    if (fgets(texte, LIGNE_MAX, stdin) == NULL) {
      printf("Fin de fichier (ou erreur) : arret.\n");
      arret = VRAI;
      continue;
    }
    else {
      nbecr = ecrireLigne(sock, texte);
      if (nbecr == -1) {
	erreur_IO("ecrireLigne");
      }
      if (strcmp(texte, "fin\n") == 0) {
	printf("Client. arret demande.\n");
	arret = VRAI;
      }
	if(strcmp(texte,"1\n") == 0){ // on peut créer un menu avec directement des commandes cela sera surement plus rapide...
	printf("taper votre nom:\n");
	scanf("%s", logi);
	envoie=ecrireLigne(sock,logi);
		if(envoie ==-1){
	printf("Problème dans l'envoie du message");}
	printf("Taper votre mot de passe:\n");
	scanf("%s",motdp);
	envoie=ecrireLigne(sock,motdp);
}

	if(strcmp(texte,"2\n") == 0){
	printf("taper votre nom:\n");
	scanf("%s", logi);
	envoie=ecrireLigne(sock,logi);
		if(envoie ==-1){
	printf("Problème dans l'envoie du message");}
	printf("Taper votre mot de passe:\n");
	scanf("%s",motdp);
	envoie=ecrireLigne(sock,motdp);
}
if(strcmp(texte,"3\n") == 0){
	printf("taper votre nom:\n");
	scanf("%s", logi);
	envoie=ecrireLigne(sock,logi);
		if(envoie ==-1){
	printf("Problème dans l'envoie du message");}
}
      else {
	printf("%s: ligne de %d octets envoyee au serveur.\n", CMD, nbecr);
      }
    }
  }

  exit(EXIT_SUCCESS);
}
