# TO-DO

## Partition implem

* pour partition init, gérer les tailles d'allocations avec data + code 

## remind debug

 qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -nographic -s -S
 gdb ./build/target/image.elf
 target remote :1234