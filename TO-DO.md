# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)


* faire un kos qui initialise tout ce qui faut niveau du pos (partitions etc), il faut que le scheduler et les fonctions à chaque timer soient dans le kos (redemander à Felipe une description des deux) a l execution et non dans la stack des partitions

* faire management process
    * comment etre sur que les processus utilise pas plus que leur stack


* refractor / formatage code:
    * trouver des noms plus accurate pour activate partition et les variables dedans
    * refractor de partionnement spatial
    * refractor de partionnement temporelle
    * regarder pok pour s'inspirer
    * remplacer malloc par ucx_malloc
    * remplacer certaines action par des fonctions get et set  (comme fonction pour get le node_s de la partition courante)
    
* faire rouler sur la carte en materielle une fois que c'est bon pour les processus

## Choses dependante de l archi
* protection memoire avec pmp_activate et mprv_activate (lie a riscv32)
* logique des longjmp et setjmp dans l'irq_handler afin d'executer les fonctions de scheduling dans la stack du kernel

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234