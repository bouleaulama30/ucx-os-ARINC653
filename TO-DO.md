# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * faire les actions quand elle passe en normal
        * -- a DEADLINE_TIME calculation may cause an overflow of the underlying -- clock. If this occurs, HM is invoked with an illegal request error code set the partition’s lock level to zero;
        * if (an error handler process has been created) then enable the error handler process for execution and fault processing;

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
    * replacer les fonctions utilent pour faire l apex comme find_first_release_point a des endroits plus coherent dans le code
    * voir s il faut mettre les flags booleen en maj   
    * placer les return error a la fin des fonctions apex 
    * voir pour mettre les calcules d overflow dans des fonctions genre deadline overflow ou time out overflow cf apex processes
    * voir pour faire un signal schedule needed pour eviter de reschedule a chaque fois que ce n est pas necessaire tout le temps
    * voir si les fonctions du pos pour gerer les timer, delay etc.. voir si les mettre dans un autre fichier ou autre
    * faire en sorte d appeler les fonctions de check des delay et des timeout uniquement a chaque tick et non a chaque appel au scheduler car on fait du temps de calcul inutil
    * a la place de call ucx_uptime a chaque fois dans l'apex des processes voir pour get une seule fois le current time au debut de la fonction et ensuite l'utiliser pour toute la fonction
    * notamment dans set_partition_mode voir pour faire des fonctions pour voir si les processes sont aperiodic not delay not suspend ou aperiodic delay ou periodic not delay etc...
    * mettre un linter C en place
    * enlever repetition check_and_release_periodic_waiting_processes
    * enlever les commentaires inutiles surtout ceux de copilot 
    * voir dans set_operating_mode pour mettre le code pour vider une partition de ces processes dans une fonction
    * enlever les print f qui sont trop lourd
    * faire une fonction update deadline ou le check de si le time capacity est infini sera effectue comme ca les process avec infinite time capacity rouleront sans pb:
    void update_process_deadline(struct process_s *process, SYSTEM_TIME_TYPE base_time) {
    if (process->processus_status->ATTRIBUTES.TIME_CAPACITY == INFINITE_TIME_VALUE) {
        // La norme exige que si la capacité est infinie, la deadline devient infinie
        process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;
    } else {
        // Calcul normal
        process->processus_status->DEADLINE_TIME = base_time + process->processus_status->ATTRIBUTES.TIME_CAPACITY;
    }
}

* merge la branche dev sur la main
* faire rouler sur la carte en materielle une fois que c'est bon toute la norme

## intra partition communication
* coder tout ce qui est relatif au lock preemption mutex pour les process

            * get_process_status cf si le process possede le lock preemption alors renvoyer la priorite max pour la current_priority
            * set_priority voir dans le cas ou le process own a mutex et ne pas rescheduler s'il n a pas de preemption lock
            * stop self, liberer les ressources comme le mutex ou autre
            * stop  gerer les mutex, remove process de la waiting queue s il etait dans l etat waiting
            * suspend  gerer le lock mutex/mutex
            * suspend self  gerer le lock mutex/mutex
            * resume  checker s il attend avec un time wait etc..., gerer le lock mutex et le check avec les ressources et les timer
            * TIMED_WAIT:
                * gerer le cas des mutexes
            * PERIODIC_WAIT:
                * gerer le cas des mutexes
            * REPLENISH:
                * gerer le cas avec le error handler
            * plus tard repassser sur toute les fonctions pour gerer les processes queue quand elles seront implem
            * implem la waiting queue (quand on aura implem la partie intra et inter communication)



## HM
* faire en sorte de mettre toutes les metriques en ns car le lsb de system_time_type est 1 ns
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

* penser a retirer le -g (mode debug) des cflags dans le make 