#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define GRID_SIZE 10

void recv_warning(int sockfd, struct sockaddr_in *server_addr,socklen_t len)
{
 int message_lenght;
 //la prima recvfrom() dira' quanto lungo e' il messaggio di warning
 recvfrom(sockfd, &message_lenght, sizeof(int), 0, NULL, NULL);
 //sapendo ora quanti dati aspettarsi, se ci si deve aspettare piu' di 0 bytes di dati,
 //la seconda recvfrom() recuperera' il messaggio di warning vero e proprio
 if(message_lenght > 0)
 {
  char warning_message[message_lenght];
  recvfrom(sockfd, warning_message, message_lenght, 0, NULL, NULL);
  warning_message[message_lenght] = '\0';
  printf("<CLIENT>:: %s\n",warning_message);
 }
}

void control_game(int *ship_x, int *ship_y,int *quit_request)
{
 char input[2];
 printf("Inserisci comando (w: sopra, a: sinistra, s: sotto, d: destra, q: esci):\n");
 char command;
 /*
 per formattare bene il comando (che e' inviato tramite stringhe), creo una stringa buffer di input che contiene TUTTO il comando
 (compreso lo '\n'), e subito dopo, leggo solo il PRIMO carattere del buffer, che e' il nostro effettivo comando
 Questo garantisce che UN SOLO COMANDO e' letto e dunque interpretato.
 */
 scanf("%s",input);
 command = input[0];
 if((command == 'w') && (*ship_x > 0))
 {
  (*ship_x)--;
 }
 else if((command == 'a') && (*ship_y > 0))
 {
  (*ship_y)--;
 }
 else if((command == 's') && (*ship_x < GRID_SIZE - 1))
 {
  (*ship_x)++;
 }
 else if((command == 'd') && (*ship_y < GRID_SIZE - 1))
 {
  (*ship_y)++;
 }
 else if(command == 'q')
 {
  *quit_request = 1;
 }
}

int main(int argc,char **argv)
{
 int sockfd;
 struct sockaddr_in server_addr;
 socklen_t len = sizeof(server_addr);
 char buffer[1024];
 /*Creazione del socket*/
 if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 {
  perror("Errore creazione socket");
  exit(EXIT_FAILURE);
 }
 memset(&server_addr, 0, sizeof(server_addr));
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(PORT);
 server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 int ship_x = GRID_SIZE - 1, ship_y = GRID_SIZE / 2; 							// Posizione iniziale della navicella
 int quitting = 0;										 // opzione per uscire dal gioco
 int collision = 0;
 /*Invio la posizione INIZIALE della navicella...*/
 printf("<CLIENT NAVICELLA>:: posizione (ship_x, ship_y) iniziale: %d, %d\n", ship_x, ship_y);
 snprintf(buffer, sizeof(buffer), "Posizione: %d, %d", ship_x, ship_y);
 sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&server_addr, len);
 /*...e subito dopo, inizia il loop del gioco*/
 printf("<CLIENT NAVICELLA>:: Entro nel loop di gioco...\n");
 while(1)
 {
  /*Ricezione dei meteoriti*/
  while(1)
  {
   int meteorite_data[2];
   recvfrom(sockfd, meteorite_data, sizeof(meteorite_data), 0, NULL, NULL);
   if(meteorite_data[0] == -1)
   {
    break; 		// Segnale di fine dati
   }
  }

  /*Vedi se arriva un messaggio di warning per meteoriti troppo vicini*/
  recv_warning(sockfd, &server_addr, len);

  /*Controllo comandi*/
  control_game(&ship_x,&ship_y,&quitting);
  
  /*Dire al server se ci si vuole scollegare o no*/
  sendto(sockfd, &quitting, sizeof(quitting), 0, (const struct sockaddr *)&server_addr, len);
  if(quitting == 1)
  {
   printf("<CLIENT NAVICELLA>:: Me ne esco!\n");
   break;
  }
  
  /*Invio della posizione della navicella*/
  printf("<CLIENT NAVICELLA>:: posizione (ship_x, ship_y): %d, %d\n", ship_x, ship_y);
  snprintf(buffer, sizeof(buffer), "Posizione: %d, %d", ship_x, ship_y);
  sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&server_addr, len);
  
  /*Vedi se il server ti ha comunicato di esserti schiantato*/
  recvfrom(sockfd, &collision, sizeof(int), 0, NULL, NULL);
  if(collision)
  {
   printf("<CLIENT NAVICELLA>:: Mi sono schiantato! Ho perso!\n");
   break;
  }
  
  sleep(1);
 }
 close(sockfd);
 return 0;
}
