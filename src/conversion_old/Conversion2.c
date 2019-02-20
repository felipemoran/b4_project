#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// tableau de référence pour identifier les lettres vis-a-vis la symbolisation titah
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



/*   L'idée repose sur le fait que pendant la réception du message, il sera de la forme de 2 tableaux : l'état de l'interruption (niveau haut ou bas) , et la durée de cette interruption
     Du coup on definit la fonction conversion qui prend en paramètre le tableau des états (tableau char de 1 et 0) , et le tableau de durées, de même taille que le premier tableau qui indique la durée de chaque niveau d'interruption.
*/



char* conversion_binaire_ascii(char* message_binaire , int* tab_duree , int duree_impulsion)
{
   int taille_message = strlen(message_binaire);
   int taille_tab_ti_tah = taille_message * 5 ;
   char tab_ti_tah[15] ;
   memset(tab_ti_tah , '\0' ,15);
   char* message = malloc(sizeof(char) * taille_tab_ti_tah);
   for (int i = 0 ; i < taille_message ; i++){
      if((message_binaire[i] == '1') && (tab_duree[i] == duree_impulsion)){
         strcat(tab_ti_tah , "ti");
      }
      else if ((message_binaire[i] == '1') && (tab_duree[i] == (3 * duree_impulsion))) strcat(tab_ti_tah , "tah");
      else if ((message_binaire[i] == '0') && (tab_duree[i] == (3 * duree_impulsion))){
         strcat(message , switch_to_letter(tab_ti_tah));
         memset(tab_ti_tah , '\0' , 15);
         continue;
      }
      else if ((message_binaire[i] == '0') && (tab_duree[i] == (7 * duree_impulsion))){
         memset(tab_ti_tah , '\0' , 15);
         continue;
      }
      else continue;
   }
  strcat(message , switch_to_letter(tab_ti_tah));  
  return message;


}
int main()
{
    char* tab_symb = "101010101";
    int tab_duree[9] = {1,1,3,3,3,3,1,7,1};
    char* res = conversion_binaire_ascii(tab_symb, tab_duree, 1);
    printf("%s \n" , res);
}

