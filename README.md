# Split-Et-Impera: A Framework for the Design of Distributed Deep Learning Applications #

Official implementation of the paper [Split-Et-Impera: A Framework for the Design of Distributed Deep Learning Applications](https://luigicapogrosso.github.io/split_et_impera/) accepted at the 26th International Symposium on Design and Diagnostics of Electronic Circuits and Systems (DDECS 2023)

**Split-Et-Impera** is a novel and practical framework that *i)* determines the set of the best-split points of a neural network based on deep network interpretability principles without performing a tedious try-and-test approach, *ii)* performs a communication-aware simulation for the rapid evaluation of different neural network rearrangements, and *iii)* suggests the best match between the application's quality of service requirements and performance in terms of accuracy and latency time.

## Dependencies ##
Split-Et-Impera has the following dependencies:
- **Sphinx** for documentation generation (optional);
- **SCNSL** for simulations of networked systems (mandatory).

SCNSL is a SystemC Network Simulation Library licensed under the LGPL license.
It extends SystemC's capabilities to perform simulations of networked systems.
Here is the [official repository](https://gitlab.com/open-kappa/scnsl), and here is the [documentation](https://open-kappa.gitlab.io/scnsl/) of the library.
For further information about SCNSL, check the [paper](https://ieeexplore.ieee.org/abstract/document/4641420).

## Installation ##
1. Clone this repo, and we'll call the directory that you cloned as ${SEI_ROOT}.
2. Install dependencies. We use Python >= 3.8 and PyTorch >= 1.7.0.
3. To use the code from sources in a clean way, it is preferable that you set up a conda environment, say:
    ```
    $ conda create -n sei python=3.8
    $ conda activate sei
    $ conda install pytorch==1.7.0 torchvision==0.8.0 cudatoolkit=11.3 -c pytorch
    $ pip install -r requirements.txt
    ```
## License ##
The software files in this repository are provided under the [MIT License](./LICENSE).

## Authors ##
Luigi Capogrosso, Federico Cunico, Michele Lora, Marco Cristani, Franco Fummi, Davide Quaglia

*Department of Computer Science, University of Verona, Italy*

`name.surname@univr.it`

## Citation ##
If you use [**Split-Et-Impera**](https://ieeexplore.ieee.org/abstract/document/10139711), please, cite the following paper:
```
@inproceedings{capogrosso2023split,
  title={Split-Et-Impera: A Framework for the Design of Distributed Deep Learning Applications},
  author={Capogrosso, Luigi and Cunico, Federico and Lora, Michele and Cristani, Marco and Fummi, Franco and Quaglia, Davide},
  booktitle={2023 26th International Symposium on Design and Diagnostics of Electronic Circuits and Systems (DDECS)},
  pages={39--44},
  year={2023},
  organization={IEEE}
}
```
