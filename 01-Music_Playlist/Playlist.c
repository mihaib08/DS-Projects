// Copyright [2020] Mihai-Eugen Barbu [legal/copyright]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./Playlist.h"

void init_list(struct LinkedList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->curr = NULL;
    list->size = 0;
}

void add_first(struct LinkedList *list, char *song) {
    struct Node *new;
    if (list == NULL) {
        return;
    }
    new = (struct Node *)malloc(sizeof(struct Node));
    snprintf(new->data, strlen(song) + 1, "%s\n", song);
    if (list->head == NULL) {
    	list->head = new;
    	list->tail = new;
    	list->curr = new;
    	list->head->prev = NULL;
        list->tail->next = NULL;
    } else {
        new->next = list->head;
        new->prev = NULL;
        list->head->prev = new;
        list->head = new;
    }
    list->size++;
}

void add_last(struct LinkedList *list, char *song) {
    struct Node *new;
    if (list == NULL) {
        return;
    }
    new = (struct Node *)malloc(sizeof(struct Node));
    snprintf(new->data, strlen(song) + 1, "%s\n", song);
    if (list->head == NULL) {
        list->head = new;
        list->tail = new;
        list->curr = new;
        list->head->prev = NULL;
        list->tail->next = NULL;
    } else {
        new->next = NULL;
        new->prev = list->tail;
        list->tail->next = new;
        list->tail = new;
    }
    list->size++;
}

void add_after(struct LinkedList *list, char *song) {
    struct Node *new;
    if (list->curr == list->tail) {
        add_last(list, song);
    } else {
        new = (struct Node *)malloc(sizeof(struct Node));
        snprintf(new->data, strlen(song) + 1, "%s\n", song);
        new->next = list->curr->next;
        new->prev = list->curr;
        list->curr->next->prev = new;
        list->curr->next = new;
        list->size++;
    }
}

void removeSong(struct LinkedList *list, struct Node *aux) {
    struct Node *del;
    if (list->curr == aux) {
        if (list->curr->next != NULL) {
            list->curr = list->curr->next;
        } else {
            list->curr = list->curr->prev;
        }
    }
    if (aux == list->head) {
        list->head->next->prev = NULL;
        list->head = list->head->next;
    } else if (aux == list->tail) {
        list->tail->prev->next = NULL;
        list->tail = list->tail->prev;
    } else {
        del = aux->prev;
        del->next->next->prev = del;
        del->next = del->next->next;
    }
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
        list->curr = NULL;
    }
    list->size--;
}

void check_song(struct LinkedList *list, char *song) {
    struct Node *aux;
    if (list == NULL || list->head == NULL) {
        return;
    }
    aux = list->head;
    while (aux != NULL) {
        if (strcmp(aux->data, song) == 0) {
            removeSong(list, aux);
            free(aux);
            return;
        }
        aux = aux->next;
    }
    return;
}

void add_interpreter(struct LinkedList *list, char *com, char *song) {
    /* Se verifica daca melodia de adaugat se afla deja in lista */
    check_song(list, song);
    if (strcmp(com, "ADD_FIRST") == 0) {
        add_first(list, song);
    } else if (strcmp(com, "ADD_LAST") == 0) {
        add_last(list, song);
    }
}

void del_first(struct LinkedList *list, FILE *fout) {
    struct Node *aux;
    if (list == NULL || list->head == NULL) {
        fprintf(fout, "Error: delete from empty playlist\n");
        return;
    }
    if (list->head == list->curr) {
        list->curr = list->curr->next;
    }
    if (list->size == 1) {
        aux = list->head;
        list->head = NULL;
        list->tail = NULL;
        list->curr = NULL;
        free(aux);
    } else {
        aux = list->head;
        list->head->next->prev = NULL;
        list->head = list->head->next;
        free(aux);
    }
    list->size--;
}

void del_last(struct LinkedList *list, FILE *fout) {
    struct Node *aux;
    if (list == NULL || list->head == NULL) {
        fprintf(fout, "Error: delete from empty playlist\n");
        return;
    }
    if (list->tail == list->curr) {
        list->curr = list->curr->prev;
    }
    if (list->size == 1) {
        aux = list->head;
        list->head = NULL;
        list->tail = NULL;
        list->curr = NULL;
        free(aux);
    } else {
        aux = list->tail;
        list->tail->prev->next = NULL;
        list->tail = list->tail->prev;
        free(aux);
    }
    list->size--;
}

void del_curr(struct LinkedList *list, FILE *fout) {
    struct Node *aux, *del;
    if (list->curr == NULL) {
        fprintf(fout, "Error: no track playing\n");
        return;
    }
    if (list->curr == list->head) {
        del_first(list, fout);
    } else if (list->curr == list->tail) {
        del_last(list, fout);
    } else {
        aux = list->curr;
        del = aux;
        aux = aux->prev;
        aux->next->next->prev = aux;
        aux->next = aux->next->next;
        list->curr = aux->next;
        list->size--;
        free(del);
    }
}

void del_interpreter(struct LinkedList *list, char *com, FILE *fout) {
    if (strcmp(com, "DEL_FIRST") == 0) {
        del_first(list, fout);
    } else if (strcmp(com, "DEL_LAST") == 0) {
        del_last(list, fout);
    } else if (strcmp(com, "DEL_CURR") == 0) {
        del_curr(list, fout);
    }
}

void move_interpreter(struct LinkedList *list, char *com, FILE *fout) {
    if (list->curr == NULL) {
        fprintf(fout, "Error: no track playing\n");
        return;
    }
    if (strcmp(com, "MOVE_NEXT") == 0) {
        if (list->curr != list->tail) {
            list->curr = list->curr->next;
        }
    } else {
        if (list->curr != list->head) {
            list->curr = list->curr->prev;
        }
    }
}

void show_interpreter(struct LinkedList *list, char *com, FILE *fout) {
    struct Node *aux;
    union record info;
    char mus[LMAX];
    if (strcmp(com, "SHOW_FIRST") == 0) {
        aux = list->head;
    } else if (strcmp(com, "SHOW_LAST") == 0) {
        aux = list->tail;
    } else if (strcmp(com, "SHOW_CURR") == 0) {
        aux = list->curr;
    }
    if (aux == NULL) {
        fprintf(fout, "Error: show empty playlist\n");
        return;
    }
    snprintf(mus, sizeof(mus), "%s", "songs/");
    snprintf(mus + 6, strlen(aux->data) + 1, "%s\n", aux->data);
    FILE *f = fopen(mus, "rb");
    fseek(f, -94, SEEK_END);
    fread(&info, 1, sizeof(union record), f);
    fprintf(fout, "Title: %.30s\n", info.data.title);
    fprintf(fout, "Artist: %.30s\n", info.data.artist);
    fprintf(fout, "Album: %.30s\n", info.data.album);
    fprintf(fout, "Year: %.4s\n", info.data.year);
    fclose(f);
}

void show_playlist(struct LinkedList *list, FILE *fout) {
    struct Node *aux;
    char mus[LMAX];
    FILE *f;
    union record info;
    if (list == NULL || list->head == NULL) {
        fprintf(fout, "[]\n");
        return;
    }
    aux = list->head;
    snprintf(mus, sizeof(mus), "%s", "songs/");
    snprintf(mus + 6, strlen(aux->data) + 1, "%s\n", aux->data);
    f = fopen(mus, "rb");
    fseek(f, -94, SEEK_END);
    fread(&info, 1, sizeof(union record), f);
    fprintf(fout, "[%s", info.data.title);
    aux = aux->next;
    fclose(f);
    while (aux != NULL) {
        snprintf(mus, sizeof(mus), "%s", "songs/");
        snprintf(mus + 6, strlen(aux->data) + 1, "%s\n", aux->data);
        f = fopen(mus, "rb");
        fseek(f, -94, SEEK_END);
        fread(&info, 1, sizeof(union record), f);
        fprintf(fout, "; %.30s", info.data.title);
        fclose(f);
        aux = aux->next;
    }
    fprintf(fout, "]\n");
}

void free_list(struct LinkedList **pp_list) {
    struct Node *aux, *urm;
    if ((*pp_list) == NULL || (*pp_list)->head == NULL) {
        return;
    }
    urm = (*pp_list)->head;
    while (urm != NULL) {
        aux = urm->next;
        free(urm);
        urm = aux;
    }
    free(urm);
    (*pp_list)->head = NULL;
    (*pp_list)->tail = NULL;
    (*pp_list)->curr = NULL;
}

int main(int argc, char *argv[]) {
    struct LinkedList *list;
    char inMedia[LMAX], outMedia[LMAX], com[LMAX], c, song[LMAX];
    int n, i, sz;
    list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    init_list(list);

    /* Fisiere in/out */
    snprintf(inMedia, strlen(argv[1]) + 1, "%s\n", argv[1]);
    snprintf(outMedia, strlen(argv[2]) + 1, "%s\n", argv[2]);
    FILE *fin = fopen(inMedia, "r");
    FILE *fout = fopen(outMedia, "w");

    fscanf(fin, "%d", &n);
    for (i = 0; i < n; ++i) {
        fscanf(fin, "%s", com);
        if (com[0] == 'A') {
            fscanf(fin, "%c", &c);
            fgets(song, LMAX, fin);
            song[strlen(song) - 1] = '\0';
            if (strcmp(com, "ADD_AFTER") == 0) {
            	if (list != NULL && list->curr != NULL) {
                    /* Verificam daca putem adauga melodia data */
                    if (strcmp(list->curr->data, song) != 0) {
                	    check_song(list, song);
                        add_after(list, song);
                    }
                }
            } else {
                add_interpreter(list, com, song);
            }
        } else if (com[0] == 'D') {
            /* Daca e comanda DEL_SONG -> mai avem parametrul <nume_melodie> */
        	if (strcmp(com, "DEL_SONG") == 0) {
        	    fscanf(fin, "%c", &c);
                fgets(song, LMAX, fin);
                song[strlen(song) - 1] = '\0';
                sz = list->size;
                check_song(list, song);
                /* Se verifica daca s-a gasit cantecul de sters in lista */
                if (sz == list->size) {
                    fprintf(fout, "Error: no song found to delete\n");
                }
        	} else {
                del_interpreter(list, com, fout);
            }
        } else if (com[0] == 'M') {
            move_interpreter(list, com, fout);
        } else if (com[0] == 'S') {
        	if (strcmp(com, "SHOW_PLAYLIST") == 0) {
        	    show_playlist(list, fout);
        	} else {
                show_interpreter(list, com, fout);
            }
        }
    }
    free_list(&list);
    fclose(fin);
    fclose(fout);
    free(list);
    return 0;
}
