README -- tema2_PC -- MATEESCU CRISTINA-RAMONA -- 321CB

	Mentiune : README-ul este scris urmarind codul - comentariile in README corespund cu cele din cod; Pentru
		   intelegerea mai usoara : cele doua se pot urmari in paralel

	1. Serverul 

		- compilare server : make 
		- rulare server : ./server <PORT>
		- Contine trei structuri de date, o functie de "Usage" si main-ul

		STRUCTURI : 
			
			- prima structura : msg ; Este folosita pentru a transmite comenzile provenite
						  din clientii de TCP 

						  Contine doua campuri de tip int : subscribe - folosit
						  cu conventia de 1 -- comanda subscribe, 0 -- comanda 
						  unsubscribe; SF -- setat conform semnificatiei din enunt

						  Contine si doua campuri de tip char* ce retin ID-ul clientului
						  si topicul la care se face subscribe/unsubscribe
			
			- cea de-a doua structura : UDPmsg ; 
						  Este folosita pentru mesajele provenite de la clientii UDP
						  
						  Contine 3 campuri ce respecta formatul mesajelor, asa cum 
						  este precizat si in enunt : topic, tip_date, continut;

			- cea de-a treia structura : msgInfo;
						  Este folosita pentru a transmite mesajele catre clientii TCP 
						  abonati la topicurile respective; 
						
						  Fata de structura anterioara, ea mai contine si doua campuri : 
						  IP-ul si Port-ul clientului de UDP de unde provine mesajul;

		
		FUNCTIA de Usage : 
			
			- Este folosita pentru a afisa pe ecran un mesaj cu argumentele cu care trebuie rulat serverul

		
		main() : 

			- Am definit 4 structuri de tip map folosind biblioteca STL din c++
			- Ele sunt folosite pentru a stoca diverse date ce asigura functionalitatile mentionate in 
			enunt

				* topics : reprezinta un map ce are cheia : string ( topic ) si valoarea : vector<string> 
				(vector de ID-uri ) si stocheaza pentru fiecare topic existent - ID-urile clientilor
				abonati la acel topic

				* IDs : reprezinta un map ce are cheia : int ( descriptor de socket ) si valoarea : 
				string ( ID ) si asociaza socketului unui client de TCP cu ID-ul cu care a fost deschis
				! retine doar clientii activi (online) la un moment dat

				* mesaje : reprezinta un map ce are cheia : string ( ID ) si valoarea : vector<msgInfo> 
				( vector de mesaje ) si stocheaza mesajele adresate clientilor abonati la anumite topicuri
				avand optiunea SF = 1, trimite cat timp acestia au fost deconectati

				* SF : reprezinta un map ce are cheia : string ( topic ) si valoarea : vector<string>
				( vector de ID-uri ) si stocheaza topicurile la care clientii au fost abonati cu optiunea
				SF = 1 ;

			
			- se declara structuri si variabile - semnificatiile lor vor fi explicate mai tarziu;

			- se declara multimile de descriptori 
			
			- Se verifica daca numarul de argumente cu care a fost rulat serverul este corect ; 
			  In cazul numarului gresit de argumente : se apeleaza functia Usage care va afisa 
			  la ecran formatul de rulare; apoi se iese din program

			- Se initializeaza multimile de descriptori

			- Se vor deschide cei doi socketi : unul de TCP si unul de UDP 

			TCP (pasi) :
				
				* Se deschide socketul ( Se verifica daca s-a reusit )

				* Se pastreaza portul primit ca parametru in variabila portno (int)
				  Se verifica daca s-a reusit parsarea din string in int

				* Se completeaza structura serv_addr cu datele serverului : familie, port;

				* Se realizeaza apelul bind() cu argumentele : socketul de TCP deschis, structura
				  de date;
				  Se verifica daca s-a reusit apelul

				* Se realizeaza apelul listen() ; Se verifica;

				* Se adauga in multimea descriptorilor noul descriptor al socketului de TCP;

			! Pentru verificarea erorilor se foloseste Macro-ul DIE definit in laborator, care 
			realizeaza si afisarea unui mesaj de eroare

			UDP (pasi) :

				* Se deschide socketul ( Se verifica daca s-a reusit )

				* Se realizeaza apelul bind() ( cu verificare )

				* Se adauga in multimea descriptorilor noul descriptor al socketului de UDP;

			
			- Se mai adauga in multimea descriptorilor si STDIN ( 0 ) 

			- Intr-o bucla de while - ( din care se iese pentru erori sau comanda "exit" ) se parcurge
			  multimea descriptorilor setati ( apelul select() ) printr-o constructie de tip "for"

			- In functie de descriptor se deosebesc mai multe cazuri : 


			Descriptorul socketului de TCP - pe care se asculta conexiuni : 
			
				* Daca acesta este setat - A venit o noua cerere de conexiune de la un client TCP
				* variabila newsockfd - socketul noului client conectat
				* structura cli_addr  - datele clientului : IP, Port
				* Descriptorul socketului va fi intors de apelul accept() - care este verificat
				* Acest descriptor este adaugat in multimea de citire

				* Se asteapta un mesaj ce contine ID-ul de conectare al noului client - acestui ID 
				i se verifica validitatea : sa nu existe deja un client conectat cu acest ID ( intrucat
				nu trebuie sa existe 2 clienti cu acelasi ID ) ; Pentru aceasta verificare se itereaza
				prin mapul IDs de ID-uri ale clientilor conectati si daca nu este gasit niciun ID identic
				atunci el este considerat valid
				
				* Se trimite feedbackul validarii - o variabila int : 0 - invalid, 1 - valid

				* Daca ID ul a fost valid - se poate pastra conexiunea si in acest caz - se afiseaza
				pe ecran mesajul corespunzator si se verifica daca exista mesaje stocate pe care clientul 
				trebuia sa le primeasca cat a fost deconectat;
				In map-ul de mesaje se cauta ID-ul si daca acesta exista se vor trimite toate mesajele 	
				din vectorul de mesaje;
				
				* Daca ID ul nu a fost valid - se inchide conexiunea si se scoate socketul din multimea
				descriptorilor;


			Descriptorul corespunzator STDIN ( citirea de la tastatura ) :

				* Serverul poate primi de la tastatura comanda "exit" - pentru care se vor inchide toate
				conexiunile si apoi se va iesi din program;

				* Orice alta comanda este considerata invalida

			
			Descriptorul corespunzator clientilor UDP :
			
				* Se primeste un mesaj 
				* Mesajul este parsat : spart in componente, folosind structura de mesaj
				* Se copiaza datele in noua structura, adaugand portul clientului si IP ul in campurile
				corespunzatoare
				* Se copiaza octetii in buffer
				* Pentru a trimite tuturor clientilor abonati la topicul corespunzator : 
					
					- Se cauta ID urile clientilor abonati la topic in map-ul de topicuri (topics)
					daca topicul exista
					- Pentru fiecare client - se cauta socketul asociat in map-ul de ID-uri (IDs )
					- Daca scoketul a fost gasit - se trimite mesajul 
					- Daca socketul nu a fost gasit - clientul este deconectat - verific daca 
					este activa optiunea SF ( 1 ) - adica il caut in map-ul de abonari la topicuri ce
					au aceasta optiune ( SF ) ; Daca l-am gasit - Doresc sa stochez mesajul in
					map-ul de mesaje ( concatenez la vectorul existent sau creez unul nou daca acesta
					nu exista)



			Descriptorii corespunzatori clientilor TCP :

				* se asteapta mesaje de subscribe/unsubscribe
				* folosim una din strcturi pentru a accesa componentele
				
				* Deconectare client : 
						
					- Se afiseaza mesajul corespunzator
					- Se scoate scoketul din multimea de descriptori
					- Se actualizeaza map-ul de ID-uri (Clienti activi)

				* Subscribe : (campul de subscribe setat pe 1 )
					
					- Cautam ID asociat socketului pentru a-l adauga in mapul de topicuri
					- Verific daca topicul exista pentru a adauga ID-ul in vectorul de ID-uri; 
					Sau creez unul nou in care adaug ID ul, apoi adaug perechea (topic, vector) in
					map;

					- Daca optiunea SF este activa : se realizeaza acelasi procedeu pentru a adauga
					si in celalalt map (SF) al abonatilor la topicuri cu SF = 1

					! in cazul in care un client face subscribe de 2 ori la acelasi topic, comanda
					va fi ignorata
					! in cazul in care prima data era abonat la un topic cu SF = 0 si se aboneaza
					din nou la acelasi topic cu SF = 1 el se va actualiza
					! in cazul in care prima data era abonat la un topic cu SF = 1 si se aboneaza
					din nou la acelasi topic cu SF = 0 , comanda va fi ignorata si optiunea va 
					ramane setata pe 1

				* Unsubscribe : (campul subscribe = 0 )
						
					- Se cauta ID-ul ascoiat socketului ;
					- map-ul de topicuri de cauta ID-ul si se elimina din vector
					- daca topicul nu exista : comanda este ignorata
					
					- Verific daca ID ul exista si in map-ul SF - daca da, atunci il elimin

			
	La final : se inchid socketii.


	2. Clientul TCP

		- Contine cele doua structuri din server 
		- Functia de Usage 
		
		- Se verifica daca numarul de argumente e corect ( folosim Usage )
		- Se valideaza ID-ul : sa nu aiba mai mult de 10 caractere
		- Se deschide un socket (Verificare )
		- Se seteaza optiunea TCP_NODELAY 
		- Se construieste structura cu datele serverului
		- Prin apelul connect - se deschide o conexiune (Verificare)
		- Se trimite ID-ul la server pentru a verifica daca exista deja - validare
		- Se initializeaza multimile de descriptori
		- Se adauga descriptorul pentru STDIN (0)
		
		- Multimea descriptorilor in cazul clientului contine doar doi descriptori : 

		Descriptorul pentru citirea de la tastatura : 
			
			* Se citeste o linie 
			* Pentru comanda exit : se iese din program
			* Se parseaza linia dupa spatii : daca numarul de campuri este mai mare decat 3
			atunci comanda este invalida
			
			* Daca primul camp este "subscribe" 
				
				- Se valideaza linia 
				- Se verifica lungimea topicului
				- Se verifica optiunea (SF)
				- Se compune mesajul folosind structura de date declarata atat in server cat si in client
				- Se trimite mesajul la server
				- Apoi se afiseaza feedbackul corespunzator pe ecran

			* Daca primul camp este "unsubscribe"

				- Se valideaza linia 
				- Se verifica lungimea topicului
				- Se compune mesajul folosind structura de date declarata atat in server cat si in client
				- Se trimite mesajul la server
				- Apoi se afiseaza feedbackul corespunzator pe ecran

			* Daca primul camp este orice altceva : comanda este invalida


		Descriptorul pentru date de la server :

			* Se asteapta un mesaj
			* Se verifica deconectarea serverului -> se inchide si clientul
			* Mesajul este parsat folosind cea de-a doua structura de date
			* Se extrag campurile si sunt afisate conform formatului specificat in enunt
			* In functie de tipul de date - se interpreteaza continutul si se afiseaza parsat

			! pentru SHORT_REAL se tine cont de afisarea cu 2 zecimale
			! pentru STRING se tine cont de afisarea a maxim 1500 caractere 
		
					
					
					
				 
				
				
			
				

			
 
			
