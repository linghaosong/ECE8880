# Open the final checkpoint in the Vivado GUI with the kernel's cells in red;
# the rest of the device is the U55C platform shell.
# Usage: vivado -mode gui -source layout.tcl -tclargs <dcp> <kernel cell>
lassign $argv dcp kernel
open_checkpoint $dcp
highlight_objects -color red [get_cells -hierarchical -filter "IS_PRIMITIVE && NAME =~ $kernel/*"]
