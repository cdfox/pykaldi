// -*- coding: utf-8 -*-
/* Copyright (c) 2013, Ondrej Platek, Ufal MFF UK <oplatek@ufal.mff.cuni.cz>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
 * WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
 * MERCHANTABLITY OR NON-INFRINGEMENT.
 * See the Apache 2 License for the specific language governing permissions and
 * limitations under the License. */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include "pykaldi-gmm-decode-faster.h"

using namespace kaldi;

// function types for loading functions from shared library
typedef CKaldiDecoderWrapper* (*CKDW_constructor_t)(void);
typedef void (*CKDW_void_t)(CKaldiDecoderWrapper*);
typedef bool (*CKDW_bool_t)(CKaldiDecoderWrapper*);
typedef size_t (*CKDW_size_t)(CKaldiDecoderWrapper*);
typedef void (*CKDW_frame_in_t)(CKaldiDecoderWrapper*, unsigned char *, size_t);
typedef void (*CKDW_pop_hyp_t)(CKaldiDecoderWrapper*, int *, size_t);
typedef int (*CKDW_setup_t)(CKaldiDecoderWrapper*, int, char **);

template<class FT> FT load_function(const char *nameFce, void *lib);
void fill_frame_random(unsigned char *frame, size_t size);
size_t next_frame(unsigned char * data_target, size_t target_size,
                  unsigned char *src, size_t src_position);
int main(int argc, char **argv);
void printHyp(int *word_ids, size_t num_words);
size_t read_16bitpcm_file(const std::string & filename, unsigned char **pcm);


int main(int argc, char **argv) {
  // open the library
  char nameLib[] = "libpykaldi.so";
  void *lib = dlopen(nameLib, RTLD_NOW);
  if (!lib) {
      printf("Cannot open library: %s\n", dlerror());
      return 1;
  }   
  // load functions from shared library
  CKDW_constructor_t new_Decoder = load_function<CKDW_constructor_t>("new_KaldiDecoderWrapper", lib);
  if (!new_Decoder) return 2;
  CKDW_void_t del_Decoder = load_function<CKDW_void_t>("del_KaldiDecoderWrapper", lib);
  if (!del_Decoder) return 3;
  CKDW_setup_t setup = load_function<CKDW_setup_t>("Setup", lib);
  if (!setup) return 4;
  CKDW_void_t reset = load_function<CKDW_void_t>("Reset", lib);
  if (!reset) return 5;
  CKDW_frame_in_t frame_in = load_function<CKDW_frame_in_t>("FrameIn", lib);
  if (!frame_in) return 6;
  CKDW_size_t decode = load_function<CKDW_size_t>("Decode", lib);
  if (!decode) return 7;
  CKDW_size_t finish_decoding = load_function<CKDW_size_t>("FinishDecoding", lib);
  if (!finish_decoding) return 8;
  CKDW_pop_hyp_t pop_hyp = load_function<CKDW_pop_hyp_t>("PopHyp", lib);
  if (!pop_hyp) return 9;

  // use the loaded functions
  CKaldiDecoderWrapper *d = new_Decoder();
  int retval = setup(d, argc, argv);
  if(retval != 0)
    std::cout << "\nWrong arguments!\n"
      "Return code 0 because it is mainly used as test!\n" << std::endl;
    return 0;

  unsigned char * pcm;
  std::string filename("pykaldi/binutils/online-data/audio/test1.wav");
  size_t pcm_size = read_16bitpcm_file(filename, &pcm);
  
  // send data in at once, use the buffering capabilities
  size_t frame_len = 2120, pcm_position = 0;
  // reading 16bit audio into char array -> 1 sample == 2 chars
  size_t frame_size = frame_len * 2; 
  unsigned char *frame = new unsigned char[frame_size];
  while(pcm_position + frame_size < pcm_size) {
    pcm_position = next_frame(frame, frame_size, pcm, pcm_position);
    frame_in(d, frame, frame_len);
  }
  delete[] frame;


  // decode() returns false if there are no more features for decoder
  size_t total_words = 0;
  for(size_t i = 0; i < 100; ++i) {
      size_t num_words = decode(d);
      int * word_ids = new int[num_words];
      pop_hyp(d, word_ids, num_words);
      printHyp(word_ids, num_words);
      delete[] word_ids;

      total_words += num_words;
  } 
  // Obtain last hypothesis
  {
      size_t num_words = finish_decoding(d);
      int * word_ids = new int[num_words];
      pop_hyp(d, word_ids, num_words);
      printHyp(word_ids, num_words);
      delete[] word_ids;

      total_words += num_words;
  }
  del_Decoder(d);

  std::cout << "Totally decoded words: " << total_words << std::endl;
  dlclose(lib);
  return 0;
}

void printHyp(int *word_ids, size_t num_words) {
  std::cout << ", decoded words: " << num_words << std::endl;
  // print the words
  for(size_t j = 0; j < num_words; ++j)
    std::cout << word_ids[j] << " ";
  std::cout << std::endl;
}


template<class FT> FT load_function(const char *nameFce, void *lib) {
  FT f = NULL;

  dlerror();  // reset errors
  if (!lib) {
      printf("Cannot open library: %s\n", dlerror());
      return NULL;
  }   

  dlerror();  // reset errors
  f = (FT)dlsym(lib, nameFce); 
  const char *dlsym_error = dlerror();
  if (dlsym_error) {
      printf("Cannot load symbol '%s', %s\n", nameFce, dlsym_error );
      dlclose(lib);
      return NULL;
  }

  return f;
}

void fill_frame_random(unsigned char *frame, size_t size) {
  for(size_t i=0; i<size; ++i) {
    frame[i] = random() & 0xff;
  }
}

/// Read the file like it is 16bit raw pcm
size_t read_16bitpcm_file(const std::string & filename, unsigned char **pcm) {
  size_t size = 0;
  *pcm = NULL;

  std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
  if (file.is_open()) {
    size = file.tellg();
    *pcm = new unsigned char [size];
    file.seekg(0, std::ios::beg);
    file.read((char *)*pcm, size);
    file.close();
  }
  return size;
}

size_t next_frame(unsigned char * data_target, size_t target_size,
                  unsigned char *src, size_t src_position) {
  for(size_t i=0; i < target_size; ++i) {
    data_target[i] = src[src_position + i];
  }
  return src_position + target_size;
}
