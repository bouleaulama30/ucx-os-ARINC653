#!/bin/bash

# Configuration
LOG_FILE="benchmark_results.txt"
TEMP_LOG="last_run.log"
ITERATIONS=200
# On définit la commande exacte (copiée de votre log make)
CMD_QEMU="qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ../build/target/image.elf -nographic"

# Nettoyage
echo "Démarrage du benchmark ($ITERATIONS itérations)..."
> $LOG_FILE
rm -f $TEMP_LOG

for ((i=1; i<=ITERATIONS; i++))
do
    # 1. Lancer QEMU directement en arrière-plan
    $CMD_QEMU > $TEMP_LOG 2>&1 &
    PID_QEMU=$!

    # 2. Attendre que le motif "[PERF]" apparaisse dans le fichier (méthode plus fiable que le sleep)
    # On boucle pendant max 2 secondes
    FOUND=0
    for ((j=0; j<20; j++)); do
        if grep -q "\[PERF\]" $TEMP_LOG; then
            FOUND=1
            break
        fi
        sleep 0.1
    done

    # 3. Tuer QEMU proprement
    kill -9 $PID_QEMU > /dev/null 2>&1
    wait $PID_QEMU 2>/dev/null

    # 4. Extraire le temps si trouvé
    if [ $FOUND -eq 1 ]; then
        TIME_VAL=$(grep "\[PERF\] main() init time:" $TEMP_LOG | awk '{print $5}')
        echo "$TIME_VAL" >> $LOG_FILE
        echo -ne "Run $i/$ITERATIONS : ${TIME_VAL} ms \r"
    else
        echo "Run $i/$ITERATIONS : Erreur (Timeout ou pas de sortie)"
    fi
done

echo -e "\n\nBenchmark terminé."

# Calcul des stats (inchangé)
awk '{
    if(min==""){min=max=$1}; 
    if($1>max)max=$1; 
    if($1<min)min=$1; 
    total+=$1; count+=1
} 
END {
    if (count > 0) 
        print "Moyenne: " total/count " ms\nMin: " min " ms\nMax: " max " ms"; 
    else 
        print "Aucune donnée valide.";
}' $LOG_FILE
