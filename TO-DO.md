# TO-DO

## Partition implem

* implémentation set_operating_mode:
    * shut down the partition;
    * inhibit process scheduling and switch back to initialization mode;
    * faire les actions quand elle passe en normal (set to READY processes, activate process scheduling etc..)
* faire scheduler pour les partitions

## remind debug

* build:
    * make ucx ARCH=riscv/riscv32-qemu
    * make l'app
    * make run_riscv32

* qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
* gdb ./build/target/image.elf
* target remote :1234