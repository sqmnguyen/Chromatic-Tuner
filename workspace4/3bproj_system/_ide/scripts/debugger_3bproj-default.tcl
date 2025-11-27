# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: X:\workspace4\3bproj_system\_ide\scripts\debugger_3bproj-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source X:\workspace4\3bproj_system\_ide\scripts\debugger_3bproj-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent Nexys A7 -100T 210292BE37C9A" && level==0 && jtag_device_ctx=="jsn-Nexys A7 -100T-210292BE37C9A-13631093-0"}
fpga -file X:/workspace4/3bproj/_ide/bitstream/system_wrapper.bit
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
loadhw -hw X:/workspace4/lab3b/export/lab3b/hw/system_wrapper.xsa -regs
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
rst -system
after 3000
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
dow X:/workspace4/3bproj/Debug/3bproj.elf
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
con
