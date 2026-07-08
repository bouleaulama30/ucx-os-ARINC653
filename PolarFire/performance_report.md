# Rapport des Tests de Performance (ARINC 653 / UCX-OS)

Ce rapport présente les mesures de temps d'exécution des différents tests de performance exécutés sur la cible PolarFire SoC.

## Synthèse des résultats (perf1 à perf11)

| Test | Mesure | Moyen (ns) | WCET (ns) | BCET (ns) | Écart-Type (ns) | Échantillons |
|---|---|---|---|---|---|---|
| **perf1** | `CNTX SWITCH` (Changement de contexte) | 2 001 | 5 165 | 1 931 | 116.58 | 499 996 |
| **perf2** | `Mutex release unblock` | 2 884 | 3 145 | 2 861 | 555.33 | 3 000 |
| **perf3** | `Mutex acquisition` | 1 050 | 1 636 | 1 048 | 77.40 | 300 |
| **perf3** | `Mutex release` | 1 475 | 2 241 | 1 471 | 117.28 | 300 |
| **perf4** | `Mutex PIP` (Héritage de priorité - 4999 éch.) | 7 385 | 9 548 | 7 385 | 1 575.67 | 4 999 |
| **perf5** | `Mutex PIP` (Héritage de priorité - 49 éch.) | 7 546 | 7 926 | 7 385 | 446.12 | 49 |
| **perf6** | `Task Delay Latency` (Raw delta) | ~ -100 ms | N/A | N/A | N/A | 17 (échantillons bruts) |
| **perf7** | `Sem: Wait block` | 1 393 | 1 508 | 1 388 | 131.11 | 1 000 |
| **perf7** | `Sem: Signal unblock` | 823 | 865 | 823 | 93.02 | 1 000 |
| **perf8** | `Sem: Signal with prio` | 870 | 943 | 861 | 133.48 | 1 000 |
| **perf9** | `Sem: Signal` (Simple) | 111 | 338 | 101 | 10.63 | 1 000 |
| **perf9** | `Sem: Wait` (Simple) | 110 | 120 | 105 | 2.00 | 1 000 |
| **perf10** | `Sem Workload` (Ordonnancement et charges) | 992 | 1 261 | 955 | 163.51 | 50 |
| **perf11** | `SEM_CREATE` | 1 935 | 2 075 | 1 848 | 79.13 | 4 |
| **perf11** | `MUTEX_CREATE` | 1 786 | 1 933 | 1 635 | 151.12 | 4 |
| **perf11** | `SEMAPHORE_SIGNAL` | 155 | 231 | 125 | 21.94 | 232 |
| **perf11** | `SEMAPHORE_WAIT` | 155 | 241 | 138 | 23.09 | 232 |
| **perf11** | `SEMAPHORE_ID` | 556 | 793 | 316 | 181.60 | 232 |
| **perf11** | `SEM_STATUS` | 135 | 160 | 106 | 23.10 | 232 |
| **perf11** | `MUTEX_ACQUIRE` | 1 308 | 1 783 | 1 283 | 66.31 | 232 |
| **perf11** | `MUTEX_RELEASE` | 1 811 | 2 355 | 1 755 | 125.91 | 232 |
| **perf11** | `MUTEX_ID` | 476 | 640 | 321 | 120.61 | 232 |
| **perf11** | `MUTEX_STATUS` | 161 | 220 | 133 | 29.01 | 232 |
| **perf11** | `RESET_MUTEX` | 1 925 | 2 203 | 1 895 | 22.72 | 232 |
| **perf14** | `ADPCM` (Algorithme de compression audio) | 10 423 481 | 10 427 250 | 10 420 763 | 7 662.91 | 102 |
| **perf15** | `DIJKSTRA` (Algorithme de plus court chemin) | 13 830 981 | 13 841 006 | 13 797 075 | 10 033.11 | 206 |
| **perf16** | `SOBEL` (Algorithme de filtrage Sobel) | 25 932 453 | 25 939 031 | 25 926 875 | 3 039.00 | 206 |
| **perf17** | `APEX APP` (Application APEX) | 3 646 | 5 725 | 3 590 | 183.41 | 272 |
| **perf18** | `APEX APP` (Application APEX - Config 2) | 6 930 | 10 648 | 6 865 | 335.28 | 213 |
| **perf13** | `PARTITION SWITCH` (Changement de partition) | 2 955 | 3 640 | 2 508 | 198.57 | 100 |
| **perf19 (A)** | `SAMPLE_APEX_APP_A` (Partition 1) | 342 286 470 | 342 286 470 | 342 286 470 | 0.00 | 1 |
| **perf19 (B)** | `SAMPLE_APEX_APP_B` (Partition 2 - 13k it.) | 8 630 330 | 8 630 330 | 8 630 330 | 0.00 | 1 |

---

## Test perf12 : Suite Complète de Benchmarks Système (UCX-OS / ARINC 653)

Ce test mesure l'ensemble des primitives de l'OS (Initialisation, Création, Partitionnement, Mutex, Sémaphores, Mémoire et Communications par Ports/Queues).

| Primitives / API | Moyen (ns) | WCET (ns) | BCET (ns) | Écart-Type (ns) | Échantillons |
|---|---|---|---|---|---|
| **PRINT_INIT** | 2 451 | 2 451 | 2 451 | 0.00 | 1 |
| **DUMMY_INIT1** | 2 210 | 2 210 | 2 210 | 0.00 | 2 |
| **SEM_CREATE** | 1 913 | 2 090 | 1 728 | 0.00 | 4 |
| **BUFFER_CREATE** | 2 555 | 2 650 | 2 465 | 0.00 | 4 |
| **BLACKBOARD** (Create/Write) | 2 591 153 | 2 603 385 | 2 578 923 | 0.00 | 2 |
| **MALLOC** | 336 | 570 | 285 | 28.30 | 124 |
| **CREATE_EVENT** | 1 818 | 2 013 | 1 685 | 0.00 | 4 |
| **CREATE_QUEUING_PORT** | 2 348 | 2 743 | 2 098 | 0.00 | 4 |
| **CREATE_SAMPLING_PORT** | 1 373 | 1 488 | 1 288 | 0.00 | 4 |
| **PART_STATUS** | 91 | 130 | 90 | 18.07 | 120 |
| **LOCK** | 4 505 | 10 923 | 1 883 | 2 007.97 | 120 |
| **UNLOCK** | 4 115 | 5  845 | 2  370 | 1 006.74 | 120 |
| **BBoard16** (Write) | 405 | 553 | 368 | 54.88 | 120 |
| **ReadBBoard16** | 486 | 633 | 450 | 53.73 | 120 |
| **ReadBBoard64** | 961 | 1 038 | 941 | 51.72 | 120 |
| **ReceiveBuffer16** | 7 842 268 | 11 155 825 | 2 076 776 | 2 609 289.50 | 120 |
| **SEMAPHORE_SIGNAL** | 161 | 253 | 116 | 32.76 | 120 |
| **SEMAPHORE_WAIT** | 150 | 220 | 130 | 16.66 | 120 |
| **SET_PRIORITY** | 3 890 | 8 631 | 2 478 | 1 702.06 | 120 |
| **MY_ID** | 96 | 163 | 86 | 23.58 | 120 |
| **PROCESS_ID** | 358 | 398 | 338 | 30.42 | 120 |
| **PROCESS_STATUS** | 240 | 303 | 220 | 25.10 | 120 |
| **SEMAPHORE_ID** | 568 | 830 | 295 | 169.22 | 120 |
| **SEM_STATUS** | 148 | 173 | 110 | 24.52 | 120 |
| **EVENT_SET** | 175 | 228 | 120 | 29.87 | 120 |
| **EVENT_ID** | 591 | 855 | 340 | 171.10 | 120 |
| **STATUS_EVENT** | 150 | 213 | 103 | 26.39 | 120 |
| **EVENT_WAIT** | 160 | 265 | 126 | 29.80 | 120 |
| **WRITE_MEM** | 113 241 | 118 268 | 112 533 | 1 191.33 | 120 |
| **READ_MEM** | 40 | 78 | 40 | 10.38 | 120 |
| **GET_TICKS** | 58 | 83 | 55 | 11.61 | 120 |
| **SAMPLING_PORT_ID** | 400 | 521 | 275 | 63.88 | 120 |
| **SAMPLING_PORT_STATUS** | 143 | 206 | 100 | 31.06 | 120 |
| **TEST_QUEUE_STATUS** | 145 | 216 | 106 | 26.51 | 120 |
| **TEST_QUEUE_ID** | 310 | 433 | 203 | 69.90 | 120 |
| **WRITE_QUEUE** | 688 | 855 | 601 | 55.52 | 60 |
| **READ_QUEUE** | 530 | 680 | 503 | 12.48 | 60 |
| **WRITE_SAMPLING** | 581 | 685 | 526 | 48.90 | 60 |
| **READ_SAMPLING** | 2 950 615 | 3 032 131 | 2 866 425 | 81 370.96 | 60 |

---

## Détails des Benchmarks Individuels (perf1 à perf10)

### Test perf1 : Changement de contexte (`CNTX SWITCH`)
* **Nombre d'échantillons** : 499 996
* **Temps d'exécution (Ticks)** :
  * Moyen : 1201 ticks | Pire : 3099 ticks | Meilleur : 1159 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 2001 ns | Pire : 5165 ns | Meilleur : 1931 ns | **Écart-type** : 116.58 ns

### Test perf2 : Mutex (Ordonnancement croisé)
#### 1. Mutex release unblock
* **Moyen** : 2884 ns | **WCET** : 3145 ns | **BCET** : 2861 ns | **Écart-type** : 555.33 ns

### Test perf3 : Mutex (Acquisition et Libération simples)
#### 1. Mutex acquisition
* **Moyen** : 1050 ns | **WCET** : 1636 ns | **BCET** : 1048 ns | **Écart-type** : 77.40 ns
#### 2. Mutex release
* **Moyen** : 1475 ns | **WCET** : 2241 ns | **BCET** : 1471 ns | **Écart-type** : 117.28 ns

### Test perf4 : Mutex PIP (Priority Inheritance Protocol - 4999 échantillons)
* **Moyen** : 7385 ns | **WCET** : 9548 ns | **BCET** : 7385 ns | **Écart-type** : 1575.67 ns

### Test perf5 : Mutex PIP (Priority Inheritance Protocol - 49 échantillons)
* **Moyen** : 7546 ns | **WCET** : 7926 ns | **BCET** : 7385 ns | **Écart-type** : 446.12 ns

### Test perf6 : Latence de temporisation de tâche (`Task Delay Latency`)
* **Note** : Contient un décalage de -100 ms (-60.5M ticks) en raison de la différence entre la temporisation cible (500 ms) et l'appel de veille effectif (400 ms).

### Test perf7 : Sémaphore
#### 1. Semaphore Wait Block
* **Moyen** : 1393 ns | **WCET** : 1508 ns | **BCET** : 1388 ns | **Écart-type** : 131.11 ns
#### 2. Semaphore Signal Unblock
* **Moyen** : 823 ns | **WCET** : 865 ns | **BCET** : 823 ns | **Écart-type** : 93.02 ns

### Test perf8 : Sémaphore (Signalement avec Priorité)
* **Moyen** : 870 ns | **WCET** : 943 ns | **BCET** : 861 ns | **Écart-type** : 133.48 ns

### Test perf9 : Sémaphore (Opérations Simples sans blocage)
#### 1. Semaphore Signal
* **Moyen** : 111 ns | **WCET** : 338 ns | **BCET** : 101 ns | **Écart-type** : 10.63 ns
#### 2. Semaphore Wait
* **Moyen** : 110 ns | **WCET** : 120 ns | **BCET** : 105 ns | **Écart-type** : 2.00 ns

### Test perf10 : Sémaphore Workload
* **Moyen** : 992 ns | **WCET** : 1261 ns | **BCET** : 955 ns | **Écart-type** : 163.51 ns

### Test perf14 : ADPCM (Algorithme de compression audio)
* **Nombre d'échantillons** : 102
* **Temps d'exécution (Ticks)** :
  * Moyen : 6 254 089 ticks | Pire : 6 256 350 ticks | Meilleur : 6 252 458 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 10 423 481 ns (~10.42 ms) | Pire : 10 427 250 ns | Meilleur : 10 420 763 ns | **Écart-type** : 7 662.91 ns

### Test perf15 : DIJKSTRA (Algorithme de plus court chemin)
* **Nombre d'échantillons** : 206
* **Temps d'exécution (Ticks)** :
  * Moyen : 8 298 589 ticks | Pire : 8 304 604 ticks | Meilleur : 8 278 245 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 13 830 981 ns (~13.83 ms) | Pire : 13 841 006 ns | Meilleur : 13 797 075 ns | **Écart-type** : 10 033.11 ns

### Test perf16 : SOBEL (Algorithme de filtrage Sobel)
* **Nombre d'échantillons** : 206
* **Temps d'exécution (Ticks)** :
  * Moyen : 15 559 472 ticks | Pire : 15 563 419 ticks | Meilleur : 15 556 125 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 25 932 453 ns (~25.93 ms) | Pire : 25 939 031 ns | Meilleur : 25 926 875 ns | **Écart-type** : 3 039.00 ns

### Test perf17 : APEX APP (Application APEX)
* **Nombre d'échantillons** : 272
* **Temps d'exécution (Ticks)** :
  * Moyen : 2188 ticks | Pire : 3435 ticks | Meilleur : 2154 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 3646 ns | Pire : 5725 ns | Meilleur : 3590 ns | **Écart-type** : 183.41 ns

### Test perf18 : APEX APP (Application APEX - Config 2)
* **Nombre d'échantillons** : 213
* **Temps d'exécution (Ticks)** :
  * Moyen : 4158 ticks | Pire : 6389 ticks | Meilleur : 4119 ticks
* **Temps d'exécution (Nanosecondes)** :
  * Moyen : 6930 ns | Pire : 10648 ns | Meilleur : 6865 ns | **Écart-type** : 335.28 ns

### Test perf13 : PARTITION SWITCH (Commutation de partition)
* **Nombre d'échantillons** : 100
* **Temps d'exécution (Ticks)** :
  * Moyen : 1750 ticks | Pire : 2184 ticks | Meilleur : 1505 ticks
* **Temps d'exécution (Nanosecondes / Microsecondes)** :
  * Moyen : 2955 ns (2.95 µs) | Pire : 3640 ns (3.64 µs) | Meilleur : 2508 ns (2.51 µs) | **Écart-type** : 198.57 ns

### Test perf19 : Application APEX multi-processus et inter-partition
Ce benchmark mesure l'application APEX complète communicant entre deux partitions (Partition 1 et Partition 2).
* **SAMPLE_APEX_APP_A** (Mesure globale de la Partition 1, incluant la boucle de monitoring et d'envoi CRC) :
  * **Nombre d'échantillons** : 1
  * **Temps d'exécution (Ticks)** : 205 371 882 ticks
  * **Temps d'exécution (Nanosecondes)** : 342 286 470 ns (~342.28 ms)
* **SAMPLE_APEX_APP_B** (Mesure de la multiplication de matrices et écriture disque simulée de la Partition 2 - réduite à 13 331 itérations de boucle d'écriture disque) :
  * **Nombre d'échantillons** : 1
  * **Temps d'exécution (Ticks)** : 5 178 198 ticks
  * **Temps d'exécution (Nanosecondes)** : 8 630 330 ns (~8.63 ms)

