###############################################################################
# Copyright (c) 2015-2018, Lawrence Livermore National Security, LLC.
#
# Produced at the Lawrence Livermore National Laboratory
#
# LLNL-CODE-716457
#
# All rights reserved.
#
# This file is part of Ascent.
#
# For details, see: http://ascent.readthedocs.io/.
#
# Please also read ascent/LICENSE
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the disclaimer below.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the disclaimer (as noted below) in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the LLNS/LLNL nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
# LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################

set(CMAKE_BUILD_TYPE "Release" CACHE PATH "")

# c compiler
set(CMAKE_C_COMPILER "/opt/mpich/bin/mpicc" CACHE PATH "")
set(C_COMPILE_FLAGS "-fPIC" CACHE PATH "")

# cpp compiler
set(CMAKE_CXX_COMPILER "/opt/mpich/bin/mpicxx" CACHE PATH "")
set(CXX_COMPILE_FLAGS "-fPIC" CACHE PATH "")

# fortran compiler (need for cloverleaf)
set(ENABLE_FORTRAN ON CACHE PATH "")
set(CMAKE_Fortran_COMPILER  "/usr/bin/f95" CACHE PATH "")

# OPENMP (optional: for proxy apps)
set(ENABLE_OPENMP ON CACHE PATH "")

# MPI Support
set(ENABLE_MPI  ON CACHE PATH "")

set(MPI_C_COMPILER  "/opt/mpich/bin/mpicc" CACHE PATH "")
set(MPI_C_COMPILE_FLAGS "-fPIC" CACHE PATH "")

set(MPI_CXX_COMPILER "/opt/mpich/bin/mpixx" CACHE PATH "")
set(MPI_CXX_COMPILE_FLAGS "-fPIC" CACHE PATH "")

set(MPI_Fortran_COMPILER "/opt/mpich/bin/mpif90" CACHE PATH "")

set(MPIEXEC "/opt/mpich/bin/mpirun" CACHE PATH "")

set(MPIEXEC_NUMPROC_FLAG -n CACHE PATH "")

##python
set(ENABLE_PYTHON OFF CACHE PATH "")

# CUDA support
#set(ENABLE_CUDA ON CACHE PATH "")

# NO CUDA Support
set(ENABLE_CUDA OFF CACHE PATH "")

#lulesh vis mode
set(VIS_MESH ON CACHE PATH "")

# ascent
set(ASCENT_DIR "/home/jkress/packages/ascentMaster/install-release" CACHE PATH "")

# conduit
set(CONDUIT_DIR "/home/jkress/packages/conduit/install" CACHE PATH "")

#vtk-h
set(VTKH_DIR "/home/jkress/packages/vtkhMaster/install" CACHE PATH "")

#
# vtkm
#

# tbb
set(ASCENT_VTKM_USE_TBB OFF CACHE PATH "")
#set(TBB_DIR "/usr/include" CACHE PATH "")

# vtkm
set(VTKM_DIR "/home/jkress/packages/vtkmMaster/vtkmInstall" CACHE PATH "")

# HDF5 support (optional)
# hdf5
set(HDF5_DIR "/opt/hdf5/" CACHE PATH "")
set(HDF5_INCLUDE_DIRS "/opt/hdf5/include" CACHE PATH "")

set(ADIOS_DIR "/home/jkress/packages/ADIOS/install" CACHE PATH "")

#SPHINX documentation building
#set("SPHINX_EXECUTABLE" "/path/to/sphinx-build" CACHE PATH "")

##################################
# end boilerplate host-config
##################################
