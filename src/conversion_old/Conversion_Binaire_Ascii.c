#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Fonction switch_to_letter

char* switch_to_letter(char* tab)
{
 char* carac = malloc(sizeof(char) * 4);
 if (strcmp(tab , "titah") == 0) carac = "A";
 else if (strcmp(tab , "tahtititi") == 0) carac = "B" ;
 else if (strcmp(tab , "tahtitahti") == 0) carac = "C" ;
 else if (strcmp(tab , "tahtiti") == 0) carac = "D" ;
 else if (strcmp(tab , "ti") == 0) carac = "E" ;
 else if (strcmp(tab , "tititahti") == 0) carac = "F" ;
 else if (strcmp(tab , "tahtahti") == 0) carac = "G" ;
 else if (strcmp(tab , "titititi") == 0) carac = "H" ;
 else if (strcmp(tab , "titi") == 0) carac = "I" ;
 else if (strcmp(tab , "titahtahtah") == 0) carac = "J" ;
 else if (strcmp(tab , "tahtitah") == 0) carac = "K" ;
 else if (strcmp(tab , "titahtiti") == 0) carac = "L" ;
 else if (strcmp(tab , "tahtah") == 0) carac = "M" ;
 else if (strcmp(tab , "tahti") == 0) carac = "N" ;
 else if (strcmp(tab , "tahtahtah") == 0) carac = "O" ;
 else if (strcmp(tab , "titahtahti") == 0) carac = "P" ;
 else if (strcmp(tab , "tahtahtitah") == 0) carac = "Q" ;
 else if (strcmp(tab , "titahti") == 0) carac = "R" ;
 else if (strcmp(tab , "tititi") == 0) carac = "S" ;
 else if (strcmp(tab , "tah") == 0) carac = "T" ;
 else if (strcmp(tab , "tititah") == 0) carac = "U" ;
 else if (strcmp(tab , "titititah") == 0) carac = "V" ;
 else if (strcmp(tab , "titahtah") == 0) carac = "W" ;
 else if (strcmp(tab , "tahtititah") == 0) carac = "X" ;
 else if (strcmp(tab , "tahtitahtah") == 0) carac = "Y" ;
 else if (strcmp(tab , "tahtahtiti") == 0) carac = "Z" ;
 else if (strcmp(tab , "titahtahtahtah") == 0) carac = "1"  ;
 else if (strcmp(tab , "tititahtahtah") == 0) carac = "2" ;
 else if (strcmp(tab , "titititahtah") == 0) carac = "3" ;
 else if (strcmp(tab , "tititititah") == 0) carac = "4" ;
 else if (strcmp(tab , "tititititi") == 0) carac = "5" ;
 else if (strcmp(tab , "tahtitititi") == 0) carac = "6" ;
 else if (strcmp(tab , "tahtahtititi") == 0) carac = "7" ;
 else if (strcmp(tab , "tahtahtahtiti") == 0) carac = "8" ;
 else if (strcmp(tab , "tahtahtahtahti") == 0) carac = "9" ;
 else if (strcmp(tab , "tahtahtahtahtah") == 0) carac = "0" ;
   return carac;
}


//Fonction_Conversion_tableau_en_char*
char* conversion_tab_str(int* tab, int N){
   char* str = malloc(sizeof(char) * N);
   int i = 0;
   for (i=0 ; i < N ; i++){
      if(tab[i]) str[i] = '1';
      else str[i] = '0';
   }
   return str;
}


//Fonction_Identification_ti_tah_e
char* Identification_Code(char* tab){
   char* res;
   if( strcmp(tab , "1") == 0 ) res = "ti";
   else if ( strcmp(tab , "111") == 0 ) res = "tah";
   else if ( strcmp(tab , "000") == 0 ) res = "ec";
   else if ( strcmp(tab , "0000000") == 0 ) res = "el";
   else res = "ev";
   return res;
}

//Fonction_Recuperation_Partie_Tableau_Entre_Deux_Indices
char* Recuperer_portion_tableau(char* tableau, int indice_debut, int indice_fin){
   char* portion;
   portion = malloc(sizeof(char) * (indice_fin - indice_debut +1));
   for (int i = indice_debut ; i< indice_fin ; i++){
      portion[i - indice_debut] = tableau[i];
   }
   return portion;
}

char** Recuperer_portion_tableau2(char** tableau, int indice_debut, int indice_fin){
   char** portion;
   portion = malloc(sizeof(char) * (indice_fin - indice_debut +1));
   for (int i = indice_debut ; i< indice_fin ; i++){
      portion[i - indice_debut] = tableau[i];
   }
   return portion;
}

//Fonction_Conversion_Binaire_ASCII
char* Conversion_binaire_ascii(char* message_binaire)
{
   int taille_message = strlen(message_binaire);
   int* tab_indice_decoupage = malloc(sizeof(int) * taille_message);
   int indice_indice_decoupage = 0;
   char** tab_ti_tah_e = malloc(sizeof(char) * taille_message);
   
   // Construction tableau qui contient les indices de découpage du code binaire
   for(int i=0 ; i < taille_message -1 ; i++){
      if( (((int)message_binaire[i] - 48) ^ ((int)message_binaire[i+1] - 48)) == 1){
         tab_indice_decoupage[indice_indice_decoupage] = i+1;
         indice_indice_decoupage = indice_indice_decoupage + 1;
      }
   }

   int indice_decoupage_max = indice_indice_decoupage;   
   char* fragment_binaire = NULL;
   
    //Extraire un tableau contenant ti_tah_ev_ec_el
    for(int i=0 ; i < indice_decoupage_max ; i++){
      if (i==0) fragment_binaire = Recuperer_portion_tableau(message_binaire,0, tab_indice_decoupage[0]); 
    else fragment_binaire = Recuperer_portion_tableau(message_binaire, tab_indice_decoupage[i-1], tab_indice_decoupage[i]);
      char* mot_de_code = Identification_Code(fragment_binaire);
      tab_ti_tah_e[i] = mot_de_code;
   }
   
   //Enlever les ev
   int k = 0;
   char** tab_ti_tah_ecl = malloc(sizeof(char) * taille_message);
   for(int i=0 ; i < indice_decoupage_max ; i++){
      if (strcmp(tab_ti_tah_e[i] , "ev") != 0){
         tab_ti_tah_ecl[k] = tab_ti_tah_e[i];
         k = k + 1;
      }
   }   
   int taille_tab_symb = k;

   //Recuperer le message
   char message[200] ; 
   memset(message , '\0' , 200 );
   char lettre_ti_tah[500] ;
   memset(lettre_ti_tah , '\0' , 500 );
   char bout_lettre[500] ;
   memset(bout_lettre , '\0' , 500 );
   for(int i = 0 ; i < 1000 ; i++) bout_lettre[i] = '\0';
   bout_lettre[0] = '\0' ;
   for (int i=0 ; i < taille_tab_symb  ; i++){
      if ((strcmp(tab_ti_tah_ecl[i] , "ec") != 0)  &&  (strcmp(tab_ti_tah_ecl[i] , "el") != 0)){
         strcat(bout_lettre , tab_ti_tah_ecl[i]);
         printf("%s \n" , bout_lettre);
      }
      else if (strcmp(tab_ti_tah_ecl[i] , "ec") == 0){
          strcat(message , switch_to_letter(bout_lettre));
          printf("%s \n" , message);
          memset(bout_lettre , '\0' , 500 );
      }
      else if (strcmp(tab_ti_tah_ecl[i] , "el") == 0){
          strcat(message , switch_to_letter(bout_lettre));
          strcat(message , " ");
          memset(bout_lettre , '\0' , 500 );
      } 
   }
   
   return message;
}
int main()
{
   char* bin = "10111000101110001110101010001";
   char* res = malloc(sizeof(char) * 10);
   res = Conversion_binaire_ascii(bin);
   printf("%s \n" , res);   

}
