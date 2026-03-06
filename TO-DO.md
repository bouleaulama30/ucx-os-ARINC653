# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition
        * changer ucx_yield pour fit avec les process
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes (le faire avec l apex start), activate process scheduling etc..)
    * faire de nouveau un test complet de la fonction pour savoir si c est fonctionnel apres avoir implementer la logique des processes


* faire management process                    
    * coder les fonctions de l'APEX sur les processes
        * stop
            * gerer les mutex
            * remove process de la waiting queue s il etait dans l etat waiting
            * stop any time counters associated with the specified process;
            * prevent the specified process from causing a deadline overrun fault;
        * stop self
            * cancel le timer affilier a la tache courante pour eviter une faute plus tard
        * redistribuer les points non coder (comme preemption lock mutex) vers les autres to do
            * get_process_status cf si le process possede le lock preemption alors renvoyer la priorite max pour la current_priority
            * set_priority voir dans le cas ou le process own a mutex et ne pas rescheduler s'il n a pas de preemption lock
            * stop self, liberer les ressources comme le mutex ou autre

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
    voir si la structure tcb dans celle de process_s est toujours utile (etat dont on ne se sert plus comme tcb.state)
    * revoir l ordre de declaration des fonctions dans les fichier
    * voir si find_processes_by_id et les autres fonctions on les met dans process.c ou on les laisse dans arinc_partition.c
    * ce morceau de code  pour avoir la partition courante est souvent present, voir pour faire une fonction (get_current_partition) 
    * le morceau de code pour save la tache courante et revenir a la boucle du scheduler est souvent presente, donc faire une fonction
    * mettre/enlever des commentaires
    
* faire une batterie de test unitaire que l'on peut test a chaque fois et qui couvre au maximum les partition et les processes

* faire rouler sur la carte en materielle une fois que c'est bon pour les processus

## intra partition communication
* coder tout ce qui est relatif au lock preemption mutex pour les process

## HM

* implem is_executing_error_handler pour get_my_id et get_my_index
* implem is_main_process (trouver une condition necessaire et suffisante genre il n y a pas de current process) pour get_my_id et get_my_index

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