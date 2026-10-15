# R-Type

R-Type est un projet de jeu de tir spatial multijoueur inspiré du jeu d’arcade du même nom. Les joueurs pilotent un vaisseau et affrontent des ennemis dans une partie partagée en réseau.

Le projet comprend un serveur qui gère la partie, une logique de jeu pour les joueurs, ennemis et projectiles, ainsi qu’un module réseau commun destiné aux échanges entre le serveur et les clients. Le client a pour rôle d’afficher la partie et de transmettre les commandes du joueur.

## Build

### Prérequis

- CMake 3.20 ou supérieur.
- Un compilateur compatible C++20, comme GCC ou Clang.
- Make ou Ninja.
- Les headers d’Asio standalone (`asio.hpp`).

Toutes les commandes ci-dessous sont à exécuter depuis la racine du dépôt.

La configuration CMake actuelle construit le serveur et ses bibliothèques réseau et jeu. Elle ne définit pas encore de cible pour le client.

### Configurer et compiler

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

La première commande configure le projet dans le dossier `build/`. La seconde compile le serveur et ses dépendances. Les fichiers générés restent dans ce dossier.

Pour compiler uniquement la cible serveur et ses dépendances :

```sh
cmake --build build --target rtype_server -j
```

Après une modification du code, relancer la commande de compilation suffit.

### Build de débogage

Pour générer un exécutable avec les symboles de débogage, utilisez un dossier distinct :

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

### Recompiler entièrement

```sh
cmake --build build --clean-first -j
```

### Lancer le serveur

```sh
./build/rtype_server
```

Le serveur écoute actuellement sur le port UDP **4242**. Pour la version de débogage, utilisez `./build-debug/rtype_server`.

## Documentation

- [Guide d’utilisation du réseau](docs/network.md)
- [Protocole réseau](docs/protocol.md)
