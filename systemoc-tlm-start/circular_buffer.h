
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "debug_on.h"


/******************************************************************************
 *
 */
template<typename T>
class circular_buffer
{
public:
  // FIXME: T has to be default constructable. Concept check this!
  typedef T                           data_type;
  typedef circular_buffer<data_type>  this_type;

  //
  circular_buffer(const int size = 1) :
    m_buffer(size), // fill buffer with default constructed objects
    m_size(size),
    m_read(0),
    m_write(0),
    m_num_data(0)
  {}

  // buffer size
  size_t get_size(void) const { return m_size; }

  // free slots
  size_t num_free(void) const { return m_size - m_num_data; }

  // available data
  size_t num_available(void) const { return m_num_data; }

  //
  bool is_empty(void) const { return m_num_data == 0; }

  //
  bool is_full(void) const { return ((m_write == m_read) && !is_empty()); }

  //
  void put(const data_type &d);

  //
  data_type get(void);

  // skip n data without getting it
  void skip(const size_t n)
  {
    assert(n <= m_num_data);
    m_read = (m_read + n) % m_size;
    m_num_data -= n;
  }

protected:
  std::vector<data_type>  m_buffer;

  const size_t            m_size;
  size_t                  m_read;
  size_t                  m_write;
  size_t                  m_num_data;
};


//
template<typename T>
void circular_buffer<T>::put(const data_type &d)
{
  DBG_OUT("put(): num_data: " << m_num_data << std::endl);
  assert(!is_full());

  m_buffer[m_write] = d;
  m_write = (m_write + 1) % m_size;
  ++m_num_data;
}


//
template<typename T>
T circular_buffer<T>::get(void)
{
  assert(!is_empty());
  DBG_OUT("get(): num_data: " << m_num_data << std::endl);

  const size_t ret = m_read;
  m_read = (m_read + 1) % m_size;
  --m_num_data;
  DBG_OUT("get(): num_data: " << m_num_data << std::endl);
  return m_buffer[ret];
}


/******************************************************************************
 * circular buffer including random access to all its data by operator[].
 *
 */
template<typename T>
class circular_buffer_ra_data :
  public circular_buffer<T>
{
public:
  typedef circular_buffer_ra_data<T>              this_type;
  typedef typename circular_buffer<T>::data_type  data_type;

  //
  circular_buffer_ra_data(const size_t n) :
    circular_buffer<T>(n)
  {}

  // random access data in buffer
  const data_type &operator[](size_t n) const
  {
    assert(n < this->num_available());
    const size_t pos = (this->m_read + n) % this->get_size();
    return this->m_buffer[pos];
  }

  // this allows you to even alter the data.
  data_type &operator[](size_t n)
  {
    return const_cast<data_type&>(static_cast<const this_type&>(*this)[n]);
  }
};


/*****************************************************************************/


/*
 *
 */
template<typename T>
void set_all_vec_elements(std::vector<T> &vec, const T t)
{
  typedef std::vector<T> vector_type;
  typedef typename vector_type::iterator iter_type;

  for (iter_type iter = vec.begin(); iter != vec.end(); ++iter)
    *iter = t;
}


/*
 * check if one vector element in given range equals 't'.
 */
template<typename T>
bool vec_element_equals(typename std::vector<T>::const_iterator iter,
                        const typename std::vector<T>::const_iterator end,
                        const T &t)
{
  for (; iter != end; ++iter)
    if (*iter == t)
      return true;

  return false;
}


/******************************************************************************
 * circular buffer including random access to all its free space by operator[].
 *
 */
template<typename T>
class circular_buffer_ra_free_space :
  public circular_buffer<T>
{
private:
  typedef circular_buffer<T> base_type;
  typedef std::vector<bool>  bitvector_type;

public:
  typedef circular_buffer_ra_free_space<T>        this_type;
  typedef typename circular_buffer<T>::data_type  data_type;

  // compiler can't find members from template base class. 'using' them avoids
  //   prefixing every access with 'this->'
  using base_type::num_free;
  using base_type::m_write;
  using base_type::m_size;
  using base_type::m_buffer;
  using base_type::m_num_data;

  //
  circular_buffer_ra_free_space(const size_t n) :
    circular_buffer<T>(n),
    m_written_flags(n, false) // TODO: maybe to big
  {}

  // random access free space in buffer
  const data_type &operator[](const size_t n) const
  {
    assert(n < num_free());
    const size_t pos = (m_write + n) % m_size;
    return m_buffer[pos];
  }

  //
  data_type &operator[](const size_t n)
  {
    // remember all write attempts (for commit)
    m_written_flags[n] = true;
    // use const version to avoid code duplication
    return const_cast<data_type&>(static_cast<const this_type&>(*this)[n]);
  }

  // to let someone read your data you have to commit random accessed space
  //  from time to time with this function.
  void commit_ra_write(const size_t n = 1);

private:
  // disable put() because weird things can happen when mixed with random
  //  access.
  void put(const data_type &d) {}
  
  bitvector_type m_written_flags;
};


//
template<typename T>
void circular_buffer_ra_free_space<T>::commit_ra_write(const size_t n)
{
  bitvector_type::const_iterator iter = m_written_flags.begin();

  assert(n <= num_free());
  
  // every commited token must be written
  for (size_t i = n; i > 0; --i) {
    assert(iter != m_written_flags.end());
    assert(*iter == true);
    ++iter;
  }

  // no other token should be written
  if (vec_element_equals(iter, m_written_flags.end(), true)) {
    assert(0);
  }

  m_write = (m_write + n) % m_size;
  m_num_data += n;

  // FIXME: set only needed
  set_all_vec_elements(m_written_flags, false);
}


#endif // CIRCULAR_BUFFER_H
