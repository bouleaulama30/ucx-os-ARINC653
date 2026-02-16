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
    
* pb1: Chaque interruption timer fonction de l'os (krnl_dispatcher, krnl_scheduler, activate_partition) utilisent la stack de l'OS et de maniere general pour toutes fonction de l os ca ce fera dans la partition qui la call 
* pb2: OS tourne en mode Machine pour riscv, pas design pour mode user, donc utilisation du MPRV+MPP donc techniquement partitions pas faire acces memoire mais peuvent call une fonctions.
* pb3: Partition peuvent faire lecture et ecriture au kernel, c est le pb qu il n y a pas de syscall qui font un contexte switch et qui delegue tout au kernel

* faire rouler sur la carte en materielle une fois que c'est bon pour les partitions

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234