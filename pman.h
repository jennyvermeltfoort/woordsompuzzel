/**
 * @file pman.h
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com),
 * https://github.com/jennyvermeltfoort
 * @brief Puzzel manager implementation.
 * @date 2025-04-27
 * @copyright Copyright (c) 2025
 */

#ifndef __PMAN_H
#define __PMAN_H

#include <stdbool.h>

/**
 * @brief Aantal woorden van de puzzel, niet configurable.
 */
#define AANTAL_WOORDEN 3

/**
 * @brief Het maximum grondgetal, wel configurable.
 */
#define GRONDGETAL_MAX 26

/**
 * @brief Resultaten van de puzzel manager. Per functie is de
 * betekenis verschillend, echter zijn de resultaten wel algemeen
 * gecategoriseerd.
 */
typedef enum {
    PMAN_RES_OK = 0, /**! Het resultaat is goed. */
    PMAN_RES_ERR,    /**! Het resultaat is niet goed. */
} pman_res_t;

/**
 * @brief Handle naar een puzzel manager. Gebruik `pman_create` om
 * een manager aan te maken.
 */
typedef struct {
    int lengtes[AANTAL_WOORDEN]; /**! De lengtes van de woorden,
                                    indexing is 1 op 1. */
    char woord[AANTAL_WOORDEN]
              [GRONDGETAL_MAX +
               1];  /**! De woorden van de puzzel, woord[0] bovenste
                       regel, woord[1] middelste regel, woord[2]
                       onderste regel(de sum). (+1 voor eol)*/
    int grondgetal; /**! Het grondgetal.*/
} pman_handle_t;

/**
 * @brief Resultaat van pman_zoek_oplossingen.
 */
typedef struct {
    char letter[GRONDGETAL_MAX]; /**! Alle toegekende letters, gevuld
                                    vanaf 0 tot size - 1. */
    int waarde[GRONDGETAL_MAX];  /**! Alle waardes van de toegekende
                                    letters indexing is 1 op 1. */
    int size;        /**! De hoeveelheid toegekende letters. */
    int bekeken;     /**! Het aantal bekeken suboplossingen. */
    int oplossingen; /**! De hoeveelheid oplossingen voor de puzzel.
                      */
    bool zoek_uniek; /**! Wanneer true zoek alleen naar unieke
                        oplossingen, letter[] en waarde[] worden niet
                        gevuld. */
} pman_oplossing_t;

/**
 * @brief Resultaat van pman_contrueer_puzzels.
 */
typedef struct {
    char letter[GRONDGETAL_MAX]; /**! Letters van woord[2] die een
                                    unieke oplossing geeft voor de
                                    hele puzzel, gevuld vanaf 0 tot
                                    size - 1. */
    int size;                    /**! De lengte van letters[]. */
} pman_puzzel_t;

/**
 * @brief Vernietig een manager.
 * @param h De handle.
 */
void pman_destroy(const pman_handle_t *);

/**
 * @brief Alloceer geheugen en maak een puzzel manager aan.
 * @param gg Grondgetal.
 * @param w0 Woord 0.
 * @param sw0 Lengte woord 0.
 * @param w1 Woord 1.
 * @param sw1 Lengte woord 1.
 * @param w2 Woord 2.
 * @param sw2 Lengte woord 2.
 * @return const pman_handle_t* const Een pointer naar handle van een
 * geinitalizeerde manager. NULL wanneer de puzzel niet op te lossen
 * is, of wanneer er iets anders niet klopt.
 */
const pman_handle_t *const pman_create(
    int grondgetal, const char *woord0, int size_woord0,
    const char *woord1, int size_woord1, const char *woord2,
    int size_woord2);

/**
 * @brief Ken een waarde toe aan een letter.
 * @param h De handle van een geinitializeerde manager.
 * @param l De letter.
 * @param w De waarde.
 * @return pman_res_t PMAN_RES_OK: ok, PMAN_RES_ERR: waarde is niet
 * toegekend; letter is al toegekend, of de waarde is al aan een
 * andere letter toegekend, of de puzzel heeft deze letter niet.
 */
pman_res_t pman_waarde_verstrek(const pman_handle_t *const,
                                char letter, int waarde);

/**
 * @brief Ontdoe een waarde van een letter.
 * @param h De handle van een geinitializeerde manager.
 * @param l De letter.
 * @return pman_res_t PMAN_RES_OK: ok, PMAN_RES_ERR: waarde is niet
 * toegekend, of de puzzel heeft deze letter niet.
 */
pman_res_t pman_waarde_ontdoe(const pman_handle_t *const,
                              char letter);

/**
 * @brief Vind alle oplossingen van een puzzel.
 * @param h De handle van een geinitializeerde manager.
 * @param o De oplossing.
 */
void pman_zoek_oplossingen(const pman_handle_t *const,
                           pman_oplossing_t *oplossing);

/**
 * @brief Vind alle mogelijke woorden voor woord 2 zodat de puzzel een
 * unieke oplossing heeft.
 * @param h De handle van een geinitializeerde manager.
 * @param r Het resultaat.
 * @return int De hoeveelheid puzzels.
 */
int pman_contrueer_puzzels(const pman_handle_t *const,
                           pman_puzzel_t *puzzel);

/**
 * @brief Print de puzzel.
 * @param h De handle van een geinitializeerde manager.
 */
void pman_print(const pman_handle_t *const);

#endif  // __PMAN_H