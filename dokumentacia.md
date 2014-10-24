
Čo je proboj
------------

Proboj je počítačová hra, ktorej hráčmi nie ste vy, ale programy, čo napíšete.

Téma proboja je Bomberman


Zdrojáky
--------

Štandardný hráč čiže klient (v adresári `klienti/template`) sa skladá z jediného
zdrojáku `main.cpp`. Ale môžte ho rozdeliť aj na viacero. Dokonca môžte použiť
aj iný jazyk (aj keď to je trochu problematické, v prípade záujmu sa ma
opýtajte).

V serveri je tiež zopár zdrojákov, čo vás bude zaujímať.

- `common.h` obsahuje základné štruktúry, čo váš klient dostane k dispozícii.
- `update.cpp` a `update.h` obsahujú všetky herné konštanty, a tiež
  implementáciu väčšiny herných pravidiel, takže ak v pravidlách nie je niečo
  jasné, skúste sa tam pozrieť.
- v `main.cpp` sú tiež nejaké pravidlá (ako sa ťahá apod.), ale to je asi menej
  dôležité.

Kľudne si prečítajte aj ostatné zdrojáky, ja sa len poteším, ale pri kódení
vášho klienta vám asi nepomôžu.


Ako kódiť klienta
-----------------

Skopírujte obsah `klienti/template` do iného adresára a niečo v ňom nakóďte.

V koreni proboju spustite `make`, čím všetko skompilujete. (Ak váš klient nie je
vnútri `klienti`, nastavte v jeho `Makefile` správny `SERVERDIR` a spustite
`make` aj v ňom.)

Potom spustite `./server/server zaznamy/01 mapy/mapa1.ppm klienti/vasklient
klienti/vasklient klienti/hlupy` To spustí hru s troma hráčmi (vaším, druhým
vaším a hlúpym) a uloží záznam do `zaznamy/01`. Ten si môžete pozrieť s príkazom
`./observer/observer zaznamy/01`.

Server sa vášho klienta pýta, čo chce robiť. Ak klient neodpovie včas, bude
automaticky zabitý a reštartovaný. Prvýkrát dostane viac času, aby sa mohol
inicializovať.

Keď server spustíte u vás, je to len na skúšku. Na hlavnom počítači to beží na
ostro. Je tam aj webové rozhranie, cez ktoré môžete uploadovať vašich klientov.
Uploadujú sa zdrojáky a tie sa potom skompilujú (konkrétne sa spustí `make
naserveri SERVERDIR=/adresar/kde/je/server`).


Ako sa ťahá
-----------

TODO zdokumentovať.


Pravidlá hry
------------

TODO zdokumentovať.

mapa, hraci, kamene, neznicitelne kamene, bomby, bonusy.

mapa:
- policka
    je bomba
    je hrac
    je kamen
    je neprechodny kamen
    je bonus (urci sa po odstraneni kamena)

hrac:
- sila bomby
- pocet bomb
- pozicia
- zivy/mrtvy
- skore
- tah: ide alebo polozi bombu pod seba

bomba:
- sila
- cas do vybuchu
- neprechodna

bonus:
- ad hoc hovadina
- vznika po vybucnhuti kamena
- +1 ku max. poctu bomb
- +1 ku sile bomb (dosahu)
- ohnostroj == kopa bomb s casmi x, x+1, x+2, x+3
- antibonus - mina, diera, dalsia bomba

ficurie:
- zvukove efekty :D
- obrazky



