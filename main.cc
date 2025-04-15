// Hoofdprogramma voor oplossing voor tweede programmeeropdracht Algoritmiek,
// voorjaar 2025: Woordsompuzzel
//
// Biedt de gebruiker een menustructuur om
// * een grondtal en een drietal woorden op te geven waarbij waardes aan
//   letters kunnen worden toegekend, letters weer kunnen worden vrijgemaakt,
//   en het aantal oplossingen kan worden bepaald
// * een grondtal en twee woorden op te geven, waarbij complete puzzels
//   met een unieke oplossing worden geconstrueerd
//
// Rudy van Vliet, 7 april 2025

#include <iostream>
#include <cstring>
#include <ctime>  // voor clock() en clock_t
#include "woordsompuzzel.h"
using namespace std;

//*************************************************************************

// Schrijf het menu op het scherm en vraag een keuze van de gebruiker.
// Retourneer: de keuze van de gebruiker
int keuzeUitMenu ()
{ int keuze;

  cout << endl;
  cout << "1. Een waarde toekennen aan een letter" << endl;
  cout << "2. Een letter weer vrij maken" << endl;
  cout << "3. Aantal oplossingen van puzzel bepalen" << endl;
  cout << "4. Stoppen met deze puzzel" << endl;
  cout << endl;
  cout << "Maak een keuze: ";
  cin >> keuze;

  return keuze;

}  // keuzeUitMenu

//*************************************************************************

void drukAfOplossing (vector < pair <char,int> > oplossing)
{ int grootte = oplossing.size();

  cout << "Een gevonden oplossing is:" << endl;
  for (int i=0;i<grootte;i++)
    cout << " " << oplossing[i].first << "=" << oplossing[i].second;

  cout << endl;

}  // drukAfOplossing

//*************************************************************************

// Vraag de gebruiker om de drie woorden van een woordsompuzzel, en bied
// een menu om met deze puzzel te werken.
void doePuzzel ()
{ WoordSomPuzzel *wsp1;  // pointer, om makkeijk nieuwe objecten te kunnen maken
                   // en weer weg te gooien
  string nwWoord[AantalWoorden];
  clock_t t1, t2;
  char kar;
  int grondtal,
      keuze,   // uit menu
      nwWaarde,
      nrOplossingen;
  long long deelOplossingen;
  vector < pair<char,int> > oplossing;

  cout << endl;
  cout << "Voer het grondtal van de puzzel in." << endl;
  cout << "grondtal: ";
  cin >> grondtal;
  cout << "Voer de drie woorden van de puzzel in." << endl;
  cout << "Woord 0: ";
  cin >> nwWoord[0];
  cout << "Woord 1: ";
  cin >> nwWoord[1];
  cout << "Woord 2: ";
  cin >> nwWoord[2];
  wsp1 = new WoordSomPuzzel (grondtal, nwWoord[0], nwWoord[1], nwWoord[2]);

  do
  {
    wsp1 -> drukAfPuzzel ();
    keuze = keuzeUitMenu ();

    switch (keuze)
    { case 1: cout << endl;
              cout << "Geef de letter waaraan je een waarde wil toekennen: ";
              cin >> kar;
              cout << "Geef de waarde: ";
              cin >> nwWaarde;
              if (!(wsp1->kenWaardeToe (kar, nwWaarde)))
              { cout << endl;
                cout << "Er is geen waarde toegekend." << endl;
              }
              break;
      case 2: cout << endl;
              cout << "Geef de letter die je wil vrijmaken: ";
              cin >> kar;
              if (!(wsp1->maakLetterVrij (kar)))
              { cout << endl;
                cout << "Er is geen letter vrij gemaakt." << endl;
              }
              break;
      case 3:
        t1 = clock ();
        nrOplossingen = wsp1 -> zoekOplossingen (deelOplossingen, oplossing);
        t2 = clock ();

        cout << endl;
        cout << "De puzzel kent " << nrOplossingen
             << " verschillende oplossing(en)." << endl;
        cout << "Het zoeken van de oplossingen kostte " << (t2-t1)
             << " clock ticks, ofwel "
             << (((double)(t2-t1))/CLOCKS_PER_SEC) << " seconden." << endl;
        cout << "We hebben daarbij " << deelOplossingen
             << " deeloplossingen bekeken." << endl;
        if (nrOplossingen>=1)
          drukAfOplossing (oplossing);
        break;
      case 4: break;
      default: cout << endl;
               cout << "Voer een goede keuze in!" << endl;
    }  // switch

  } while (keuze!=4);

  delete wsp1;

}  // doePuzzel

//*************************************************************************

// Vraag de gebruiker om de eerste twee woorden van een woordsompuzzel,
// en bepaal daarbij het aantal complete puzzels met een unieke oplossing.
void construeerPuzzels ()
{ WoordSomPuzzel *wsp1;  // pointer, om makkeijk nieuwe objecten te kunnen maken
                   // en weer weg te gooien
  string nwWoord[AantalWoorden];
    // We gebruiken alleen nwWoord[0] en nwWoord[1]...
  clock_t t1, t2;
  int grondtal,
      nrPuzzels;

  cout << "Voer het grondtal van de puzzel in." << endl;
  cout << "grondtal: ";
  cin >> grondtal;
  cout << endl;
  cout << "Voer de eerste twee woorden van de puzzel in." << endl;
  cout << "Woord 0: ";
  cin >> nwWoord[0];
  cout << "Woord 1: ";
  cin >> nwWoord[1];

  wsp1 = new WoordSomPuzzel (grondtal, nwWoord[0], nwWoord[1], nwWoord[1]);
   // We geven nwWoord[1] twee keer mee, ook als woord dat de som moet
   // voorstellen. Uiteindelijk gaan we die som niet gebruiken.

  t1 = clock ();
  nrPuzzels = wsp1 -> construeerPuzzels (nwWoord[2]);
  t2 = clock ();

  cout << endl;
  cout << "We vonden " << nrPuzzels << " puzzels met een unieke oplossing." << endl;
  cout << "Het construeren van de puzzels kostte " << (t2-t1)
       << " clock ticks, ofwel "
       << (((double)(t2-t1))/CLOCKS_PER_SEC) << " seconden." << endl;
  if (nrPuzzels>=1)
    cout << "Een mogelijk woord 2 met een unieke oplossing: "
         << nwWoord[2] << endl;

  delete wsp1;

}  // construeerPuzzels

//*************************************************************************

void hoofdmenu ()
{ int keuze;

  do
  {
    cout << endl;
    cout << "1. Een woordsom-puzzel oplossen" << endl;
    cout << "2. Woordsom-puzzels construeren bij twee gegeven woorden" << endl;
    cout << "3. Stoppen" << endl;
    cout << endl;
    cout << "Maak een keuze: ";
    cin >> keuze;
    switch (keuze)
    { case 1: doePuzzel ();
              break;
      case 2: construeerPuzzels ();
              break;
      case 3: break;
      default: cout << endl;
               cout << "Voer een goede keuze in!" << endl;
    }

  } while (keuze!=3);

}  // hoofdmenu

//*************************************************************************
  
int main ()
{
  hoofdmenu ();

  return 0;

}
