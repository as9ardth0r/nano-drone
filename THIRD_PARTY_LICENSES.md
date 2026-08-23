# Licences tierces

Ce dépôt vendorise (copie directement dans le dépôt, plutôt que de les
télécharger à la compilation) un sous-ensemble minimal des en-têtes CMSIS
nécessaires pour compiler le firmware. Ces fichiers restent sous leur
licence d'origine, distincte de la licence MIT du reste du dépôt (voir
`LICENSE`).

## ARM CMSIS-Core (CMSIS_5)

- **Fichiers concernés** : `firmware/Drivers/CMSIS_Core/*.h`
- **Source** : https://github.com/ARM-software/CMSIS_5
- **Licence** : Apache License 2.0
- **Copyright** : (c) ARM Limited

## STMicroelectronics cmsis_device_f4

- **Fichiers concernés** :
  `firmware/Drivers/stm32f405xx.h`,
  `firmware/Drivers/stm32f4xx.h`,
  `firmware/Drivers/system_stm32f4xx.h`,
  `firmware/Core/Src/system_stm32f4xx.c`,
  `firmware/startup/startup_stm32f405xx.s`
- **Source** : https://github.com/STMicroelectronics/cmsis_device_f4
- **Licence** : Apache License 2.0
- **Copyright** : (c) STMicroelectronics

Le texte complet de la licence Apache 2.0 est disponible à
https://www.apache.org/licenses/LICENSE-2.0
