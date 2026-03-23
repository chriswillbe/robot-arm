# ============================================================
# build_vivado.tcl
#
# Rebuild the Vivado project from repo sources and generate
# a bitstream using the block design wrapper as top.
#
# Assumptions:
#   - Custom packaged IP lives in ./fpga/ip
#   - Constraints live in ./fpga/constraints/robot_controller.xdc
#   - Block design Tcl lives in ./fpga/bd/robot_controller_bd.tcl
#   - BD name is "robot_controller"
#   - Target part is Zybo Z7-20: xc7z020clg400-1
# ============================================================

# ----- Project settings -----
set project_name robot_controller
set project_dir  [file normalize ./build/vivado]
set part_name    xc7z020clg400-1
set bd_name      robot_controller

set repo_root    [file normalize .]
set ip_repo_dir  [file normalize ./fpga/ip]
set xdc_file     [file normalize ./fpga/constraints/robot_controller.xdc]
set bd_tcl_file  [file normalize ./fpga/bd/robot_controller_bd.tcl]
set bd_file_repo [file normalize ./fpga/bd/robot_controller.bd]

# ----- Start clean project -----
create_project $project_name $project_dir -part $part_name -force
set_property target_language Verilog [current_project]

# ----- Register local IP repo -----
if {[file isdirectory $ip_repo_dir]} {
    puts "INFO: Adding local IP repo: $ip_repo_dir"
    set_property ip_repo_paths [list $ip_repo_dir] [current_project]
    update_ip_catalog
} else {
    puts "ERROR: Local IP repo not found: $ip_repo_dir"
    exit 1
}

# ----- Add constraints -----
if {[file exists $xdc_file]} {
    puts "INFO: Adding constraints: $xdc_file"
    add_files -fileset constrs_1 $xdc_file
} else {
    puts "ERROR: Constraints file not found: $xdc_file"
    exit 1
}

# ----- Recreate or import block design -----
if {[file exists $bd_tcl_file]} {
    puts "INFO: Rebuilding block design from Tcl: $bd_tcl_file"
    source $bd_tcl_file
} elseif {[file exists $bd_file_repo]} {
    puts "WARNING: BD Tcl not found, importing BD file directly: $bd_file_repo"
    add_files -norecurse $bd_file_repo
} else {
    puts "ERROR: No block design source found."
    puts "       Expected one of:"
    puts "         $bd_tcl_file"
    puts "         $bd_file_repo"
    exit 1
}

# ----- Find BD file inside generated project -----
set bd_obj [get_files -quiet */$bd_name.bd]
if {[llength $bd_obj] == 0} {
    puts "ERROR: Could not find block design '$bd_name.bd' after import/source."
    exit 1
}
set bd_obj [lindex $bd_obj 0]

puts "INFO: Using BD object: $bd_obj"

# ----- Validate and generate BD outputs -----
validate_bd_design
save_bd_design
generate_target all $bd_obj

# Some flows behave better with synth checkpoint disabled on BD file
catch { set_property generate_synth_checkpoint false $bd_obj }

# ----- Create and add wrapper -----
set wrapper_files [make_wrapper -files [list $bd_obj] -top]
if {[llength $wrapper_files] == 0} {
    puts "ERROR: make_wrapper did not return a wrapper file."
    exit 1
}

foreach wf $wrapper_files {
    puts "INFO: Adding wrapper: $wf"
    add_files -norecurse $wf
}

set wrapper_top "${bd_name}_wrapper"
set_property top $wrapper_top [current_fileset]
puts "INFO: Top module set to: $wrapper_top"

# ----- Update compile order -----
update_compile_order -fileset sources_1

# ----- Build bitstream -----
launch_runs synth_1
wait_on_run synth_1

launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1

# ----- Optional: export hardware platform with bitstream -----
set xsa_out [file normalize ./build/${project_name}.xsa]
open_run impl_1
write_hw_platform -fixed -include_bit -file $xsa_out -force
# write_hw_platform -fixed -include_bit -file $xsa_out

puts "INFO: Build complete."
puts "INFO: Bitstream should be under:"
puts "      $project_dir/$project_name.runs/impl_1/"
puts "INFO: XSA written to:"
puts "      $xsa_out"
