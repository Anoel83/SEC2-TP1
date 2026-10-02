// Jeux d'essais automatiques (assertions) sur de petits fichiers dont le
// contenu est connu à la main.
#include "lexique.hpp"
#include "lexique_ligne.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static int echecs = 0;
static int total = 0;

#define VERIFIE(cond)                                                        \
    do {                                                                     \
        ++total;                                                             \
        if (!(cond)) {                                                       \
            ++echecs;                                                        \
            std::cerr << "ECHEC " << __FILE__ << ':' << __LINE__ << " : "    \
                      << #cond << '\n';                                      \
        }                                                                    \
    } while (0)

static std::string dossier;

static void testDecoupage() {
    auto m = Lexique::decouper("Don't stop \xe2\x80\x94 the CAT\xe2\x80\x99s toy!\r");
    std::vector<std::string> attendu{"don", "t", "stop", "the", "cat", "s", "toy"};
    VERIFIE(m == attendu);
    VERIFIE(Lexique::decouper("  ,;--  ").empty());
    VERIFIE(Lexique::decouper("\xef\xbb\xbfThe").at(0) == "the"); // BOM
}

static void testLexique() {
    Lexique l("petit1", dossier + "petit1.txt");
    VERIFIE(l.nbMotsDifferents() == 11);
    VERIFIE(l.nbMotsTotal() == 18);
    VERIFIE(l.occurrences("the") == 4);
    VERIFIE(l.occurrences("cat") == 3);
    VERIFIE(l.occurrences("dog") == 2);
    VERIFIE(l.occurrences("absent") == 0);
    VERIFIE(l.occurrences("The") == 0); // les mots sont stockés en minuscules

    VERIFIE(l.supprimer("dog"));
    VERIFIE(!l.supprimer("dog"));
    VERIFIE(!l.contient("dog"));
    VERIFIE(l.nbMotsDifferents() == 10);

    auto top = l.plusFrequents(2);
    VERIFIE(top.size() == 2 && top[0].first == "the" && top[1].first == "cat");
}

static void testFusionDifference() {
    Lexique a("petit1", dossier + "petit1.txt");
    Lexique b("petit2", dossier + "petit2.txt");
    VERIFIE(b.nbMotsDifferents() == 7);
    VERIFIE(b.nbMotsTotal() == 10);

    Lexique f = a;
    f += b;
    VERIFIE(f.nbMotsDifferents() == 14);
    VERIFIE(f.nbMotsTotal() == 28);
    VERIFIE(f.occurrences("the") == 6);
    VERIFIE(f.occurrences("cat") == 4);
    VERIFIE(f.occurrences("bird") == 2);
    VERIFIE(f.occurrences("toy") == 1);
    // b n'est pas modifié
    VERIFIE(b.nbMotsDifferents() == 7);

    Lexique d = a;
    d -= b;
    VERIFIE(d.nbMotsDifferents() == 7);
    VERIFIE(!d.contient("the") && !d.contient("cat") && !d.contient("a"));
    VERIFIE(d.occurrences("dog") == 2);
    VERIFIE(!d.contient("bird")); // mot de b absent de a : pas ajouté

    // cas limites
    Lexique vide;
    Lexique c = a;
    c += vide;
    VERIFIE(c.mots() == a.mots());
    c -= vide;
    VERIFIE(c.mots() == a.mots());
    vide += a;
    VERIFIE(vide.mots() == a.mots());
    c += c; // auto-fusion : occurrences doublées
    VERIFIE(c.occurrences("the") == 8);
    c -= c; // auto-différence : lexique vide
    VERIFIE(c.vide());

    // (a + b) - b ne contient plus aucun mot de b
    Lexique g = a;
    g += b;
    g -= b;
    for (const auto& [mot, occ] : b.mots()) VERIFIE(!g.contient(mot));
}

static void testSauvegarde() {
    Lexique l("petit2", dossier + "petit2.txt");
    const std::string fichier = "test_sauvegarde.txt";
    VERIFIE(l.sauvegarder(fichier));
    std::ifstream in(fichier);
    std::string ligne;
    std::vector<std::string> entrees;
    while (std::getline(in, ligne))
        if (!ligne.empty() && ligne[0] != '#') entrees.push_back(ligne);
    std::vector<std::string> attendu{"a 2",   "and 1", "bird 2", "cat 1",
                                     "fish 1", "sees 1", "the 2"};
    VERIFIE(entrees == attendu);
    std::remove(fichier.c_str());
    VERIFIE(!l.sauvegarder("/dossier/inexistant/x.txt"));

    bool exception = false;
    try {
        Lexique x("x", "fichier_inexistant.txt");
    } catch (const std::exception&) {
        exception = true;
    }
    VERIFIE(exception);
}

static void testAffichage() {
    Lexique l("p2", dossier + "petit2.txt");
    std::ostringstream os;
    os << l;
    VERIFIE(os.str().find("bird : 2") != std::string::npos);
}

static void testLexiqueLigne() {
    LexiqueLigne l("petit1", dossier + "petit1.txt");
    // même comptage que la classe de base
    Lexique base("petit1", dossier + "petit1.txt");
    VERIFIE(l.mots() == base.mots());

    VERIFIE((l.lignes("cat") == LexiqueLigne::Lignes{1, 2, 3}));
    VERIFIE((l.lignes("the") == LexiqueLigne::Lignes{1, 2, 3})); // pas de doublon ligne 1
    VERIFIE((l.lignes("dog") == LexiqueLigne::Lignes{1, 2}));
    VERIFIE((l.lignes("toy") == LexiqueLigne::Lignes{3}));
    VERIFIE(l.lignes("absent").empty());

    VERIFIE(l.supprimer("cat"));
    VERIFIE(l.lignes("cat").empty() && !l.contient("cat"));

    // fusion avec un autre LexiqueLigne : union triée des lignes
    LexiqueLigne a("petit1", dossier + "petit1.txt");
    LexiqueLigne b("petit2", dossier + "petit2.txt");
    a += b;
    VERIFIE(a.occurrences("cat") == 4);
    VERIFIE((a.lignes("cat") == LexiqueLigne::Lignes{1, 2, 3}));
    VERIFIE((a.lignes("the") == LexiqueLigne::Lignes{1, 2, 3}));
    VERIFIE((a.lignes("bird") == LexiqueLigne::Lignes{1, 2}));

    // différence : les lignes des mots retirés disparaissent aussi
    LexiqueLigne d("petit1", dossier + "petit1.txt");
    d -= b;
    VERIFIE(!d.contient("the") && d.lignes("the").empty());
    VERIFIE((d.lignes("dog") == LexiqueLigne::Lignes{1, 2}));

    // polymorphisme : manipulation via une référence sur la classe de base
    Lexique& ref = d;
    ref -= base;
    VERIFIE(d.vide() && d.lignes("dog").empty());

    std::ostringstream os;
    os << static_cast<const Lexique&>(b);
    VERIFIE(os.str().find("bird : 2 [lignes 1 2]") != std::string::npos);
}

int main(int argc, char** argv) {
    dossier = argc > 1 ? std::string(argv[1]) + "/" : "tests/";
    testDecoupage();
    testLexique();
    testFusionDifference();
    testSauvegarde();
    testAffichage();
    testLexiqueLigne();
    std::cout << (total - echecs) << '/' << total << " tests reussis\n";
    return echecs == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
