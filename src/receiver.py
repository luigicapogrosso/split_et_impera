#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = "Luigi Capogrosso, Federico Cunico, Michele Lora, \
              Marco Cristani, Franco Fummi, Davide Quaglia"
__version__ = "1.0.0"
__maintainer__ = "Luigi Capogrosso, Federico Cunico"
__email__ = "name.surname@univr.it"

import os
import json


def receiver(save_dir,
             in_filename="out_netsim.json",
             out_filename="out_receiver.json"):
    with open(os.path.join(save_dir, in_filename), "r") as f:
        data = json.load(f)

    size = 0
    time = 0
    last_idx = -1
    packet_idx = 0
    fdvca_enc_array = []

    for d in data["array"]:
        curr_idx = d["number"]

        if curr_idx > last_idx:
            size = size + d["size"]
            if d["time"] != "inf":
                time = d["time"]
            last_idx = curr_idx
        else:
            fdvca_enc_array.append({"number":packet_idx,
                                    "time":time,
                                    "size":size,
                                    "data":"none"})
            packet_idx = packet_idx + 1
            size = d["size"]
            if d["time"] != "inf":
                time = d["time"]
            last_idx = curr_idx

    fdvca_enc_array.append({"number":packet_idx,
                            "time":time,
                            "size":size,
                            "data":"none"})

    # Create the output file content.
    fdvca_enc_result = {"encoding":"file",
                        "file":"filename.xyz",
                        "array":fdvca_enc_array}

    # Save the output file.
    with open(os.path.join(save_dir, out_filename), "w") as f:
        json.dump(fdvca_enc_result, f, ensure_ascii=False, indent=4)


if __name__ == "__main__":
    pass
