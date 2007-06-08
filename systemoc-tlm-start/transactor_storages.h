
#ifndef TRANSACTOR_STORAGES_H
#define TRANSACTOR_STORAGES_H


#include <systemoc/smoc_chan_if.hpp>

#include "circular_buffer.h"
#include "debug_on.h"



/******************************************************************************
 * get and put data in bytewise manner. This makes it independent of concrete
 * data type. TODO: split in read/write
 *
 */
class StorageByteAccessIf
{
public:
  typedef unsigned char byteType;
  
  // storage is empty?
  virtual bool isEmpty(void) const = 0;

  // storage has data available?
  virtual bool isFree(void) const = 0;

  // not virtual to prevent bypassing of test
  size_t getBytes(byteType *a, const size_t n)
  {
    DBG_OUT("StorageByteAccessIf::getBytes().\n");
    if (isEmpty() || (n < dataSize()))
      return 0;

    return getBytesImpl(a, dataSize());
  }

  // NOTE: now n == dataSize(), maybe allow bursts
  size_t putBytes(const byteType *a, const size_t n)
  {
    DBG_OUT("StorageByteAccessIf::putBytes(); n = " << n << std::endl);
    if (!isFree() || (n != dataSize()))
      return 0;

    return putBytesImpl(a, dataSize());
  }

private:
//protected:
  //
  virtual size_t getBytesImpl(byteType *a, const size_t n) = 0;

  //
  virtual size_t putBytesImpl(const byteType *a, const size_t n) = 0;

  //
  virtual size_t dataSize(void) const = 0;
};


/******************************************************************************
 * smoc_port_in_plug needs this interface to access storage
 *
 */
template<typename T>
class SmocPortInStorageReadIf
{
public:
  typedef smoc_channel_access<const T&>         readAccessType;
  typedef typename readAccessType::return_type  accessReturnType;//const T&

  //
  virtual readAccessType* getReadChannelAccess(void) = 0;
};


/******************************************************************************
 * smoc_port_out_plug needs this interface to access storage.
 *
 */
template<typename T>
class SmocPortOutStorageWriteIf
{
public:
  typedef smoc_channel_access<smoc_storage_wom<T> >  writeAccessType;
  typedef typename writeAccessType::return_type      accessReturnType;

  //
  virtual writeAccessType* getWriteChannelAccess(void) = 0;
};


/******************************************************************************
 * Functions independent from data type and needed by transactor and adapter
 *
 */
class StorageInIf :
  public StorageByteAccessIf
{
public:
  //
  virtual size_t numAvailable(void) const = 0;

  //
  virtual void skip(const size_t n) = 0;

  //
  virtual void invalidate(const size_t n) = 0;
};


/******************************************************************************
 *
 *
 */
class StorageOutIf :
  public StorageByteAccessIf
{
public:
  //
  virtual bool tokenIsValid(size_t n) const = 0;

  //
  virtual size_t numFree(void) const = 0;

  //
  virtual void commitRaWrite(size_t n) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class SmocPortInStorage :
  public SmocPortInStorageReadIf<T>, // interface for smoc port
  //public StorageByteAccessIf,        // interface for PlugAggregation
  public StorageInIf                 // remaining data independent functions
{
public:
  typedef typename SmocPortInStorageReadIf<T>::readAccessType   readAccessType;
  typedef typename SmocPortInStorageReadIf<T>::accessReturnType
    accessReturnType;
  typedef circular_buffer_ra_data<smoc_storage<T> >             storageType;

  //
  SmocPortInStorage(const size_t size = 1) :
    mStorage(size),
    mReadAccessWrapper(mStorage)
  {}

  //
  virtual ~SmocPortInStorage(void) {}

  // wrapper for random_access_circular_buffer::is_full()
  bool isFull(void) const { return mStorage.is_full(); }

  // wrapper for random_access_circular_buffer::put()
  void put(const T& t) { mStorage.put(t); }
  
  /*
   * StorageInIf
   */
  //
  size_t numAvailable(void) const { return mStorage.num_available(); }

  // wrapper for random_access_circular_buffer::skip()
  void skip(const size_t n) { mStorage.skip(n); }

  // call invalidate() on smoc_storages
  void invalidate(const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
      mStorage[i].invalidate();
  }

  /*
   * StorageByteAccessIf
   */
  // wrapper for random_access_circular_buffer::is_empty()
  bool isEmpty(void) const { return mStorage.is_empty(); }

  //
  bool isFree(void) const { return mStorage.num_free() > 0; }

  /*
   * smoc_port_in_storage_read_if
   */
  //
  readAccessType* getReadChannelAccess(void) { return &mReadAccessWrapper; }

private:
  /*
   * StorageByteAccessIf
   */
  // only smoc port should read from storage
  size_t getBytesImpl(byteType *a, const size_t n)
  {
    assert(0);
    return 0;
  }

  //
  size_t putBytesImpl(const byteType *a, const size_t n)
  {
    assert(n == dataSize());
    mStorage.put(*(reinterpret_cast<const T*>(a)));
    return dataSize();
  }

  //
  size_t dataSize(void) const { return sizeof(T); }


  /********************************************************
   * wraps our storage so smoc_port_in likes it
   */
  class ReadAccessWrapper :
    public readAccessType
  {
  public:
    ReadAccessWrapper(const storageType &storage) :
      mStorage(storage)
#ifndef NDEBUG
      , mLimit(0)
#endif
    {}

    /*
     * write_access_type
     */
#ifndef NDEBUG
    // set limit and assert in operator[]
    void setLimit(size_t l) { mLimit = l; }
#endif

    const accessReturnType operator[](size_t n) const
    {
      assert(n < mLimit);
      return mStorage[n];
    }

    accessReturnType operator[](size_t n)
    {
      assert(n < mLimit);
      return mStorage[n];
    }
    
    bool tokenIsValid(size_t n) const
    {
      assert(n < mLimit);
      return mStorage[n].isValid();
    }

  private:
    const storageType  &mStorage;
#ifndef NDEBUG
    size_t              mLimit;
#endif
  };
  /*******************************************************/

  // local storage
  storageType        mStorage;  
  // and its wrapper object
  ReadAccessWrapper  mReadAccessWrapper;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class SmocPortOutStorage :
  public SmocPortOutStorageWriteIf<T>, // interface for smoc port
  //public StorageByteAccessIf,          // interface for PlugOutAggregation
  public StorageOutIf
{
public:
  typedef T  dataType;
  typedef typename SmocPortOutStorageWriteIf<T>::writeAccessType
    writeAccessType;
  typedef typename SmocPortOutStorageWriteIf<T>::accessReturnType
    accessReturnType;
  typedef circular_buffer_ra_free_space<smoc_storage<T> >  storageType;

  //
  SmocPortOutStorage(const size_t size = 1) :
    mStorage(size),
    mWriteAccessWrapper(mStorage)
  {}

  //
  virtual ~SmocPortOutStorage(void) {}

  // FIXME: some functions into StorageOutIf?
#if 0
  //
  void invalidate(const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
      mStorage[i].invalidate();
  }
#endif

  // wrapper for random_access_circular_buffer::is_full()
  bool isFull(void) const { return mStorage.is_full(); }

  // wrapper for circular_buffer_ra_write::get()
  //dataType get(void) { return mStorage.get(); }

  // wrapper for circular_buffer_ra_write::num_available()
  size_t numAvailable(void) const { return mStorage.num_available(); }

  // wrapper for circular_buffer_ra_write::operator[]
  const dataType& operator[](size_t n) const
  {
    assert(mStorage[n].isValid());
    return mStorage[n];
  }

  /*
   * StorageByteAccessIf
   */
  // wrapper for random_access_circular_buffer::is_empty()
  bool isEmpty(void) const { return mStorage.is_empty(); }

  //
  bool isFree(void) const { return mStorage.num_free() > 0; }

  /*
   * StorageOutIf
   */
  //
  bool tokenIsValid(size_t n) const { return mStorage[n].isValid(); }

  // wrapper for circular_buffer_ra_write::num_free()
  size_t numFree(void) const { return mStorage.num_free(); }

  // wrapper for circular_buffer_ra_write::commit_ra_write()
  void commitRaWrite(size_t n) { return mStorage.commit_ra_write(n); }

  /*
   * SmocPortOutStorageWriteIf
   */
  writeAccessType* getWriteChannelAccess(void)
  {
    return &mWriteAccessWrapper;
  }

private:
  /*
   * StorageByteAccessIf
   */
  // only smoc port should read from storage
  size_t getBytesImpl(byteType *a, const size_t n)
  {
    assert(n == dataSize());
    T data = mStorage.get();
    memcpy(a, &data, sizeof(data));
    return dataSize();
  }

  //
  size_t putBytesImpl(const byteType *a, const size_t n)
  {
    assert(0);
    return 0;
  }

  //
  size_t dataSize(void) const { return sizeof(T); }

  /********************************************************
   * wraps our storage for smoc_port_out
   */
  class writeAccessWrapper :
    public writeAccessType
  {
  public:
    writeAccessWrapper(storageType &storage) :
      mStorage(storage)
#ifndef NDEBUG
      , mLimit(0)
#endif
    {}

    /*
     * write_access_type
     */
#ifndef NDEBUG
    // set limit and assert in operator[]
    void setLimit(size_t l) { mLimit = l; }
#endif

    //
    const accessReturnType operator[](size_t n) const
    {
      assert(n < mLimit);
      return mStorage[n];
    }

    //
    accessReturnType operator[](size_t n)
    {
      assert(n < mLimit);
      return mStorage[n];
    }
    
    //
    bool tokenIsValid(size_t n) const
    {
      assert(n < mLimit);
      return mStorage[n].isValid();
    }

  private:
    storageType  &mStorage;
#ifndef NDEBUG
    size_t         mLimit;
#endif
  };
  /*******************************************************/

  // local storage
  storageType         mStorage;
  // and its wrapper object
  writeAccessWrapper  mWriteAccessWrapper;
};



#endif // TRANSACTOR_STORAGES_H
