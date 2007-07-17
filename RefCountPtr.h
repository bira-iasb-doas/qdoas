#ifndef _REFCOUNTPTR_H_GUARD
#define _REFCOUNTPTR_H_GUARD

// reference counted pointer

template<typename T>
class RefCountPtr {
 public:
  RefCountPtr();
  RefCountPtr(T *t);
  RefCountPtr(const RefCountPtr<T> &other);
  ~RefCountPtr();

  RefCountPtr<T>& operator=(const RefCountPtr<T> &rhs);

  T& operator*(void);
  const T& operator*(void) const;
  T* operator->(void);
  const T* operator->(void) const;

  bool operator==(const RefCountPtr &rhs) const;
  bool operator!=(const RefCountPtr &rhs) const;

  bool operator==(unsigned long rhs) const;
  bool operator!=(unsigned long rhs) const;

 private:
  T *m_d;                     // data
  mutable unsigned int *m_rc; // reference count
};

template<typename T>
RefCountPtr<T>::RefCountPtr() :
  m_d(NULL),
  m_rc(NULL)
{
}

template<typename T>
RefCountPtr<T>::RefCountPtr(T *t) :
  m_d(NULL),
  m_rc(NULL)
{
  if (t) {
    m_d = t;
    m_rc = new unsigned int;
    *m_rc = 1;
  }
}

template<typename T>
RefCountPtr<T>::RefCountPtr(const RefCountPtr<T> &other) :
  m_d(other.m_d),
  m_rc(other.m_rc)
{  
  // increase the count if the count storage exists
  if (m_rc) ++(*m_rc);
}

template<typename T>
RefCountPtr<T>::~RefCountPtr()
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references to the data ... delete
    delete m_d;
    delete m_rc;
  }
}

template<typename T>
RefCountPtr<T>& RefCountPtr<T>::operator=(const RefCountPtr<T> &rhs)
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references
    delete m_d;
    delete m_rc;
  }
  m_d = rhs.m_d;   // point to same data
  m_rc = rhs.m_rc; // same ref count storage 
  
  if (m_rc) ++(*m_rc); // increase the reference count

  return *this;
}


template<typename T>
T& RefCountPtr<T>::operator*(void)
{
  return *m_d;
}

template<typename T>
const T& RefCountPtr<T>::operator*(void) const
{
  return *m_d;
}

template<typename T>
T* RefCountPtr<T>::operator->(void)
{
  return m_d;
}

template<typename T>
const T* RefCountPtr<T>::operator->(void) const
{
  return m_d;
}

template<typename T>
bool RefCountPtr<T>::operator==(const RefCountPtr &rhs) const
{
  return (m_d == rhs.m_d);
}
 
template<typename T>
bool RefCountPtr<T>::operator!=(const RefCountPtr &rhs) const
{
  return (m_d != rhs.m_d);
}

template<typename T>
bool RefCountPtr<T>::operator==(unsigned long rhs) const
{
  return (m_d == (T*)rhs);
}

template<typename T>
bool RefCountPtr<T>::operator!=(unsigned long rhs) const
{
  return (m_d != (T*)rhs);
}

#endif
