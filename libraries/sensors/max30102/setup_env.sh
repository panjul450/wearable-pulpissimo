# Add chroot binaries to PATH
export PATH="$PATH:/home/dzul/ICDEC/max30102_sensor/chroot/bin"

# Load PULP Runtime configuration
source /home/dzul/ICDEC/max30102_sensor/pulp-runtime/configs/pulpissimo.sh
source /home/dzul/ICDEC/max30102_sensor/pulp-runtime/configs/fpgas/pulpissimo/genesys2.sh

# FPGA frequency configuration
export PULPRT_CONFIG_ASFLAGS="-DARCHI_FPGA_PER_FREQUENCY=10000000 -DARCHI_FPGA_SOC_FREQUENCY=10000000"
export PULPRT_CONFIG_CFLAGS="-DARCHI_FPGA_PER_FREQUENCY=10000000 -DARCHI_FPGA_SOC_FREQUENCY=10000000"
export PULPRT_CONFIG_CXXFLAGS="-DARCHI_FPGA_PER_FREQUENCY=10000000 -DARCHI_FPGA_SOC_FREQUENCY=10000000"

echo "PULP environment has been set up successfully."