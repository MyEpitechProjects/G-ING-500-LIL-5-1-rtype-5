# Protocole réseau R-Type — version 1

Ce document décrit le format binaire actuellement implémenté dans
[Paquet.hpp](../network/include/Paquet.hpp),
[Paquet.cpp](../network/src/Paquet.cpp) et
[PacketCodec.cpp](../network/src/PacketCodec.cpp).

## Transport et encodage

- Transport : UDP, un paquet applicatif par datagramme.
- Version : `1` (`PROTOCOL_VERSION`).
- Taille maximale acceptée : **1 200 octets**, en-tête applicatif inclus, hors en-têtes UDP/IP.
- Entiers non signés de 8, 16 ou 32 bits ; les entiers multioctets sont encodés en **big-endian** (octet de poids fort en premier).
- Aucun padding : les champs sont écrits individuellement, sans copier les structures C++ en mémoire.
- Les coordonnées sont encodées par copie des bits d'un `float` de 32 bits vers un `uint32_t`, puis écriture en big-endian.
- Les offsets ci-dessous sont exprimés en octets, à partir de zéro.

## En-tête commun — 8 octets

| Offset | Taille | Champ | Description |
| --- | --- | --- | --- |
| 0 | 1 | `type` | Type de paquet, voir tableau suivant |
| 1 | 1 | `version` | Version du protocole, obligatoirement `1` |
| 2 | 2 | `sequence` | Numéro de séquence, valeur par défaut : `0` |
| 4 | 4 | `sessionId` | Identifiant de session, valeur par défaut : `0` |

Le codec transporte `sequence` et `sessionId` sans vérifier leur progression,
leur unicité ou leur validité métier. Il n'incrémente pas automatiquement la séquence.

## Types de paquets

| Valeur | Nom | Rôle prévu | Taille totale |
| --- | --- | --- | --- |
| `1` | `INPUT` | Commandes du joueur, client → serveur | 10 octets |
| `2` | `SNAPSHOT` | État des entités, serveur → client | `14 + 16 × N` octets |
| `3` | `HELLO` | Demande de connexion, client → serveur | 8 octets |
| `4` | `WELCOME` | Attribution d'une entité joueur, serveur → client | 12 octets |

Ces directions décrivent le rôle des messages ; le codec ne contrôle pas le sens
de circulation et ne réalise pas lui-même la connexion ou la gestion des sessions.

### HELLO — type 3

Contient uniquement l'en-tête commun, sans charge utile.

### WELCOME — type 4

| Offset | Taille | Champ | Description |
| --- | --- | --- | --- |
| 8 | 4 | `playerEntityId` | Identifiant de l'entité attribuée au joueur |

### INPUT — type 1

| Offset | Taille | Champ | Description |
| --- | --- | --- | --- |
| 8 | 2 | `buttons` | Masque des directions pressées |

| Bit | Valeur | Bouton |
| --- | --- | --- |
| 0 | `0x0001` | `UP` |
| 1 | `0x0002` | `DOWN` |
| 2 | `0x0004` | `LEFT` |
| 3 | `0x0008` | `RIGHT` |

Les boutons se combinent avec un OU binaire : `UP | RIGHT = 0x0009`.
La valeur `0` signifie qu'aucune direction n'est pressée. Le codec accepte les directions opposées simultanément.

### SNAPSHOT — type 2

| Offset | Taille | Champ | Description |
| --- | --- | --- | --- |
| 8 | 4 | `serverTick` | Tick serveur associé à cet état |
| 12 | 2 | `count` | Nombre d'entités `N` |
| 14 | `16 × N` | `entities` | Entités consécutives, dans l'ordre du vecteur |

Un snapshot vide est valide et mesure 14 octets. Le nombre maximal d'entités est :

```text
MAX_SNAPSHOT_ENTITIES = (1200 - 14) / 16 = 74 (division entière)
Taille d'un snapshot de 74 entités = 1198 octets
```

Chaque entité occupe exactement **16 octets**. Les offsets suivants sont relatifs
au début de l'entité ; l'entité d'indice `i` commence à `14 + 16 × i`.

| Offset relatif | Taille | Champ | Encodage |
| --- | --- | --- | --- |
| 0 | 4 | `id` | `uint32_t` |
| 4 | 1 | `kind` | `uint8_t` : `PLAYER = 1`, `ENEMY = 2`, `PROJECTILE = 3` |
| 5 | 4 | `x` | Bits du flottant 32 bits, en big-endian |
| 9 | 4 | `y` | Bits du flottant 32 bits, en big-endian |
| 13 | 2 | `health` | `uint16_t` |
| 15 | 1 | `flags` | Masque d'état |

Les flags définis sont `ALIVE = 0x01` et `INVULNERABLE = 0x02`.
Ils peuvent être combinés. Actuellement, les autres bits de `flags` ne sont pas
rejetés par le codec. Les coordonnées doivent être finies : NaN et les infinis
sont refusés. Aucune limite de position ni contrainte métier sur `health` ou `id`
n'est appliquée par le codec.

## Validation et erreurs

`PacketCodec::deserialize()` retourne un `std::unique_ptr<IPacket>` contenant
le type concret décodé, ou `nullptr` si le paquet est invalide.

Un paquet est rejeté si :

- le pointeur fourni est nul, la taille est inférieure à 8 ou supérieure à 1 200 octets ;
- la version est différente de `1` ou le type est inconnu ;
- la taille ne correspond pas exactement à celle du type : les données tronquées et les octets supplémentaires sont refusés ;
- un `INPUT` contient des bits de boutons non définis ;
- un `SNAPSHOT` annonce plus de 74 entités ou une taille différente de `14 + 16 × count` ;
- une entité contient un type inconnu ou une coordonnée non finie.

Les méthodes `serialize() const` retournent les octets du paquet. Elles lèvent
`std::invalid_argument` si la version ou le type de l'en-tête ne correspond pas
au paquet, si les boutons sont invalides, ou si le snapshot contient trop
d'entités ou une entité invalide. Une entité invalide n'est pas ignorée :
la sérialisation du snapshot échoue.

## Exemples hexadécimaux

HELLO, version 1, séquence 0 et session 0 :

```text
03 01 00 00 00 00 00 00
```

INPUT, version 1, séquence 1, session 42, boutons UP et RIGHT :

```text
01 01 00 01 00 00 00 2A 00 09
```

WELCOME, version 1, séquence 0, session 42 et entité joueur 7 :

```text
04 01 00 00 00 00 00 2A 00 00 00 07
```

SNAPSHOT vide, version 1, séquence 2, session 42 et tick serveur 100 :

```text
02 01 00 02 00 00 00 2A 00 00 00 64 00 00
```

## Portée du protocole actuel

Le format ne définit ni acquittement, ni retransmission, ni réassemblage de
snapshots répartis sur plusieurs datagrammes. Le codec ne gère pas les pertes,
les doublons ou le réordonnancement UDP. Il ne fournit ni authentification,
ni chiffrement, ni validation de l'appartenance d'un émetteur à une session.
Ces comportements relèvent des couches réseau et applicative à compléter.
