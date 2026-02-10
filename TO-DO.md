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
    

* fixe pb corruption mémoire une fois que la soumission de l'article est passée
    * faire en sorte de lever exeption si écriture interdite ou débordement
    * memory violation (MMU, MPU)
    * CPU exception/fault
    * Kernel exception handler
    * routine dumb qui affiche pb de memory

### static scheduler

* faire scheduler pour les partitions:
    * regarder comment implémenter les temps idle
    * faire du contrôle d'erreur sur les fonctions:
        * module_scheduler_init
        * partition_scheduler
            * regarder si la current partition est idle
                * faire une fonction deactivate partition ?
        * activate partition
        * signal partition
    * regarder comment ils font sur POK
    * faire test plus violents

* caractéristiques:
    * si partition is IDLE alors elle ne peut pas être schedulé
    * module schedule doit suivre la conf XML pour l'ordre, le début et la durée des partitions
    * il doit satisfaire les attributs comme partition period, duration...
    * la somme des partitions times windows doit être sup ou égale à la duration pendant une partition period


## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234