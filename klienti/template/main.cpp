#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  // (sem patri vas kod)
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: return Prikaz(<smer>, <ci polozim bombu>)
Prikaz zistiTah() {
  // (sem patri vas kod)
  return Prikaz();
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  unsigned int seed = time(NULL) * getpid();
  srand(seed);

  nacitaj(cin, mapa);
  fprintf(stderr, "START pid=%d, seed=%u\n", getpid(), seed);
  inicializuj();

  while (cin.good()) {
    nacitaj(cin, stav);
    uloz(cout, zistiTah());
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

