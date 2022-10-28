Tema 2 PCOM - Dogaru Laurentiu 323CC

-Am rezolvat majoritatea punctelor cerute de tema, in afara de: optiunea
store-and-forward (dar abonarile unui client se pastreaza la deconectare si
reconectare cu acelasi ID)
-De asemenea, am incadrat mesajele in structuri de dimensiune fixa

Server:

-Pentru mesajele primite de la clientii UDP, am folosit o structura cu
campurile topic, type, payload, ip_address si port
-Functia build_msg creeaza o astfel de structura, primind ca parametru
buffer-ul in care se afla mesajul de la clientul UDP
-Pentru mesajele primite de la clienti TCP de catre server, am folosit o
structura cu campurile type si payload, numita request
-Am deschis un socket udp si i-am asociat un port si o adresa cu bind()
-Am deschis un socket tcp, i-am asociat un port si o adresa cu bind(), am
golit multimile descriptorilor de citire si cea temporara si am dezactivat
algoritmul lui Nagle
-Am setat socketul sa fie pasiv prin listen(), acel socket ascultant si
intarziind cererile de conectare ce provin de la clienti
-Am adaugat socketii si fluxul de intrare in multimea descriptorilor de citire
-Prin functia select(), serverul poate controla mai multi descriptori in
acelasi timp
-In timpul executiei, m-am folosit de doua map-uri: unul in care cheia este
ID-ul unui client, iar valoarea este un set cu topic-urile la care este abonat,
iar in celalalt cheia este ID-ul clientului, iar valoarea este socket-ul pe
care se comunica cu acesta
-Daca se primesc date de la STDIN, singurul caz valabil e acela cand se
primeste "exit" si serverul se inchide, odata cu toti clientii conectati. Orice
alta comanda este invalida.
-Cand se primesc date pe socket-ul dat ca parametru anterior lui listen,
inseamna ca a venit o cerere de conexiune, pe care serverul o accepta. Functia
accept() intoarce un nou socket, care este adaugat in multimea de descriptori,
iar in cli_addr_tcp se afla informatii despre conexiunea facuta (adresa, port)
-Cand se primesc date pe socket-ul UDP, se pun in buffer, se creeaza structura
pe baza mesajului, si este trimisa tuturor clientilor care sunt abonati la
topic-ul din mesaj
-Cand se primesc date de pe alt socket care se afla in multimea de descriptori
de citire si valoarea intoarsa de recv() este pozitiva, inseamna ca un client
TCP deja conectat a trimis date, mai exact o structura request
-Daca tipul request-ului este 0, inseamna ca a fost trimis fix dupa conectarea
clientului. In payload se va afla ID-ul clientului. Se verifica daca ID-ul face
parte din map-ul cu ID-uri si socketi. Daca face parte, serverul va afisa
mesajul corespunzator si va trimite clientului o strucutra de tip message cu
tipul 4, in urma caruia clientul se va inchide. Daca nu face parte deja, ID-ul
si socket-ul sunt adaugate in map si este afisat mesajul "New client connected"
-Daca tipul request-ului este 1, clientul s-a deconectat de la server. Este
scos din map si socket-ul lui din multimea de descriptori si se afiseaza
mesajul "Client disconnected".
-Daca tipul request-ului este 2, este o cerere de subscribe. Daca este prima
subscriptie, se creeaza o intrare corespunzatoare in map-ul subscription, iar
daca nu, se adauga topic-ul aflat in payload in set-ul din map corespunzator
ID-ului clientului.(Payload-ul contine pe primii 10 octeti ID-ul, iar pe
urmatorii topic-ul)
-In cazul in care tipul request-ului este 3, este o cere de unsubscribe. 
Continutul payload-ului este acelasi ca in cazul anterior. Se scoate din set-ul
corespunzatorul clientului topic-ul respectiv
-In final, se inchid toti socketii

Subscriber:

-Am deschis un socket tcp si am dezactivat algoritmul lui Nagle
-Am construit structura de tip sockaddr "servaddr" cu argumentele primite la
pornire
-Am initiat conectarea la server si stabilirea unuii three way handshake cu
functia connect()
-Imediat dupa conectare, se trimite server-ului o structura request cu tipul 0,
care contine si ID-ul clientului
-Am adaugat socket-ul si fluxul de intrare in multimea descriptorilor de citire
-Daca se primesc date de la STDIN, exitsta 3 cazuri: exit,
subscribe <topic> <sf> si unsubscribe <topic>
-Daca se doreste inchiderea clientului, se trimite server-ului un request cu
tipul 1, care contine si ID-ul clientului
-In cazul subscribe si unsubscribe, se trimite server-ului un request de tip 2,
respectiv 3, care contine ID-ul, topic-ul si flag-ul SF in cazul comenzii
subscribe
-In afara de cele 3 cazuri mentionate, orice alta comanda este invalidata
-Daca se primesc date de la socket, reprezinta o structura de tip message
-Un mesaj cu tipul 4 reprezinta solicitarea de inchidere a clientului. Mesajele
cu tipul 0-3 reprezinta date venite initial de la un client UDP. Clientul
afiseaza corespunzator mesajul primit.