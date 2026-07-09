# EmbeddedPQC_UCXOS

Polarfire SoC Embedded PQC project + UCX_OS base port + FPGA fabric interface.

## Documentation & Setup
For detailed instructions on how to set up the project, compile the code, configure search paths, and run performance tests using Microchip SoftConsole, please refer to the setup guide at **`arch/riscv/PolarFire/README.md`**.

## Compiler Flags
Add the following compiler options:
`-march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -DMEASURE_STATIC=1 -DPERFORMANCE_PRINT -DTEST_PERF19`