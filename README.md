WARNING: {IL LINGUAGGIO UTILIZZATO PER QUESTO PROGETTO E' IL LINGUAGGIO C}

L’applicazione è stata codificata seguendo il paradigma client-server, con i due
che adoperano il protocollo UDP per comunicare tra di loro.
Mediante l’utilizzo delle funzioni sendto() e recvfrom(), il client ed il server,
comunicano tra loro scambiandosi pacchetti UDP ogni 2 secondi. Ricordiamo
che queste funzioni sono l’equivalente delle funzioni read() e write().
In particolare, il server genera i meteoriti, ovvero i pacchetti UDP, e li invia al
client che dovrà spostarsi all’interno della griglia di gioco per evitare eventuali
collisioni.
La griglia viene visualizzata lato server, lato client possono essere immessi i comandi di gioco, di cui:
WASD per il movimento e Q per uscire dalla partita..
