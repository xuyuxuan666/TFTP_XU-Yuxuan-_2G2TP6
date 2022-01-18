#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

// ./gettftp addresse_serveur filename
int main(int argc, char *argv[]) {  //Q1 
	argc++;
	char* serveur = argv[1];
	char* filename= argv[2];

	
	char RRQ[256];	
	
	struct addrinfo filtre,* resultat;
	int code_erreur, sock;

	//Q2 Appel à getaddrinfo pour obtenir l’adresse du serveur.
	memset(&filtre, 0,sizeof(filtre));//La fonctionpermet d’initialiser le filtre en mettant toute les valeurs à 0.
	filtre.ai_family = AF_UNSPEC;    /* autorise IPv4 ou IPv6 */
	filtre.ai_socktype = SOCK_DGRAM; 
	filtre.ai_flags = AI_PASSIVE;    
	filtre.ai_protocol = 0;          
	filtre.ai_canonname = NULL;
	filtre.ai_addr = NULL;
	filtre.ai_next = NULL;
	
	//on obtient l'adreese du serveur
	code_erreur=getaddrinfo(serveur,"1069",&filtre,&resultat);
	if (code_erreur != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(code_erreur));
		exit(EXIT_FAILURE);
	}else{
		fprintf(stderr, "getaddrinfo: ok\n");
	}
	
	if(resultat ==NULL){
		fprintf(stderr, "on ne peut pas se connecter\n");
		exit(EXIT_FAILURE);
	}else{
		fprintf(stderr, "connection: ok\n");
	}
	//Q3 *création du socket
	sock= socket(resultat->ai_family, resultat-> ai_socktype, resultat->ai_protocol);
	//erreur de la creation du socket
	if(sock==-1){
		fprintf(stderr, "erreur crétion socket\n");
		exit(EXIT_FAILURE);
	}
	//Q4
	
	// demande lecture
	RRQ[0]=0;RRQ[1]=1; 							//opcode		2 bytes 	1 Read request (RRQ)||2 Write request (WRQ)||3 Data (DATA) ||4 Acknowledgment (ACK)||5 Error (ERROR)
	strcpy(&RRQ[2],filename);					//filename		string
	RRQ[2+strlen(filename)]=0;					//0				1 byte	
	strcpy(&RRQ[3+strlen(filename)],"octet");	//Mode			string
	RRQ[8+strlen(filename)]=0;					//0				1 byte 
	int lenght_RRQ= 9+strlen(filename);
	
	if(!sendto(sock,RRQ,lenght_RRQ,0,resultat->ai_addr, resultat->ai_addrlen)){
		fprintf(stderr, "erreur demande de lecture \n");
	}
	

	char recv[516];


	int lenght_recv=recvfrom(sock, recv, 516, 0,resultat->ai_addr,&(resultat->ai_addrlen));
	//2 bytes     2 bytes      n bytes
    // Opcode |   Block #  |   Data     |

	fprintf(stderr,"Opcode: %d,%d  Block %d,%d\nTaille des donnés: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	
	char ACK[100];
	ACK[0]=0;ACK[1]=4; 				//opcode		2 bytes
	
	int lenght_ACK=4;
	int file =open(filename,O_CREAT|O_WRONLY,0x777);
	write(file,recv+4,lenght_recv-4);
	while(lenght_recv==516){
		ACK[2]=recv[2];ACK[3]=recv[3];		
		
		sendto(sock,ACK,lenght_ACK,0,resultat->ai_addr, resultat->ai_addrlen);
		
		lenght_recv=recvfrom(sock, recv, 516, 0,resultat->ai_addr,&(resultat->ai_addrlen));
		write(file,recv+4,lenght_recv-4);
		fprintf(stderr,"Opcode: %d,%d  Block %d,%d\nTaille des donnés: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
				
	}
	exit(EXIT_SUCCESS); 	
}
