# VP8

VP8 is a compressed video data format, described in [RFC
6386](https://www.ietf.org/rfc/rfc6386.txt).

The WebM movie (moving image) format consists of muliple VP8 frames, both key
(I-Frames) and non-key (B-Frames, P-Frames, etc). Separate from WebM, a WebP
still image essentially wraps one VP8 key frame. An animated WebP image wraps
one VP8 key frame per animation frame (or VP8 Lossless data instead, for some
or all of its frames), each independent of the others.
