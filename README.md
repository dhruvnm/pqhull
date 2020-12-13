# pqhull
Parallel Quickhull



To compile:
run `make` in current directory

To Generate Input:
    Uniform Disk
        ./genPoints -n <number of points> -o <outfile> -r 100
    Uniform Rectangle
        ./genPoints -n <number of points> -o <outfile> --min -50 --max 50
    Exponential
        ./genPoints -n <number of points> -o <outfile> -e 100

To Run Programs
    Serial
        ./parallelSearch -s -n <number of points> -i <input file> -o <outfile>
    ParallelSearch
        mpirun --mca mpi_cuda_support 0 -np <number of processes> ./parallelSearch -s -n <number of points> -i <input file> -o <outfile>
    ProcessPool
        mpirun --mca mpi_cuda_support 0 -np <number of processes> ./processPool -s -n <number of points> -i <input file> -o <outfile>
    PartitionSpace
        mpirun --mca mpi_cuda_support 0 -np <number of processes> ./partitionSpace -s -n <number of points> -i <input file> -o <outfile>
    ForkJoin
        mpirun --mca mpi_cuda_support 0 -np <number of processes> ./forkJoin -s -n <number of points> -i <input file> -o <outfile>
