#include <hip/hip_runtime.h>


//wrapper template to manage arrays on the device. it is in charge of managing them as RAII, with the malloc done on the
//constructors and the free done in the destructor. 
//It will only use async operations to perform memcpy from host to device and vice versa,
//for this reason all the memcpy and memset function will require a stream parameter
//operations with 0 elements are wrong. this will not be enforced, and if an operation is done with that value is performed on the whole array
//as it is used as default value for the functions
//passing more elements than available space will result in undefined behavior
//creating a mem_wrapper with 0 elements is meaningless and should not be done.
template<typename T>
struct mem_wrapper{
  
  using value_type = T;

  T* host_ptr;
  T* dev_ptr;
  std::size_t num_elem;
  int device_id;

  struct dev_side{
    T *__restrict__ dev_ptr;
    std::size_t num_elems;
  };
  
  //delete default ctor and copy ctor and copy operator
  mem_wrapper()=delete;
  mem_wrapper(const mem_wrapper& other) = delete;
  mem_wrapper& operator=( const mem_wrapper& other )=delete;

  //define ctor and dtor
  mem_wrapper(std::size_t size)
  {
    hipSetDevice(&device_id);
    hipHostAlloc(host_ptr,size * sizeof(T));
    hipAlloc(dev_ptr,size * sizeof(T));
    num_elem = size;
  }

  ~mem_wrapper()
  {
    hipSetDevice(device_id);
    if (host_ptr != nullptr)
      hipHostFree(host_ptr);
    if (dev_ptr != nullptr)
      hipFree(dev_ptr);
  }

  //define move ctor
  mem_wrapper( mem_wrapper&& other)
  {
    host_ptr = other.host_ptr;
    dev_ptr = other.dev_ptr;
    other.host_ptr=nullptr;
    other.dev_ptr=nullptr;
  }
  //define move operator
  mem_wrapper& operator=( mem_wrapper&& other ){
    //call the "destructor" to free the old allocation
    hipSetDevice(device_id);
    if(dev_ptr!=nullptr)    
      hipFree(    dev_ptr);
    if(host_ptr!=nullptr)  
      hipHostFree(host_ptr);
    //call the "move ctor" to get the new values
    dev_ptr = other.dev_ptr; other.dev_ptr=nullptr;
    host_ptr = other.host_ptr; other.host_ptr=nullptr;
    array_elems=other.array_elems;
    device_id=other.device_id;
    return *this;
  }

  //to simplify the usage of these structures we also provide "update host" and other ancillary functions. 
  //Two function to copy in-out are provided: one is "punctual", in the sense that copies a num of elems that NEEDS to be specified
  //at a given offset (that MUST be provided)
  //The other will just copy "all", which means that is up to the programmer to be sure that the size of the source pointer
  //is the same of array_elems of this item.
  //cpy in and out will work with the HOST side of the mem wrapper!
  void copy_in(const T* __restrict__ other, const std::size_t offset, const std::size_t num_elem)
  {
    std::memcpy(&host_ptr[offset],other,num_elem*sizeof(T));
  }
  void copy_in(const T* __restrict__ other)
  {
    std::memcpy(host_ptr,other,array_elems*sizeof(T));
  }
  void copy_out(T* __restrict__ other, const std::size_t offset, const std::size_t num_elem) const
  {
    std::memcpy(other,&host_ptr[offset],num_elem*sizeof(T));
  }
  void copy_out(T* __restrict__ other) const
  {
    std::memcpy(other,host_ptr,array_elems*sizeof(T));
  }

  //d2h and h2d function will move the WHOLE array between host and device.
  //move d2h (size)
  void cpy_to_host(const hipStream_t local_stream=0)
  {
    hipMemcpyAsync(host_ptr, dev_ptr, array_elems*sizeof(T), hipMemcpyDeviceToHost,local_stream);
  }
  //move h2d (size)
  void cpy_to_dev(const hipStream_t local_stream=0)
  {
    hipMemcpyAsync(dev_ptr, host_ptr, array_elems*sizeof(T), hipMemcpyHostToDevice,local_stream);
  }
      
  //memset functions will work with the whole array.
  //memset host
  void memset_host(const T value)
  {
    std::memset( host_ptr , value, array_elems*sizeof(T));
  }
  //memset device
  void memset_dev(const hipStream_t local_stream=0, const T value=0)
  {
    hipMemsetAsync(dev_ptr , value, array_elems*sizeof(T),local_stream);
  }
      
  //finally a function that gets the device view
  inline dev_side get_device_representation()
  {
    return dev_side{dev_ptr,array_elems};
  }
  

};