# TO-DO

## banc de test sur amd
* installer vivado 
* mettre deux soft core sur une carte en utilisant le hf-riscv et les faire communiquer entre eux
* faire afdx en P4
* simuler et vérifier son comportement via l'outil
* générer le rtl via l'outil
* intégrer l'ip au banc de test


## documentation
* faire orga github pour les projets
* update doc pour les app testsbench
    * comment mettre un hss dedans
    * comment faire fonctionner l'app avionic (flow de travail) (via spi et carte holt)
    * les outils utilisé (uart, usb flash, hss payload)
    * mettre des refs au bon repo github avec les tuto pour comprendre amp etc...
    * ajouter le truc pour l'IP des cartes holt sur fpga et le flow libero soc
    * je veux plus tard pouvoir refaire l'app facilement limite mettre juste les binaires ou le elf dans un des repo comme ça c'est plus simple
    * ajouter la doc pour le mappage des pins et les branchements elec aussi 



## refactoring
* faire de la doc sur comment fonctionne le HM et le system
* mettre des commentaires

## Optimisation

* ajouter un mecanisme pour dire a une partition que des ressources sont dispo pour une autre partition comme ca on ne verifie pas a chaque tick les ressources des port

* essayer de rassembler les parcours de liste a chaque tick en une seule fonction

* appeler les checks dynamiquement selon la period la plus petite pour eviter les parcours de liste a chaque tick

* retirer les mallocs et les free en utilisant le pool allocator utiliser durant la communication inter et intra partition

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