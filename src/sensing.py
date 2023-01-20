#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = "Luigi Capogrosso, Federico Cunico, Michele Lora, \
              Marco Cristani, Franco Fummi, Davide Quaglia"
__version__ = "1.0.0"
__maintainer__ = "Luigi Capogrosso, Federico Cunico"
__email__ = "name.surname@univr.it"

import os
import json
import pickle
import logging


# Create logger.
logger = logging.getLogger("FDVCA_compute")
logger.setLevel(logging.DEBUG)

# Create console handler and set level to debug.
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)

formatter = logging.Formatter("%(levelname)s: %(message)s")

# Add formatter to ch and ch to logger.
ch.setFormatter(formatter)
logger.addHandler(ch)


def compute(save_dir, test_path, out_filename="out_compute.json"):
    logger.debug(f"Loading the {test_path} data [...]")

    # Get the application data.
    with open(test_path, "rb") as f:
        fdvca_enc_array = pickle.load(f)

    logger.debug(f"Successfully loaded {test_path} data.")

    # Create the output file content.
    fdvca_enc_result = {"encoding":"file",
                        "file":"filename.xyz",
                        "array":fdvca_enc_array}

    # Save the output file.
    with open(os.path.join(save_dir, out_filename), "w") as f:
        json.dump(fdvca_enc_result, f, ensure_ascii=False, indent=4)


if __name__ == "__main__":
    pass
