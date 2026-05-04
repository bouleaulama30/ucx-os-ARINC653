# TO-DO

* faire rouler sur la carte en materielle une fois que c'est bon toute la norme

## Partition implem

* implémentation set_operating_mode:
    * faire les actions quand elle passe en normal
        * -- a DEADLINE_TIME calculation may cause an overflow of the underlying -- clock. If this occurs, HM is invoked with an illegal request error code set the partition’s lock level to zero;
        * if (an error handler process has been created) then enable the error handler process for execution and fault processing;

## Process

* Lock_preemption:
    * current process is error handler process

* Unlock_preemption:
    * current process is error handler process

* suspend self  error handler

* refaire un test de suspend/self, resume, stop/self pour voir si c'est toujours ok:
    * le faire aussi quand il y a un mutex ou une ressource en attente

## partition communication
* TIMED_WAIT:
    * gerer le cas error handler
* PERIODIC_WAIT:
    * gerer le cas error handler
* REPLENISH:
    * gerer le cas avec le error handler

### inter communication
* faire allocation des spaces (cf message felipe pour les ports) comme pour le intrapartition
* SEND_QUEUING_MESSAGE:
    * voir current process error handler
* RECEIVE_QUEUING_MESSAGE:
    * voir current process error handler

### intra communication
* send buffer:
    * to do error handler

* receive buffer:
    * to do error handler

* read bb:
    * to do error handler

* wait semaphore:
    * to do error handler

* wait_event:
    * to do error handler

* wait_periodic_event:
    * current process is error handler process

* Mutex:
    * mutex acquire:
        * when (current process is the error handler process) =>
    * mutex reset:
        * and the current process is not the error handler process) =>
        * when (the current process is not the error handler process and the current process is not the identified process) => -- mutex can only be reset by the error handler or ownin
## HM
* implem l'apex du HM et la test

* create error handler process:
    * configure this partition so that processes on other processor cores do not make progress (i.e., pause) when the error handler process is scheduled; (a faire quand on fera du multi core)
    * deplacer le contexte init dans le raise application_error
* gestion de la list des error processes:
    * ajouter un pending_error_code pour les processes et laisser l"os ajouter l'erreur dans la liste au moment du get error (cf gemini)
    
* adapter le reste de l apex a ca
* faire les fonctions kernel pour gerer les erreurs dans le cas ou il n y a pas de error process:
    * passage de l erreur au niveau partition 
    * passage de l erreur au niveau module
* gerer le cas ou l'irq handler catch l erreur (pour la rediriger vers les bonnes fonctions)
* faire en sorte de mettre toutes les metriques en ns car le lsb de system_time_type est 1 ns
* implem is_executing_error_handler pour get_my_id et get_my_index
* implem is_main_process (trouver une condition necessaire et suffisante genre il n y a pas de current process) pour get_my_id et get_my_index

## Optimisation

* retirer les mallocs et les free en utilisant le pool allocator utiliser durant la communication inter et intra partition
* ajouter un mecanisme pour dire a une partition que des ressources sont dispo pour une autre partition comme ca on ne verifie pas a chaque tick les ressources des port

* essayer de rassembler les parcours de liste a chaque tick en une seule fonction
* appeler les checks dynamiquement selon la period la plus petite pour eviter les parcours de liste a chaque tick

## refactoring

* enlever les reference aux communication port dans partition init 
* sampling_port_s -> sp_s ?
* mettre des commentaires
* voir si pour les ressources comme bb, la reference vers la partition ou le waiting_blackboard est necessaire dans la struct
* voir pour mettre krnl acquire et release mutex dans un fichier du krnl

## Choses dependante de l archi
* protection memoire avec pmp_activate et mprv_activate (lie a riscv32)
* logique des longjmp et setjmp dans l'irq_handler afin d'executer les fonctions de scheduling dans la stack du kernel


## Warning
* beaucoup d'utilisation de malloc, voir si necessaire d'enlever tous les mallocs
    * mais alors comment faire pour les liste et tout
## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234

* penser a retirer le -g (mode debug) des cflags dans le make 