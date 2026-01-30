# TO-DO

## Partition implem

* comprendre l'importation et signification des symboles ldscript 
* changer ldscript pour mettre data_end à la fin de la région et non faire le truc avec 256ko

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234