#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include "morse.h"
#include "speaker.h"

#define MIN_WPM                5
#define MAX_WPM               25 
#define PARIS_DOTS            50
#define MINUT_SEC             60
#define MICRO_SEC        1000000

/* ################# [ Declare methods ] ################ */
void printInfo(int mode);
void inicializeMorse(int WPM);
int launchMorse(char * message);
void handler(int signal);
void interrupt_handler(int signal);
void finalize_handler(int signal, siginfo_t * info, void * others);

/* ################# [ Global vars ] #################### */
message_t messageMorse;
time_t id_clock;
int finalized = 0;

void finalize_handler(int signal, siginfo_t * info, void * others) {
   
   if (info->si_value.sival_int == 0) {
      fprintf(stdout, "\n[ + ] Finalitzacio correcta Estat: [ INTERRUPTED ]\n\n");
   }
   else {
      fprintf(stdout, "\n[ + ] Finalitzacio correcta Estat: [ OK ]\n\n");
   }
   finalized = 1;
}

void interrupt_handler(int signal) {
   union sigval value;

   fprintf(stdout, "\n[ + ] L'usuari ha cancel.lat la comunicacio, parant speaker ...\n");
   
   speaker_disable(); timer_delete(id_clock);
   fprintf(stdout, "\n[ + ] Speaker parat correctament [ OK ]\n");
   
   /* Send finalize signal */
   value.sival_int = 0; 
   sigqueue(getpid(), SIGRTMIN + 1, value);   
}

void handler(int signal) {
   static int spkOn = 0;
   union sigval value;

   if ((messageMorse.buffer[messageMorse.counter] == '.') && (!spkOn)) {
      /* activem el speaker */
      speaker_enable();
      spkOn = 1;         
   }
   else if ((messageMorse.buffer[messageMorse.counter] == '\x20') && (spkOn)) {
      /* desectivem el speaker */
      speaker_disable();
      spkOn = 0; 
   }
 
   if (messageMorse.counter == strlen(messageMorse.buffer)) { 
      speaker_disable(); timer_delete(id_clock);
      value.sival_int = 1; sigqueue(getpid(), SIGRTMIN + 1, value); 
   }  
   else messageMorse.counter++;
}

/* ######################################################
   ##  [ + ] Proc main: cast string to morse code      ##
   ###################################################### */
    
int main(int argc, char ** argv) {
   int wpm;

   if (argc == 3) {
      wpm = atoi(argv[2]);
      if ((wpm >= MIN_WPM) && (wpm <= MAX_WPM)) {
         inicializeMorse(wpm); 
         if (launchMorse(argv[1]) != 0) { printInfo(3); return -3; } 
      } 
      else { 
         printInfo(2);
         return -2;
      }
   }
   else {
      printInfo(1);
      return -1;
   }
}

int castingStringToMorse(char * message) {
   int i, iniPos, j, err = 0;
   int find_letter;
   unsigned int find_total_letters = 0;
   unsigned int total_len = 0;

   message = strupr(message);
   if (strlen(message) > 0) {
      for (i = 0; ((i < strlen(message)) && (message[i] == '\x20')); i++); 

      if (i < strlen(message)) {
         iniPos = i;
         for (; ((i < strlen(message)) && (err == 0)); i++) {
            find_letter = 0;
            if (message[i] != '\x20') {
               for (j = 0; ((j < MAX_LETTERS) && (find_letter == 0)); j++) {
                  if (morse_table[j].letter == message[i]) {
                     find_total_letters++;
                     if (i == iniPos) { 
                        total_len = strlen(morse_table[j].signals);
                        if (total_len < SIZE_MESSAGE) {
                           strcpy(messageMorse.buffer, morse_table[j].signals);
                        }
                        else err = -1;
                     }            
                     else {
                        total_len += strlen(morse_table[j].signals) + 3;
                        if (total_len < SIZE_MESSAGE) {
                           strcat(messageMorse.buffer, "   "); 
                           strcat(messageMorse.buffer, morse_table[j].signals);
                        }
                        else err = -1;
                     }
            
                     find_letter = 1;            
                  }   
               }
               if ((find_letter == 0) && (j == MAX_LETTERS)) {
                  err = -1;
               }
            }
            else {
               for (;((i < strlen(message)) && (message[i] == '\x20')); i++);
               if (i < strlen(message)) {
                  i--;
                  total_len += 7;
                  if (total_len < SIZE_MESSAGE) {
                     total_len += 7;
                     strcat(messageMorse.buffer, "       ");
                  }
                  else err = -1;
               }
            }
         }
      }
      else return -2;
   }
   else return -1;
   
   if ((err == 0)) {
      return 0;
   }
   else return -4;
}

int launchMorse(char * message) {
   struct sigaction m_saction; 
   struct sigaction int_saction;
   struct sigaction f_saction;
   struct sigevent m_sigevent;    
   struct itimerspec m_itimer;

   /* Traduim el missatge */
   if (castingStringToMorse(message) == 0) {
      fprintf(stdout, "\n[ - ] Codi Traduit: %s\n", messageMorse.buffer);    

      /* Capture interval timer signal */
      sigemptyset(&m_saction.sa_mask);
      sigaddset(&m_saction.sa_mask, SIGRTMIN);
      sigaddset(&m_saction.sa_mask, SIGRTMIN + 1);
      sigaddset(&m_saction.sa_mask, SIGINT);
      m_saction.sa_flags = 0;
      m_saction.sa_handler = &handler;
      sigaction(SIGRTMIN, &m_saction, NULL);

      /* Capture control+c keys => disable speaker */
      sigemptyset(&int_saction.sa_mask);
      sigaddset(&int_saction.sa_mask, SIGRTMIN); 
      sigaddset(&int_saction.sa_mask, SIGRTMIN + 1);  
      sigaddset(&int_saction.sa_mask, SIGINT);
      int_saction.sa_flags = 0;   
      int_saction.sa_handler = &interrupt_handler;
      sigaction(SIGINT, &int_saction, NULL);
  
      /* Capture finalize signal */
      sigemptyset(&f_saction.sa_mask);
      sigaddset(&f_saction.sa_mask, SIGRTMIN); 
      sigaddset(&f_saction.sa_mask, SIGRTMIN + 1);
      sigaddset(&f_saction.sa_mask, SIGINT);
      f_saction.sa_flags = SA_SIGINFO;
      f_saction.sa_sigaction = &finalize_handler;
      sigaction(SIGRTMIN + 1, &f_saction, NULL);
  
      m_sigevent.sigev_notify = SIGEV_SIGNAL;
      m_sigevent.sigev_signo = SIGRTMIN;

      m_itimer.it_value.tv_sec = 0;
      m_itimer.it_value.tv_nsec = MICRO_SEC - 1;
      m_itimer.it_interval.tv_sec = 0;
      m_itimer.it_interval.tv_nsec = messageMorse.usec * 1000;

      timer_create(CLOCK_REALTIME, &m_sigevent, &id_clock);
      timer_settime(id_clock, 0, &m_itimer, NULL);

      /* Wait finalize */
      for(;(finalized == 0);) pause(); 
 
      return 0; 
   }
   else return -1;
}

/* ######################################################
   ##  MorseInicialitzar: inicialize morse system      ##
   ###################################################### */

void inicializeMorse(int WPM) {
   messageMorse.counter = 0;
   messageMorse.usec = ((MINUT_SEC * MICRO_SEC)/(WPM * PARIS_DOTS));
}

/* ######################################################
   ##  printInfo: print information program to execute ##
   ###################################################### */

void printInfo(int mode) {
   fprintf(stderr, "\n\n_____________________________ [ ERROR ] _____________________________\n\n");
   if (mode == 1) fprintf(stderr, "\t[ ERROR ] El nombre de parametres introduits es incorrecte\n");
   else if (mode == 2)  fprintf(stderr, "\t[ ERROR ] El rang de les paraules_per_minut es: %d a %d\n", MIN_WPM, MAX_WPM);
   else if (mode == 3) fprintf(stderr, "\t[ ERROR ] No s'ha pogut realitzar la traduccio a codi Morse\n\t          Caracters invalids o missatge massa gran\n");
 
   fprintf(stderr, "\t[ CARGA ] sitr frase_morse paraules_per_minut\n");
   fprintf(stderr, "\t[ EX -> ] sitr \"aixo es un test\" 20\n"); 
   fprintf(stderr, "_____________________________________________________________________\n\n");
}