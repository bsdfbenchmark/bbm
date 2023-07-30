#ifndef _BBM_PFM_H_
#define _BBM_PFM_H_

#include <array>
#include <string>
#include <cstddef>
#include <fstream>
#include <stdexcept>

#include "core/Endian.h"
#include "core/bitmap.h"
#include "util/iterator_util.h"

/************************************************************************/
/*! \file pfm.h
    \brief Read/write a bitmap to/from PFM
*************************************************************************/

namespace bbm {
  namespace io {

    /********************************************************************/
    /*! \brief Export a bitmap to a PFM

      \param filename = PFM filename to write to.
      \param data = bitmap to export
      \param channels = which channels to export to the PFM file (default {0,1,2})
                        use -1 to ignore (write 0)

      Write a bitmap to a 3-channel little endian PFM file. Allows the
      selection of channels to write out in case the bitmap contains more (or
      less) than 3 channels.
    *********************************************************************/
    template<typename T>
      void exportPFM(const std::string& filename, const bitmap<T>& data, const std::array<ptrdiff_t,3>& channels = {0,1,2})
    {
      // check for empty data
      if(data.width() == 0 || data.height() == 0) throw std::runtime_error("io::exportPFM: No data in bitmap");

      // open
      std::ofstream ofs(filename.c_str(), std::ios::binary);

      // write header: (3 channel, little endian only)
      char header[3] = {'P', 'F', '\n'};
      ofs.write(static_cast<char *>(header), 3);

      // write image size
      const unsigned int num_channels = 3;
      ofs << data.width() << " " << data.height() << '\n';
      
      // write endianess (little)
      ofs << "-1.000000" << '\n';

      // convert data to output format
      size_t size = data.width()*data.height()*num_channels;
      auto buffer = std::make_unique<float[]>(size);

      auto buffer_ref = buffer.get();
      for(size_t y=data.height()-1; /*y >= 0 &&*/ y < data.height(); y--)
        for(size_t x=0; x < data.width(); x++, buffer_ref += 3)
        {
          auto data_ref = bbm::begin(data(x,y));
          auto data_size = bbm::size(data(x,y));

          for(size_t c = 0; c < num_channels; c++)
            buffer_ref[c] = (channels[c] >= 0 && size_t(channels[c]) < data_size) ? float(data_ref[channels[c]]) : 0.0f;
        }
      
      // ensure the data is stored as little endian
      endian::little(buffer.get(), buffer.get() + size);

      // write buffer
      ofs.write(reinterpret_cast<char*>(buffer.get()), size*sizeof(float));

      // Done.
    }

    
    /********************************************************************/
    /*! \brief Import a bitmap from a PFM

      \param filename = PFM filename to read from
      \param data = bitmap store the data in
      \param channels = which channels in data to fill (default {0,1,2}; 
                        use -1 to ignore a channel from the PFM)

      Read a bitmap from a PFM file (1 and 3 channels as well as little and
      big endian PFMs are supported). Allows the selection of channels to
      write out in in the bitmap.
    *********************************************************************/
    template<typename T, size_t N=3>
      void importPFM(const std::string& filename, bitmap<T>& data, const std::array<ptrdiff_t,N>& channels = {0,1,2})
    {
      // open
      std::ifstream ifs(filename.c_str(), std::ios::binary);

      // scan header
      char header[3];
      ifs.read(static_cast<char *>(header), 3);
      if(header[0] != 'P') throw std::runtime_error("io::importPFM: Not a recognized PFM format");
      char type = header[1];

      // skip comments
      if(ifs.peek() == '#') ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // get width, and height
      size_t width, height;
      ifs >> width >> height;

      // skip comments (again)
      if(ifs.peek() == '#') ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // get endianess
      float endianess;
      ifs >> endianess;

      bool bigEndian = (endianess > 0);
        
      // Did we read too much?
      ifs.unget();
      while(ifs.get() != '0') ifs.unget();
      ifs.get();

      // determine number of channels in file
      size_t num_channels = (type == 'f') ? 1 : 3;

      // grab data from file
      size_t size = width*height*num_channels;
      auto buffer = std::make_unique<float[]>(size);
      ifs.read(reinterpret_cast<char*>(buffer.get()), size*sizeof(float));

      // convert PFM endian representation to machine endian representation
      if(bigEndian) endian::big(buffer.get(), buffer.get() + size);
      else endian::little(buffer.get(), buffer.get() + size);
        
      // if same size; keep original bitmap and fill in
      // if different size, reshape bitmap
      if(data.width() != width || data.height() != height)
          data.reshape(width, height);
        
      // copy data into bitmap guided by channels
      auto buffer_ref = buffer.get();
      for(size_t y=height-1; /*y >= 0 &&*/ y < height; y--)
        for(size_t x=0; x < width; x++, buffer_ref += num_channels)
        {
          auto data_ref = bbm::begin(data(x,y));
          auto data_size = bbm::size(data(x,y));
          
          for(size_t c = 0; c < std::min(N, num_channels); c++)
            if(channels[c] >= 0 && channels[c] < data_size)
              data_ref[channels[c]] = buffer_ref[c];
        }
      
      // Done.	    
    }
    
  } // end io namespace
} // end bbm namespace

#endif /* _BBM_PFM_H_ */
