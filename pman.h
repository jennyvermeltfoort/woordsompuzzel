
#ifndef __PMAN_H
#define __PMAN_H

#define AANTAL_WOORDEN 3U
#define WAARDE_MAX_LETTERS 26
#define WOORD_MAX_LETTERS 26

typedef struct {
    int letter_waarde[WAARDE_MAX_LETTERS];
    char woord[AANTAL_WOORDEN][WOORD_MAX_LETTERS];
    int lengtes[AANTAL_WOORDEN];
    int grontgetal;
} pman_t;  // woord manager.

typedef enum {
    PMAN_RES_OK = 0,
    PMAN_RES_ERR,
} pman_res_t;

pman_res_t pman_puzzel_bereken_validiteit(pman_t*);
pman_res_t pman_verstrek_waarde(pman_t*, char letter, int waarde);
pman_res_t pman_ontdoe_waarde(pman_t*, char letter);
void pman_print(pman_t* pman);

#endif  // __PMAN_H