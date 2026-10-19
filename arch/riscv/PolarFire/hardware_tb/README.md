# Hardware Testbench: AMP Avionics Testbench on PolarFire SoC Discovery Kit

This folder contains all the components, configurations, and binaries required to run the **avionics hardware testbench (Hardware Testbench)** on the **PolarFire SoC Discovery Kit (MPFS-DISCO-KIT)** board.

The system is based on an **AMP (Asymmetric Multi-Processing)** architecture utilizing two 64-bit RISC-V application cores (U54) interconnected via internal shared memory (**L2 LIM**):
1. **Hart 1 (Core 1 - U54_1)**: **Microchip Bare-metal** system executing the hardware communication layer (SPI controller or FPGA ARINC 429 IP) and the generation/decoding of avionics frames.
2. **Hart 2 (Core 2 - U54_2)**: Certifiable real-time operating system **UCX-OS-ARINC653**, executing two spatially and temporally isolated partitions communicating via a **Sampling Port**.
3. **Hart 0 (E51)**: Monitor core executing the first-stage bootloader **HSS (Hart Software Services)** in non-volatile memory (**eNVM**).

---

## Table of Contents

1. [Folder Structure](#1-folder-structure)
2. [Architecture Overview](#2-architecture-overview)
3. [Hardware & Software Prerequisites](#3-hardware--software-prerequisites)
4. [Step 1: Board Preparation (Libero Design & HSS in eNVM)](#4-step-1-board-preparation-libero-design--hss-in-envm)
5. [Step 2: Quick Deployment with Provided Binaries](#5-step-2-quick-deployment-with-provided-binaries)
6. [Step 3: Flash the Micro SD Card with USBImager](#6-step-3-flash-the-micro-sd-card-with-usbimager)
7. [Step 4: UART Connections and System Launch](#7-step-4-uart-connections-and-system-launch)
8. [Step 5: Hardware Wiring & Pinout Diagram (SPI Mode)](#8-step-5-hardware-wiring--pinout-diagram-spi-mode)
9. [Step 6: Procedure for the ARINC 429 FPGA IP (FPGA Mode)](#9-step-6-procedure-for-the-arinc-429-fpga-ip-fpga-mode)
10. [Step 7: Modifying and Recompiling Applications (If Needed)](#10-step-7-modifying-and-recompiling-applications-if-needed)
11. [Troubleshooting & Best Practices](#11-troubleshooting--best-practices)

---

## 1. Folder Structure

```text
arch/riscv/PolarFire/hardware_tb/
├── README.md                 # Complete installation and execution guide
├── INFO-README.md            # Initial notes and reference links
├── holt_spi/                 # Variant 1: Direct SPI communication to Holt board
│   ├── PolarFire.elf         # UCX-OS ARINC 653 executable (Core 2, DDR @ 0x80000000)
│   ├── mpfs-spi-master-slave.elf # Bare-metal SPI executable (Core 1, LIM @ 0x08000000)
│   └── payload.bin           # Merged binary image ready to flash on SD
├── holt_fpga/                # Variant 2: Communication via ARINC 429 IP in the FPGA fabric
│   ├── PolarFire.elf         # UCX-OS ARINC 653 executable (Core 2, DDR @ 0x80000000)
│   ├── mpfs-spi-master-slave.elf # Bare-metal FPGA executable (Core 1, LIM @ 0x08000000)
│   └── payload.bin           # Merged binary image ready to flash on SD
└── hss_payload/              # HSS boot image generation tool (Payload Generator)
    ├── hss-payload-generator     # Linux executable binary
    ├── hss-payload-generator.exe # Windows executable binary
    ├── msys-*.dll                # Dynamic libraries for Windows
    ├── payload.yaml              # AMP payload configuration file
    └── README.md                 # Official documentation for the HSS generator
```

> [!TIP]
> If you do not wish to modify the source code, **the pre-compiled `payload.bin` files are already ready to use** in `holt_spi/` and `holt_fpga/`. You can skip directly to [Step 3](#6-step-3-flash-the-micro-sd-card-with-usbimager).

---

## 2. Architecture Overview

### Two Supported Hardware Variants

```
+---------------------------------------------------------------------------------------------------+
| POLARFIRE SOC DISCOVERY KIT (MPFS-DISCO-KIT)                                                      |
|                                                                                                   |
|  +--------------------+         +---------------------------------------------------------------+ |
|  | Hart 0 (E51)       |         | Hart 1 (U54_1) - MICROCHIP BARE-METAL                         | |
|  | HSS Bootloader     |         | - Flight data generator (Altitude: climb, level-off...)       | |
|  | (Resides in eNVM)  |         | - ARINC 429 frame encoding / decoding                         | |
|  +--------------------+         | - Hardware SPI driver OR FPGA Fabric IP interface             | |
|                                 +-------------------------------+-------------------------------+ |
|                                                                 |                                 |
|                                                                 v (IPC Write)                     |
|                                 +---------------------------------------------------------------+ |
|                                 | L2 LIM SHARED MEMORY (Address 0x08060000)                     | |
|                                 | Magic Header: 0x4152494E ("ARIN")                             | |
|                                 | Structure: shared_altitude_data_t (meters, feet, frame, ...)  | |
|                                 +-------------------------------+-------------------------------+ |
|                                                                 |                                 |
|                                                                 v (IPC Read with fence)           |
|  +--------------------------------------------------------------+-------------------------------+ |
|  | Hart 2 (U54_2) - UCX-OS-ARINC653 RTOS                                                        | |
|  |                                                                                              | |
|  |  +--------------------------------+               +---------------------------------------+  | |
|  |  | Partition 1: P1_OUT_ALT        |               | Partition 2: P2_IN_ALT                |  | |
|  |  | - Reads LIM shared memory      |               | - Reads SAMPLING_PORT                 |  | |
|  |  | - Writes to SAMPLING_PORT      | -(ARINC 653)->| - Interprets flight phase             |  | |
|  |  |   (Period: 200ms)              |    Channel    | - Displays telemetry on UART          |  | |
|  |  +--------------------------------+               +---------------------------------------+  | |
|  +----------------------------------------------------------------------------------------------+ |
+---------------------------------------------------------------------------------------------------+
```

### Variant Details:
- **Variant 1: SPI Mode (`holt_spi`)**  
  The Bare-metal system (Core 1) directly uses the PolarFire SoC SPI controllers routed to the expansion header pins connected to the Holt ARINC 429 transceiver board(s).
- **Variant 2: FPGA Mode (`holt_fpga`)**  
  A dedicated hardware ARINC 429 IP (developed in MATLAB/Simulink and instantiated in the FPGA fabric via Libero SoC) handles the serialization/deserialization of the avionics protocol.

---

## 3. Hardware & Software Prerequisites

### Hardware
- **PolarFire® SoC Discovery Kit Board** (Microchip MPFS-DISCO-KIT, MPFS095T chip).
- **Holt ARINC 429 Transceiver Board(s)** (e.g., Holt HI-3593 / HI-8582 evaluation boards or equivalent).
- **Micro SD Card** (4 GB to 32 GB capacity, Class 10 recommended).
- **USB Cable** for board connection (FlashPro JTAG programming and 4 virtual UART ports).
- **Female-to-Female Dupont Jumper Wires** to connect the Discovery Kit GPIO header to the Holt board.
- **Twisted Pair Cable** for the ARINC 429 differential avionics bus ($A$ and $B$, $\pm 10\text{V}$).

### PC Software
- **Microchip Libero® SoC v2024.1 or v2025.2+** (with free *Silver* license).
- **Microchip SoftConsole v2022.2-RISC-V-747** (Eclipse IDE environment + 64-bit RISC-V cross-compilation toolchain).
- **FlashPro Express** (bundled with Libero SoC or *Program and Debug Tools*).
- **USBImager**: Ultra-lightweight and reliable utility to flash raw disk images to SD cards.  
  👉 Download: [GitLab USBImager](https://gitlab.com/bztsrc/usbimager)
- **Tera Term VT** (or any multi-window serial terminal supporting 115200 baud).  
  👉 Download: [GitHub Tera Term](https://github.com/TeraTermProject/teraterm/releases)
- **HSS Payload Generator** (provided directly in `hss_payload/`).
- *(Optional for FPGA mode)*: **MATLAB / Simulink** with **HDL Coder** (to regenerate the IP from the `arinc429_ip` repository).

---

## 4. Step 1: Board Preparation (Libero Design & HSS in eNVM)

Before running the system for the first time, the PolarFire SoC board must have:
1. The base FPGA bitstream programmed into the device.
2. The **HSS** bootloader programmed into internal non-volatile memory (**eNVM**) with the board configured in **Boot Mode 1**.

### 1.1 Program the Reference Design into the FPGA
The reference design configures system clocks, DDR4 memory controller, and I/O pin multiplexing.
1. Download or clone the Microchip reference repository:  
   [polarfire-soc-discovery-kit-reference-design on GitHub](https://github.com/polarfire-soc/polarfire-soc-discovery-kit-reference-design)
2. Open **Libero SoC**.
3. Open the script execution dialog: `Project > Execute Script...` (or shortcut `Ctrl + U`).
4. Select the `MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl` script and click **Run**.
5. Once project generation finishes, double-click **Program Device** in the *Design Flow* tab to program the FPGA via FlashPro Express.

### 1.2 Build and Program the HSS Bootloader into eNVM
The HSS (Hart Software Services) is the zero-stage bootloader (ZSBL) executing on the E51 monitor core.
1. Consult the official documentation and video tutorial:
   - Documentation: [PolarFire SoC Software Tool Flow - Programming the eNVM](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/knowledge-base/polarfire-soc-software-tool-flow.md#programming-the-envm)
   - Video tutorial: [Microchip Media Center - Video Tutorial eNVM HSS](https://www.microchip.com/en-us/about/media-center/videos/caNsH4dDnLU)
   - HSS source code: [hart-software-services on GitHub](https://github.com/polarfire-soc/hart-software-services)
2. In **SoftConsole**, import the HSS project configured for the Discovery Kit (`mpfs-discovery-kit`).
3. Verify that the board boot mode is set to **Boot Mode 1** (automatic boot of HSS from eNVM upon reset).

---

## 5. Step 2: Quick Deployment with Provided Binaries

Two pre-generated complete images are ready to be deployed depending on the chosen physical configuration:

| Configuration | Folder | Binary File | Description |
| :--- | :--- | :--- | :--- |
| **SPI Variant** | `arch/riscv/PolarFire/hardware_tb/holt_spi/` | `payload.bin` | Direct SPI controller to Holt board(s) |
| **FPGA Variant** | `arch/riscv/PolarFire/hardware_tb/holt_fpga/` | `payload.bin` | ARINC 429 IP synthesized in the FPGA fabric |

Each folder also contains:
- `PolarFire.elf`: Compiled executable for UCX-OS-ARINC653 (Core 2).
- `mpfs-spi-master-slave.elf`: Compiled executable for the Microchip Bare-metal system (Core 1).

---

## 6. Step 3: Flash the Micro SD Card with USBImager

The `payload.bin` binary must be written as a raw disk image onto the Micro SD card. **USBImager** is the recommended tool.

### Step-by-Step Procedure:
1. Insert the Micro SD card into your PC (via an SD adapter or USB card reader).
2. Launch **USBImager**.
3. In the first field (Image), click `...` and select the file:
   - `arch/riscv/PolarFire/hardware_tb/holt_spi/payload.bin` (for SPI mode)  
   *OR*
   - `arch/riscv/PolarFire/hardware_tb/holt_fpga/payload.bin` (for FPGA mode).
4. In the second field (Device), carefully select the Micro SD card (take care not to overwrite system drives!).
5. Click the **Write** button. Writing takes only a few seconds (size $\approx 60\text{ KB}$).
6. Once the operation is complete, safely eject the Micro SD card from the PC.
7. Insert the Micro SD card into the dedicated slot on the PolarFire SoC Discovery Kit (**J1**).

---

## 7. Step 4: UART Connections and System Launch

The USB-to-Serial bridge chip on the Discovery Kit exposes **multiple virtual COM ports** over a single USB cable.

### 7.1 Identifying COM Ports
Connect the USB cable between the Discovery Kit and your computer.
- **On Windows**: Open *Device Manager* > *Ports (COM & LPT)*. You will see 3 or 4 consecutive ports (for example: `COM4`, `COM5`, `COM6`, `COM7`).
- **On Linux**: Run `ls -l /dev/ttyUSB*` (you will see `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`, etc.).

### 7.2 Configuring Tera Term (or PuTTY)
Open **three separate Tera Term windows**, configured to **115200 baud, 8 data bits, 1 stop bit, no parity (115200 8N1)**:

| Tera Term Instance | Typical Port (Windows) | Role in Testbench | Stream Description |
| :--- | :--- | :--- | :--- |
| **Console 1** | `COM4` | **Hart 0 (E51) - HSS Bootloader** | HSS menu, SD boot status, system commands |
| **Console 2** | `COM5` | **Hart 1 (U54_1) - Microchip Bare-metal** | ARINC 429 SPI/FPGA TX/RX logs |
| **Console 3** | `COM6` | **Hart 2 (U54_2) - UCX-OS ARINC 653** | RTOS traces, P1 and P2 partitions, received telemetry |

### 7.3 Starting the Application
1. Once all three windows are open, press the **RESET** push button on the Discovery Kit board (or type the command `RESET` into the HSS Console on `COM4`).
2. On **Console 1 (COM4 - HSS)**, you will see the HSS boot log loading the two ELF contexts from the SD card into RAM:
   ```text
   [HSS] Reading payload from MMC/SD...
   [HSS] Booting Hart 1 at 0x08000000 (PRV_M)...
   [HSS] Booting Hart 2 at 0x80000000 (PRV_M)...
   ```
3. On **Console 2 (COM5 - Bare-metal)**, simulated flight messages and ARINC 429 word transmissions are displayed:
   ```text
   [Core 1] ARINC 429 Flight Generator Started
   [Core 1] TX ARINC Word: 0x0138406A | Alt: 1250 m | Phase: MONTEE
   ```
4. On **Console 3 (COM6 - UCX-OS ARINC 653)**, you will observe the kernel boot and inter-partition communication:
   ```text
   =======================================================
     UCX-OS ARINC 653 RTOS - Optimized Telemetry App      
     Execution Hart / Core ID : 2 (U54_2)              
   =======================================================
   [P1 Init] CREATE_SAMPLING_PORT P1_OUT_ALT rc=0 (port_id=0)
   [P1 Init] CREATE_PROCESS p1_process1 (id=0, rc=0)
   [P2 Init] CREATE_SAMPLING_PORT P2_IN_ALT rc=0 (port_id=1)
   [P2 Init] CREATE_PROCESS p2_process1 (id=0, rc=0)
   [P1] IPC->Port OK | Alt: 1250 m | Count: 42
   [P2] READ OK | Alt: 1250 m (4101 ft) | Phase: MONTEE | ARINC: 0x0138406A | Trame: #42 | 8400 ms
   ```

---

## 8. Step 5: Hardware Wiring & Pinout Diagram (SPI Mode)

In SPI mode, the connection between the PolarFire SoC and the Holt evaluation boards is established via the **MikroBUS connector (J5)** (for transmission) and the **40-pin Raspberry Pi extension connector (J10)** (for reception) on the Discovery Kit.

### 8.1 SPI Pins on MikroBUS Connector (J5) (Transmission)

```text
Discovery Kit J5 (8 pins)                     Holt Board 1 ARINC 429
+-----------------------------+                 +---------------------+
| Pin 7  : 3.3V Power         |---------------->| VDD / 3.3V Logic    |
| Pin 8  : GND Ground         |---------------->| GND                 |
| Pin 6  : SPI0_MOSI          |---------------->| SI  (Serial In)     |
| Pin 5  : SPI0_MISO          |---------------->| SO  (Serial Out)    |
| Pin 4  : SPI0_SCLK          |---------------->| SCK (Serial Clock)  |
| Pin 3  : SPI0_SS0           |---------------->| /CS (Chip Select)   |
+-----------------------------+                 +---------------------+
```

### 8.2 SPI Pins on 40-Pin Raspberry Pi Connector (J10) (Reception)

```text
Discovery Kit J10 (40 pins)                   Holt Board 2 ARINC 429
+-----------------------------+                 +---------------------+
| Pin 1  : 3.3V Power         |<----------------| VDD / 3.3V Logic    |
| Pin 6  : GND Ground         |<----------------| GND                 |
| Pin 19 : SPI0_MOSI (GPIO10) |<----------------| SI  (Serial In)     |
| Pin 21 : SPI0_MISO (GPIO9)  |<----------------| SO  (Serial Out)    |
| Pin 23 : SPI0_SCLK (GPIO11) |<----------------| SCK (Serial Clock)  |
| Pin 24 : SPI0_SS0  (GPIO8)  |<----------------| /CS (Chip Select)   |
+-----------------------------+                 +---------------------+
```

### 8.3 ARINC 429 Avionics Bus Interconnection ($\pm 10\text{V}$)
Connect the transmitter board output to the receiver board input:

```text
+-----------------------+                         +-----------------------+
|  Holt Board 1 (TX)    |                         |  Holt Board 2 (RX)    |
|                       |                         |                       |
|   AOUT Terminal (TX_A)|========================>|   ARX Terminal (RX_A) |
|   BOUT Terminal (TX_B)|========================>|   BRX Terminal (RX_B) |
+-----------------------+                         +-----------------------+
        ARINC 429 Differential Avionics Line (+10V / 0V / -10V Levels)
```

---

## 9. Step 6: Procedure for the ARINC 429 FPGA IP (FPGA Mode)

For the FPGA variant, ARINC 429 encoding and decoding are no longer handled in software, but by a **hardware VHDL/Verilog IP** implemented in the FPGA fabric.

The complete IP source files are located in the dedicated external repository: **`arinc429_ip`**.

- Follow step **9.1** if you want to directly program the board's FPGA with the ARINC 429 IP.
- Follow steps **9.2** and **9.3** if you want to modify the MATLAB model.

### 9.1 Opening the Project in Libero SoC
You can directly open the Libero SoC project located in the **`arinc429_ip`** repository. This project already contains the IP connected to the main design.
Simply run the Libero design flow:
- `Synthesize` -> `Place and Route` -> `Generate Bitstream`.
- Export and program the new bitstream using **FlashPro Express**.

### 9.2 Modeling & HDL Generation in MATLAB / Simulink
1. Clone or open the `arinc429_ip` repository folder.
2. Open **MATLAB**.
3. Open the main Simulink model: `ARINC429_converter_v3.slx`.
4. In Simulink, open **HDL Coder** (`Apps > HDL Coder > HDL Workflow Advisor`).
5. Configure the target:
   - Synthesis for PolarFire SoC (PolarFire family).
   - IP clock frequency: matching the fabric (e.g., 50 MHz or 100 MHz).
6. Execute the **Generate HDL** step: generated VHDL/Verilog files are located in the `hdlsrc/` subfolder.

### 9.3 Integration in Libero SoC
1. Open your Libero SoC project for the Discovery Kit.
2. Import the IP into the project:
   - **Method 1 (Automated)**: Execute the TCL script provided in `arinc429_ip` using `Ctrl + U`:  
     `source import_arinc429_ip.tcl`
   - **Method 2 (Manual)**: Go to `Catalog > Create Core from HDL...`, select the VHDL files generated by MATLAB, and instantiate the block in your **SmartDesign** canvas.
3. Connect the IP interfaces:
   - Register bus (APB or AXI) connected to MSS FIC0/FIC1 bus.
   - ARINC 429 Inputs/Outputs connected to physical I/O buffers routing to the extension connector pins.
4. Run the Libero design flow:
   - `Synthesize` -> `Place and Route` -> `Generate Bitstream`.
5. Export and program the new bitstream using **FlashPro Express**.

---

## 10. Step 7: Modifying and Recompiling Applications (If Needed)

If you need to adapt the applications for your own tests, follow the steps below.

### 10.1 Preparing and Compiling the UCX-OS Application
1. **Replace the main application code**:  
   Copy the contents of the template file `app/arinc_app_altitude.c` into the PolarFire target entry file `arch/riscv/PolarFire/src/app/app_main.c`.
2. **Configure ARINC 653 static descriptors**:
   - Copy the header file `include/arinc/static/static_conf_arinc_app_altitude.h` to `include/arinc/static_conf.h` in your SoftConsole project.
   - Copy the implementation file `arinc/static/static_conf_arinc_app_altitude.c` to `src/ucx/arinc/static_conf.c` in your SoftConsole project.
3. **Compile the project in SoftConsole**:
   - Right-click the `PolarFire` project > **Build Project**.
   - The generated binary is located in `Debug/PolarFire.elf` (or `Release/PolarFire.elf`).

### 10.2 Preparing and Compiling the Bare-metal Application
1. In the SoftConsole project `mpfs-spi-master-slave`, open `src/platform/drivers/mss/mss_spi/u54_1.c` (or the Hart 1 application file).
2. Ensure that the transmit loop writes to the shared address **`0x08060000`** with the magic value `0x4152494E`.
3. Compile the project under the target configuration: **`LIM-Debug-DiscoveryKit`**.
4. The generated binary is located in `LIM-Debug-DiscoveryKit/mpfs-spi-master-slave.elf`.

### 10.3 Generating a New `payload.bin`
1. Navigate to the `hss_payload/` folder:
   ```bash
   cd arch/riscv/PolarFire/hardware_tb/hss_payload
   ```
2. Verify the absolute or relative paths of the two `.elf` files in `payload.yaml`:
   ```yaml
   set-name: 'AMP_BM_UCXOS'
   hart-entry-points: {u54_1: '0x08000000', u54_2: '0x80000000'}

   payloads:
     'path/to/mpfs-spi-master-slave.elf': {exec-addr: '0x08000000', owner-hart: u54_1, priv-mode: prv_m, skip-opensbi: true}
     'path/to/PolarFire.elf': {exec-addr: '0x80000000', owner-hart: u54_2, priv-mode: prv_m, skip-opensbi: true}
   ```
3. Run the generator:
   - **On Windows**:
     ```cmd
     hss-payload-generator.exe -c payload.yaml payload.bin
     ```
   - **On Linux**:
     ```bash
     chmod +x hss-payload-generator
     ./hss-payload-generator -c payload.yaml payload.bin
     ```
4. Copy the new `payload.bin` file to your target folder (`holt_spi/` or `holt_fpga/`) and flash it onto the Micro SD card using USBImager (Step 3).

---

## 11. Troubleshooting & Best Practices

### 1. HSS Console displays nothing upon RESET
- Check the USB cable and the board power jumpers.
- Verify that the selected COM port corresponds to the E51 core (first virtual port, typically the lowest number, e.g., COM4).
- Ensure that the baud rate is strictly set to **115200 baud**.
- Verify that **Boot Mode 1** is active and that the HSS has been programmed into eNVM.

### 2. HSS displays `Failed to read payload from MMC/SD`
- The Micro SD card is not inserted or improperly seated in slot J1.
- The card was filesystem-formatted instead of written as a raw image: you must use **USBImager** to write `payload.bin` directly to the raw block device.

### 3. Cores boot but altitude data stays at 0 or does not change
- **Magic Number Verification**: Core 2 expects the header word `0x4152494E` ("ARIN" in ASCII). If Core 1 has not written yet or wrote to an address different from `0x08060000`, Core 2 will wait.
- **Memory Barrier (Fence)**: The U54_2 core has an L1 cache. The application code in `app_main.c` uses the assembly instruction:
  ```c
  asm volatile ("fence rw, rw" ::: "memory");
  ```
  to force reloading from the physical L2 LIM shared memory.
- **ARINC 429 Inverted Wiring**: Verify that the AOUT line is properly connected to ARX, and BOUT to BRX. If the lines are swapped ($A$ to $B$), parity and labels will be rejected by the decoder.

---

> For any additional information regarding the ARINC 653 operating system, static configuration of HM health tables, or the two-level scheduler, refer to the `README.md` file located at the root of the ucx-os-ARINC653 repository.
