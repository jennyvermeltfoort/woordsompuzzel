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