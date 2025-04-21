
#ifndef __PMAN_H
#define __PMAN_H

#define AANTAL_WOORDEN 3
#define GRONDGETAL_MAX 26
#define WOORD_MAX_LETTERS GRONDGETAL_MAX

typedef enum {
    PMAN_RES_OK = 0,
    PMAN_RES_ERR,
} pman_res_t;

typedef struct {
    int lengtes[AANTAL_WOORDEN];
    char woord[AANTAL_WOORDEN]
              [WOORD_MAX_LETTERS + 1];  // woorden 0 tot 2. w0+w1=w2,
                                        // +1 null termination
    int grondgetal;
} pman_handle_t;  // puzzel manager.

typedef struct {
    char letter[GRONDGETAL_MAX];
    int waarde[GRONDGETAL_MAX];
    int size;
    int bekeken;
    int aantal;
} pman_oplossing_t;

void pman_destroy(const pman_handle_t *);

const pman_handle_t *const pman_create(
    int grondgetal, const char *woord0, int size_woord0,
    const char *woord1, int size_woord1, const char *woord2,
    int size_woord2);

pman_res_t pman_waarde_verstrek(const pman_handle_t *const,
                                char letter, int waarde);
pman_res_t pman_waarde_ontdoe(const pman_handle_t *const,
                              char letter);

int pman_zoek_oplossingen(const pman_handle_t *const h,
                          pman_oplossing_t *o);

void pman_print(const pman_handle_t *const);

#endif  // __PMAN_H