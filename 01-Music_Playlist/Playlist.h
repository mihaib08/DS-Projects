// Copyright [2020] Mihai-Eugen Barbu [legal/copyright]
#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#define LMAX 512

union record {
    char mus[94];
    struct data {
        char title[30];
        char artist[30];
        char album[30];
        char year[4];
    } data;
};

struct Node {
    char data[LMAX];
    struct Node *next;
    struct Node *prev;
};

struct LinkedList {
    int size;
    struct Node *head;
    struct Node *tail;
    struct Node *curr;
};

#endif  // PLAYLIST_H_
