#ifndef LEXIQUE_LIGNE_HPP
#define LEXIQUE_LIGNE_HPP

#include "lexique.hpp"

#include <map>
#include <string>
#include <vector>

// Lexique qui mémorise en plus, pour chaque mot, les numéros des lignes du
// fichier où il apparaît.
//
// Conteneur ajouté : std::map<std::string, std::vector<unsigned>>
//   - le fichier est lu de haut en bas, donc les numéros de ligne arrivent
//     dans l'ordre croissant : un simple push_back garde le vecteur trié
//     (et il suffit de comparer avec back() pour éviter les doublons quand
//     un mot apparaît plusieurs fois sur la même ligne) ;
//   - le vecteur est compact en mémoire et se fusionne en O(a + b).
class LexiqueLigne : public Lexique {
public:
    using Lignes = std::vector<unsigned>;

    explicit LexiqueLigne(std::string nom = "lexique");
    LexiqueLigne(std::string nom, const std::string& fichier);

    // Lignes où apparaît le mot (vecteur vide si absent).
    const Lignes& lignes(const std::string& mot) const;

    bool supprimer(const std::string& mot) override;

    // Fusion : si 'autre' est aussi un LexiqueLigne, ses lignes sont
    // fusionnées ; sinon seules les occurrences sont ajoutées.
    Lexique& operator+=(const Lexique& autre) override;
    // operator-= hérité : il appelle supprimer() (virtuel), qui retire aussi
    // les lignes.

    void afficher(std::ostream& os) const override;

protected:
    void enregistrer(const std::string& mot, unsigned ligne) override;
    void ecrireEntree(std::ostream& os, const std::string& mot,
                      unsigned occ) const override;

private:
    static void afficherLignes(std::ostream& os, const Lignes& l);

    std::map<std::string, Lignes> lignes_;
};

#endif // LEXIQUE_LIGNE_HPP
