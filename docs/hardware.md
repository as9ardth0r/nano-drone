# Matériel — nomenclature détaillée

Configuration retenue : STM32F405, réseau ToF 4 directions (avant/haut/gauche/droite),
châssis brushed 65 mm. Références réelles, vérifiées au moment de la rédaction —
les prix évoluent, à revérifier avant achat.

## Nomenclature (BOM)

| # | Composant | Référence précise | Caractéristiques clés | Poids approx. | Rôle |
|---|---|---|---|---|---|
| 1 | Microcontrôleur | **STM32F405RGT6** (LQFP64) | Cortex-M4F, 168 MHz, 1 Mo Flash, 192 Ko RAM (128 Ko SRAM + 64 Ko CCM) | ~0,2 g (puce seule) | Calcul, boucle de contrôle |
| 2 | Cristal HSE | Cristal quartz **8 MHz**, boîtier HC-49S-SMD ou 3225 | + 2× condensateurs céramique **20 pF** (charge) | <0,1 g | Horloge de référence PLL |
| 3 | Régulateur 3.3V | **MCP1700-3302E/TO** ou **AP2112K-3.3** | LDO, dropout ~200 mV, 250-600 mA selon réf. | <0,1 g | Alim MCU/capteurs depuis le 1S |
| 4 | Télémètres ToF ×4 | **Pololu #3415** — carte VL53L1X | 400 cm max, I²C, 2,6–5,5 V (régulateur+level-shifter intégrés), XSHUT+INT sortis, **13×18×2 mm** | ~0,7 g/pièce (2,8 g pour 4) | Détection avant/haut/gauche/droite |
| 5 | Centrale inertielle | Module **GY-521 (MPU-6050)** | Accel+gyro 6 axes, I²C, régulateur onboard | ~2 g | Stabilisation (boucle bas niveau à écrire) |
| 6 | Moteurs brushed ×4 | **716 coreless, ~19000 KV** (ex. Coliao/Hobbypower 7×16 mm) | Ø7×16 mm, arbre Ø0,8-1 mm, 3,7 V, ~2,8 g/pièce | ~11,2 g (4×) | Propulsion |
| 7 | Hélices ×4 (2 CW + 2 CCW) | **45 mm**, compatibles arbre 0,8-1 mm | Paire CW/CCW obligatoire (couple) | <1 g (4×) | Portance |
| 8 | MOSFET driver moteur ×4 | **AO3400A** (SOT-23, N-MOS logic-level) | Vgs(th) faible (~1-2 V), pilotable direct depuis GPIO 3.3V | négligeable | Interface PWM 3.3V → courant moteur |
| 9 | Diode roue libre ×4 | **1N4148** ou Schottky **BAT54** | Protection contre les pics inductifs moteur | négligeable | Protection MOSFET |
| 10 | Châssis | Frame **65 mm compatible moteurs 716/720** (ex. gamme "Tiny6X"/"Beta65S") | ~3-4 g, entraxe 65 mm | ~3,5 g | Structure |
| 11 | Batterie | LiPo **1S 300 mAh**, connecteur PH2.0 ou BT2.0 | 3,7-3,8 V nominal, ~63×11×6,6 mm | ~8-9 g | Alimentation |
| 12 | Connecteur programmation | Header **SWD 4 broches** (SWDIO/SWCLK/GND/3V3), pas 1,27 mm ou 2,54 mm selon encombrement | — | négligeable | Flash/debug via ST-Link |
| 13 | Condensateurs découplage | **6-8× 100 nF céramique 0402/0603** + **1× 10 µF tantale/céramique** (bulk, près du régulateur) | — | négligeable | Stabilité alimentation |
| 14 | Résistances pull-up I²C ×2 | **4,7 kΩ** (SCL, SDA) | Un seul jeu pour tout le bus (partagé ToF+IMU) | négligeable | Bus I²C |
| 15 | Résistance BOOT0 | **10 kΩ pull-down** sur BOOT0 | Démarrage depuis la Flash par défaut | négligeable | Séquence de boot |
| 16 | Fils XSHUT ×4 | Fil fin **AWG32**, ~15-20 mm/capteur | — | négligeable | GPIO -> XSHUT de chaque ToF |

**Masse totale estimée (hors PCB et câblage) : ~28-30 g** — cohérent avec la classe
"tiny whoop" 65 mm (ces châssis volent typiquement entre 25 et 35 g tout compris).

## Pourquoi ces choix précis

- **STM32F405 plutôt que F411** : continuité avec la station météo (même famille,
  même toolchain déjà en main). Le F411 (LQFP48, plus petit) serait un vrai gain
  de poids/taille pour une itération future — noté mais pas retenu ici pour rester
  sur du connu.
- **VL53L1X plutôt que VL53L5CX** : le L5CX (8×8 zones) demande ~90 Ko de RAM/Flash
  pour son driver officiel — hors de portée confortable d'un F405 partagé avec le
  reste du firmware. Le L1X (une zone, une distance) suffit pour la détection
  directionnelle simple visée ici.
- **4 capteurs (pas 5)** : le firmware avait initialement prévu avant/arrière/haut/
  gauche/droite (voir `avoidance.c`, 5 directions supportées côté algorithme), mais
  le nombre de broches GPIO libres après I²C + PWM 4 moteurs + SWD ne permettait pas
  de câbler proprement un 5ᵉ XSHUT sans réviser tout le plan de brochage. Retenu :
  avant/haut/gauche/droite, sans arrière — capteur arrière ajoutable sur une
  itération future avec un GPIO expander (ex. PCF8574) plutôt que des broches MCU
  directes.
- **Brushed plutôt que brushless** : plus simple (pas d'ESC/firmware BLHeli à
  gérer), pilotage PWM direct MOSFET — cohérent avec une première itération
  centrée sur la validation de l'évitement, pas la performance de vol.

## Plan de brochage (STM32F405RGT6, LQFP64)

| Fonction | Broche(s) | AF | Notes |
|---|---|---|---|
| HSE | PH0/PH1 (OSC_IN/OUT) | — | Cristal 8 MHz |
| I2C1 (ToF ×4 + IMU, bus partagé) | PB6 (SCL), PB7 (SDA) | AF4 | Pull-up 4,7 kΩ communes |
| XSHUT ToF avant | PA0 | GPIO sortie | |
| XSHUT ToF haut | PA1 | GPIO sortie | |
| XSHUT ToF gauche | PA2 | GPIO sortie | |
| XSHUT ToF droite | PA3 | GPIO sortie | |
| PWM moteur avant-gauche | PA6 (TIM3_CH1) | AF2 | |
| PWM moteur avant-droit | PA7 (TIM3_CH2) | AF2 | |
| PWM moteur arrière-gauche | PB0 (TIM3_CH3) | AF2 | |
| PWM moteur arrière-droit | PB1 (TIM3_CH4) | AF2 | |
| SWDIO / SWCLK | PA13 / PA14 | AF0 | Programmation/debug |
| BOOT0 | — | — | Pull-down 10 kΩ (démarrage Flash) |

Ce tableau correspond exactement à ce qu'implémentent `i2c_bus.c` et
`motors_pwm.c` — **aucun conflit de broches**, contrairement à une première
version de ce firmware qui plaçait PWM et I²C tous les deux sur PB6/PB7 (repéré
et corrigé avant publication, voir historique des commits).

## Ce qui n'est pas encore spécifié

- **PCB** : ce document donne la nomenclature et le brochage logique, pas de
  routage. Pour un premier prototype, un montage sur PCB perfboard/veroboard à
  l'échelle est réaliste vu le nombre de composants ; un vrai PCB 4 couches
  deviendrait pertinent pour optimiser le poids en itération suivante.
- **Antenne/radio** : aucun lien radio n'est spécifié ici (le scénario "vole tout
  droit, évite les murs" ne nécessite pas de télécommande). Ajouter un récepteur
  (ex. ExpressLRS) serait une extension naturelle mais change le bilan de poids.
