
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>
#include <map>

#include "common.h"

extern const int DX[4];
extern const int DY[4];

extern const int kMaximalnaDlzkaHry;

extern const int kPociatocnaSilaBomb; // je to polomer (ak si vzdialeny r policok, tak si v bezpeci)
extern const int kPociatocnyMaxPocetBomb;

extern const int kBonusSanca[BONUS_POCET_TYPOV];

extern const int kBonusZakladnaSanca;

extern const int kBombaTimer;

extern const int kStitTrvanie; 

extern const int kOhnostrojKto;
extern const int kOhnostrojPocet;
extern const int kOhnostrojStart;
extern const int kOhnostrojSila;
extern const int kOhnostrojPocetPokusov;

extern const int kVianocePocet;
extern const int kVianocePocetPokusov;


extern const int kBodyZaHlinu;
extern const int kBodyZaZabitie;
extern const int kBodyZaSamovrazdu;
extern const int kBodyZaPrezitie;

void zapniObservation(std::ostream* observation);

Stav zaciatokHry(const Mapa& mapa);
void prehladajBfs(const Teren& teren, Bod start, Teren& vzdialenost);
void prehladajLokalneBfs(const Teren& teren, Bod start, int rozsahLimit, std::map<Bod,int>& vzdialenost);
void odsimulujKolo(const Mapa& mapa, Stav& stav, const std::vector<Odpoved>& akcie);
void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, const Teren& viditelne, Stav& novy);
void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved);
std::vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav);
bool hraSkoncila(const Mapa& mapa, const Stav& stav);
int zistiSkore(const Stav& stav, int hrac);

#endif
