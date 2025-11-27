# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct X:\workspace4\lab3b\platform.tcl
# 
# OR launch xsct and run below command.
# source X:\workspace4\lab3b\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {lab3b}\
-hw {X:\LAB3A_ECE153A\3A\system_wrapper.xsa}\
-proc {microblaze_0} -os {standalone} -out {X:/workspace4}

platform write
platform generate -domains 
platform active {lab3b}
platform generate
