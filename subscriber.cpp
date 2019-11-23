#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include <string>
#include <sstream>
#include <vector>
#include <netinet/tcp.h>
#include <math.h>

using namespace std;

/*Strctura mesaj */
typedef struct {

	/* subscribe = 0 -- unsubscribe
	   subscribe = 1 -- subscribe */
	int subscribe, SF;
	char topic[50],ID[10];

}msg;

typedef struct {
		
	int port;
	char IP[15];
	char topic[50];
	char tip_date;
	char payload[1500];	

} msgInfo;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s <ID_Client> <IP_Server> <Port_Server>\n", file);
	exit(0);
}


int main(int argc, char *argv[])
{
	char buffer[BUFLEN];	

	/*ID = sir de maximum 10 caractere */
	char ID[10];


	/* Verificare argumente linie de comanda */
	if (argc < 4) {
		usage(argv[0]);
	}


	/* argv[1] - ID
	   argv[2] - IP_Server
	   argv[3] - PORT_Server */

	/* ID - sir de maximum 10 caractere */
	DIE (strlen(argv[1]) > 10 ,"ID invalid : Depasirea numarului de caractere");


	/* Socket TCP */	
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(socket_fd < 0, "Socket error");
	

	/* Dezactivare algoritm Neagle */
	int yes = 1;
	int result = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY,  (char*)(&yes), sizeof(int));

	DIE ( result <0 , "TCP_NODELAY error");

	/* Formatare IP, constructie sutructura date server */
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	int ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "Invalid IP");
	
	
	/* Deschidere conexiune */
	ret = connect(socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "Connect Error");

	
	
	/* Validare ID_Client */
	strcpy(ID,argv[1]);
	int n = send(socket_fd, ID, sizeof(ID), 0);
	DIE (n < 0, "Validare ID nereusita"); 
	
	int ValidID;
	n = recv ( socket_fd, &ValidID, sizeof(ValidID), 0);
	DIE (n < 0, "Validare ID nereusita"); 
	DIE (ValidID == 0, "ID existent" );

	

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime temporara

	/* Se golesc multimile de descriptori : read_fds (de citire) ,tmp_fds (temporara) */
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	/* Socket Server */
	FD_SET(socket_fd, &read_fds);

	/* Descriptorul pentru citirea de la tastatura : 0 */
	FD_SET(0, &read_fds);


	while (1) {
	
		tmp_fds = read_fds; 
		ret = select(socket_fd + 1, &tmp_fds, NULL, NULL, NULL);
		
		/* Citire de la tastatura */
		if (FD_ISSET(0, &tmp_fds)){

		
			/* Cleanup Structuri */
			memset(buffer, 0, BUFLEN);

			
			msg message;
			memset(&message, 0, sizeof(message));

			string line;
		

			/* Citire linie de la tastatura */
			getline(cin, line);

			if ( line.empty() )
				continue;
			
			
			/* Pentru exit - inchiderea clientului */
			if (line.compare("exit")==0) {
				break;
			}


			/* Split linie dupa spatii */
			string token;
			vector<string> words;
			stringstream ss(line);

			while ( getline(ss, token, ' ')){
	
				words.push_back(token);
			}

			
			if ( words.size() > 3 ){
				printf("Comanda invalida\n");
				continue;
			}


			/* Daca primul cuvant e subscribe => verificare restul + compunere mesaj + send */
			if ( !(words.front()).compare("subscribe") ){
				
				if ( words.size() != 3 ){
					printf("Comanda invalida\n");
					continue;
				}
				
				
				
				/* Validare topic */
				if ( (words.at(1)).size() > 50 ){
					printf("Topic invalid\n");
					continue;
				}

				
				/* Validare SF */
				
				if ( words.at(2).compare("0") && words.at(2).compare("1")  ) {
					printf("Comanda invalida\n");
					continue;
				}
				

				/* Compunere mesaj */
				stringstream sf(words.at(2));
				int SF = 0;
				sf >> SF;
				msg message;
				memset ( &message, 0, sizeof(message) );
				message.subscribe = 1;
				message.SF = SF;
				strcpy ( message.topic , words.at(1).c_str() );
				strcpy ( message.ID, ID );

				/* Copiere in buffer */
				memset(buffer, 0, BUFLEN);
				memcpy(buffer, &message, sizeof(message) );
				
				
				/* Trimitere la server */
				int n = send(socket_fd, buffer, BUFLEN, 0);
				DIE(n < 0, "send");

				
				/* Feedback pe ecran */
				printf("subscribed %s\n", message.topic);

			}
			/* Daca primul cuvant e unsubscribe => verificare restul + compunere mesaj + send */
			else if ( !(words.front()).compare("unsubscribe") ){

				if ( words.size() != 2 ){
					printf("Comanda invalida\n");
					continue;
				}


				/* Validare topic */
				if ( (words.at(1)).size() > 50 ){
					printf("Topic invalid\n");
					continue;
				}
				
				
				/* Compunere mesaj */
				msg message;
				memset ( &message, 0, sizeof(message) );
				message.subscribe = 0;
				strcpy ( message.topic , words.at(1).c_str() );
				strcpy ( message.ID, ID );

				/* Copiere in buffer */
				memset(buffer, 0, BUFLEN);
				memcpy(buffer, &message, sizeof(message) );
				
				
				/* Trimitere la server */
				int n = send(socket_fd, buffer, BUFLEN, 0);
				DIE(n < 0, "send");


				/* Feedback pe ecran */					
				printf("unsubscribed %s\n", message.topic);
			

			}
			/* Daca nu e niciunul : input invalid */
			else {
			

				printf("Comanda invalida\n");
				continue;
				
			}

		
		}

		if (FD_ISSET(socket_fd, &tmp_fds)){


			/* Primeste mesaj */
			memset(buffer, 0 , BUFLEN);
			n = recv ( socket_fd, buffer, BUFLEN, 0);
			DIE(n == 0, "Server disconnected" );

			
				
			/* Parseaza */
			msgInfo message;
			memset( &message, 0, sizeof(msgInfo));
			memcpy ( &message, buffer, BUFLEN );
			
			/* Afiseaza */
			printf("%s:%d - %s - ",message.IP, message.port, message.topic);

			/*Afisare tip_date */

			/* INT */
			if ( message.tip_date == 0 ){
				printf("INT - ");

				char sign;
				memcpy(&sign, message.payload, 1);

				uint32_t integer;
				memcpy ( &integer, message.payload+1, sizeof(uint32_t));
				
				if ( sign == 1 )
					printf("-");
				printf("%d\n", ntohl(integer));
				
				}

			/* SHORT_REAL */
			else if ( message.tip_date == 1){
				printf("SHORT_REAL - ");

				uint16_t short_real;
				memcpy ( &short_real, message.payload, sizeof(uint16_t));
				
				printf("%.2f\n", ntohs(short_real)/100.0 );
				
				}

			/* FLOAT */
			else if ( message.tip_date == 2){
				printf("FLOAT - ");

				char sign;
				memcpy(&sign, message.payload, 1);

				uint32_t module;
				memcpy ( &module, message.payload+1, sizeof(uint32_t));

				uint32_t mod = ntohl(module);

				uint16_t expo;
				memcpy (&expo, message.payload+1+sizeof(uint32_t), sizeof(uint16_t));

				if ( sign == 1 )
					printf("-");

				double rez = (mod*1.0)/pow(10,expo);
				printf("%g\n",rez);

				
				}

			/* STRING */
			else if ( message.tip_date == 3){
				printf("STRING - %.1500s\n",message.payload);
				}

			

		}
	}

	close(socket_fd);

	return 0;	
}
