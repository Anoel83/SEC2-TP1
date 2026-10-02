// Comparaison des conteneurs envisagés pour le lexique, sur les textes réels.
// Le texte est découpé une fois pour toutes : on ne mesure que le coût du
// conteneur (construction, recherche, fusion, différence, sortie triée).
#include "lexique.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using Mots = std::vector<std::string>;
using Paire = std::pair<std::string, unsigned>;

Mots lireMots(const std::string& fichier) {
    std::ifstream in(fichier, std::ios::binary);
    Mots mots;
    std::string ligne;
    while (std::getline(in, ligne))
        for (auto& m : Lexique::decouper(ligne)) mots.push_back(std::move(m));
    return mots;
}

template <class F>
double ms(F&& f, int repetitions = 1) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < repetitions; ++i) f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count() / repetitions;
}

unsigned long long puits = 0; // empêche l'optimiseur de supprimer les calculs

// ---------------------------------------------------------------- std::map
struct AvecMap {
    std::map<std::string, unsigned> d;
    void construire(const Mots& mots) { for (auto& m : mots) ++d[m]; }
    unsigned chercher(const std::string& m) const {
        auto it = d.find(m);
        return it == d.end() ? 0 : it->second;
    }
    void fusion(const AvecMap& o) { // parcours parallèle O(n + m)
        auto it = d.begin();
        for (auto& [m, n] : o.d) {
            while (it != d.end() && it->first < m) ++it;
            if (it != d.end() && it->first == m) it->second += n;
            else d.emplace_hint(it, m, n);
        }
    }
    void difference(const AvecMap& o) {
        auto it = d.begin();
        auto jt = o.d.begin();
        while (it != d.end() && jt != o.d.end()) {
            if (it->first < jt->first) ++it;
            else if (jt->first < it->first) ++jt;
            else { it = d.erase(it); ++jt; }
        }
    }
    std::vector<Paire> trie() const { return {d.begin(), d.end()}; }
    std::size_t taille() const { return d.size(); }
};

// ------------------------------------------------------ std::unordered_map
struct AvecUnorderedMap {
    std::unordered_map<std::string, unsigned> d;
    void construire(const Mots& mots) { for (auto& m : mots) ++d[m]; }
    unsigned chercher(const std::string& m) const {
        auto it = d.find(m);
        return it == d.end() ? 0 : it->second;
    }
    void fusion(const AvecUnorderedMap& o) { for (auto& [m, n] : o.d) d[m] += n; }
    void difference(const AvecUnorderedMap& o) { for (auto& e : o.d) d.erase(e.first); }
    std::vector<Paire> trie() const {
        std::vector<Paire> v(d.begin(), d.end());
        std::sort(v.begin(), v.end());
        return v;
    }
    std::size_t taille() const { return d.size(); }
};

// --------------------------------------------- std::vector trié (dichotomie)
struct AvecVecteurTrie {
    std::vector<Paire> d;
    static bool inf(const Paire& p, const std::string& m) { return p.first < m; }
    void construire(const Mots& mots) {
        for (auto& m : mots) {
            auto it = std::lower_bound(d.begin(), d.end(), m, inf);
            if (it != d.end() && it->first == m) ++it->second;
            else d.insert(it, {m, 1}); // décalage O(n)
        }
    }
    unsigned chercher(const std::string& m) const {
        auto it = std::lower_bound(d.begin(), d.end(), m, inf);
        return it != d.end() && it->first == m ? it->second : 0;
    }
    void fusion(const AvecVecteurTrie& o) {
        std::vector<Paire> r;
        r.reserve(d.size() + o.d.size());
        auto i = d.cbegin();
        auto j = o.d.cbegin();
        while (i != d.cend() && j != o.d.cend()) {
            if (i->first < j->first) r.push_back(*i++);
            else if (j->first < i->first) r.push_back(*j++);
            else { r.push_back({i->first, i->second + j->second}); ++i; ++j; }
        }
        r.insert(r.end(), i, d.cend());
        r.insert(r.end(), j, o.d.cend());
        d = std::move(r);
    }
    void difference(const AvecVecteurTrie& o) {
        std::vector<Paire> r;
        auto j = o.d.begin();
        for (auto& p : d) {
            while (j != o.d.end() && j->first < p.first) ++j;
            if (j == o.d.end() || j->first != p.first) r.push_back(p);
        }
        d = std::move(r);
    }
    std::vector<Paire> trie() const { return d; }
    std::size_t taille() const { return d.size(); }
};

// ---------------------------------------- std::vector non trié (recherche linéaire)
struct AvecVecteurNonTrie {
    std::vector<Paire> d;
    std::vector<Paire>::iterator trouver(const std::string& m) {
        return std::find_if(d.begin(), d.end(), [&](const Paire& p) { return p.first == m; });
    }
    void construire(const Mots& mots) {
        for (auto& m : mots) {
            auto it = trouver(m);
            if (it != d.end()) ++it->second;
            else d.push_back({m, 1});
        }
    }
    unsigned chercher(const std::string& m) const {
        auto it = std::find_if(d.begin(), d.end(), [&](const Paire& p) { return p.first == m; });
        return it == d.end() ? 0 : it->second;
    }
    void fusion(const AvecVecteurNonTrie& o) {
        for (auto& [m, n] : o.d) {
            auto it = trouver(m);
            if (it != d.end()) it->second += n;
            else d.push_back({m, n});
        }
    }
    void difference(const AvecVecteurNonTrie& o) {
        d.erase(std::remove_if(d.begin(), d.end(),
                               [&](const Paire& p) { return o.chercher(p.first) != 0; }),
                d.end());
    }
    std::vector<Paire> trie() const {
        std::vector<Paire> v = d;
        std::sort(v.begin(), v.end());
        return v;
    }
    std::size_t taille() const { return d.size(); }
};

template <class C>
void mesurer(const char* nom, const Mots& mis, const Mots& ndp, const Mots& requetes) {
    C a, b;
    double tConstr = ms([&] { a.construire(mis); });
    b.construire(ndp);

    double tRech = ms([&] {
        for (auto& m : requetes) puits += a.chercher(m);
    });
    double tFusion = ms([&] { C c = a; c.fusion(b); puits += c.taille(); }, 5)
                   - ms([&] { C c = a; puits += c.taille(); }, 5);
    double tDiff = ms([&] { C c = a; c.difference(b); puits += c.taille(); }, 5)
                 - ms([&] { C c = a; puits += c.taille(); }, 5);
    double tTri = ms([&] { puits += a.trie().size(); }, 5);

    std::cout << std::left << std::setw(22) << nom << std::right << std::fixed
              << std::setprecision(1) << std::setw(14) << tConstr << std::setw(14)
              << tRech << std::setw(12) << std::max(0.0, tFusion) << std::setw(12)
              << std::max(0.0, tDiff) << std::setw(14) << tTri << '\n';
}

} // namespace

int main(int argc, char** argv) {
    const std::string data = argc > 1 ? argv[1] : "data";
    const Mots mis = lireMots(data + "/lesMiserables_A.txt");
    const Mots ndp = lireMots(data + "/notreDameDeParis_A.txt");
    // Requêtes : tous les mots de Notre-Dame cherchés dans le lexique des Misérables
    const Mots& requetes = ndp;

    std::cout << "Les Miserables : " << mis.size() << " mots ; Notre-Dame : "
              << ndp.size() << " mots ; " << requetes.size() << " recherches\n"
              << "Temps en ms\n\n"
              << std::left << std::setw(22) << "conteneur" << std::right
              << std::setw(14) << "construction" << std::setw(14) << "recherches"
              << std::setw(12) << "fusion" << std::setw(12) << "difference"
              << std::setw(14) << "sortie triee" << '\n';

    mesurer<AvecMap>("std::map", mis, ndp, requetes);
    mesurer<AvecUnorderedMap>("std::unordered_map", mis, ndp, requetes);
    mesurer<AvecVecteurTrie>("vector trie", mis, ndp, requetes);
    if (argc > 2 && std::string(argv[2]) == "--tout")
        mesurer<AvecVecteurNonTrie>("vector non trie", mis, ndp, requetes);
    else
        std::cout << "vector non trie       (ajouter --tout : plusieurs secondes)\n";
    return puits == 42 ? 1 : 0;
}
