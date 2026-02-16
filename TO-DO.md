# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)

* refractor / formatage code:
    * trouver des noms plus accurate pour activate partition et les variables dedans
    * refractor de partionnement spatial
    * refractor de partionnement temporelle
    * regarder pok pour s'inspirer
    * remplacer malloc par ucx_malloc
    

* fixe pb corruption mémoire une fois que la soumission de l'article est passée
    * lire la ISA pour comprendre le fonctionnement de la PMP et du MPRV
    * test configuration de la PMP pour P1 et P2 avec le MPRV:
        * faire une fonction generique qui sera appele par le scheduler pour programmer dynamiquement les acces aux partitions
    * faire attention au bit MPRV qui ne va pas etre conserver pour traiter une interruption du timer par exemple
    * modifier le handler pour faire les etapes suivantes
    * faire en sorte de lever exeption si écriture interdite ou débordement
    * Kernel exception handler
    * routine dumb qui affiche pb de memory
    * enlever truc du commit avant pour la fonction de transition
    * comprendre ajout crt0

* faire rouler sur la carte en materielle une fois que c'est bon pour les partitions

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234