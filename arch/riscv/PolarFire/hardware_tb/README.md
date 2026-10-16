# Hardware Testbench
Hardware testbench is a real avionic application composed with two core (**ucx-os-ARINC653** and **BM Microship system**) on **AMP** architecture.

## liste étape à décrire

- prerequis
    - mettre design générale sur la carte via libero soc (donner le github)
    - faire fonctionner amp sur la carte (donner lien github/readme)
        -  HSS à mettre sur la carte (mettre lien github/readme)
        - installer hss payload (cf readme/github)
    - installer l'outil pour flash
    - installer l'outil pour l'uart

- Préparation des executable (si besoin de faire des modif des applications)
    - mettre l'app avionics au bon endroit dans ucx-os
    - mettre la bonne app spi ou fpga dans u54_1.c sur hart1 dans l'os de microship (autre repo github)

- Mettre les applications sur la carte
    - Faire un binaire avec deux elf avec le hss payload (pas besoin si rien à modif car le binaire est déjà dans le repo et idem les elf aussi)
    - flash le binaire sur une carte micro sd avec usb machin
    - mettre la micro sd dans la carte
    - ouvrir les uart com4, 5, 6 avec 115200 de baudrate sur tera machin
    - faire la commande "RESET" sur le menu du bootloader

- Pour le spi
    - faire un schema ou explication du shema electrique ou des pins

- Pour le FPGA 
    - expliquer comment faire le projet sur libéro soc (importer l'ip via tcl) ou voir si possible de git le projet avec le bon dossier projet