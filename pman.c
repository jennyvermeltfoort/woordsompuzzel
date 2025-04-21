

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
    int c[GRONDGETAL_MAX];
    int kolom_posities[AANTAL_LETTERS][GRONDGETAL_MAX];
    int kolom_posities_index[AANTAL_LETTERS];
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
    return pman->is_letter_toegekend[(int)l];
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
    if (rlo->size < 2) {
        return rlo->start;
    }
    rlo_knoop_t* k = rlo->start;
    rlo->start = rlo->start->volgende;
    rlo->eind->volgende = rlo->start;
    rlo->size--;
    return k;
}

// inline
rlo_knoop_t* rlo_start(rlo_t* rlo) { return rlo->start; }

// inline
void rlo_put(rlo_t* rlo, rlo_knoop_t* k) {
    if (k != NULL) {
        k->volgende = rlo->start;
        rlo->eind->volgende = k;
        rlo->eind = k;
        rlo->size++;
    }
}

pman_res_t waarde_verstrek(pman_t* pman, char l, int w) {
    if (pman->heeft_letter[(int)l] &&
        !pman->is_letter_toegekend[(int)l] &&
        !pman->is_waarde_toegekend[w]) {
        pman->letter_waarde[(int)l] = w;
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
        pman->letter_waarde[(int)l] = 0;
        return PMAN_RES_OK;
    };
    return PMAN_RES_ERR;
}

// inline
pman_res_t valideer_kolom(pman_t* pman, int ki) {
    char lw0 = woord_neem_letter(pman, 0, ki);
    char lw1 = woord_neem_letter(pman, 1, ki);
    char lw2 = woord_neem_letter(pman, 2, ki);
    int c = pman->c[ki];

    if (letter_is_verstrekt(pman, lw0) &&
        letter_is_verstrekt(pman, lw1) &&
        letter_is_verstrekt(pman, lw2)) {
        if (c != WAARDE_UNDF) {
            int w = (neem_waarde(pman, lw0) + neem_waarde(pman, lw1) +
                     c) %
                    pman->handle.grondgetal;
            int d = w - neem_waarde(pman, lw2);
            if (d != 0) {
                return PMAN_RES_ERR;
            }
        } else {
            // todo make this shit nicer
            int w =
                (neem_waarde(pman, lw0) + neem_waarde(pman, lw1)) %
                pman->handle.grondgetal;
            int d = w - neem_waarde(pman, lw2);

            if (d < 0 || d > 1) {
                return PMAN_RES_ERR;
            }
        }

    } else if (letter_is_verstrekt(pman, lw0) &&
               letter_is_verstrekt(pman, lw1) && c != WAARDE_UNDF) {
        int w =
            (neem_waarde(pman, lw0) + neem_waarde(pman, lw1) + c) %
            pman->handle.grondgetal;
        return (pman->is_waarde_toegekend[w] == true) ? PMAN_RES_ERR
                                                      : PMAN_RES_OK;
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

void c_verstrek(pman_t* pman, char l) {
    for (int i = 0; i < pman->kolom_posities_index[(int)l]; i++) {
        int ki = pman->kolom_posities[(int)l][i];
        char lw0 = woord_neem_letter(pman, 0, ki);
        char lw1 = woord_neem_letter(pman, 1, ki);
        int c = pman->c[ki];
        if (letter_is_verstrekt(pman, lw0) &&
            letter_is_verstrekt(pman, lw1) && c != WAARDE_UNDF) {
            pman->c[ki + 1] =
                (neem_waarde(pman, lw0) + neem_waarde(pman, lw1) +
                 c) >= pman->handle.grondgetal;
        }
    }
}

void c_ontdoe(pman_t* pman, char l) {
    for (int i = 0; i < pman->kolom_posities_index[(int)l]; i++) {
        int ki = pman->kolom_posities[(int)l][i];
        char lw1 = woord_neem_letter(pman, 1, ki);
        if (!letter_is_verstrekt(pman, lw1)) {
            pman->c[ki + 1] = WAARDE_UNDF;
        }
    }
}

void kopieer_oplossing(pman_t* pman, pman_oplossing_t* o) {
    for (int i = 0; i < pman->aantal_unieke_letters; i++) {
        char l = pman->unieke_letters[i];
        o->letter[i] = l;
        o->waarde[i] = neem_waarde(pman, l);
        o->size++;
    }
    o->aantal++;
}

bool _rlo_put(rlo_t* rlo, rlo_knoop_t* k) {
    rlo_put(rlo, k);
    return true;
}

bool _reset_0(pman_t* pman, pman_handle_t* h, rlo_t* rlo,
              rlo_knoop_t** k, int ki, char wl0) {
    if (ki < h->lengtes[0]) {
        rlo_put(rlo, k[0]);
        waarde_ontdoe(pman, wl0);
    }
    return true;
}

bool _reset_1(pman_t* pman, pman_handle_t* h, rlo_t* rlo,
              rlo_knoop_t** k, int ki, char wl1, char wl2) {
    if (ki < h->lengtes[1]) {
        rlo_put(rlo, k[1]);
        waarde_ontdoe(pman, wl1);
    }

    waarde_ontdoe(pman, wl2);
    return true;
}

bool vul_kolom(pman_t* pman, rlo_t* rlo, int ki, int* c,
               pman_oplossing_t* o) {
    pman_handle_t* h = &pman->handle;
    char wl0 = woord_neem_letter(pman, 0, ki);
    char wl1 = woord_neem_letter(pman, 1, ki);
    char wl2 = woord_neem_letter(pman, 2, ki);
    rlo_knoop_t* k[AANTAL_WOORDEN] = {};
    int s0 = rlo->size;

    if (ki == h->lengtes[2]) {
        kopieer_oplossing(pman, o);
        return true;
    }

    do {
        if (ki < h->lengtes[0] && wl0 != wl1) {
            do {
                waarde_ontdoe(pman, wl0);
                rlo_put(pman, k[0]);
                k[0] = rlo_pop(rlo);
                o->bekeken++;
            } while (waarde_verstrek(pman, wl0, k[0]) !=
                         PMAN_RES_OK &&
                     s0--);
        }

        int s1 = rlo->size;
        int w2 = 0;
        do {
            waarde_ontdoe(pman, wl2);
            if (ki < h->lengtes[1]) {
                do {
                    waarde_ontdoe(pman, wl1);
                    rlo_put(pman, k[1]);
                    k[1] = rlo_pop(rlo);
                    o->bekeken++;
                } while (waarde_verstrek(pman, wl1, k[1]) !=
                             PMAN_RES_OK &&
                         s1--);
            }

            w2 = neem_waarde(pman, wl0) + neem_waarde(pman, wl1) +
                 c[ki];
            c[ki + 1] = (w2 >= h->grondgetal);
        } while (!(waarde_verstrek(pman, wl2, w2 % h->grondgetal) ==
                       PMAN_RES_OK &&
                   vul_kolom(pman, rlo, ki + 1, c, o) == false) &&
                 s1--);
    } while (s0--);

    return true;
}

int zoek_oplossing(pman_t* pman, rlo_t* rlo, int ul_index,
                   pman_oplossing_t* o) {
    int acc = 0;
    pman_handle_t* h = &pman->handle;

    int c[GRONDGETAL_MAX] = {0};
    int ki = 0;

    vul_kolom(pman, rlo, ki, c, o);
    return o->aantal;
}

int pman_zoek_oplossingen(const pman_handle_t* const h,
                          pman_oplossing_t* o) {
    pman_t* pman = container_of(h, pman_t, handle);

    rlo_t rlo = {};  // roterende lijst van waardes
    rlo_init(&rlo, pman->aantal_unieke_letters);

    int acc = zoek_oplossing(pman, &rlo, 0, o);
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
        const char* w = pman->handle.woord[z];
        do {
            printf("%c", *w);
        } while (*++w != '\0');
        printf("\n");
    }

    printf("Toegekende letters:\n");
    for (char i = 'A'; i <= 'Z'; i++) {
        int w = pman->letter_waarde[(int)i];
        if (pman->is_waarde_toegekend[w]) {
            printf("%c,%i; ", i, pman->letter_waarde[(int)i]);
        }
    }
    printf("\n");
}

pman_res_t pman_init(pman_t* pman) {
    for (int i = 0; i < GRONDGETAL_MAX; i++) {
        pman->c[i] = WAARDE_UNDF;
    }
    pman->c[0] = 0;

    for (int i = 0; i < AANTAL_WOORDEN; i++) {
        if (!pman->handle.woord[i][0]) {
            return PMAN_RES_ERR;
        }
    }

    int ui = 0;
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
                    pman->unieke_letters[ui++] = w;
                }
            }
        }
    }
    pman->aantal_unieke_letters = ui;
    pman->unieke_letters[ui] = LETTER_UNDF;

    bool gezien[AANTAL_LETTERS][GRONDGETAL_MAX] = {0};
    for (int i = 0; i < pman->handle.lengtes[2]; i++) {
        for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
            if (i < pman->handle.lengtes[wi]) {
                char w = woord_neem_letter(pman, wi, i);
                if (!gezien[(int)w][i]) {
                    pman->kolom_posities[(
                        int)w][pman->kolom_posities_index[(int)w]++] =
                        i;
                    gezien[(int)w][i] = true;
                }
            }
        }
    }

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
