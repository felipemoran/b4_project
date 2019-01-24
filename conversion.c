#include <stdio.h>
#include <ctype.h>

#define ti 0
#define ta 1
#define ec 2
#define el 3
#define none 4

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6
#define H 7
#define I 8
#define J 9
#define K 10
#define L 11
#define M 12
#define N 13
#define O 14
#define P 15
#define Q 16
#define R 17
#define S 18
#define T 19
#define U 20
#define V 21
#define W 22
#define X 23
#define Y 24
#define Z 25
#define n0 26
#define n1 27
#define n2 28
#define n3 29
#define n4 30
#define n5 31
#define n6 32
#define n7 33
#define n8 34
#define n9 35

int morse[36][5] = {
        {ti, ta, none, none, none}, // A
        {ta, ti, ti, ti, none}, // B
        {ta, ti, ta, ti, none}, // C
        {ta, ti, ti, none, none}, // D
        {ti, none, none, none, none}, // E
        {ti, ti, ta, ti, none}, // F
        {ta, ta, ti, none, none}, // G
        {ti, ti, ti, ti, none}, // H
        {ti, ti, none, none, none}, // I
        {ti, ta, ta, ta, none}, // J
        {ta, ti, ta, none, none}, // K
        {ti, ta, ti, ti, none}, // L
        {ta, ta, none, none, none}, // M
        {ta, ti, none, none, none}, // N
        {ta, ta, ta, none, none}, // O
        {ti, ta, ta, ti, none}, // P
        {ta, ta, ti, ta, none}, // Q
        {ti, ta, ti, none, none}, // R
        {ti, ti, ti, none, none}, // S
        {ta, none, none, none, none}, // T
        {ti, ti, ta, none, none}, // U
        {ti, ti, ti, ta, none}, // V
        {ti, ta, ta, none, none}, // W
        {ta, ti, ti, ta, none}, // X
        {ta, ti, ta, ta, none}, // Y
        {ta, ta, ti, ti, none}, // Z
        {ta, ta, ta, ta, ta}, // n0
        {ti, ta, ta, ta, ta}, // n1
        {ti, ti, ta, ta, ta}, // n2
        {ti, ti, ti, ta, ta}, // n3
        {ti, ti, ti, ti, ta}, // n4
        {ti, ti, ti, ti, ti}, // n5
        {ta, ti, ti, ti, ti}, // n6
        {ta, ta, ti, ti, ti}, // n7
        {ta, ta, ta, ti, ti}, // n8
        {ta, ta, ta, ta, ti}, // n9
};

int symbols[4][5] = {
        {1, 0, 2, 2, 2}, // ti
        {1, 1, 1, 0, 2}, // ta
        {0, 0, 2, 2, 2}, // ec
        {0, 0, 0, 0, 2}, // el
};

int convert_ASCII_to_bin(char* message_ascii, int message_ascii_size, int* message_binary);

int main() {
    char message_ascii[] = {'A','B',' ','B', 'A'};
    int message_ascii_size = 5;
    int message_binary[500];

    convert_ASCII_to_bin(message_ascii, message_ascii_size, message_binary);
    return 0;
}


int convert_ASCII_to_bin(char* message_ascii, int message_ascii_size, int* message_binary) {
    int index_ascii = 0;

    int message_tita[100];
    int message_tita_size = 100;
    int index_tita = 0;

    int index_binary = 0;


    int letter_index;
    char letter_upper;

    for (index_ascii=0; index_ascii < message_ascii_size; index_ascii++) {
//    while (message_ascii[index_ascii] != 0 && index_ascii < message_ascii_size) {
        letter_upper = message_ascii[index_ascii];

        if (letter_upper == ' ') {
            message_tita[index_tita++] = el;
            continue;
        }

        if (toupper(letter_upper) >= 'A' && toupper(letter_upper) <= 'Z') {
            letter_index = toupper(letter_upper) - 'A';
        } else if (letter_upper >= '0' && message_ascii[index_ascii] <= '9') {
            letter_index = letter_upper - '0' + n0;
        }

        for(int symbol_index=0; symbol_index < 5; symbol_index++) {
            if (morse[letter_index][symbol_index] == none) {
                continue;
            }
            message_tita[index_tita++] = morse[letter_index][symbol_index];
        }
        message_tita[index_tita++] = ec;
    }

    int index_tita_max = index_tita;

    for (index_tita=0; index_tita<index_tita_max; index_tita++) {
        for (int index_symbol=0; index_symbol<5; index_symbol++) {
            int symbol_binary = symbols[message_tita[index_tita]][index_symbol];

            if (symbol_binary != 2) {
                message_binary[index_binary++] = symbol_binary;
            }
        }
    }
    return 0;
}
