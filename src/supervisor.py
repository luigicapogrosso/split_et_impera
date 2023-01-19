#!/usr/bin/env python

import os
import logging
import argparse
import subprocess
import numpy as np

from stats import *
from sensing import compute
from receiver import receiver
from torchvision import models
from transmitter import transmitter


# Create logger.
logger = logging.getLogger("FDVCA_supervisor")
logger.setLevel(logging.DEBUG)

# Create console handler and set level to debug.
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)

formatter = logging.Formatter("%(levelname)s: %(message)s")

# Add formatter to ch and ch to logger.
ch.setFormatter(formatter)
logger.addHandler(ch)

# Set the application list.
tests = {
    "LC"  : "placeholder_for_training_output.pkl",
    "RC" : "placeholder_for_training_output.pkl",
    "SC"  : "placeholder_for_training_output.pkl",
    "EE"  : "placeholder_for_training_output.pkl"
}


def main(args):
    # Check command-line arguments.
    if not (args.protocol == "tcp" or args.protocol == "udp"):
        logger.critical("Unsupported protocol. Set 'tcp' or 'udp'.")
    elif not (args.loss_rate >= 0.0 and args.loss_rate <= 5.0):
        logger.critical("Unsupported loss rate value. Set [0-5]%.")
    elif not os.path.exists(args.save_dir):
        os.makedirs(args.save_dir)
    elif not args.test in list(tests.keys()):
        logger.critical("Unsupported test. Set 'LC', 'RC', 'SC', or 'EE'.")

    logger.debug(f"Starting {args.protocol} simulation "
                 f"with {args.loss_rate} loss rate. "
                 f"Results will be saved in {args.save_dir} [...]")

    # Sensing.
    logger.debug(f"Starting 'sensing' [...]")
    compute(args.save_dir, tests[args.test])
    logger.debug(f"Successfully completed the application computation.")

    # Transmitter.
    logger.debug(f"Starting 'transmitter' [...]")
    transmitter(args.save_dir)
    logger.debug(f"Successfully completed the transmitter computation.")

    # Network simulation.
    logger.debug(f"Starting the network simulation [...]")
    subprocess.run(["netsim/build/netsim",
                    os.path.join(args.save_dir, "out_transmitter.json"),
                    os.path.join(args.save_dir, "out_netsim.json"),
                    str(args.loss_rate/100),
                    str(100),
                    args.protocol])
    logger.debug(f"Successfully completed the network simulation.")

    # Receiver.
    logger.debug(f"Starting 'receiver' [...]")
    receiver(args.save_dir)
    logger.debug(f"Successfully completed the receiver computation.")

    logger.debug(f"Starting calculating the simulation statistics [...]")

    # Packet loss.
    n_of_lost_pkts = np.array(get_packet_loss(args.save_dir))
    n_of_non_lost_frames = len(np.where(n_of_lost_pkts == 0)[0])
    logger.debug(f"Number of lost packets per frames: {n_of_lost_pkts}")
    logger.debug(f"Number of lost frames: "
                 f"{len(n_of_lost_pkts) - n_of_non_lost_frames}")

    # Overall latency.
    tot_l = get_overall_latency_time(args.save_dir)
    logger.debug(f"Overall latency: {tot_l:.3f}s")

    # Frame latency.
    max_frame_l, frame_idx, avg_frame_l = get_max_frame_latency_time(args.save_dir)
    logger.debug(f"Max latency: {max_frame_l:.3f}s at frame: {frame_idx}")
    avg_frame_l = avg_frame_l if avg_frame_l > 0 else 0
    logger.debug(f"Average latency: {avg_frame_l:.3f}s")

    # Neural Network summary.
    nn_summary = get_nn_summary(models.vgg16())
    logger.debug(f"Neural Network summary:\n{nn_summary}")
    logger.debug(f"Successfully completed the simulation.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("protocol", type=str, help="'tcp' | 'udp'")
    parser.add_argument("loss_rate", type=int, help="[0-5]%")
    parser.add_argument("--save_dir", type=str, default="./",
                        help="dir where saving simulation results")
    parser.add_argument("--test", type=str, choices=list(tests.keys()),
                        default="LC",
                        help="the key of the test to be executed")
    args = parser.parse_args()

    main(args=args)
