#include "lexique_ligne.hpp"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <utility>

LexiqueLigne::LexiqueLigne(std::string nom) : Lexique(std::move(nom)) {}

LexiqueLigne::LexiqueLigne(std::string nom, const std::string& fichier)
    : Lexique(std::move(nom)) {
    // Appelé ici (et non dans le constructeur de Lexique) pour que l'appel
    // virtuel à enregistrer() atteigne bien la version de LexiqueLigne.
    charger(fichier);
}

void LexiqueLigne::enregistrer(const std::string& mot, unsigned ligne) {
    Lexique::enregistrer(mot, ligne);
    if (ligne == 0) return; // ligne inconnue
    Lignes& l = lignes_[mot];
    if (l.empty() || l.back() < ligne) {
        l.push_back(ligne);
    } else if (l.back() != ligne) {
        // ajout manuel hors ordre : on garde le vecteur trié et sans doublon
        auto pos = std::lower_bound(l.begin(), l.end(), ligne);
        if (*pos != ligne) l.insert(pos, ligne);
    }
}

const LexiqueLigne::Lignes& LexiqueLigne::lignes(const std::string& mot) const {
    static const Lignes aucune;
    auto it = lignes_.find(mot);
    return it == lignes_.end() ? aucune : it->second;
}

bool LexiqueLigne::supprimer(const std::string& mot) {
    lignes_.erase(mot);
    return Lexique::supprimer(mot);
}

Lexique& LexiqueLigne::operator+=(const Lexique& autre) {
    const auto* autreLL = dynamic_cast<const LexiqueLigne*>(&autre);
    if (autreLL == this) {
        // l += l : on double les occurrences, les lignes ne changent pas
        Lexique::operator+=(autre);
        return *this;
    }
    Lexique::operator+=(autre);
    if (autreLL) {
        for (const auto& [mot, l2] : autreLL->lignes_) {
            Lignes& l1 = lignes_[mot];
            Lignes fusion;
            fusion.reserve(l1.size() + l2.size());
            std::set_union(l1.begin(), l1.end(), l2.begin(), l2.end(),
                           std::back_inserter(fusion));
            l1 = std::move(fusion);
        }
    }
    return *this;
}

void LexiqueLigne::afficherLignes(std::ostream& os, const Lignes& l) {
    for (std::size_t i = 0; i < l.size(); ++i) {
        os << (i ? " " : "") << l[i];
    }
}

void LexiqueLigne::ecrireEntree(std::ostream& os, const std::string& mot,
                                unsigned occ) const {
    os << mot << ' ' << occ << " :";
    for (unsigned n : lignes(mot)) os << ' ' << n;
    os << '\n';
}

void LexiqueLigne::afficher(std::ostream& os) const {
    os << "Lexique avec lignes \"" << nom_ << "\" (" << nbMotsDifferents()
       << " mots differents, " << nbMotsTotal() << " au total)\n";
    for (const auto& [mot, occ] : mots_) {
        os << "  " << mot << " : " << occ << " [lignes ";
        afficherLignes(os, lignes(mot));
        os << "]\n";
    }
}
