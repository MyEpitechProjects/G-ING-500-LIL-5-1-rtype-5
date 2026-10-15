# Guide d’utilisation du réseau

Ce guide décrit le module présent dans `network/`, partagé entre le client et le serveur. Il transporte des paquets en **UDP IPv4**. Il ne dépend ni du moteur de jeu ni de la bibliothèque graphique.

## 1. Récupérer et compiler le module

Dans ce dépôt, utilisez directement `network/include/` et `network/src/`. Pour intégrer le module dans un autre projet, copiez ces deux dossiers ensemble en conservant leur organisation.

Prérequis : un compilateur C++20, les headers d’Asio standalone (`<asio.hpp>`, pas Boost.Asio) et le support des threads. Le module utilise notamment `std::bit_cast`, ce qui nécessite C++20.

Ajoutez le chemin des headers et ces **quatre sources** à votre cible :

```makefile
CPPFLAGS += -I../network/include
CXXFLAGS += -std=c++20 -pthread
LDFLAGS += -pthread

SRC += ../network/src/NetworkHandler.cpp \
       ../network/src/PacketCodec.cpp \
       ../network/src/Paquet.cpp \
       ../network/src/WakeSignal.cpp
```

Ces chemins correspondent à un Makefile placé dans `client/` ou `server/`. Les sources doivent être ajoutées avant le calcul de `OBJ`, et `$(LDFLAGS)` utilisé à la liaison. Conservez les autres sources et bibliothèques de votre application.

Le Makefile serveur référence déjà ces sources. Celui du client possède le chemin des headers, mais il faut encore lui ajouter les quatre sources pour utiliser le réseau.

Depuis la racine, un programme autonome `example.cpp` utilisant le module se compile ainsi :

```sh
g++ -std=c++20 -Wall -Wextra -Werror -pthread \
    -Inetwork/include example.cpp \
    network/src/NetworkHandler.cpp network/src/PacketCodec.cpp \
    network/src/Paquet.cpp network/src/WakeSignal.cpp \
    -o example
```

## 2. Organisation et API

| Élément | Rôle |
| --- | --- |
| `NetworkHandler.hpp` | Socket, boucle Asio, files d’envoi et de réception |
| `Paquet.hpp` | Types concrets des paquets et constantes du protocole |
| `IPacket.hpp` | Interface polymorphe avec `serialize()` |
| `PacketCodec.hpp` | Encodage et décodage binaire |
| `WakeSignal.hpp` | Réveil facultatif d’une boucle consommatrice |

Incluez `NetworkHandler.hpp` pour utiliser le module ; il inclut les types de paquets nécessaires.

```cpp
Net::NetworkHandler network(portLocal, wakeSignal);
```

Le constructeur ouvre et attache immédiatement la socket sur toutes les interfaces IPv4. Le port est le **port local**, pas celui du serveur distant. Utilisez un port fixe côté serveur, et `0` côté client pour laisser le système choisir un port disponible. Une erreur d’ouverture ou un port déjà occupé peut faire lever une exception au constructeur.

| Méthode | Utilisation |
| --- | --- |
| `run()` | Démarre la réception puis exécute la boucle Asio ; appel bloquant dans un thread dédié |
| `send(packet, destination)` | Transfère un paquet dans la file d’envoi ; l’envoi réel est exécuté par la boucle Asio |
| `popPacket()` | Retire un paquet reçu, ou renvoie `std::nullopt` immédiatement si la file est vide |
| `stop()` | Arrête la boucle Asio ; ne joint pas le thread et ne réveille pas `WakeSignal` |
| `receive()` | Arme une réception ; déjà appelée par `run()`, ne pas l’appeler en plus |

Utilisez un seul thread exécutant `run()` par handler. `send()` et `popPacket()` protègent leurs files par mutex et peuvent être appelées depuis le thread applicatif. Le handler doit rester vivant jusqu’à la fin du thread réseau.

Une réception fournit :

```cpp
Net::IncomingPacket<Net::PacketCodec::Packet>
// .sender : asio::ip::udp::endpoint (IP et port source)
// .packet : std::unique_ptr<IPacket>
```

## 3. Intégration côté client

Les extraits suivants se placent dans le cycle de vie du client : initialisation, boucle graphique, puis fermeture.

### Démarrer et envoyer HELLO

```cpp
#include "NetworkHandler.hpp"
#include <atomic>
#include <thread>

Net::NetworkHandler network(0, nullptr);
asio::ip::udp::endpoint server(
    asio::ip::make_address("127.0.0.1"), 4242
);

std::atomic<bool> networkFailed{false};
std::thread networkThread([&] {
    try {
        network.run();
    } catch (const std::exception& error) {
        std::cerr << "Erreur réseau : " << error.what() << '\n';
        networkFailed.store(true);
        network.stop();
    }
});

network.send(std::make_unique<Net::HelloPacket>(), server);
```

Remplacez l’adresse et le port par ceux du serveur. `127.0.0.1` désigne la machine locale. `make_address` attend une adresse numérique ; pour un nom DNS, utilisez un `asio::ip::udp::resolver` et sélectionnez un endpoint IPv4.

UDP n’établit pas de connexion : HELLO/WELCOME est un échange applicatif. Le handler n’effectue aucune relance automatique si un paquet est perdu.

### Consommer les paquets à chaque frame

Déclarez cet état avant la boucle graphique :

```cpp
bool welcomed = false;
std::uint32_t sessionId = 0;
std::uint32_t playerEntityId = 0;
```

À chaque frame, videz la file sans bloquer le rendu :

```cpp
while (auto incoming = network.popPacket()) {
    if (incoming->sender != server)
        continue;

    if (auto* welcome = dynamic_cast<Net::WelcomePacket*>(incoming->packet.get())) {
        sessionId = welcome->header.sessionId;
        playerEntityId = welcome->playerEntityId;
        welcomed = true;
    } else if (auto* snapshot = dynamic_cast<Net::SnapshotPacket*>(incoming->packet.get())) {
        for (const auto& entity : snapshot->entities) {
            // Mettre à jour l’objet graphique identifié par entity.id :
            // entity.x, entity.y, entity.kind, entity.health, entity.flags.
        }
    }
}
```

Les pointeurs obtenus par `dynamic_cast` appartiennent à `incoming->packet`. Ne les conservez pas après destruction de ce paquet : copiez les données nécessaires au rendu. Le filtre d’endpoint compare IP et port ; ce n’est pas une authentification.

### Envoyer les commandes

Après réception de WELCOME, construisez un nouveau paquet pour chaque envoi :

```cpp
if (welcomed) {
    auto input = std::make_unique<Net::InputPacket>();
    input->header.sessionId = sessionId;
    input->buttons = Net::InputButton::UP | Net::InputButton::RIGHT;
    network.send(std::move(input), server);
}
```

`buttons = 0` signifie qu’aucune direction n’est active. Pour tester une direction à la réception :

```cpp
bool up = (inputPacket.buttons & Net::InputButton::UP) != 0;
```

`|` combine les boutons ; `&` teste leur présence. Le protocole actuel accepte seulement `UP`, `DOWN`, `LEFT`, `RIGHT` : aucun bit de tir n’est défini.

### Fermer le client

À la fermeture de la fenêtre, ou si `networkFailed` est vrai :

```cpp
network.stop();
if (networkThread.joinable())
    networkThread.join();
```

Effectuez aussi ce nettoyage si le code applicatif lève une exception après le démarrage du thread. Détruire un `std::thread` encore joignable termine le programme. Le destructeur du handler ne réalise pas ce nettoyage pour vous.

## 4. Intégration côté serveur avec WakeSignal

Un serveur peut dormir en attendant du travail. L’exemple ci-dessous est une boucle réseau minimale : il attribue un identifiant de session à chaque endpoint et répond aux HELLO. L’intégration du jeu et le déclenchement de l’arrêt restent à brancher dans l’application.

```cpp
#include "NetworkHandler.hpp"
#include <atomic>
#include <thread>
#include <unordered_map>

void runNetworkServer(std::atomic<bool>& running,
                      const std::shared_ptr<WakeSignal>& wake) {
    Net::NetworkHandler network(4242, wake);
    std::unordered_map<asio::ip::udp::endpoint, std::uint32_t> sessions;
    std::uint32_t nextId = 1;

    std::thread networkThread([&] {
        try {
            network.run();
        } catch (const std::exception& error) {
            std::cerr << "Erreur réseau : " << error.what() << '\n';
        }
        running.store(false);
        wake->notify();
    });

    try {
        while (running.load()) {
            wake->wait();
            if (!running.load())
                break;

            while (auto incoming = network.popPacket()) {
                if (dynamic_cast<Net::HelloPacket*>(incoming->packet.get())) {
                    auto [it, inserted] = sessions.try_emplace(incoming->sender, nextId);
                    if (inserted)
                        ++nextId;
                    auto welcome = std::make_unique<Net::WelcomePacket>();
                    welcome->header.sessionId = it->second;
                    // Pour cet exemple, même identifiant pour session et joueur.
                    // Avec le jeu, utiliser l’identifiant de l’entité réellement créée.
                    welcome->playerEntityId = it->second;
                    network.send(std::move(welcome), incoming->sender);
                } else if (auto* input = dynamic_cast<Net::InputPacket*>(incoming->packet.get())) {
                    auto it = sessions.find(incoming->sender);
                    if (it == sessions.end() || input->header.sessionId != it->second)
                        continue;
                    const bool up = (input->buttons & Net::InputButton::UP) != 0;
                    (void)up; // Transmettre les commandes à la file d’entrée du jeu.
                }
            }
            // Vider ici les autres files associées au même WakeSignal,
            // par exemple les snapshots produits par le thread de jeu.
        }
    } catch (...) {
        network.stop();
        networkThread.join();
        throw;
    }
    network.stop();
    networkThread.join();
}
```

L’appelant crée `std::atomic<bool> running{true}` et `auto wake = std::make_shared<WakeSignal>()`. Un autre thread peut demander l’arrêt avec `running.store(false)` puis `wake->notify()`. Ne faites pas cet appel à `notify()` directement depuis un gestionnaire de signal POSIX : il utilise un mutex.

### Diffuser un snapshot

Construisez les données, puis allouez un paquet par destinataire :

```cpp
Net::SnapshotPacket snapshot;
snapshot.serverTick = 42;
snapshot.entities.push_back(Net::EntityState{
    1, Net::EntityKind::PLAYER, 100.0f, 200.0f, 100, Net::EntityFlag::ALIVE
});

for (const auto& [endpoint, sessionId] : sessions) {
    auto packet = std::make_unique<Net::SnapshotPacket>(snapshot);
    packet->header.sessionId = sessionId;
    network.send(std::move(packet), endpoint);
}
```

Un `unique_ptr` ne se copie pas. Après `send(std::move(packet), ...)`, il est nul : ne le réutilisez pas pour un autre client. Ne passez jamais un paquet nul à `send()` : l’implémentation actuelle le déréférence sans vérification lors de la sérialisation.

## 5. Bien utiliser WakeSignal

`WakeSignal` est une sonnette entre producteurs de travail et consommateur. Il ne contient ni paquet ni résultat : les données restent dans leurs files respectives.

- `notify()` mémorise un réveil (`_pending = true`) et réveille un thread en attente.
- `wait()` bloque jusqu’à ce qu’un réveil soit disponible, puis le consomme (`_pending = false`).
- Une notification reçue avant `wait()` reste mémorisée : le prochain appel ne bloque pas.
- Plusieurs notifications peuvent se regrouper en un seul réveil. Ce n’est pas un compteur de paquets.

Le handler notifie automatiquement après avoir ajouté un paquet **valide** à la file de réception. Un paquet invalide est rejeté sans notification. `send()` n’utilise pas ce signal pour déclencher l’envoi : il utilise `asio::post`.

Après chaque réveil, videz **toutes** les files concernées. Si réseau et jeu partagent le même `shared_ptr<WakeSignal>`, le consommateur doit vérifier à la fois la file réseau et la file de sortie du jeu. Chaque producteur doit déposer ses données sous la protection appropriée, puis appeler `notify()`.

Prévoyez un seul consommateur par signal : `notify_one()` ne diffuse pas à tous les threads. Le signal ne remplace pas les mutex des files et ne rend pas les autres objets automatiquement thread-safe.

Dans une boucle graphique, passez `nullptr` au handler et appelez `popPacket()` à chaque frame. Un `wait()` sur le thread de rendu figerait l’affichage en l’absence de trafic. L’API actuelle ne propose pas d’attente avec délai.

Pour arrêter un consommateur bloqué, changez son indicateur d’arrêt puis appelez `notify()`. `network.stop()` seul ne débloque pas `wake->wait()`.

## 6. Paquets et limites du protocole

Le codec gère la sérialisation : inutile de fabriquer les octets lors d’un usage normal de `send()` et `popPacket()`.

| Paquet | Sens habituel | Données |
| --- | --- | --- |
| `HelloPacket` | Client → serveur | En-tête seul |
| `WelcomePacket` | Serveur → client | En-tête et `playerEntityId` |
| `InputPacket` | Client → serveur | En-tête et masque `buttons` |
| `SnapshotPacket` | Serveur → client | En-tête, `serverTick`, liste `entities` |

Chaque en-tête contient `type`, `version`, `sequence` et `sessionId`. Les constructeurs par défaut des paquets concrets initialisent le bon type et la version. Si vous fournissez un `PacketHeader` vous-même, initialisez explicitement son `type`.

`sequence` et `sessionId` valent zéro par défaut. Le handler ne les attribue pas, ne les incrémente pas et ne valide pas leur signification applicative. La gestion des sessions, la sélection des snapshots récents et le traitement des doublons appartiennent au client/serveur.

Le format utilise des entiers multi-octets en ordre réseau (big-endian). Les positions sont des flottants encodés sur 32 bits. La limite est de **1 200 octets par paquet**, soit **74 entités par snapshot** (14 octets de base et 16 par entité). Le codec ne découpe pas les snapshots plus grands et ne les réassemble pas.

Le décodage rejette notamment les tailles incorrectes, les versions/types inconnus, les boutons non autorisés, les types d’entité inconnus et les positions non finies. `deserialize()` renvoie alors `nullptr` ; le handler affiche `invalid packet` et poursuit la réception.

La sérialisation peut lever `std::invalid_argument` pour des données invalides. Elle a lieu dans le thread exécutant `run()`, pas nécessairement dans celui qui appelle `send()` : placez un `try/catch` autour de `run()`.

Voir aussi [la documentation du protocole](protocol.md) ; les définitions effectivement utilisées se trouvent dans [Paquet.hpp](../network/include/Paquet.hpp) et [PacketCodec.cpp](../network/src/PacketCodec.cpp).

## 7. État actuel et points d’intégration

Le module est un transport UDP : pas de garantie de livraison, d’ordre, d’absence de doublons, de reconnexion, de timeout de session, de chiffrement ou d’authentification intégrés. `send()` ne confirme pas la réception par le destinataire.

La version actuelle conserve les octets d’envoi dans un `shared_ptr` capturé par le callback Asio : leur mémoire reste valide jusqu’à la fin de l’opération. Préservez cette durée de vie si vous modifiez le transport.

## 8. Vérifier une intégration

1. Compiler les deux exécutables avec les quatre sources réseau.
2. Lancer le serveur sur le port choisi, puis le client vers cette IP et ce port.
3. Vérifier la réception de HELLO puis de WELCOME et les identifiants échangés.
4. Envoyer INPUT et vérifier le masque de directions côté serveur.
5. Envoyer un snapshot avec une entité et vérifier ses valeurs côté client.
6. Connecter un deuxième client pour vérifier la diffusion sans réutilisation d’un `unique_ptr` déplacé.
7. Fermer sans trafic réseau pour vérifier que les attentes sont réveillées et les threads joints.
