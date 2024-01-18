#Docker image with extras
FROM datamachines/jetsonnano-cuda_tensorflow_opencv:10.2_2.3_4.5.1-20210218

RUN apt update && apt install -y cheese camorama rustc geany portaudio19-dev

RUN python3 -m pip install --upgrade pip

RUN pip3 install pyserial openai requests pyttsx3 wave pyusb click pyaudio

RUN pip3 install face_recognition

RUN git clone https://github.com/wjakob/tbb.git && cd tbb/build && cmake .. && make -j && make install

####
RUN apt update && apt dist-upgrade -y

RUN pip3 install --upgrade setuptools && pip3 install -U setuptools && pip3 install cython 

RUN apt-get install -y llvm-10  libblas-dev liblapack-dev libatlas-base-dev gfortran

RUN cd /usr/bin && ln -s llvm-config-10 llvm-config 

RUN pip3 install llvmlite

RUN apt install -y mecab mecab-utils libmecab-dev

RUN pip3 install mecab-python3

RUN pip3 install TTS 

#### ADD more here if needed (DONT TOUCH ABOVE!)
RUN pip3 install pvporcupine argparse pvrecorder
RUN apt update && apt install -y alsa-tools alsa-utils alsa-base
####

#Final cleanup
RUN apt clean && apt autoremove -y && rm -rf /var/lib/apt/lists/*

#ENTRYPOINT ["/usr/sbin/apache2ctl", "-D", "FOREGROUND"]

