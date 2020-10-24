##############################################
# Project settings

# Create a project
open_project	-reset block_matrix_mul.prj

# The source file and test bench
add_files			block_matrix_mul.cpp
add_files			block_matrix_mul.h
add_files -tb	    tb_block_matrix_mul.cpp
# Specify the top-level function for synthesis
# set_top

###########################
# Solution settings

# Create solution1
open_solution -reset solution1

# Specify a Xilinx device and clock period
# - Do not specify a clock uncertainty (margin)
# - Let the  margin to default to 12.5% of clock period
set_part {xcvu9p-flga2104-2l-e}
create_clock -period 4 -name default
#set_clock_uncertainty 1.25

# Simulate the C code 
# csim_design

# Do not perform any other steps
# - The basic project will be opened in the GUI 
exit
