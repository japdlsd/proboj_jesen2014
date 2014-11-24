
#include <ostream>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "update.h"

// clockwise zhora
const int DX[] = { 0, 1, 0, -1 };
const int DY[] = { -1, 0, 1, 0 };

const int kMaximalnaDlzkaHry = 700;

const int kPociatocnaSilaBomb = 2; // je to polomer (ak si vzdialeny r policok, tak si v bezpeci)
const int kPociatocnyMaxPocetBomb = 1;

const int kBonusSanca[BONUS_POCET_TYPOV] = 
    {25, 25, 20, 10, 10, 10}; // (percenta) urcuje aky typ bonusu padne, ak padne

const int kBonusZakladnaSanca = 25; // (percenta) sanca, ze bude nejaky bonus

const int kBombaTimer = 7;

const int kStitTrvanie = 50; // pocet kol

const int kOhnostrojKto = -1;
const int kOhnostrojPocet = 6;
const int kOhnostrojStart = 15;
const int kOhnostrojSila = 3;
const int kOhnostrojPocetPokusov = 10;

const int kVianocePocet = 5;
const int kVianocePocetPokusov = 10;

const int kFreezeInkrement = 30;

const int kBodyZaHlinu = 1;
const int kBodyZaZabitie = 100;
const int kBodyZaSamovrazdu = -20;
const int kBodyZaPrezitie = 500;

static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }

//@TODO observer

#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


inline static Hrac vytvorHraca(const Mapa &mapa, const Bod &poloha, const int i){
  Hrac h = Hrac();
  h.x = poloha.x;
  h.y = poloha.y;
  h.skore = 0;
  h.jeZivy = true;
  h.pocetBomb = 0;
  h.maxPocetBomb = kPociatocnyMaxPocetBomb;
  h.silaBomb = kPociatocnaSilaBomb;
  h.maStit = 0;

  // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
  h.mapovanie.resize(mapa.pocetHracov);
  for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
  random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
  swap(h.mapovanie[0], h.mapovanie[i]);

  return h;
}

Stav zaciatokHry(const Mapa& mapa) {
  Stav stav = Stav();   // tento zapis vsetko inicializuje na nuly

  vector<Bod> starty;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.uvodnyTeren.get(x, y) == MAPA_START) {
      starty.push_back(Bod(x, y));
    }
  }
  random_shuffle(starty.begin(), starty.end());
  
  stav.teren = mapa.uvodnyTeren;

  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if(mapa.uvodnyTeren.get(x, y) == MAPA_START) {
      stav.teren.set(x, y, MAPA_VOLNO);
    }
  }

  stav.hraci.clear();
  for (int i = 0; i < mapa.pocetHracov; i++) {
    stav.hraci.push_back(vytvorHraca(mapa, starty[i], i));
  }

  return stav;
}

void prehladajBfs(const Teren& teren, Bod start, Teren& vzdialenost) {
  int inf = teren.w() * teren.h() * 2;
  vzdialenost.vyprazdni(teren.w(), teren.h(), inf);
  queue<Bod> Q;
  vzdialenost.set(start, 0);
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.get(n) != inf) continue;
      vzdialenost.set(n, vzdialenost.get(p) + 1);
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}

void prehladajLokalneBfs(const Teren& teren, Bod start, int rozsahLimit, map<Bod,int>& vzdialenost) {
  vzdialenost.clear();
  queue<Bod> Q;
  vzdialenost[start] = 0;
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    if (vzdialenost[p] == rozsahLimit) continue;
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.count(n)) continue;
      vzdialenost[n] = vzdialenost[p] + 1;
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}

bool nieJeTuHrac(const Stav& stav, const Bod& p){
  for(int i = 0; i < (int)stav.hraci.size(); i++) 
    if(stav.hraci[i].jeZivy && stav.hraci[i].pozicia() == p)
        return false;
  return true;
}

int ktoJeNaPolicku(const Stav& stav, const Bod& p){
  for(int i = 0; i < (int)stav.hraci.size(); i++) if(stav.hraci[i].jeZivy){
    if(stav.hraci[i].pozicia() == p) return i;
  }
  return -1;
}

Bomba vytvorBombu(const int id, const int kto, const int sila, const int timer, const Bod& poloha){
  Bomba bomba = Bomba();
  bomba.id = id;
  bomba.kto = kto;
  bomba.sila = sila;
  bomba.timer = timer;
  bomba.x = poloha.x;
  bomba.y = poloha.y;
  
  bomba.isFrozen = false;

  return bomba;
}

Bomba vytvorBombu(Stav& stav, const int i){
  Hrac& hrac = stav.hraci[i];
  return vytvorBombu(stav.dalsiId++, i, hrac.silaBomb, kBombaTimer, hrac.pozicia());
}

Bonus vytvorBonus(const int id, const Bod& poloha){
  Bonus bonus = Bonus();
  bonus.id = id;
  bonus.x = poloha.x;
  bonus.y = poloha.y;
  
  int pokus = rand() % 100;
  for(int i = 0; i < BONUS_POCET_TYPOV; i++){
    if(pokus < kBonusSanca[i]){
      bonus.typ = i;
      break;
    }
    else{
      pokus -= kBonusSanca[i];
    }
  }

  return bonus;
}

void aktivujBonus(Stav &stav, Hrac &hrac, const Bonus& bonus, 
    map<Bod, Bonus>& bonusyPodlaPolohy, map<Bod, Bomba>& bombyPodlaPolohy){
  if(bonus.typ == BONUS_SILA){
    hrac.silaBomb += 1;
  }
  else if(bonus.typ == BONUS_STIT){
    hrac.maStit = kStitTrvanie + 1; // pozor, to sa nestackuje (+1 lebo na konci kola sa to odrata)
  }
  else if(bonus.typ == BONUS_POCET){
    hrac.maxPocetBomb += 1;
  }
  else if(bonus.typ == BONUS_OHNOSTROJ){
    // buahahahaha!!!
    int ostavaPokusov = kOhnostrojPocetPokusov;
    int ostavaBomb = kOhnostrojPocet;

    while(ostavaPokusov-- > 0 && ostavaBomb > 0){
      const int x = rand() % stav.teren.w();
      const int y = rand() % stav.teren.h();

      if(stav.teren.get(x, y) == MAPA_VOLNO){ // ano, moze vzniknut aj u hraca pod nohami
        stav.teren.set(x, y, MAPA_BOMBA);
        bombyPodlaPolohy[Bod(x, y)] = vytvorBombu(stav.dalsiId++, kOhnostrojKto, kOhnostrojSila, 
            kOhnostrojStart + (kOhnostrojPocet - ostavaBomb) + 1, Bod(x, y));
        ostavaBomb--;
      }
    }
  }
  else if(bonus.typ == BONUS_VIANOCE){
    // Vianoce nie su rekurzivne, nemoze vygnerovat dalsi bonus Vianoce
    // bonus nemoze vzniknut u hraca pod nohami, robilo by to bordel
    int ostavaPokusov = kVianocePocetPokusov;
    int ostavaBonusov = kVianocePocet;

    while(ostavaPokusov-- > 0 && ostavaBonusov){
      const int x = rand() % stav.teren.w();
      const int y = rand() % stav.teren.h();

      if(stav.teren.get(Bod(x, y)) == MAPA_VOLNO && !jeTuHrac(stav, Bod(x, y))){
        Bonus bonus = vytvorBonus(stav.dalsiId++, Bod(x, y));

        if(bonus.typ == BONUS_VIANOCE) continue;
        bonusyPodlaPolohy[Bod(x, y)] = bonus;
        stav.teren.set(Bod(x, y), MAPA_BONUS);

        ostavaBonusov--;
      }
    }
  }
  else if(bonus.typ == BONUS_FREEZE){
    // vsetky bomby dostanu +konstantu ku timerom
    FOREACH(it, bombyPodlaPolohy){
      it->second.timer += kFreezeInkrement;
      it->second.isFrozen = true;
    }
  }
}

int cisloHracaPodlaPolohy(const Stav& stav, const Bod& poloha){
  for(int i = 0; i < (int)stav.hraci.size(); i++){
    if(stav.hraci[i].jeZivy && stav.hraci[i].pozicia() == poloha) return i;
  }
  return -1;
}

bool jeTuHrac(Stav& stav, const Bod& p){
  return cisloHracaPodlaPolohy(stav, p) > -1;
}

void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  OBSERVE("odsimulujKolo.zacina", stav.cas, stav.cas + 1);
   
  // vykonaju sa prikazy
  //     poradie tahania je urcene nahodne (priorita, ktora sa meni kazde kolo)
  // buchnu bomby, ktore maju timer 0
  //     zarataju sa body
  // znizia sa timery bomb a bonusov
  
  // predpocitavania
  // ====================
  map<Bod, Bonus> bonusyPodlaPolohy;
  FOREACH(it, stav.bonusy) bonusyPodlaPolohy[it->pozicia()] = *it;
  // od tejto chvili sa stav.bonusy nepouziva

  map<Bod, Bomba> bombyPodlaPolohy;
  FOREACH(it, stav.bomby) bombyPodlaPolohy[it->pozicia()] = *it;
  // od tejto chvili sa stav.bomby nepouziva
  
  // vykonavanie prikazov
  // =====================

  // urci sa priorita
  vector<int> priorita(mapa.pocetHracov);
  for(int i = 0; i < mapa.pocetHracov; i++) priorita[i] = i;
  random_shuffle(priorita.begin(), priorita.end());
  
  // polozia bomby
  for(int _prior = 0; _prior < mapa.pocetHracov; _prior++){
    const int i = priorita[_prior];
    // mrtvi netahaju
    if(! stav.hraci[i].jeZivy) continue;
  
    Hrac &hrac = stav.hraci[i];
    const Prikaz& prikaz = akcie[i];

    if(prikaz.kladiem && hrac.pocetBomb < hrac.maxPocetBomb && stav.teren.get(hrac.pozicia()) == MAPA_VOLNO){
      stav.teren.set(hrac.pozicia(), MAPA_BOMBA);
      bombyPodlaPolohy[hrac.pozicia()] = vytvorBombu(stav, i);
      hrac.pocetBomb++;
      OBSERVE("Kladiem", i, hrac.x, hrac.y);
    }
  }

  // tahaju hraci
  for(int _prior = 0; _prior < mapa.pocetHracov; _prior++){
    const int i = priorita[_prior];
    // mrtvi netahaju
    if(! stav.hraci[i].jeZivy) continue;

    Hrac& hrac = stav.hraci[i];
    const Prikaz& prikaz = akcie[i];

    if(prikaz.smer == SMER_NIKAM) continue;
    if(! (0 <= prikaz.smer && prikaz.smer <= 3)) continue; // zly smer
    
    const Bod kam = hrac.pozicia() + Bod(DX[prikaz.smer], DY[prikaz.smer]);
    if(! priechodne(stav.teren.get(kam))) continue; // policko nie je prechodne
    if(jeTuHrac(stav, kam)) continue; // na danom policku je iny hrac

    // no, konecne mozeme ist
    hrac.x = kam.x;
    hrac.y = kam.y;
    OBSERVE("Idem", i, DX[prikaz.smer], DY[prikaz.smer]);
    
    if(bonusyPodlaPolohy.find(kam) != bonusyPodlaPolohy.end()){
      Bonus bonus = bonusyPodlaPolohy[kam];
      bonusyPodlaPolohy.erase(kam);
			stav.teren.set(kam, MAPA_VOLNO);
      aktivujBonus(stav, hrac, bonus, bonusyPodlaPolohy, bombyPodlaPolohy);
    }
  }

  // idu vybuchovat bomby
  // =======================

  // ako to cele funguje:
  // najprv s pozrieme, 
  //    ktore policka maju byt v ohni, 
  //    ktore bomby maju vybuchnut, 
  //    ktore policka s hlinou maju zaniknut
  //    ktori hraci maju zomriet (anuluju sa im stity, ak mali)
  // nasledne sa pozrieme, kto za co dostane body
  //    body sa prideluju majitelovi bomby, ktora priamo zasahuje objekt znicenia
  //    ak su to viacere bomby, tak su to viaceri hraci. Za kazde trafenie bombou su samostatne body 
  //        (combo, ak trafis ho dvomi bombami v rovnakom case :D)
  // nasledne zomru hraci
  // nasledne zaniknu policka s hlinou, vygeneruju sa bonusy
  // nasledne sa odprataju vybuchnute bomby, znizia sa patricne countery u hracov
  
  queue<Bod> Q; // tam su iba bomby
  FOREACH(it, bombyPodlaPolohy) if(it->second.timer == 0){
    Q.push(it->first);
  }
  
  set<Bod> polickaVOhni; // vsetko, co ma horiet
  map<Bod, vector<int> > hlinyCoMajuHoriet; // a indexy hracov, bomby ktorych ich zasahuju, aj viackrat
  map<int, vector<int> > hraciCoMajuHoriet;
  set<Bod> polohyBombCoVybuchnu;
  
  while(! Q.empty()){
    const Bod poloha = Q.front(); Q.pop();
    if(polohyBombCoVybuchnu.find(poloha) != polohyBombCoVybuchnu.end()) continue;
    polohyBombCoVybuchnu.insert(poloha);
    Bomba bomba = bombyPodlaPolohy[poloha];
    
    polickaVOhni.insert(poloha);
    {
      const int obet = cisloHracaPodlaPolohy(stav, poloha);
      if(obet > -1){
        hraciCoMajuHoriet[obet].push_back(bomba.kto);
      }
    }

    for(int smer = 0; smer < 4; smer++){
      for(int r = 1; r < bomba.sila; r++){
        const Bod p = poloha + Bod(DX[smer] * r, DY[smer] * r);
        const int typ = stav.teren.get(p);
        
        const int obet = cisloHracaPodlaPolohy(stav, p);
        if(obet > -1) hraciCoMajuHoriet[obet].push_back(bomba.kto);

        if(priechodne(typ)){
          polickaVOhni.insert(p);
        }
        else if(typ == MAPA_KAMEN || typ == MAPA_OKRAJ){
          break;
        }
        else if(typ == MAPA_HLINA){
          polickaVOhni.insert(p);
          hlinyCoMajuHoriet[p].push_back(bomba.kto);
          break;
        }
        else if(typ == MAPA_BOMBA){
          polickaVOhni.insert(p);
          Q.push(p);
        }
      }
    }
  }

  // bodovanie + upratovanie
  FOREACH(it, polickaVOhni){
    bonusyPodlaPolohy.erase(*it);
		stav.teren.set(*it, MAPA_VOLNO);
    OBSERVE("Hori", it->x, it->y);
	}
	
	FOREACH(it, hlinyCoMajuHoriet){
    FOREACH(itt, it->second) if(*itt > -1){
      stav.hraci[*itt].skore += kBodyZaHlinu;
    }
    stav.teren.set(it->first, MAPA_VOLNO);
    
    // vygenerujeme bonus
    if(rand()%100 < kBonusZakladnaSanca){
      stav.teren.set(it->first, MAPA_BONUS);
      bonusyPodlaPolohy[it->first] = vytvorBonus(stav.dalsiId++, it->first);
    }
  }

  FOREACH(it, hraciCoMajuHoriet){
    if(stav.hraci[it->first].maStit > 0){
      stav.hraci[it->first].maStit = 0;
      continue;
    }
    FOREACH(itt, it->second) if(*itt > -1){
      if(it->first != *itt) stav.hraci[*itt].skore += kBodyZaZabitie;
      else  stav.hraci[*itt].skore += kBodyZaSamovrazdu;
    }
    OBSERVE("SomZomrel", stav.hraci[it->first].x, stav.hraci[it->first].y);
    stav.hraci[it->first].jeZivy = false;
  }

  FOREACH(it, polohyBombCoVybuchnu){
    Bomba bomba = bombyPodlaPolohy[*it];
    if(bomba.kto > -1){
      stav.hraci[bomba.kto].pocetBomb--;
    }
    stav.teren.set(*it, MAPA_VOLNO);
    bombyPodlaPolohy.erase(*it);
  }

  // prehodenie veci naspat do stavu
  stav.bomby.clear();
  FOREACH(it, bombyPodlaPolohy){
    stav.bomby.push_back(it->second);
  }

  stav.bonusy.clear();
  FOREACH(it, bonusyPodlaPolohy){
    stav.bonusy.push_back(it->second);
  }

  // znizenie timerov
  FOREACH(it, stav.hraci) if(it->jeZivy){
    it->maStit = max(0, it->maStit - 1);
  }
  
  FOREACH(it, stav.bomby){
    it->timer--;
  }
				
  {  
    vector<int> zijuci = ktoriZiju(mapa, stav);
    if(zijuci.size() == 1) stav.hraci[zijuci[0]].skore += kBodyZaPrezitie;
	}

  OBSERVE("odsimulujKolo.konci", stav.cas, stav.cas + 1);
  stav.cas++;
}

void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, Stav& novy) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;
  novy = stav;
  novy.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]] = stav.hraci[i];
  }
  for(int i = 0; i < mapa.pocetHracov; i++){
    novy.hraci[i].mapovanie.clear();
  }
}

void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  // this function intentionally left blank
}

vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav) {
  vector<int> zijuci;
  for(int i = 0; i < mapa.pocetHracov; i++) if(stav.hraci[i].jeZivy){
    zijuci.push_back(i);
  }
  return zijuci;
}

bool hraSkoncila(const Mapa& mapa, const Stav& stav) {
  int pocetZivychHracov = 0;

  FOREACH(it, stav.hraci) if(it->jeZivy) pocetZivychHracov++;

  return pocetZivychHracov <= 1 || stav.cas >= kMaximalnaDlzkaHry;
}

vector<int> zistiSkore(const Mapa& mapa, const Stav& stav) {
  vector<int> bla;
  for(int i = 0; i < mapa.pocetHracov; i++) bla.push_back(stav.hraci[i].skore);
  //vector<int> zijuci = ktoriZiju(mapa, stav);
  //if(zijuci.size() == 1) bla[zijuci[0]] += kBodyZaPrezitie;
  for(int i = 0; i < mapa.pocetHracov; i++) bla[i] = max(bla[i], 0);
  return bla;
}
