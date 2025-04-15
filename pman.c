
#include "pman.h"

pman_res_t pman_verstrek_waarde(pman_t* pman, char l, int w) {
    if (!pman->letter_waarde[l]) {
        pman->letter_waarde[l] = w;
        return PMAN_RES_OK;
    };
    return PMAN_RES_ERR;
}

pman_res_t pman_ontdoe_waarde(pman_t* pman, char l) {
    if (pman->letter_waarde[l]) {
        pman->letter_waarde[l] = 0;
        return PMAN_RES_OK;
    };
    return PMAN_RES_ERR;
}

void pman_print(pman_t* pman) {
    printf("Grontgetal: %i \n", pman->grontgetal);

    printf("Puzzel:\n");
    for (int z = 0; z < AANTAL_WOORDEN; z++) {
        printf("%i: ", z);
        for (int i = 0; i < pman->lengtes[z]; i++) {
            printf("%c", pman->woord[z][i]);
        }
        printf("\n");
    }

    printf("Toegekende letters:\n");
    for (char i = 'A'; i <= 'Z'; i++) {
        printf("%c,%i; ", i, pman->letter_waarde[i]);
    }
}

pman_res_t pman_puzzel_bereken_validiteit(pman_t* pman) {}
