# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)


* faire management process
    * terminer la fonction create_process
        * tester pour le moment le workflow avec les syscall deja existant et les processes dans la stack
        * attention au interrupt tick
        
    * comment defini statiquement la stack des processus:
        * faire un pointeur que l on incremente statique a partir de la section data
        
    * comment etre sur que les processus utilise pas plus que leur stack
        * soit utiliser le canary actuel, mais sinon adapter la conf de la pmp/MMU
        
    * completer les "a changer" dans la fonction create_process et ucx_process_spawn
        * garder coherence entre avec le nom des etats et des priorites dans ucx_process_spawn

    * faire un scheduling clean et coherent avec l'apex
        * tester avec le scheduler de base
        * voir si on passe en interne aux partitions
        
    
    * coder les fonctions de l'APEX sur les processes
        * utiliser et adapter les syscall deja present comme ucx_task_id   

    * tester les fonctions de l'APEX sur les processes

* refractor / formatage code:
    * mettre des ifndef pour les ajouts a l os de base pour maintenir le code isole:
        * main, struct kcb, ucx.h, process.c.h, fonction dispatch, interrupt_tick dans le hal 
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