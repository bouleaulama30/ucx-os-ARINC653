#include <ucx.h>

// Déclaration des symboles du linker script de la p1
extern uint8_t _p1_code_start[];
extern uint8_t _p1_code_end[];

extern uint8_t _p1_data_start[];
extern uint8_t _p1_data_end[];

// Déclaration des symboles du linker script de la p2
extern uint8_t _p2_code_start[];
extern uint8_t _p2_code_end[];

extern uint8_t _p2_data_start[];
extern uint8_t _p2_data_end[];

__attribute__((section(".p2_code")))
void p1_process3(void)
{   
int32_t cnt = 300000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	
    // APEX_INTEGER other_process_id;
	PROCESS_STATUS_TYPE other_process_status;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);
	GET_PROCESS_STATUS(1, &other_process_status, &return_code);
	
	if(return_code == NO_ERROR){
		printf("Le nom du process: %s, with priority %d\n", other_process_status.ATTRIBUTES.NAME, other_process_status.CURRENT_PRIORITY);
	}
	while (1) {
		if(cnt % 2 == 0){
			// SET_PARTITION_MODE(IDLE, &return_code);
			printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
			SET_PRIORITY(1, 3, &return_code);
			// SET_PRIORITY(1, 3, &return_code);

		}
		
		if(cnt == 300001)
			SUSPEND(1, &return_code);
			// SET_PARTITION_MODE(COLD_START, &return_code);
		if(cnt == 300005)
			RESUME(1, &return_code);
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);

		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();

	}
}

__attribute__((section(".p2_code")))
void p2_process1(void)
{   
	int32_t cnt = 400000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);
	while (1) {
		if(cnt % 2 == 0){
			printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
			SET_PRIORITY(1, 1, &return_code);
			// SET_PRIORITY(0, 3, &return_code);
		}
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();

	}
}

// Test Suite 1 : Le "Round-Robin" et TIMED_WAIT (Time Management)
// Objectif : Vérifier que TIMED_WAIT(0) cède bien le CPU aux processus de même priorité, et que TIMED_WAIT(>0) endort correctement le processus .
// Configuration XML requise : Créer deux processus apériodiques (ProcA et ProcB) avec exactement la même priorité (ex: 50).
// Résultat attendu : L'affichage doit être strictement A B A B A B (ou B A B A B A), prouvant que la file FIFO des processus READY de même priorité fonctionne. Ensuite, B doit s'arrêter, A dort 50ms (le processeur passe en wfi), puis A se réveille et s'arrête.
__attribute__((section(".p1_code")))
void test_round_robin_A(void) {
    RETURN_CODE_TYPE ret;
    int compteur = 0;
    
    printf("[Test 1] Demarrage Proc A\n");
    while (compteur < 3) {
        printf("A\n");
        compteur++;
        // Cède la place à Proc B
        TIMED_WAIT(0, &ret); 
        if (ret != NO_ERROR) printf("Erreur TIMED_WAIT A: %d\n", ret);
    }
    
    printf("\n[Test 1] Proc A fait un vrai dodo de 50ms\n");
    TIMED_WAIT(50, &ret); // 50ms
    printf("[Test 1] Proc A reveille !\n");
    
    STOP_SELF();
}

__attribute__((section(".p1_code")))
void test_round_robin_B(void) {
    RETURN_CODE_TYPE ret;
    int compteur = 0;
    
    printf("[Test 1] Demarrage Proc B\n");
    while (compteur < 3) {
        printf("B\n");
        compteur++;
        // Cède la place à Proc A
        TIMED_WAIT(0, &ret); 
    }
    STOP_SELF();
}

// Test Suite 2 : La précision de PERIODIC_WAIT (Time Management)

// Objectif : Vérifier que l'OS ne dérive pas dans le temps (Time Drift) et que les "Release Points" sont calculés avec une précision absolue.

// Configuration requise : Un processus Périodique (Période = 100ms, soit 100 000 000 ns).
// Résultat attendu : La variable diff doit être imprimée et valoir exactement ou de façon extrêmement proche 100 000 000 (100ms), peu importe le temps pris par la boucle for. Cela prouve que votre Next release point est indépendant du temps d'exécution
__attribute__((section(".p1_code")))
void test_periodic_accuracy(void) {
    RETURN_CODE_TYPE ret;
    SYSTEM_TIME_TYPE t1, t2, diff;
    int cycles = 0;

    printf("[Test 2] Demarrage Proc Periodique (Periode=100ms)\n");

    while (cycles < 5) {
        // 1. On lit l'heure exacte du système
        GET_TIME(&t1, &ret);
        
        // 2. On fait un peu de "travail" bidon
        for(volatile int i=0; i<10000; i++); 
        
        // 3. On attend le prochain cycle
        PERIODIC_WAIT(&ret);
        
        // 4. On relit l'heure dès le réveil !
        GET_TIME(&t2, &ret);
        
        diff = t2 - t1;
        printf("[Test 2] Cycle %d: Temps ecoule = %ld ns\n", cycles, diff);
        cycles++;
    }
    STOP_SELF();
}

// Test Suite 3 : Les transitions mortelles START, SUSPEND, STOP (Process Management)

// Objectif : Vérifier la machine à états APEX, la protection contre les doubles START, et la mise en attente forcée.

// Configuration requise : Un Processus "Maître" (Priorité Haute = 100), un Processus "Esclave" (Priorité Basse = 50, initialisé en DORMANT).
// Résultat attendu : Le Maître doit manipuler l'Esclave comme une marionnette. L'Esclave affiche "Je tourne...", puis s'arrête (Suspension), reprend (Resume), puis meurt définitivement (Stop). Si setjmp/longjmp est mal géré dans les changements d'états asynchrones, ce test fera crasher l'OS (c'est donc un test de robustesse massif !).
__attribute__((section(".p1_code")))
void test_states_slave(void) {
    RETURN_CODE_TYPE ret;
    printf("[Esclave] Je suis vivant !\n");
    while(1) {
        printf("[Esclave] Je tourne...\n");
        TIMED_WAIT(10, &ret); // 10ms
    }
}

__attribute__((section(".p1_code")))
void test_states_master(void) {
    RETURN_CODE_TYPE ret;
    APEX_INTEGER id_slave;
    PROCESS_STATUS_TYPE status;

    // 1. Récupération de l'ID
    GET_PROCESS_ID("EsclaveProc", &id_slave, &ret);
    
    // 2. Test de START
    printf("[Maitre] START de l'esclave...\n");
    START(id_slave, &ret);
    if (ret != NO_ERROR) printf("Erreur START!\n");

    // 3. Test du double START (Doit retourner NO_ACTION selon la norme)
    START(id_slave, &ret);
    if (ret == NO_ACTION) printf("[Maitre] Double START bloque avec succes (NO_ACTION)\n");

    // Laisse l'esclave tourner un peu
    TIMED_WAIT(25, &ret); // 25ms

    // 4. Test de SUSPEND
    printf("[Maitre] SUSPEND de l'esclave...\n");
    SUSPEND(id_slave, &ret);
    
    GET_PROCESS_STATUS(id_slave, &status, &ret);
    if (status.PROCESS_STATE == WAITING) printf("[Maitre] L'esclave est bien WAITING (suspendu)!\n");

    TIMED_WAIT(20, &ret); // 20ms

    // 5. Test de RESUME
    printf("[Maitre] RESUME de l'esclave...\n");
    RESUME(id_slave, &ret);

    TIMED_WAIT(20, &ret); // 20ms

    // 6. Test de STOP
    printf("[Maitre] STOP de l'esclave...\n");
    STOP(id_slave, &ret);
    
    GET_PROCESS_STATUS(id_slave, &status, &ret);
    if (status.PROCESS_STATE == DORMANT) printf("[Maitre] L'esclave est bien tue (DORMANT)!\n");

    STOP_SELF();
}


// Test Suite 4 : La préemption dynamique SET_PRIORITY (Process Management)

// Objectif : Prouver que changer de priorité déclenche une préemption immédiate (un changement de contexte) si un processus moins prioritaire était en train de s'exécuter.

// Configuration requise : ProcA (Prio 50, démarre en 1er), ProcB (Prio 10, démarre en 2ème).
// Résultat attendu : Si votre fonction process_schedule() et vos appels à ucx_task_yield() dans SET_PRIORITY sont corrects, l'affichage sera :

//     [A] Je demarre avec Prio 50.

//     [A] Je baisse ma priorite a 5...

//     [B] HAHA ! J'ai preempte A... (La préemption a fonctionné !)

//     [B] Je travaille...

//     [A] Je reprends le controle...
__attribute__((section(".p1_code")))
void test_prio_A(void) {
    RETURN_CODE_TYPE ret;
    
    printf("[A] Je demarre avec Prio 50.\n");
    
    // On s'abaisse à la priorité 5 ! 
    // ProcB (qui est à 10 et READY) doit NOUS COUPER LA PAROLE INSTANTANEMENT !
    printf("[A] Je baisse ma priorite a 5...\n");
    SET_PRIORITY(0, 5, &ret);
    
    // On ne devrait arriver ici qu'après que B ait terminé ou fait un Wait !
    printf("[A] Je reprends le controle (B a du finir ou dormir) !\n");
    
    STOP_SELF();
}

__attribute__((section(".p1_code")))
void test_prio_B(void) {
    RETURN_CODE_TYPE ret;
    
    printf("[B] HAHA ! J'ai preempte A car il s'est abaisse ! Je suis Prio 10.\n");
    printf("[B] Je travaille...\n");
    
    // B cède sa place, A (prio 5) peut reprendre.
    STOP_SELF();
}

int app_main(void)
{
	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p1_data_size =  _p1_data_end -_p1_data_start;
	size_t p1_code_size =  _p1_code_end -_p1_code_start;

	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p2_data_size =  _p2_data_end -_p2_data_start;
	size_t p2_code_size =  _p2_code_end -_p2_code_start;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   DEFAULT_PARTITION_CONFIG.duration,
				   DEFAULT_PARTITION_CONFIG.identifier,
				   DEFAULT_PARTITION_CONFIG.num_assigned_cores,
				   DEFAULT_PARTITION_CONFIG.name,
				   DEFAULT_PARTITION_CONFIG.region_name_code_mem,
				   (void*)_p1_code_start,
				   (size_t)p1_code_size,
				   DEFAULT_PARTITION_CONFIG.access_code_mem,
				   DEFAULT_PARTITION_CONFIG.region_name_data_mem,
				   (void*)_p1_data_start,
				   p1_data_size,
				   DEFAULT_PARTITION_CONFIG.access_data_mem,
				//    test_spatial_violation_p2,
				   p1_main_process,
				   DEFAULT_PARTITION_CONFIG.is_system_partition);

	partition_init(P2_CONFIG.period,
				   P2_CONFIG.duration,
				   P2_CONFIG.identifier,
				   P2_CONFIG.num_assigned_cores,
				   P2_CONFIG.name,
				   P2_CONFIG.region_name_code_mem,
				   (void*)_p2_code_start,
				   (size_t)p2_code_size,
				   P2_CONFIG.access_code_mem,
				   P2_CONFIG.region_name_data_mem,
				   (void*)_p2_data_start,
				   p2_data_size,
				   P2_CONFIG.access_data_mem,
				//    test_spatial_violation_p1,
				   p2_main_process,
				   P2_CONFIG.is_system_partition);

	return 1;
}