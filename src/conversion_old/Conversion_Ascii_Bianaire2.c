#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Ascii_Binaire.h"

int main()
{  
   char* msg = "ABCD";
   char* binaire  = malloc(sizeof(char) * 10000);
   binaire = conversion_ascii_binaire(msg);
   printf("%s \n" , binaire);
}


//Fonction_Convert
char* convert(char lettre){
   char* equivalent = malloc(sizeof(char) * 20);
   switch(lettre){
      case 48 : 
         equivalent = "11101110111011101110";
         break;
      case 49 : 
         equivalent = "101110111011101110";
         break;
      case 50 : 
         equivalent = "1010111011101110";
         break;
      case 51 : 
         equivalent = "10101011101110";
         break;
      case 52 : 
         equivalent = "101010101110";
         break;
      case 53 : 
         equivalent = "1010101010";
         break;
      case 54 : 
         equivalent = "111010101010";
         break;
      case 55 : 
         equivalent = "11101110101010";
         break;
      case 56 : 
         equivalent = "1110111011101010";
         break;
      case 57 : 
         equivalent = "111011101110111010";
         break;
      case 65 : //A
         equivalent = "101110";
         break;
      case 66 : //B
         equivalent = "1110101010";
         break; 
      case 67 : //C
         equivalent = "1110111010";
         break;
      case 68 : //D
         equivalent = "11101010";
         break;
      case 69 : //E
         equivalent = "10";
         break;
      case 70 : //F
         equivalent = "1010111010";
         break;
      case 71 : //G
         equivalent = "1110111010";
         break;
      case 72 : //H
         equivalent = "10101010";
         break;
      case 73 : //I
         equivalent = "1010";
         break;
      case 74 : //J
         equivalent = "10111011101110";
         break;
      case 75 : //K
         equivalent = "1110101110";
         break;
      case 76 : //L
         equivalent = "1011101010";
         break;
      case 77 : //M
         equivalent = "11101110";
         break;
      case 78 : //N
         equivalent = "111010";
         break;
      case 79 : //O
         equivalent = "111011101110";
         break;
      case 80 : //P
         equivalent = "101110111010";
         break;
      case 81 : //Q
         equivalent = "11101110101110";
         break;
      case 82 : //R
         equivalent = "10111010";
         break;
      case 83 : //S
         equivalent = "101010";
         break;
      case 84 : //T
         equivalent = "1110";
         break;
      case 85 : //U
         equivalent = "10101110";
         break;
      case 86 : //V
         equivalent = "1010101110";
         break;
      case 87 : //W
         equivalent = "1011101110";
         break;
      case 88 : //X
         equivalent = "111010101110";
         break;
      case 89 : //Y
         equivalent = "11101011101110";
         break;
      case 90 : //Z
         equivalent = "111011101010";
         break;
      default :
         equivalent = "";
         break;s
   return equivalent;
   }
 
}

//Fonction_Conversion
char* conversion_ascii_binaire(char* message_ascii)
{
   int taille_message = strlen(message_ascii);
   char* inter = malloc(sizeof(char) * 16);
   char* message = malloc(sizeof(char) * (taille_message) * 20);
   strcpy(message , convert(message_ascii[0]));
   for (int i = 1 ; i < taille_message -1 ; i++){
      strcpy(inter , convert(message_ascii[i]));
      strcat(message, inter);
   }
   return message;   
}

