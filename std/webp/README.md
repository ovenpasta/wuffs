# WebP

WebP is an image format (still or animated), described in the [WebP Container
Specification](https://developers.google.com/speed/webp/docs/riff_container).
It is a container for [VP8 Lossy](https://www.ietf.org/rfc/rfc6386.txt) or [VP8
Lossless](https://developers.google.com/speed/webp/docs/webp_lossless_bitstream_specification).

The ["Making Software - Image
Compression"](https://www.makingsoftware.com/chapters/image-compression) book
chapter also has a good overview of a number of image formats, including WebP.

There are two WebP variants: lossy (based on VP8) and lossless (based on what's
essentially an unrelated design, other than "WebP" branding). Wuffs' VP8
implementation is in a separate package, as the WebM moving image format also
uses VP8. There's no Wuffs WebM package yet but there might be in the future.

For an extended format (VP8X) WebP image, including an animated one, the work
buffer length (from `workbuf_len`) is calculated from the image's width and
height and from the RIFF chunk length, which is an upper bound (as the encoded
VP8 data is copied into the work buffer) for any VP8 chunk's length. The work
buffer is therefore roughly as long as the file, for an animated image even if
each of its frames is small. A VP8X file whose RIFF chunk length is more than
the largest VP8 chunk length that std/vp8 supports (just under 144 MiB) is
rejected as an "unsupported WebP file".
