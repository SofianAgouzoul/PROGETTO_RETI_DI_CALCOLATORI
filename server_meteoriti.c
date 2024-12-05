#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define GRID_SIZE 10
#define MAX_METEORITES 10

int meteorites[MAX_METEORITES][2];			// Posizioni [x, y] dei meteoriti
int meteorite_count = 0;

// Inizializza il socket del server
int initialize_server(struct sockaddr_in *server_addr)
{
 int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
 if(sockfd < 0)
 {
  perror("Errore creazione socket");
  exit(EXIT_FAILURE);
 }
 memset(server_addr, 0, sizeof(*server_addr));
 server_addr->sin_family = AF_INET;
 server_addr->sin_port = htons(PORT);
 server_addr->sin_addr.s_addr = INADDR_ANY;
 if(bind(sockfd, (const struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
 {
  perror("Errore binding socket");
  exit(EXIT_FAILURE);
 }
 return sockfd;
}

// Aggiorna i meteoriti e verifica collisioni e impatti imminenti
void update_meteorites()
{
 for(int i = 0; i < meteorite_count; i++)
 {
  meteorites[i][0]++;					// Sposta i meteoriti verso il basso
  // Rimuovi meteoriti fuori dalla griglia
  if(meteorites[i][0] >= GRID_SIZE)
  {
   meteorites[i][0] = meteorites[meteorite_count - 1][0];
   meteorites[i][1] = meteorites[meteorite_count - 1][1];
   meteorite_count--;
   i--;
  }
 }
}

//Verifica se si sono verificate collisioni
void check_collision(int ship_x, int ship_y, int *collision_met_x, int *collision_met_y, int *collision)
{
 for(int i=0;i<meteorite_count;i++)
 {
  if((meteorites[i][0] == ship_x) && (meteorites[i][1] == ship_y))
  {
   *collision_met_x = meteorites[i][0];
   *collision_met_y = meteorites[i][1];
   *collision = 1;
   break;
  }
 }
}

//Verifica le collisioni imminenti (cioe' se il meteorite e' a una o due celle sopra un meteorite)
void check_warning_imminent_collision(int ship_x, int ship_y, int *imminent_met_x, int *imminent_met_y, int *imminent_collision)
{
 for(int i=0;i<meteorite_count;i++)
 {
  if(((meteorites[i][0] == ship_x - 1) || (meteorites[i][0] == ship_x - 2)) && (meteorites[i][1] == ship_y))
  {
   *imminent_met_x = meteorites[i][0];
   *imminent_met_y = meteorites[i][1];
   *imminent_collision = 1;
   break;
  }
 }
}

// Genera un nuovo meteorite
void generate_new_meteorite(int ship_x, int ship_y)
{
 if(meteorite_count < MAX_METEORITES)
 {
  meteorites[meteorite_count][0] = 0;
  //meteorites[meteorite_count][1] = rand() % GRID_SIZE;
  meteorites[meteorite_count][1] = ship_y;			//generiamo il meteorite nella stessa colonna dove si trova la navicella
  meteorite_count++;
 }
}

// Invia i meteoriti al client
void send_meteorites(int sockfd, struct sockaddr_in *client_addr, socklen_t len)
{
 for(int i = 0; i < meteorite_count; i++)
 {
  int meteorite_data[2] = {meteorites[i][0], meteorites[i][1]};
  printf("<SERVER>:: Invio meteorite: (%d, %d)\n", meteorite_data[0], meteorite_data[1]);
  sendto(sockfd, meteorite_data, sizeof(meteorite_data), 0, (struct sockaddr *)client_addr, len);
 }
 // Segnale di fine dati
 int end_signal = -1;
 printf("<SERVER>:: Invio segnale di fine dati.\n");
 sendto(sockfd, &end_signal, sizeof(end_signal), 0, (struct sockaddr *)client_addr, len);
}

// Invia un messaggio di avviso al client nel caso in cui ci sia un impatto imminente
void send_warning(int sockfd, struct sockaddr_in *client_addr, socklen_t len, int imminent_met_x, int imminent_met_y)
{
 char warning_message[255];
 printf("-----------------------------------------------------------------------------------------------\n");
 printf("imminent_met_x = %d\nimminent_met_y = %d\n", imminent_met_x, imminent_met_y);
 snprintf(warning_message, 255, "WARNING:: Impatto Imminente con il meteorite a coordinate (%d, %d)!!! Spostarsi!!!", imminent_met_x, imminent_met_y);
 int message_lenght = strlen(warning_message);
 sendto(sockfd, &message_lenght, sizeof(int), 0, (struct sockaddr *)client_addr, len);
 printf("<SERVER>:: Invio avviso di impatto imminente.\n");
 printf("-----------------------------------------------------------------------------------------------\n");
 sendto(sockfd, warning_message, strlen(warning_message), 0, (struct sockaddr *)client_addr, len);
}

// Stampa la griglia di gioco
void print_grid(int ship_x, int ship_y)
{
 char grid[GRID_SIZE][GRID_SIZE];
 memset(grid, '.', sizeof(grid));
 for(int i = 0; i < meteorite_count; i++)
 {
  if(meteorites[i][0] < GRID_SIZE && meteorites[i][1] < GRID_SIZE)
  {
   grid[meteorites[i][0]][meteorites[i][1]] = '*';
  }
 }
 if(ship_x >= 0 && ship_x < GRID_SIZE && ship_y >= 0 && ship_y < GRID_SIZE)
 {
  grid[ship_x][ship_y] = 'S';
 }
 printf("\n");
 for(int x = 0; x < GRID_SIZE; x++)
 {
  for(int y = 0; y < GRID_SIZE; y++)
  {
   printf("%c ", grid[x][y]);
  }
  printf("\n");
 }
}

// Riceve la nuova posizione della navicella
int receive_ship_position(int sockfd, struct sockaddr_in *client_addr, socklen_t *len, int *ship_x, int *ship_y)
{
 char buffer[1024];
 int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)client_addr, len);
 if(n < 0)
 {
  perror("<SERVER>:: Errore ricezione dati");
  return -1;
 }
 buffer[n] = '\0';
 int new_x, new_y;
 if(sscanf(buffer, "Posizione: %d, %d", &new_x, &new_y) != 2)
 {
  fprintf(stderr, "<SERVER>:: Errore parsing posizione navicella\n");
  return -1;
 }
 *ship_x = new_x;
 *ship_y = new_y;
 return 0;
}

int main(int argc,char **argv)
{
 struct sockaddr_in server_addr, client_addr;
 socklen_t len = sizeof(client_addr);
 // Inizializza il server
 int sockfd = initialize_server(&server_addr);
 srand(time(NULL));
 int ship_x,ship_y;
 int collision = 0;
 int imminent_collision = 0;
 int imminent_met_x, imminent_met_y;
 int collision_met_x, collision_met_y;
 int quitting = 0;
 printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
 printf("Ricorda:\nship_x = RIGA SULLA MATRICE\nship_y = COLONNA SULLA MATRICE\n");
 printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("<SERVER>:: In attesa che la navicella client sia attiva...\n");
 receive_ship_position(sockfd, &client_addr, &len, &ship_x, &ship_y);
 printf("<SERVER>:: Posizione iniziale della nave ricevuta, che comincino i giochi!\n");
 while(1)
 {
  quitting = 0;
  imminent_collision = 0;
  
  /*Genera nuovi meteoriti*/
  generate_new_meteorite(ship_x, ship_y);
  
  /*Stampa la griglia aggiornata*/
  print_grid(ship_x, ship_y);
  
  /*Invia meteoriti e avvisi*/
  send_meteorites(sockfd, &client_addr, len);
  
  /*Controlla collisioni imminenti perche' ci sono meteoriti vicini alla navicella*/
  check_warning_imminent_collision(ship_x, ship_y, &imminent_met_x, &imminent_met_y, &imminent_collision);
  if(imminent_collision)
  {
   //se vi e' un impatto imminente, va inviato il messaggio di warning
   send_warning(sockfd, &client_addr, len, imminent_met_x, imminent_met_y);
  }
  else
  {
   //altrimenti si invia la lunghezza 0 per dire che il client non si deve bloccare in attesa di bytes componenti un messaggio di warning
   int message_lenght = 0;
   sendto(sockfd, &message_lenght, sizeof(int), 0, (struct sockaddr *)&client_addr, len);
  }
  
  /*Se il client ha richiesto di uscire dalla partita, terminare il server*/
  recvfrom(sockfd, &quitting, sizeof(int), 0, (struct sockaddr *)&client_addr, &len);
  if(quitting == 1)
  {
   printf("<SERVER>:: Il client si e' disconnesso... mi disconnetto anche io!\n");
   break;
  }
  
  /*Riceve la nuova posizione della navicella*/
  if(receive_ship_position(sockfd, &client_addr, &len, &ship_x, &ship_y) < 0)
  {
   continue;
  }
  
  /*Controlla se vi e' stata una collisione*/
  check_collision(ship_x, ship_y, &collision_met_x, &collision_met_y, &collision);
  
  /*Invia l'esito del check della collisione al client, cosi' anche il client si disconnette se ha colliso*/
  sendto(sockfd, &collision, sizeof(int), 0, (struct sockaddr *)&client_addr, len);
  if(collision)
  {
   printf("<SERVER>:: Collisione rilevata con il meteorite in posizione (%d, %d)! La navicella è stata distrutta.\n", collision_met_x, collision_met_y);
   break;
  }
  
  /*Aggiorna i meteoriti*/
  update_meteorites();
  
  /*Controlla se vi e' stata una collisione anche in questo caso per evitare sovrascritture della posizione del meteorite*/
  check_collision(ship_x, ship_y, &collision_met_x, &collision_met_y, &collision);
  
  /*Invia l'esito del check della collisione al client, cosi' anche il client si disconnette se ha colliso*/
  sendto(sockfd, &collision, sizeof(int), 0, (struct sockaddr *)&client_addr, len);
  if(collision)
  {
   printf("<SERVER>:: Collisione rilevata con il meteorite in posizione (%d, %d)! La navicella è stata distrutta.\n", collision_met_x, collision_met_y);
   break;
  }
  
  sleep(1);
 }
 close(sockfd);
 return 0;
}
