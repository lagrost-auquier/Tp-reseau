/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>


void construire_message(char *message, char motif, int lg) { 
	int i;
	for (i=0;i<lg;i++){
		message[i] = motif;
	}
} 

void afficher_chaine(char *message, int lg) { 
	int i; 
	for (i=0;i<lg;i++){
		printf("%c", message[i]); 
	}
}

int longueur_nombre(int Nombre) {
  int lg=0;
  int leNombre = Nombre; 
  if (leNombre>=0) {  
    while (leNombre >= 1) {
      leNombre = leNombre/10;
      lg++;
    }
  }
  else {
    printf("Le numero du message ne doit pas etre negatif\n");
    exit(1);
  } 
  return lg;
}
void entete_message(int numero_message) {
  if (numero_message < 99999){
    int lg = 5-longueur_nombre(numero_message);
    int i;
    for (i=0;i<lg;i++){
      printf("-");
    }
    printf("%d",numero_message);
  }
  else {
    printf("Nombre de message a envoyer doit etre inferieur a 99999\n");
    exit(1);
  }
}

void afficher_message(char *message, int lg_message, int nb_message){	
	printf("[");
	entete_message(nb_message);
	afficher_chaine(message, lg_message);
	printf("]\n");
}

int ouvrir_socket() {
	int sock;
  	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    		printf("échec lors de la création du socket\n");
    		exit(1);
  	}
	return sock;
}

int ouvrir_socket_tcp() {
	int sock;
  	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    		printf("échec lors de la création du socket TCP\n");
    		exit(1);
  	}
	return sock;
}

void fermer_socket(int sock) {
  	if (close(sock) == -1) {
    		printf("échec lors de la fermeture du socket\n");
    		exit(1);
  	}
}
  
void puits_UDP (int port, int nb_message, int lg_message) {
	int sock;
	sock = ouvrir_socket();
	struct sockaddr_in adr_local;
	memset((char *)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET;
	adr_local.sin_port = htons(port);
	adr_local.sin_addr.s_addr = INADDR_ANY; 
	if(bind(sock, (struct sockaddr *)&adr_local, sizeof(adr_local)) == -1){
		printf("Echec du bind\n");
		exit(1);
	}
	char *message = malloc((sizeof(char))*lg_message);  
	struct sockaddr padr_em;
	unsigned int plg_adr=sizeof(struct sockaddr_in);
	int i;
	while (i<nb_message) {
		i+=1;
		if (recvfrom(sock, message, (sizeof(char))*lg_message, 0, &padr_em, &plg_adr) == -1){
			printf("echec de reception 2\n");
			exit(1);
		}	
		printf("PUIT : Reception n° %d (%d)",i, lg_message);
		afficher_message(message, lg_message,i);
	}
	printf("PUIT : Fin de la reception\n");
	fermer_socket(sock);
}
	
void source_UDP(char *adresse, int port, int nb_message, int lg_message) {
	int sock = ouvrir_socket();
	struct hostent *hp = NULL;
	struct sockaddr_in adr_distant;
	memset((char *)&adr_distant, 0, sizeof(adr_distant));
	adr_distant.sin_family = AF_INET; 
	adr_distant.sin_port = htons(port); 
	if ((hp = gethostbyname(adresse)) == NULL){
		printf("Erreur gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(adr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);
 	char *message = malloc(lg_message*sizeof(char));
	int i;
	for (i=0;i<nb_message;i++) {
	  construire_message(message, 65+(i%26),lg_message);
		if (sendto(sock, message, lg_message, 0, (struct sockaddr *) &adr_distant, sizeof(adr_distant)) == -1){
			printf("echec de l'envoi\n"); 
			exit(1);
		}
		printf("SOURCE : Envoi n° %d (%d)",i+1, lg_message);
		afficher_message(message, lg_message, i+1);
  	}
  	printf("SOURCE : Fin des envois!\n");
	fermer_socket(sock);
}

void source_TCP(char *adresse, int port, int nb_message, int lg_message) {
	int sock = ouvrir_socket();
	struct hostent *hp = NULL;
	struct sockaddr_in adr_distant;
	memset((char *)&adr_distant, 0, sizeof(adr_distant));
	adr_distant.sin_family = AF_INET; 
	adr_distant.sin_port = htons(port); 
	if ((hp = gethostbyname(adresse)) == NULL){
		printf("Erreur gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(adr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);
	if (connect(sock,(struct sockaddr *)&adr_distant,sizeof(adr_distant)) == -1){
		perror("error connect \n");
		exit(1);
	}
	char *message = malloc(lg_message*sizeof(char));
	int i;
	for (i=0;i<nb_message;i++) {
	  	construire_message(message, 65+(i%26),lg_message);
		if (write(sock, message, lg_message) == -1){
			perror("echec de l'envoi\n"); 
			exit(1);
		}
		printf("SOURCE : Envoi n° %d (%d)",i+1, lg_message);
		afficher_message(message, lg_message, i+1);
  	}
  	printf("SOURCE : Fin des envois!\n");
	fermer_socket(sock);
}

void puits_TCP (int port, int nb_message, int lg_message) {
	int sock, sock_bis;
	int nb_max;
	sock = ouvrir_socket_tcp();
	struct sockaddr_in adr_local;
	memset((char *)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET;
	adr_local.sin_port = htons(port);
	adr_local.sin_addr.s_addr = INADDR_ANY; 
	if(bind(sock, (struct sockaddr *)&adr_local, sizeof(adr_local)) == -1){
		printf("Echec du bind\n");
		exit(1);
	}
	char *message = malloc((sizeof(char))*lg_message);  
	struct sockaddr padr_em;
	unsigned int plg_adr=sizeof(struct sockaddr_in);
	nb_max = 10;
	listen(sock,nb_max);
	while (1){
		if ((sock_bis = accept(sock,(struct sockaddr *)&padr_em, &plg_adr)) == -1)
		
		{
			printf("échec du accept \n");
			exit(1);
			}
			switch(fork()){
				case -1 :
					printf("erreur fork \n");
					exit(1);
				case 0 :
					close(sock);
					int i;
					for (i=0; i<nb_message; i++){
						if ((lg_message=read(sock_bis, message, lg_message))<0){
							printf("échec du read \n");
							exit(1);
							}
						printf("PUIT : Reception n° %d (%d)",i+1, lg_message);
						afficher_chaine(message,lg_message);
						printf("\n");
						}
						exit(0);
				default : 
					close(sock_bis);
				}		
			}
	printf("PUIT : Fin de la reception\n");
	fermer_socket(sock);
	}	

	


void main (int argc, char **argv)
{
	int c;
	int n = 0;
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int lg_message = 30; /* Longueur des messages à envoyer ou à recevoir, par défaut : 30 en émission et en réception */
	int source = -1 ; /* 0=puits, 1=source */
	char * adresse;
	int port;
	int protocole=1; /* 1=TCP, 0=UDP */
	char * protoUtilise;
  
  while ((c = getopt(argc, argv, "pun:l:s")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
	printf("usage: cmd [-p|-s][-n ##][-l##]\n");
	exit(1);
      }
      source = 0;
   
      break;

    case 's':
      if (source == 0) {
	printf("usage: cmd [-p|-s][-n ##][-l##]\n");
	exit(1) ;
      }
      source = 1;
      break;
			
    case 'n':
      nb_message = atoi(optarg);
      break;
	  
    case 'l':
      lg_message = atoi(optarg);   
      break;
			
    case 'u':
      protocole=0;
      break;
		    	        

    default:
      printf("usage: cmd [-p|-s][-n ##][-l##]\n");
      break;
    }
  }

  port = atoi(argv[argc-1]);
  if (source == 1) {
		  adresse = argv[argc-2];
	}
	
	
	
	switch (source) {
	case 1:	
		if (protocole==0){
			protoUtilise = "UDP";
			printf("SOURCE : lg_mesg_emis : %d, port : %d, nb_envois : %d, TP : %s, dest : %s \n",lg_message, port, nb_message, protoUtilise, adresse);
			break;
		}	
		
		else{
			protoUtilise = "TCP";	
			printf("SOURCE : lg_mesg_emis : %d, port : %d, nb_envois : %d, TP : %s, dest : %s \n",lg_message, port, nb_message, protoUtilise, adresse);
			break;
		}

	case 0:
		if (protocole==0){
			protoUtilise = "UDP";
			printf("PUIT : lg_mesg_recu : %d, port : %d, nb_reception : %d, TP : %s\n",lg_message, port, nb_message, protoUtilise);
			break;
		}
		else{
			protoUtilise = "TCP";	
			printf("PUIT : lg_mesg_recu : %d, port : %d, nb_reception : %d, TP : %s\n",lg_message, port, nb_message, protoUtilise);
			break;
		}
	}
	if (protocole == 0)
	{
	  {
	    if (source){
	      source_UDP(adresse, port, nb_message, lg_message);
	    }
	    else{
	      puits_UDP(port, nb_message, lg_message);
	    }
	  }
	}
	else 
	{
	    if (source){
	    source_TCP(adresse, port, nb_message, lg_message);
	    }
	    else{
	      puits_TCP(port, nb_message, lg_message);
	    }
	}
}
