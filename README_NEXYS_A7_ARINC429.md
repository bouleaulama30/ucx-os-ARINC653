# Guide de Compilation et Exécution : ARINC 653 sur Nexys A7-100T (HF-RISCV Dual-Core)

Ce guide explique pas-à-pas comment compiler, configurer et exécuter les applications avioniques conformes au standard **ARINC 653 Part 1** sur l'architecture dual-core HF-RISCV de la carte Digilent Nexys A7-100T.

---

## 1. Vue d'ensemble de l'Architecture

- **Cible Matérielle** : Dual-Core HF-RISCV RV32I à 25 MHz sur FPGA Xilinx Artix-7.
- **Mémoire BRAM** : 256 Ko par cœur (initialisée via `code0.txt` et `code1.txt`).
- **Passerelle ARINC 429** : Convertisseur matériel Simulink mappé en MMIO à l'adresse `0x50000000`.
- **Cœur 0** : Émetteur ARINC 429 périodique (Label 312 Altitude, SDI, SSM, parité impaire).
- **Cœur 1** : Récepteur et décodeur ARINC 429 temps réel.

```
+-----------------------------------------------------------------------------+
|                                Nexys A7 FPGA                                |
|                                                                             |
|  +---------------------------+             +-----------------------------+  |
|  |     CŒUR 0 (ARINC 653)    |             |      CŒUR 1 (ARINC 653)     |  |
|  | P1 : Émetteur Label 312   |             | P1 : Récepteur / Décodeur   |  |
|  | P2 : Monitoring Système   |             | P2 : Instrumentation PFD    |  |
|  +-------------+-------------+             +--------------+--------------+  |
|                | MMIO Write                               ^ MMIO Read       |
|                v                                          |                 |
|       +------------------------------------------------------------+        |
|       |     Convertisseur ARINC 429 (Encodeur -> Décodeur Série)   |        |
|       +------------------------------------------------------------+        |
|                                     |                                       |
|                                     v                                       |
|                          LED 6 (Bascule sur mot reçu)                       |
+-----------------------------------------------------------------------------+
```

---

## 2. Configuration Statique ARINC 653

Le standard ARINC 653 exige une définition statique préalable de toutes les ressources et fenêtres temporelles.
Les fichiers associés sont :
- [`include/arinc/static/static_conf_arinc429.h`](file:///home/leo/ucx-os-ARINC653/include/arinc/static/static_conf_arinc429.h) (copié dans `include/arinc/static_conf.h`)
- [`arinc/static/static_conf_arinc429.c`](file:///home/leo/ucx-os-ARINC653/arinc/static/static_conf_arinc429.c) (copié dans `arinc/static_conf.c`)

### Paramètres clés :
- **Major Frame (MZF)** : 100 ms (10 ticks d'horloge @ 100 Hz).
- **Partition 1 (50 ms)** : Héberge l'application ARINC 429 principale (`p1_process1`).
- **Partition 2 (50 ms)** : Héberge la tâche de surveillance/cockpit (`p2_process1`).
- **Limites Système** : Dimensionnées pour tenir dans les 256 Ko de BRAM (`MAX_LOG_ENTRIES = 16`, 16 processus max).

---

## 3. Structure des Applications

### 3.1 Cœur 0 : Émetteur Altitude ARINC 429 ([`app/arinc653_tx.c`](file:///home/leo/ucx-os-ARINC653/app/arinc653_tx.c))

- **Section mémoire** : `__attribute__((section(".p1_code"))) void p1_process1(void)`
- **Registres MMIO utilisés** :
  - `0x50000000` : `ARINC_TX_DATA` (Mot de 32 bits à émettre).
  - `0x50000004` : `ARINC_TX_DELAY` (1000 cycles d'inter-message gap).
  - `0x50000008` : `ARINC_TX_SPEED` (125 cycles pour 100 kbps @ 25 MHz).
- **Format du mot** :
  - Label 312 octal (Altitude standard).
  - Donnée 16 bits : altitude en pieds ($500\text{ m} \rightarrow 5000\text{ m}$).
  - SSM = `0b10` (Normal Operation), SDI = `0b00`.
  - Bit 31 : Parité impaire calculée.
- **Temporisation** : `TIMED_WAIT(100, &return_code)` (attente APEX standard de 100 ticks = 1 seconde).

### 3.2 Cœur 1 : Récepteur et Décodeur ARINC 429 ([`app/arinc653_rx.c`](file:///home/leo/ucx-os-ARINC653/app/arinc653_rx.c))

- **Section mémoire** : `__attribute__((section(".p1_code"))) void p1_process1(void)`
- **Registres MMIO utilisés** :
  - `0x50000010` : `ARINC_RX_DATA` (Sortie directe décodeur).
  - `0x50000014` : `ARINC_RX_STATUS` (Bit 0 : mot prêt).
  - `0x50000018` : `ARINC_RX_LATCHED` (Mot verrouillé en matériel).
- **Décodage** : Extraction du label, conversion pieds/mètres, vérification de parité impaire.
- **Temporisation** : `TIMED_WAIT(50, &return_code)` (scrutation toutes les 500 ms).

---

## 4. Procédure de Compilation

Pour compiler et générer les images mémoire pour la Nexys A7 :

```bash
cd /home/leo/ucx-os-ARINC653

# Étape 1 : Nettoyer l'environnement et compiler la bibliothèque statique libucxos.a
make clean
make ucx ARCH=riscv/hf-riscv

# Étape 2 : Compiler l'émetteur (Cœur 0) et copier vers le projet hardware
make arinc653_tx ARCH=riscv/hf-riscv
cp build/target/code.txt /home/leo/nexys-A7-hwtb/doc-leo-complet/doc-leo/02-code/003-ucx-start/src/code0.txt

# Étape 3 : Compiler le récepteur (Cœur 1) et copier vers le projet hardware
make arinc653_rx ARCH=riscv/hf-riscv
cp build/target/code.txt /home/leo/nexys-A7-hwtb/doc-leo-complet/doc-leo/02-code/003-ucx-start/src/code1.txt
```

> [!TIP]
> La cible `make arinc653_tx` et `make arinc653_rx` génèrent automatiquement le fichier `build/target/code.txt` formaté en mots hexadécimaux de 32 bits (`hexdump -v -e '4/1 "%02x" "\n"'`), directement interprétable par Vivado pour l'initialisation de la BRAM FPGA.

---

## 5. Synthèse et Programmation FPGA

Une fois `code0.txt` et `code1.txt` copiés :

```bash
# Générer le bitstream Vivado (prend ~4 minutes)
cd /home/leo/nexys-A7-hwtb/test-hfriscv
./build_bitstream.sh

# Programmer le FPGA (carte allumée et branchée en micro-USB)
./program_fpga.sh
```

---

## 6. Vérification de l'Exécution (Terminal Série & LEDs)

Ouvrir un terminal série (PuTTY ou picocom) sur `/dev/ttyUSB1` à **57600 bauds (8N1)** :

### Cœur 0 (`SW0 = 0`, LED 4 éteinte)
Affiche le boot ARINC 653, la configuration de la Major Frame et l'envoi des trames :
```text
boot
UCX-OS ARINC-653 v1.1.1
heap_init() 0x40003000 - 0x4003e000, 241664 bytes free
[ARINC 653] Module Initialized: NexysA7_DualCore
[ARINC 653] Major Frame: 100 ms | 2 Partitions

=======================================================
   UCX-OS ARINC 653 - CORE 0 TX (Label 312 Altitude)   
   Partition 1 / Process 1 (ARINC 429 Altitude TX)     
=======================================================

[CORE 0 ARINC653 TX #1] Alt = 500 m (1640 ft) | Word = 0xc01a0053
[CORE 0 ARINC653 P2] Telemetry Monitor Heartbeat #1 (OK)
[CORE 0 ARINC653 TX #2] Alt = 550 m (1804 ft) | Word = 0xc01c3053
```

### Cœur 1 (`SW0 = 1`, LED 4 allumée)
Affiche le décodage en temps réel des mots reçus :
```text
boot
UCX-OS ARINC-653 v1.1.1
heap_init() 0x40003000 - 0x4003e000, 241664 bytes free
[ARINC 653] Module Initialized: NexysA7_DualCore
[ARINC 653] Major Frame: 100 ms | 2 Partitions

=======================================================
   UCX-OS ARINC 653 - CORE 1 RX (Continuous Polling)   
   Partition 1 / Process 1 (ARINC 429 Altitude RX)     
=======================================================

[CORE 1 ARINC653 RX #1] DATA=0xc01a0053 | STATUS=0x00000001 | LATCHED=0xc01a0053
  >>> [ARINC 653] RECU MOT ARINC 429 VALIDE <<<
      Mot Brut          = 0xc01a0053
      Label Octal       = 312 (raw: 0x53)
      Altitude Decodee  = 500 m (1640 ft)
      Parite            = OK (ODD)

[CORE 1 ARINC653 P2] Primary Flight Display (PFD) Active #1
```

### Indicateurs Matériels
- **LED 6 (ARINC RX Toggle)** : Bascule d'état à chaque mot ARINC 429 reçu par le décodeur matériel.
- **LED 7 (Heartbeat)** : Clignotement régulier à ~1 Hz.
