Zakladne veci
---------------

mapa, hraci, kamene, neznicitelne kamene, bomby, bonusy.

mapa:
- policka
    - je bomba
    - je hrac
    - je kamen
    - je neprechodny kamen
    - je bonus (urci sa po odstraneni kamena)

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
- vianoce == kopa bonusov po celej mape
- antibonus - mina, diera, dalsia bomba

ficurie:
- zvukove efekty :D
- obrazky

Nezabudnut
-----------
- [ ] modifikovat klienta: teraz nedostava zvlast viditelny teren
- [ ] @Tomi: serializacia bool'ov : je korektna?
- [ ] doriesit `update.h` a externovanie konstant
- [ ] celkovo `klient/main.cpp` : jednodusit, vymysliet protokol posielania odpovede
- [ ] ### observer
- [ ] urvat `viditelnyTeren` z kodu
- [ ] premenovat `pribliznyTeren` na `uvodnyTeren`
- [ ] lepsie vyzerajuce skore (x100)
- [ ] body za last standing (povodna pointa Bobmermanu :)  za hlinu 3, za frag 50, za last man standing 400, za samovrazdu -40)
- [ ] skore nemoze klesnut pod nulu :D
- [ ] static inline vytvorHraca
- [ ] aftetwards: delete `prvy_review.pdf` :D 


- [ ] rewrite odsimulujKolo
- [ ] prepisat fazu vybuchovania
- [ ] pre kazdu fciu z `update.cpp` rozhodnut ci si zasluzi mat predponu extern
