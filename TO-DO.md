# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * faire les actions quand elle passe en normal
        * -- a DEADLINE_TIME calculation may cause an overflow of the underlying -- clock. If this occurs, HM is invoked with an illegal request error code set the partition’s lock level to zero;
        * if (an error handler process has been created) then enable the error handler process for execution and fault processing;

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


## Warning
* beaucoup d'utilisation de malloc, voir si necessaire d'enlever tous les mallocs
## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234

* penser a retirer le -g (mode debug) des cflags dans le make 