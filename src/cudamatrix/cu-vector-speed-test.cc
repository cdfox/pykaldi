// cudamatrix/cu-vector-speed-test.cc

// Copyright 2013  Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#include <iostream>
#include <vector>
#include <cstdlib>

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-vector.h"
#include "cudamatrix/cu-math.h"

using namespace kaldi;


namespace kaldi {

template<typename Real>
std::string NameOf() {
  return (sizeof(Real) == 8 ? "<double>" : "<float>");
}

template<typename Real> void TestCuVectorSoftmax(int32 dim) {
  BaseFloat time_in_secs = 0.05;
  CuVector<Real> M(dim);
  M.SetRandn();

  Timer tim;
  int32 iter = 0;
  for (;tim.Elapsed() < time_in_secs; iter++) {
    M.ApplySoftMax();
  }

  BaseFloat fdim = dim;
  BaseFloat gflops = (fdim * iter) / (tim.Elapsed() * 1.0e+09);
  KALDI_LOG << "For CuVector::Softmax" << NameOf<Real>() << ", for dim = "
            << dim << ", speed was " << gflops << " gigaflops.";
}


template<typename Real> void TestCuVectorSum(int32 dim) {
  BaseFloat time_in_secs = 0.05;
  CuVector<Real> M(dim);
  M.SetRandn();

  Timer tim;
  int32 iter = 0;
  for (;tim.Elapsed() < time_in_secs; iter++) {
    M.Sum();
  }

  BaseFloat fdim = dim;
  BaseFloat gflops = (fdim * iter) / (tim.Elapsed() * 1.0e+09);
  KALDI_LOG << "For CuVector::Sum" << NameOf<Real>() << ", for dim = "
            << dim << ", speed was " << gflops << " gigaflops.";
}


template<typename Real> void TestCuVectorVecVecOne(int32 dim) {
  BaseFloat time_in_secs = 0.05;
  CuVector<Real> M(dim);
  M.SetRandn();

  Timer tim;
  int32 iter = 0;
  for (;tim.Elapsed() < time_in_secs; iter++) {
    CuVector<Real> ones(dim);
    ones.Set(1.0);
    VecVec(M, ones);
  }

  BaseFloat fdim = dim;
  BaseFloat gflops = (fdim * iter) / (tim.Elapsed() * 1.0e+09);
  KALDI_LOG << "For CuVector::VecVecOne" << NameOf<Real>() << ", for dim = "
            << dim << ", speed was " << gflops << " gigaflops.";
}




template<typename Real> void TestCuVectorAddDiagMatMat(int32 dim) {
  BaseFloat time_in_secs = 0.05;
  CuVector<Real> v(dim);
  v.SetRandn();
  CuMatrix<Real> N(dim, dim), O(dim, dim);
  N.SetRandn(); O.SetRandn();

  Timer tim;
  int32 iter = 0;
  
  for (;tim.Elapsed() < time_in_secs; iter++) {
    v.AddDiagMatMat(1.0, N, kNoTrans, O, kNoTrans, 1.0);
  }

  BaseFloat fdim = dim;
  BaseFloat gflops = (fdim * fdim * iter) / (tim.Elapsed() * 1.0e+09);
  KALDI_LOG << "For CuVector::AddDiagMatMat" << NameOf<Real>() << ", for dim = "
            << dim << ", speed was " << gflops << " gigaflops.";
}



template<typename Real> void CudaVectorSpeedTest() {
  std::vector<int32> sizes;
  sizes.push_back(16);
  sizes.push_back(128);
  sizes.push_back(256);
  sizes.push_back(1024);
  int32 ns = sizes.size();
  for (int32 s = 0; s < ns; s++) {
	  TestCuVectorSoftmax<Real>(sizes[s]);
  }


  for (int32 s = 0; s < ns; s++) {
          TestCuVectorSum<Real>(sizes[s]);
  }

  for (int32 s = 0; s < ns; s++) {
          TestCuVectorVecVecOne<Real>(sizes[s]);
  }

  for (int32 s = 0; s < ns; s++) {
    TestCuVectorAddDiagMatMat<Real>(sizes[s]);
  }
  
}


} // namespace kaldi


int main() {
    //Select the GPU
#if HAVE_CUDA == 1
    CuDevice::Instantiate().SelectGpuId("yes"); //-2 .. automatic selection
#endif

    kaldi::CudaVectorSpeedTest<float>();
#if HAVE_CUDA == 1
  if (CuDevice::Instantiate().DoublePrecisionSupported()) {
    kaldi::CudaVectorSpeedTest<double>();
  } else {
    KALDI_WARN << "Double precision not supported";
  }
#else
  kaldi::CudaVectorSpeedTest<double>();
#endif
  std::cout << "Tests succeeded.\n";
}

