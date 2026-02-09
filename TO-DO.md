# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition;
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)

* refractor / formatage code:
    * regarder pok pour s'inspirer
    

* fixe pb corruption mémoire une fois que la soumission de l'article est passée
    * faire en sorte de lever exeption si écriture interdite ou débordement

### static scheduler

* faire scheduler pour les partitions:
    * faire activate partition (voir pour changer le nom):
        * suivre commentaire sur la fonction
    * faire du contrôle d'erreur sur les fonctions:
        * module_scheduler_init
        * partition_scheduler
        * activate partition
    * regarder comment implémenter les temps idle
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