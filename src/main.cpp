// Programme de démonstration du TP1 : construit les lexiques des deux romans
// de Victor Hugo, les sauvegarde et illustre les différentes opérations.
#include "lexique.hpp"
#include "lexique_ligne.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <string>

namespace {

template <class F>
double chrono(F&& f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

void resume(const Lexique& l) {
    std::cout << "  " << l.nom() << " : " << l.nbMotsDifferents()
              << " mots differents, " << l.nbMotsTotal() << " mots au total\n";
}

void top(const Lexique& l, std::size_t k) {
    std::cout << "  " << k << " mots les plus frequents de " << l.nom() << " :";
    for (const auto& [mot, occ] : l.plusFrequents(k))
        std::cout << ' ' << mot << '(' << occ << ')';
    std::cout << '\n';
}

} // namespace

int main(int argc, char** argv) {
    const std::string data = argc > 1 ? argv[1] : "data";
    const std::string fMis = data + "/lesMiserables_A.txt";
    const std::string fNdp = data + "/notreDameDeParis_A.txt";

    try {
        std::cout << "=== 1. Creation des lexiques ===\n";
        Lexique mis("lesMiserables"), ndp("notreDameDeParis");
        double t1 = chrono([&] { mis.charger(fMis); });
        double t2 = chrono([&] { ndp.charger(fNdp); });
        resume(mis);
        resume(ndp);
        std::cout << "  temps de construction : " << t1 << " ms et " << t2
                  << " ms\n";
        top(mis, 10);
        top(ndp, 10);

        std::cout << "\n=== 2. Sauvegarde ===\n";
        for (const Lexique* l : {&mis, &ndp}) {
            std::string f = "lexique_" + l->nom() + ".txt";
            std::cout << "  " << f << (l->sauvegarder(f) ? " ecrit" : " ECHEC")
                      << '\n';
        }

        std::cout << "\n=== 3. Recherche / suppression ===\n";
        for (const char* mot : {"valjean", "cosette", "quasimodo", "esmeralda",
                                "paris", "zzzz"}) {
            std::cout << "  '" << mot << "' : lesMiserables=" << mis.occurrences(mot)
                      << ", notreDameDeParis=" << ndp.occurrences(mot) << '\n';
        }
        Lexique copie = mis;
        std::cout << "  suppression de 'valjean' : "
                  << (copie.supprimer("valjean") ? "ok" : "absent")
                  << ", occurrences ensuite = " << copie.occurrences("valjean")
                  << ", mots differents = " << copie.nbMotsDifferents() << '\n';

        std::cout << "\n=== 4. Fusion (+=) et difference (-=) ===\n";
        Lexique fusion = mis;
        fusion.renommer("fusion");
        double tf = chrono([&] { fusion += ndp; });
        resume(fusion);
        std::cout << "  'paris' dans la fusion : " << fusion.occurrences("paris")
                  << " (= " << mis.occurrences("paris") << " + "
                  << ndp.occurrences("paris") << ")  [" << tf << " ms]\n";

        Lexique propreMis = mis;
        propreMis.renommer("propres_lesMiserables");
        double td = chrono([&] { propreMis -= ndp; });
        resume(propreMis);
        top(propreMis, 10);
        Lexique propreNdp = ndp;
        propreNdp.renommer("propres_notreDameDeParis");
        propreNdp -= mis;
        resume(propreNdp);
        top(propreNdp, 10);
        std::cout << "  [difference : " << td << " ms]\n";
        propreNdp.sauvegarder("lexique_propres_notreDameDeParis.txt");

        std::cout << "\n=== 5. Operateur << sur un petit lexique ===\n";
        Lexique petit("extrait");
        for (const std::string& m : Lexique::decouper(
                 "The Hunchback of Notre-Dame, by Victor Hugo: the book!"))
            petit.ajouter(m);
        std::cout << petit;

        std::cout << "\n=== 6. Lexique avec numeros de ligne ===\n";
        LexiqueLigne ndpL("notreDameDeParis");
        double tl = chrono([&] { ndpL.charger(fNdp); });
        resume(ndpL);
        std::cout << "  temps de construction : " << tl << " ms\n";
        for (const char* mot : {"ananke", "quasimodo"}) {
            const auto& l = ndpL.lignes(mot);
            std::cout << "  '" << mot << "' : " << ndpL.occurrences(mot)
                      << " occurrences sur " << l.size() << " lignes";
            if (!l.empty()) {
                std::cout << " (premieres :";
                for (std::size_t i = 0; i < l.size() && i < 8; ++i)
                    std::cout << ' ' << l[i];
                std::cout << " ...)";
            }
            std::cout << '\n';
        }
        ndpL.sauvegarder("lexique_lignes_notreDameDeParis.txt");
        std::cout << "  lexique_lignes_notreDameDeParis.txt ecrit\n";

        LexiqueLigne petitL("extrait");
        petitL.ajouter("hugo", 3);
        petitL.ajouter("victor", 3);
        petitL.ajouter("hugo", 7);
        std::cout << petitL;
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << '\n';
        return 1;
    }
    return 0;
}
