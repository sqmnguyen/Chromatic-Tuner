# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: X:\workspace3\Lab_3A_system\_ide\scripts\systemdebugger_lab_3a_system_standalone.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source X:\workspace3\Lab_3A_system\_ide\scripts\systemdebugger_lab_3a_system_standalone.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent Nexys A7 -100T 210292BE37C9A" && level==0 && jtag_device_ctx=="jsn-Nexys A7 -100T-210292BE37C9A-13631093-0"}
fpga -file X:/workspace3/Lab_3A/_ide/bitstream/system_wrapper.bit
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
loadhw -hw X:/workspace3/ECE153A_Lab3/export/ECE153A_Lab3/hw/system_wrapper.xsa -regs
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
rst -system
after 3000
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
dow X:/workspace3/Lab_3A/Debug/Lab_3A.elf
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
con
