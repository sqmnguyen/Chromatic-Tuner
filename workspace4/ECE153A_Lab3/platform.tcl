# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct X:\workspace3\ECE153A_Lab3\platform.tcl
# 
# OR launch xsct and run below command.
# source X:\workspace3\ECE153A_Lab3\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {ECE153A_Lab3}\
-hw {X:\LAB3A_ECE153A\3A\system_wrapper.xsa}\
-proc {microblaze_0} -os {standalone} -out {X:/workspace3}

platform write
platform generate -domains 
platform active {ECE153A_Lab3}
platform generate
platform config -updatehw {X:/LAB3A_ECE153A/3A/system_wrapper.xsa}
platform generate -domains 
platform generate
platform clean
platform generate
