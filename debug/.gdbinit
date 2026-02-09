target remote :1234
layout split
layout regs
focus cmd
b main
c


define reload
    monitor system_reset
    load
    # Optionnel : réinitialiser le PC manuellement si le reset QEMU ne suffit pas
    set $pc = _entry
    printf "Programme rechargé et prêt.\n"
end
