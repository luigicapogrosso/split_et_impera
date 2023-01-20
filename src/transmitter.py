#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = "Luigi Capogrosso, Federico Cunico, Michele Lora, \
              Marco Cristani, Franco Fummi, Davide Quaglia"
__version__ = "1.0.0"
__maintainer__ = "Luigi Capogrosso, Federico Cunico"
__email__ = "name.surname@univr.it"

import os
import json


def transmitter(save_dir,
                in_filename="out_compute.json",
                out_filename="out_transmitter.json",
                buffer_len=1024):
    with open(os.path.join(save_dir, in_filename), "r") as f:
        data = json.load(f)

    fdvca_enc_array = []
    read_len = buffer_len - 14

    for d in data["array"]:
        idx = 0
        size = 0
        while size != d["size"]:
            packet_len = min(read_len, d["size"] - size)
            fdvca_enc_array.append({"number":idx,
                                    "time":d["time"],
                                    "size":packet_len,
                                    "data":"none"})
            idx = idx + 1
            size = size + packet_len

    # Create the output file content.
    fdvca_enc_result = {"encoding":"file",
                        "file":"filename.xyz",
                        "array":fdvca_enc_array}

    # Save the output file.
    with open(os.path.join(save_dir, out_filename), "w") as f:
        json.dump(fdvca_enc_result, f, ensure_ascii=False, indent=4)


if __name__ == "__main__":
    pass
