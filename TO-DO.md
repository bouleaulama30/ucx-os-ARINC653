# TO-DO

## matériel
* voir pour mettre les includes des tests autre part
* faire la doc pour les flags a mettre dans softconsole
* faire des virtuals links pour polarfire de l os pour ne pas a avoir a modifier les fichiers deux fois en cas de changements
* merge toutes les branches sur la main (polarfire et testbench)
* faire de la doc pour integrer le portage sur la carte clean dans le projet
* faire clean up des trucs qui ne servent a rien
* faire une integration clean
* Issue testperf12: dans list_push le malloc fail, c est le probleme d avoir des processes avec mallocs (si ca pose beaucoup de probleme dans la suite envisager de changer)

## refactoring

* enlever les reference aux communication port dans partition init 
* mettre des commentaires
* faire de la documentation

<!-- à faire après test unitaire -->
* sampling_port_s -> sp_s ?
* voir si pour les ressources comme bb, la reference vers la partition ou le waiting_blackboard est necessaire dans la struct
* voir pour mettre krnl acquire et release mutex dans un fichier du krnl
* voir pour mettre des fonctions lié aux channels dans la comm inter partition dans les fichiers du noyau

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

## remind debug
* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234

* penser a retirer le -g (mode debug) des cflags dans le make 