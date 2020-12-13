#!/bin/bash -l

#SBATCH -N 2
#SBATCH -t 15:00
#SBATCH --constraint=rhel8

#module load openmpi/gcc

N_POINTS=4096 #2^12
N_POINTS=65536 #2^16
N_POINTS=1048576 #2^20
N_POINTS=16777216 #2^24

DIST=disk-r100
DIST=rect-l100
DIST=exp-e.01

echo n${N_POINTS}_${DIST}

N_PROCESSES=1
INPUT=experimentalinput/n$N_POINTS-$DIST.txt
mkdir n${N_POINTS}_${DIST}
echo "Serial"
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_serial.txt" >> "n${N_POINTS}_${DIST}/serial.times"
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
# mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"

N_PROCESSES=2
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"

N_PROCESSES=4
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"

N_PROCESSES=8
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"

N_PROCESSES=16
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"

N_PROCESSES=32
echo "Parallel"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_${DIST}/parallelSearch"
echo "Process Pool"
mpirun --mca mpi_cuda_support 0 -np $((N_PROCESSES + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_processPool.txt" >> "n${N_POINTS}_${DIST}/processPools"
echo "Partition Space"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_${DIST}/partitionSpace"
echo "Fork Join"
mpirun --mca mpi_cuda_support 0 -np $N_PROCESSES         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_${DIST}/forkJoin"
