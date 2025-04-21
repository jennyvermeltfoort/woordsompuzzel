

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

// inline
int min(int a, int b) { return (a < b) ? a : b; }
// inline
int max(int a, int b) { return (a > b) ? a : b; }

// inline
void str_cpy(const char* s1, char* s2, int ss1) {
    while (ss1--) {
        *s2++ = *s1++;
    }
}

typedef struct {
    pman_handle_t handle;
    int letter_waarde[AANTAL_LETTERS];
    bool heeft_letter[AANTAL_LETTERS];
    bool is_letter_verstrekt[AANTAL_LETTERS];
    bool is_waarde_verstrekt[GRONDGETAL_MAX];
    int letters_opl[GRONDGETAL_MAX + 1];
    int aantal_letters_opl;
    int letters[GRONDGETAL_MAX + 1];
    int aantal_letters;
    int c[GRONDGETAL_MAX];
} pman_t;  // puzzel manager.

// inline
char woord_neem_letter(pman_t* pman, int wi, int li) {
    return pman->handle.woord[wi][pman->handle.lengtes[wi] - 1 - li];
}

// inline
char neem_waarde(pman_t* pman, char l) {
    return pman->letter_waarde[(int)l];
}

// inline
bool letter_is_verstrekt(pman_t* pman, char l) {
    return pman->is_letter_verstrekt[(int)l];
}

// inline
bool waarde_is_verstrekt(pman_t* pman, int w) {
    return pman->is_waarde_verstrekt[w];
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

int zoek_oplossing(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                   int c[static GRONDGETAL_MAX], int li, int ki);

void rlo_init(rlo_t* rlo, int size) {
    rlo->size = size;
    rlo->start = &rlo->a[0];
    rlo->eind = &rlo->a[rlo->size - 1];

    for (int i = 0; i < rlo->size - 1; i++) {
        rlo->a[i].volgende = &rlo->a[i + 1];
    }
    rlo->eind->volgende = rlo->start;

    for (int i = 0; i < rlo->size; i++) {
        rlo->a[i].waarde = i;
    }
}

// inline
rlo_knoop_t* rlo_pop(rlo_t* rlo) {
    rlo_knoop_t* k = rlo->start;
    if (rlo->size > 1) {
        rlo->start = rlo->start->volgende;
        rlo->eind->volgende = rlo->start;
    }
    rlo->size--;
    return k;
}

// inline
void rlo_put(rlo_t* rlo, rlo_knoop_t* k) {
    k->volgende = rlo->start;
    rlo->eind->volgende = k;
    rlo->eind = k;
    rlo->size++;
}

pman_res_t waarde_verstrek(pman_t* pman, char l, int w) {
    if (pman->heeft_letter[(int)l] && !letter_is_verstrekt(pman, l) &&
        !waarde_is_verstrekt(pman, w)) {
        pman->letter_waarde[(int)l] = w;
        pman->is_letter_verstrekt[(int)l] = true;
        pman->is_waarde_verstrekt[w] = true;
        return PMAN_RES_OK;
    }
    return PMAN_RES_ERR;
}

pman_res_t waarde_ontdoe(pman_t* pman, char l) {
    if (letter_is_verstrekt(pman, l)) {
        pman->is_letter_verstrekt[(int)l] = false;
        pman->is_waarde_verstrekt[pman->letter_waarde[(int)l]] =
            false;
        pman->letter_waarde[(int)l] = 0;
        return PMAN_RES_OK;
    }
    return PMAN_RES_ERR;
}

void kopieer_oplossing(pman_t* pman, pman_oplossing_t* o) {
    for (int i = 0; i < pman->aantal_letters; i++) {
        char l = pman->letters[i];
        o->letter[i] = l;
        o->waarde[i] = neem_waarde(pman, l);
        o->size++;
    }
}

int verwerk_kolom(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                  int c[static GRONDGETAL_MAX], int li, int ki) {
    char wl0 = woord_neem_letter(pman, 0, ki);
    char wl1 = woord_neem_letter(pman, 1, ki);
    char wl2 = woord_neem_letter(pman, 2, ki);

    if (letter_is_verstrekt(pman, wl0) &&
        letter_is_verstrekt(pman, wl1)) {
        int w =
            neem_waarde(pman, wl0) + neem_waarde(pman, wl1) + c[ki];
        c[ki + 1] = (w >= pman->handle.grondgetal);
        w = w % pman->handle.grondgetal;

        if (waarde_verstrek(pman, wl2, w) == PMAN_RES_OK) {
            int acc = zoek_oplossing(pman, o, rlo, c, li + 1, ki + 1);
            waarde_ontdoe(pman, wl2);
            return acc;
        }

        if (letter_is_verstrekt(pman, wl2) &&
            neem_waarde(pman, wl2) == w) {
            return zoek_oplossing(pman, o, rlo, c, li + 1, ki + 1);
        }

        return 0;
    }

    return zoek_oplossing(pman, o, rlo, c, li + 1, ki);
}

int zoek_oplossing(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                   int c[static GRONDGETAL_MAX], int li, int ki) {
    char l = pman->letters[li];
    int acc = 0;

    if (l == LETTER_UNDF) {
        for (int i = 0; i < AANTAL_WOORDEN; i++) {
            char wl = woord_neem_letter(pman, i,
                                        pman->handle.lengtes[i] - 1);
            if (neem_waarde(pman, wl) == 0) {
                return acc;
            }
        }

        int eki = pman->handle.lengtes[2] - 1;
        if (c[eki + 1] == 1) {
            return acc;
        }

        int l0ew = neem_waarde(pman, woord_neem_letter(pman, 0, eki));
        int l1ew = neem_waarde(pman, woord_neem_letter(pman, 1, eki));
        char lw2 = woord_neem_letter(pman, 2, eki);
        int w = (l0ew + l1ew + c[eki]) % pman->handle.grondgetal;
        if (letter_is_verstrekt(pman, lw2) &&
            w != neem_waarde(pman, lw2)) {
            return acc;
        }

        if (o->size == 0) {
            if (!letter_is_verstrekt(pman, lw2) &&
                waarde_verstrek(pman, lw2, w) != PMAN_RES_OK) {
                return acc;
            }
            kopieer_oplossing(pman, o);
            waarde_ontdoe(pman, lw2);
        }

        return acc + 1;
    }

    if (letter_is_verstrekt(pman, l)) {
        return verwerk_kolom(pman, o, rlo, c, li, ki);
    }

    for (int i = rlo->size; i > 0; i--) {
        rlo_knoop_t* k = rlo_pop(rlo);

        if (waarde_verstrek(pman, l, k->waarde) == PMAN_RES_OK) {
            acc += verwerk_kolom(pman, o, rlo, c, li, ki);
            waarde_ontdoe(pman, l);
        }

        rlo_put(rlo, k);
    }

    return acc;
}

void rlo_test(rlo_t* rlo) {
    if (rlo->size < 1) {
        printf("leaf, %i.\n", rlo->start->waarde);
        return;
    }

    printf("L: ");
    rlo_knoop_t* n = rlo->start;
    for (int i = rlo->size + 1; i > 0; i--) {
        printf("-> %i. ", n->waarde);
        n = n->volgende;
    }
    printf("\n");

    for (int i = rlo->size; i > 0; i--) {
        rlo_knoop_t* k = rlo_pop(rlo);
        rlo_test(rlo);
        rlo_put(rlo, k);
    }
}

int pman_zoek_oplossingen(const pman_handle_t* const h,
                          pman_oplossing_t* o) {
    pman_t* pman = container_of(h, pman_t, handle);
    int c[GRONDGETAL_MAX] = {0};
    rlo_t rlo = {};  // roterende lijst van waardes
    rlo_init(&rlo, h->grondgetal);

    int acc = zoek_oplossing(pman, o, &rlo, c, 0, 0);
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
        for (int i = 0; i < h->lengtes[z]; i++) {
            printf("%c", w[i]);
        }
        printf("\n");
    }

    printf("Toegekende letters:\n");
    for (int i = 0; i < pman->aantal_letters; i++) {
        if (letter_is_verstrekt(pman, pman->letters[i])) {
            printf("%c=%i; ", pman->letters[i],
                   neem_waarde(pman, pman->letters[i]));
        }
    }
    printf("\n");
}

pman_res_t pman_init(pman_t* pman) {
    for (int i = 0; i < AANTAL_WOORDEN; i++) {
        if (!pman->handle.woord[i][0]) {
            return PMAN_RES_ERR;
        }
    }

    for (int i = 0; i < pman->handle.lengtes[2]; i++) {
        for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
            if (i < pman->handle.lengtes[wi]) {
                char w = woord_neem_letter(pman, wi, i);
                if (w < 'A' || w > 'Z') {
                    return PMAN_RES_ERR;
                }
            }
        }
    }

    for (int i = 0; i < pman->handle.lengtes[2]; i++) {
        for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
            if (i < pman->handle.lengtes[wi]) {
                char w = woord_neem_letter(pman, wi, i);
                if (!pman->heeft_letter[(int)w]) {
                    pman->heeft_letter[(int)w] = true;
                    pman->letters[pman->aantal_letters++] = w;
                }
            }
        }
    }
    pman->letters[pman->aantal_letters] = LETTER_UNDF;

    if (max(pman->handle.lengtes[0], pman->handle.lengtes[1]) + 1 <
            pman->handle.lengtes[2] ||
        pman->aantal_letters > pman->handle.grondgetal) {
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
