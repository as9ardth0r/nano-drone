# nanodrone-avoidance

### Évitement de collision pour nano-drone — STM32F405 + réseau ToF 5 directions + pilotage smartphone

Un drone qui détecte les murs, le plafond et ce qu'il y a derrière lui en
approche et reste stable sans percuter, grâce à un réseau de télémètres laser
(avant/arrière/haut/gauche/droite), un contrôleur réactif à champ de
potentiel, et une liaison BLE pour le piloter depuis un smartphone.

## Ce qui est réel et vérifié

Pas un exercice de style : chaque brique a été compilée et testée pour de vrai,
pas juste écrite.

| Brique | Vérifié comment |
|---|---|
| **Simulation Python** (`sim/nanodrone_sim/`) | Le plus important : sans évitement, le drone percute le mur ET le plafond (scénario de référence) ; avec évitement, il ne percute jamais, dans 3 configurations différentes (frontale, verticale, diagonale vers un coin) |
| **Portage C de l'algorithme d'évitement** (`firmware/Core/Src/avoidance.c`) | Compilé nativement et comparé numériquement à la version Python via `ctypes` sur 5 cas de test — résultats identiques à 1e-4 près |
| **Protocole de commande BLE** (`command_link.c`/`.py`) | Idem : parseur C comparé au Python sur trames valides et invalides, y compris le comportement de sécurité (repli en vol stationnaire si le lien est coupé plus de 0,5 s) |
| **Firmware STM32F405** (`firmware/`) | Compile et **linke réellement** avec `arm-none-eabi-gcc` contre les en-têtes CMSIS officiels ARM/ST → binaire `.elf`. Un vrai bug a été attrapé par le compilateur au passage (macro `GPIO_PIN` qui encodait la valeur ASCII du port au lieu de son index) |
| **Plan de brochage** | 5 capteurs ToF + PWM + I²C + BLE + SWD, vérifié sans conflit — voir `docs/hardware.md`. Une erreur de raisonnement précédente ("pas assez de GPIO pour un 5ᵉ capteur") a été corrigée après relecture du plan complet |
| **Conception PCB** | `docs/pcb.md` — largeurs de piste calculées (IPC-2221, pas à l'estimation), budget de capacité du bus I²C justifiant le choix 100 kHz plutôt que 400 kHz |

**34 tests, tous passent.**

## Ce qui n'est PAS vérifié — et ne peut pas l'être ici

Aucun test sur banc réel, aucun vol : construit et compilé dans un environnement
sandboxé sans accès au matériel physique. En particulier :

- **La mesure de distance VL53L1X elle-même** n'est pas implémentée dans
  `tof_array.c` — seul l'adressage I²C multi-capteurs (XSHUT) l'est, car il est
  documenté publiquement. La séquence de calibration/mesure du VL53L1X ne l'est
  pas ; le code appelle le driver officiel ST (VL53L1X ULD) à un point
  d'intégration clairement marqué plutôt que de deviner des registres.
- **La stabilisation d'attitude bas niveau** (boucle PID taux/angle) n'existe
  pas — `dynamics.py` et `main.c` supposent qu'elle fonctionne parfaitement pour
  isoler et valider la seule couche d'évitement.
- **La portée BLE réelle** dépend du placement de l'antenne (voir docs/pcb.md)
  — non mesurée, pas de prototype physique.
- **Aucune calibration réelle** (bruit capteur, dérive IMU, latence de boucle,
  vibrations moteur) n'est modélisée.

## Nomenclature matérielle et PCB

- **[docs/hardware.md](docs/hardware.md)** — nomenclature précise (références
  réelles, poids, rôle), plan de brochage complet, protocole BLE. Masse totale
  estimée ~31-33 g.
- **[docs/pcb.md](docs/pcb.md)** — largeurs de piste calculées, stack-up,
  placement mécanique des capteurs déportés.

## Structure du dépôt

```
sim/nanodrone_sim/
├── world.py           # modèle de pièce, distance rayon/mur
├── sensors.py          # réseau de capteurs ToF simulés
├── avoidance.py         # champ de potentiel répulsif (miroir de avoidance.c)
├── command_link.py       # protocole BLE + repli sécuritaire (miroir de command_link.c)
├── dynamics.py             # dynamique simplifiée (masse ponctuelle)
└── scenario.py               # orchestration d'un scénario complet
firmware/
├── Core/Inc, Core/Src   # code applicatif (drivers, avoidance.c, command_link.c, main.c)
├── Drivers/               # en-têtes CMSIS vendorisés (voir THIRD_PARTY_LICENSES.md)
├── startup/                # linker script + démarrage (STM32F405RGTx)
└── Makefile                 # compilation arm-none-eabi-gcc
tests/
├── test_world.py
├── test_avoidance.py                        # preuve : évitement fonctionne, baseline échoue
├── test_avoidance_c_matches_python.py         # validation croisée C/Python
├── test_command_link.py                        # protocole BLE + sécurité
└── test_command_link_c_matches_python.py         # validation croisée C/Python
docs/
├── hardware.md              # nomenclature détaillée + brochage
└── pcb.md                     # recommandations PCB, calculs de piste
.github/workflows/build.yml  # CI : tests Python + compilation firmware
```

## Installation et usage

```bash
# simulation
pip install -r sim/requirements.txt
pytest tests/ -v                    # 34 tests

# firmware (nécessite gcc-arm-none-eabi)
cd firmware
make                                 # produit build/nanodrone_fc.elf
```

Flashage sur la carte réelle (ST-Link, une fois le matériel assemblé selon
`docs/hardware.md`) : `st-flash write build/nanodrone_fc.bin 0x08000000` — pas
testé ici, pas de matériel disponible dans cet environnement.

Pilotage : connecter une appli BLE générique ("Serial Bluetooth Terminal" ou
équivalent) au module HM-10, envoyer des lignes `V<vx>,<vy>,<vz>*<checksum>\n`
— voir `docs/hardware.md` pour le détail du protocole.

## Licence

MIT pour le code original — voir `LICENSE`. Fichiers CMSIS vendorisés sous
Apache 2.0 — voir `THIRD_PARTY_LICENSES.md`.
