# TO-DO

* faire rouler sur la carte en materielle une fois que c'est bon toute la norme

## Partition implem

* set_operating_mode:
    * -- a DEADLINE_TIME calculation may cause an overflow of the underlying -- clock. If this occurs, HM is invoked with an illegal request error code set the partition’s lock level to zero;

## Process
* refaire un test de suspend/self, resume, stop/self pour voir si c'est toujours ok:
    * le faire aussi quand il y a un mutex ou une ressource en attente

## partition communication

### inter communication

### intra communication

## HM

* definir dans la conf static la table au niveau process, partition et module (conforme avec les errors recovery action possiblent)

* create error handler process:
    * configure this partition so that processes on other processor cores do not make progress (i.e., pause) when the error handler process is scheduled; (a faire quand on fera du multi core)

* gerer le cas ou l'irq handler catch l erreur et que ca vient de l os (pas de partition courante et tout) alors faire un panic de l os ( sinon ca va loop sur les erreurs)

* faire en sorte de mettre toutes les metriques en ns car le lsb de system_time_type est 1 ns

* voir ce qu on fait de START_CONDITION_TYPE et du demarrage de la partition (si elle était restart par le hm ou demarrage normal)

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
* l'irq handler pour la traduction des erreurs os en arinc

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