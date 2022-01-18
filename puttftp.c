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
int main(int argc, char *argv[]) {
	argc++;
	char* serveur = argv[1];
	char* filename= argv[2];
	
	int size_file=0;
	
	char WRQ[256];
		
	struct addrinfo filtre,* resultat;
	int code_erreur, sock;
	
	memset(&filtre, 0,sizeof(filtre));
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
	//*création du socket
	sock= socket(resultat->ai_family, resultat-> ai_socktype, resultat->ai_protocol);
	//erreur de la creation du socket
	if(sock==-1){
		fprintf(stderr, "erreur crétion socket\n");
		exit(EXIT_FAILURE);
	}
	
	
	//Q5 *Demande d'écriture
	WRQ[0]=0;WRQ[1]=2; 							//opcode		2 bytes 	1 Read request (RRQ)||2 Write request (WRQ)||3 Data (DATA) ||4 Acknowledgment (ACK)||5 Error (ERROR)
	strcpy(&WRQ[2],filename);					//filename		string
	WRQ[2+strlen(filename)]='\0';				//0				1 byte	
	strcpy(&WRQ[3+strlen(filename)],"octet");	//Mode			string
	WRQ[8+strlen(filename)]='\0';				//0				1 byte 
	int lenght_WRQ= 9+strlen(filename);
	
	if(!sendto(sock,WRQ,lenght_WRQ,0,resultat->ai_addr, resultat->ai_addrlen)){
		fprintf(stderr, "erreur demande de lecture \n");
	}
		
	char recv[516];
	int lenght_recv=recvfrom(sock, recv, 15, 0,resultat->ai_addr,&(resultat->ai_addrlen));
	fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nTaille des donnés: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	size_file=size_file+lenght_recv-4;
		
	int file = open(filename,O_RDWR,0x777);
	char DATA[516];
	memset( DATA, '\0', sizeof(DATA) );	 
	DATA[0]=0;DATA[1]=3; 				//opcode		2 bytes
	int block =1;
	DATA[2]=0,DATA[3]=block;	
	int nb_read =read(file,&DATA[4],512);				
	fprintf(stderr,"nbread: %d\n",nb_read);
	sendto(sock,DATA,nb_read+4,0,resultat->ai_addr, resultat->ai_addrlen);	
	recvfrom(sock, recv, 15, 0,resultat->ai_addr,&(resultat->ai_addrlen));
	fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nTaille des donnés: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);
	
	while(nb_read==512){
		block++;
		DATA[2]=0,DATA[3]=block;
		nb_read =read(file,&DATA[4],512);
		fprintf(stderr,"nbread: %d\n",nb_read);				
		sendto(sock,DATA,nb_read+4,0,resultat->ai_addr, resultat->ai_addrlen);
		recvfrom(sock, recv, 15, 0,resultat->ai_addr,&(resultat->ai_addrlen));
		fprintf(stderr,"Opcode: %d,%d  Block %d,%d \nTaille des donnés: %d\n",recv[0],recv[1],recv[2],recv[3],lenght_recv-4);				
		
	}
	/*
   FILE* fp = fopen(filename, "r");

   //fseek(fp, 0L, SEEK_END);
   //int res = ftell(fp);
   //fprintf(stderr," %d",res);
      
   	char DATA[516];
	memset( DATA, '\0', sizeof(DATA) );
	 
	DATA[0]=0;DATA[1]=3; 				//opcode		2 bytes
	DATA[2]=0,DATA[3]=1;
	int nb_read =fread(&DATA[4], sizeof(char), 512, fp);
	fprintf(stderr," %d",nb_read);

	sendto(sock,DATA,nb_read+4,0,resultat->ai_addr, resultat->ai_addrlen);
	recvfrom(sock, recv, 15, 0,resultat->ai_addr,&(resultat->ai_addrlen));
      
   	while(nb_read!=0){
		DATA[2]=recv[2],DATA[3]=recv[3];
		fseek(fp, nb_read, SEEK_SET);
		nb_read =fread(&DATA[4], sizeof(char), 512, fp);			
		sendto(sock,DATA,nb_read+4,0,resultat->ai_addr, resultat->ai_addrlen);
	}
	*/
}	
	
	
