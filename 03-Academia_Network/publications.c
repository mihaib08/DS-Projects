/* Copyright [2020] <Mihai-Eugen Barbu + Tudor-Alexandru Niculescu> */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "publications.h"

#define HMAX 50000

typedef struct author author;
struct author {
    char *name;
    int64_t id;
    char *institution;
};

typedef struct article article;
struct article {
    int64_t id;
    char *title;
    int year;
    char *venue;

    int num_fields;
    char **fields;

    int num_authors;
    author *authors;

    int num_refs;
    int64_t *references;

    int vis;
};

typedef struct Node paper;
struct Node {
    article *article;
    paper *next;
};

/* Hash_list --> chaining */
typedef struct List List;
struct List {
    paper *head;
    int size;
};

/* Task 5 */
typedef struct cited_paper_node cited_paper_node;
struct cited_paper_node {
    int64_t id;
    int year;
    char *title;

    /* Count cited */
    int ct;

    /* Task_10 *util* */
    double fact;

    int64_t parent_id;

    cited_paper_node *next;
};

typedef struct Cited_List Cited_List;
struct Cited_List {
    cited_paper_node *head;
    cited_paper_node *tail;
};

typedef struct Field_Node Field_Node;
struct Field_Node {
    char *field;
    int num_authors;
    char **institutions;
    int *ids;

    int64_t paper_id;
    char *paper_title;
    int year;

    Field_Node *next;
};

typedef struct Fields Fields;
struct Fields {
    Field_Node *head;
};

typedef struct id_node id_node;
struct id_node {
    int64_t id;
    id_node *next;
};

typedef struct id_List id_List;
struct id_List {
    id_node *head;
};

typedef struct Ref_Node Ref_Node;
struct Ref_Node {
    int64_t id;
    id_List *ids;
    int ct;

    int vis;

    Ref_Node *next;
};

typedef struct Refs Refs;
struct Refs {
    Ref_Node *head;
};

/* Queue */
typedef struct q_node q_node;
struct q_node{
    int64_t id;
    struct q_node *next;
};

struct LinkedList {
    q_node *head;
    q_node *tail;
};

typedef struct Queue Queue;
struct Queue {
    struct LinkedList *list;
};

/* Venue_Hash */
typedef struct v_node v_node;
struct v_node {
    char *venue;
    int64_t id;
    v_node *next;
};

typedef struct Venues Venues;
struct Venues {
    v_node *head;
};

/* Author_Node */
typedef struct a_node a_node;
struct a_node {
    int64_t id;
    id_List *ids;
    int ct;
    int vis;

    id_List *paper_ids;

    char *name;

    a_node *next;
};

typedef struct Authors Authors;
struct Authors {
    a_node *head;
};


struct publications_data {
    int hmax;
    List *buckets;
    int num_articles;

    Fields *field_buckets;
    Refs *ref_buckets;
    Venues *v_buckets;
    Authors *a_buckets;

    int *years;
};

void init_list(List *list) {
    list->head = NULL;
    list->size = 0;
}

void init_fields(Fields *list) {
    list->head = NULL;
}

void init_refs(Refs *list) {
    list->head = NULL;
}

void init_venues(Venues *list) {
    list->head = NULL;
}

void init_authors(Authors *list) {
    list->head = NULL;
}

unsigned int hash_function_string(void *a) {
    /*
     * Credits: http://www.cse.yorku.ca/~oz/hash.html
     */
    unsigned char *puchar_a = (unsigned char*) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

    return hash;
}

PublData* init_publ_data(void) {
    int i;
    PublData *data;
    data = (PublData *)malloc(sizeof(PublData));
    DIE(data == NULL, "data malloc");

    data->hmax = HMAX;
    data->buckets = (List *)malloc(data->hmax * sizeof(List));
    DIE(data->buckets == NULL, "data malloc");

    data->field_buckets = (Fields *)malloc(data->hmax * sizeof(Fields));
    DIE(data->field_buckets == NULL, "fields malloc");

    data->ref_buckets = (Refs *)malloc(data->hmax * sizeof(Refs));
    DIE(data->ref_buckets == NULL, "refs malloc");

    data->v_buckets = (Venues *)malloc(data->hmax * sizeof(Venues));
    DIE(data->v_buckets == NULL, "venues malloc");

    data->a_buckets = (Authors *)malloc(data->hmax * sizeof(Authors));
    DIE(data->a_buckets == NULL, "authors malloc");

    for (i = 0; i < data->hmax; ++i) {
        init_list(&data->buckets[i]);

        init_fields(&data->field_buckets[i]);

        init_refs(&data->ref_buckets[i]);

        init_venues(&data->v_buckets[i]);

        init_authors(&data->a_buckets[i]);
    }
    data->num_articles = 0;

    data->years = calloc(2025, sizeof(int));

    return data;
}

void free_article(article *article) {
    int i;
    free(article->title);
    free(article->venue);

    /* Free_Authors */
    for (i = 0; i < article->num_authors; ++i) {
        free(article->authors[i].name);
        free(article->authors[i].institution);
    }
    free(article->authors);

    /* Free_fields */
    for (i = 0; i < article->num_fields; ++i) {
        free(article->fields[i]);
    }
    free(article->fields);

    /* Free_refs */
    free(article->references);

    free(article);
}

void free_list(List **pp_list) {
    paper *curr, *aux;
    curr = (*pp_list)->head;
    while (curr != NULL) {
        aux = curr->next;
        free_article(curr->article);
        free(curr);
        curr = aux;
    }
}

void free_fields(Fields **pp_list) {
    int i;
    Field_Node *curr, *aux;
    curr = (*pp_list)->head;

    while (curr != NULL) {
        aux = curr->next;

        free(curr->field);
        for (i = 0; i < curr->num_authors; ++i) {
            free(curr->institutions[i]);
        }
        free(curr->institutions);
        free(curr->ids);

        free(curr->paper_title);
        free(curr);
        curr = aux;
    }
}

void free_ids(id_List **pp_list) {
    id_node *curr, *aux;
    curr = (*pp_list)->head;

    while (curr != NULL) {
        aux = curr->next;
        free(curr);
        curr = aux;
    }
}

void free_refs(Refs **pp_list) {
    Ref_Node *curr, *aux;
    curr = (*pp_list)->head;

    while (curr != NULL) {
        aux = curr->next;
        free_ids(&curr->ids);
        free(curr->ids);
        free(curr);
        curr = aux;
    }
}

void free_venues(Venues **pp_list) {
    v_node *curr, *aux;
    curr = (*pp_list)->head;

    while (curr != NULL) {
        aux = curr->next;
        free(curr->venue);
        free(curr);
        curr = aux;
    }
}

void free_authors(Authors **pp_list) {
    a_node *curr, *aux;
    curr = (*pp_list)->head;

    while (curr != NULL) {
        aux = curr->next;
        free_ids(&curr->ids);
        free_ids(&curr->paper_ids);
        free(curr->name);
        free(curr->paper_ids);
        free(curr->ids);
        free(curr);
        curr = aux;
    }
}

void destroy_publ_data(PublData* data) {
    int i;
    List *bucket;
    Fields *f_bucket;
    Refs *r_bucket;
    Venues *v_bucket;
    Authors *a_bucket;

    for (i = 0; i < data->hmax; ++i) {
        bucket = &data->buckets[i];
        free_list(&bucket);

        f_bucket = &data->field_buckets[i];
        free_fields(&f_bucket);

        r_bucket = &data->ref_buckets[i];
        free_refs(&r_bucket);

        v_bucket = &data->v_buckets[i];
        free_venues(&v_bucket);

        a_bucket = &data->a_buckets[i];
        free_authors(&a_bucket);
    }
    free(data->buckets);
    free(data->field_buckets);
    free(data->ref_buckets);
    free(data->v_buckets);
    free(data->a_buckets);
    free(data->years);
    free(data);
}

void add_paper(PublData* data, const char* title, const char* venue,
    const int year, const char** author_names, const int64_t* author_ids,
    const char** institutions, const int num_authors, const char** fields,
    const int num_fields, const int64_t id, const int64_t* references,
    const int num_refs) {

    int i, j, ind, len;
    List *bucket;

    paper *new;
    new = (paper *)malloc(sizeof(paper));
    DIE(new == NULL, "paper malloc");

    article *info;
    info = (article *)malloc(sizeof(article));
    DIE(info == NULL, "article malloc");

    info->id = id;
    len = strlen(title);
    info->title = malloc((len + 1) * sizeof(char));
    snprintf(info->title, len + 1, "%s\n", title);

    len = strlen(venue);
    info->venue = (char *)malloc((len + 1) * sizeof(char));

    snprintf(info->venue, len + 1, "%s\n", venue);

    info->year = year;

    info->num_authors = num_authors;
    info->authors = (author *)malloc(num_authors * sizeof(author));

    DIE(info->authors == NULL, "authors malloc");

    /* Author_Hash */
    int ok;
    int64_t a_ind;
    a_node *curr, *a_new;
    Authors *a_bucket;

    id_node *id_new;

    for (i = 0; i < info->num_authors; ++i) {
        len = strlen(author_names[i]);
        info->authors[i].name = (char *)malloc((len + 1) * sizeof(char));
        snprintf(info->authors[i].name, len + 1, "%s\n", author_names[i]);

        info->authors[i].id = author_ids[i];

        len = strlen(institutions[i]);
        info->authors[i].institution = (char *)malloc((len + 1) * sizeof(char));
        snprintf(info->authors[i].institution, len + 1,
                "%s\n", institutions[i]);

        a_ind = author_ids[i] % data->hmax;
        a_bucket = &data->a_buckets[a_ind];
        curr = a_bucket->head;

        while (curr != NULL && curr->id != author_ids[i]) {
            curr = curr->next;
        }

        if (curr == NULL) {
            a_new = (a_node *)malloc(sizeof(a_node));
            a_new->id = author_ids[i];

            a_new->vis = 0;
            a_new->ct = 0;

            a_new->next = a_bucket->head;
            a_bucket->head = a_new;

            a_new->paper_ids = (id_List *)malloc(sizeof(id_List));
            a_new->paper_ids->head = NULL;

            id_node *paper_id;
            paper_id = (id_node *)malloc(sizeof(id_node));
            paper_id->id = id;
            paper_id->next = a_new->paper_ids->head;
            a_new->paper_ids->head = paper_id;

            a_new->ids = (id_List *)malloc(sizeof(id_List));
            a_new->ids->head = NULL;

            len = strlen(author_names[i]);
            a_new->name = (char *)malloc((len + 1) * sizeof(char));
            snprintf(a_new->name, len + 1, "%s\n", author_names[i]);
        } else {
            id_node *paper_id;
            paper_id = (id_node *)malloc(sizeof(id_node));
            paper_id->id = id;
            paper_id->next = curr->paper_ids->head;
            curr->paper_ids->head = paper_id;
        }

        for (j = 0; j < i; ++j) {
            /* j --> i */
            curr = a_bucket->head;
            ok = 0;
            while (curr != NULL && ok == 0) {
                if (curr->id == author_ids[i]) {
                    /* Add id[j] --> id[i] */
                    id_new = (id_node *)malloc(sizeof(id_node));
                    id_new->id = author_ids[j];

                    id_new->next = curr->ids->head;
                    curr->ids->head = id_new;

                    curr->ct++;
                    ok = 1;
                }
                curr = curr->next;
            }
        }

        for (j = i + 1; j < num_authors; ++j) {
            /* j --> i */
            curr = a_bucket->head;
            ok = 0;
            while (curr != NULL && ok == 0) {
                if (curr->id == author_ids[i]) {
                    /* Add id[j] --> id[i] */
                    id_new = (id_node *)malloc(sizeof(id_node));
                    id_new->id = author_ids[j];

                    id_new->next = curr->ids->head;
                    curr->ids->head = id_new;

                    curr->ct++;
                    ok = 1;
                }
                curr = curr->next;
            }
        }
    }

    info->num_fields = num_fields;
    info->fields = (char **)malloc(num_fields * sizeof(char *));
    DIE(info->fields == NULL, "fields malloc");
    for (i = 0; i < info->num_fields; ++i) {
        len = strlen(fields[i]);
        info->fields[i] = (char *)malloc((len + 1) * sizeof(char));
        snprintf(info->fields[i], len + 1, "%s\n", fields[i]);

        int f_ind;
        Fields *f_bucket;
        Field_Node *f_new;

        char *f_field;
        f_field = (char *)malloc((len + 1) * sizeof(char));
        snprintf(f_field, len + 1, "%s\n", fields[i]);
        f_ind = hash_function_string(f_field) % data->hmax;
        free(f_field);

        f_bucket = &data->field_buckets[f_ind];
        f_new = (Field_Node *)malloc(sizeof(Field_Node));

        f_new->field = (char *)malloc((len + 1) * sizeof(char));
        snprintf(f_new->field, len + 1, "%s\n", fields[i]);

        f_new->num_authors = num_authors;
        f_new->institutions = (char **)malloc(num_authors * sizeof(char *));
        f_new->ids = (int *)malloc(num_authors * sizeof(int));
        for (j = 0; j < num_authors; ++j) {
            len = strlen(institutions[j]);
            f_new->institutions[j] = (char *)malloc((len + 1) * sizeof(char));

            snprintf(f_new->institutions[j], len + 1, "%s\n", institutions[j]);
            f_new->ids[j] = author_ids[j];
        }
        f_new->next = f_bucket->head;
        f_bucket->head = f_new;

        f_new->paper_id = id;

        len = strlen(title);
        f_new->paper_title = (char *)malloc((len + 1) * sizeof(char));
        snprintf(f_new->paper_title, len + 1, "%s\n", title);

        f_new->year = year;
    }

    info->num_refs = num_refs;
    info->references = (int64_t *)malloc(num_refs * sizeof(int64_t));
    DIE(info->references == NULL, "refs malloc");
    for (i = 0; i < info->num_refs; ++i) {
        info->references[i] = references[i];

        int r_ind, ok;
        Refs *r_bucket;
        Ref_Node *r_new, *curr;

        r_ind = references[i] % data->hmax;
        r_bucket = &data->ref_buckets[r_ind];

        ok = 0;
        curr = r_bucket->head;
        while (curr != NULL && ok == 0) {
            if (curr->id == references[i]) {
                curr->ct++;

                /* Add paper id --> id_node */
                id_node *id_new;

                id_new = (id_node *)malloc(sizeof(id_node));
                id_new->id = id;
                id_new->next = curr->ids->head;
                curr->ids->head = id_new;

                ok = 1;
            }
            curr = curr->next;
        }

        if (ok == 0) {
            r_new = (Ref_Node *)malloc(sizeof(Ref_Node));
            r_new->id = references[i];
            r_new->ct = 1;
            r_new->vis = 0;
            r_new->next = r_bucket->head;
            r_bucket->head = r_new;

            /* Add paper id --> id_node */
            id_node *id_new;

            id_new = (id_node *)malloc(sizeof(id_node));
            id_new->id = id;

            /* Alloc id_List */
            r_new->ids = (id_List *)malloc(sizeof(id_List));
            r_new->ids->head = NULL;

            id_new->next = r_new->ids->head;
            r_new->ids->head = id_new;
        }
    }

    v_node *v_new;
    Venues *v_bucket;
    int v_ind;
    char *ven;

    len = strlen(venue);

    ven = (char *)malloc((len + 1) * sizeof(char));
    snprintf(ven, len + 1, "%s\n", venue);
    v_ind = hash_function_string(ven) % data->hmax;

    v_bucket = &data->v_buckets[v_ind];
    v_new = (v_node *)malloc(sizeof(v_node));

    v_new->venue = (char *)malloc((len + 1) * sizeof(char));
    snprintf(v_new->venue, len + 1, "%s\n", venue);
    v_new->id = id;

    v_new->next = v_bucket->head;
    v_bucket->head = v_new;

    free(ven);

    info->vis = 0;

    new->article = info;

    /* Hash Function ?? */
    ind = id % data->hmax;
    bucket = &data->buckets[ind];

    new->next = bucket->head;
    bucket->head = new;
    bucket->size++;

    data->years[year]++;

    data->num_articles++;
}

/* Queue op */

int64_t front(Queue *q) {
    q_node *curr;
    curr = q->list->head;
    return curr->id;
}

void dequeue(Queue *q) {
    q_node *curr;
    curr = q->list->head;
    q->list->head = q->list->head->next;
    if (q->list->head == NULL) {
        q->list->tail = NULL;
    }
    free(curr);
}

void enqueue(Queue *q, int64_t new_data) {
    q_node *new;
    new = (q_node *)malloc(sizeof(q_node));

    new->id = new_data;
    if (q->list->head == NULL) {
        new->next = q->list->head;
        q->list->head = new;
        q->list->tail = new;
    } else {
        new->next = NULL;
        q->list->tail->next = new;
        q->list->tail = new;
    }
}

char* get_oldest_influence(PublData* data, const int64_t id_paper) {
    int ind, i;
    int64_t r_id;

    static int k = 0;
    int min_year = 3000;
    int ref_max;
    int64_t id_min = -1;
    List *bucket;
    Refs *r_bucket;
    Ref_Node *r_node;
    paper *node, *sol;
    sol = NULL;

    ind = id_paper % data->hmax;
    bucket = &data->buckets[ind];
    node = bucket->head;

    k++;

    while (node != NULL && node->article->id != id_paper) {
        node = node->next;
    }

    if (node == NULL || node->article->num_refs == 0) {
        return "None";
    } else {
        node->article->vis = k;

        /* BFS */

        /* 
         * Add refs to Queue
         */
        Queue *q;

        /* Init_Queue */
        q = (Queue *)malloc(sizeof(Queue));
        q->list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
        q->list->head = NULL;
        q->list->tail = NULL;

        for (i = 0; i < node->article->num_refs; ++i) {
            enqueue(q, node->article->references[i]);
        }
        while (q->list->head != NULL) {
            r_id = front(q);

            dequeue(q);
            ind = r_id % data->hmax;
            bucket = &data->buckets[ind];

            node = bucket->head;
            while (node != NULL && node->article->id != r_id) {
                node = node->next;
            }
            if (node != NULL && node->article->vis != k) {
                node->article->vis++;
                for (i = 0; i < node->article->num_refs; ++i) {
                    enqueue(q, node->article->references[i]);
                }
                if (node->article->year < min_year) {
                    min_year = node->article->year;
                    sol = node;
                    id_min = node->article->id;
                } else if (node->article->year == min_year) {
                    int curr_ref;
                    int64_t curr_id;

                    /* Check ref_max */
                    r_bucket = &data->ref_buckets[ind];
                    r_node = r_bucket->head;
                    while (r_node != NULL && r_node->id != r_id) {
                        r_node = r_node->next;
                    }

                    if (r_node != NULL) {
                        curr_ref = r_node->ct;
                        curr_id = r_node->id;
                    }

                    /* Id_min refs */
                    int ind_min = id_min % data->hmax;
                    r_bucket = &data->ref_buckets[ind_min];
                    r_node = r_bucket->head;
                    while (r_node != NULL && r_node->id != id_min) {
                        r_node = r_node->next;
                    }

                    if (r_node != NULL) {
                        ref_max = r_node->ct;
                    }

                    if (ref_max < curr_ref) {
                        id_min = curr_id;
                        sol = node;
                    } else if (ref_max == curr_ref) {
                        if (curr_id < id_min) {
                            id_min = curr_id;
                            sol = node;
                        }
                    }
                }
            }
        }
        free(q->list);
        free(q);
    }
    if (id_min == -1) {
        return "None";
    }
    return sol->article->title;
}

float get_venue_impact_factor(PublData* data, const char* venue) {
    int v_ind, len;

    int ct, nr;

    char *ven;
    v_node *v_node;
    Venues *v_bucket;

    len = strlen(venue);
    ven = (char *)malloc((len + 1) * sizeof(char));
    snprintf(ven, len + 1, "%s\n", venue);

    v_ind = hash_function_string(ven) % data->hmax;
    v_bucket = &data->v_buckets[v_ind];

    v_node = v_bucket->head;

    ct = nr = 0;

    /* Search venue */
    while (v_node != NULL) {
        if (strcmp(v_node->venue, ven) == 0) {
            nr++;
            /* Refs */
            int r_ind;
            Ref_Node *r_node;
            Refs *r_bucket;

            r_ind = v_node->id % data->hmax;
            r_bucket = &data->ref_buckets[r_ind];
            r_node = r_bucket->head;

            while (r_node != NULL && r_node->id != v_node->id) {
                r_node = r_node->next;
            }

            if (r_node != NULL) {
                ct += r_node->ct;
            }
        }

        v_node = v_node->next;
    }

    double res;
    res = (double)ct / nr;

    free(ven);

    return res;
}

int get_number_of_influenced_papers(PublData* data, const int64_t id_paper,
    const int distance) {
    static int k = 0;

    int r_ind;
    Ref_Node *r_node;
    Refs *r_bucket;

    k++;

    r_ind = id_paper % data->hmax;
    r_bucket = &data->ref_buckets[r_ind];

    r_node = r_bucket->head;
    while (r_node != NULL && r_node->id != id_paper) {
        r_node = r_node->next;
    }

    if (r_node == NULL) {
        return 0;
    } else {
        int ok;
        int64_t i_id;
        Queue *q;

       /* Init_Queue */
        q = (Queue *)malloc(sizeof(Queue));
        q->list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
        q->list->head = NULL;
        q->list->tail = NULL;

        /* niv --> distance */
        int niv = 1;
        int nr, nr_next, l;
        int ct = 0;

        id_node *i_node;
        id_List *id_bucket;

        id_bucket = r_node->ids;
        i_node  = id_bucket->head;

        r_node->vis = k;

        nr = r_node->ct;
        l = 0;
        nr_next = 0;

        while (i_node != NULL) {
            enqueue(q, i_node->id);
            i_node = i_node->next;
        }

        ok = 0;
        while (q->list->head != NULL && ok == 0) {
            i_id = front(q);

            dequeue(q);

            l++;

            r_ind = i_id % data->hmax;
            r_bucket = &data->ref_buckets[r_ind];
            r_node = r_bucket->head;

            while (r_node != NULL && r_node->id != i_id) {
                r_node = r_node->next;
            }

            if (r_node == NULL) {
                paper *node;
                List *bucket;

                bucket = &data->buckets[r_ind];
                node = bucket->head;
                while (node != NULL && node->article->id != i_id) {
                    node = node->next;
                }

                if (node->article->vis != k) {
                    ct++;
                    node->article->vis = k;
                }
            }

            if (r_node != NULL && r_node->vis != k) {
                r_node->vis = k;
                ct++;

                nr_next += r_node->ct;

                id_bucket = r_node->ids;
                i_node = id_bucket->head;

                while (i_node != NULL) {
                    enqueue(q, i_node->id);
                    i_node = i_node->next;
                }
            }
            if (l == nr) {
                if (niv == distance) {
                    ok = 1;
                    while (q->list->head != NULL) {
                        dequeue(q);
                    }
                } else {
                    nr = nr_next;
                    l = 0;
                    nr_next = 0;
                    niv++;
                }
            }
        }
        free(q->list);
        free(q);
        return ct;
    }
}

void free_q(Queue *q) {
    while (q->list->head != NULL) {
        dequeue(q);
    }
}

int get_erdos_distance(PublData* data, const int64_t id1, const int64_t id2) {
    static int k = 0;

    k++;

    int ind;
    Authors *a_bucket;
    a_node *curr;

    ind = id1 % data->hmax;
    a_bucket = &data->a_buckets[ind];

    curr = a_bucket->head;
    while (curr != NULL && curr->id != id1) {
        curr = curr->next;
    }

    if (curr == NULL) {
        return -1;
    }

    /* Erdos_Distance */
    int niv = 1;

    int nr, nr_next, l;

    /* BFS - id1 --> id2 */

    Queue *q;
    q = (Queue *)malloc(sizeof(Queue));
    q->list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    q->list->head = NULL;
    q->list->tail = NULL;

    id_node *i_node;
    id_List *id_bucket;

    id_bucket = curr->ids;
    i_node = id_bucket->head;

    while (i_node != NULL) {
        enqueue(q, i_node->id);

        if (i_node->id == id2) {
            free_q(q);
            free(q->list);
            free(q);

            return 1;
        }

        i_node = i_node->next;
    }

    curr->vis = k;

    nr = curr->ct;
    nr_next = 0;
    l = 0;

    int64_t i_id;
    int a_ind;

    while (q->list->head != NULL) {
        i_id = front(q);

        dequeue(q);
        l++;

        a_ind = i_id % data->hmax;

        a_bucket = &data->a_buckets[a_ind];
        curr = a_bucket->head;

        while (curr != NULL && curr->id != i_id) {
            curr = curr->next;
        }

        if (curr != NULL && curr->vis != k) {
            curr->vis = k;

            nr_next += curr->ct;

            id_bucket = curr->ids;
            i_node = id_bucket->head;

            while (i_node != NULL) {
               if (i_node->id == id2) {
                   free_q(q);
                   free(q->list);
                   free(q);

                   return niv + 1;
               }

                enqueue(q, i_node->id);
                i_node = i_node->next;
            }
        }

        if (l == nr) {
            niv++;
            nr = nr_next;
            nr_next = 0;
            l = 0;
        }
    }
    free_q(q);
    free(q->list);
    free(q);

    return -1;
}

void free_cited(Cited_List **pp_list) {
    cited_paper_node *curr, *aux;

    if (pp_list == NULL || *pp_list == NULL) {
        return;
    }

    curr = (*pp_list)->head;
    while (curr != NULL) {
        aux = curr->next;
        free(curr->title);
        free(curr);
        curr = aux;
    }
}

char** get_most_cited_papers_by_field(PublData* data, const char* field,
    int* num_papers) {

    int nr;
    int num = *(int *)num_papers;

    int ok;

    Cited_List *list;
    cited_paper_node *new, *curr, *prev;

    list = (Cited_List *)malloc(sizeof(Cited_List));
    list->head = NULL;

    int f_ind, len;
    char *f_field;
    Fields *f_bucket;
    Field_Node *f_node;

    len = strlen(field);

    f_field = (char *)malloc((len + 1) * sizeof(char));
    snprintf(f_field, len + 1, "%s\n", field);
    f_ind = hash_function_string(f_field) % data->hmax;
    free(f_field);

    f_bucket = &data->field_buckets[f_ind];
    f_node = f_bucket->head;

    int r_ind;
    Refs *r_bucket;
    Ref_Node *r_node;

    nr = 0;

    while (f_node != NULL) {
        if (strcmp(f_node->field, field) == 0) {
            new = (cited_paper_node *)malloc(sizeof(cited_paper_node));
            len = strlen(f_node->paper_title);
            new->title = (char *)malloc((len + 1) * sizeof(char));
            snprintf(new->title, len + 1, "%s\n", f_node->paper_title);
            new->id = f_node->paper_id;
            new->year = f_node->year;

            nr++;

            r_ind = f_node->paper_id % data->hmax;
            r_bucket = &data->ref_buckets[r_ind];

            r_node = r_bucket->head;
            while (r_node != NULL && r_node->id != f_node->paper_id) {
                r_node = r_node->next;
            }

            if (r_node != NULL) {
                new->ct = r_node->ct;
            } else {
                new->ct = 0;
            }

            if (list->head == NULL) {
                new->next = list->head;
                list->head = new;
            } else {
                curr = list->head;
                prev = curr;

                ok = 0;

                if (curr->ct < new->ct) {
                    new->next = list->head;
                    list->head = new;

                    ok = 1;
                } else if (curr->ct == new->ct) {
                    if (curr->year < new->year) {
                        new->next = list->head;
                        list->head = new;

                        ok = 1;
                    } else if (curr->year == new->year && new->id < curr->id) {
                        new->next = list->head;
                        list->head = new;

                        ok = 1;
                    }
                }

                while (curr != NULL && ok == 0) {
                    if (curr->ct == new->ct) {
                        if (curr->year < new->year) {
                            new->next = curr;
                            prev->next = new;

                            ok = 1;
                        } else if (curr->year == new->year) {
                            if (curr->id > new->id) {
                                new->next = curr;
                                prev->next = new;

                                ok = 1;
                            }
                        }
                    } else if (curr->ct < new->ct) {
                        new->next = curr;
                        prev->next = new;

                        ok = 1;
                    }

                    prev = curr;
                    curr = curr->next;
                }

                if (ok == 0) {
                    new->next = NULL;
                    prev->next = new;
                }
            }
        }
        f_node = f_node->next;
    }

    if (nr < num) {
        *num_papers = nr;
    }

    num = *(int *)num_papers;
    char **sol;
    sol = (char **)malloc(num * sizeof(char *));
    int i;

    curr = list->head;

    for (i = 0; i < num; ++i) {
        len = strlen(curr->title);
        sol[i] = (char *)malloc((len + 1) * sizeof(char));

        snprintf(sol[i], len + 1, "%s\n", curr->title);

        curr = curr->next;
    }

    free_cited(&list);
    free(list);

    return sol;
}

int get_number_of_papers_between_dates(PublData* data, const int early_date,
    const int late_date) {
    int i, ct;
    ct = 0;

    for (i = early_date; i <= late_date; ++i) {
        ct += data->years[i];
    }

    return ct;
}

int get_number_of_authors_with_field(PublData* data, const char* institution,
    const char* field) {
    int len, ind, i, j;
    char *f_field;
    Fields *bucket;
    Field_Node *curr;

    int nr_auth = 0, ok;
    int *ids;

    len = strlen(field);

    f_field = (char *)malloc((len + 1) * sizeof(char));
    DIE(f_field == NULL, "field malloc");

    snprintf(f_field, len + 1, "%s\n", field);
    ind = hash_function_string(f_field) % data->hmax;
    free(f_field);

    bucket = &data->field_buckets[ind];
    curr = bucket->head;
    while (curr != NULL) {
        if (strcmp(curr->field, field) == 0) {
            for (i = 0; i < curr->num_authors; ++i) {
                if (strcmp(curr->institutions[i], institution) == 0) {
                    if (nr_auth == 0) {
                        ids = calloc(1, sizeof(int));
                        ids[nr_auth++] = curr->ids[i];
                    } else {
                        ok = 0;
                        for (j = 0; j < nr_auth && ok == 0; ++j) {
                            if (ids[j] == curr->ids[i]) {
                                ok = 1;
                            }
                        }
                        if (ok == 0) {
                            ids = realloc(ids, (nr_auth + 1) * sizeof(int));
                            ids[nr_auth++] = curr->ids[i];
                        }
                    }
                }
            }
        }
        curr = curr->next;
    }
    if (nr_auth) {
        free(ids);
    }
    return nr_auth;
}

int* get_histogram_of_citations(PublData* data, const int64_t id_author,
    int* num_years) {

    *num_years = 0;

    int v[2021] = {0};

    int min_year = 2021;

    Authors *a_bucket;
    a_node *a_node;
    int a_ind;

    id_node *p_id;
    int ind;

    Refs *r_bucket;
    Ref_Node *r_node;

    List *bucket;
    paper *node;

    a_ind = id_author % data->hmax;
    a_bucket = &data->a_buckets[a_ind];
    a_node = a_bucket->head;

    int ok;
    ok = 0;
    while (a_node != NULL && ok == 0) {
        if (a_node->id == id_author) {
            p_id = a_node->paper_ids->head;

            while (p_id != NULL) {
                ind = p_id->id % data->hmax;
                bucket = &data->buckets[ind];
                node = bucket->head;

                while (node != NULL && node->article->id != p_id->id) {
                    node = node->next;
                }

                if (node->article->year < min_year) {
                    min_year = node->article->year;
                }


                r_bucket = &data->ref_buckets[ind];
                r_node = r_bucket->head;
                while (r_node != NULL && r_node->id != p_id->id) {
                    r_node = r_node->next;
                }

                if (r_node != NULL) {
                    v[2020 - node->article->year] += r_node->ct;
                }

                p_id = p_id->next;
            }
            ok = 1;
        }
        a_node = a_node->next;
    }

    *num_years = 2020 - min_year + 1;

    int *yr, i;

    yr = (int *)malloc((*num_years) * sizeof(int));
    for (i = 0; i < (*num_years); ++i) {
        yr[i] = v[i];
    }
    return yr;
}

/* Cited Queue */

void dequeue_cited(Cited_List *q) {
    cited_paper_node *curr;
    curr = q->head;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(curr);
}

void enqueue_cited(Cited_List *q, cited_paper_node *new) {
    if (q->head == NULL) {
        new->next = q->head;
        q->head = new;
        q->tail = new;
    } else {
        new->next = NULL;
        q->tail->next = new;
        q->tail = new;
    }
}

char** get_reading_order(PublData* data, const int64_t id_paper,
    const int distance, int* num_papers) {

    *num_papers = 0;

    static int k = 0;

    k++;

    int len, i, ok;

    int nr;

    int ind;
    List *bucket;
    paper *node;

    Cited_List *list;
    cited_paper_node *c_node, *new, *prev;

    Cited_List *q;
    cited_paper_node *curr, *aux, *prev_aux;

    ind = id_paper % data->hmax;
    bucket = &data->buckets[ind];

    node = bucket->head;

    while (node != NULL && node->article->id != id_paper) {
        node = node->next;
    }

    if (node == NULL) {
        return NULL;
    } else {
        nr = 0;
        node->article->vis = k;

        list = (Cited_List *)malloc(sizeof(Cited_List));
        list->head = NULL;

        q = (Cited_List *)malloc(sizeof(Cited_List));
        q->head = NULL;
        q->tail = NULL;

        for (i = 0; i < node->article->num_refs; ++i) {
            new = (cited_paper_node *)malloc(sizeof(cited_paper_node));
            new->id = node->article->references[i];
            new->ct = 1;
            new->parent_id = node->article->id;

            enqueue_cited(q, new);
        }

        while (q->head != NULL && q->head->ct <= distance) {
            curr = q->head;

            ind = curr->id % data->hmax;
            bucket = &data->buckets[ind];
            node = bucket->head;

            while (node != NULL && node->article->id != curr->id) {
                node = node->next;
            }

            if (node != NULL) {
                if (node->article->vis == k) {
                    aux = list->head;

                    if (aux->id == node->article->id) {
                        list->head = list->head->next;

                        free(aux->title);
                        free(aux);
                    } else {
                        prev_aux = aux;
                        while (aux->id != node->article->id) {
                            prev_aux = aux;
                            aux = aux->next;
                        }

                        prev_aux->next = aux->next;
                        free(aux->title);
                        free(aux);
                    }
                } else {
                    nr++;
                    node->article->vis = k;
                }

                new = (cited_paper_node *)malloc(sizeof(cited_paper_node));
                new->id = node->article->id;
                new->ct = curr->ct;
                new->year = node->article->year;
                new->parent_id = curr->parent_id;

                len = strlen(node->article->title);
                new->title = (char *)malloc((len + 1) * sizeof(char));

                snprintf(new->title, len + 1, "%s\n", node->article->title);

                c_node = list->head;
                prev = c_node;

                ok = 0;

                if (c_node == NULL) {
                    new->next = list->head;
                    list->head = new;
                    ok = 1;
                } else {
                   if (c_node->id == new->parent_id) {
                       new->next = list->head;
                       list->head = new;

                       if (c_node->next != NULL) {
                           if (c_node->next->year == new->year) {
                               if (c_node->next->id < new->id) {
                                   cited_paper_node *c_aux;

                                   c_aux = c_node->next->next;
                                   c_node->next->next = list->head;
                                   list->head = c_node->next;

                                   c_node->next = c_aux;
                               }
                           }
                       }

                       ok = 1;
                    } else {
                        if (new->year < c_node->year) {
                            new->next = list->head;
                            list->head = new;

                            ok = 1;
                        } else if (new->year == c_node->year) {
                            if (new->id < c_node->id) {
                                new->next = list->head;
                                list->head = new;

                                ok = 1;
                            }
                        }
                    }
                }

                while (c_node != NULL && ok == 0) {
                    if (c_node->id == new->parent_id) {
                        new->next = c_node;
                        prev->next = new;

                        if (c_node->next != NULL) {
                            if (c_node->next->year == new->year) {
                                if (c_node->next->id < new->id) {
                                    prev->next = c_node->next;

                                    cited_paper_node *c_aux;
                                    c_aux = c_node->next->next;
                                    c_node->next->next = new;
                                    c_node->next = c_aux;
                                }
                            }
                        }

                        ok = 1;
                    }


                    if (c_node->year > new->year && ok == 0) {
                        new->next = c_node;
                        prev->next = new;

                        ok = 1;
                    } else if (c_node->year == new->year && ok == 0) {
                        if (c_node->id > new->id) {
                            new->next = c_node;
                            prev->next = new;

                            ok = 1;
                        }
                    }
                    prev = c_node;
                    c_node = c_node->next;
                }

                if (ok == 0) {
                    new->next = c_node;
                    prev->next = new;
                }

                for (i = 0; i < node->article->num_refs; ++i) {
                    new = (cited_paper_node *)malloc(sizeof(cited_paper_node));
                    new->id = node->article->references[i];
                    new->ct = curr->ct + 1;
                    new->parent_id = node->article->id;

                    enqueue_cited(q, new);
                }
            }
            dequeue_cited(q);
        }
    }

    if (nr == 0) {
        free_cited(&list);
        free(list);

        while (q->head != NULL) {
            dequeue_cited(q);
        }
        free(q);

        return NULL;
    }

    *num_papers = nr;
    char **sol;
    sol = (char **)malloc(nr * sizeof(char *));

    curr = list->head;
    for (i = 0; i < nr; ++i) {
        len = strlen(curr->title);
        sol[i] = (char *)malloc((len + 1) * sizeof(char));

        snprintf(sol[i], len + 1, "%s\n", curr->title);

        curr = curr->next;
    }

    free_cited(&list);
    free(list);

    while (q->head != NULL) {
        dequeue_cited(q);
    }
    free(q);

    return sol;
}

char* find_best_coordinator(PublData* data, const int64_t id_author) {
    static int k = 0;

    k++;

    int ind;
    Authors *a_bucket;
    a_node *curr, *sol;

    /* Venues_List */
    Cited_List *venue_list;
    cited_paper_node *v_node, *new;

    ind = id_author % data->hmax;
    a_bucket = &data->a_buckets[ind];

    curr = a_bucket->head;

    while (curr != NULL && curr->id != id_author) {
        curr = curr->next;
    }

    if (curr == NULL) {
        return "None";
    }

    id_List *id_bucket;
    id_node *i_node, *curr_paper;

    id_bucket = curr->ids;
    i_node = id_bucket->head;

    if (i_node == NULL) {
        return "None";
    }

    int niv = 1;
    int nr, nr_next, l;

    Queue *q;
    q = (Queue *)malloc(sizeof(Queue));
    q->list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    q->list->head = NULL;
    q->list->tail = NULL;

    while (i_node != NULL) {
        enqueue(q, i_node->id);
        i_node = i_node->next;
    }

    curr->vis = k;

    int64_t i_id;
    int a_ind;

    venue_list = (Cited_List *)malloc(sizeof(Cited_List));
    venue_list->head = NULL;

    double res;
    double score, max_score, erd, max_erd;
    int64_t min_id;

    List *bucket;
    paper *node;

    Refs *r_bucket;
    Ref_Node *r_node;

    max_score = -1;

    int n_cit;
    int len;
    double ven_fact;

    nr = curr->ct;
    nr_next = 0;
    l = 0;

    while (q->list->head != NULL && niv <= 5) {
        i_id = front(q);

        dequeue(q);
        l++;
        a_ind = i_id % data->hmax;
        a_bucket = &data->a_buckets[a_ind];
        curr = a_bucket->head;

        while (curr != NULL && curr->id != i_id) {
            curr = curr->next;
        }

        if (curr->vis != k) {
            curr_paper = curr->paper_ids->head;
            score = erd = (double)exp(-niv);
            res = 0;

            while (curr_paper != NULL) {
                ind = curr_paper->id % data->hmax;
                bucket = &data->buckets[ind];

                node = bucket->head;
                while (node != NULL && node->article->id != curr_paper->id) {
                    node = node->next;
                }

                r_bucket = &data->ref_buckets[ind];
                r_node = r_bucket->head;

                while (r_node != NULL && r_node->id != curr_paper->id) {
                    r_node = r_node->next;
                }

                if (r_node == NULL) {
                    n_cit = 0;
                } else {
                    n_cit = r_node->ct;
                }

                v_node = venue_list->head;

                if (v_node == NULL) {
                    new = (cited_paper_node *)malloc(sizeof(cited_paper_node));

                    len = strlen(node->article->venue);
                    new->title = (char *)malloc((len + 1) * sizeof(char));
                    snprintf(new->title, len + 1, "%s\n", node->article->venue);

                    ven_fact = (double) get_venue_impact_factor(data,
                                        node->article->venue);

                    new->fact = (double)ven_fact;

                    new->next = venue_list->head;
                    venue_list->head = new;
                } else {
                    while (v_node != NULL
                           && strcmp(v_node->title,
                                    node->article->venue) != 0) {
                        v_node = v_node->next;
                    }

                    if (v_node == NULL) {
                        new = malloc(sizeof(cited_paper_node));
                        len = strlen(node->article->venue);
                        new->title = (char *)malloc((len + 1) * sizeof(char));
                        snprintf(new->title, len + 1,
                                "%s\n", node->article->venue);

                        ven_fact = (double) get_venue_impact_factor(data,
                                            node->article->venue);

                        new->fact = (double)ven_fact;

                        new->next = venue_list->head;
                        venue_list->head = new;
                    } else {
                        ven_fact = (double)v_node->fact;
                    }
                }

                res = res + (double)(ven_fact * n_cit);

                curr_paper = curr_paper->next;
            }

            score = score * (double)res;
            if (score > max_score) {
                max_score = score;
                min_id = curr->id;
                max_erd = erd;

                sol = curr;
            } else if (score == max_score) {
                if (erd > max_erd) {
                    min_id = curr->id;
                    max_erd = erd;

                    sol = curr;
                } else if (erd == max_erd) {
                    if (curr->id < min_id) {
                        min_id = curr->id;

                        sol = curr;
                    }
                }
            }

            curr->vis = k;

            nr_next += curr->ct;

            id_bucket = curr->ids;
            i_node = id_bucket->head;

            while (i_node != NULL) {
                enqueue(q, i_node->id);
                i_node = i_node->next;
            }
        }

        if (l == nr) {
            niv++;
            nr = nr_next;
            nr_next = 0;
            l = 0;
        }
    }

    free_q(q);
    free(q->list);
    free(q);

    free_cited(&venue_list);
    free(venue_list);

    return sol->name;
}
