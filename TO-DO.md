# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)


* faire management process
    * terminer la fonction create_process
        * gerer le cas de IDLE pour le scheduling (gerer la pmp car ce n est pas bon)
            * demander si je peux faire un processus qui est idle si c est le cas remettre activate partition bien comme avant car plus besoin de faire des disjonction de cas
        * attention au _interrupt_tick (voir si necessaire, en faire un sinon)
    * faire un main process clean

    * faire un main diff pour chaque partition:
        * faire en sorte que la partition trampoline fasse ce qu'il faut pour creer le main process
        * terminer de completer le set_operating mode car c est elle qui fait basculer
        * retirer le main process de la liste des processes
    
    * faire en sorte que finalement on appelle dispatch pour les processes sans repasser par le while(1) de la partition trampoline qui est enfaite un dispatch deguise (voir si c est pertinent)

    * comment defini statiquement la stack des processus:
        * faire un pointeur que l on incremente statique a partir de la section data
        
    * comment etre sur que les processus utilise pas plus que leur stack
        * soit utiliser le canary actuel, mais sinon adapter la conf de la pmp/MMU
        
    * completer les "a changer" dans la fonction create_process et ucx_process_spawn
        * garder coherence entre avec le nom des etats et des priorites dans ucx_process_spawn pour fit avec la norme

    * faire un scheduling clean et coherent avec l'apex
        * voir avec Felipe car il faut changer le scheduler de base je pense
        * voir si on passe en interne aux partitions avec un POS (necessite de changer le paradigme et le scheduler de base)
    
    * coder les fonctions de l'APEX sur les processes
        * utiliser et adapter les syscall deja present comme ucx_task_id   

    * tester les fonctions de l'APEX sur les processes

* refractor / formatage code:
    * mettre des ifndef pour les ajouts a l os de base pour maintenir le code isole:
        * main, struct kcb, ucx.h, process.c.h, fonction dispatch, interrupt_tick dans le hal, yield
    * trouver des noms plus accurate pour activate partition et les variables dedans
        * lier les noms entre task et processes
    * refractor de partionnement spatial
    * refractor de partionnement temporelle
    * refractor tout sur les processes
    * regarder pok pour s'inspirer
    * remplacer certaines action par des fonctions get et set  (comme fonction pour get le node_s de la partition courante)
    
* faire rouler sur la carte en materielle une fois que c'est bon pour les processus

## HM

* implem is_executing_error_handler

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