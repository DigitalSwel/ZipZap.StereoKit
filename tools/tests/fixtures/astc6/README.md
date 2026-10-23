# ASTC6 KTX2 loader fixtures

Synthetic assets copied unchanged from DigitalSwel/ImageEdit `catalog/tests/fixtures/stereo-ktx2/`, producer delivery `3da2d74` (inspected HEAD `84ff741`). `astc6-fixtures.json` records source PNG, KTX2 and reconstructed-pack hashes and exact KTX v4.4.2 arguments. No ImageEdit runtime dependency is introduced.

60x24 SBS, 30x24 padded tiles from asymmetric 29x21 RGBA eyes. One mip, straight alpha, sRGB, native ASTC 6x6. None/Zstd9 decode to exactly 640 block bytes, SHA256 `b589489498107fdbab6c0f1cd0e0e3b42130b164fa3dbe359c0dab71ecbdeaad`.

The two `uastc-*.ktx2` fixtures retain the prior producer's 32x16 UASTC profile, 512 decoded bytes, SHA256 `34368af244a5412586c86241ca18a123e62a6d27c3d78e4abde7c937acf81161`. They exercise the unchanged transcode path alongside direct ASTC.

`SKKtx2Tests` validates CPU parsing/decompression without graphics. `SKTests -astc6test` exercises concrete format support, info/decode agreement and the public asynchronous memory loader. Unsupported GPUs must fail ASTC requests explicitly and still load UASTC; only supported devices can exercise ASTC pixel upload. No physical Quest visual qualification is inferred from these fixtures.
