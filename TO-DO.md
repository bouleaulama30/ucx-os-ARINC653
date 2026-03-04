# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)
    * faire de nouveau un test complet de la fonction pour savoir si c est fonctionnel apres avoir implementer la logique des processes


* faire management process                
    * completer les "a changer" dans la fonction create_process et ucx_process_spawn
        * garder coherence entre avec le nom des etats et des priorites dans ucx_process_spawn pour fit avec la norme
        * dans activate process scheduler a changer mais temporairement le premier process de la liste est mis en current par le main process

    
    * coder les fonctions de l'APEX sur les processes
        * voir si plus simple de faire get_process_id avec id ref ou autre
        * utiliser et adapter les syscall deja present comme ucx_task_id
    * tester les fonctions de l'APEX sur les processes

* refractor / formatage code:
    * mettre des ifndef pour les ajouts a l os de base pour maintenir le code isole:
        * main, struct kcb, ucx.h, process.c.h, fonction dispatch, interrupt_tick (process et partition) dans le hal, yield (enlever les doublons entre tasks et processes)
    * trouver des noms plus accurate pour activate partition et les variables dedans
        * lier les noms entre task et processes
    * refractor de partionnement spatial
    * refractor de partionnement temporelle
    * refractor tout sur les processes
    * regarder pok pour s'inspirer
    * remplacer certaines action par des fonctions get et set  (comme fonction pour get le node_s de la partition courante)
    * faire un nettoyage dans les structures de donnees aussi
    * voir si on garde le dispatch dans la logique du scheduling des partitions car c est archi dependant
    * voir si c'est bien de mettre le process_scheduler dans le fichier process.c
    
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