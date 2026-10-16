# Hardware Testbench
Hardware testbench is a real avionic application composed with two core (**ucx-os-ARINC653** and **BM Microship system**) on **AMP** architecture.

## liste étape à décrire

- prerequis
    - mettre design générale sur la carte via libero soc ([github](https://github.com/polarfire-soc/polarfire-soc-discovery-kit-reference-design))

    - faire fonctionner amp sur la carte [github](https://github.com/polarfire-soc/polarfire-soc-amp-examples/tree/main/mpfs-rpmsg-bm)
        - HSS à build [github hss](https://github.com/polarfire-soc/hart-software-services)
        - HSS à mettre sur la carte [github tool flow](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/knowledge-base/polarfire-soc-software-tool-flow.md#programming-the-envm) et [video tuto](https://www.microchip.com/en-us/about/media-center/videos/caNsH4dDnLU) à adapter à la polarfire soc discovery kit pour les dossiers si besoin
        - installer hss payload (merge les elf en un bin) [github](https://github.com/polarfire-soc/hart-software-services/blob/master/tools/hss-payload-generator/README.md)
    - installer l'outil pour flash avec USBImager
    - installer Tera Term TV pour l'UART

- Préparation des executable (si besoin de faire des modif des applications)
    - mettre l'app avionics au bon endroit dans ucx-os (code arinc_app_altitude.c dans `arch/riscv/PolarFire/src/app/app_main.c` et les fichiers static conf .c et .h associés)
    - mettre la bonne app spi ou fpga dans u54_1.c sur hart1 dans l'os de microship (autre que repo github  j'ai mpfs-spi-master-slave)

- Mettre les applications sur la carte
    - Faire un binaire avec deux elf avec le hss payloadmachin (pas besoin si rien à modif car le binaire est déjà dans le repo et idem les elf aussi)
    - flash le binaire sur une carte micro sd avec usbimager
    - mettre la micro sd dans la carte
    - ouvrir les uart com4, 5, 6 avec 115200 de baudrate sur tera term tv
    - faire la commande "RESET" sur le menu du bootloader

- Pour le SPI
    - faire un schema ou explication du shema electrique ou des pins (les vérifier si c bon sur le readme)

- Pour le FPGA (update le readme pour ca)
    - dire c'est quel fichier mathlab à ouvrir (cf repo arinc429_ip)
    - expliquer comment faire le projet sur libéro soc (importer l'ip via tcl) ou voir si possible de git le projet avec le bon dossier projet (repo arinc429_ip)