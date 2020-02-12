import ffmpeg

stream = ffmpeg.input(" rtsp://170.93.143.139/rtplive/470011e600ef003a004ee33696235daa", ss=0)
file = stream.output("test.png")
