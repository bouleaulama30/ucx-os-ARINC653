# UCX-OS-ARINC653 on PolarFire SoC Discovery Kit

This guide describes how to configure, import, and run UCX-OS-ARINC653 on the PolarFire SoC Discovery Kit using Microchip SoftConsole (an Eclipse-based IDE).

---

## Prerequisites & Requirements

- Microchip SoftConsole installed.
- PolarFire SoC Discovery Kit hardware connected.
- UCX-OS-ARINC653 repository cloned locally.

---

## Step 1: Project Setup in SoftConsole

1. **Create a Workspace**: Open SoftConsole and create or select a workspace.
2. **Create a C Project**:
   - Go to **File > New > C Project** (or **C/C++ Project**).
   - Set the project name to **`PolarFire`**.
   - Select the appropriate RISC-V cross-compiler toolchain.
3. **Import Architecture-Specific Files**:
   - Right-click the newly created `PolarFire` project in the Project Explorer and select **Import...**
   - Choose **General > File System** and click **Next**.
   - Browse to the `arch/riscv/PolarFire` directory in your cloned repository.
   - Select all files and folders (`include/`, `src/`, `PolarFire.launch`, etc.) and import them directly into the root of your project.

---

## Step 2: Import UCX-OS Core Components

To populate the project with the core UCX-OS and ARINC653 files, import the following components from the repository root:

### 1. Global Header Files (`include`)
- Import the root `include` directory into the project root.
- **Important**: To prevent indexer and build conflicts, **exclude** the static configuration headers:
  - Right-click `include/arinc/static` in the Project Explorer.
  - Select **Resource Configurations > Exclude from Build...**
  - Check all build configurations (e.g., `Debug` and `Release`) and click **OK**.

### 2. Common Architecture Files (`arch_common`)
- Copy/import the following files from the repository's `arch/common` folder into your project's `src/ucx/arch_common/` folder:
  - `ieee754.c`
  - `ieee754.h`
  - `math.c`
  - `math.h`
  - `muldiv.c`

### 3. Core OS Subfolders
- Import the following folders from the repository root into your project's `src/ucx/` folder:
  - `arinc`
  - `init`
  - `kernel`
  - `lib`
- **Important**: Exclude the imported static folder to prevent build conflicts:
  - Right-click the `src/ucx/arinc/static` folder -> **Resource Configurations > Exclude from Build...** -> Check all configurations.

### 4. Benchmark Suite (SPFBench)
- Import the `spfbench` directory into your project's `src/` directory.
- **Important**: Exclude the benchmark test applications folder from build to prevent multiple entry-point (`main`) conflicts:
  - Right-click the `src/spfbench/test_applications` folder -> **Resource Configurations > Exclude from Build...** -> Check all configurations.

---

## Step 3: Project Configuration

Configure the compiler flags, search paths, and linker script in your project settings:

### 1. Compilation Flags
Go to: **Project Properties > C/C++ Build > Settings > Tool Settings > GNU RISC-V Cross Compiler > Miscellaneous**

Add the following options to **Other compiler flags**:
```text
-march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -DMEASURE_STATIC=1 -DPERFORMANCE_PRINT
```

> To execute a specific performance test, optionally append the preprocessor flag `-DTEST_PERFXX` (e.g., `-DTEST_PERF19` for test number 19).

### 2. Include Paths
Go to: **Project Properties > C/C++ Build > Settings > Tool Settings > GNU RISC-V Cross Compiler > Includes**

Add these search paths under **Include paths (-I)**. Customize the base path if your project folder structure differs:

```text
"${workspace_loc:/${ProjName}/PolarFire/include/arinc}"
"${workspace_loc:/${ProjName}/PolarFire/include/testperf}"
"${workspace_loc:/${ProjName}/PolarFire/src/spfbench/support_files/include}"
"${workspace_loc:/${ProjName}/PolarFire/include}"
"${workspace_loc:/${ProjName}/PolarFire/src}"
"${workspace_loc:/${ProjName}/PolarFire/src/startup}"
"${workspace_loc:/${ProjName}/PolarFire/src/bsp}"
"${workspace_loc:/${ProjName}/PolarFire/src/drivers}"
"${workspace_loc:/${ProjName}/PolarFire/include/kernel}"
"${workspace_loc:/${ProjName}/PolarFire/include/lib}"
"${workspace_loc:/${ProjName}/src/spfbench/test_applications/test_performance19/Application/Partition2/include}"
"${workspace_loc:/${ProjName}/src/spfbench/test_applications/test_performance1/Application/Partition1/include}"
```

### 3. Linker Script
Go to: **Project Properties > C/C++ Build > Settings > Tool Settings > GNU RISC-V Cross Linker > General**

- Under **Linker flags**, specify the path to the linker script:
  **`src/startup/linker.ld`**

---

## Step 4: Running Applications

You can run your own custom ARINC-653 applications or run the pre-existing demo applications included with UCX-OS.

### 4.1 Custom & Demo Applications

1. **Write or Select an Application**:
   - You can write your custom application code directly inside the `src/app/app_main.c` file of your project.
   - Alternatively, you can use one of the pre-existing ARINC demo applications located in the root `app/` folder of the UCX-OS repository:
     - `arinc_app.c` (basic template for development)
     - `arinc_app_demo1.c` (waker and worker app with a periodic sensor reader)
     - `arinc_app_demo2.c` (error handler testing at different levels)
     - `arinc_app_demo3.c` (round-robin scheduling of two aperiodic processes)
     - `arinc_app_demo4.c` (inter-partition communication between Partition 1 and Partition 2)

2. **Setup the Entry Point**:
   - Copy the contents of your chosen application file (e.g., `app/arinc_app_demo1.c`) and paste it into your project's `src/app/app_main.c` file.

3. **Configure Partition Settings**:
   - Every demo application requires a matching static configuration. You must copy the corresponding configuration file from the root `include/arinc/static/` directory and overwrite `include/arinc/static_conf.h` in your project.
   - Examples of matching configurations:
     - For `arinc_app_demo1.c`, copy the contents of `include/arinc/static/static_conf_arinc_app_demo1.h` and overwrite `include/arinc/static_conf.h`.
     - For `arinc_app_demo2.c`, copy the contents of `include/arinc/static/static_conf_arinc_app_demo2.h` and overwrite `include/arinc/static_conf.h`.
     - For `arinc_app.c`, copy the contents of `include/arinc/static/static_conf_arinc_app.h` and overwrite `include/arinc/static_conf.h`.

---

### 4.2 Running Performance Benchmarks (SPFBench)

Follow these steps to run performance tests on the PolarFire SoC:

1. **Setup Entry Point**:
   - Copy the contents of the `app/arinc_testperf.c` template (located in the root `/app` folder) and paste them into your project's `src/app/app_main.c` (located in `src/app/`).

2. **Select Partition Configurations**:
   - **For Tests 1–12 and 14–18 (Single Partition)**:
     Copy the contents of `include/arinc/static/static_conf_testperf_single_partition.h` and overwrite the contents of `include/arinc/static_conf.h`.
   - **For Tests 13 and 19 (Double Partition)**:
     Copy the contents of `include/arinc/static/static_conf_testperf_double_partition.h` and overwrite the contents of `include/arinc/static_conf.h`.

3. **Specify the Test Number**:
   - Add the flag `-DTEST_PERFXX` to the compilation flags (e.g., `-DTEST_PERF19` to run benchmark test 19).
