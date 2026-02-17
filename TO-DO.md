# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)


* faire un kos qui initialise tout ce qui faut niveau du pos (partitions etc), il faut que le scheduler et les fonctions à chaque timer soient dans le kos (redemander à Felipe une description des deux) a l execution et non dans la stack des partitions

* faire management process
    * relire la partie process


* refractor / formatage code:
    * trouver des noms plus accurate pour activate partition et les variables dedans
    * refractor de partionnement spatial
    * refractor de partionnement temporelle
    * regarder pok pour s'inspirer
    * remplacer malloc par ucx_malloc
    
* faire rouler sur la carte en materielle une fois que c'est bon pour les processus

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234