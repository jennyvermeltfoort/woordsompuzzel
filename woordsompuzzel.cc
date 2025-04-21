// Implementatie van klasse WoordSomPuzzel.

#ifndef WoordSomPuzzelCCVar
#define WoordSomPuzzelCCVar

#include "woordsompuzzel.h"

#include <cstring>
#include <exception>
#include <iostream>

#include "standaard.h"

using namespace std;

//*************************************************************************

WoordSomPuzzel::WoordSomPuzzel() {}  // default constructor

WoordSomPuzzel::~WoordSomPuzzel() {
    if (phandle != NULL) {
        pman_destroy(phandle);
    }
}  // default constructor

//*************************************************************************

WoordSomPuzzel::WoordSomPuzzel(int nwGrondtal, string nwwoord0,
                               string nwwoord1, string nwwoord2) {
    phandle =
        pman_create(nwGrondtal, nwwoord0.c_str(), nwwoord0.length(),
                    nwwoord1.c_str(), nwwoord1.length(),
                    nwwoord2.c_str(), nwwoord2.length());
    if (phandle == NULL) {
        throw new runtime_error("Deze puzzel is niet valide.");
    }
}  // constructor met parameters

//*************************************************************************

void WoordSomPuzzel::drukAfPuzzel() {
    pman_print(phandle);
}  // drukAfPuzzel

//*************************************************************************

bool WoordSomPuzzel::kenWaardeToe(char kar, int nwWaarde) {
    return (pman_waarde_verstrek(phandle, kar, nwWaarde) ==
            PMAN_RES_OK)
               ? true
               : false;
}  // kenWaardeToe

//*************************************************************************

bool WoordSomPuzzel::maakLetterVrij(char kar) {
    return (pman_waarde_ontdoe(phandle, kar) == PMAN_RES_OK) ? true
                                                             : false;
}  // maakLetterVrij

//*************************************************************************

int WoordSomPuzzel::zoekOplossingen(
    long long &deeloplossingen, vector<pair<char, int> > &oplossing) {
    pman_oplossing_t o = {0};
    pman_zoek_oplossingen(phandle, &o);
    deeloplossingen = o.bekeken;

    for (int i = 0; i < o.size; i++) {
        oplossing.push_back({o.letter[i], o.waarde[i]});
    }

    return (o.oplossingen > 0) ? o.oplossingen : -1;
}  // zoekOplossingen

//*************************************************************************

int WoordSomPuzzel::construeerPuzzels(string &mogelijkWoord2) {
    pman_puzzel_t p = {0};
    int oplossingen = pman_contrueer_puzzels(phandle, &p);

    mogelijkWoord2 = "";
    for (int i = 0; i < p.size; i++) {
        mogelijkWoord2 += p.letter[i];
    }

    return oplossingen;
}  // construeerPuzzels

//*************************************************************************

#endif
