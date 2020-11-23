#!/bin/bash -l

#SBATCH -N 2
#SBATCH -t 00:10
#SBATCH --constraint=rhel8

#module load openmpi/gcc

N_POINTS=262144
INPUT=testinput/n262144-disk-r10.txt

N_POINTS=16385
INPUT=testinput/n16385-disk-r10.txt

N_POINTS=65536
INPUT=testinput/n65536-disk-r10.txt

N_POINTS=4096
INPUT=testinput/n4096-disk-r10.txt

DIST=disk_r10
N_PROCESSES=1
mkdir n${N_POINTS}_p${N_PROCESSES}_${DIST}
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
N_PROCESSES=2
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
N_PROCESSES=4
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
N_PROCESSES=8
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
N_PROCESSES=16
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
N_PROCESSES=32
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.times"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/parallelSearch"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/processPools"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/partitionSpace"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt" >> "n${N_POINTS}_p${N_PROCESSES}_${DIST}/forkJoin"
