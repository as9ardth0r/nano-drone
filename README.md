# nanodrone-avoidance

### Évitement de collision pour nano-drone — STM32F405 + réseau ToF multi-directionnel

Un drone qui détecte les murs et le plafond en approche et reste stable sans
percuter, grâce à un réseau de télémètres laser (avant/haut/gauche/droite) et
un contrôleur réactif à champ de potentiel.

## Ce qui est réel et vérifié

Pas un exercice de style : chaque brique a été compilée et testée pour de vrai,
pas juste écrite.

| Brique | Vérifié comment |
|---|---|
| **Simulation Python** (`sim/nanodrone_sim/`) | 17 tests. Le plus important : sans évitement, le drone percute le mur ET le plafond (scénario de référence) ; avec évitement, il ne percute jamais, dans 3 configurations différentes (frontale, verticale, diagonale vers un coin) |
| **Portage C de l'algorithme d'évitement** (`firmware/Core/Src/avoidance.c`) | Compilé nativement et comparé numériquement à la version Python via `ctypes` sur 5 cas de test — résultats identiques à 1e-4 près, pas juste "porté à l'œil" |
| **Firmware STM32F405** (`firmware/`) | Compile et **linke réellement** avec `arm-none-eabi-gcc` contre les en-têtes CMSIS officiels ARM/ST → binaire `.elf` de 2,7 Ko. Un vrai bug a été attrapé par le compilateur au passage (macro `GPIO_PIN` qui encodait la valeur ASCII du port au lieu de son index — aurait généré des adresses mémoire aberrantes sur la vraie carte) |
| **Conflit de brochage** | Repéré (I²C1 et PWM moteurs se disputaient PB6/PB7) et corrigé avant publication — PWM déplacé sur TIM3 (PA6/PA7/PB0/PB1), voir `docs/hardware.md` |

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
  isoler et valider la seule couche d'évitement. Un vrai drone a besoin de cette
  boucle en plus de ce qui est ici.
- **Aucune calibration réelle** (bruit capteur, dérive IMU, latence de boucle,
  vibrations moteur) n'est modélisée. La simulation prouve que l'*algorithme*
  fonctionne dans des conditions idéalisées, pas que le *drone assemblé* volera
  sans problème du premier coup.

## Nomenclature matérielle

Voir **[docs/hardware.md](docs/hardware.md)** — liste précise des composants
(références réelles, poids, rôle), plan de brochage complet, et justification
de chaque choix (pourquoi 4 capteurs et pas 5, pourquoi brushed et pas
brushless, etc.). Masse totale estimée ~28-30 g.

## Structure du dépôt

```
sim/nanodrone_sim/
├── world.py           # modèle de pièce, distance rayon/mur
├── sensors.py          # réseau de capteurs ToF simulés
├── avoidance.py         # champ de potentiel répulsif (miroir de avoidance.c)
├── dynamics.py           # dynamique simplifiée (masse ponctuelle)
└── scenario.py             # orchestration d'un scénario complet
firmware/
├── Core/Inc, Core/Src   # code applicatif (drivers, avoidance.c, main.c)
├── Drivers/               # en-têtes CMSIS vendorisés (voir THIRD_PARTY_LICENSES.md)
├── startup/                # linker script + démarrage (STM32F405RGTx)
└── Makefile                 # compilation arm-none-eabi-gcc
tests/
├── test_world.py
├── test_avoidance.py             # preuve : évitement fonctionne, baseline échoue
└── test_avoidance_c_matches_python.py  # validation croisée C/Python
docs/hardware.md          # nomenclature détaillée + brochage
.github/workflows/build.yml  # CI : tests Python + compilation firmware
```

## Installation et usage

```bash
# simulation
pip install -r sim/requirements.txt
pytest tests/ -v                    # 17 tests, ~0.2 s

# firmware (nécessite gcc-arm-none-eabi)
cd firmware
make                                 # produit build/nanodrone_fc.elf
```

Flashage sur la carte réelle (ST-Link, une fois le matériel assemblé selon
`docs/hardware.md`) : `st-flash write build/nanodrone_fc.bin 0x08000000` — pas
testé ici, pas de matériel disponible dans cet environnement.

## Licence

MIT pour le code original — voir `LICENSE`. Fichiers CMSIS vendorisés sous
Apache 2.0 — voir `THIRD_PARTY_LICENSES.md`.
