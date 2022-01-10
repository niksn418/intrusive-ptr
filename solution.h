#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

template<typename T>
using intrusive_ptr = boost::intrusive_ptr<T>;

template<typename T>
using intrusive_ref_counter = boost::intrusive_ref_counter<T>;
