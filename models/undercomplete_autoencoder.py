#!/usr/bin/env python

import torch
from torch import nn

from typing import Optional, List


class Encoder(nn.Module):
    def __init__(
        self,
        in_shape: torch.Size,
        filters=64,
        kernel_size=5,
        expansions=None,
        init_weights: bool = True,
    ):
        super().__init__()
        in_ch = in_shape[0]
        self.in_channel = in_ch
        self.filters = filters
        self.kernel = kernel_size

        self.encoder_cnn = nn.Sequential(
            nn.Conv2d(
                in_ch,
                filters,
                kernel_size,
                stride=2,
                padding=1
            ),
            nn.BatchNorm2d(filters),
            nn.ReLU(True),

            nn.Conv2d(
                filters,
                filters*expansions[0],
                kernel_size,
                stride=2,
                padding=1
            ),

            nn.BatchNorm2d(filters * expansions[0]),
            nn.ReLU(True),
        )

        self.out_channels = filters * sum([1 for m in self.encoder_cnn if isinstance(m, nn.Conv2d)])

        if init_weights:
            self._initialize_weights()

    def _initialize_weights(self, modules=None):
        if not modules:
            modules = self.modules()

        for m in modules:
            if isinstance(m, nn.Conv2d):
                nn.init.kaiming_normal_(m.weight, mode="fan_out", nonlinearity="relu")
                if m.bias is not None:
                    nn.init.constant_(m.bias, 0)
            elif isinstance(m, nn.BatchNorm2d):
                nn.init.constant_(m.weight, 1)
                nn.init.constant_(m.bias, 0)
            elif isinstance(m, nn.Linear):
                nn.init.normal_(m.weight, 0, 0.01)
                nn.init.constant_(m.bias, 0)

    def forward(self, x):
        return self.encoder_cnn(x)


class Decoder(nn.Module):
    def __init__(
        self,
        out_ch=64,
        filters=64,
        kernel_size=5,
        expansions=None,

        fix_first=False,
        fix_second=False,

        fix_first_pad=False,
        fix_second_pad=False,

        init_weights: bool = True,
    ):
        super().__init__()
        self.out_channels = out_ch
        self.filters = filters
        self.kernel = kernel_size

        self.decoder_conv = nn.Sequential(
            nn.ConvTranspose2d(
                filters * expansions[0],
                filters,
                kernel_size,
                stride=2,
                padding=1 if not fix_first_pad else 0,
                output_padding=1 if not fix_first else 0
            ),
            nn.BatchNorm2d(filters),
            nn.ReLU(True),
            nn.ConvTranspose2d(
                filters,
                out_ch,
                kernel_size,
                stride=2,
                padding=1 if not fix_second_pad else 0,
                output_padding=1 if not fix_second else 0
            ),
        )

        if init_weights:
            self._initialize_weights()

    def _initialize_weights(self, modules=None):
        if not modules:
            modules = self.modules()

        for m in modules:
            if isinstance(m, nn.Conv2d):
                nn.init.kaiming_normal_(m.weight, mode="fan_out", nonlinearity="relu")
                if m.bias is not None:
                    nn.init.constant_(m.bias, 0)
            elif isinstance(m, nn.BatchNorm2d):
                nn.init.constant_(m.weight, 1)
                nn.init.constant_(m.bias, 0)
            elif isinstance(m, nn.Linear):
                nn.init.normal_(m.weight, 0, 0.01)
                nn.init.constant_(m.bias, 0)

    def forward(self, x):
        return self.decoder_conv(x)


class UndercompleteAutoencoderclass(nn.Module):
    def __init__(self,
                 input_shape: torch.Size,
                 expansions: Optional[List[int]] = None,
                 compression_ratio=8,
                 kernel_size=3,
                 init_weights=True):
        super().__init__()

        if not isinstance(input_shape, int):
            assert len(input_shape) == 3, "Input shape must be in the shape: [C,H,W]"
        else:
            input_shape = [input_shape]

        if expansions is None:
            expansions = [2]

        self.input_channels = input_shape[0]
        self.out_channels = input_shape[0]

        self._init_weights = init_weights

        self.kernel_size = kernel_size
        self.n_filters = int(round(self.input_channels / compression_ratio))

        self.expansions = expansions

        self.encoder: Encoder = Encoder(
            in_shape=input_shape,
            filters=self.n_filters,
            kernel_size=kernel_size,
            expansions=expansions,
            init_weights=init_weights,

        )
        self.decoder: Decoder = None

        fake_xin = torch.randn(input_shape).type(torch.FloatTensor).unsqueeze(0)
        self.create_decoder(fake_xin)

    def freeze(self):
        for name, module in self.named_parameters():
            if hasattr(module, "weight"):
                module.weight.requires_grad = False
            if hasattr(module, "bias"):
                module.bias.requires_grad = False

    def unfreeze(self):
        for n, p in self.named_parameters():
            p.requires_grad = True

    def create_decoder(self, fake_xin) -> bool:

        def _fix(fix_first, fix_second, fix_first_pad, fix_second_pad):
            self.decoder = Decoder(
                out_ch=self.out_channels,
                filters=self.n_filters,
                init_weights=self._init_weights,
                kernel_size=self.kernel_size,
                expansions=self.expansions,

                fix_first=fix_first,
                fix_second=fix_second,
                fix_first_pad=fix_first_pad,
                fix_second_pad=fix_second_pad
            )
            encoded = self.encoder(fake_xin)
            decoded = self.decoder(encoded)

            s1 = fake_xin.shape
            s2 = decoded.shape

            return s1 == s2

        # Normal run.
        attempt = _fix(False, False, False, False)
        if attempt:
            return True

        # Output padding.
        attempt = _fix(True, False, False, False)
        if attempt:
            return True
        # Output padding + padding.
        attempt = _fix(True, False, True, False)
        if attempt:
            return True

        # Output padding.
        attempt = _fix(False, True, False, False)
        if attempt:
            return True
        # Output padding + padding.
        attempt = _fix(False, True, False, True)
        if attempt:
            return True

        attempt = _fix(True, True, False, False)
        if attempt:
            return True
        attempt = _fix(True, True, True, True)
        if attempt:
            return True

        attempt = _fix(False, False, True, False)
        if attempt:
            return True
        attempt = _fix(False, False, False, False)
        if attempt:
            return True
        attempt = _fix(False, False, True, True)
        if attempt:
            return True

        import warnings
        warnings.warn("[Bottleneck AutoEncoder] Decoder creation failed!")
        return False

    def forward(self, x):
        encoded = self.encoder(x)
        decoded = self.decoder(encoded)

        return decoded

    def get_split_size(self, x):
        return self.encoder(x).shape
