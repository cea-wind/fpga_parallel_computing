##############################################
# Project settings

# Create a project
open_project	-reset histgram.prj

# The source file and test bench
add_files			histgram.cpp
add_files			histgram.h
add_files -tb	    tb_histgram.cpp
# Specify the top-level function for synthesis
# set_top				

###########################
# Solution settings

# Create solution1
open_solution -reset solution1

# Specify a Xilinx device and clock period
# - Do not specify a clock uncertainty (margin)
# - Let the  margin to default to 12.5% of clock period
set_part  {xc7k160tfbg484-1}
create_clock -period 4
#set_clock_uncertainty 1.25

# Simulate the C code 
# csim_design

# Do not perform any other steps
# - The basic project will be opened in the GUI 
exit
