#ifndef _BBM_BITMAP_H_
#define _BBM_BITMAP_H_

#include <memory>
#include <algorithm>

/***********************************************************************/
/*! \file bitmap.h
    \brief Minimal bitmap class
************************************************************************/

namespace bbm {

  template<typename T>
    class bitmap
  {
  public:
    //! \brief Default constructor
    bitmap(void) : _width(0), _height(0), _data(nullptr) {}

    //! \brief Construct an empty/unitialized bitmap of size width x height
    bitmap(size_t width, size_t height)
    {
      reshape(width, height);
    }

    //! \brief Construct an image of size width x height and set every pixel to 'init'
    bitmap(size_t width, size_t height, const T& init) : bitmap(width, height)
    {
      std::fill(begin(), end(), init);
    }

    //! \brief Copy constructor
    bitmap(const bitmap<T>& src) : bitmap(src._width, src._height)
    {
      std::copy(src.begin(), src.end(), begin());
    }

    //! \brief Move constructor
    bitmap(bitmap<T>&& src) : bitmap()
    {
      std::swap(_width, src._width);
      std::swap(_height, src._height);
      std::swap(_data, src._data);
    }

    //! \brief Assignment operator
    bitmap<T>& operator=(const bitmap<T>& src)
    {
      if(this == &src) return *this;
      reshape(src._width, src._height);
      std::copy(src.begin(), src.end(), begin());
      return *this;
    }

    //! \brief Move assignment operator
    bitmap<T>& operator=(bitmap<T>&& src)
    {
      if(this == &src) return *this;
      std::swap(_width, src._width);
      std::swap(_height, src._height);
      std::swap(_data, src._data);
      return *this;
    }
    
    //! \brief reshape the bitmap (loss of old data)
    void reshape(size_t width, size_t height)
    {
      _width = width;
      _height = height;

      _data.reset();
      if(_width != 0 && _height != 0) 
        _data = std::make_unique<T[]>(_width*_height);
    }
    
    //! @{ \name iterator support
    const T* begin(void) const { return _data.get(); }
    const T* end(void) const { return begin() + _width*_height; }

    T* begin(void) { return _data.get(); }
    T* end(void) { return begin() + _width*_height; }
    //! @}

    //! @{ \name Inspectors
    size_t width() const { return _width; }
    size_t height() const { return _height; }
    
    const T& operator()(size_t x, size_t y) const { return _data[y*_width + x]; }
    T& operator()(size_t x, size_t y) { return _data[y*_width + x]; }
    //! @}
    
  private:
    //////////////////
    // Data Members 
    //////////////////
    size_t _width, _height;
    std::unique_ptr<T[]> _data;
  };

} // end bbm namespace

#endif /* _BBM_BITMAP_H_ */
