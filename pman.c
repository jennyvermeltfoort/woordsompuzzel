

#include "pman.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define offsetof(type, member) ((size_t) & ((type*)0)->member)
#define container_of(ptr, type, member)                      \
    ({                                                       \
        const __typeof__(((type*)0)->member)* _mptr = (ptr); \
        (type*)((char*)_mptr - offsetof(type, member));      \
    })

#define WAARDE_UNDF -1
#define LETTER_UNDF 'z'
#define GRONDGETAL_MIN 2
#define AANTAL_LETTERS 256  // for each char

inline int min(int a, int b) { return (a < b) ? a : b; }
inline int max(int a, int b) { return (a > b) ? a : b; }

inline void str_cpy(const char* s1, char* s2, int ss1) {
    while (ss1--) {
        *s2++ = *s1++;
    }
    *s2 = '\0';
}

typedef struct {
    pman_handle_t handle;
    int letter_waarde[AANTAL_LETTERS];
    bool heeft_letter[AANTAL_LETTERS];
    bool is_letter_toegekend[AANTAL_LETTERS];
    bool is_waarde_toegekend[GRONDGETAL_MAX];
    int unieke_letters[GRONDGETAL_MAX + 1];
    int aantal_unieke_letters;
    int c[GRONDGETAL_MAX + 1];
    int kolom_posities[AANTAL_LETTERS][GRONDGETAL_MAX];
    int kolom_posities_index[AANTAL_LETTERS];
} pman_t;  // puzzel manager.

inline char woord_neem_letter(pman_t* pman, int wi, int li) {
    return pman->handle.woord[wi][pman->handle.lengtes[wi] - 1 - li];
}

inline char neem_waarde(pman_t* pman, char l) {
    return pman->letter_waarde[(int)l];
}

inline bool letter_is_verstrekt(pman_t* pman, char l) {
    return pman->letter_waarde[(int)l] != WAARDE_UNDF;
}

typedef struct RLO_KNOOP_T rlo_knoop_t;
struct RLO_KNOOP_T {
    int waarde;
    rlo_knoop_t* volgende;
};

typedef struct {
    rlo_knoop_t a[GRONDGETAL_MAX];
    rlo_knoop_t* start;
    rlo_knoop_t* eind;
    int size;
} rlo_t;

void rlo_init(rlo_t* rlo, int size) {
    rlo->start = &rlo->a[0];
    rlo->eind = &rlo->a[GRONDGETAL_MAX - 1];
    rlo->size = size;

    for (int i = 0; i < GRONDGETAL_MAX - 1; i++) {
        rlo->a[i].volgende = &rlo->a[i + 1];
    }
    rlo->eind->volgende = rlo->start;

    for (int i = 0; i < GRONDGETAL_MAX; i++) {
        rlo->a[i].waarde = i;
    }
}

inline rlo_knoop_t* rlo_pop(rlo_t* rlo) {
    rlo_knoop_t* k = rlo->start;
    rlo->start = rlo->start->volgende;
    rlo->eind->volgende = rlo->start;
    rlo->size--;
    return k;
}

inline rlo_knoop_t* rlo_start(rlo_t* rlo) { return rlo->start; }

inline void rlo_put(rlo_t* rlo, rlo_knoop_t* k) {
    k->volgende = rlo->start;
    rlo->eind->volgende = k;
    rlo->eind = k;
    rlo->size++;
}

pman_res_t waarde_verstrek(pman_t* pman, char l, int w) {
    if (pman->heeft_letter[(int)l] &&
        !pman->is_letter_toegekend[(int)l] &&
        !pman->is_waarde_toegekend[w]) {
        pman->letter_waarde[(int)l] = w % pman->handle.grondgetal;
        pman->is_letter_toegekend[(int)l] = true;
        pman->is_waarde_toegekend[w] = true;
        return PMAN_RES_OK;
    };
    return PMAN_RES_ERR;
}

pman_res_t waarde_ontdoe(pman_t* pman, char l) {
    if (letter_is_verstrekt(pman, l)) {
        pman->is_letter_toegekend[(int)l] = false;
        pman->is_waarde_toegekend[pman->letter_waarde[(int)l]] =
            false;
        pman->letter_waarde[(int)l] = WAARDE_UNDF;
        return PMAN_RES_OK;
    };
    return PMAN_RES_ERR;
}

pman_res_t kolom_verstrek(pman_t* pman, rlo_t* rlo, int ki,
                          rlo_knoop_t* k1, rlo_knoop_t* k2) {
    char lw0 = woord_neem_letter(pman, 0, ki);
    char lw1 = woord_neem_letter(pman, 1, ki);
    char lw2 = woord_neem_letter(pman, 2, ki);

    if (lw0 == lw1) {
        k1 = rlo_pop(rlo);
        k2 = k1;
    } else {
        k1 = rlo_pop(rlo);
        k2 = rlo_pop(rlo);
    }

    int w = k1->waarde + k2->waarde;
    waarde_verstrek(pman, lw0, k1->waarde);
    waarde_verstrek(pman, lw1, k2->waarde);
    pman_res_t res = waarde_verstrek(pman, lw2, w);

    if (res == PMAN_RES_OK) {
        pman->c[ki + 1] = w > pman->handle.grondgetal;
    }

    return res;
}

void kolom_undoe(pman_t* pman, rlo_t* rlo, int ki, rlo_knoop_t* k1,
                 rlo_knoop_t* k2) {
    waarde_ontdoe(pman, woord_neem_letter(pman, 0, ki));
    waarde_ontdoe(pman, woord_neem_letter(pman, 1, ki));
    waarde_ontdoe(pman, woord_neem_letter(pman, 2, ki));
    pman->c[ki + 1] = WAARDE_UNDF;

    if (k1 == k2) {
        rlo_put(rlo, k1);
    } else {
        rlo_put(rlo, k1);
        rlo_put(rlo, k2);
    }
}

inline pman_res_t valideer_kolom(pman_t* pman, int ki) {
    char lw0 = woord_neem_letter(pman, 0, ki);
    char lw1 = woord_neem_letter(pman, 1, ki);
    char lw2 = woord_neem_letter(pman, 2, ki);

    if (letter_is_verstrekt(pman, lw0) &&
        letter_is_verstrekt(pman, lw1) &&
        letter_is_verstrekt(pman, lw2)) {
        int ci = neem_waarde(pman, lw2) - neem_waarde(pman, lw0) -
                 neem_waarde(pman, lw1);

        if (ci < 0 || ci > 1 ||
            (pman->c[ki] != WAARDE_UNDF && pman->c[ki] != ci)) {
            return PMAN_RES_ERR;
        }

        pman->c[ki] = ci;
    } else if (letter_is_verstrekt(pman, lw0) &&
               letter_is_verstrekt(pman, lw1) &&
               pman->c[ki] != WAARDE_UNDF) {
        int w = neem_waarde(pman, lw0) + neem_waarde(pman, lw1) +
                pman->c[ki];
        return waarde_verstrek(pman, lw2, w);
    }
    return PMAN_RES_OK;
}

pman_res_t valideer_puzzel(pman_t* pman, char l) {
    for (int i = 0; i < pman->kolom_posities_index[(int)l]; i++) {
        int ki = pman->kolom_posities[(int)l][i];
        if (valideer_kolom(pman, ki) != PMAN_RES_OK) {
            return PMAN_RES_ERR;
        }
    }
    return PMAN_RES_OK;
}

void kopieer_oplossing(pman_t* pman, pman_oplossing_t* o) {
    for (int i = 0; pman->aantal_unieke_letters; i++) {
        char l = pman->unieke_letters[i];
        o->letter[i] = l;
        o->waarde[i] = neem_waarde(pman, l);
        o->size++;
    }
}

int zoek_oplossing(pman_t* pman, rlo_t* rlo, int ul_index,
                   pman_oplossing_t* o) {
    char l = pman->unieke_letters[ul_index];
    int acc = 0;

    o->bekeken += rlo->size;

    if (rlo->size < 2) {
        rlo_knoop_t* k = rlo_start(rlo);
        waarde_verstrek(pman, l, k->waarde);
        if (valideer_puzzel(pman, l) == PMAN_RES_OK) {
            acc++;
            if (o->size == 0) {
                kopieer_oplossing(pman, o);
            }
        }
        waarde_ontdoe(pman, l);
        return acc;
    }

    for (int i = rlo->size; i > 0; i--) {
        rlo_knoop_t* k = rlo_pop(rlo);
        waarde_verstrek(pman, l, k->waarde);
        if (valideer_puzzel(pman, l) == PMAN_RES_OK) {
            acc += zoek_oplossing(pman, rlo, ul_index + 1, o);
        }
        waarde_ontdoe(pman, l);
        rlo_put(rlo, k);
    }

    return acc;
}

int pman_zoek_oplossingen(const pman_handle_t* const h,
                          pman_oplossing_t* o) {
    pman_t* pman = container_of(h, pman_t, handle);
    int acc = 0;

    rlo_t rlo = {};  // roterende lijst van waardes
    rlo_init(&rlo, pman->aantal_unieke_letters);

    for (int i = rlo.size; i > 0; i--) {
        rlo_knoop_t* k1 = NULL;
        rlo_knoop_t* k2 = NULL;
        if (kolom_verstrek(pman, &rlo, 0, k1, k2) == PMAN_RES_OK) {
            acc += zoek_oplossing(pman, &rlo, 0, o);
        };
        kolom_undoe(pman, &rlo, 0, k1, k2);
    }

    return (acc > 0) ? acc : -1;
}

pman_res_t pman_waarde_verstrek(const pman_handle_t* const h, char l,
                                int w) {
    pman_t* pman = container_of(h, pman_t, handle);
    return waarde_verstrek(pman, l, w);
}

pman_res_t pman_waarde_ontdoe(const pman_handle_t* const h, char l) {
    pman_t* pman = container_of(h, pman_t, handle);
    return waarde_ontdoe(pman, l);
}

void pman_print(const pman_handle_t* const h) {
    pman_t* pman = container_of(h, pman_t, handle);
    printf("Grontgetal: %i \n", h->grondgetal);

    printf("Puzzel:\n");
    for (int z = 0; z < AANTAL_WOORDEN; z++) {
        printf("%i: ", z);
        const char* w = h->woord[z];
        do {
            printf("%c", *w);
        } while (*++w != '\0');
        printf("\n");
    }

    printf("Toegekende letters:\n");
    for (char i = 'A'; i <= 'Z'; i++) {
        int w = pman->letter_waarde[(int)i];
        if (w >= 0) {
            printf("%c,%i; ", i, pman->letter_waarde[(int)i]);
        }
    }
}

pman_res_t pman_init(pman_t* pman) {
    pman->c[0] = 0;
    for (int i = 1; i < GRONDGETAL_MAX; i++) {
        pman->c[i] = WAARDE_UNDF;
    }

    for (int i = 0; i < AANTAL_LETTERS; i++) {
        pman->letter_waarde[i] = WAARDE_UNDF;
    }

    for (int i = 0; i < AANTAL_WOORDEN; i++) {
        if (!pman->handle.woord[i][0]) {
            return PMAN_RES_ERR;
        }
    }

    int ui = 0;
    for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
        char* w = pman->handle.woord[wi];
        int ki = pman->handle.lengtes[wi] - 1;
        do {
            if (*w < 'A' || *w > 'Z') {
                return PMAN_RES_ERR;
            }

            if (!pman->heeft_letter[(int)*w]) {
                pman->heeft_letter[(int)*w] = true;
                pman->aantal_unieke_letters++;
                pman->unieke_letters[ui++] = *w;
            }

            pman->kolom_posities[(
                int)*w][pman->kolom_posities_index[(int)*w]++] = ki;
        } while (ki-- > 0);
    }
    pman->unieke_letters[GRONDGETAL_MAX] = LETTER_UNDF;

    if (max(pman->handle.lengtes[0], pman->handle.lengtes[1]) + 1 <
            pman->handle.lengtes[2] ||
        pman->aantal_unieke_letters > pman->handle.grondgetal) {
        return PMAN_RES_ERR;
    }

    return PMAN_RES_OK;
}

void pman_destroy(const pman_handle_t* h) {
    pman_t* pman = container_of(h, pman_t, handle);
    free(pman);
}

const pman_handle_t* const pman_create(int gg, const char* w0,
                                       int sw0, const char* w1,
                                       int sw1, const char* w2,
                                       int sw2) {
    if (sw0 > GRONDGETAL_MAX || sw1 > GRONDGETAL_MAX ||
        sw2 > GRONDGETAL_MAX || gg > GRONDGETAL_MAX ||
        gg < GRONDGETAL_MIN) {
        return NULL;
    }

    pman_t* pman = malloc(sizeof(pman_t));
    pman->handle.grondgetal = gg;
    pman->handle.lengtes[0] = sw0;
    pman->handle.lengtes[1] = sw1;
    pman->handle.lengtes[2] = sw2;
    str_cpy(w0, pman->handle.woord[0], sw0);
    str_cpy(w1, pman->handle.woord[1], sw1);
    str_cpy(w2, pman->handle.woord[2], sw2);

    if (pman_init(pman) != PMAN_RES_OK) {
        free(pman);
        return NULL;
    }

    return &pman->handle;
}
