# Matériel — nomenclature détaillée

Configuration retenue : STM32F405, réseau ToF 5 directions (avant/arrière/
haut/gauche/droite), liaison BLE pour le pilotage smartphone, châssis brushed
65 mm. Références réelles, vérifiées au moment de la rédaction — les prix
évoluent, à revérifier avant achat.

## Nomenclature (BOM)

| # | Composant | Référence précise | Caractéristiques clés | Poids approx. | Rôle |
|---|---|---|---|---|---|
| 1 | Microcontrôleur | **STM32F405RGT6** (LQFP64) | Cortex-M4F, 168 MHz, 1 Mo Flash, 192 Ko RAM (128 Ko SRAM + 64 Ko CCM) | ~0,2 g (puce seule) | Calcul, boucle de contrôle |
| 2 | Cristal HSE | Cristal quartz **8 MHz**, boîtier HC-49S-SMD ou 3225 | + 2× condensateurs céramique **20 pF** (charge) | <0,1 g | Horloge de référence PLL |
| 3 | Régulateur 3.3V | **MCP1700-3302E/TO** ou **AP2112K-3.3** | LDO, dropout ~200 mV, 250-600 mA selon réf. | <0,1 g | Alim MCU/capteurs/BLE depuis le 1S |
| 4 | Télémètres ToF ×5 | **Pololu #3415** — carte VL53L1X | 400 cm max, I²C, 2,6–5,5 V (régulateur+level-shifter intégrés), XSHUT+INT sortis, **13×18×2 mm**, FOV ~27° | ~0,7 g/pièce (3,5 g pour 5) | Détection avant/arrière/haut/gauche/droite |
| 5 | Centrale inertielle | Module **GY-521 (MPU-6050)** | Accel+gyro 6 axes, I²C, régulateur onboard | ~2 g | Stabilisation (boucle bas niveau à écrire) |
| 6 | Module BLE | **HM-10 (puce CC2541)**, carte nue sans base | UART AT-commands, esclave BLE 4.0, 3,3 V natif (pas de level-shifting requis avec le F405), ~27×13 mm | ~2 g | Liaison smartphone (voir plus bas) |
| 7 | Moteurs brushed ×4 | **716 coreless, ~19000 KV** (ex. Coliao/Hobbypower 7×16 mm) | Ø7×16 mm, arbre Ø0,8-1 mm, 3,7 V, ~2,8 g/pièce | ~11,2 g (4×) | Propulsion |
| 8 | Hélices ×4 (2 CW + 2 CCW) | **45 mm**, compatibles arbre 0,8-1 mm | Paire CW/CCW obligatoire (couple) | <1 g (4×) | Portance |
| 9 | MOSFET driver moteur ×4 | **AO3400A** (SOT-23, N-MOS logic-level) | Vgs(th) faible (~1-2 V), pilotable direct depuis GPIO 3.3V | négligeable | Interface PWM 3.3V → courant moteur |
| 10 | Diode roue libre ×4 | **1N4148** ou Schottky **BAT54** | Protection contre les pics inductifs moteur | négligeable | Protection MOSFET |
| 11 | Châssis | Frame **65 mm compatible moteurs 716/720** (ex. gamme "Tiny6X"/"Beta65S") | ~3-4 g, entraxe 65 mm | ~3,5 g | Structure |
| 12 | Batterie | LiPo **1S 300 mAh**, connecteur PH2.0 ou BT2.0 | 3,7-3,8 V nominal, ~63×11×6,6 mm | ~8-9 g | Alimentation |
| 13 | Connecteur programmation | Header **SWD 4 broches** (SWDIO/SWCLK/GND/3V3), pas 1,27 mm ou 2,54 mm selon encombrement | — | négligeable | Flash/debug via ST-Link |
| 14 | Condensateurs découplage | **6-8× 100 nF céramique 0402/0603** + **1× 10 µF tantale/céramique** (bulk, près du régulateur) | — | négligeable | Stabilité alimentation |
| 15 | Résistances pull-up I²C ×2 | **4,7 kΩ** (SCL, SDA) | Un seul jeu pour tout le bus (ToF×5 + IMU) | négligeable | Bus I²C |
| 16 | Résistance BOOT0 | **10 kΩ pull-down** sur BOOT0 | Démarrage depuis la Flash par défaut | négligeable | Séquence de boot |
| 17 | Fils XSHUT/I²C déportés | Fil fin **AWG32**, ~15-25 mm/capteur latéral | 5 fils par capteur latéral (VCC/GND/SDA/SCL/XSHUT) | négligeable | Cartes ToF déportées en périphérie — voir docs/pcb.md |

**Masse totale estimée (hors PCB et câblage) : ~31-33 g** — cohérent avec la
classe "tiny whoop" 65 mm (ces châssis volent typiquement entre 25 et 35 g
tout compris) ; l'ajout du 5ᵉ capteur et du module BLE reste dans cette
enveloppe.

## Liaison smartphone (BLE)

Le HM-10 est un module esclave BLE UART transparent : câblé en direct sur
USART3 du STM32 (voir brochage plus bas), il apparaît côté téléphone comme un
périphérique BLE nommé "HMSoft" par défaut (renommable en AT). **Aucune
application dédiée n'est nécessaire pour un premier essai** — une appli BLE
générique de type "Serial Bluetooth Terminal" (Android) ou "LightBlue" (iOS)
suffit pour envoyer des trames texte brutes.

Protocole de commande implémenté (`command_link.c`/`.py`, testé et validé
croisé C/Python) : une ligne ASCII `V<vx>,<vy>,<vz>*<checksum_hex>\n`, vitesses
en mm/s. **Sécurité intégrée** : si aucune trame valide n'est reçue depuis
0,5 s (lien coupé, téléphone hors de portée...), le drone revient
automatiquement en vol stationnaire plutôt que de continuer sur la dernière
commande reçue — voir `tests/test_command_link.py::test_link_falls_back_to_hover_after_timeout`.

Débit : 9600 bauds (défaut HM-10), largement suffisant pour ces trames courtes
à la fréquence où un pilote humain envoie des commandes.

## Pourquoi ces choix précis

- **STM32F405 plutôt que F411** : continuité avec la station météo (même
  famille, même toolchain déjà en main). Le F411 (LQFP48, plus petit) serait
  un vrai gain de poids/taille pour une itération future — noté mais pas
  retenu ici pour rester sur du connu.
- **VL53L1X plutôt que VL53L5CX** : le L5CX (8×8 zones) demande ~90 Ko de
  RAM/Flash pour son driver officiel — hors de portée confortable d'un F405
  partagé avec le reste du firmware. Le L1X (une zone, une distance) suffit
  pour la détection directionnelle simple visée ici.
- **5 capteurs, correction d'une erreur précédente** : la première version de
  ce document affirmait que le nombre de broches GPIO disponibles empêchait
  d'ajouter un capteur arrière, et suggérait un GPIO expander (PCF8574) comme
  contournement. En reprenant le plan de brochage complet (ci-dessous), ce
  n'était pas exact — un boîtier LQFP64 expose largement assez de broches
  libres (PA4 était tout simplement inutilisé). Corrigé : capteur arrière
  câblé en direct sur PA4, pas d'expander.
- **HM-10 plutôt qu'un module WiFi (ESP32 compagnon, etc.)** : le BLE
  consomme moins, le module est plus léger, et le protocole texte simple
  (voir plus haut) colle à l'usage visé (commandes de pilotage, pas de flux
  vidéo). Un ESP32 compagnon serait pertinent si un retour vidéo FPV devenait
  un objectif — hors périmètre ici.
- **Brushed plutôt que brushless** : plus simple (pas d'ESC/firmware BLHeli à
  gérer), pilotage PWM direct MOSFET — cohérent avec une première itération
  centrée sur la validation de l'évitement, pas la performance de vol.

## Plan de brochage (STM32F405RGT6, LQFP64)

| Fonction | Broche(s) | AF | Notes |
|---|---|---|---|
| HSE | PH0/PH1 (OSC_IN/OUT) | — | Cristal 8 MHz |
| I2C1 (ToF ×5 + IMU, bus partagé) | PB6 (SCL), PB7 (SDA) | AF4 | 100 kHz — voir docs/pcb.md pour la justification |
| XSHUT ToF avant | PA0 | GPIO sortie | |
| XSHUT ToF arrière | PA4 | GPIO sortie | |
| XSHUT ToF haut | PA1 | GPIO sortie | |
| XSHUT ToF gauche | PA2 | GPIO sortie | |
| XSHUT ToF droite | PA3 | GPIO sortie | |
| PWM moteur avant-gauche | PA6 (TIM3_CH1) | AF2 | |
| PWM moteur avant-droit | PA7 (TIM3_CH2) | AF2 | |
| PWM moteur arrière-gauche | PB0 (TIM3_CH3) | AF2 | |
| PWM moteur arrière-droit | PB1 (TIM3_CH4) | AF2 | |
| Liaison BLE (HM-10) | PB10 (USART3_TX), PB11 (USART3_RX) | AF7 | 9600 bauds |
| SWDIO / SWCLK | PA13 / PA14 | AF0 | Programmation/debug |
| BOOT0 | — | — | Pull-down 10 kΩ (démarrage Flash) |

Ce tableau correspond exactement à ce qu'implémentent `i2c_bus.c`,
`motors_pwm.c` et `uart3.c` — **aucun conflit de broches**. USART3 a été
choisi plutôt que le mapping par défaut d'USART2 (PA2/PA3) précisément parce
que PA2/PA3 sont déjà utilisées par XSHUT gauche/droite dans ce plan.

## Ce qui n'est plus "pas encore spécifié"

- **PCB** : voir **[docs/pcb.md](pcb.md)** — largeurs de piste calculées
  (IPC-2221), stack-up, placement mécanique des capteurs déportés, budget de
  capacité I²C.
- **Radio/pilotage** : voir la section liaison smartphone ci-dessus.

## Ce qui reste ouvert

- **Antenne BLE** : la portée réelle dépendra du placement (voir docs/pcb.md) —
  non mesurée, aucun prototype physique construit dans cet environnement.
- **Appli smartphone dédiée** : le protocole est documenté et testé côté
  firmware/simulation, mais aucune application mobile n'a été développée ici —
  une appli BLE générique suffit pour un premier usage (voir plus haut).
