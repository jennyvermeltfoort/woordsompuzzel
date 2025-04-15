// Implementatie van klasse WoordSomPuzzel.

#ifndef WoordSomPuzzelCCVar
#define WoordSomPuzzelCCVar

#include "woordsompuzzel.h"

#include <cstring>
#include <iostream>

#include "standaard.h"

using namespace std;

//*************************************************************************

WoordSomPuzzel::WoordSomPuzzel() {}  // default constructor

//*************************************************************************

WoordSomPuzzel::WoordSomPuzzel(int nwGrondtal, string nwwoord0,
                               string nwwoord1, string nwwoord2) {
    std::strcpy(pman.woord[0], nwwoord0.c_str());
    std::strcpy(pman.woord[1], nwwoord1.c_str());
    std::strcpy(pman.woord[2], nwwoord2.c_str());
    pman.lengtes[0] = nwwoord0.length();
    pman.lengtes[1] = nwwoord1.length();
    pman.lengtes[2] = nwwoord2.length();

    if (!pman_puzzel_bereken_validiteit(&pman)) {
        throw new exception("Deze puzzel is niet valide.");
    };
}  // constructor met parameters

//*************************************************************************

void WoordSomPuzzel::drukAfPuzzel() {
    pman_print(&pman);
}  // drukAfPuzzel

//*************************************************************************

bool WoordSomPuzzel::kenWaardeToe(char kar, int nwWaarde) {
    return (pman_verstrek_waarde(&pman, kar, nwWaarde) == PMAN_RES_OK)
               ? true
               : false;
}  // kenWaardeToe

//*************************************************************************

bool WoordSomPuzzel::maakLetterVrij(char kar) {
    return (pman_ontdoe_waarde(&pman, kar) == PMAN_RES_OK) ? true
                                                           : false;
}  // maakLetterVrij

//*************************************************************************

int WoordSomPuzzel::zoekOplossingen(
    long long &deeloplossingen, vector<pair<char, int> > &oplossing) {
    deeloplossingen = 0;
    return 0;
}  // zoekOplossingen

//*************************************************************************

int WoordSomPuzzel::construeerPuzzels(string &mogelijkWoord2) {
    return 0;
}  // construeerPuzzels

//*************************************************************************

#endif
