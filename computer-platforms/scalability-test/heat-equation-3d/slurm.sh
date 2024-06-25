#!/bin/bash
#SBATCH --job-name=heat_eq-16
#SBATCH --account=project_465001194
#SBATCH --error=%x.%J.err
#SBATCH --output=%x.%J.out
#SBATCH --partition=small-g
#SBATCH --time=00:05:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=8
#SBATCH --gpus-per-node=8
#SBATCH --reservation=CSC_summer_school_gpu

export MPICH_GPU_SUPPORT_ENABLED=1
srun heat_hip
