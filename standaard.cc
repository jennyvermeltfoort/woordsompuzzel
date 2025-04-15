// Implementatie van standaard functies.

#include <iostream>
#include <cstdlib>  // voor rand
#include "standaard.h"
using namespace std;

//*************************************************************************

bool integerInBereik (const char *variabele, int waarde,
                      int minWaarde, int maxWaarde)
{
  if (waarde>=minWaarde && waarde<=maxWaarde)
    return true;
  else
  { cout << variabele << "=" << waarde << ", maar moet in [" << minWaarde
         << "," << maxWaarde << "] liggen." << endl;
    return false;
  }

}  // integerInBereik

//*************************************************************************

bool integerInBereik (int waarde, int minWaarde, int maxWaarde)
{
  if (waarde>=minWaarde && waarde<=maxWaarde)
    return true;
  else
    return false;

}  // integerInBereik

//*************************************************************************

int randomGetal (int min, int max)
{ int bereik,
      r;

  bereik = max - min + 1;

  r = ((rand())%bereik) + min;
  return r;

}  // randomGetal

//*************************************************************************
  
void genereerRandomPermutatie (int n, int *permutatie)
{ int r,    // random positie in rest van array
      tmp;

  for (int i=0;i<n;i++)
    permutatie[i] = i;

  for (int i=0;i<n;i++)
  {   // vul permutatie[i]
    r = randomGetal (i,n-1);
    tmp = permutatie[i];
    permutatie[i] = permutatie[r];
    permutatie[r] = tmp;
  }

}  // genereerRandomPermutatie

