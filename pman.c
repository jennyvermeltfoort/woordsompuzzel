

#include "pman.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WAARDE_UNDF -1
#define LETTER_UNDF 'z'
#define GRONDGETAL_MIN 2
#define AANTAL_LETTERS 256

#define OFFSET_OF(type, member) ((size_t) & ((type*)0)->member)
#define CONTAINER_OF(ptr, type, member)                      \
    ({                                                       \
        const __typeof__(((type*)0)->member)* _mptr = (ptr); \
        (type*)((char*)_mptr - OFFSET_OF(type, member));     \
    })
#define PMAN_NEEM_LETTERS_KOLOM(_pman, _ki)   \
    {                                         \
        woord_neem_letter(_pman, 0, _ki),     \
            woord_neem_letter(_pman, 1, _ki), \
            woord_neem_letter(_pman, 2, _ki), \
    }

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

// inline
bool heeft_letter(pman_t* pman, char l) {
    return pman->heeft_letter[(int)l];
}

// inline
void geef_letter(pman_t* p, char l) {
    if (!heeft_letter(p, l) && !letter_is_verstrekt(p, l)) {
        p->heeft_letter[(int)l] = true;
        p->letters[p->aantal_letters++] = l;
        p->letters[p->aantal_letters] = LETTER_UNDF;
    }
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

rlo_knoop_t* rlo_neem_voor(rlo_t* rlo) {
    rlo_knoop_t* k = rlo->start;
    if (rlo->size > 1) {
        rlo->start = rlo->start->volgende;
        rlo->eind->volgende = rlo->start;
    }
    rlo->size--;
    return k;
}

rlo_knoop_t* rlo_neem(rlo_t* rlo, int waarde) {
    rlo_knoop_t* k = rlo_neem_voor(rlo);
    while (k->waarde != waarde) {
        rlo_geef_eind(rlo, k);
        k = rlo_neem_voor(rlo);
    }
    return k;
}

void rlo_geef_eind(rlo_t* rlo, rlo_knoop_t* k) {
    k->volgende = rlo->start;
    rlo->eind->volgende = k;
    rlo->eind = k;
    rlo->size++;
}

pman_res_t waarde_verstrek(pman_t* pman, char l, int w) {
    if (!letter_is_verstrekt(pman, l) &&
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
        pman->letter_waarde[(int)l] = WAARDE_UNDF;  // waarom?
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

void zoek_oplossing(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                    int li, int ki);

pman_res_t check_linkse_kolom(pman_t* p) {
    int ki = max(p->handle.lengtes[0], p->handle.lengtes[1]) - 1;
    char wl[3] = PMAN_NEEM_LETTERS_KOLOM(p, ki);

    if (letter_is_verstrekt(p, wl[0]) &&
        letter_is_verstrekt(p, wl[1])) {
        int w = neem_waarde(p, wl[0]) + neem_waarde(p, wl[1]);
        if ((w > p->handle.grondgetal) != p->c[ki + 1]) {
            return PMAN_RES_OK;
        }
    }
    return PMAN_RES_OK;
}

void verwerk_kolom(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                   int li, int ki) {
    char wl[3] = PMAN_NEEM_LETTERS_KOLOM(pman, ki);

    if (!(letter_is_verstrekt(pman, wl[0]) &&
          letter_is_verstrekt(pman, wl[1]))) {
        return zoek_oplossing(pman, o, rlo, li + 1, ki);
    }

    if (check_linkse_kolom(pman) != PMAN_RES_OK) {
        return;
    }

    int w = neem_waarde(pman, wl[0]) + neem_waarde(pman, wl[1]) +
            pman->c[ki];
    pman->c[ki + 1] = (w >= pman->handle.grondgetal);
    w = w % pman->handle.grondgetal;

    if (waarde_verstrek(pman, wl[2], w) == PMAN_RES_OK) {
        zoek_oplossing(pman, o, rlo, li + 1, ki + 1);
        waarde_ontdoe(pman, wl[2]);
        return;
    }

    if (letter_is_verstrekt(pman, wl[2]) &&
        neem_waarde(pman, wl[2]) == w) {
        zoek_oplossing(pman, o, rlo, li + 1, ki + 1);
        return;
    }

    return;
}

pman_res_t valideer_eind(pman_t* pman, pman_oplossing_t* o) {
    int eki = pman->handle.lengtes[2] - 1;
    // char wl[3] = PMAN_NEEM_LETTERS_KOLOM(pman, eki);

    // Controleer of er geen leading zero's zijn.
    for (int i = 0; i < AANTAL_WOORDEN; i++) {
        char wl =
            woord_neem_letter(pman, i, pman->handle.lengtes[i] - 1);
        if (neem_waarde(pman, wl) == 0) {
            return PMAN_RES_ERR;
        }
    }

    // Carry van de kolom buiten de lengte van het tweede mag niet
    // 1 zijn.
    if (pman->c[eki + 1] == 1) {
        return PMAN_RES_ERR;
    }

    if (!o->zoek_uniek && o->size == 0) {
        kopieer_oplossing(pman, o);
    }

    return PMAN_RES_OK;
}

void zoek_oplossing(pman_t* pman, pman_oplossing_t* o, rlo_t* rlo,
                    int li, int ki) {
    char l = pman->letters[li];

    if (o->zoek_uniek && o->oplossingen > 1) {
        return;
    }

    if (l == LETTER_UNDF) {
        o->oplossingen += (valideer_eind(pman, o) == PMAN_RES_OK);
        return;
    }

    o->bekeken++;
    if (letter_is_verstrekt(pman, l)) {
        verwerk_kolom(pman, o, rlo, li, ki);
        return;
    }

    for (int i = rlo->size; i > 0; i--) {
        rlo_knoop_t* k = rlo_neem_voor(rlo);

        if (waarde_verstrek(pman, l, k->waarde) == PMAN_RES_OK) {
            verwerk_kolom(pman, o, rlo, li, ki);
            waarde_ontdoe(pman, l);
        }

        rlo_geef_eind(rlo, k);
    }

    return;
}

pman_res_t pman_init(pman_t* p) {
    // Bouw een lijst met daarin alle unieke letters. Deze wordt
    // opgebouwd van rechts boven naar beneden, dit zigzaggend naar
    // links. Hierdoor is de lijst meteen gesorteed zodat de meest
    // rechtse kolom als eerste kan worden toegekend in
    // zoek_oplossingen.
    if (max(p->handle.lengtes[0], p->handle.lengtes[1]) <
        p->handle.lengtes[2]) {
        p->c[p->handle.lengtes[2]] = 1;
        waarde_verstrek(
            p, p->handle.woord[2][p->handle.lengtes[2] - 1], 1);
    }

    for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
        for (int i = 0; i < p->handle.lengtes[wi]; i++) {
            char w = woord_neem_letter(p, wi, i);
            if (w < 'A' || w > 'Z') {
                return PMAN_RES_ERR;
            }
            geef_letter(p, w);
        }
    }

    if (p->aantal_letters > p->handle.grondgetal ||
        max(p->handle.lengtes[0], p->handle.lengtes[1]) + 1 <
            p->handle.lengtes[2] ||
        min(p->handle.lengtes[0], p->handle.lengtes[1]) >
            p->handle.lengtes[2]) {
        return PMAN_RES_ERR;
    }

    return PMAN_RES_OK;
}

void pman_zoek_oplossingen(const pman_handle_t* const h,
                           pman_oplossing_t* o) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);
    rlo_t rlo = {};

    rlo_init(&rlo, h->grondgetal);
    zoek_oplossing(pman, o, &rlo, 0, 0);
}

int construeer_puzzels(pman_t* p, pman_puzzel_t* r, rlo_t* rlo,
                       int lw, int li) {
    int acc = 0;

    if (li >= lw - 2) {
        pman_oplossing_t o = {.oplossingen = 0, .zoek_uniek = true};
        r->bekeken++;
        p->handle.lengtes[2] = li + 1;
        zoek_oplossing(p, &o, rlo, 0, 0);
        if (o.oplossingen == 1 && r->size == 0) {
            for (int i = 0; i < p->handle.lengtes[2]; i++) {
                r->letter[i] = p->handle.woord[2][i];
            }
            r->size = p->handle.lengtes[2];
        }
        acc += (o.oplossingen == 1) ? 1 : 0;
    }

    if (li >= lw - 1) {
        return acc;
    }

    for (int i = 0; i < p->aantal_letters; i++) {
        p->handle.woord[2][li] = p->letters[i];
        acc += construeer_puzzels(p, r, rlo, lw, li + 1);
    }

    return acc;
}

int pman_contrueer_puzzels(const pman_handle_t* const h,
                           pman_puzzel_t* r) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);
    pman_t p = {};
    rlo_t rlo = {};

    memcpy(&p.handle, &pman->handle, sizeof(p.handle));
    pman_init(&p);

    int mu = p.handle.grondgetal - p.aantal_letters;
    int b = 'A' - 1;
    for (int i = 0; i < mu; i++) {
        while (heeft_letter(pman, ++b));
        geef_letter(&p, b);
    }

    rlo_init(&rlo, p.handle.grondgetal);

    int lw = max(p.handle.lengtes[0], p.handle.lengtes[1]) + 1;

    int acc = construeer_puzzels(&p, r, &rlo, lw, 0);
    return (acc > 0) ? acc : -1;
}

pman_res_t pman_waarde_verstrek(const pman_handle_t* const h, char l,
                                int w) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);
    return heeft_letter(pman, l) ? waarde_verstrek(pman, l, w)
                                 : PMAN_RES_ERR;
}

pman_res_t pman_waarde_ontdoe(const pman_handle_t* const h, char l) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);
    return heeft_letter(pman, l) ? waarde_ontdoe(pman, l)
                                 : PMAN_RES_ERR;
}

void pman_print(const pman_handle_t* const h) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);

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

void pman_destroy(const pman_handle_t* h) {
    pman_t* pman = CONTAINER_OF(h, pman_t, handle);
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
