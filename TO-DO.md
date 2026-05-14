# TO-DO

## matériel
* essayer de faire rouler l'os sur la carte de Felipe

## test
* faire une batterie de test unitaires pour voir si le comportement de toute les fonctions de l'apex est correct
    * arinc TIME
    * arinc intercommunication 
    * arinc intracommunication
    * arinc HM

## refactoring

* enlever les reference aux communication port dans partition init 
* mettre des commentaires
* faire de la documentation
* faire deux autres app de demo:
    * faire une app ou on communique entre partition
    * faire une app avec les resume suspend etc... (reprendre celle que l on a deja)

<!-- à faire après test unitaire -->
* sampling_port_s -> sp_s ?
* voir si pour les ressources comme bb, la reference vers la partition ou le waiting_blackboard est necessaire dans la struct
* voir pour mettre krnl acquire et release mutex dans un fichier du krnl

## Optimisation

* retirer les mallocs et les free en utilisant le pool allocator utiliser durant la communication inter et intra partition

* ajouter un mecanisme pour dire a une partition que des ressources sont dispo pour une autre partition comme ca on ne verifie pas a chaque tick les ressources des port

* essayer de rassembler les parcours de liste a chaque tick en une seule fonction

* appeler les checks dynamiquement selon la period la plus petite pour eviter les parcours de liste a chaque tick

## Partition implem
## Process
## partition communication
### inter communication
### intra communication
## HM

* create error handler process:
    * configure this partition so that processes on other processor cores do not make progress (i.e., pause) when the error handler process is scheduled; (a faire quand on fera du multi core)

* gerer le cas ou l'irq handler catch l erreur et que ca vient de l os (pas de partition courante et tout) alors faire un panic de l os ( sinon ca va loop sur les erreurs)

* faire en sorte de mettre toutes les metriques en ns car le lsb de system_time_type est 1 ns
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