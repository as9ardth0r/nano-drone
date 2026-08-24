# PCB — recommandations de conception

Pas de fichiers de routage (KiCad/Gerber) ici, mais les contraintes chiffrées
nécessaires pour en faire un — calculées, pas approximées à l'œil.

## Stack-up

**2 couches** pour ce premier prototype : couche du haut (composants + signaux),
couche du bas en plan de masse quasi plein (améliore le retour de courant et
réduit l'EMI des commutations MOSFET, important vu la proximité avec le bus I²C
sensible). Un 4 couches (signal / masse / alim / signal) serait la suite
logique si l'intégrité du bus I²C pose problème en pratique — voir la section
budget de capacité plus bas.

## Largeurs de piste (calcul IPC-2221, pas une estimation)

Formule : `I = k × ΔT^0.44 × Aire(mils²)^0.725`, avec k=0,048 (piste externe),
et objectif ΔT=20°C d'échauffement — valeurs standard, pas un choix arbitraire.

| Piste | Courant visé | Cuivre 1 oz (35 µm) | Cuivre 2 oz (70 µm) |
|---|---|---|---|
| Alimentation moteur individuelle | 2 A | **~0,52 mm** (20 mil) | ~0,30 mm (12 mil) |
| Tronc batterie (4 moteurs cumulés, pic) | 6 A | **~2,35 mm** (92 mil) | ~1,17 mm (46 mil) |
| I²C / signaux logiques | <10 mA | 0,25 mm (10 mil) — limité par la fabrication, pas le courant |

**Recommandation** : 2 oz de cuivre si le fabricant le propose sans surcoût
important — les pistes moteur/batterie deviennent nettement plus gérables sur
une carte de cette taille (65 mm d'entraxe).

## Budget de capacité du bus I²C — pourquoi le firmware tourne à 100 kHz

Le bus dessert 5 capteurs ToF (Pololu VL53L1X, chacun sur un fil déporté vers
une carte fille en périphérie, pas soudé au ras du MCU) + l'IMU. La spec I²C
plafonne la capacité totale du bus à **400 pF en mode rapide (400 kHz)**, contre
**1000 pF en mode standard (100 kHz)**. Avec 5-6 câbles de quelques centimètres
vers des cartes filles, le budget 400 pF est risqué à respecter ; le firmware
(`i2c_bus.c`) a donc été réglé sur 100 kHz plutôt que 400 kHz — décision prise
en concevant le plan de câblage, pas un réglage par défaut laissé tel quel. Le
débit perdu n'est pas le facteur limitant ici (quelques lectures de distance
par cycle de contrôle, pas un flux de données).

## Placement des capteurs ToF — un vrai problème mécanique, pas que du routage

Les 5 capteurs ne peuvent pas tous être plaqués à plat sur une carte
horizontale : "haut" peut être soudé directement sur le dessus de la carte
principale (fenêtre optique vers le ciel, à condition qu'aucun bras de châssis
ne coupe son champ de vision), mais "avant/arrière/gauche/droite" doivent
regarder à l'horizontale — donc être montés à la verticale, en bordure de
carte.

Solution retenue : les 4 cartes Pololu VL53L1X restantes sont montées à 90° en
périphérie (sur de petits supports ou directement calées contre les bras du
châssis), reliées à la carte principale par de courts fils (VCC, GND, SDA,
SCL, XSHUT — 5 fils par capteur). C'est ce qui motive le choix de garder les
cartes Pololu (déjà en petit format, avec trous de fixation M2) plutôt que les
puces VL53L1X nues, qui demanderaient un routage direct sur la carte
principale bien plus délicat à ce format.

**Champ de vision à dégager** : le VL53L1X a un FOV typique de ~27° — prévoir
un cône dégagé de tout obstacle (bras de châssis, vis, câble) sur cet angle
devant chaque fenêtre optique, sous peine de mesurer la distance jusqu'au
propre châssis du drone plutôt que jusqu'à un obstacle réel.

## Emplacements réservés

- **Module BLE HM-10** : à plat sur la carte principale, antenne PCB orientée
  vers l'extérieur du châssis (pas au centre, entourée de moteurs/câbles —
  dégraderait la portée).
- **Header SWD** : accessible carte assemblée, pas sous un bras de châssis.
- **Connecteur batterie** : au centre de gravité si possible, pour ne pas
  déséquilibrer le drone une fois la batterie branchée.
- **Découplage** : un 100 nF au plus près de chaque broche VDD du STM32F405
  (plusieurs broches d'alimentation sur ce boîtier LQFP64 — vérifier le
  datasheet pour le nombre exact), le 10 µF bulk au plus près du régulateur
  3.3V.

## Ce qui reste à faire

Routage réel (KiCad recommandé, cohérent avec un usage libre/reproductible),
empreintes des composants (les Pololu, le HM-10 et le châssis 65 mm ont tous
des trous de fixation à aligner), et une passe de vérification DRC/ERC avant
tout envoi en fabrication.
