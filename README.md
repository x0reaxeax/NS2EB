# EFI MSR FUZZER (NS2EB)

### A UEFI MSR Fuzzer for uncovering processor secrets and undocumented MSRs (EDK2 version)

NS2EB is a small EFI application for fuzzing Model-Specific Registers (MSRs) in x86_64 processors. Leveraging the fan-favorite [Time Stamp Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter) (`RDTSC`), custom General Protection Fault interrupt handler, and optionally a disassembly backing via [Zydis-Amalgamated](https://github.com/zyantific/zydis).

## Table of contents
 * [Features](#features)
 * [Why "NS2EB"?](#why-ns2eb)
 * [Getting Started](#getting-started)
 * [Log file](#log-file)
	 * [Sample Log Format](#sample-log-format)
* [Graph](#graph)
* [Anomaly Extraction](#anomaly-extraction)
* [Customizations](#customizations)
* [Important Info](#important-info)
* [Credits](#credits)
* [Disclaimer](#disclaimer)

### Features
- **Brute-Force MSR Analysis:** Reads all MSRs from a given range of values (valid and invalid).
- **Custom GP Fault Handling:** Able to execute invalid MSRs reads by handling GP Faults.
- **EFI-Based:** Runs natively in UEFI environments, fully isolated from OS-level services overhead.
- **Performance Modes:** Change testing ranges, verbosity and speed via `NEED_MORE_SPEED` and `MAX_MSR`.
- **Disassembler-Backed Analysis:** Optionally disassembles faulting instructions for insights.
- **Graph result overview:** Includes a separate script to visualize execution time flukes.

### Why "NS2EB"?
This project is fully inspired by Christopher Domas' ([@xoreaxeaxeax](https://github.com/xoreaxeaxeax)) [Project Nightshyft](https://www.youtube.com/watch?v=XH0F9r0siTI), although the original Project Nightshyft was never made public, and was only demonstrated during a conference talk.
Therefore the super-creative name "Nightshyft 2: Electric Boogaloo" was used, however, it should be noted that this program **CANNOT** replicate the quality and functionality of the original project.
Please refer to the following picture for technical explanation:
![supercomputer-generated-explanation](https://i.imgur.com/uHVAxOx.jpeg)


### Getting Started

1. **Clone the repo:**
   ```bash
   $ git clone --recurse-submodules https://github.com/youruser/NS2EB.git
   $ cd NS2EB
   ```
2. **Build the binary:**
	```bash
	$ chmod +x build.sh
	$ ./build.sh
	```
3. **Copy the binary to a disk or an image on the target system:**
	For testing under QEMU, `Mtools`' `mcopy` can be used to easily write to an image file:
	```bash
	$ mcopy -i disk.img /path/to/target/efi.efi ::
	```
	Or deploy to a UEFI-supported system.

	**NOTE:** QEMU does **NOT** read any actual MSRs from the host system, and all of the MSR support is emulated (unless running in a **KVM-accelerated** VM, see [Important Info](#important-info)). Observed anomalies are therefore just flukes, and no meaningful data should be expected from emulated runs.   
4. **Run the EFI application from an EFI Shell:**
	```bash
	$ efi.efi
	```
 	Preview:
 	![rundemo](https://i.imgur.com/WRQEo4w.png)
### Log File
A logfile with a default name `ns2be.log` is generated on the same drive and directory where the application is located.

The logfile can be used for creating a graph interpretation of the test.
See [Graph](#graph) section for more information.

#### Sample Log Format:
```
I=0x00;T=0x1020
V=0x01;T=0x0F80
I=0x02;T=0x2280
I=0x03;T=0x11E0
I=0x04;T=0x1080
I=0x05;T=0x0FC0
I=0x06;T=0x0FE0
I=0x07;T=0x1200
I=0x08;T=0x1280
I=0x09;T=0x1120
I=0x0A;T=0x12C0
I=0x0B;T=0x1220
I=0x0C;T=0x1180
I=0x0D;T=0x1140
I=0x0E;T=0x10C0
I=0x0F;T=0x10C0
V=0x10;T=0x0CC0
V=0x11;T=0x0CC0
V=0x12;T=0x0C40
I=0x13;T=0x14E0
I=0x14;T=0x11E0
``` 

`I/V` indicates invalid/valid MSR, and `T` represents the execution timing in cycles.

### Graph
A standalone Python3 script is included with the project, which can be used to generate a graph of the testing results:
![qemugraphdemo](https://i.imgur.com/vAUPEf8.png)
(QEMU demo)

To build a graph from the results, use the [graph-gen.py](https://github.com/x0reaxeax/NS2EB/blob/main/graph-gen.py) script, with the generated logfile present in the same directory as the script:
```bash
$ python3 graph-gen.py
```
### Anomaly Extraction
Another included script [extract-anomalies.py](https://github.com/x0reaxeax/NS2EB/blob/main/extract-anomalies.py) can help extract anomalies of high-spike clock cycles from the generated logfile.
Simply run
```bash
$ python3 extract-anomalies.py
```
with the logfile present in the same folder as the script.

### Customizations

#### `NEED_MORE_SPEED`
By default, a `NEED_MORE_SPEED` mode is active, which limits the number of console outputs when testing the MSRs.
You can disable this by commenting the line `#define NEED_MORE_SPEED` in [efi.c](https://github.com/x0reaxeax/NS2EB/blob/main/efi.c), which will allow for displaying the values of all tested MSRs in real-time.
However, since most firmwares are sadly incapable of implementing a well-optimized `SIMPLE_TEXT_OUTPUT` protocol, the testing time will likely increase astronomically:

QEMU virtualized environment with OVMF firmware:

 - 65536 (0x10000) MSRs with `NEED_MORE_SPEED`: 3.54 seconds
 - 65536 (0x10000) MSRs without `NEED_MORE_SPEED`: 1 minute 32 seconds

So yes, just by writing console output in real-time, the testing time increased over 30 times.

#### `MSR_MIN` & `MSR_MAX`

These constants control the starting MSR and maximum MSR values to run in the testing loop.
This will directly impact the size of the produced logfile.
See [Important Info](#important-info) section before changing these.

#### `ZYDIS_DISASM_BACKING`

This hidden configuration within [CPU.asm](https://github.com/x0reaxeax/NS2EB/blob/main/CPU.asm) will enable [Zydis](https://github.com/zyantific/zydis)-backed verification of fault-generating instructions from within the Interrupt Handler.

The tool was written on top of this support in the beginning, however, to cut down execution times, it was switched to this naïve error-prone check:
```c
if (0x320F == *(UINT16 *) RIP) {
   /* for sure RDMSR, source: trust me bro */
}
```
You can re-enable this by adding the following line next to any existing `%define` directive in [CPU.asm](https://github.com/x0reaxeax/NS2EB/blob/main/CPU.asm):
```
%define ZYDIS_DISASM_BACKING
```

*(Currently untested on EDK2 build)*

### Important Info

* #### Logfile Size
	* In order to prevent the logfile growing to massive sizes on the disk, the default `MSR_MAX` value is set to `0x100000` (1'048'576) MSRs, which will consume roughly **19.2MB** of disk space.
	If the value is increased to maximum amount - `0xFFFFFFFF`, it should be expected the logfile growing to around **78.645GB**.
	It is obviously recommended to test with smaller chunks.
* #### Interrupt Handler and benchmarking
	* The Interrupt Handler takes over execution on each attempt to read an invalid MSR, however, it should be noted that the Interrupt Handler executes all of its shenanigans first, before executing the second (closing) `RDTSC` instruction, to calculate a clock diff.
		This should hopefully have no significant impact on the results, besides the invalid MSR times having a higher execution time ground.
	* This can be also avoided almost completely by placing the `RDTSC` instruction at the top of the Interrupt Handler.
* #### Testing on a KVM-accelerated VM
	* If NS2EB is running inside a KVM-accelerated VM, be aware that some MSRs might be actually passed through to the host system instead of being fully emulated. This means that certain MSRs (e.g., TSC, APERF, MPERF,..) might (probably) return real host values, while other MSRs might be virtualized (e.g., VMX, MTRRs,..) and return fake values.

* #### Testing on real hardware
	* I hope it's fairly obvious that this tool should be used on your own risk, and that I am **NOT** responsible for any damages caused by this educational-purposes-based project.
	This hacky superglued code can go haywire at any moment.
* #### `ExecFlag` force-termination
  	* The displayed `ExecFlag` variable is an `int32` flag that controls the execution of the fuzz loop.
  	* Since there's no `CTRL-C`-like event handler, the flag can be used to gracefully terminate the loop early via a debugger or DMA.
  	* The displayed value is the address of this flag.

### Credits

 - Christopher Domas ([@xoreaxeaxeax](https://github.com/xoreaxeaxeax)) for the idea and the [incredible presentation](https://www.youtube.com/watch?v=XH0F9r0siTI).
 - [Zydis](https://github.com/zyantific/zydis) for amalgamated version of Zydis disassembler
 - [OSDev Wiki](https://wiki.osdev.org/) for technical descriptions
 - [TianoCore EDK2](https://github.com/tianocore/edk2)
 - ChatGPT for Python scripts, help with EDK2 adaptation, and for mental support 🧡

### Disclaimer
This project is licensed under the **MIT License**. The content of this repository exists purely for **educational purposes**, and the author is not responsible for any damages caused by this software.

### License

**MIT License**

Copyright (c) 2025 x0reaxeax

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
