#!/bin/bash -l

#SBATCH -N 1
#SBATCH -t 00:10
#SBATCH --constraint=rhel8

module load openmpi/gcc

N_POINTS=4096
INPUT=testinput/n4096-disk-r10.txt
DIST=disk_r10
N_PROCESSES=1
./parallelSearch -s -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_serial.txt"
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt"
N_PROCESSES=2
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt"
N_PROCESSES=4
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt"
N_PROCESSES=8
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt"
N_PROCESSES=16
mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./parallelSearch -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_parallelSearch.txt"
mpirun --mca mpi_cuda_support 0 -np $((N_POINTS + 2)) ./processPool    -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_processPool.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./partitionSpace -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_partitionSpace.txt"
# mpirun --mca mpi_cuda_support 0 -np $N_POINTS         ./forkJoin       -n $N_POINTS -i $INPUT -o "n${N_POINTS}_p${N_PROCESSES}_${DIST}_forkJoin.txt"


N_POINTS=16385
INPUT=testinput/n16385-disk-r10.txt


N_POINTS=65536
INPUT=testinput/n65536-disk-r10.txt


N_POINTS=262144
INPUT=testinput/n262144-disk-r10.txt
