# Qclassification

A new Python API for Intention classification at Qwant Research.
The API is based on fasttext a c++ toolkit for word embeddings and neural classification.

Contact: c.servan@qwantresearch.com

## Installation

```  git clone https://github.com/QwantResearch/qclassification.git
     git clone https://github.com/QwantResearch/qnlp-toolkit.git
     git clone https://github.com/facebookresearch/fastText.git
  
  sudo -H python3 -m pip --upgrade pytest pybind11 falcon requests json wsgiref falcon falcon_cors 
  
  pushd fastText && mkdir build && cd build && cmake .. && make -j4 && sudo make install && popd
  pushd qnlp-toolkit && mkdir build && cd build && cmake .. && make -j4 && sudo make install && popd
  pushd qclassification/fastText/ && bash compile.bash && popd
  pushd qclassification/libTokenizer/ && bash cbuild.sh && popd
  
``` 

## Launch the API

Set up the models in the file `models_config.txt` and set the IP and desired listened port in the file `qclassif_api.py`.
Then:

```  
  cd qclassif
  python3 qclassif_api.py
``` 

