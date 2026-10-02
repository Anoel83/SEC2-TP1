#ifndef LEXIQUE_HPP
#define LEXIQUE_HPP

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

// Lexique : ensemble de mots (en minuscules) associés à leur nombre
// d'occurrences.
//
// Conteneur choisi : std::map<std::string, unsigned>
//   - recherche / insertion / suppression en O(log n) ;
//   - les mots restent triés par ordre alphabétique, ce qui donne un
//     affichage et une sauvegarde lisibles sans tri supplémentaire ;
//   - le parcours ordonné permet de fusionner (+=) et de faire la
//     différence (-=) de deux lexiques en un seul parcours linéaire
//     O(n + m), à la manière de la fusion du tri fusion.
class Lexique {
public:
    using Dictionnaire = std::map<std::string, unsigned>;

    explicit Lexique(std::string nom = "lexique");
    // Construit le lexique à partir du texte contenu dans 'fichier'.
    // Lance std::runtime_error si le fichier ne peut pas être ouvert.
    Lexique(std::string nom, const std::string& fichier);
    virtual ~Lexique() = default;

    Lexique(const Lexique&) = default;
    Lexique& operator=(const Lexique&) = default;

    const std::string& nom() const { return nom_; }
    void renommer(const std::string& nom) { nom_ = nom; }

    // Lit le fichier ligne par ligne et ajoute chacun de ses mots.
    void charger(const std::string& fichier);

    // Ajoute un mot (déjà normalisé) au lexique. 'ligne' vaut 0 si inconnue.
    void ajouter(const std::string& mot, unsigned ligne = 0);

    // Sauvegarde le lexique dans un fichier texte. Retourne false en cas d'échec.
    bool sauvegarder(const std::string& fichier) const;

    // Nombre d'occurrences du mot (0 s'il est absent).
    unsigned occurrences(const std::string& mot) const;
    bool contient(const std::string& mot) const { return occurrences(mot) != 0; }

    // Supprime le mot. Retourne true s'il était présent.
    virtual bool supprimer(const std::string& mot);

    // Nombre de mots différents.
    std::size_t nbMotsDifferents() const { return mots_.size(); }
    // Nombre total de mots (somme des occurrences).
    std::size_t nbMotsTotal() const;
    bool vide() const { return mots_.empty(); }
    void vider();

    // Les 'k' mots les plus fréquents (par occurrences décroissantes).
    std::vector<std::pair<std::string, unsigned>> plusFrequents(std::size_t k) const;

    const Dictionnaire& mots() const { return mots_; }

    // Fusion : ajoute les mots de 'autre' (les occurrences s'additionnent).
    virtual Lexique& operator+=(const Lexique& autre);
    // Différence : retire les mots présents dans 'autre'.
    virtual Lexique& operator-=(const Lexique& autre);

    // Affichage (appelé par operator<<, redéfini par les classes dérivées).
    virtual void afficher(std::ostream& os) const;

    // Découpe une ligne de texte en mots normalisés (minuscules, sans
    // ponctuation). Les séquences UTF-8 de ponctuation (tirets longs,
    // guillemets typographiques, BOM...) sont traitées comme séparateurs.
    static std::vector<std::string> decouper(const std::string& ligne);

protected:
    // Point d'extension appelé pour chaque mot lu dans le fichier.
    virtual void enregistrer(const std::string& mot, unsigned ligne);
    // Écrit une entrée dans un fichier de sauvegarde.
    virtual void ecrireEntree(std::ostream& os, const std::string& mot,
                              unsigned occ) const;

    std::string nom_;
    Dictionnaire mots_;
};

std::ostream& operator<<(std::ostream& os, const Lexique& lex);

#endif // LEXIQUE_HPP
