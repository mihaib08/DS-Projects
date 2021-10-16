#ifndef NETWORK__COMPS_H_
#define NETWORK__COMPS_H_

#include <stdint.h>

#define HMAX 50000

/** 
 * Strucure definitions for
 * simulating the network
 */

typedef struct author author;
struct author;

typedef struct article article;
struct article;

typedef struct Node paper;
struct Node;

/** (Hash) List 
 *    --> direct chaining
 */
typedef struct List List;
struct List;

/* Task 5 */
typedef struct cited_paper_node cited_paper_node;
struct cited_paper_node;

typedef struct Cited_List Cited_List;
struct Cited_List;

typedef struct Field_Node Field_Node;
struct Field_Node;

typedef struct Fields Fields;
struct Fields;

typedef struct id_node id_node;
struct id_node;

typedef struct id_List id_List;
struct id_List;

typedef struct Ref_Node Ref_Node;
struct Ref_Node;

/**
 * References List
 */
typedef struct Refs Refs;
struct Refs;

/* Queue */
typedef struct q_node q_node;
struct q_node;

struct LinkedList;

typedef struct Queue Queue;
struct Queue;

/**
 * Venue Hashtable
 */
typedef struct v_node v_node;
struct v_node;

typedef struct Venues Venues;
struct Venues;

/** 
 * Author Node
 */
typedef struct a_node a_node;
struct a_node;

/**
 * List of authors (a_node)
 */
typedef struct Authors Authors;
struct Authors;

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

#endif /* NETWORK__COMPS_H */