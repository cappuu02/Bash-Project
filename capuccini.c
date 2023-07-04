#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

char str[20];
int num_filosofi = 0; //numero da avere obbligatorio
int flag_stallo, flag_soluzione, flag_starvation = 0; //valori interi
sem_t sem; // semaforo
struct timespec tempo;
sem_t * forchetta[5]; //creo un array di fochette (tante quanti i filosofi)
int pipefd[2]; //Array dei file descriptor
int count_stallo = 0; //variabile che utilizzo per gestire il problema di stallo
int check = 0; //VARIABILE PER USCIRE DA STALLO E STARVATION
struct timespec tempo_starvation;
int a = 0; //utilizzati nello starvation
int b = 0; //utilizzati nello starvation


//FUNZIONE CHE CANCELLA LE FORCHETTE
void cancella_forchetta(){
    for (int j = 0; j < num_filosofi; j++) {
        sprintf(str, "%d", j);
        sem_close(forchetta[j]);
        sem_unlink(str);
    }
    close(pipefd[0]);
    close(pipefd[1]);

    return;
}

//FUNZIONE HEADLER PER GESTIRE IL CTRL+C
void f_handler(int iSignalcode){
    printf("Handler: Ricevuto signal [CTRL+C]: %d\n", iSignalcode);
    cancella_forchetta();
    check = 1;
    return;
}

//FUNZIONE INIZIO CENA FILOSOFI
void inizio_cena(sem_t * forchetta[], int i, int num_filosofi){
    tempo.tv_sec = 0;//0 secondi
    tempo.tv_nsec = 500000000;//cinquecentomilioni di nanosecondi
    while(1){
        //CASISTICA CON TUTTI INPUT 0
        if(flag_stallo == 0 && flag_soluzione == 0 && flag_starvation == 0){
            printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), i);
            sem_wait(forchetta[i]);
            printf("il filosofo con pid %d HA PRESO la forchetta destra\n", getpid());

            printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), (i+1)%num_filosofi);
            sem_wait(forchetta[(i+1)%num_filosofi]);
            printf("il filosofo con pid %d HA PRESO la forchetta sinistra\n", getpid());
        }
        //CASISTICA PROBLEMA DELLO STALLO
        if(flag_stallo == 1 && flag_soluzione == 0 && flag_starvation == 0){
            printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), i);
            sem_wait(forchetta[i]);
            printf("il filosofo con pid %d HA PRESO la forchetta destra\n", getpid());

            sleep(1);//FAVORISCO LO STALLO 

            printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), (i+1)%num_filosofi);

            read(pipefd[0], &count_stallo, sizeof(int));
            count_stallo++;
            printf("incremento count stallo\n");
            write(pipefd[1], &count_stallo, sizeof(int));
            if(count_stallo == num_filosofi){ //STALLO RILEVATO
                printf("\n[ERROR]: STALLO RILEVATO\n");
                kill(0, SIGINT);
                if(check == 1){
                    printf("escoooo filosofo %d\n", getpid());
                    break;
                }
            }

            sem_wait(forchetta[(i+1)%num_filosofi]);
            printf("il filosofo con pid %d HA PRESO la forchetta sinistra\n", getpid());
            read(pipefd[0], &count_stallo, sizeof(int));

            count_stallo--;
            printf("DECREMENTO\n");
            write(pipefd[1], &count_stallo, sizeof(int));
        }

        //SOLUZIONE STALLO
        if((flag_soluzione == 1 && flag_stallo == 0 && flag_starvation == 0) || (flag_soluzione == 1 && flag_stallo == 1 && flag_starvation == 0)){
            int destra = i;
            int sinistra = i+1 % num_filosofi;

            if(i == num_filosofi - 1){
                destra = i+1 % num_filosofi;
                sinistra = i;
            }

            printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), destra);
            sem_wait(forchetta[destra]);
            printf("il filosofo con pid %d HA PRESO la forchetta destra %d\n", getpid(), destra);

            printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), sinistra);
            sem_wait(forchetta[sinistra]);
            printf("il filosofo con pid %d HA PRESO la forchetta sinistra %d\n", getpid(), sinistra);
        }

        //MANGIA CON STARVATION
        if((flag_starvation == 1 && flag_soluzione == 0 && flag_stallo == 0) || (flag_starvation == 1 && flag_soluzione == 1 && flag_stallo == 0)){
            int right = i;
            int left = (i+1)% num_filosofi;

            //inversione della forchetta dx/sx per l'ultimo filosofo
            if (i == num_filosofi - 1) {
            right= (i + 1) % num_filosofi;
            left= i;
            }
                clock_gettime(CLOCK_REALTIME, &tempo_starvation);
                tempo_starvation.tv_sec += 8;
                printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), right);
                a = sem_timedwait(forchetta[i], &tempo_starvation);
                if(a == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                printf("il filosofo con pid %d HA PRESO la forchetta destra\n", getpid());
                fflush(stdout);

                printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), left);
                b = sem_timedwait(forchetta[(i+1)%num_filosofi], &tempo_starvation);
                if(b == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                printf("il filosofo con pid %d HA PRESO la forchetta sinistra\n", getpid());
            }

            //PROBLEMA STALLO CON STARVATION (DIFFICILE)
            if(flag_stallo == 1 && flag_starvation == 1 && flag_soluzione == 0){
                clock_gettime(CLOCK_REALTIME, &tempo_starvation);
                tempo_starvation.tv_sec += 8;

               printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), i);
                a = sem_timedwait(forchetta[i], &tempo_starvation);
                if(a == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                printf("il filosofo con pid %d HA PRESO la forchetta destra\n", getpid());

                read(pipefd[0], &count_stallo, sizeof(int));
                count_stallo++;
                printf("INCREMENTO COUNT STALLO\n");
                write(pipefd[1], &count_stallo, sizeof(int));
                if(count_stallo == num_filosofi){
                    printf("\n[ERROR]: STALLO RILEVATO\n");
                    printf("HO CHIUSO I FILE DESCRIPTOR IN LETTURA E SCRITTURA\n");
                    kill(0, SIGINT); //non bisogna fare cancella forchetta dato che lo facciamo già nel SIGINT (CTRL+C )
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                }

                printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), (i+1)%num_filosofi);
                b = sem_timedwait(forchetta[(i+1)%num_filosofi], &tempo_starvation);
                if(b == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                printf("il filosofo con pid %d HA PRESO la forchetta sinistra\n", getpid());

                read(pipefd[0], &count_stallo, sizeof(int));
                count_stallo--;
                printf("Decremento");
                write(pipefd[1], &count_stallo, sizeof(int));
            }

            //IF ULTIMO CASO
            if(flag_stallo == 1 && flag_soluzione == 1 && flag_starvation == 1){
                tempo.tv_nsec = 900000000;//cinquecentomilioni di nanosecondi
                int right = i;
                int left = (i+1)% num_filosofi;

                //inversione della forchetta dx/sx per l'ultimo filosofo
                if (i == num_filosofi - 1) {
                    right= (i + 1) % num_filosofi;
                    left= i;
                }

                clock_gettime(CLOCK_REALTIME, &tempo_starvation);
                tempo_starvation.tv_sec += 8;

               printf("il filosofo con pid %d ATTENDO la forchetta destra %d\n", getpid(), right);
                a = sem_timedwait(forchetta[right], &tempo_starvation);
                if(a == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                if(a != -1){
                printf("il filosofo con pid %d HA PRESO la forchetta destra\n", getpid());

                read(pipefd[0], &count_stallo, sizeof(int));
                count_stallo++;
                printf("INCREMENTO COUNT STALLO\n");
                write(pipefd[1], &count_stallo, sizeof(int));
                if(count_stallo == num_filosofi){
                    printf("\n[ERROR]: STALLO RILEVATO\n");
                    printf("HO CHIUSO I FILE DESCRIPTOR IN LETTURA E SCRITTURA\n");
                    kill(0, SIGINT); //non bisogna fare cancella forchetta dato che lo facciamo già nel SIGINT (CTRL+C )
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                }

                printf("il Filosofo con pid %d ATTENDO la forchetta sinistra %d\n", getpid(), left);
                b = sem_timedwait(forchetta[left], &tempo_starvation);
                if(b == -1){
                    if(errno == ETIMEDOUT){
                    printf("[ERROR]: rilevata starvation\n");
                    kill(0, SIGINT);
                    if(check == 1){
                        printf("escoooo filosofo %d\n", getpid());
                        break;
                    }
                    }
                }
                }
                if(b != -1){
                    printf("il filosofo con pid %d HA PRESO la forchetta sinistra\n", getpid());

                    read(pipefd[0], &count_stallo, sizeof(int));
                    count_stallo--;
                    printf("Decremento\n");
                    write(pipefd[1], &count_stallo, sizeof(int));
                }
            }
        printf("sta mangiando filosofo %d\n", getpid());
        nanosleep(&tempo, NULL);
        sem_post(forchetta[i]);
        printf("il filosofo con pid %d sta LASCIANDO la forchetta destra %d\n", getpid(), i);
        sem_post(forchetta[(i+1)%num_filosofi]);
        printf("il filosofo con pid %d sta LASCIANDO la forchetta sinistra %d\n", getpid(), (i+1)%num_filosofi);
    }//fine while
}//FINE FUNZIONE


//FUNZIONE MAIN
int main(int argc, char *argv[]){
    struct sigaction sa; //sigaction è il tipo della struct
    memset(&sa, '\0', sizeof(struct sigaction));
    sa.sa_handler = f_handler;
    sigaction(SIGINT, &sa, NULL);


    if(argc < 2) {
		printf("Usage: %s  no argument\n", argv[0]);
		exit(EXIT_FAILURE);
	}
    num_filosofi = atoi(argv[1]); //numero dei filosofi del programma


    //FLAG STALLO
    if(argv[2] != NULL){
      flag_stallo = atoi(argv[2]);  //atoi converte un char in int
      if(flag_stallo == 0){
        printf("\n[FLAG]: flag STALLO disattivato\n");
      }
      else{
      printf("\n[FLAG]: flag STALLO attivato\n");
    }
    }
    else{
        printf("\n[FLAG]: flag STALLO disattivato\n");
    }


    //FLAG SOLUZIONE
    if(argv[3] != NULL){
      flag_soluzione = atoi(argv[3]);  //atoi converte un char in int
      if(flag_soluzione == 0){
        printf("\n[FLAG]: flag SOLUZIONE disattivato\n");
      }
      else{
      printf("\n[FLAG]: flag SOLUZIONE attivato\n");
    }
    }
    else{
        printf("\n[FLAG]: flag SOLUZIONE disattivato\n");
    }


    //FLAG STARVATION
    if(argv[4] != NULL){
      flag_starvation = atoi(argv[4]);  //atoi converte un char in int
      if(flag_starvation == 0){
        printf("\n[FLAG]: flag STARVATION disattivato\n");
      }
      else{
      printf("\n[FLAG]: flag STARVATION attivato\n");
    }
    }
    else{
        printf("\n[FLAG]: flag STARVATION disattivato\n");
    }


    //CREAZIONE DELLE PIPE
    if (pipe(pipefd) == -1) { //se pipe == -1 allora ho un ERRORE!!!
        perror("pipe");
        exit(EXIT_FAILURE); //se ho un errore allora esco dal programma
    }
    write(pipefd[1], &count_stallo, sizeof(int));


    //CREAZIONE FORCHETTE (SEMAFORI)
    for(int j = 0; j < num_filosofi; j++){ //ciclo for per le forchette
        snprintf(str,sizeof(str), "%d", j);
        if((forchetta[j] = sem_open(str, O_CREAT, S_IRWXU ,1)) == SEM_FAILED){
            printf("Errore in sem_open, errno = %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }

    //CREAZIONE FORK
    pid_t filosofo[num_filosofi];
    for(int i = 0; i < num_filosofi; i++){
        pid_t pid = fork();
        if(pid == -1) {
		        perror("[NOTICE]: Errore in fork!\n");
		          exit(-1);
        } else if(pid == 0) {
                    printf("\n");
                    printf("sono il Filosofo num. %d con Pid %d\n", i, getpid());
                    inizio_cena(forchetta, i, num_filosofi);
                    exit(0);
            }else{
                filosofo[i] = pid;
            }
    }//fine for

  

    for(int i = 0; i < num_filosofi; i++){
        waitpid(filosofo[i], NULL, 0);
        printf("il filosofo %d sta lasciando la cena...\n", filosofo[i]);
    }

        //cancella_forchetta();
        printf("\n[FINE DELLA CENA DEI FILOSOFI]\n");
        printf("\nSaluti dal parent dei filosofi %d\n", getppid());

        
        
        return 0;//return permette al processo padre di terminare al termine del programma
}//fine main

//VERSIONE 4 LUGLIO (errata: il problema risulta essere la funzione cancella)