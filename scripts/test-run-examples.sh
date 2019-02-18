#!/bin/bash
###############################################################################
# Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
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

# add spack's mpiexec to path
#export PATH=$PATH:`ls -d uberenv_libs/spack/opt/spack/*/*/mpich-*/bin/`

# run noise
build-debug/examples/synthetic/noise/noise_ser

# run kripke
mpiexec -n 2 build-debug/examples/proxies/kripke/kripke_par --procs 2,1,1  --zones 32,32,32 --niter 3 --dir 1:12 --grp 1:1 --legendre 4 --quad 20:20 --dir 50:1

# run clover
cp build-debug/examples/proxies/cloverleaf3d-ref/clover.in .
mpiexec -n 2 build-debug/examples/proxies/cloverleaf3d-ref/cloverleaf3d_par

# run lulesh
mpiexec -n 8 build-debug/examples/proxies/lulesh2.0.3/lulesh_par -p -i 5


if [ -d "build-debug/examples/proxies/laghos" ]; then
  ./build-debug/examples/proxies/laghos/laghos_ser -rs 1 -tf 0.2 -m build-debug/examples/proxies/laghos/data/square01_quad.mesh --visit
fi
