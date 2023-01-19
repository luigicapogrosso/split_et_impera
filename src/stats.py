#!/usr/bin/env python

import os
import json

from torchinfo import summary


def get_packet_loss(save_dir, in_filename="out_netsim.json"):
    with open(os.path.join(save_dir, in_filename), "r") as f:
        data = json.load(f)

    res = []
    lost_pkts = 0
    last_idx = -1

    for d in data["array"]:
        curr_idx = d["number"]

        if curr_idx > last_idx:
            if d["time"] == "inf":
                lost_pkts = lost_pkts + 1
            last_idx = curr_idx
        else:
            res.append(lost_pkts)
            lost_pkts = 0
            if d["time"] == "inf":
                lost_pkts = lost_pkts + 1
            last_idx = curr_idx

    res.append(lost_pkts)

    return res


def get_overall_latency_time(save_dir, in_filename="out_receiver.json"):
    with open(os.path.join(save_dir, in_filename), "r") as f:
        data = json.load(f)

    i = 0
    tot = 0
    max = 0
    for d in data["array"]:
        l = d["time"]
        if l > max:
            max = l
        tot = tot + l
        i = i + 1

    return max


def get_max_frame_latency_time(save_dir,
                               in_filename_1="out_compute.json",
                               in_filename_2="out_receiver.json"):
    with open(os.path.join(save_dir, in_filename_1), "r") as f:
        compute_data = json.load(f)

    with open(os.path.join(save_dir, in_filename_2), "r") as f:
        receiver_data = json.load(f)

    i = 0
    tot = 0
    max = 0
    max_idx = -1
    for d in compute_data['array']:
        l = receiver_data['array'][i]['time'] - d['time']
        if l > max:
            max = l
            max_idx = i
        tot = tot + l
        i = i + 1

    return max, max_idx, tot/i


def get_nn_summary(net, bs=16):
    model = net
    batch_size = bs

    model_stats = summary(model,
                          input_size=(batch_size, 3, 224, 224),
                          verbose=0)

    return str(model_stats)