/*
 * Copyright (c) 2020-2021, NVIDIA CORPORATION.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of
 *       conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TOR (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *//*
 */

/** @file   network.h
 *  @author Thomas Müller, NVIDIA
 *  @brief  API interface of a neural network implementation
 */

#pragma once

#include <tiny-cuda-nn/common.h>
#include <tiny-cuda-nn/object.h>


TCNN_NAMESPACE_BEGIN

void cutlass_free_workspace(cudaStream_t stream);

enum class WeightUsage {
	Inference,
	Forward,
	Backward,
};

template <typename T>
class Network : public DifferentiableObject<T, T> {
public:
	virtual ~Network() { }

	virtual void inference_mixed_precision(cudaStream_t stream, const GPUMatrix<T, MatrixLayout::ColumnMajor>& input, GPUMatrix<T, MatrixLayout::ColumnMajor>& output, MatrixLayout output_layout = MatrixLayout::ColumnMajor) = 0;
};

template <typename T>
Network<T>* create_network(json network);

TCNN_NAMESPACE_END
