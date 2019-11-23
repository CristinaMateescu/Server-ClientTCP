#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;


/*Strctura mesaj */
typedef struct {

	/* subscribe = 0 -- unsubscribe
	   subscribe = 1 -- subscribe */
	int subscribe, SF;
	char topic[50],ID[10];

}msg;

typedef struct {

	char topic[50];
	char tip_date;
	char payload[1500];

}UDPmsg;

typedef struct {
		
	int port;
	char IP[15];
	char topic[50];
	char tip_date;
	char payload[1500];	

} msgInfo;


void usage(char *file)
{
	fprintf(stderr, "Usage: %s <PORT_DORIT>\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{	
	
	/* Evidenta topicurilor la care sunt abonati clientii */
	map<string,vector<string>> topics;

	/* Evidenta ID-urilor clientilor */
	map<int, string> IDs;
		
	/* Stocare mesaje ale clientilor abonati offline */
	map<string, vector<msgInfo>> mesaje;

	/* Stocare id-uri clienti abonati cu SF = 1 */
	map<string,vector<string>> SF;
		


	/* Declaratii structuri */
	int newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;



	/* Multimea de citire folosita in select() */
	fd_set read_fds;

	/* Multime folosita temporar */	
	fd_set tmp_fds;	

	/* Valoare maxima fd din multimea read_fds */	
	int fdmax;		



	/* Verificare numar argumente */
	if (argc < 2) {
		usage(argv[0]);
	}



	/* Se golesc multimile de descriptori: read_fds ( de citire ), tmp_fds ( multimea temporara )*/
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);



	/* SOCKET TCP */

	int sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfdTCP < 0, "Socket error");


	/*Port */
	portno = atoi(argv[1]);
	DIE(portno == 0, "Invalid Port");



	/* Constructie structura server */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfdTCP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "Bind error");


	
	ret = listen(sockfdTCP, MAX_CLIENTS);
	DIE(ret < 0, "listen");


	/* Se adauga noul fd (socketul pe care se asculta conexiuni) in multimea read_fds */
	FD_SET(sockfdTCP, &read_fds);
	fdmax = sockfdTCP;



	/* SOCKET UDP */

	int sockfdUDP = socket ( AF_INET, SOCK_DGRAM, 0 );
	DIE(sockfdUDP < 0, "UDP Socket error");

	/* Legare proprietăți de socketul UDP */	
	ret = bind (sockfdUDP, (struct sockaddr*)(&serv_addr), (socklen_t)sizeof(struct sockaddr_in) );
	DIE(ret < 0, "Bind error");

	/* Se adauga fd-ul socketului UDP in multimea read_fds */
	FD_SET( sockfdUDP, &read_fds);
	if ( sockfdUDP > fdmax )
	fdmax = sockfdUDP;
	


	/* Se adauga stdin in multimea descriptorilor */
	FD_SET(0, &read_fds);

	
	

	while (1) {
			
		/* Salvam multimea descriptorilor, folosim multimea temporara */
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		/* Parcurgem descriptorii */
		for (i = 0; i <= fdmax; i++) {
		  if (FD_ISSET(i, &tmp_fds)) {

			/* A venit o CERERE DE CONEXIUNE */
			if (i == sockfdTCP) {
					

				clilen = sizeof(cli_addr);
				newsockfd = accept(sockfdTCP, (struct sockaddr *) &cli_addr, &clilen);
				DIE(newsockfd < 0, "accept");


				/* Se adauga noul socket intors de accept() la multimea descriptorilor de citire */
				FD_SET(newsockfd, &read_fds);
				if (newsockfd > fdmax) { 
					fdmax = newsockfd;
				}


				/*Primeste ID-ul noului client care doreste sa se conecteze */
				char ID[10];
				n = recv(newsockfd, ID, sizeof(ID), 0);
				DIE( n < 0 , "ID" );
					

				/*Verifica daca ID este valid -- nu exista in map-ul de ID uri */
				int valid = 1;
					
				/* Iteram prin map-ul de ID -uri */
				map<int, string>::iterator it;
				for ( it = IDs.begin(); it!=IDs.end(); ++it )
					if ( strcmp(ID, (it->second).c_str()) == 0 ){
						
						valid = 0;
						break;}			

				
				/* Trimitem feedbackul validarii ID-ului */
				n = send(newsockfd, &valid, sizeof(int),0);
				DIE( n < 0 , "ID" );

				if ( valid ) {
					printf("New client (%s) connected from %s:%d.\n",ID,
						inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port) );


					/* Verificam daca exista mesaje stocate si netrimise */
						
					/*Caut id in map ul mesaje */
					map<string, vector<msgInfo>>::iterator it = mesaje.find(ID);
					
					if ( it != mesaje.end() ){

						/*parcurg vectorul de mesaje*/
						vector<msgInfo>::iterator v_it;
						for ( v_it = (it->second).begin(); v_it != (it->second).end() ; ++v_it ){
							
							
						   /*trimit fiecare mesaj */
						   memcpy ( buffer, &(*v_it) , sizeof(msgInfo)); 
						   n = send (newsockfd, buffer, BUFLEN, 0);}

						
						/* Stergem mesajele trimise*/
						(it->second).clear();
					}

					

					/* Inserare ID in map-ul principal de ID-uri */	
					string strID(ID);
					IDs.insert(make_pair(newsockfd, strID) );

				}

				/*Daca ID-ul nu este valid */
				else {

					/* Inchide conexiunea */
					close(newsockfd);
					
					/* Scoate socketul din multimea de citire */
					FD_CLR(newsockfd, &read_fds);
				}

											
			/*Citire de la tastatura */
			} else if (i==0) {

					
				string line;

				/* Citire linie de la tastatura */
				getline(cin, line);


				/* Pentru comanda exit */
				if (line.compare("exit")==0) {

					/*Inchidere conexiuni */
					int j;
					for (j = 1; j <= fdmax; j++) 
						if ( FD_ISSET(j, &tmp_fds))
							close(j);	
					return 0;
				}

				/*Pentru orice alta comanda */
				else {
					printf("Comanda invalida\n");
				}


				
			/*Socket clienti UDP */
			}else if (i==sockfdUDP) { 
				
					
				/*Recv message */
				clilen = sizeof(cli_addr);
				n = recvfrom( sockfdUDP, buffer, BUFLEN, 0, (struct sockaddr *)&cli_addr, &clilen);
				DIE ( n < 0 , "recvfrom" );


				UDPmsg message;
				memset( &message, 0 , sizeof(message));
				memcpy( &message, buffer, n );


				/*Parsare message*/
					
				msgInfo info;
				memset( &info, 0 , sizeof(msgInfo));
				info.port = ntohs(cli_addr.sin_port);
				strcpy (info.IP ,inet_ntoa(cli_addr.sin_addr) );
				strcpy ( info.topic, message.topic);
				info.tip_date = message.tip_date;
				memcpy ( info.payload, message.payload, 1500);

					
				memcpy ( buffer, &info, sizeof(info));

					
				/* Cautam topicul in map */
				string topic(message.topic);
				auto topic_it = topics.find ( topic );
					
					
				/* Trimitem mesaje daca topicul exista */
				if ( topic_it != topics.end() ) {

											
					/* Pentru fiecare client abonat la topic */
					vector<string>::iterator it;
					for ( it = (topic_it->second).begin(); it!=(topic_it->second).end(); ++it){
							
						string ID = *it;
						int sock;
							
						/* Caut socketul asociat in map */
						map<int, string>::iterator id_it;
						for ( id_it = IDs.begin(); id_it!=IDs.end(); ++id_it )
							if ( ID.compare((id_it->second).c_str()) == 0 ){
								sock = id_it->first;
								break;
							}


						/*Trimit mesajul daca am gasit socketul */
						if ( id_it != IDs.end() ){
							n = send(sock, buffer, BUFLEN,0);
							DIE( n < 0 , "ID" );}

						/* Altfel stochez mesajul in map daca exista optiunea SF =1 */
						else {
								
							/* Caut ID in vectorul de SF din map de topicuri */
							auto sf_it = SF.find ( topic );
							
							if ( sf_it != SF.end() ){
							vector<string>::iterator id_sf;
								
							for ( id_sf = (sf_it->second).begin(); id_sf!=(sf_it->second).end(); ++id_sf )
							if ( ID.compare(*id_sf) == 0 ){
								break;
							}


							/* ID exista in vectorul de sf */
							if ( id_sf != (sf_it->second).end() ){
								
							/* Caut vectorul de mesaje dupa id */
							map<string,vector<msgInfo>>::iterator ms_it = mesaje.find(ID);

							/* Daca exista adaug mesajul in el */
							if ( ms_it != mesaje.end() )
								(ms_it->second).push_back(info);

							/* Altfel creez vectorul si adaug noua pereche*/
							else {

							vector<msgInfo> id_msg;
							id_msg.push_back(info);
							mesaje.insert(make_pair (ID, id_msg ));}
							}

						}
					}	
				}

			}
				
			/* Socketi clienti TCP */
			}else {
					
				memset(buffer, 0, BUFLEN);
				n = recv(i, buffer, sizeof(buffer), 0);
				DIE(n < 0, "recv");


				/* Daca s-a inchis conexiunea */
				if (n == 0) {
						

					/* Caut ID-ul asociat socketului i */
					printf("Client (%s) disconnected.\n",(IDs.find(i)->second).c_str() );
					close(i);
						
					/* Se scoate din multimea de citire socketul inchis */
					FD_CLR(i, &read_fds);

					/* Scoatem clientul din map-ul clientilor activi */
					IDs.erase(i);

				} else {
						
					/* subscribe / unsubscribe - actualizeaza topics */
					msg message;
					memset(&message, 0, sizeof(msg));
					memcpy(&message, buffer, sizeof(msg));
						

					/* Abonare client la topic */
					if (message.subscribe == 1 ){


						/* Caut ID dupa cheia i in map-ul de id uri */
						string ID = IDs.find(i)->second;

						string topic(message.topic);
						auto topic_it = topics.find ( topic );


						/* Verific daca topicul exista */
						if ( topic_it != topics.end() ){

								
							/* Daca ID-ul nu exista deja */
							if ( find((topic_it->second).begin(),
								  (topic_it->second).end(),ID ) == (topic_it->second).end() )
								/*Adaug in vector noul ID */
								topic_it->second.push_back(ID);

						}
						else {
							/*Creez un vector nou cu un element : ID-ul*/
							vector<string> id;
							id.push_back(ID);
							
							/*Inserez in map topicul cu valoarea : vectorul */
							topics.insert(make_pair (topic, id ) );
						}

							

						//Daca SF = 1 adaug si in celalalt map
						if ( message.SF == 1 ){

							auto sf_it = SF.find ( topic );


							/* Verific daca topicul exista */
							if ( sf_it != SF.end() ){

								/* Daca ID-ul nu exista deja */
								if ( find((sf_it->second).begin(),
								  (sf_it->second).end(),ID ) == (sf_it->second).end() )
									/*Adaug in vector noul ID */
									sf_it->second.push_back(ID);

							}
							else {
								/*Creez un vector nou cu un element : ID-ul*/
								vector<string> id;
								id.push_back(ID);
							
							       	/*Inserez in map topicul cu valoarea : vectorul */
								SF.insert(make_pair (topic, id ) );
							}


						}
							
								

					}
					/* Dezabonare client */
					else {

						/* Caut ID dupa cheia i in map-ul de id uri */
						string ID = IDs.find(i)->second;
							
						/* Caut topicul in map */
						string topic(message.topic);
						auto topic_it = topics.find ( topic );

							
						/* Verific daca exista topicul */
						if ( topic_it != topics.end() ){


							/* Elimin din vector ID */
							(topic_it->second).erase (remove ((topic_it->second).begin(), 
											  (topic_it->second).end(),ID ), 
											   (topic_it->second).end() );

							
						}


						/*scot din lista de sf = 1 */

						/* Caut topicul in map */	
						auto sf_it = SF.find ( topic );

							
						/* Verific daca exista topicul */
						if ( sf_it != SF.end() ){


							/* Elimin din vector ID */
							(sf_it->second).erase (remove ((sf_it->second).begin(), 
											 (sf_it->second).end(),ID ), 
											  (sf_it->second).end() );

							
						}
											
					     }
						
					}
				}
			}
		}
	}
	
	/*Inchidere socketi */
	close(sockfdTCP);
	close(sockfdUDP);

	return 0;
}
