/* Copyright [2020] Mihai-Eugen Barbu */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 2^16 ~ 65000 --> int 
 * 2^32 ~ 2 * 10^9 --> int64_t/int32_t
 */
#define BITS 16

struct Hashtable {
    int *buckets;
    int hmax;
    int (*zeros)(int64_t);
    int (*hash_ind)(int64_t);
};

/* Numarul de biti = 0 din restul hashului 
 **    --> de la n >> (BITS - 1)
 */
int zeros(int64_t n) {
    int64_t nn;
    int k, poz;
    poz = BITS - 1;
    k = 0;
    nn = n >> poz;
    while ((nn % 2 == 0) && (poz >= 0)) {
        k++;
        nn = n >> (--poz);
    }
    return k;
}

int hash_ind(int64_t n) {
    int nn;
    nn = (int)(n >> BITS);
    return nn;
}

void init_ht(struct Hashtable *ht, int hmax) {
    int i;
    ht->hmax = hmax;
    ht->buckets = (int *)malloc(ht->hmax * sizeof(int));
    for (i = 0; i < ht->hmax; ++i) {
        ht->buckets[i] = 0;
    }
    ht->zeros = zeros;
    ht->hash_ind = hash_ind;
}

int64_t hash_function_int(void *a) {
    /*
     * Credits: https://stackoverflow.com/a/12996028/7883884
     */
    int64_t uint_a = *((int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

void hll_aprox(int ct, int64_t s, int64_t nr) {
    double E, Z, a_m;
    Z = (double)nr / s;
    a_m = 1.079 / ct;
    a_m = a_m + 1;
    a_m = (double)0.7213 / a_m;
    E = (double)Z * ct * ct * a_m;
    printf("%d\n", (int)E);
}

int main(int argc, char *argv[]) {
    struct Hashtable *ht;
    int m, n, ind, ct, max_zeros, p, i;
    int64_t n_hashed, maxi, s, k, nr;

    if (argc != 2) {
        fprintf(stderr, "Can't read the input file\n");
        return -1;
    }

    ht = (struct Hashtable *)malloc(sizeof(struct Hashtable));
    FILE *fin = fopen(argv[1], "r");
    /* Max_buckets */
    m = 1 << 16;
    init_ht(ht, m);

    /* maxi = max_hashed_number */
    maxi = (int64_t)1 << 32;
    while (!feof(fin)) {
        fscanf(fin, "%d", &n);
        n_hashed = (int64_t)hash_function_int(&n);
        n_hashed %= maxi;
        ind = ht->hash_ind(n_hashed);
        ct = ht->zeros(n_hashed);
        if (ct > ht->buckets[ind]) {
            ht->buckets[ind] = ct;
        }
    }

    max_zeros = 0;
    for (i = 0; i < m; ++i) {
        if (ht->buckets[i] > max_zeros) {
            max_zeros = ht->buckets[i];
        }
    }

    /* Precalculam suma 2^(-M[j])
     **     --> aducere la numitor comun
    */
    s = 0;
    ct = 0;
    for (i = 0; i < m; ++i) {
        if (ht->buckets[i]) {
            p = max_zeros - ht->buckets[i];
            k = (int64_t)1 << p;
            s = (int64_t)s + k;
            ct++;
        }
    }
    nr = (int64_t)1 << max_zeros;

    hll_aprox(ct, s, nr);

    free(ht->buckets);
    free(ht);
    fclose(fin);
    return 0;
}
