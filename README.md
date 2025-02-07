# Tiny CUDA Neural Networks

This is a small, self-contained framework for training and querying neural networks. Most notably, it contains a lightning fast "fully fused" multi-layer perceptron as well as support for various advanced input encodings, losses, and optimizers.

This framework powers the following publication:

> __Real-time Neural Radiance Caching for Path Tracing__  
> [Thomas Müller](https://tom94.net), [Fabrice Rousselle](https://research.nvidia.com/person/fabrice-rousselle), [Jan Novák](http://jannovak.info), [Alexander Keller](https://research.nvidia.com/person/alex-keller)  
> _To appear: ACM Transactions on Graphics (SIGGRAPH) 2021_
>
> [GTC talk](https://gtc21.event.nvidia.com/media/Fully%20Fused%20Neural%20Network%20for%20Radiance%20Caching%20in%20Real%20Time%20Rendering%20%5BE31307%5D/1_liqy6k1c)

For business inquiries, please contact <researchinquiries@nvidia.com>.  
For press and other inquiries, please contact Hector Marinez at <hmarinez@nvidia.com>.


## Performance

![Image](data/readme/fully-fused-vs-tensorflow.png)
_Fully fused networks vs. TensorFlow v2.5.0 w/ XLA. Measured on 64 (solid line) and 128 (dashed line) neurons wide multi-layer perceptrons on an RTX 3090. Generated by `benchmarks/bench_ours.cu` and `benchmarks/bench_tensorflow.py`._


## License and Citation

This framework is licensed under the BSD 3-clause license. Please see `LICENSE.txt` for details.

If you use it in your research, we would appreciate a citation via
```bibtex
@misc{tiny-cuda-nn,
    Author = {Thomas M\"uller},
    Year = {2021},
    Note = {https://github.com/nvlabs/tiny-cuda-nn},
    Title = {Tiny {CUDA} Neural Network Framework}
}
```

Special thanks go to the NRC authors for helpful discussions and to [Nikolaus Binder](https://research.nvidia.com/person/nikolaus-binder) for providing part of the infrastructure of this framework, as well as for help with utilizing TensorCores from within CUDA.


## Usage

Tiny CUDA neural networks have a simple C++/CUDA API:

```cpp
#include <tiny-cuda-nn/common.h>

// Configure the model
nlohmann::json config = {
	{"loss", {
		{"otype", "L2"}
	}},
	{"optimizer", {
		{"otype", "Adam"},
		{"learning_rate", 1e-3},
	}},
	{"encoding", {
		{"otype", "OneBlob"},
		{"n_bins", 32},
	}},
	{"network", {
		{"otype", "FullyFusedMLP"},
		{"n_neurons", 64},
		{"n_hidden_layers", 5},
		{"activation", "ReLU"},
		{"output_activation", "None"},
	}},
};

using namespace tcnn;

auto [loss, optimizer, network, trainer] =
	create_from_config(n_input_dims_to_encode, n_input_dims_to_pass_through, n_output_dims, config);

// Train the model
GPUMatrix<float, MatrixLayout::ColumnMajor> training_batch_inputs(n_input_dims, batch_size);
GPUMatrix<float, MatrixLayout::ColumnMajor> training_batch_targets(n_output_dims, batch_size);

for (int i = 0; i < n_training_steps; ++i) {
	generate_training_batch(&training_batch_inputs, &training_batch_targets); // <-- your code

	float loss;
	trainer->training_step(nullptr, training_batch_inputs, training_batch_targets, &loss);
	std::cout << "iteration=" << i << " loss=" << loss << std::endl;
}

// Use the model
GPUMatrix<float, MatrixLayout::ColumnMajor> inference_inputs(n_input_dims, batch_size);
generate_inputs(&inference_inputs); // <-- your code

GPUMatrix<float, MatrixLayout::ColumnMajor> inference_outputs(n_output_dims, batch_size);
network->inference(nullptr, inference_inputs, inference_outputs);
```


## Example: learning a 2D image

We provide a sample application where an image function _(x,y) -> (R,G,B)_ is learned. It can be run via
```sh
tiny-cuda-nn/build> ./mlp_learning_an_image ../data/images/albert.exr ../data/config.json
```
producing an image every 1000 training steps. Each 1000 steps should take roughly 0.8 seconds with the default configuration on an RTX 3090.

| Learned image after 1,000 steps | Learned image after 10,000 steps | Reference image |
|:---:|:---:|:---:|
| ![1,000 steps](data/readme/learned_image_after_1000_steps.jpg) | ![10,000 steps](data/readme/learned_image_after_10000_steps.jpg) | ![reference](data/readme/reference_image.jpg) |



## Requirements

- CUDA __v11.2 or higher__.
- CMake __v3.17 or higher__.
- A __C++14__ capable compiler.
- A high-end NVIDIA GPU that supports TensorCores and has a large amount of shared memory. The framework was tested primarily with an RTX 3090.
	- Ampere GeForce GPUs: compiles out of the box.
	- Ampere A100: requires changing `CMAKE_CUDA_ARCHITECTURE` to 80 in `CMakeLists.txt`.
	- Turing GPUs: requires changing `CMAKE_CUDA_ARCHITECTURE` to 75 in `CMakeLists.txt` as well as changing `SmArch` in `include/tiny-cuda-nn/cutlass_matmul.h` to `cutlass::arch::Sm75`.


## Compilation

Begin by cloning this repository and all its submodules using the following command:
```sh
> git clone --recursive https://github.com/nvlabs/tiny-cuda-nn
> cd tiny-cuda-nn
tiny-cuda-nn>
```

Then, use CMake to generate build files:

```sh
tiny-cuda-nn> mkdir build
tiny-cuda-nn> cd build
tiny-cuda-nn/build> cmake ..
```

Then, depending on your operating system

On Windows, open `tiny-cuda-nn/build/tiny-cuda-nn.sln` in Visual Studio and click the "Build" button.
On Linux you can compile with
```sh
tiny-cuda-nn/build> make -j
```

## Components

The following is a summary of all components of this framework that are currently released. Please consult [the JSON documentation](DOCUMENTATION.md) for how to configure them.


| Networks | &nbsp; | &nbsp;
| :--- | :---------- | :-----
| Fully fused MLP | `src/fully_fused_mlp.cu` | Lightning fast implementation of small multi-layer perceptrons (MLPs).
| CUTLASS MLP     | `src/cutlass_mlp.cu`     | MLP based on [CUTLASS](https://github.com/NVIDIA/cutlass)' GEMM routines. Slower than fully-fused, but handles larger networks and still is reasonably fast.
| CUTLASS ResNet  | `src/cutlass_resnet.cu`  | Fully connected residual network based on CUTLASS' GEMM routines.

| Input encodings | &nbsp; | &nbsp;
| :--- | :---------- | :-----
| Identity | `include/tiny-cuda-nn/encodings/identity.h` | Leaves values untouched.
| Oneblob | `include/tiny-cuda-nn/encodings/oneblob.h` | From Neural Importance Sampling [[Müller et al. 2019]](https://tom94.net/data/publications/mueller18neural/mueller18neural-v4.pdf) and Neural Control Variates [[Müller et al. 2020]](https://tom94.net/data/publications/mueller20neural/mueller20neural.pdf).
| Frequency | `include/tiny-cuda-nn/encodings/frequency.h` | From NeRF [[Mildenhall et al. 2020]](https://www.matthewtancik.com/nerf).
| NRC | `include/tiny-cuda-nn/encodings/nrc.h` | Combined oneblob and frequency encoding used in Neural Radiance Caching [[Müller et al. 2021]](https://tom94.net/).

| Losses | &nbsp; | &nbsp;
| :--- | :---------- | :-----
| L2 | `include/tiny-cuda-nn/losses/l2.h` | Standard L2 loss.
| Relative L2 | `include/tiny-cuda-nn/losses/relative_l2.h` | Relative L2 loss normalized by the network prediction [[Lehtinen et al. 2018]](https://github.com/NVlabs/noise2noise).
| Relative L2 Luminance | `include/tiny-cuda-nn/losses/relative_l2_luminance.h` | Same as above, but normalized by the luminance of the network prediction. Only applicable when network prediction is RGB. Used in Neural Radiance Caching [[Müller et al. 2021]](https://tom94.net/).
| Cross Entropy | `include/tiny-cuda-nn/losses/cross_entropy.h` | Standard cross entropy loss. Only applicable when the network prediction is a PDF.
| Variance | `include/tiny-cuda-nn/losses/variance_is.h` | Standard variance loss. Only applicable when the network prediction is a PDF.

| Optimizers | &nbsp; | &nbsp;
| :--- | :---------- | :-----
| Adam | `include/tiny-cuda-nn/optimizers/adam.h` | Implementation of Adam [[Kingma and Ba 2014]](https://arxiv.org/abs/1412.6980), generalized to AdaBound [[Luo et al. 2019]](https://github.com/Luolc/AdaBound).
| Novograd | `include/tiny-cuda-nn/optimizers/lookahead.h` | Implementation of Novograd [[Ginsburg et al. 2019]](https://arxiv.org/abs/1905.11286).
| SGD | `include/tiny-cuda-nn/optimizers/sgd.h` | Standard stochastic gradient descent (SGD).
| Shampoo | `include/tiny-cuda-nn/optimizers/shampoo.h` | Implementation of the 2nd order Shampoo optimizer [[Gupta et al. 2018]](https://arxiv.org/abs/1802.09568) with home-grown optimizations as well as those by [Anil et al. [2020]](https://arxiv.org/abs/2002.09018).
| Average | `include/tiny-cuda-nn/optimizers/average.h` | Wraps another optimizer and computes a linear average of the weights over the last N iterations. The average is used for inference only (does not feed back into training).
| Batched | `include/tiny-cuda-nn/optimizers/batched.h` | Wraps another optimizer, invoking the nested optimizer once every N steps on the averaged gradient. Has the same effect as increasing the batch size but requires only a constant amount of memory. |
| EMA | `include/tiny-cuda-nn/optimizers/average.h` | Wraps another optimizer and computes an exponential moving average of the weights. The average is used for inference only (does not feed back into training).
| Exponential Decay | `include/tiny-cuda-nn/optimizers/exponential_decay.h` | Wraps another optimizer and performs piecewise-constant exponential learning-rate decay.
| Lookahead | `include/tiny-cuda-nn/optimizers/lookahead.h` | Wraps another optimizer, implementing the lookahead algorithm [[Zhang et al. 2019]](https://arxiv.org/abs/1907.08610).



