# SEC2-TP1 – Lexique (C++)

Voir [RAPPORT.md](RAPPORT.md).

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure   # jeux d'essais
cmake --build build --target run             # démonstration sur data/
```

Les exécutables sont `build/tp1` (argument : dossier des textes, `data` par
défaut) et `build/tests/tests_tp1` (argument : dossier `tests`). Sous Windows /
Visual Studio, ils sont dans un sous-dossier `Release` ou `Debug`.
