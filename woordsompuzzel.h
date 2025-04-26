// Definitie van klasse WoordSomPuzzel.

#ifndef WoordSomPuzzelHVar  // om te voorkomen dat dit .h bestand
                            // meerdere keren
#define WoordSomPuzzelHVar  // wordt ge-include

#include <string>
#include <utility>
#include <vector>

#include "constantes.h"

extern "C" {
#include "pman.h"
}

class WoordSomPuzzel {
   public:
    // Default constructor
    WoordSomPuzzel();
    ~WoordSomPuzzel();

    // Constructor met parameters
    // Controleer
    // * of nwGrondtal een geldig grondtal is,
    // * of nwWoord0, nwWoord1, nwWoord2 niet leeg zijn en alleen
    // letters
    //   uit het alfabet bevatten (hoogstens nwGrondtal verschillende
    //   hoofdletters),
    // * en of de lengtes van nwWoord0, nwWoord1 en nwWoord2 in
    // principe
    //   een oplossing voor de woordsompuzzel nwWoord0 + nwWoord1 =
    //   nwWoord2 toestaan.
    // Zo ja, sla de parameters op in membervariabelen. Dan is er een
    // geldige puzzel. Zo nee, dan is er geen geldige puzzel.
    WoordSomPuzzel(int nwGrondtal, string nwWoord0, string nwWoord1,
                   string nwWoord2);

    // Druk het grondtal, de drie woorden van de puzzel en eventueel
    // reeds aan letters toegekende waardes (samen met de letters) af
    // op het scherm. Controleer eerst of er een geldige puzzel is.
    void drukAfPuzzel();

    // Ken aan letter kar de waarde nwWaarde toe.
    // Controleer eerst:
    // * of er een geldige puzzel is
    // * of kar een letter is in de puzzel, waaraan nog geen waarde is
    //   toegekend
    // * of nwWaarde een geldige waarde is bij het huidige grondtal,
    //   die nog niet aan een andere letter is toegekend
    // Je hoeft (hier) niet te controleren of de puzzel met deze
    // toekenning nog wel oplossingen heeft. Retourneer:
    // * true, als alle controles goed uitpakken
    // * false, anders
    // Post:
    // * Als returnwaarde true is, is de waarde vastgelegd
    // * Anders is er niets veranderd.
    bool kenWaardeToe(char kar, int nwWaarde);

    // Maak een toegekende waarde aan letter kar ongedaan.
    // Controleer eerst:
    // * of er een geldige puzzel is
    // * of kar een letter is in de puzzel, waaraan al een waarde is
    //   toegekend
    // Retourneer:
    // * true, als alle controles goed uitpakken
    // * false, anders
    // Post:
    // * Als returnwaarde true is, is de toekenning van een waarde aan
    // letter
    //   kar ongedaan gemaakt.
    // * Anders is er niets veranderd.
    bool maakLetterVrij(char kar);

    // Bepaal alle oplossingen voor de woordsompuzzel met het grondtal
    // en de woorden uit de constructor met parameters, (indien van
    // toepassing) rekening houdend met reeds aan letters toegekende
    // waarden. Controleer eerst of er een geldige puzzel is.
    // Retourneer:
    // * het aantal oplossingen, als er een geldige puzzel is
    // * -1, anders
    // Post:
    // * Parameter deeloplossingen bevat het aantal deeloplossingen
    // dat we
    //   in deze zoektocht hebben bekeken.
    // * Als de returnwaarde >= 1 is, bevat parameter oplossing een
    // oplossing
    //   voor de puzzel, dwz een rij paren van een letter en een
    //   waarde voor die letter. De volgorde van de rij paren maakt
    //   niet uit.
    // * Dezelfde waardes zijn toegekend aan dezelfde letters als voor
    //   aanroep van deze functie.
    int zoekOplossingen(long long &deeloplossingen,
                        vector<pair<char, int> > &oplossing);

    // Bepaal hoeveel verschillende puzzels met een unieke oplossing
    // er te maken zijn bij het grondtal en de woorden nwWoord0 en
    // nwWoord1 uit de constructor met parameters. Negeer hierbij
    // eventueel reeds aan letters toegekende waarden. Controleer
    // eerst of er een geldige puzzel is. Retourneer:
    // * het aantal puzzels met een unieke oplossing bij het grondtal
    //   en nwWoord0 en nwWoord1, als er een geldige puzzel is
    // * -1, anders
    // Post:
    // * Als de returnwaarde >= 1 is, bevat parameter mogelijkWoord2
    //   een van de gevonden mogelijkheden voor woord2 waarbij de
    //   puzzel een unieke oplossing heeft.
    int construeerPuzzels(string &mogelijkWoord2);

   private:
    const pman_handle_t *phandle;  // puzzel handle
};

#endif
