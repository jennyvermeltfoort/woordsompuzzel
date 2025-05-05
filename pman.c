/**
 * @file pman.c
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com),
 * https://github.com/jennyvermeltfoort
 * @brief Puzzel manager implementation.
 * @date 2025-04-27
 * @copyright Copyright (c) 2025
 */

#include "pman.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#define WAARDE_UNDF -1
#define LETTER_UNDF 'z'
#define GRONDGETAL_MIN 2
#define AANTAL_LETTERS 256

/**
 * @brief Bereken met de pointer van een child in een struct de
 * pointer van de parent. Zorg ervoor dat de pointer naar de child ook
 * daadwerkelijk gealloceerd is binnen de verwachte parent.
 */
#define OFFSET_OF(type, member) ((size_t) & ((type*)0)->member)
#define CONTAINER_OF(ptr, type, member)                      \
    ({                                                       \
        const __typeof__(((type*)0)->member)* _mptr = (ptr); \
        (type*)((char*)_mptr - OFFSET_OF(type, member));     \
    })

/**
 * @brief Neem alle letters van een kolom.
 */
#define PMAN_NEEM_LETTERS_KOLOM(_pman, _ki)   \
    {                                         \
        woord_lees_letter(_pman, 0, _ki),     \
            woord_lees_letter(_pman, 1, _ki), \
            woord_lees_letter(_pman, 2, _ki), \
    }

inline int min(int a, int b) { return (a < b) ? a : b; }
inline int max(int a, int b) { return (a > b) ? a : b; }
inline void str_cpy(const char* s1, char* s2, int ss1) {
    while (ss1--) {
        *s2++ = *s1++;
    }
}

/**
 * @brief Puzzel manager struct, gebruikt pman_handle_t als interface
 * handle. Beslaat een aantal mappings van letters naar bepaalde
 * informatie.
 */
typedef struct {
    pman_handle_t handle;
    int letter_waarde[AANTAL_LETTERS];
    bool heeft_letter[AANTAL_LETTERS];
    bool letter_aanpasbaar[AANTAL_LETTERS];
    bool is_letter_verstrekt[AANTAL_LETTERS];
    bool is_waarde_verstrekt[GRONDGETAL_MAX];
    int kolom[GRONDGETAL_MAX + 1];
    char letters[GRONDGETAL_MAX + 1];
    int laatste_letter;
    int aantal_letters;
    int c[GRONDGETAL_MAX];
} pman_t;

inline char woord_lees_letter(pman_t* p, int wi, int li) {
    return p->handle.woord[wi][p->handle.lengtes[wi] - 1 - li];
}

inline char neem_waarde(pman_t* p, char l) {
    return p->letter_waarde[(int)l];
}

inline bool letter_is_verstrekt(pman_t* p, char l) {
    return p->is_letter_verstrekt[(int)l];
}

inline bool waarde_is_verstrekt(pman_t* p, int w) {
    return p->is_waarde_verstrekt[w];
}

inline bool heeft_letter(pman_t* p, char l) {
    return p->heeft_letter[(int)l];
}

inline bool letter_is_aanpasbaar(pman_t* p, char l) {
    return p->letter_aanpasbaar[(int)l];
}

inline void geef_letter(pman_t* p, char l) {
    if (!heeft_letter(p, l) && !letter_is_verstrekt(p, l)) {
        p->heeft_letter[(int)l] = true;
        p->letter_aanpasbaar[(int)l] = true;
        p->letters[p->aantal_letters++] = l;
        p->letters[p->aantal_letters] = LETTER_UNDF;
    }
}

/**
 * @brief Een simpele knoop voor een graf.
 */
typedef struct KNOOP_T knoop_t;
struct KNOOP_T {
    int waarde;
    knoop_t* volgende;
};

/**
 * @brief Simpele FIFO implementatie. Gebruikt a[] als opslag medium,
 * denk aan ruimtelijke localiteit.
 */
typedef struct {
    knoop_t a[GRONDGETAL_MAX];
    knoop_t* start;
    knoop_t* eind;
    int size;
} rlo_t;

/**
 * @brief Initializeer RLO met `size` aantal opeenvolgende nummers.
 *  Verifieerd niet of size kleiner is als GRONDGETAL_MAX.
 *
 * @param rlo De RLO.
 * @param size Het aantal opeenvolgende nummers, maximaal
 * GRONDGETAL_MAX.
 */
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

/**
 * @brief Neem een knoop aan het begin van de ketting.
 * @param rlo Een geinitializeerde rlo.
 * @return knoop_t* De knoop
 */
knoop_t* rlo_neem_voor(rlo_t* rlo) {
    knoop_t* k = rlo->start;
    rlo->start = rlo->start->volgende;
    rlo->eind->volgende = rlo->start;
    rlo->size--;
    if (rlo->size == 0) {
        rlo->start = NULL;
    }
    return k;
}

/**
 * @brief Insert een knoop aan het einde van de ketting.
 * @param rlo Een geinitializeerde rlo.
 * @param k De knoop.
 */
void rlo_geef_eind(rlo_t* rlo, knoop_t* k) {
    if (rlo->start == NULL) {
        rlo->start = k;
        rlo->eind = k;
    }
    k->volgende = rlo->start;
    rlo->eind->volgende = k;
    rlo->eind = k;
    rlo->size++;
}

/**
 * @brief Ken een waarde toe aan een letter.
 * @param p Een geinitializeerde manager.
 * @param l De letter.
 * @param w De waarde.
 * @return pman_res_t PMAN_RES_OK: ok, PMAN_RES_ERR: waarde is niet
 * toegekend; letter is al toegekend of de waarde is al aan een andere
 * letter toegekend.
 */
pman_res_t waarde_verstrek(pman_t* p, char l, int w) {
    if (!letter_is_verstrekt(p, l) && !waarde_is_verstrekt(p, w)) {
        p->letter_waarde[(int)l] = w;
        p->is_letter_verstrekt[(int)l] = true;
        p->is_waarde_verstrekt[w] = true;
        return PMAN_RES_OK;
    }
    return PMAN_RES_ERR;
}

/**
 * @brief Ontdoe de waarde van een letter wanneer deze is toegekend.
 * @param p Een geinitializeerde manager.
 * @param l De letter.
 * @return pman_res_t PMAN_RES_OK: ok, PMAN_RES_ERR: waarde is niet
 * toegekend.
 */
pman_res_t waarde_ontdoe(pman_t* p, char l) {
    if (letter_is_verstrekt(p, l)) {
        p->is_letter_verstrekt[(int)l] = false;
        p->is_waarde_verstrekt[p->letter_waarde[(int)l]] = false;
        p->letter_waarde[(int)l] = 0;
        return PMAN_RES_OK;
    }
    return PMAN_RES_ERR;
}

/**
 * @brief Kopieer alle toegekende waardes naar een oplossing_t.
 * @param p Een geinitializeerde manager.
 * @param o De oplossing
 */
void kopieer_oplossing(pman_t* p, pman_oplossing_t* o) {
    for (int i = 0; i < p->aantal_letters; i++) {
        char l = p->letters[i];
        o->letter[i] = l;
        o->waarde[i] = neem_waarde(p, l);
        o->size++;
    }
}

/**
 * @brief Controleer of de puzzel klopt door te kijken naar leading
 * zero's en de carry van de meest linkse kolom.
 * @param p Een geinitializeerde manager.
 * @return pman_res_t PMAN_RES_OK: puzzel is ok, PMAN_RES_ERR: puzzel
 * is niet ok.
 */
inline pman_res_t valideer_eind(pman_t* p) {
    int eki = max(p->handle.lengtes[0], p->handle.lengtes[1]) - 1;

    // Controleer of er geen leading zero's zijn.
    for (int wi = 0; wi < AANTAL_WOORDEN; wi++) {
        char wl = woord_lees_letter(p, wi, p->handle.lengtes[wi] - 1);
        if (neem_waarde(p, wl) == 0) {
            return PMAN_RES_ERR;
        }
    }

    bool c = max(p->handle.lengtes[0], p->handle.lengtes[1]) <
             p->handle.lengtes[2];
    if (p->c[eki + 1] != c) {
        return PMAN_RES_ERR;
    }

    return PMAN_RES_OK;
}

/**
 * @brief Valideer alle kollomen van *cki tot eki. Letters van woord 2
 * worden hier mogelijk toegekend. flg behoud informatie over welke
 * letters van woord 2 tijdens deze functie call zijn toegekend.
 * Kolommen van rechts naar links, 0 tot lengte woord 2.
 * @param p Een geinitializeerde manager.
 * @param eki Kolom tot.
 * @param cki Kolom vanaf.
 * @param flg true shifted naar de kolom indexen die zijn toegekend.
 * @return pman_res_t PMAN_RES_OK: puzzel is valide, PMAN_RES_ERR: de
 * puzzel is fout.
 */
inline pman_res_t verwerk_kolommen(pman_t* p, int eki, int* cki,
                                   int* flg) {
    while (eki > *cki) {
        char wl[3] = PMAN_NEEM_LETTERS_KOLOM(p, *cki);
        int w = neem_waarde(p, wl[0]) + neem_waarde(p, wl[1]) +
                p->c[*cki];
        p->c[*cki + 1] = (w >= p->handle.grondgetal);
        w = w % p->handle.grondgetal;

        if (waarde_verstrek(p, wl[2], w) == PMAN_RES_OK) {
            *flg |= (1U << *cki);
        } else if (!letter_is_verstrekt(p, wl[2]) ||
                   neem_waarde(p, wl[2]) != w) {
            return PMAN_RES_ERR;
        }

        *cki += 1;
    }
    return PMAN_RES_OK;
}

/**
 * @brief Vind alle oplossingen voor een puzzel.
 * @param p Een geinitializeerde manager.
 * @param o Het resultaat.
 * @param rlo Een circulair buffer met waardes 0 .. g-1.
 * @param li Huidige letter index in p->letter[].
 * @param ki Kolom index tot welke geverifieerd is.
 */
void zoek_oplossing(pman_t* p, pman_oplossing_t* o, rlo_t* rlo,
                    int li, int ki) {
    char l = p->letters[li];
    int flg = 0;
    int oki = ki;

    o->bekeken++;

    if (o->zoek_uniek && o->oplossingen > 1) {
        return;
    }

    if (verwerk_kolommen(p, p->kolom[li], &ki, &flg) == PMAN_RES_OK) {
        if (p->laatste_letter == li) {
            o->oplossingen += (valideer_eind(p) == PMAN_RES_OK);

            if (!o->zoek_uniek && o->oplossingen == 1 &&
                o->size == 0) {
                kopieer_oplossing(p, o);
            }
        } else {
            if (letter_is_verstrekt(p, l)) {
                zoek_oplossing(p, o, rlo, li + 1, ki);
            } else {
                for (int i = rlo->size; i > 0; i--) {
                    knoop_t* k = rlo_neem_voor(rlo);

                    if (waarde_verstrek(p, l, k->waarde) ==
                        PMAN_RES_OK) {
                        zoek_oplossing(p, o, rlo, li + 1, ki);
                        waarde_ontdoe(p, l);
                    }

                    rlo_geef_eind(rlo, k);
                }
            }
        }
    }

    while (ki-- > oki) {
        if (flg & (1U << ki)) {
            waarde_ontdoe(p, woord_lees_letter(p, 2, ki));
        }
    }
}

/**
 * @brief Kijk voor een gegeven puzzel welke woord[2] leidden tot een
 * unieke oplossing.
 * @param p Een geinitializeerde manager.
 * @param r Het resultaat.
 * @param rlo Een circulair buffer met waardes 0 .. g-1.
 * @param lw Maximum lengte van woord[2].
 * @param li Huidige letter index in p->letter[].
 * @param avli Index binnen p->letter[] waar vanaf vrije letters
 * bestaan.
 * @param avl Hoeveelheid beschikbare vrije letters waaruit gekozen
 * mag worden.
 * @return int De hoeveelheid unieke puzzels.
 */
int construeer_puzzels(pman_t* p, pman_puzzel_t* r, rlo_t* rlo,
                       int lw, int li, int avli, int avl) {
    int acc = 0;
    pman_puzzel_t* shres = NULL;
    int* shacc = NULL;
    int shmidres = 0;
    int shmidacc = 0;
    int pids[GRONDGETAL_MAX] = {0};

    if (li == 0) {
        shmidacc = shmget(0, sizeof(int), IPC_CREAT | 0644);
        shmidres = shmget(1, sizeof(pman_puzzel_t), IPC_CREAT | 0644);
        shacc = (int*)shmat(shmidacc, NULL, 0);
        shres = (pman_puzzel_t*)shmat(shmidres, NULL, 0);
    }

    if (li >= lw - 1) {
        pman_oplossing_t o = {.oplossingen = 0, .zoek_uniek = false};

        p->handle.lengtes[2] = li;
        if (max(p->handle.lengtes[0], p->handle.lengtes[1]) <
            p->handle.lengtes[2]) {
            waarde_verstrek(p, p->handle.woord[2][0], 1);
        }

        zoek_oplossing(p, &o, rlo, 0, 0);

        waarde_ontdoe(p, p->handle.woord[2][0]);

        if (r != NULL && o.oplossingen == 1 && r->size == 0) {
            for (int i = 0; i < p->handle.lengtes[2]; i++) {
                r->letter[i] = p->handle.woord[2][i];
            }
            r->size = p->handle.lengtes[2];
        }

        acc += (o.oplossingen == 1);
    }

    if (li >= lw) {
        return acc;
    }

    int s = min(avli + avl, p->aantal_letters);
    for (int i = 0; i < s; i++) {
        int _avl = avl;
        if (i >= (avli + avl - 1)) {
            _avl++;
        }
        p->handle.woord[2][li] = p->letters[i];

        if (li == 0) {
            pids[i] = fork();
            if (pids[i] == 0) {
                pman_puzzel_t* _r = (i == 0) ? shres : NULL;
                *shacc += construeer_puzzels(p, _r, rlo, lw, li + 1,
                                             avli, _avl);
                exit(EXIT_SUCCESS);
            }
        } else {
            acc +=
                construeer_puzzels(p, r, rlo, lw, li + 1, avli, _avl);
        }
        p->handle.woord[2][li] = '\0';
    }

    if (li == 0) {
        for (int i = 0; i < p->aantal_letters; i++) {
            while (waitpid(pids[i], NULL, WNOHANG) == 0);
        }

        memcpy(r, shres, sizeof(pman_puzzel_t));
        acc = *shacc;
        shmdt(shacc);
        shmdt(shres);
        shmctl(shmidacc, IPC_RMID, NULL);
        shmctl(shmidres, IPC_RMID, NULL);
    }

    return acc;
}

/**
 * @brief Initializeer de manager zodat deze klaar is voor
 * zoek_oplossing.
 * @param p De manager.
 * @return pman_res_t PMAN_RES_OK: success, PMAN_RES_ERR: de puzzel
 * beschikt over karakter waardes die niet compatible zijn.
 */
pman_res_t man_init_oplosser(pman_t* p) {
    // Bouw een lijst met daarin alle unieke letters. Deze wordt
    // opgebouwd van rechts boven naar beneden, dit zigzaggend naar
    // links. Hierdoor is de lijst meteen gesorteed zodat de meest
    // rechtse kolom als eerste kan worden toegekend in
    // zoek_oplossingen.
    for (int i = 0; i < p->handle.lengtes[2]; i++) {
        for (int wi = 0; wi < AANTAL_WOORDEN - 1; wi++) {
            if (i < p->handle.lengtes[wi]) {
                char w = woord_lees_letter(p, wi, i);
                if (w < 'A' || w > 'Z') {
                    return PMAN_RES_ERR;
                }
                if (!heeft_letter(p, w)) {
                    p->kolom[p->aantal_letters] = i;
                }
                geef_letter(p, w);
            }
        }
    }
    p->kolom[p->aantal_letters] = p->handle.lengtes[2];
    p->laatste_letter =
        p->aantal_letters;  // laatste letter om toegekend te worden
                            // in zoek_oplossingen. (Base case)
    return PMAN_RES_OK;
}

/**
 * @brief Initializeer pman_t en controleer of de puzzel oplosbaar is.
 * @param p De manager.
 * @return pman_res_t PMAN_RES_OK: succes, PMAN_RES_ERR: de puzzel is
 * niet oplosbaar.
 */
pman_res_t man_init(pman_t* p) {
    if (man_init_oplosser(p) != PMAN_RES_OK) {
        return PMAN_RES_ERR;
    }

    for (int i = 0; i < p->handle.lengtes[2]; i++) {
        char w = woord_lees_letter(p, 2, i);
        if (w < 'A' || w > 'Z') {
            return PMAN_RES_ERR;
        }

        geef_letter(p, w);
    }

    // Meest linkse letter van woord[2] kan al worden toegekend
    // wanneer woord[2] groter is dan de overige twee woorden.
    if (max(p->handle.lengtes[0], p->handle.lengtes[1]) <
        p->handle.lengtes[2]) {
        waarde_verstrek(p, p->handle.woord[2][0], 1);
        p->letter_aanpasbaar[(int)p->handle.woord[2][0]] = false;
    }

    if (p->aantal_letters > p->handle.grondgetal ||
        max(p->handle.lengtes[0], p->handle.lengtes[1]) + 1 <
            p->handle.lengtes[2] ||
        max(p->handle.lengtes[0], p->handle.lengtes[1]) >
            p->handle.lengtes[2] ||
        min(p->handle.lengtes[0], p->handle.lengtes[1]) + 2 <
            p->handle.lengtes[2]) {
        return PMAN_RES_ERR;
    }

    return PMAN_RES_OK;
}

void pman_zoek_oplossingen(const pman_handle_t* const h,
                           pman_oplossing_t* o) {
    pman_t* p = CONTAINER_OF(h, pman_t, handle);
    rlo_t rlo = {};

    rlo_init(&rlo, h->grondgetal);
    zoek_oplossing(p, o, &rlo, 0, 0);
}
int pman_contrueer_puzzels(const pman_handle_t* const h,
                           pman_puzzel_t* r) {
    pman_t* pi = CONTAINER_OF(h, pman_t, handle);
    pman_t p = {};
    rlo_t rlo = {};

    memcpy(&p.handle, &pi->handle, sizeof(p.handle));
    man_init_oplosser(&p);

    // Append de lijst van letters met "vrije letters", zie
    // construeer_puzzels voor meer info.
    int avli = p.aantal_letters;
    int avl = p.handle.grondgetal - p.aantal_letters;
    int b = 'A' - 1;
    for (int i = 0; i < avl; i++) {
        while (heeft_letter(&p, ++b));
        geef_letter(&p, b);
    }

    rlo_init(&rlo, p.handle.grondgetal);
    int lw = max(p.handle.lengtes[0], p.handle.lengtes[1]) + 1;
    int acc = construeer_puzzels(&p, r, &rlo, lw, 0, avli, 1);

    return (acc > 0) ? acc : -1;
}

pman_res_t pman_waarde_verstrek(const pman_handle_t* const h, char l,
                                int w) {
    pman_t* p = CONTAINER_OF(h, pman_t, handle);
    return letter_is_aanpasbaar(p, l) ? waarde_verstrek(p, l, w)
                                      : PMAN_RES_ERR;
}

pman_res_t pman_waarde_ontdoe(const pman_handle_t* const h, char l) {
    pman_t* p = CONTAINER_OF(h, pman_t, handle);
    return letter_is_aanpasbaar(p, l) ? waarde_ontdoe(p, l)
                                      : PMAN_RES_ERR;
}

void pman_print(const pman_handle_t* const h) {
    pman_t* p = CONTAINER_OF(h, pman_t, handle);

    printf("Grondgetal: %i \n", h->grondgetal);

    printf("Puzzel:\n");
    for (int z = 0; z < AANTAL_WOORDEN; z++) {
        printf("%i: ", z);
        printf("%s\n", p->handle.woord[z]);
    }

    printf("Toegekende letters:\n");
    for (int i = 0; i < p->aantal_letters; i++) {
        if (letter_is_verstrekt(p, p->letters[i])) {
            printf("%c=%i; ", p->letters[i],
                   neem_waarde(p, p->letters[i]));
        }
    }
    printf("\n");
}

void pman_destroy(const pman_handle_t* h) {
    pman_t* p = CONTAINER_OF(h, pman_t, handle);
    free(p);
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

    pman_t* p = calloc(1, sizeof(pman_t));
    p->handle.grondgetal = gg;
    p->handle.lengtes[0] = sw0;
    p->handle.lengtes[1] = sw1;
    p->handle.lengtes[2] = sw2;
    str_cpy(w0, p->handle.woord[0], sw0);
    str_cpy(w1, p->handle.woord[1], sw1);
    str_cpy(w2, p->handle.woord[2], sw2);

    if (man_init(p) != PMAN_RES_OK) {
        free(p);
        return NULL;
    }

    return &p->handle;
}
