#include "lexique.hpp"

#include "utilitaire.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <utility>

Lexique::Lexique(std::string nom) : nom_(std::move(nom)) {}

Lexique::Lexique(std::string nom, const std::string& fichier)
    : nom_(std::move(nom)) {
    charger(fichier);
}

void Lexique::charger(const std::string& fichier) {
    std::ifstream in(fichier, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + fichier);
    }
    std::string ligne;
    unsigned numero = 0;
    while (std::getline(in, ligne)) {
        ++numero;
        for (const std::string& mot : decouper(ligne)) {
            enregistrer(mot, numero);
        }
    }
}

void Lexique::ajouter(const std::string& mot, unsigned ligne) {
    if (!mot.empty()) {
        enregistrer(mot, ligne);
    }
}

void Lexique::enregistrer(const std::string& mot, unsigned /*ligne*/) {
    ++mots_[mot];
}

namespace {

// Longueur de la séquence UTF-8 de ponctuation commençant en s[i], 0 sinon.
std::size_t ponctuationUtf8(const std::string& s, std::size_t i) {
    auto octet = [&](std::size_t k) {
        return k < s.size() ? static_cast<unsigned char>(s[k]) : 0u;
    };
    unsigned char c = octet(i);
    // U+2000..U+206F (ponctuation générale : — – ‘ ’ “ ” … etc.)
    if (c == 0xE2 && octet(i + 1) == 0x80) return 3;
    // BOM U+FEFF
    if (c == 0xEF && octet(i + 1) == 0xBB && octet(i + 2) == 0xBF) return 3;
    // U+00A0..U+00BF (espace insécable, « », ¿, ¡ ...)
    if (c == 0xC2 && octet(i + 1) >= 0xA0 && octet(i + 1) <= 0xBF) return 2;
    return 0;
}

} // namespace

std::vector<std::string> Lexique::decouper(const std::string& ligne) {
    std::vector<std::string> resultat;
    std::string courant;

    auto terminer = [&]() {
        if (courant.empty()) return;
        util::to_lower(courant);
        if (!courant.empty()) resultat.push_back(std::move(courant));
        courant.clear();
    };

    std::size_t i = 0;
    while (i < ligne.size()) {
        unsigned char c = static_cast<unsigned char>(ligne[i]);
        if (std::size_t n = ponctuationUtf8(ligne, i)) {
            terminer();
            i += n;
        } else if (std::isalnum(c) || c >= 0x80) {
            // lettre / chiffre ASCII, ou octet d'un caractère accentué
            courant += static_cast<char>(c);
            ++i;
        } else {
            // espace, ponctuation ASCII, '\r' (fichiers CRLF)...
            terminer();
            ++i;
        }
    }
    terminer();
    return resultat;
}

bool Lexique::sauvegarder(const std::string& fichier) const {
    std::ofstream out(fichier);
    if (!out) return false;
    out << "# Lexique : " << nom_ << '\n'
        << "# Mots differents : " << nbMotsDifferents() << '\n'
        << "# Mots au total : " << nbMotsTotal() << '\n';
    for (const auto& [mot, occ] : mots_) {
        ecrireEntree(out, mot, occ);
    }
    return static_cast<bool>(out);
}

void Lexique::ecrireEntree(std::ostream& os, const std::string& mot,
                           unsigned occ) const {
    os << mot << ' ' << occ << '\n';
}

unsigned Lexique::occurrences(const std::string& mot) const {
    auto it = mots_.find(mot);
    return it == mots_.end() ? 0 : it->second;
}

bool Lexique::supprimer(const std::string& mot) {
    return mots_.erase(mot) > 0;
}

std::size_t Lexique::nbMotsTotal() const {
    std::size_t total = 0;
    for (const auto& entree : mots_) total += entree.second;
    return total;
}

void Lexique::vider() {
    mots_.clear();
}

std::vector<std::pair<std::string, unsigned>>
Lexique::plusFrequents(std::size_t k) const {
    std::vector<std::pair<std::string, unsigned>> v(mots_.begin(), mots_.end());
    k = std::min(k, v.size());
    std::partial_sort(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k), v.end(),
                      [](const auto& a, const auto& b) {
                          return a.second != b.second ? a.second > b.second
                                                      : a.first < b.first;
                      });
    v.resize(k);
    return v;
}

// Fusion de deux dictionnaires triés en un seul parcours (O(n + m)).
// 'it' avance dans *this pendant que 'autre' est parcouru dans l'ordre :
//   - mot présent des deux côtés : on additionne les occurrences ;
//   - mot absent de *this : on l'insère juste avant 'it' (insertion
//     avec indice, en temps constant amorti).
Lexique& Lexique::operator+=(const Lexique& autre) {
    auto it = mots_.begin();
    for (const auto& [mot, occ] : autre.mots_) {
        while (it != mots_.end() && it->first < mot) ++it;
        if (it != mots_.end() && it->first == mot) {
            it->second += occ;
        } else {
            mots_.emplace_hint(it, mot, occ);
        }
    }
    return *this;
}

// Différence en un seul parcours (O(n + m)) : on avance simultanément dans
// les deux dictionnaires triés et on efface les mots communs.
Lexique& Lexique::operator-=(const Lexique& autre) {
    if (this == &autre) {
        vider();
        return *this;
    }
    auto it = mots_.begin();
    auto jt = autre.mots_.begin();
    while (it != mots_.end() && jt != autre.mots_.end()) {
        if (it->first < jt->first) {
            ++it;
        } else if (jt->first < it->first) {
            ++jt;
        } else {
            const std::string mot = it->first;
            ++it;
            supprimer(mot); // virtuel : la classe dérivée nettoie aussi ses données
            ++jt;
        }
    }
    return *this;
}

void Lexique::afficher(std::ostream& os) const {
    os << "Lexique \"" << nom_ << "\" (" << nbMotsDifferents()
       << " mots differents, " << nbMotsTotal() << " au total)\n";
    for (const auto& [mot, occ] : mots_) {
        os << "  " << mot << " : " << occ << '\n';
    }
}

std::ostream& operator<<(std::ostream& os, const Lexique& lex) {
    lex.afficher(os);
    return os;
}
