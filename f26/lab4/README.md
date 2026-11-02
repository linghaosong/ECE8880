# Lab 4: Vivado Implementation

In this lab, you take your Lab 2 KNN kernel **or** your Lab 3 CNN kernel
through Vivado implementation on the U55C platform: synthesis, placement,
physical optimization, and routing. You then compare the routed resource
utilization with the HLS estimate and inspect the final layout.

First, [install the bundled course version of TAPA](../tapa/README.md) if
your workspace does not have it. The Makefiles select Xilinx Vivado/Vitis
2024.2 from your workspace.

## Setup

Choose one design and copy your final Lab 2 or Lab 3 source folder, with all
three files, into its folder:

```bash
cd f26/lab4
cp -r ../lab2/src knn/   # KNN, or
cp -r ../lab3/src cnn/   # CNN
cd knn                   # or: cd cnn
```

The Makefiles expect this layout; `src/` is the only part you add:

```text
lab4/
├── scripts/       tool scripts used by both Makefiles (do not edit)
├── knn/
│   ├── Makefile
│   ├── link.cfg   HBM bank of each kernel port
│   └── src/       your Lab 2 files: knn.cpp, knn.h, main.cpp
└── cnn/
    ├── Makefile
    ├── link.cfg
    └── src/       your Lab 3 files: cnn.cpp, cnn.h, main.cpp
```

Before building, check that the folder matches your design:

- The kernel top-level function is still `KNNKernel` or `CnnKernel`; the
  Makefile, `link.cfg`, and the scripts refer to it.
- `CLOCK_PERIOD` in the Makefile sets the clock for both HLS and
  implementation: 4.00 ns (250 MHz) for KNN and 3.33 ns (300 MHz) for CNN.
  `make impl` reruns HLS at this period, so KNN uses a faster clock than the
  5.00 ns of Lab 2.
- The `sp=` lines in `link.cfg` name your kernel's `tapa::mmap` arguments (see
  below).

Run every command below from `lab4/knn` or `lab4/cnn`, in this order:

| Step | Command | Time |
| --- | --- | --- |
| Check correctness | `make swsim` | Minutes |
| HLS and implementation | `nohup make impl > impl.log 2>&1 &` | About 2 hours or more |
| Routed kernel utilization | `make util` | A few minutes |
| View the layout (on the desktop) | `make layout` | A few minutes |

Check the design before implementing it:

```bash
make swsim
```

KNN reads the data in `../../lab2/cifar-10` and uses the small Lab 2 sample
(320 training images per class, 160 test images). CNN reads `../../lab3/data`
and runs the full layer.

## Run implementation

Implementation takes hours. Run it in the background so that closing the
browser tab or terminal does not stop it:

```bash
nohup make impl > impl.log 2>&1 &
tail -f impl.log      # Ctrl-C stops tail only; the build continues
```

`make impl` first runs HLS (`make hls`), then
`v++ --link --target hw`, which runs Vivado synthesis, placement, physical
optimization, routing, post-route physical optimization, and bitstream
generation inside the U55C platform. For KNN, the hardware is built for the
full workload (50,000 training and 10,000 test images), as in Lab 2's
`hlsfull`.

`link.cfg` places each kernel port (`tapa::mmap` argument) in its own HBM
bank; without it, all ports share one bank and its bandwidth. It uses the
port names of the starter's `KNNKernel` or `CnnKernel`. If you renamed, added,
or removed kernel ports, update its `sp=` lines to match. If a line names a
port that does not exist, `v++` stops within seconds with
`ERROR: [CFGEN 83-2292] --sp tag applied to an invalid port or argument name`.

The kernel clock target is `1000 / CLOCK_PERIOD` MHz: 250 MHz for KNN and
300 MHz for CNN. `CLOCK_PERIOD` in the Makefile sets both the HLS clock and
the implementation target. If routing does not meet the target, Vitis lowers
the kernel clock automatically (`[AUTO-FREQ-SCALING-04]` in `impl.log`) and
still builds the design.

Plan for the run time and disk space:

- The reference designs took about 2 hours each on an 8-core workspace.
  Larger designs take longer.
- One run needs about 2 GB of disk (`_x/` alone is about 1.5 GB). Check free
  space with `df -h ~`; `make cleanall` removes everything the build produced.
- Vivado placement and routing peaked at about 18 GB of memory for the
  reference designs; use a workspace with at least 32 GB.
- Keep the workspace running until the build finishes;
  `pgrep -af "make impl"` prints nothing once it is done.

These messages are expected:

- `CRITICAL WARNING: [v++ 17-1396] The specified XO container failed
  integrity check`
- CNN only: `WARNING: [v++ 60-889] User-specified kernel frequency for ID 0 is
  the same as the default frequency 300 MHz, so it will be ignored`

The Makefile also works around two tool issues on the course workspaces;
keep these parts:

- Vivado's license check crashes in the workspace's `libudev`. The Makefile
  builds a stub (`compat/libudev.so.1` from `../scripts/udev_stub.c`) and loads it
  for the Vivado tools only.
- Vitis 2024.2 computes a non-default kernel clock (such as KNN's 250 MHz)
  with an outdated clock-wizard call, which would constrain the kernel to
  100 MHz. `../scripts/clkwiz_compat.tcl` fixes it; when it is used,
  `_x/link/vivado/vpl/prj/prj.runs/impl_1/runme.log` shows
  `ECE8880: kernel clock requested ... MHz`.

## Results

| File | Contents |
| --- | --- |
| `knn.xclbin` / `cnn.xclbin` | The FPGA binary |
| `knn.xclbin.info` / `cnn.xclbin.info` | Build summary; `DATA_CLK` is the final kernel clock |
| `_x/reports/link/imp/impl_1_hw_bb_locked_timing_summary_postroute_physopted.rpt` | Final timing: WNS >= 0 means the clock target was met |
| `_x/reports/link/imp/impl_1_full_util_routed.rpt` | Routed utilization of the whole device, including the platform shell |
| `_x/link/vivado/vpl/prj/prj.runs/impl_1/level0_wrapper_postroute_physopt.dcp` | Final design checkpoint, after post-route physical optimization |

## Compare routed utilization with HLS

The full-device report includes the platform shell. To get the routed
utilization of your kernel alone, run:

```bash
make util
```

It opens the final checkpoint in Vivado batch mode (a few minutes) and writes
`kernel_util.rpt`: one row for the kernel (`level0_i/ulp/KNNKernel_1` or
`level0_i/ulp/CnnKernel_1`) and one row per TAPA task instance (for example,
`KnnDist_0` or `cnncore_0`). The kernel name appears in two rows, the
reconfigurable partition and its contents; both show the same totals. Rows
named after your streams (FIFOs), `control_s_axi_U`, and `__tapa_fsm_unit`
are the logic TAPA generates around your tasks.

The HLS estimates are in `work.out/report.yaml`: `area: total` for the whole
kernel, and `area: breakdown` per task (multiply by `count` for tasks with
several instances). Compare these columns:

| HLS (`report.yaml`) | Routed (`kernel_util.rpt`) |
| --- | --- |
| `LUT` | `Total LUTs` (logic LUTs plus LUTRAMs and SRLs) |
| `FF` | `FFs` |
| `BRAM_18K` | 2 x `RAMB36` + `RAMB18` |
| `URAM` | `URAM` |
| `DSP` | `DSP Blocks` |

Expect differences: HLS estimates each task before Vivado optimizes the
netlist, and Vivado may map logic differently from what HLS assumed.

## View the layout

The final layout is the post-route physical optimization checkpoint listed
above. To view it:

1. Open the **NoVNC Desktop** app of your workspace from the Coder dashboard.
2. Open a terminal on the desktop and run `make layout` from `lab4/knn` or
   `lab4/cnn`.
3. Vivado opens the checkpoint (this takes a few minutes) and shows the
   Device window with your kernel's cells in red. The rest of the placed
   logic is the U55C platform shell. If the Device window is not visible,
   choose **Window > Device**.
4. To color individual tasks, open the **Netlist** window, expand
   `level0_i > ulp > KNNKernel_1 > inst` (or `CnnKernel_1`), right-click a
   task, and choose **Highlight Leaf Cells** with a color.
5. Take a screenshot of the Device window.

Close Vivado when you are done; with the checkpoint open it uses about 16 GB
of memory.
