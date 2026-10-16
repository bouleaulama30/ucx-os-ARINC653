# Hardware Testbench : Banc de Test Avionique AMP sur PolarFire SoC Discovery Kit

Ce dossier contient l'ensemble des éléments, configurations et binaires nécessaires pour exécuter le **banc de test matériel avionique (Hardware Testbench)** sur la carte **PolarFire SoC Discovery Kit (MPFS-DISCO-KIT)**.

Le système repose sur une architecture **AMP (Asymmetric Multi-Processing)** exploitant deux cœurs d'application RISC-V 64 bits (U54) interconnectés via la mémoire interne partagée (**L2 LIM**) :
1. **Hart 1 (Core 1 - U54_1)** : Système **Bare-metal Microchip** exécutant la couche de communication matérielle (contrôleur SPI ou IP FPGA ARINC 429) et la génération/décodage des trames avioniques.
2. **Hart 2 (Core 2 - U54_2)** : Système d'exploitation temps-réel certifiable **UCX-OS-ARINC653**, exécutant deux partitions spatialement et temporellement isolées communiquant via un **Sampling Port**.
3. **Hart 0 (E51)** : Cœur moniteur exécutant le bootloader de premier niveau **HSS (Hart Software Services)** en mémoire non volatile (**eNVM**).

---

## Sommaire

1. [Structure du Dossier](#1-structure-du-dossier)
2. [Vue d'Ensemble de l'Architecture](#2-vue-densemble-de-larchitecture)
3. [Prérequis Matériels & Logiciels](#3-prérequis-matériels--logiciels)
4. [Étape 1 : Préparation de la Carte (Design Libero & HSS en eNVM)](#4-étape-1--préparation-de-la-carte-design-libero--hss-en-envm)
5. [Étape 2 : Déploiement Rapide avec les Binaires Fournis](#5-étape-2--déploiement-rapide-avec-les-binaires-fournis)
6. [Étape 3 : Flasher la Carte Micro SD avec USBImager](#6-étape-3--flasher-la-carte-micro-sd-avec-usbimager)
7. [Étape 4 : Connexions UART et Lancement du Système](#7-étape-4--connexions-uart-et-lancement-du-système)
8. [Étape 5 : Câblage Matériel & Schéma des Broches (Mode SPI)](#8-étape-5--câblage-matériel--schéma-des-broches-mode-spi)
9. [Étape 6 : Procédure pour l'IP FPGA ARINC 429 (Mode FPGA)](#9-étape-6--procédure-pour-lip-fpga-arinc-429-mode-fpga)
10. [Étape 7 : Modification et Recompilation des Applications (Si Besoin)](#10-étape-7--modification-et-recompilation-des-applications-si-besoin)
11. [Dépannage & Bonnes Pratiques](#11-dépannage--bonnes-pratiques)

---

## 1. Structure du Dossier

```text
arch/riscv/PolarFire/hardware_tb/
├── README.md                 # Le présent guide complet d'installation et d'exécution
├── INFO-README.md            # Notes initiales et liens de référence
├── holt_spi/                 # Variante 1 : Communication directe SPI vers carte Holt
│   ├── PolarFire.elf         # Exécutable UCX-OS ARINC 653 (Core 2, DDR @ 0x80000000)
│   ├── mpfs-spi-master-slave.elf # Exécutable Bare-metal SPI (Core 1, LIM @ 0x08000000)
│   └── payload.bin           # Image binaire fusionnée prête à flasher sur SD
├── holt_fpga/                # Variante 2 : Communication via IP ARINC 429 dans la fabric FPGA
│   ├── PolarFire.elf         # Exécutable UCX-OS ARINC 653 (Core 2, DDR @ 0x80000000)
│   ├── mpfs-spi-master-slave.elf # Exécutable Bare-metal FPGA (Core 1, LIM @ 0x08000000)
│   └── payload.bin           # Image binaire fusionnée prête à flasher sur SD
└── hss_payload/              # Outil de génération d'image de boot HSS (Payload Generator)
    ├── hss-payload-generator     # Binaire exécutable Linux
    ├── hss-payload-generator.exe # Binaire exécutable Windows
    ├── msys-*.dll                # Bibliothèques dynamiques pour Windows
    ├── payload.yaml              # Fichier de configuration du payload AMP
    └── README.md                 # Documentation officielle du générateur HSS
```

> [!TIP]
> Si vous ne souhaitez pas modifier le code source, **les fichiers `payload.bin` pré-compilés sont déjà prêts à l'emploi** dans `holt_spi/` et `holt_fpga/`. Vous pouvez passer directement à l'[Étape 3](#6-étape-3--flasher-la-carte-micro-sd-avec-usbimager).

---

## 2. Vue d'Ensemble de l'Architecture

### Deux Variantes Matérielles Prises en Charge

```
+---------------------------------------------------------------------------------------------------+
| POLARFIRE SOC DISCOVERY KIT (MPFS-DISCO-KIT)                                                      |
|                                                                                                   |
|  +--------------------+         +---------------------------------------------------------------+ |
|  | Hart 0 (E51)       |         | Hart 1 (U54_1) - BARE-METAL MICROCHIP                         | |
|  | Bootloader HSS     |         | - Générateur de données de vol (Altitude : montée, palier...) | |
|  | (Réside en eNVM)   |         | - Encodage / Décodage trames ARINC 429                        | |
|  +--------------------+         | - Pilote SPI matériel OU interface IP Fabric FPGA             | |
|                                 +-------------------------------+-------------------------------+ |
|                                                                 |                                 |
|                                                                 v (Ecriture IPC)                  |
|                                 +---------------------------------------------------------------+ |
|                                 | L2 LIM SHARED MEMORY (Adresse 0x08060000)                     | |
|                                 | Header Magique: 0x4152494E ("ARIN")                           | |
|                                 | Structure: shared_altitude_data_t (mètres, pieds, trame, ...)  | |
|                                 +-------------------------------+-------------------------------+ |
|                                                                 |                                 |
|                                                                 v (Lecture IPC avec fence)        |
|  +--------------------------------------------------------------+-------------------------------+ |
|  | Hart 2 (U54_2) - RTOS UCX-OS-ARINC653                                                        | |
|  |                                                                                              | |
|  |  +--------------------------------+               +---------------------------------------+  | |
|  |  | Partition 1 : P1_OUT_ALT       |               | Partition 2 : P2_IN_ALT               |  | |
|  |  | - Lit la mémoire partagée LIM  |               | - Lit le port SAMPLING_PORT           |  | |
|  |  | - Écrit dans le SAMPLING_PORT  | -(Canal ARINC)-> - Interprète la phase de vol         |  | |
|  |  |   (Période 200ms)              |     653       | - Affiche la télémétrie sur l'UART    |  | |
|  |  +--------------------------------+               +---------------------------------------+  | |
|  +----------------------------------------------------------------------------------------------+ |
+---------------------------------------------------------------------------------------------------+
```

### Détail des Variantes :
- **Variante 1 : Mode SPI (`holt_spi`)**
  Le Bare-metal (Core 1) utilise directement les contrôleurs SPI du PolarFire SoC reliés aux broches d'extension vers la ou les cartes Holt (transceivers ARINC 429).
- **Variante 2 : Mode FPGA (`holt_fpga`)**
  Une IP matérielle ARINC 429 (développée sous MATLAB/Simulink et instanciée dans la fabric FPGA via Libero SoC) assure la sérialisation/désérialisation du protocole avionique.

---

## 3. Prérequis Matériels & Logiciels

### Matériel
- **Carte PolarFire® SoC Discovery Kit** (Microchip MPFS-DISCO-KIT, puce MPFS095T).
- **Carte(s) transceiver ARINC 429 Holt** (ex: cartes d'évaluation Holt HI-3593 / HI-8582 ou équivalent).
- **Carte Micro SD** (capacité 4 Go à 32 Go, classe 10 recommandée).
- **Câble USB** pour la connexion à la carte (programmation JTAG FlashPro et 4 ports UART virtuels).
- **Câbles de liaison Dupont femelles-femelles** pour relier le connecteur GPIO de la Discovery Kit à la carte Holt.
- **Paire torsadée** pour la ligne différentielle avionique ARINC 429 ($A$ et $B$, $\pm 10\text{V}$).

### Logiciels PC
- **Microchip Libero® SoC v2024.1 ou v2025.2+** (avec licence gratuite *Silver*).
- **Microchip SoftConsole v2022.2-RISC-V-747** (environnement IDE Eclipse + chaîne de compilation RISC-V 64).
- **FlashPro Express** (fourni avec Libero SoC ou avec les *Program and Debug Tools*).
- **USBImager** : utilitaire ultra-léger et fiable pour flasher des images raw sur carte SD.  
  👉 Téléchargement : [GitLab USBImager](https://gitlab.com/bztsrc/usbimager)
- **Tera Term VT** (ou tout émulateur série multi-fenêtres supportant 115200 bauds).  
  👉 Téléchargement : [GitHub Tera Term](https://github.com/TeraTermProject/teraterm/releases)
- **HSS Payload Generator** (fourni directement dans `hss_payload/`).
- *(Optionnel pour le mode FPGA)* : **MATLAB / Simulink** avec **HDL Coder** (pour régénérer l'IP du dépôt `arinc429_ip`).

---

## 4. Étape 1 : Préparation de la Carte (Design Libero & HSS en eNVM)

Avant de lancer le système pour la première fois, la carte PolarFire SoC doit posséder :
1. Le bitstream FPGA de base programmé dans la puce.
2. Le bootloader **HSS** programmé dans la mémoire non volatile interne (**eNVM**) avec la carte configurée en **Boot Mode 1**.

### 1.1 Programmer le Design de Référence dans le FPGA
Le design de référence configure les horloges, la mémoire DDR4 et le multiplexage des broches I/O.
1. Télécharger ou cloner le dépôt de référence Microchip :  
   [polarfire-soc-discovery-kit-reference-design sur GitHub](https://github.com/polarfire-soc/polarfire-soc-discovery-kit-reference-design)
2. Ouvrir **Libero SoC**.
3. Ouvrir la fenêtre d'exécution de script : `Project > Execute Script...` (ou raccourci `Ctrl + U`).
4. Sélectionner le script `MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl` et cliquer sur **Run**.
5. Une fois le projet généré, double-cliquer sur **Program Device** dans le volet *Design Flow* pour programmer le FPGA via FlashPro Express.

### 1.2 Construire et Programmer le Bootloader HSS dans l'eNVM
Le HSS (Hart Software Services) est le zero-stage bootloader (ZSBL) tournant sur le cœur moniteur E51.
1. Consulter la documentation et le tutoriel vidéo officiels :
   - Documentation : [PolarFire SoC Software Tool Flow - Programming the eNVM](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/knowledge-base/polarfire-soc-software-tool-flow.md#programming-the-envm)
   - Tutoriel vidéo : [Microchip Media Center - Video Tutorial eNVM HSS](https://www.microchip.com/en-us/about/media-center/videos/caNsH4dDnLU)
   - Code source HSS : [hart-software-services sur GitHub](https://github.com/polarfire-soc/hart-software-services)
2. Dans **SoftConsole**, importer le projet HSS configuré pour la Discovery Kit (`mpfs-discovery-kit`).

3. Vérifier que le mode de démarrage de la carte est bien le **Boot Mode 1** (démarrage automatique du HSS depuis l'eNVM au reset).

---

## 5. Étape 2 : Déploiement Rapide avec les Binaires Fournis

Deux images complètes pré-générées sont prêtes à être déployées selon la configuration physique retenue :

| Configuration | Dossier | Fichier Binaire | Description |
| :--- | :--- | :--- | :--- |
| **Variante SPI** | `arch/riscv/PolarFire/hardware_tb/holt_spi/` | `payload.bin` | Contrôleur SPI direct vers carte(s) Holt |
| **Variante FPGA** | `arch/riscv/PolarFire/hardware_tb/holt_fpga/` | `payload.bin` | IP ARINC 429 synthétisée dans la fabric FPGA |

Chaque dossier contient également :
- `PolarFire.elf` : L'exécutable compilé d'UCX-OS-ARINC653 (Core 2).
- `mpfs-spi-master-slave.elf` : L'exécutable compilé du système Bare-metal Microchip (Core 1).

---

## 6. Étape 3 : Flasher la Carte Micro SD avec USBImager

Le binaire `payload.bin` doit être gravé en format brut sur la carte Micro SD. **USBImager** est l'outil recommandé

### Procédure pas à pas :
1. Insérer la carte Micro SD dans votre PC (via adaptateur SD ou lecteur USB).
2. Lancer **USBImager**.
3. Dans le premier champ (Image), cliquer sur `...` et sélectionner le fichier :
   - `arch/riscv/PolarFire/hardware_tb/holt_spi/payload.bin` (pour le mode SPI)  
   *OU*
   - `arch/riscv/PolarFire/hardware_tb/holt_fpga/payload.bin` (pour le mode FPGA).
4. Dans le deuxième champ (Device), sélectionner soigneusement la carte Micro SD (attention à ne pas écraser vos disques durs système !).
5. Cliquer sur le bouton **Write**. L'écriture ne prend que quelques secondes (taille $\approx 60\text{ Ko}$).
6. Une fois l'opération terminée, éjecter proprement la carte Micro SD du PC.
7. Insérer la carte Micro SD dans le slot dédié de la PolarFire SoC Discovery Kit (**J1**).

---

## 7. Étape 4 : Connexions UART et Lancement du Système

La puce pont USB-Série de la Discovery Kit expose **plusieurs ports COM virtuels** sur un seul câble USB.

### 7.1 Repérage des Ports COM
Brancher le câble USB entre la Discovery Kit et votre ordinateur.
- **Sous Windows** : Ouvrir le *Gestionnaire de périphériques* > *Ports (COM et LPT)*. Vous verrez 3 ou 4 ports consécutifs (par exemple : `COM4`, `COM5`, `COM6`, `COM7`).
- **Sous Linux** : Exécuter `ls -l /dev/ttyUSB*` (vous verrez `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`, etc.).

### 7.2 Configuration de Tera Term (ou PuTTY)
Ouvrir **trois fenêtres distinctes de Tera Term**, configurées à **115200 bauds, 8 bits de données, 1 bit de stop, pas de parité (115200 8N1)** :

| Instance Tera Term | Port Typique (Windows) | Rôle dans le Banc de Test | Description du Flux |
| :--- | :--- | :--- | :--- |
| **Console 1** | `COM4` | **Hart 0 (E51) - HSS Bootloader** | Menu HSS, statut de boot SD, commandes système |
| **Console 2** | `COM5` | **Hart 1 (U54_1) - Bare-metal Microchip** | Traces d'émission/réception SPI/FPGA ARINC 429 |
| **Console 3** | `COM6` | **Hart 2 (U54_2) - UCX-OS ARINC 653** | Traces RTOS, partitions P1 et P2, télémétrie reçue |

### 7.3 Démarrage de l'Application
1. Une fois les trois fenêtres ouvertes, appuyez sur le bouton-poussoir **RESET** de la carte Discovery Kit (ou tapez la commande `RESET` dans la Console HSS sur `COM4`).
2. Sur la **Console 1 (COM4 - HSS)**, vous verrez le journal de démarrage HSS qui charge les deux contextes ELF depuis la carte SD vers la RAM :
   ```text
   [HSS] Reading payload from MMC/SD...
   [HSS] Booting Hart 1 at 0x08000000 (PRV_M)...
   [HSS] Booting Hart 2 at 0x80000000 (PRV_M)...
   ```
3. Sur la **Console 2 (COM5 - Bare-metal)**, les messages de vol simulés et de transmission de mots ARINC 429 s'affichent :
   ```text
   [Core 1] ARINC 429 Flight Generator Started
   [Core 1] TX ARINC Word: 0x0138406A | Alt: 1250 m | Phase: MONTEE
   ```
4. Sur la **Console 3 (COM6 - UCX-OS ARINC 653)**, vous observerez le démarrage du noyau et le dialogue inter-partitions :
   ```text
   =======================================================
     UCX-OS ARINC 653 RTOS - Optimized Telemetry App      
     Execution Hart / Core ID : 2 (U54_2)              
   =======================================================
   [P1 Init] CREATE_SAMPLING_PORT P1_OUT_ALT rc=0 (port_id=0)
   [P1 Init] CREATE_PROCESS p1_process1 (id=0, rc=0)
   [P2 Init] CREATE_SAMPLING_PORT P2_IN_ALT rc=0 (port_id=1)
   [P2 Init] CREATE_PROCESS p2_process1 (id=0, rc=0)
   [P1] IPC->Port OK | Alt: 1250 m | Count: 42
   [P2] READ OK | Alt: 1250 m (4101 ft) | Phase: MONTEE | ARINC: 0x0138406A | Trame: #42 | 8400 ms
   ```

---

## 8. Étape 5 : Câblage Matériel & Schéma des Broches (Mode SPI)

En mode SPI, la liaison entre le PolarFire SoC et la carte éval Holt s'effectue via le connecteur d'extension **Raspberry Pi 40 broches (J10)** ou le connecteur **MikroBUS** de la Discovery Kit.

### 8.1 Broches SPI sur le Connecteur Raspberry Pi 40 broches (J10)

```text
Discovery Kit J10 (40 broches)                  Carte Holt ARINC 429
+-----------------------------+                 +---------------------+
| Pin 1  : 3.3V Power         |---------------->| VDD / 3.3V Logic    |
| Pin 6  : GND Ground         |---------------->| GND                 |
| Pin 19 : SPI0_MOSI (GPIO10) |---------------->| SI  (Serial In)     |
| Pin 21 : SPI0_MISO (GPIO9)  |---------------->| SO  (Serial Out)    |
| Pin 23 : SPI0_SCLK (GPIO11) |---------------->| SCK (Serial Clock)  |
| Pin 24 : SPI0_SS0  (GPIO8)  |---------------->| /CS (Chip Select)   |
+-----------------------------+                 +---------------------+
```

> [!NOTE]
> Si vous utilisez deux cartes Holt distinctes (Architecture 2 : une émettrice et une réceptrice) :
> - **Carte Holt 1 (TX)** : Reliée sur le contrôleur `SPI0` (Broches 19, 21, 23, 24).
> - **Carte Holt 2 (RX)** : Reliée sur le contrôleur `SPI1` (disponible sur le connecteur MikroBUS ou routé sur le connecteur J10 via les broches libres `SPI1_SS0`, `SPI1_MOSI`, `SPI1_MISO`, `SPI1_SCLK`).

### 8.2 Liaison Bus Avionique ARINC 429 ($\pm 10\text{V}$)
Relier la sortie de la carte émettrice à l'entrée de la carte réceptrice avec une paire torsadée :

```text
+-----------------------+                         +-----------------------+
|  Carte Holt 1 (TX)    |                         |  Carte Holt 2 (RX)    |
|                       |                         |                       |
|   Borne AOUT (TX_A)   |========================>|   Borne ARX (RX_A)    |
|   Borne BOUT (TX_B)   |========================>|   Borne BRX (RX_B)    |
|   Masse / Blindage    |------------------------>|   Masse / Blindage    |
+-----------------------+                         +-----------------------+
        Ligne différentielle avionique ARINC 429 (Niveaux +10V / 0V / -10V)
```

---

## 9. Étape 6 : Procédure pour l'IP FPGA ARINC 429 (Mode FPGA)

Pour la variante FPGA, le décodage et l'encodage ARINC 429 ne sont plus gérés logiciellement par bitbang ou SPI standard, mais par une **IP matérielle VHDL/Verilog** implantée dans la matrice FPGA.

Les sources complètes de l'IP se trouvent dans le dépôt externe dédié : **`arinc429_ip`**.

### 9.1 Modélisation & Génération HDL sous MATLAB / Simulink
1. Cloner ou ouvrir le dossier du dépôt `arinc429_ip`.
2. Ouvrir **MATLAB**.
3. Ouvrir le modèle Simulink principal : `arinc429_transceiver.slx` (ou exécuter le script de configuration `setup_arinc429.m`).
4. Dans Simulink, ouvrir **HDL Coder** (`Apps > HDL Coder > HDL Workflow Advisor`).
5. Configurer la cible :
   - Synthèse pour PolarFire SoC (famille PolarFire).
   - Fréquence d'horloge de l'IP : correspondante à la fabric (ex: 50 MHz ou 100 MHz).
6. Exécuter l'étape **Generate HDL** : les fichiers VHDL/Verilog générés se trouvent dans le sous-dossier `hdlsrc/`.

### 9.2 Intégration dans Libero SoC
1. Ouvrir votre projet Libero SoC pour la Discovery Kit.
2. Importer l'IP dans le projet :
   - **Méthode 1 (Automatisée)** : Exécuter le script TCL fourni dans `arinc429_ip` via `Ctrl + U` :  
     `source import_arinc429_ip.tcl`
   - **Méthode 2 (Manuelle)** : Aller dans `Catalog > Create Core from HDL...`, sélectionner les fichiers VHDL générés par MATLAB, puis instancier le bloc dans votre schéma **SmartDesign**.
3. Raccorder les interfaces de l'IP :
   - Bus de registres (APB ou AXI) relié au bus MSS FIC0/FIC1.
   - Entrées/Sorties ARINC 429 connectées aux tampons I/O physiques vers les broches du connecteur d'extension.
4. Lancer la chaîne de conception Libero :
   - `Synthesize` -> `Place and Route` -> `Generate Bitstream`.
5. Exporter et programmer le nouveau bitstream via **FlashPro Express**.

---

## 10. Étape 7 : Modification et Recompilation des Applications (Si Besoin)

Si vous devez adapter les applications pour vos propres tests, suivez les étapes ci-dessous.

### 10.1 Préparation et Compilation de l'Application UCX-OS
1. **Remplacer le code applicatif principal** :  
   Copier le contenu du fichier template `app/arinc_app_altitude.c` dans le fichier d'entrée de la cible PolarFire `arch/riscv/PolarFire/src/app/app_main.c`.
2. **Configurer les descripteurs statiques ARINC 653** :
   - Copier le fichier d'en-tête `include/arinc/static/static_conf_arinc_app_altitude.h` vers `include/arinc/static_conf.h` dans votre projet SoftConsole.
   - Copier le fichier d'implémentation `arinc/static/static_conf_arinc_app_altitude.c` vers `src/ucx/arinc/static_conf.c` dans votre projet SoftConsole.
3. **Compiler le projet sous SoftConsole** :
   - Clic droit sur le projet `PolarFire` > **Build Project**.
   - Le binaire généré se trouve dans `Debug/PolarFire.elf` (ou `Release/PolarFire.elf`).

### 10.2 Préparation et Compilation de l'Application Bare-metal
1. Dans le projet SoftConsole `mpfs-spi-master-slave`, ouvrir `src/platform/drivers/mss/mss_spi/u54_1.c` (ou le fichier applicatif du Hart 1).
2. Vérifier que la boucle d'émission écrit bien dans l'adresse partagée **`0x08060000`** avec le magic `0x4152494E`.
3. Compiler le projet sous la configuration cible : **`LIM-Debug-DiscoveryKit`**.
4. Le binaire généré se trouve dans `LIM-Debug-DiscoveryKit/mpfs-spi-master-slave.elf`.

### 10.3 Générer un Nouveau `payload.bin`
1. Se positionner dans le dossier `hss_payload/` :
   ```bash
   cd arch/riscv/PolarFire/hardware_tb/hss_payload
   ```
2. Vérifier les chemins absolus ou relatifs des deux fichiers `.elf` dans `payload.yaml` :
   ```yaml
   set-name: 'AMP_BM_UCXOS'
   hart-entry-points: {u54_1: '0x08000000', u54_2: '0x80000000'}

   payloads:
     'chemin/vers/mpfs-spi-master-slave.elf': {exec-addr: '0x08000000', owner-hart: u54_1, priv-mode: prv_m, skip-opensbi: true}
     'chemin/vers/PolarFire.elf': {exec-addr: '0x80000000', owner-hart: u54_2, priv-mode: prv_m, skip-opensbi: true}
   ```
3. Exécuter le générateur :
   - **Sous Windows** :
     ```cmd
     hss-payload-generator.exe -c payload.yaml payload.bin
     ```
   - **Sous Linux** :
     ```bash
     chmod +x hss-payload-generator
     ./hss-payload-generator -c payload.yaml payload.bin
     ```
4. Copier le nouveau fichier `payload.bin` vers votre dossier cible (`holt_spi/` ou `holt_fpga/`) et flasher sur la Micro SD avec USBImager (Étape 3).

---

## 11. Dépannage & Bonnes Pratiques

### 1. La console HSS n'affiche rien lors du RESET
- Vérifiez le câble USB et les cavaliers d'alimentation de la carte.
- Vérifiez que le port COM sélectionné correspond bien au cœur E51 (premier port virtuel, souvent le numéro le plus bas, ex: COM4).
- Assurez-vous que le baudrate est strictement fixé à **115200 bauds**.
- Vérifiez que le **Boot Mode 1** est bien activé et que le HSS a été programmé en eNVM.

### 2. Le HSS affiche `Failed to read payload from MMC/SD`
- La carte Micro SD n'est pas insérée ou est mal insérée dans le slot J1.
- La carte a été formatée au lieu d'être écrite en image brute : utilisez impérativement **USBImager** pour écrire `payload.bin` directement sur le périphérique bloc.

### 3. Les cœurs démarrent mais les données d'altitude restent à 0 ou n'évoluent pas
- **Vérification du Magic Number** : Le Core 2 attend le mot d'en-tête `0x4152494E` ("ARIN" en ASCII). Si le Core 1 n'a pas encore écrit ou a une adresse différente de `0x08060000`, le Core 2 attendra.
- **Barrière mémoire (Fence)** : Le cœur U54_2 dispose d'un cache L1. Le code applicatif dans `app_main.c` utilise l'instruction assembleur :
  ```c
  asm volatile ("fence rw, rw" ::: "memory");
  ```
  pour forcer le rechargement effectif de la mémoire physique partagée L2 LIM.
- **Inversion du câblage ARINC 429** : Vérifiez que la ligne AOUT est bien connectée à ARX, et BOUT à BRX. Si les lignes sont croisées ($A$ sur $B$), la parité et les labels seront rejetés par le décodeur.

---

> Pour toute information complémentaire sur le système d'exploitation ARINC 653, la configuration statique des tables de santé HM ou l'ordonnanceur deux niveaux, consultez le fichier `README.md` situé à la racine du dépôt ucx-os-ARINC653.
